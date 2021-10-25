//
// Created by wy on 2021/10/25.
//

#ifndef WY_CUDA_RTREE_H
#define WY_CUDA_RTREE_H
//DATATYPE 数据类型
//ELEMTYPE 元素的类型
//NUMDIMS 维度数2/3
//TMAXNODES 阶数
//ELEMTYPEREAL 计算体积用的，你们可以直接当ELEMTYPE，不用管

//注意：我们使用节点全部用new/delete
//注意：插入和删除数据需要了解其恒定最小边界矩形。
class RTFileStream;

#define RTREE_TEMPLATE template<class DATATYPE, class ELEMTYPE, int NUMDIMS, class ELEMTYPEREAL, int TMAXNODES, int TMINNODES>
#define RTREE_QUAL RTree<DATATYPE, ELEMTYPE, NUMDIMS, ELEMTYPEREAL, TMAXNODES, TMINNODES>
template<class DATATYPE, class ELEMTYPE, int NUMDIMS,
        class ELEMTYPEREAL = ELEMTYPE, int TMAXNODES = 8, int TMINNODES = TMAXNODES / 2>
class RTree
{
protected:

    struct Node;  // 由其他内部结构和迭代器使用

public:

    enum
    {
        MAXNODES = TMAXNODES,                         //节点中最大个数
        MINNODES = TMINNODES,                         //节点中最小个数
    };


public:

    RTree();
    virtual ~RTree();

    //最大点
    //最小点
    //操作数据id 可能是零，但不允许出现负数
    void Insert(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId); //刘


    void Remove(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId);  //王


    //return返回找到的条目数
    //int Search(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], bool a_resultCallback(DATATYPE a_data, void* a_context), void* a_context);

    //从树中删除所有条目
    void RemoveAll();

    //统计此容器中的数据元素。这很慢，因为没有维护内部计数器。
    int Count();

    //从文件中加载树内容
    bool Load(RTFileStream& a_stream);


    //将树内容保存到文件
    bool Save(RTFileStream& a_stream);

    //迭代器
    class Iterator
    {
    private:

        enum { MAX_STACK = 32 }; //最大栈大小。几乎允许n^32，其中n是节点中的分支数

        struct StackElement
        {
            Node* m_node;
            int m_branchIndex;
        };

    public:

        Iterator()                                    { Init(); }

        ~Iterator()                                   { }

        bool IsNull()                                 { return (m_tos <= 0); }

        bool IsNotNull()                              { return (m_tos > 0); }

        //访问当前数据元素。调用方必须首先确保迭代器不为NULL。
        DATATYPE& operator*()
        {
            StackElement& curTos = m_stack[m_tos - 1];
            return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
        }


        //查找下一个数据元素
        bool operator++()                             { return FindNextData(); }

        //获取此节点的边界
        void GetBounds(ELEMTYPE a_min[NUMDIMS], ELEMTYPE a_max[NUMDIMS])
        {
            StackElement& curTos = m_stack[m_tos - 1];
            Branch& curBranch = curTos.m_node->m_branch[curTos.m_branchIndex];

            for(int index = 0; index < NUMDIMS; ++index)
            {
                a_min[index] = curBranch.m_rect.m_min[index];
                a_max[index] = curBranch.m_rect.m_max[index];
            }
        }

    private:

        void Init()                                   { m_tos = 0; }

        //查找树中的下一个数据元素（仅供内部使用）
        bool FindNextData()
        {
            for(;;)
            {
                if(m_tos <= 0)
                {
                    return false;
                }
                StackElement curTos = Pop(); //复制堆栈顶部，因为在使用时它可能会更改

                if(curTos.m_node->IsLeaf())
                {
                    //尽可能地继续浏览数据
                    if(curTos.m_branchIndex+1 < curTos.m_node->m_count)
                    {
                        //还有更多数据，请指向下一个
                        Push(curTos.m_node, curTos.m_branchIndex + 1);
                        return true;
                    }
                    //没有更多的数据，因此它将回落到以前的level
                }
                else
                {
                    if(curTos.m_branchIndex+1 < curTos.m_node->m_count)
                    {
                        Push(curTos.m_node, curTos.m_branchIndex + 1);
                    }
                    // 由于 cur 节点不是叶子，因此先Push到下一层以深入到树中
                    Node* nextLevelnode = curTos.m_node->m_branch[curTos.m_branchIndex].m_child;
                    Push(nextLevelnode, 0);

                    // 如果我们Push了一个新的叶子，当数据在Tos准备好时退出
                    if(nextLevelnode->IsLeaf())
                    {
                        return true;
                    }
                }
            }
        }

        //将节点和分支推送到迭代栈上（仅供内部使用）
        void Push(Node* a_node, int a_branchIndex)
        {
            m_stack[m_tos].m_node = a_node;
            m_stack[m_tos].m_branchIndex = a_branchIndex;
            ++m_tos;
        }

        //从迭代栈中弹出元素（仅供内部使用）
        StackElement& Pop()
        {
            --m_tos;
            return m_stack[m_tos];
        }

        StackElement m_stack[MAX_STACK];              //迭代
        int m_tos;                                    //栈顶索引

        friend RTree;
    };

    //从第一个开始迭代
    void GetFirst(Iterator& a_it)
    {
        a_it.Init();
        Node* first = m_root;
        while(first)
        {
            if(first->IsInternalNode() && first->m_count > 1)
            {
                a_it.Push(first, 1); // 深入到兄弟分支
            }
            else if(first->IsLeaf())
            {
                if(first->m_count)
                {
                    a_it.Push(first, 0);
                }
                break;
            }
            first = first->m_branch[0].m_child;
        }
    }


    void GetNext(Iterator& a_it)                    { ++a_it; }

    //迭代器是 NULL 还是结束
    bool IsNull(Iterator& a_it)                     { return a_it.IsNull(); }

    //在迭代器位置获取对象
    DATATYPE& GetAt(Iterator& a_it)                 { return *a_it; }

protected:

    ///最小边界矩形（n维）
    struct Rect
    {
        ELEMTYPE m_min[NUMDIMS];                      ///边界框的最小尺寸
        ELEMTYPE m_max[NUMDIMS];                      ///边界框的最大尺寸
    };

    /// 可以是数据，也可以是另一子树
    ///  父级决定了这一点。
    /// 如果父级为0，则这是数据
    struct Branch
    {
        Rect m_rect;                                  //Bounds
        union
        {
            Node* m_child;                              //子节点
            DATATYPE m_data;                            //Data Id or Ptr
        };
    };


    struct Node
    {
        bool IsInternalNode()                         { return (m_level > 0); }
        bool IsLeaf()                                 { return (m_level == 0); }

        int m_count;
        int m_level;
        Branch m_branch[MAXNODES];
    };

    ///删除操作后重新插入的节点链接列表
    struct ListNode
    {
        ListNode* m_next;
        Node* m_node;
    };

    ///用于查找分割分区的变量
    struct PartitionVars
    {
        int m_partition[MAXNODES+1];   //划分
        int m_total;     //总量size
        int m_minFill;    //最小填充
        int m_taken[MAXNODES+1];
        int m_count[2];     //分割的两个组里的size
        Rect m_cover[2];     //分割的两个rect
        ELEMTYPEREAL m_area[2];   //

        Branch m_branchBuf[MAXNODES+1];   //当缓存用
        int m_branchCount;      //分支当前数量
        Rect m_coverSplit;   //存缓存当前的矩阵
        ELEMTYPEREAL m_coverSplitArea;    //大小
    };

    Node* AllocNode();
    void FreeNode(Node* a_node);
    void InitNode(Node* a_node);
    void InitRect(Rect* a_rect);
    bool InsertRectRec(Rect* a_rect, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level);     //刘
    bool InsertRect(Rect* a_rect, const DATATYPE& a_id, Node** a_root, int a_level);                           //刘
    Rect NodeCover(Node* a_node);
    bool AddBranch(Branch* a_branch, Node* a_node, Node** a_newNode);
    void DisconnectBranch(Node* a_node, int a_index);
    int PickBranch(Rect* a_rect, Node* a_node);
    Rect CombineRect(Rect* a_rectA, Rect* a_rectB);
    void SplitNode(Node* a_node, Branch* a_branch, Node** a_newNode);
    ELEMTYPEREAL RectSphericalVolume(Rect* a_rect);
    ELEMTYPEREAL RectVolume(Rect* a_rect);
    ELEMTYPEREAL CalcRectVolume(Rect* a_rect);
    void GetBranches(Node* a_node, Branch* a_branch, PartitionVars* a_parVars);
    void ChoosePartition(PartitionVars* a_parVars, int a_minFill);
    void LoadNodes(Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars);
    void InitParVars(PartitionVars* a_parVars, int a_maxRects, int a_minFill);
    void PickSeeds(PartitionVars* a_parVars);
    void Classify(int a_index, int a_group, PartitionVars* a_parVars);
    bool RemoveRect(Rect* a_rect, const DATATYPE& a_id, Node** a_root);                           //王
    bool RemoveRectRec(Rect* a_rect, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode);                 //王
    ListNode* AllocListNode();
    void FreeListNode(ListNode* a_listNode);
    bool Overlap(Rect* a_rectA, Rect* a_rectB);
    void ReInsert(Node* a_node, ListNode** a_listNode);
    bool Search(Node* a_node, Rect* a_rect, int& a_foundCount, bool a_resultCallback(DATATYPE a_data, void* a_context), void* a_context);
    void RemoveAllRec(Node* a_node);
    void Reset();
    void CountRec(Node* a_node, int& a_count);

    bool SaveRec(Node* a_node, RTFileStream& a_stream);
    bool LoadRec(Node* a_node, RTFileStream& a_stream);

    Node* m_root;                                    //根节点
};
//合并矩阵A和B，形成大矩阵
RTREE_TEMPLATE
typename RTREE_QUAL::Rect RTREE_QUAL::CombineRect(Rect* a_rectA, Rect* a_rectB)
{
    Rect newRect;

    for(int index = 0; index < NUMDIMS; ++index)
    {
        newRect.m_min[index] = Min(a_rectA->m_min[index], a_rectB->m_min[index]);
        newRect.m_max[index] = Max(a_rectA->m_max[index], a_rectB->m_max[index]);
    }

    return newRect;
}

// 添加一个分支到一个节点。 如有必要，拆分节点。
// 如果节点没有分裂，则返回 0。 旧节点更新。
// 如果节点分裂，则返回 1，将 *new_node 设置为新节点的地址。
// 旧节点更新，成为两个之一。
RTREE_TEMPLATE
bool RTREE_QUAL::AddBranch(Branch* a_branch, Node* a_node, Node** a_newNode)
{
    if(a_node->m_count < MAXNODES)  // Split won't be necessary 不需要拆分
    {
        a_node->m_branch[a_node->m_count] = *a_branch;
        ++a_node->m_count;

        return false;
    }
    else
    {
        ASSERT(a_newNode);

        SplitNode(a_node, a_branch, a_newNode);
        return true;
    }
}

// 拆分一个节点。
// 划分节点分支和两个节点之间的额外分支。
// 旧节点是新节点之一，并且创建了一个真正的新节点。
// 尝试多种选择分区的方法，使用最佳结果。
RTREE_TEMPLATE
void RTREE_QUAL::SplitNode(Node* a_node, Branch* a_branch, Node** a_newNode)
{
    // 可以在这里使用本地，但成员或外部更快，因为它被重用
    PartitionVars localVars;
    PartitionVars* parVars = &localVars;
    int level;   //记住当前拆分节点的级别

    //将所有分支加载到缓冲区中，初始化旧节点
    level = a_node->m_level;
    GetBranches(a_node, a_branch, parVars);

    // 查找分区
    ChoosePartition(parVars, MINNODES);

    // Put branches from buffer into 2 nodes according to chosen partition
    *a_newNode = AllocNode();
    (*a_newNode)->m_level = a_node->m_level = level;
    LoadNodes(a_node, *a_newNode, parVars);

}

//将所有分支加载到缓冲区中，初始化旧节点
RTREE_TEMPLATE
void RTREE_QUAL::GetBranches(Node* a_node, Branch* a_branch, PartitionVars* a_parVars)
{

    //加载分支缓冲区
    for(int index=0; index < MAXNODES; ++index)
    {
        a_parVars->m_branchBuf[index] = a_node->m_branch[index];
    }
    a_parVars->m_branchBuf[MAXNODES] = *a_branch;
    a_parVars->m_branchCount = MAXNODES + 1;

    //计算包含集合中所有内容的矩形
    a_parVars->m_coverSplit = a_parVars->m_branchBuf[0].m_rect;
    for(int index=1; index < MAXNODES+1; ++index)  //遍历合并矩阵
    {
        a_parVars->m_coverSplit = CombineRect(&a_parVars->m_coverSplit, &a_parVars->m_branchBuf[index].m_rect);
    }
    a_parVars->m_coverSplitArea = RectVolume(&a_parVars->m_coverSplit);  //计算体积  --->给定矩形的边界球体的精确体积

    InitNode(a_node);   //初始化节点
}

//计算矩形的n维体积
RTREE_TEMPLATE
ELEMTYPEREAL RTREE_QUAL::RectVolume(Rect* a_rect)
{
    ELEMTYPEREAL volume = (ELEMTYPEREAL)1;

    for(int index=0; index<NUMDIMS; ++index)
    {
        volume *= a_rect->m_max[index] - a_rect->m_min[index];
    }


    return volume;
}

RTREE_TEMPLATE
void RTREE_QUAL::InitNode(Node* a_node)
{
    a_node->m_count = 0;
}

//实例一个节点
RTREE_TEMPLATE
typename RTREE_QUAL::Node* RTREE_QUAL::AllocNode()
{
    Node* newNode;
    newNode=new Node;
    InitNode(newNode);
    return newNode;
}

RTREE_TEMPLATE
void RTREE_QUAL::FreeNode(Node* a_node)
{
    delete a_node;
}

RTREE_TEMPLATE
void RTREE_QUAL::InitRect(Rect* a_rect)
{
    for(int index = 0; index < NUMDIMS; ++index)
    {
        a_rect->m_min[index] = (ELEMTYPE)0;
        a_rect->m_max[index] = (ELEMTYPE)0;
    }
}

//找到包含节点分支中所有矩形的最小矩形。
RTREE_TEMPLATE
typename RTREE_QUAL::Rect RTREE_QUAL::NodeCover(Node* a_node)
{
    int firstTime =true;
    Rect rect;
    InitRect(&rect);

    for (int index = 0; index < a_node->m_count; ++index)
    {
        if (firstTime)
        {
            rect=a_node->m_branch[index].m_rect;
            firstTime= false;
        }
        else
        {
            rect=CombineRect(&rect,&(a_node->m_branch[index].m_rect));
        }
    }
    return rect;
}
// 断开依赖节点。
// 当计数改变时，调用者必须返回（或停止使用迭代索引）
RTREE_TEMPLATE
void RTREE_QUAL::DisconnectBranch(Node* a_node, int a_index)
{
    //通过与最后一个元素交换来删除元素以防止数组中的间隙
    a_node->m_branch[a_index] = a_node->m_branch[a_node->m_count - 1];

    --a_node->m_count;
}

// 选择一个分支。 选择需要最小增加的那个
// 容纳新矩形的区域。 这将导致
// 当前节点中覆盖矩形的最小总面积。
// 在平局的情况下，选择之前较小的那个，得到
// 搜索时的最佳分辨率
RTREE_TEMPLATE
int RTREE_QUAL::PickBranch(Rect* a_rect, Node* a_node)
{
    bool firstTime = true;
    ELEMTYPEREAL increase;
    ELEMTYPEREAL bestIncr = (ELEMTYPEREAL)-1;
    ELEMTYPEREAL area;
    ELEMTYPEREAL bestArea;
    int best;
    Rect tempRect;

    for(int index=0; index < a_node->m_count; ++index)
    {
        Rect* curRect = &a_node->m_branch[index].m_rect;
        area = CalcRectVolume(curRect);
        tempRect = CombineRect(a_rect, curRect);
        increase = CalcRectVolume(&tempRect) - area;
        if((increase < bestIncr) || firstTime)
        {
            best = index;
            bestArea = area;
            bestIncr = increase;
            firstTime = false;
        }
        else if((increase == bestIncr) && (area < bestArea))
        {
            best = index;
            bestArea = area;
            bestIncr = increase;
        }
    }
    return best;
}
#endif //WY_CUDA_RTREE_H
