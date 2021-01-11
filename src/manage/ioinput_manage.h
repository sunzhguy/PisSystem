/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-11 10:02:32
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-11 11:22:16
 */

#ifndef IOINPUT_MANAGE_H
#define IOINPUT_MANAGE_H

#include "../evio/evio.h"
#include "../net/evnet.h"
#include "../nanomsg/pair.h"
#include "../nanomsg/nn.h"
#include "../main.h"
typedef struct _T_IONANOMSG_EVFDS  T_IONANOMSG_EVFDS;

struct _T_IONANOMSG_EVFDS {
	int iSysFd;				/* sys fd*/
	int iNanoMsgFd;			/* nanomsg fd */
	T_EVENT_FD *ptEventFd;
	void (*pfEventCallBack)(T_EVENT_CTL *, T_IONANOMSG_EVFDS *);
	void *pvArg;
};
typedef struct {
	T_EVENT_CTL  *ptEventCtl;
    T_IONANOMSG_EVFDS tNanomsgEvFds;
	T_MAINSERVER *ptServer;
}T_IOINPUT_EVCTL;

void *IOInput_Thread_Handle(void *_pvArg);
#endif