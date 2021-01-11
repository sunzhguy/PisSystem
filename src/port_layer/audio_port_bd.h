/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-21 10:40:44
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-11 09:57:33
 */
#ifndef AUDIO_PORT_BROADCAST
#define AUDIO_PORT_BROADCAST
#include "../evio/evio.h"
#include "../net/evnet.h"
#include "../nanomsg/pair.h"
#include "../nanomsg/nn.h"
#include "../main.h"

#define BROADCAST_AUDIO_DATA_SIZE		(1024)
//广播音频
#define BROADCAST_AUDIO_LOCAL_PORT		(51151)
#define BROADCAST_AUDIO_REMOTE_PORT	    (51152)
#define BROADCAST_AUDIO_REMOTE_IP		"168.168.102.255"

#define BROADCAST_AUDIO_REMOTE_IP_HEX  (0xa8a866ff)


typedef struct _T_BDAUDIONANOMSG_EVFDS  T_BDAUDIONANOMSG_EVFDS;

struct _T_BDAUDIONANOMSG_EVFDS {
	int iSysFd;				/* sys fd*/
	int iNanoMsgFd;			/* nanomsg fd */
	T_EVENT_FD *ptEventFd;
	void (*pfEventCallBack)(T_EVENT_CTL *, T_BDAUDIONANOMSG_EVFDS *);
	void *pvArg;
};

typedef struct {
	T_EVENT_CTL  *ptEventCtl;
    T_EVENT_UDP  *ptEventUdp;
    T_BDAUDIONANOMSG_EVFDS tNanomsgEvFds;
	T_MAINSERVER *ptServer;
	pthread_mutex_t tThread_Mutex;
}T_BDAUDIO_NET_EVCTL;

void *BDAudio_Thread_Handle(void *_pvArg);

void  AudioPortBD_SendData(uint8_t* _pcBuf,uint32_t _u32DatLen);
#endif