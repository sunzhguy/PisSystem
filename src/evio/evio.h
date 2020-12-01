/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 11:42:41
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-01 15:20:53
 */ 

#ifndef _EVIO_H_
#define _EVIO_H_

#include <stdint.h>
#include <sys/epoll.h>
#include "../timer/evtimer.h"

typedef struct _T_EVENT_FD   T_EVENT_FD;
typedef struct _T_EVENT_CTL  T_EVENT_CTL;
typedef enum   _E_EV_TYPE    E_EV_TYPE;
typedef void (*PF_EVENT_CALLBACK)(T_EVENT_CTL *_ptEventCtrl, T_EVENT_FD *_ptEventFd, int32_t _fd, E_EV_TYPE _e_type, void *_arg);
enum _E_EV_TYPE {
	EV_READ,
	EV_WRITE,
	EV_ERROR
};

struct _T_EVENT_FD {
	int32_t iFd;					/* handler */
	uint32_t iEvents;				/* 事件*/
	char 	bIsDel;
	PF_EVENT_CALLBACK pfEventCallBack;					/* 用户回调函数*/
	void *pvArg;					/* 用户参数 */
};

#if 0
/*epool 事件轮询*/
typedef void (*ev_loop_start_t)(ev_ctl_t *);
/*添加fd 进epool 事件 中*/
typedef ev_fd_t *(*ev_fd_add_t)(ev_ctl_t *, int32_t, ev_cb_t, void *);
/*移除ev_fd 进一步移除 ev_fd_t*/
typedef void (*ev_fd_del_t)(ev_ctl_t *, ev_fd_t *);
typedef void (*ev_fd_set_t)(ev_ctl_t *, ev_fd_t *, ev_cb_t, void *);
typedef void (*ev_watch_write_t)(ev_ctl_t *, ev_fd_t *);
typedef void (*ev_watch_read_t)(ev_ctl_t *, ev_fd_t *);
typedef void (*ev_unwatch_write_t)(ev_ctl_t *, ev_fd_t *);
typedef void (*ev_unwatch_read_t)(ev_ctl_t *, ev_fd_t *);
typedef void (*ev_start_timer_t)(ev_ctl_t *, T_EV_TIMER *, uint64_t, ev_timer_cb_t, void *);
typedef void (*ev_stop_timer_t)(ev_ctl_t *, T_EV_TIMER *);
#endif

struct _T_EVENT_CTL {
	int32_t            iEpolFd;
	struct epoll_event atEpollEventList[32];
	char               bLooping;
	uint32_t           u32DeferredCnt;
	uint32_t           u32DeferredCap;
	T_EVENT_FD 	     **pttEventFDeferred_to_close;
	T_EV_TIMER_CTRL   *ptEventTimerCtrl;  //定时器超时控制
	#if 0
	ev_loop_start_t start;
	ev_fd_add_t add;
	ev_fd_del_t del;
	ev_fd_set_t set;
	ev_watch_write_t watch_write;
	ev_watch_read_t watch_read;
	ev_unwatch_write_t unwatch_write;
	ev_unwatch_read_t unwatch_read;
    
	ev_start_timer_t timer_start;
	ev_stop_timer_t timer_stop;
	#endif
};

uint64_t EVIO_Get_CurentTime_ms(void);

T_EVENT_CTL *EVIO_EventCtl_Create(void);

/*事件轮询*/
void EVIO_EventCtlLoop_Start(T_EVENT_CTL *_ptEventCtl);
/*初始化定时器，设置定时器超时时间 以及定时器回调 参数等*/
void EVIO_EventTimer_Init(T_EV_TIMER *_ptEventTimer, uint64_t _u64timeout_ms, PF_EVTIMER_CB _pfEvTimerCB, void *_pvArg);

/*添加一个定时器 事件控制*/
void EVIO_EventTimer_Start(T_EVENT_CTL *_ptEventCtl,T_EV_TIMER *_ptEventTimer);


/*停止事件轮询定时器，即删除定时器*/
void EVIO_EventTimer_Stop(T_EVENT_CTL *_ptEventCtl,T_EV_TIMER *_ptEventTimer);

void EVIO_EventCtl_Free(T_EVENT_CTL *_ptEventCtl);
/*添加fd 进evfd 并加入epool 事件 中*/
T_EVENT_FD *EVIO_EventFd_Add(T_EVENT_CTL *_ptEventCtl, int32_t _ifd, PF_EVENT_CALLBACK _pfCB, void *_pvArg);

/*移除ev_fd 进一步移除 ev_fd_t*/
void EVIO_EventFd_Del(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd);
void EVIO_Event_Watch_Write(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd);
void EVIO_Event_UnWatch_Write(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd);
void EVIO_Event_Watch_Read(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd);
void EVIO_Event_UnWatch_Read(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd);

#endif