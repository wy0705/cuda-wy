#include <iostream>
#include "Lump.h"
#include "master.h"
int main() {

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
/*
 * 合并矩阵A和B，形成大矩阵
 typename Rect CombineRect(Rect* a_rectA, Rect* a_rectB)
{
  Rect newRect;

  for(int index = 0; index < NUMDIMS; ++index)
  {
    newRect.m_min[index] = Min(a_rectA->m_min[index], a_rectB->m_min[index]);
    newRect.m_max[index] = Max(a_rectA->m_max[index], a_rectB->m_max[index]);
  }

  return newRect;
}*/
/*

///用于查找分割分区的变量
  struct PartitionVars
  {
    int m_partition[MAXNODES+1];   //划分
    int m_total;     //总量
    int m_minFill;    //最小填充
    int m_taken[MAXNODES+1];
    int m_count[2];   //分割两个组的size
    Rect m_cover[2];   //分割两个组的rect
    ELEMTYPEREAL m_area[2];

    Branch m_branchBuf[MAXNODES+1];   //当缓存用
    int m_branchCount;      //分支当前数量
    Rect m_coverSplit;   //存缓存当前的矩阵
    ELEMTYPEREAL m_coverSplitArea;    //大小
  };


// 添加一个分支到一个节点。 如有必要，拆分节点。
// 如果节点没有分裂，则返回 0。 旧节点更新。
// 如果节点分裂，则返回 1，将 *new_node 设置为新节点的地址。
// 旧节点更新，成为两个之一。
bool AddBranch(Branch* a_branch, Node* a_node, Node** a_newNode)
{
  if(a_node->m_count < MAXNODES)  // 数量小于阶数，不需要拆分
  {
    a_node->m_branch[a_node->m_count] = *a_branch; //插入该节点分支的尾部
    ++a_node->m_count;  //节点中分支数量++
    return false;
  }
  else     //超过结束，拆分节点
  {
    SplitNode(a_node, a_branch, a_newNode);
    return true;
  }

// 拆分一个节点。
// 划分节点分支和两个节点之间的额外分支。
// 旧节点是新节点之一，并且创建了一个真正的新节点。
// 尝试多种选择分区的方法，使用最佳结果。
void SplitNode(Node* a_node, Branch* a_branch, Node** a_newNode)
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

  ASSERT((a_node->m_count + (*a_newNode)->m_count) == parVars->m_total);
}

//将所有分支加载到缓冲区中，初始化旧节点
void GetBranches(Node* a_node, Branch* a_branch, PartitionVars* a_parVars)
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
  a_parVars->m_coverSplitArea = CalcRectVolume(&a_parVars->m_coverSplit);  //计算体积  --->给定矩形的边界球体的精确体积

  InitNode(a_node);   //初始化节点
}
}*/