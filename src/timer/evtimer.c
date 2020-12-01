/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 08:45:42
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-01 15:15:28
 */ 

#include <stdio.h>
#include <stdlib.h>
#include "evtimer.h"

/*最小堆 定时器值比较*/
static int32_t _EV_TIMER_Compare(const void *_v1, const void *_v2)
{
    const T_EV_TIMER *pt1 = _v1;
	const T_EV_TIMER *pt2 = _v2;

	if(pt1->u64Expire > pt2->u64Expire)
		return -1;
	else
		return 0;
}

/*返回最小堆根节点得值*/
static T_EV_TIMER *_EV_TIMER_Peek(void *_self)
{
	T_HEAP *ptHeap = (T_HEAP *)_self;
	return (T_EV_TIMER *)(ptHeap->tree == NULL ? NULL : ptHeap->tree[0]);
}
/*定时器最小堆索引记录，对删除定时器得值起到作用*/
static void _EV_TIMER_Index(void *_self, const uint32_t _index)
{
	T_EV_TIMER *ptEvTimer = (T_EV_TIMER *)_self;
	 //printf("<expire-%d index-%d-->%d\r\n",self->expire,self->index,_index);
	ptEvTimer->u64Index= _index;
}

/* 删除某个具体的timer ，这个timer要自己释放*/
static int32_t _EV_TIMER_Remove(void *_self, T_EV_TIMER *_pTimer)
{
	T_HEAP *ptHeap = (T_HEAP *)_self;
	void *vdtemp;
	int32_t ipos, lpos, rpos, mpos;

	if (0 == ptHeap->iSize) {
		return -1;
	}
	/* 最后一个成员顶替要被删除成员的位置*/
	printf("delete timer_index:%d\r\n",_pTimer->u64Index);
	ptHeap->tree[_pTimer->u64Index] = ptHeap->tree[ptHeap->iSize - 1];
	/* 然后改变其索引值*/
	ptHeap->pfIndex(ptHeap->tree[_pTimer->u64Index], _pTimer->u64Index);

	if (ptHeap->iSize - 1 > 0) {
		if ((vdtemp = (void **)realloc(ptHeap->tree, (ptHeap->iSize - 1) * sizeof(void *))) == NULL) {
	      	return -1;
	   	}
	   	else {
	    	ptHeap->tree = vdtemp;
	   	}

	   	ptHeap->iSize--;
	}
	else {
	   	free(ptHeap->tree);
	   	ptHeap->tree = NULL;
	   	ptHeap->iSize = 0;
	   	return 0;
	}

	ipos = _pTimer->u64Index;

	while (1) {
		lpos = HEAP_LEFT(ipos);
		rpos = HEAP_RIGHT(ipos);

		if (lpos < MiniHeap_Size(ptHeap) && ptHeap->pfCompare(ptHeap->tree[ipos], ptHeap->tree[lpos]) < 0) {
			mpos = lpos;
		}
		else {
			mpos = ipos;
		}

		if (rpos < MiniHeap_Size(ptHeap) && ptHeap->pfCompare(ptHeap->tree[mpos], ptHeap->tree[rpos]) < 0) {
			mpos = rpos;
		}

		if (mpos == ipos) {
			break;
		}
		else {
			vdtemp = ptHeap->tree[mpos];
			ptHeap->tree[mpos] = ptHeap->tree[ipos];
			ptHeap->tree[ipos] = vdtemp;
			
			if (ptHeap->pfIndex) {			
				ptHeap->pfIndex(ptHeap->tree[mpos], mpos);			
				ptHeap->pfIndex(ptHeap->tree[ipos], ipos);
			}
			
			ipos = mpos;
		}
	}

	return 0;
}


/*创建定时器控制*/
T_EV_TIMER_CTRL *EV_TIMER_Ctl_Create(void)
{
	T_EV_TIMER_CTRL *ptEvTimerCtrl = calloc(1, sizeof(T_EV_TIMER_CTRL));
	if(NULL == ptEvTimerCtrl)
		return NULL;

	ptEvTimerCtrl->pfEvTimerCtlDtor = MiniHeap_Destroy;
	ptEvTimerCtrl->pfEvTimerCtlPush = MiniHeap_Insert;
	ptEvTimerCtrl->pfEvTimerCtlPop  = MiniHeap_Extract;
	ptEvTimerCtrl->pfEvTimerCtlSize = MiniHeap_Size;
	ptEvTimerCtrl->pfEvTimerCtlPeek = _EV_TIMER_Peek;
	//ctl->match = ev_timer_match;
	ptEvTimerCtrl->pfEvTimerCtlRemove = _EV_TIMER_Remove;
	MiniHeap_Init(ptEvTimerCtrl, _EV_TIMER_Compare,NULL, _EV_TIMER_Index);	
	return ptEvTimerCtrl;


}

