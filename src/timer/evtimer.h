/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 08:45:29
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-07-21 15:53:17
 */ 

#ifndef _MINI_TIMER_H_
#define _MINI_TIMER_H_
#include <stdint.h>
#include "../mini-heap/mini-heap.h"

typedef struct _ev_timer_t ev_timer_t;
typedef Heap  timer_miniHeap;
typedef void (*ev_timer_cb_t)(void *, ev_timer_t *, void *);

struct _ev_timer_t {
	uint64_t index;
	uint64_t expire;
	ev_timer_cb_t cb;
	void *arg;
};
typedef void (*ev_timer_ctl_dtor_t)(void *);
typedef int32_t (*ev_timer_ctl_push_t)(void *, const void *);
typedef int32_t (*ev_timer_ctl_pop_t)(void *, void **);
typedef int32_t (*ev_timer_ctl_remove_t)(void *, ev_timer_t *);
typedef ev_timer_t *(*ev_timer_ctl_peek_t)(void *);
typedef int32_t (*ev_timer_ctl_size_t)(void *);
typedef ev_timer_t *(*ev_timer_ctl_match_t)(void *, void *);

typedef struct ev_timer_ctl_t {
	timer_miniHeap    timer_list;    //此变量传递强制转换作为最小堆存储管理来使用
	ev_timer_ctl_dtor_t dtor;
	ev_timer_ctl_push_t push;
	ev_timer_ctl_pop_t pop;
	ev_timer_ctl_remove_t remove;
	ev_timer_ctl_peek_t peek;
	ev_timer_ctl_size_t size;
	//ev_timer_ctl_match_t match;
}ev_timer_ctl_t;


ev_timer_ctl_t *ev_timer_ctl_create(void);
#endif