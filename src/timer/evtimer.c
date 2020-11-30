/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 08:45:42
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-07-17 16:17:49
 */ 

#include <stdio.h>
#include <stdlib.h>
#include "evtimer.h"

/*最小堆 定时器值比较*/
int32_t ev_timer_compare(const void *v1, const void *v2)
{
    const ev_timer_t *t1 = v1;
	const ev_timer_t *t2 = v2;

	if(t1->expire > t2->expire)
		return -1;
	else
		return 0;
}

/*返回最小堆根节点得值*/
ev_timer_t *ev_timer_ctl_peek(void *_self)
{
	timer_miniHeap *self = _self;
	return (ev_timer_t *)(self->tree == NULL ? NULL : self->tree[0]);
}
/*定时器最小堆索引记录，对删除定时器得值起到作用*/
void ev_timer_index(void *_self, const uint32_t _index)
{
	ev_timer_t *self = _self;
	 //printf("<expire-%d index-%d-->%d\r\n",self->expire,self->index,_index);
	self->index = _index;
}

/* 删除某个具体的timer ，这个timer要自己释放*/
int32_t ev_timer_ctl_remove(void *_self, ev_timer_t *timer)
{
	timer_miniHeap *self = _self;
	void *temp;
	int32_t ipos, lpos, rpos, mpos;

	if (0 == self->size) {
		return -1;
	}
	/* 最后一个成员顶替要被删除成员的位置*/
	printf("delete timer_index:%d\r\n",timer->index);
	self->tree[timer->index] = self->tree[self->size - 1];
	/* 然后改变其索引值*/
	self->index(self->tree[timer->index], timer->index);

	if (self->size - 1 > 0) {
		if ((temp = (void **)realloc(self->tree, (self->size - 1) * sizeof(void *))) == NULL) {
	      	return -1;
	   	}
	   	else {
	    	self->tree = temp;
	   	}

	   	self->size--;
	}
	else {
	   	free(self->tree);
	   	self->tree = NULL;
	   	self->size = 0;
	   	return 0;
	}

	ipos = timer->index;

	while (1) {
		lpos = heap_left(ipos);
		rpos = heap_right(ipos);

		if (lpos < heap_size(self) && self->compare(self->tree[ipos], self->tree[lpos]) < 0) {
			mpos = lpos;
		}
		else {
			mpos = ipos;
		}

		if (rpos < heap_size(self) && self->compare(self->tree[mpos], self->tree[rpos]) < 0) {
			mpos = rpos;
		}

		if (mpos == ipos) {
			break;
		}
		else {
			temp = self->tree[mpos];
			self->tree[mpos] = self->tree[ipos];
			self->tree[ipos] = temp;
			
			if (self->index) {			
				self->index(self->tree[mpos], mpos);			
				self->index(self->tree[ipos], ipos);
			}
			
			ipos = mpos;
		}
	}

	return 0;
}


/*创建定时器控制*/
ev_timer_ctl_t *ev_timer_ctl_create(void)
{

	ev_timer_ctl_t *ctl = calloc(1, sizeof(ev_timer_ctl_t));
	if(NULL == ctl)
		goto err;

	ctl->dtor = heap_destroy;
	ctl->push = heap_insert;
	ctl->pop = heap_extract;
	ctl->size = heap_size;
	ctl->peek = ev_timer_ctl_peek;
	//ctl->match = ev_timer_match;
	ctl->remove = ev_timer_ctl_remove;
	heap_init(ctl, ev_timer_compare,NULL, ev_timer_index);	
	return ctl;
	
err:
	return NULL;

}

