/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-17 09:53:04
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-07-22 10:36:50
 */ 
#ifndef _EVNET_H_
#define _EVNET_H_

#include <stdint.h>
#include "../evio/evio.h"


#define MAXUDPCNT           1           //UDP通信个数
#define MAX_TCPSERCNT       1           //TCP SERVER 通信个数
#define MAX_TCPLOCLIENT     1           //TCP LOCAL 客户端个数
#define MAXCLIENTS			4          //作为server listen client 个数

#define  MAX_BUF_CNT       (MAXUDPCNT+MAX_TCPSERCNT+MAX_TCPLOCLIENT+MAXCLIENTS)

#define EV_BUFFER_SIZE 		4096
#define EV_HALFBUF_SIZE		2048

struct ev_tcp;
struct ev_udp;
typedef enum ev_tcp_event {
    EV_TCP_ACCEPT,
	EV_TCP_CONNECTED,
	EV_TCP_ACCEPT_OVERFLOW,
    EV_TCP_DATA,
    EV_TCP_ERROR,
    EV_TCP_CLOSE
}ev_tcp_event_t;

typedef void (*ev_tcp_cb_t)(ev_ctl_t *, struct ev_tcp *, ev_tcp_event_t , void *);
//typedef void (*ev_udp_cb_t)(ev_ctl_t *, struct ev_udp *, ev_udp_event_t , void *);

typedef enum ev_tcp_type {
    EV_TCP_LISTEN,
	EV_TCP_LOGGIN,
    EV_TCP_CONNECT,
    EV_TCP_NORMAL,
}ev_tcp_type_t;

typedef struct ev_buffer {
	char r[EV_BUFFER_SIZE];
	int32_t rlen;
	int32_t roff;
	char w[EV_BUFFER_SIZE];
	int32_t wlen;
	int32_t woff;
	int32_t using;
}ev_buffer_t;

typedef struct ev_tcp {
	ev_tcp_type_t type;
	int32_t fd;
	ev_buffer_t *buffer;
	ev_fd_t *evfd;
	ev_tcp_cb_t cb;
	void *arg;
}ev_tcp_t;



typedef void (*ev_udp_cb_t)(ev_ctl_t *, struct ev_udp *, void *);

typedef struct ev_udp{
	int32_t fd;
	ev_buffer_t *buffer;
	ev_fd_t *evfd;
	ev_udp_cb_t cb;//接收数据回调控制
	void *arg;
}ev_udp_t;

#if 0
ev_tcp_t *ev_tcp_client_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_tcp_cb_t cb, void *arg);
ev_tcp_t *ev_tcp_serv_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_tcp_cb_t cb, void *arg);
void ev_tcp_close(ev_ctl_t *evctl, ev_tcp_t *evtcp);
void ev_tcp_set(ev_ctl_t *evctl, ev_tcp_t *evtcp, ev_tcp_cb_t cb, void *arg);
int ev_tcp_msg(ev_ctl_t *evctl, ev_tcp_t *evtcp, const char *data, int32_t size);
#endif

ev_udp_t* udp_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_udp_cb_t cb,void *arg);

#endif

