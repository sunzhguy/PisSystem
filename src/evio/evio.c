/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 11:42:51
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-11-30 10:36:29
 */ 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "evio.h"
 

uint64_t get_cur_time_ms(void)
{
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#define TIMEMOUT_TM  1000


static int _ev_calc_timeout(ev_ctl_t *evctl )
{
    ev_timer_ctl_t *ctl = evctl->timer_ctl;//事件轮询定时器控制
    if (!ctl->size(ctl))
        return TIMEMOUT_TM;

    uint64_t now =get_cur_time_ms();
    //printf("now===%d\r\n",now);
    ev_timer_t *timer = ctl->peek(ctl);/*最小堆root 节点查看*/
    //printf("peek:%p\r\n",timer);
    //printf("expire===%d\r\n",timer->expire);
	/* 已经有任务超时 */
    if (timer->expire <= now)
        return 0;

    uint64_t period = timer->expire - now;
    if (period >= TIMEMOUT_TM)
        return TIMEMOUT_TM;

	/* 挂起到最近一个任务超时时间 */
    return period;
}

static void _ev_timer_run(ev_ctl_t *evctl)
{

    ev_timer_ctl_t *ctl = evctl->timer_ctl;//事件轮询定时器控制
    uint64_t now = get_cur_time_ms();
    uint64_t max_times = ctl->size(ctl);
    uint64_t cur_times = 0;

    while (ctl->size(ctl) && cur_times++ < max_times) {
        ev_timer_t *timer = ctl->peek(ctl);
        if (timer->expire <= now) {
            ctl->pop(ctl, NULL);
			if(timer->cb){
                timer->cb(evctl, timer, timer->arg);
            }
        } else {
            break;
        }
    }
}

/*事件轮询*/
void ev_loop_start(ev_ctl_t *evctl)
{
    _ev_timer_run(evctl);
    int timeout = _ev_calc_timeout(evctl);
    int ev_cnt = epoll_wait(evctl->epfd, evctl->evlist, 32, timeout);
	if (-1 == ev_cnt) {
		return;
	}
    evctl->looping = 1;

	int i;
    for (i = 0; i < ev_cnt; ++i) {
        ev_fd_t *evfd = evctl->evlist[i].data.ptr;
        uint32_t events = evctl->evlist[i].events;

        if (evfd->is_del)
            continue;
        if ((events & EPOLLIN) && (evfd->ev & EPOLLIN))
            evfd->cb(evctl, evfd, evfd->fd, EV_READ, evfd->arg);
        if (!evfd->is_del && (events & EPOLLOUT) && (evfd->ev & EPOLLOUT))
            evfd->cb(evctl, evfd, evfd->fd, EV_WRITE, evfd->arg);
        if (!evfd->is_del && events & (EPOLLHUP | EPOLLERR))
            evfd->cb(evctl, evfd, evfd->fd, EV_ERROR, evfd->arg);
    }

    evctl->looping = 0;

	for (i = 0; i < evctl->deferred_cnt; ++i)
        free(evctl->deferred_to_close[i]);

	evctl->deferred_cnt = 0;
}

/*设置事件回调参数  以及传值*/
static void ev_fd_set(ev_ctl_t *evctl, ev_fd_t *evfd, ev_cb_t cb, void *arg)
{
    evfd->cb = cb;
    evfd->arg = arg;
}
/*添加fd 进evfd 并加入epool 事件 中*/
 ev_fd_t *ev_fd_add(ev_ctl_t *evctl, int32_t fd, ev_cb_t cb, void *arg)
{
    ev_fd_t *evfd = calloc(1, sizeof(ev_fd_t));
	if (NULL == evfd)
		return NULL;

	evfd->is_del = 0;
    evfd->fd = fd;
    evfd->cb = cb;
    evfd->arg = arg;
    struct epoll_event add_event;
    add_event.events = 0;
    add_event.data.ptr = evfd;
    if (-1 == epoll_ctl(evctl->epfd, EPOLL_CTL_ADD, fd, &add_event)) {
        free(evfd);
        return NULL;
    }

    return evfd;
}

/*移除ev_fd 进一步移除 ev_fd_t*/
 void ev_fd_del(ev_ctl_t *evctl, ev_fd_t *evfd)
{
    epoll_ctl(evctl->epfd, EPOLL_CTL_DEL, evfd->fd, NULL);
    if (evctl->looping) {
        evfd->is_del = 1;
        if (evctl->deferred_cnt == evctl->deferred_cap)
        {
        evctl->deferred_to_close = realloc(evctl->deferred_to_close,++evctl->deferred_cap * sizeof(ev_fd_t));
        }
        evctl->deferred_to_close[evctl->deferred_cnt++] = evfd;
        return;
    }
    free(evfd);
	evfd = NULL;
}

static void _ev_watch_events(ev_ctl_t *evctl, ev_fd_t *evfd)
{
    struct epoll_event mod_event;
    mod_event.events = evfd->ev;
    mod_event.data.ptr = evfd;
    epoll_ctl(evctl->epfd, EPOLL_CTL_MOD, evfd->fd, &mod_event);
}

void ev_watch_write(ev_ctl_t *evctl, ev_fd_t *evfd)
{
    evfd->ev |= EPOLLOUT;
    _ev_watch_events(evctl, evfd);
}

void ev_unwatch_write(ev_ctl_t *evctl, ev_fd_t *evfd)
{
    evfd->ev &= ~EPOLLOUT;
    _ev_watch_events(evctl, evfd);
}

void ev_watch_read(ev_ctl_t *evctl, ev_fd_t *evfd)
{
    evfd->ev |= EPOLLIN;
    _ev_watch_events(evctl, evfd);
}

void ev_unwatch_read(ev_ctl_t *evctl, ev_fd_t *evfd)
{
    evfd->ev &= ~EPOLLIN;
    _ev_watch_events(evctl, evfd);
}

/*添加一个定时器 事件控制*/
void ev_init_timer(ev_timer_t *timer, uint64_t timeout_ms, ev_timer_cb_t cb, void *arg)
{
	
    uint64_t expire = get_cur_time_ms() + timeout_ms;
    timer->expire = expire;
    timer->index = -1;
    //printf("expire----%d\r\n",expire);
    timer->cb = cb;
    timer->arg = arg;
	
}

void ev_start_timer(ev_ctl_t *evctl,ev_timer_t *timer)
{
   ev_timer_ctl_t *ctl = evctl->timer_ctl;
    ctl->push(ctl, timer);
}

/*停止事件轮询定时器，即删除定时器*/
void ev_stop_timer(ev_ctl_t *evctl, ev_timer_t *timer)
{
	ev_timer_ctl_t *ctl = evctl->timer_ctl;
    ctl->remove(ctl, timer);
}
void evctl_free(ev_ctl_t *evctl)
{
	close(evctl->epfd);
	evctl->timer_ctl->dtor(evctl->timer_ctl);
	free(evctl->timer_ctl);
	if (evctl->deferred_to_close)
		free(evctl->deferred_to_close);
	free(evctl);
}

ev_ctl_t *evctl_create(void)
{
	ev_ctl_t *evctl = calloc(1, sizeof(ev_ctl_t));
	if (NULL == evctl)
		goto err1;

	evctl->epfd = epoll_create1(0);
	if (-1 == evctl->epfd)
		goto err2;

	evctl->timer_ctl = ev_timer_ctl_create();
	if (NULL == evctl->timer_ctl)
		goto err3;

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

	return evctl;

err3:
	close(evctl->epfd);
err2:
	free(evctl);
err1:
	return NULL;
}