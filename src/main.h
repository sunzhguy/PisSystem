/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 09:04:27
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-01 14:58:45
 */ 

#ifndef _MAIN_H
#define _MAIN_H


struct servfds {
	int s;			/* sys fd*/
	int n;			/* nanomsg fd */
	T_EVENT_FD *evfd;
	void (*cb)(T_EVENT_CTL *, struct servfds *);
	void *arg;
};

struct servloop {
	T_EVENT_CTL *evctl;
	struct servfds  sfds_udp;//主服务循环 ---》UDP 与main loop 之间的通信
};

struct server {
    T_EV_TIMER ev_timer;//添加一个定时器
    struct servloop sloop;//主服务循环
	int32_t start_num; //启动server 服务线程数
	pthread_mutex_t start_lock;
    pthread_cond_t start_cond;
};

#endif