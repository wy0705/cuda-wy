//
// Created by wy on 2021/10/20.
//
#include "master.h"


#ifndef WY_CUDA_LUMP_H
#define WY_CUDA_LUMP_H
class Module
{
private:
    enum {
        NUMS=16,      //每个Block有几个Cake
        SIZE=200,     //每一块Cake的size
        MODULESIZE=10,  //每个Module有10个Block
    };

    struct Cake_y{
        int loc;
        string datas[SIZE];
    };
    struct Cake_x{
        int loc;    //当前x轴的位置
        Cake_y* m_x[SIZE];

    };
    struct Cake{
        bool IsFull() {return (m_count==40000);}

        int m_count; //有多少数据
        int n;  //标注在一个Block中的第几个Cake
        Cake_x* m_x;   //先确定x轴的值
        string m_tablename; //记录每个cake中的表名
        string m_naturename; //记录每个cake中的属性名
    };
    struct ListCake
    {
        ListCake* m_next;
        Cake* m_cake;
    };
    struct Matrix{           //矩阵---坐标
        long min[2];
        long max[2];
    };
    struct Block{
        bool IsNull() {return (m_count==0);}
        bool IsNotNull() {return (m_count>0);}

        int m_count; //已用数量
        int n; //标注是Module中的第几个Block
        Cake m_cake[NUMS];  //指向每个Cake的指针
    };


public:
    Block* m_bilokRoot;//第一个Block

    Module();
    virtual ~Module();


    class Iterator
    {
    private:

        enum { MAX_Cache = 10 };

        struct StackElement
        {
            Block* m_block;
            int m_cakeIndex;
        };

    public:

        Iterator()                                    { Init(); }

        ~Iterator()                                   { }

        bool IsNull()                                 { return (m_tos <= 0); }

        bool IsNotNull()                              { return (m_tos > 0); }

        /// 访问当前数据元素。调用方必须首先确保迭代器不为NULL。
        Cake& operator*()
        {
            StackElement& curTos = m_stack[m_tos - 1];
            return curTos.m_block->m_cake[curTos.m_cakeIndex];
        }

        /// 寻找下一个数据元素
        bool operator++()                             { return FindNextData(); }


    private:

        void Init()                                   { m_tos = 0; }

        /// Find the next data element in the tree (For internal use only)
        bool FindNextData()
        {
            for(;;)
            {
                if(m_tos <= 0)
                {
                    return false;
                }
                StackElement curTos = Pop(); //复制堆栈顶部，因为在使用时它可能会更改
                if (curTos.m_cakeIndex+1<curTos.m_block->m_count)//把当前栈顶的block中的cake全部浏览一遍
                {
                    //可以浏览该cake（待写）
                    Push(curTos.m_block,curTos.m_cakeIndex+1);
                    return true;
                }
            }
        }

        ///将节点和分支推送到迭代堆栈上（仅供内部使用）
        void Push(Block* a_block, int a_cakeIndex)
        {
            if(m_tos<MAX_Cache)
            {
                m_stack[m_tos].m_block = a_block;
                m_stack[m_tos].m_cakeIndex = a_cakeIndex;
                ++m_tos;
            } else
            {
                cout<<"栈溢出"<<endl;
            }
        }

        ///从迭代堆栈中弹出元素（仅供内部使用）
        StackElement& Pop()
        {
            if (m_tos>0)
            {
                --m_tos;
                return m_stack[m_tos];
            } else
            {
                cout<<"栈中无元素"<<endl;
            }
        }

        //遍历当前所有Block，找到Block剩余cake空间大于size的
        bool findBlockForSize(int size,Block* block){
            for (int i = m_tos; i > 0 ; --i) {
                if ((16-m_stack[i].m_block->m_count)>size)//说明该block有剩余空间
                {
                    StackElement& curTos = m_stack[i];
                    block=curTos.m_block;
                    return true;
                }
            }
            //该block没有剩余空间
            return false;
        }

        //遍历当前所有block中的cake，寻找有没有表名Tablename
        bool findBlockForTablename(string Tablename,Block* block){
            for (int i = m_tos; i > 0; --i) {
                StackElement curTos = m_stack[i];
                for (int j = curTos.m_block->m_count; j > 0; --j) {
                    if (curTos.m_block->m_cake->m_tablename==Tablename)
                    {
                        block=curTos.m_block;
                        return true;
                    }
                }
            }
            //整个模块中没有Tablename
            return false;
        }

        StackElement m_stack[MAX_Cache];              ///迭代
        int m_tos;                                    ///栈顶索引

        friend Module; // Allow hiding of non-public functions while allowing manipulation by logical owner
    };


    bool AddBlock(Block* block);
    Matrix* insertTable(string Tablename,Nature* natures);    //建表
    int deleteTable(Matrix* location,string Tablename);         //删表
    //int updateTablename(string oldTablename,string newTablename);      //改表名
    Matrix* updateForNature(Matrix* location,string Tablename,Nature* natures);    //改列
    bool AddCake(Block* m_block,Cake* m_cake);   //向block中添加cake
    bool InitCake(Cake* m_cake);
    bool InitBlock(Block* m_block);
    bool TableToListCake(Block* m_block,string m_ablename,ListCake* m_listcake);  //将block中的tablename读到listcake中
    bool ComparedNature(ListCake* m_listcake,Nature* m_nature,Nature* differnature);  //true为添加属性，false为删除属性，differnature放的是差异属性
    int ToMatrix(ListCake* m_listCake,Matrix* m_matrix);//计算位置

};
#endif //WY_CUDA_LUMP_H
