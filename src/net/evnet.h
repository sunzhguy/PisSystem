/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-17 09:53:04
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-02 11:39:28
 */ 
#ifndef _EVNET_H_
#define _EVNET_H_

#include <stdint.h>
#include "../evio/evio.h"


#define MAXUDPCNT           4          //UDP通信个数

#define MAX_TCPSERCNT       1           //TCP SERVER 通信个数
#define MAX_TCPLOCLIENT     1           //TCP LOCAL 客户端个数
#define MAXCLIENTS			1          //作为server listen client 个数

#define  MAX_BUF_CNT       (MAXUDPCNT+MAX_TCPSERCNT+MAX_TCPLOCLIENT+MAXCLIENTS)

#define EV_BUFFER_SIZE 		4096
#define EV_HALFBUF_SIZE		2048

typedef struct _T_EVENT_TCP T_EVENT_TCP;
typedef struct _T_EVENT_UDP T_EVENT_UDP;
typedef enum  {
    EV_TCP_ACCEPT,
	EV_TCP_CONNECTED,
	EV_TCP_ACCEPT_OVERFLOW,
    EV_TCP_DATA,
    EV_TCP_ERROR,
    EV_TCP_CLOSE
}E_EVENT_TCP;

typedef void (*PF_EVENT_TCP_CALLBACK)(T_EVENT_CTL *, T_EVENT_TCP *, E_EVENT_TCP , void *);

typedef enum  {
    EV_TCP_LISTEN,
	EV_TCP_LOGGIN,
    EV_TCP_CONNECT,
    EV_TCP_NORMAL,
}E_EVENT_TCP_TYPE;

typedef struct  {
	char acReadBuffer[EV_BUFFER_SIZE];
	int32_t iReadLen;
	int32_t iReadOffset;
	char acWriteBuffer[EV_BUFFER_SIZE];
	int32_t u32Writelen;
	int32_t u32WriteOffset;
	int32_t bUsing;
}T_EVNET_BUFFER;

struct T_EVENT_TCP {
	E_EVENT_TCP_TYPE eType;
	int32_t iFd;
	T_EVNET_BUFFER *ptEvNetBuffer;
	T_EVENT_FD *ptEventFd;
	PF_EVENT_TCP_CALLBACK pfCallBack;
	void *pvArg;
};



typedef void (*PF_EVENT_UDP_CALLBACK)(T_EVENT_CTL *, T_EVENT_UDP *, void *);

struct _T_EVENT_UDP{
	int32_t iSocketFd;
	T_EVNET_BUFFER *ptEvNetBuffer;
	T_EVENT_FD *ptEventFd;
	PF_EVENT_UDP_CALLBACK pfEventCallBack;//接收数据回调控制
	void *pvArg;
};

#if 0
ev_tcp_t *ev_tcp_client_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_tcp_cb_t cb, void *arg);
ev_tcp_t *ev_tcp_serv_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_tcp_cb_t cb, void *arg);
void ev_tcp_close(ev_ctl_t *evctl, ev_tcp_t *evtcp);
void ev_tcp_set(ev_ctl_t *evctl, ev_tcp_t *evtcp, ev_tcp_cb_t cb, void *arg);
int ev_tcp_msg(ev_ctl_t *evctl, ev_tcp_t *evtcp, const char *data, int32_t size);
#endif

T_EVENT_UDP* EV_NET_EventUDP_CreateAndStart(T_EVENT_CTL *_ptEventCtl, char *_pcIpaddr, uint16_t _Port, PF_EVENT_UDP_CALLBACK _pfEventCallBack,void *_pvArg);

#endif

