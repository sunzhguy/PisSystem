/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-16 11:42:41
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-07-22 10:46:40
 */ 

#ifndef _EVIO_H_
#define _EVIO_H_

#include <stdint.h>
#include <sys/epoll.h>
#include "../timer/evtimer.h"

typedef struct ev_fd   ev_fd_t;
typedef struct ev_ctl  ev_ctl_t;
typedef enum   ev_type ev_type_t;
typedef void (*ev_cb_t)(ev_ctl_t *evctl, ev_fd_t *evfd, int32_t fd, ev_type_t evtype, void *arg);
enum ev_type {
	EV_READ,
	EV_WRITE,
	EV_ERROR
};

struct ev_fd {
	int32_t fd;					/* handler */
	uint32_t ev;				/* 事件*/
	char is_del;
	ev_cb_t cb;					/* 用户回调函数*/
	void *arg;					/* 用户参数 */
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
typedef void (*ev_start_timer_t)(ev_ctl_t *, ev_timer_t *, uint64_t, ev_timer_cb_t, void *);
typedef void (*ev_stop_timer_t)(ev_ctl_t *, ev_timer_t *);
#endif

struct ev_ctl {
	int32_t epfd;
	struct epoll_event evlist[32];
	char looping;
	uint32_t deferred_cnt;
	uint32_t deferred_cap;
	ev_fd_t **deferred_to_close;
	ev_timer_ctl_t *timer_ctl;  //定时器超时控制
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

uint64_t get_cur_time_ms(void);

ev_ctl_t *evctl_create(void);

/*事件轮询*/
void ev_loop_start(ev_ctl_t *evctl);
/*初始化定时器，设置定时器超时时间 以及定时器回调 参数等*/
void ev_init_timer(ev_timer_t *timer, uint64_t timeout_ms, ev_timer_cb_t cb, void *arg);

/*添加一个定时器 事件控制*/
void ev_start_timer(ev_ctl_t *evctl,ev_timer_t *timer);


/*停止事件轮询定时器，即删除定时器*/
void ev_stop_timer(ev_ctl_t *evctl, ev_timer_t *timer);

void evctl_free(ev_ctl_t *evctl);
/*添加fd 进evfd 并加入epool 事件 中*/
ev_fd_t *ev_fd_add(ev_ctl_t *evctl, int32_t fd, ev_cb_t cb, void *arg);

/*移除ev_fd 进一步移除 ev_fd_t*/
void ev_fd_del(ev_ctl_t *evctl, ev_fd_t *evfd);

void ev_watch_write(ev_ctl_t *evctl, ev_fd_t *evfd);
void ev_unwatch_write(ev_ctl_t *evctl, ev_fd_t *evfd);

void ev_watch_read(ev_ctl_t *evctl, ev_fd_t *evfd);
void ev_unwatch_read(ev_ctl_t *evctl, ev_fd_t *evfd);

#endif