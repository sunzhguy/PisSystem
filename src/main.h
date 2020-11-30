/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 09:04:27
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-11-30 11:57:21
 */ 

#ifndef _MAIN_H
#define _MAIN_H


struct servfds {
	int s;			/* sys fd*/
	int n;			/* nanomsg fd */
	ev_fd_t *evfd;
	void (*cb)(ev_ctl_t *, struct servfds *);
	void *arg;
};

struct servloop {
	ev_ctl_t *evctl;
	struct servfds  sfds_udp;//主服务循环 ---》UDP 与main loop 之间的通信
};

struct server {
    ev_timer_t ev_timer;//添加一个定时器
    struct servloop sloop;//主服务循环
	int32_t start_num; //启动server 服务线程数
	pthread_mutex_t start_lock;
    pthread_cond_t start_cond;
};

#endif