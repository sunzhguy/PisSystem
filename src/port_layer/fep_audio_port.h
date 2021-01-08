/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-07 16:22:31
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-08 15:35:11
 */

#ifndef FEP_AUDIO_PORT_H
#define FEP_AUDIO_PORT_H
#include "../evio/evio.h"
#include "../net/evnet.h"
#include "../nanomsg/pair.h"
#include "../nanomsg/nn.h"
#include "../main.h"


#define FEP_AUDIO_LOCAL_PORT		(53152)
#define FEP_AUDIO_REMOTE_PORT	    (53151)
#define FEP_AUDIO_REMOTE_IP		  "168.168.102.255"

#define NANOMSG_FEPSYNAUDIO_BDAUDIO  0x01         /*Nanomsg*/


typedef struct _T_NANOMSG_EVFDS  T_NANOMSG_EVFDS;

struct _T_NANOMSG_EVFDS {
	int iSysFd;				/* sys fd*/
	int iNanoMsgFd;			/* nanomsg fd */
	T_EVENT_FD *ptEventFd;
	void (*pfEventCallBack)(T_EVENT_CTL *, T_NANOMSG_EVFDS *);
	void *pvArg;
};

typedef struct {
	T_EVENT_CTL  *ptEventCtl;
    T_EVENT_UDP  *ptEventUdp;
	T_NANOMSG_EVFDS tNanoMsg;
	T_MAINSERVER *ptServer;
}T_FEP_AUDIO_NET_EVCTL;

void *FEP_Audio_SyncThread_Handle(void *_pvArg);


#endif