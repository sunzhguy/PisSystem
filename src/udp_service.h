/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 10:22:12
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 16:26:31
 */ 
/*网络服务线程*/
#ifndef UDP_SERVICE
#define UDP_SERVICE

#include "evio/evio.h"
#include "net/evnet.h"
#include "nanomsg/pair.h"
#include "nanomsg/nn.h"
#include "main.h"


typedef struct _T_UDP_NANOMSG_EVFDS  T_UDP_NANOMSG_EVFDS;

struct _T_UDP_NANOMSG_EVFDS {
	int iSysFd;				/* sys fd*/
	int iNanoMsgFd;			/* nanomsg fd */
	T_EVENT_FD *ptEventFd;
	void (*pfEventCallBack)(T_EVENT_CTL *, T_UDP_NANOMSG_EVFDS *);
	void *pvArg;
};

typedef struct _T_UDP_NET_EVCTL{
	char cInitFlag;
	T_EVENT_CTL  *ptEventCtl;
    T_EVENT_CTL  *ptUDPEventCtl;
    T_EVENT_UDP *ptEventUdp;
	T_UDP_NANOMSG_EVFDS tNanoMsgUDPNet;
	T_MAINSERVER *ptServer;
}T_UDP_NET_EVCTL;

void *UDP_SERVICE_Thread_Handle(void *_pvArg);
void  UDP_SERVICE_SendData(uint8_t* _pcBuf,uint32_t _u32DatLen);
#endif