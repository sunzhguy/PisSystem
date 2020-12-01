/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 10:06:14
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-01 14:33:47
 */ 
#include <stdlib.h>
#include <string.h>

#include "mini-heap.h"
/*定义heap执行中需要使用的私有宏*/
//#define heap_parent(npos) ((int)(((npos)-1)/2))  /*npos的父结点*/
//#define heap_left(npos) (((npos)*2)+1)           /*npos的左兄弟结点*/
//#define heap_right(npos) (((npos)*2)+2)          /*npos的右兄弟结点*/


int  MiniHeap_Size(void *self)
{
    T_HEAP *ptHeap = (T_HEAP*)self;
    return ptHeap->iSize;
}
/*heap_init 堆的初始化*/
void MiniHeap_Init(void *self,PF_HEAP_COMPARE compare,PF_HEAP_DESTROY destroy,PF_HEAP_INDEX index)
{
    T_HEAP *ptHeap = (T_HEAP*)self;
    /*只需要将size设为0，destroy成员指向destroy，将tree指针设置为NULL*/
    ptHeap->iSize = 0;
    ptHeap->pfCompare = compare;
    ptHeap->pfDestory = destroy;
    ptHeap->pfIndex = index;
    ptHeap->tree = NULL;
    return ;
}
/*heap_destroy  销毁堆*/
void MiniHeap_Destroy(void *self)
{
    int i;
     T_HEAP *ptHeap = (T_HEAP*)self;
    /*移除堆中所有的结点*/
    if(ptHeap->pfDestory != NULL)
    {
        for(i=0; i<MiniHeap_Size(ptHeap);i++)
        {
            /*调用用户自定义函数释放动态分配的数据*/
            ptHeap->pfDestory(ptHeap->tree[i]);
        }
    }
    /*释放为堆分配的空间*/
    free(ptHeap->tree);

    memset(ptHeap,0,sizeof(T_HEAP));
    return;
}

/*heap_insert  向堆中插入结点*/
int MiniHeap_Insert(void *self,const void *data)
{
    void *vdtemp;
    int  ipos;
    int  ppos;
    T_HEAP *ptHeap = (T_HEAP*)self;
    int size = (MiniHeap_Size(ptHeap)+1)*sizeof(void *);
    /*为结点分配空间*/
    if((vdtemp = (void **)realloc(ptHeap->tree,size)) == NULL)
    {
        return -1;
    }
    else
    {
        ptHeap->tree = vdtemp;
    }
    /*将结点插入到堆的最末端*/
    ptHeap->tree[MiniHeap_Size(ptHeap)] = (void *)data;
    /*将新结点向上推动，恢复堆的排序特点*/
    ipos = MiniHeap_Size(ptHeap);    /*堆结点数的数值*/

    if (ptHeap->pfIndex) {			
		/* 设置新节点索引值*/	
		ptHeap->pfIndex(ptHeap->tree[ipos], ipos);
	}

    ppos = HEAP_PARENT(ipos);  /*ipos位置结点的父结点*/
    /*如果堆不为空，且新成员(优先级)小于父节点==>最小堆模式*/
    while(ipos>0 && ptHeap->pfCompare(ptHeap->tree[ppos],ptHeap->tree[ipos])<0)
    {
        /* 临时存放父节点成员 */
        vdtemp = ptHeap->tree[ppos];
        /* 新成员存放到父节点位置 */
        ptHeap->tree[ppos] = ptHeap->tree[ipos];
        /* 父节点成员被放置到之前新成员所在的位置 */
        ptHeap->tree[ipos] = vdtemp;

	    if (ptHeap->pfIndex) {			
			/* 修改新节点索引值*/ 	
			ptHeap->pfIndex(ptHeap->tree[ppos], ppos);			
			/* 修改父节点索引值*/ 	
			ptHeap->pfIndex(ptHeap->tree[ipos], ipos);
		}
        /* 改变新节点成员在指针数组中的下标为之前其父节点的下标值 */
        ipos = ppos;
        ppos = HEAP_PARENT(ipos);
    }

    /*堆插入与排序完成，调整堆的结点数量值*/
    ptHeap->iSize++;

    return 0;
}

/*heap_extract 释放堆顶部的结点*/
int MiniHeap_Extract(void *self,void **data)
{
    void *vdsave,*vdtemp;
    int  ipos,lpos,rpos,mpos;
    T_HEAP *ptHeap = (T_HEAP*)self;
    /*不允许从空的堆中释放结点*/
    if(MiniHeap_Size(ptHeap) == 0)
        return -1;

    /*释放堆顶部的结点*/
    /*首先将data指向将要释放结点的数据*//* 取堆中首元素 */	
	if(NULL != data)
    *data = ptHeap->tree[0];

    /*将save指向未位结点*/
    vdsave = ptHeap->tree[MiniHeap_Size(ptHeap)-1];

    if(MiniHeap_Size(ptHeap)-1 > 0)
    {   /*为堆分配一个稍小一点的空间*/
        if((vdtemp = (void **)realloc(ptHeap->tree,(MiniHeap_Size(ptHeap)-1)*sizeof(void *)))==NULL)
        {
            return -1;
        }
        else
        {
            ptHeap->tree = vdtemp;
        }
        /*调整堆的大小*/
        ptHeap->iSize--;
    }
    else
    {   /*只有一个结点，释放并重新管理堆，并返回*/
        free(ptHeap->tree);
        ptHeap->tree = NULL;
        ptHeap->iSize = 0;
        return 0;
    }
    /*将末位结点拷贝到根结点中*/
    ptHeap->tree[0] = vdsave;

    /*重新调整树的结构*/
    ipos = 0;                /*顶元素*/
    if (ptHeap->pfIndex) {			
		ptHeap->pfIndex(ptHeap->tree[0], ipos);			
	}
    lpos = HEAP_LEFT(ipos);  /*左子结点*/
    rpos = HEAP_RIGHT(ipos); /*右子结点*/

    /*父结点与两个子结点比较、交换，直到不再需要交换为止，或者结点到达一个叶子位置*/
    while(1)
    {
        /*选择子结点与当前结点进行交换*/
        lpos = HEAP_LEFT(ipos);
        rpos = HEAP_RIGHT(ipos);
        /*父结点与左子结点位置不正确,左子结点大于其父结点*/
        if(lpos < MiniHeap_Size(ptHeap) && ptHeap->pfCompare(ptHeap->tree[ipos],ptHeap->tree[lpos])<0)
        {
            mpos = lpos;  /* 最小堆模式时，mpos值为当前父节点和左孩子中的优先级高(即值较小者)的下标 */
        }
        else
        {
            mpos = ipos;
        }

        if(rpos < MiniHeap_Size(ptHeap) && ptHeap->pfCompare(ptHeap->tree[mpos],ptHeap->tree[rpos])<0)
        {
            /* 之前优先级高的节点和右孩子进行比较 */
            mpos = rpos;
        }

        /*当mpos和ipos相等时，堆特性已经被修复,结束循环*/
        if(mpos == ipos)
        {
            	/* 父节点的优先级要比两个孩子的优先级高(即值更小)时退出 */
            break;
        }
        else
        {
            /*交换当前结点与被选中的结点的内容*/
            vdtemp = ptHeap->tree[mpos];
            ptHeap->tree[mpos] = ptHeap->tree[ipos];
            ptHeap->tree[ipos] = vdtemp;

	        if (ptHeap->pfIndex) {			
				ptHeap->pfIndex(ptHeap->tree[mpos], mpos);			
				ptHeap->pfIndex(ptHeap->tree[ipos], ipos);
			}
            /*下移一层，以继续执行堆排序*/
            ipos = mpos;
        }
    }
    return 0;
}