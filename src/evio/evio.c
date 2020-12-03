/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 11:42:51
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-03 14:28:23
 */ 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "evio.h"

#define TIMEMOUT_TM  1000

uint64_t EVIO_Get_CurentTime_ms(void)
{
    struct timespec tts;
    clock_gettime(CLOCK_MONOTONIC, &tts);
    return tts.tv_sec * 1000 + tts.tv_nsec / 1000000;
}




static int _EVIO_Calc_Timeout(T_EVENT_CTL *_ptEventCtl)
{
    T_EV_TIMER_CTRL *ptEvTimerCtl = _ptEventCtl->ptEventTimerCtrl;//事件轮询定时器控制
    if (!ptEvTimerCtl->pfEvTimerCtlSize(ptEvTimerCtl))
        return TIMEMOUT_TM;

    uint64_t now =EVIO_Get_CurentTime_ms();
    T_EV_TIMER *ptEvenTimer = ptEvTimerCtl->pfEvTimerCtlPeek(ptEvTimerCtl);/*最小堆root 节点查看*/
 
	/* 已经有任务超时 */
    if (ptEvenTimer->u64Expire <= now)
        return 0;

    uint64_t period = ptEvenTimer->u64Expire - now;
    if (period >= TIMEMOUT_TM)
        return TIMEMOUT_TM;

	/* 挂起到最近一个任务超时时间 */
    return period;
}

static void _EVIO_EvTimer_Run(T_EVENT_CTL *_ptEventCtl)
{

    T_EV_TIMER_CTRL *ptEvTimerCtl = _ptEventCtl->ptEventTimerCtrl;//事件轮询定时器控制
    uint64_t now = EVIO_Get_CurentTime_ms();
    uint64_t max_times = ptEvTimerCtl->pfEvTimerCtlSize(ptEvTimerCtl);
    uint64_t cur_times = 0;

    while (ptEvTimerCtl->pfEvTimerCtlSize(ptEvTimerCtl) && cur_times++ < max_times) {
        T_EV_TIMER *ptEvenTimer = ptEvTimerCtl->pfEvTimerCtlPeek(ptEvTimerCtl);
        if (ptEvenTimer->u64Expire <= now) {
            ptEvTimerCtl->pfEvTimerCtlPop(ptEvTimerCtl, NULL);
			if(ptEvenTimer->pfEvTimerCb){
                ptEvenTimer->pfEvTimerCb(_ptEventCtl, ptEvenTimer, ptEvenTimer->pvArg);
            }
        } else {
            break;
        }
    }
}

/*事件轮询*/
void EVIO_EventCtlLoop_Start(T_EVENT_CTL *_ptEventCtl)
{
    _EVIO_EvTimer_Run(_ptEventCtl);
    int timeout = _EVIO_Calc_Timeout(_ptEventCtl);
    int ev_cnt = epoll_wait(_ptEventCtl->iEpolFd, _ptEventCtl->atEpollEventList, 32, timeout);
	if (-1 == ev_cnt) {
		return;
	}
    _ptEventCtl->bLooping = 1;

	int i;
    for (i = 0; i < ev_cnt; ++i) {
        T_EVENT_FD *ptEventFd = _ptEventCtl->atEpollEventList[i].data.ptr;
        uint32_t events = _ptEventCtl->atEpollEventList[i].events;

        if (ptEventFd->bIsDel)
            continue;
        if ((events & EPOLLIN) && (ptEventFd->iEvents & EPOLLIN))
            ptEventFd->pfEventCallBack(_ptEventCtl, ptEventFd, ptEventFd->iFd, E_EV_READ, ptEventFd->pvArg);
        if (!ptEventFd->bIsDel && (events & EPOLLOUT) && (ptEventFd->iEvents & EPOLLOUT))
            ptEventFd->pfEventCallBack(_ptEventCtl, ptEventFd, ptEventFd->iFd, E_EV_WRITE, ptEventFd->pvArg);
        if (!ptEventFd->bIsDel && events & (EPOLLHUP | EPOLLERR))
            ptEventFd->pfEventCallBack(_ptEventCtl, ptEventFd, ptEventFd->iFd, E_EV_ERROR, ptEventFd->pvArg);
    }

    _ptEventCtl->bLooping = 0;

	for (i = 0; i < _ptEventCtl->u32DeferredCnt; ++i)
        free(_ptEventCtl->pttEventFDeferred_to_close[i]);

	_ptEventCtl->u32DeferredCnt = 0;
}

#if 0
/*设置事件回调参数  以及传值*/
static void _EVIO_EventFd_Set(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, PF_EVENT_CALLBACK _pfCB, void *_pvArg)
{
    _ptEventFd->pfEventCallBack = _pfCB;
    _ptEventFd->pvArg = _pvArg;
}
#endif

/*添加fd 进evfd 并加入epool 事件 中*/
 T_EVENT_FD *EVIO_EventFd_Add(T_EVENT_CTL *_ptEventCtl, int32_t _ifd, PF_EVENT_CALLBACK _pfCB, void *_pvArg)
{
    T_EVENT_FD *ptEventFd = calloc(1, sizeof(T_EVENT_FD));
	if (NULL == ptEventFd)
		return NULL;

	ptEventFd->bIsDel = 0;
    ptEventFd->iFd = _ifd;
    ptEventFd->pfEventCallBack = _pfCB;
    ptEventFd->pvArg = _pvArg;
    struct epoll_event tEpollEvent;
    tEpollEvent.events = 0;
    tEpollEvent.data.ptr = ptEventFd;
    if (-1 == epoll_ctl(_ptEventCtl->iEpolFd, EPOLL_CTL_ADD, _ifd, &tEpollEvent)) {
        free(ptEventFd);
        return NULL;
    }

    return ptEventFd;
}

/*移除ev_fd 进一步移除 ev_fd_t*/
 void EVIO_EventFd_Del(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd)
{
    epoll_ctl(_ptEventCtl->iEpolFd, EPOLL_CTL_DEL, _ptEventFd->iFd, NULL);
    if (_ptEventCtl->bLooping) {
        _ptEventFd->bIsDel = 1;
        if (_ptEventCtl->u32DeferredCnt == _ptEventCtl->u32DeferredCap)
        {
         _ptEventCtl->pttEventFDeferred_to_close = realloc(_ptEventCtl->pttEventFDeferred_to_close,++_ptEventCtl->u32DeferredCap * sizeof(T_EVENT_FD));
        }
        _ptEventCtl->pttEventFDeferred_to_close[_ptEventCtl->u32DeferredCnt++] = _ptEventFd;
        return;
    }
    free(_ptEventFd);
	_ptEventFd = NULL;
}

static void _EVIO_Watch_Events(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd)
{
    struct epoll_event tEpollEventMod;
    tEpollEventMod.events = _ptEventFd->iEvents;
    tEpollEventMod.data.ptr = _ptEventFd;
    epoll_ctl(_ptEventCtl->iEpolFd, EPOLL_CTL_MOD, _ptEventFd->iFd, &tEpollEventMod);
}

void EVIO_Event_Watch_Write(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd)
{
    _ptEventFd->iEvents |= EPOLLOUT;
    _EVIO_Watch_Events(_ptEventCtl, _ptEventFd);
}

void EVIO_Event_UnWatch_Write(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd)
{
    _ptEventFd->iEvents &= ~EPOLLOUT;
    _EVIO_Watch_Events(_ptEventCtl, _ptEventFd);
}

void EVIO_Event_Watch_Read(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd)
{
    _ptEventFd->iEvents |= EPOLLIN;
    _EVIO_Watch_Events(_ptEventCtl, _ptEventFd);
}

void EVIO_Event_UnWatch_Read(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd)
{
    _ptEventFd->iEvents &= ~EPOLLIN;
    _EVIO_Watch_Events(_ptEventCtl, _ptEventFd);
}

/*添加一个定时器 事件控制*/
void EVIO_EventTimer_Init(T_EV_TIMER *timer, uint64_t _u64timeout_ms, PF_EVTIMER_CB _pfEvTimerCB, void *_pvArg)
{
    uint64_t expire = EVIO_Get_CurentTime_ms() + _u64timeout_ms;
    timer->u64Expire = expire;
    timer->u64Index = -1;
    timer->pfEvTimerCb = _pfEvTimerCB;
    timer->pvArg = _pvArg;
	
}

void EVIO_EventTimer_Start(T_EVENT_CTL *_ptEventCtl,T_EV_TIMER *_ptEventTimer)
{
   
   T_EV_TIMER_CTRL *ptEventTimerCtl = _ptEventCtl->ptEventTimerCtrl;
   //printf("Enter...EVCTL%p.--TIMEREVCTL>%p---->timer:%p\n",_ptEventCtl,ptEventTimerCtl,_ptEventTimer);
   ptEventTimerCtl->pfEvTimerCtlPush(ptEventTimerCtl, _ptEventTimer);
    
}

/*停止事件轮询定时器，即删除定时器*/
void EVIO_EventTimer_Stop(T_EVENT_CTL *_ptEventCtl,T_EV_TIMER *_ptEventTimer)
{
	T_EV_TIMER_CTRL *ptEventTimerCtl = _ptEventCtl->ptEventTimerCtrl;
    ptEventTimerCtl->pfEvTimerCtlRemove(ptEventTimerCtl, _ptEventTimer);
}
void EVIO_EventCtl_Free(T_EVENT_CTL *_ptEventCtl)
{
	close(_ptEventCtl->iEpolFd);
	_ptEventCtl->ptEventTimerCtrl->pfEvTimerCtlDtor(_ptEventCtl->ptEventTimerCtrl);
	free(_ptEventCtl->ptEventTimerCtrl);
	if (_ptEventCtl->pttEventFDeferred_to_close)
		free(_ptEventCtl->pttEventFDeferred_to_close);
	free(_ptEventCtl);
}

T_EVENT_CTL *EVIO_EventCtl_Create(void)
{
	T_EVENT_CTL *ptEventCtl = calloc(1, sizeof(T_EVENT_CTL));
	if (NULL == ptEventCtl)
        {
            printf("%s-%d\n",__func__,__LINE__);
            return NULL;   
        }

	ptEventCtl->iEpolFd = epoll_create1(0);
	if (-1 == ptEventCtl->iEpolFd)
        {
            printf("%s-%d\n",__func__,__LINE__);
            free(ptEventCtl);
            return NULL;
        }

	ptEventCtl->ptEventTimerCtrl = EV_TIMER_Ctl_Create();
   // printf("++++Creator+++++++ptEventCtl->ptEventTimerCtrl:%p\n",ptEventCtl->ptEventTimerCtrl);
	if (NULL == ptEventCtl->ptEventTimerCtrl)
	    {
            printf("%s-%d\n",__func__,__LINE__);
            close(ptEventCtl->iEpolFd);
            free(ptEventCtl);
            return NULL;
        }

    #if 0
	evctl->start = ev_loop_start;
	evctl->add = ev_fd_add;
	evctl->del = ev_fd_del;
	evctl->set = ev_fd_set;
	evctl->watch_write = ev_watch_write;
	evctl->watch_read = ev_watch_read;
	evctl->unwatch_write = ev_unwatch_write;
	evctl->unwatch_read = ev_unwatch_read;
	evctl->timer_start = ev_start_timer;
	evctl->timer_stop = ev_stop_timer;
    #endif

	return ptEventCtl;
}