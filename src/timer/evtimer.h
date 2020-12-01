/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 08:45:29
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-01 14:31:02
 */ 

#ifndef _MINI_TIMER_H_
#define _MINI_TIMER_H_
#include <stdint.h>
#include "../mini-heap/mini-heap.h"

typedef struct _EV_TIMER_T T_EV_TIMER;

typedef void (*PF_EVTIMER_CB)(void *, T_EV_TIMER *, void *);

struct _EV_TIMER_T {
	uint64_t u64Index;
	uint64_t u64Expire;
	PF_EVTIMER_CB pfEvTimerCb;
	void *pvArg;
};
typedef void 		(*PF_EVTIMER_CTL_MINIHP_DESTRY)(void *);
typedef int32_t 	(*PF_EVTIMER_CTL_MINIHP_PUSH)(void *, const void *);
typedef int32_t 	(*PF_EVTIMER_CTL_MINIHP_POP)(void *, void **);
typedef int32_t 	(*PF_EVTIMER_CTL_MINIHP_REMOVE)(void *, T_EV_TIMER *);
typedef T_EV_TIMER *(*PF_EVTIMER_CTL_MINIHP_PEEK)(void *);
typedef int32_t 	(*PF_EVTIMER_CTL_MINIHP_SIZE)(void *);
typedef T_EV_TIMER *(*PF_EVTIMER_CTL_MINIHP_MATCH)(void *, void *);

typedef struct _T_EV_TIMER_CTRL {
	T_HEAP    tHeapTimer;    //此变量传递强制转换作为最小堆存储管理来使用
	PF_EVTIMER_CTL_MINIHP_DESTRY pfEvTimerCtlDtor;
	PF_EVTIMER_CTL_MINIHP_PUSH 	 pfEvTimerCtlPush;
	PF_EVTIMER_CTL_MINIHP_POP    pfEvTimerCtlPop;
	PF_EVTIMER_CTL_MINIHP_REMOVE pfEvTimerCtlRemove;
	PF_EVTIMER_CTL_MINIHP_PEEK 	 pfEvTimerCtlPeek;
	PF_EVTIMER_CTL_MINIHP_SIZE 	 pfEvTimerCtlSize;
	//ev_timer_ctl_match_t match;
}T_EV_TIMER_CTRL;


T_EV_TIMER_CTRL *EV_TIMER_Ctl_Create(void);
#endif