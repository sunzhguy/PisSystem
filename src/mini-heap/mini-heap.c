/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 10:06:14
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-07-16 17:56:56
 */ 
#include <stdlib.h>
#include <string.h>

#include "mini-heap.h"
/*定义heap执行中需要使用的私有宏*/
//#define heap_parent(npos) ((int)(((npos)-1)/2))  /*npos的父结点*/
//#define heap_left(npos) (((npos)*2)+1)           /*npos的左兄弟结点*/
//#define heap_right(npos) (((npos)*2)+2)          /*npos的右兄弟结点*/


int  heap_size(void *self)
{
    Heap *heap = (Heap*)self;
    return heap->size;
}
/*heap_init 堆的初始化*/
void heap_init(void *self,heap_compare_t compare,heap_destroy_t destroy,heap_index_t index)
{
    Heap *heap = (Heap*)self;
    /*只需要将size设为0，destroy成员指向destroy，将tree指针设置为NULL*/
    heap->size = 0;
    heap->compare = compare;
    heap->destroy = destroy;
    heap->index = index;
    heap->tree = NULL;

    return ;
}
/*heap_destroy  销毁堆*/
void heap_destroy(void *self)
{
    int i;
     Heap *heap = (Heap*)self;
    /*移除堆中所有的结点*/
    if(heap->destroy != NULL)
    {
        for(i=0; i<heap_size(heap);i++)
        {
            /*调用用户自定义函数释放动态分配的数据*/
            heap->destroy(heap->tree[i]);
        }
    }
    /*释放为堆分配的空间*/
    free(heap->tree);

    memset(heap,0,sizeof(Heap));
    return;
}

/*heap_insert  向堆中插入结点*/
int heap_insert(void *self,const void *data)
{
    void *temp;
    int  ipos;
    int  ppos;
     Heap *heap = (Heap*)self;
    int size = (heap_size(heap)+1)*sizeof(void *);
    /*为结点分配空间*/
    if((temp = (void **)realloc(heap->tree,size)) == NULL)
    {
        return -1;
    }
    else
    {
        heap->tree = temp;
    }
    /*将结点插入到堆的最末端*/
    heap->tree[heap_size(heap)] = (void *)data;
    /*将新结点向上推动，恢复堆的排序特点*/
    ipos = heap_size(heap);    /*堆结点数的数值*/

    if (heap->index) {			
		/* 设置新节点索引值*/	
		heap->index(heap->tree[ipos], ipos);
	}

    ppos = heap_parent(ipos);  /*ipos位置结点的父结点*/
    /*如果堆不为空，且新成员(优先级)小于父节点==>最小堆模式*/
    while(ipos>0 && heap->compare(heap->tree[ppos],heap->tree[ipos])<0)
    {
        /* 临时存放父节点成员 */
        temp = heap->tree[ppos];
        /* 新成员存放到父节点位置 */
        heap->tree[ppos] = heap->tree[ipos];
        /* 父节点成员被放置到之前新成员所在的位置 */
        heap->tree[ipos] = temp;

	    if (heap->index) {			
			/* 修改新节点索引值*/ 	
			heap->index(heap->tree[ppos], ppos);			
			/* 修改父节点索引值*/ 	
			heap->index(heap->tree[ipos], ipos);
		}
        /* 改变新节点成员在指针数组中的下标为之前其父节点的下标值 */
        ipos = ppos;
        ppos = heap_parent(ipos);
    }

    /*堆插入与排序完成，调整堆的结点数量值*/
    heap->size++;

    return 0;
}

/*heap_extract 释放堆顶部的结点*/
int heap_extract(void *self,void **data)
{
    void *save,*temp;
    int  ipos,lpos,rpos,mpos;
    Heap *heap = (Heap*)self;
    /*不允许从空的堆中释放结点*/
    if(heap_size(heap) == 0)
        return -1;

    /*释放堆顶部的结点*/
    /*首先将data指向将要释放结点的数据*//* 取堆中首元素 */	
	if(NULL != data)
    *data = heap->tree[0];

    /*将save指向未位结点*/
    save = heap->tree[heap_size(heap)-1];

    if(heap_size(heap)-1 > 0)
    {   /*为堆分配一个稍小一点的空间*/
        if((temp = (void **)realloc(heap->tree,(heap_size(heap)-1)*sizeof(void *)))==NULL)
        {
            return -1;
        }
        else
        {
            heap->tree = temp;
        }
        /*调整堆的大小*/
        heap->size--;
    }
    else
    {   /*只有一个结点，释放并重新管理堆，并返回*/
        free(heap->tree);
        heap->tree = NULL;
        heap->size = 0;
        return 0;
    }
    /*将末位结点拷贝到根结点中*/
    heap->tree[0] = save;

    /*重新调整树的结构*/
    ipos = 0;                /*顶元素*/
    if (heap->index) {			
		heap->index(heap->tree[0], ipos);			
	}
    lpos = heap_left(ipos);  /*左子结点*/
    rpos = heap_right(ipos); /*右子结点*/

    /*父结点与两个子结点比较、交换，直到不再需要交换为止，或者结点到达一个叶子位置*/
    while(1)
    {
        /*选择子结点与当前结点进行交换*/
        lpos = heap_left(ipos);
        rpos = heap_right(ipos);
        /*父结点与左子结点位置不正确,左子结点大于其父结点*/
        if(lpos < heap_size(heap) && heap->compare(heap->tree[ipos],heap->tree[lpos])<0)
        {
            mpos = lpos;  /* 最小堆模式时，mpos值为当前父节点和左孩子中的优先级高(即值较小者)的下标 */
        }
        else
        {
            mpos = ipos;
        }

        if(rpos < heap_size(heap) && heap->compare(heap->tree[mpos],heap->tree[rpos])<0)
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
            temp = heap->tree[mpos];
            heap->tree[mpos] = heap->tree[ipos];
            heap->tree[ipos] = temp;

	        if (heap->index) {			
				heap->index(heap->tree[mpos], mpos);			
				heap->index(heap->tree[ipos], ipos);
			}
            /*下移一层，以继续执行堆排序*/
            ipos = mpos;
        }
    }
    return 0;
}