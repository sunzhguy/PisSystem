/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 08:40:25
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-03 16:54:25
 */ 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "net/evnet.h"
#include "udp_service.h"
#include "evio/evio.h"
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
	T_EVENT_CTL *ptEventCtl;
	T_UDP_NANOMSG_EVFDS tNanoMsgUDPNet;
	T_MAINSERVER *ptServer;
}T_UDP_NET_EVCTL;


 void _UDP_SERVICE_UdpRecive_EventCallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_UDP * _ptEventUDP , void *_pvArg)
 {
     //int beg =0;
	 uint32_t dat_len =0;
     T_EVNET_BUFFER *ptEvNetBuffer = _ptEventUDP->ptEvNetBuffer;
	 T_UDP_NET_EVCTL *ptUdpNetEventCtl = (T_UDP_NET_EVCTL *) _pvArg;
     //printf("UDP  recive....%s\r\n",ptEvNetBuffer->acReadBuffer);
	 dat_len = ptEvNetBuffer->iReadOffset;
     ptEvNetBuffer->iReadOffset = 0x00;
	#if 1
	{
		uint8_t *dat = nn_allocmsg(dat_len+1, 0);
		//printf("dat===%p,%p\n",dat,ptUdpNetEventCtl);
		if (NULL != dat) {
			memcpy(dat, ptEvNetBuffer->acReadBuffer, dat_len);
			int ret = nn_send(ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
			//printf("ret====%d\n",ret);
			
		}
	}
	#endif

	#if 0
		ev_buffer_t *b = evtcp->buffer;
		uint16_t beg = 0;
		uint16_t end = 0;
		uint16_t len = 0;

		while (beg < b->roff) {
			char *end_pos = (char *)memchr(b->r + beg, '\n', b->roff - beg);
			if (!end_pos)
				break;
			end = end_pos - b->r;
			len = end - beg + 1;
			int ret = cb(evctl, evtcp, arg, b->r + beg, len);
			//if (-1 == ret)
				//return;

			beg = end + 1;
		}

		uint16_t left = b->roff - beg;
		if (beg != 0 && left > 0) {
			memmove(b->r, b->r + beg, left);
		}

		b->roff = left;
	#endif
 }

void *_UDP_SERVICE_ThreadBroadCastInit(void *_pvArg)
{
     T_EVENT_CTL * ptEventCtl = EVIO_EventCtl_Create();
	 T_MAINSERVER *ptMainServer = ((T_UDP_NET_EVCTL *) _pvArg)->ptServer;
    if(ptEventCtl == NULL)
	{
		 zlog_error(ptMainServer->ptZlogCategory,"ThreadBroadCastInit ptEventCtl Create error\n");
		 abort();
	}
     T_EVENT_UDP *ptEventUdp = EV_NET_EventUDP_CreateAndStart(ptEventCtl,"168.168.102.255",5555,_UDP_SERVICE_UdpRecive_EventCallBack,_pvArg);
	 if(ptEventUdp == NULL)
     {
          zlog_error(ptMainServer->ptZlogCategory,"Event UDP create failed\r\n");
		  EVIO_EventCtl_Free(ptEventCtl);
          abort();
     }
     while(1)
     {
        EVIO_EventCtlLoop_Start(ptEventCtl);
     }
    EVIO_EventFd_Del(ptEventCtl,ptEventUdp->ptEventFd);
	EVIO_EventCtl_Free(ptEventCtl);
	return NULL;  
}

void _UDP_SERVICE_HandleUDPMainNsg(T_EVENT_CTL *_ptEventCtl, T_UDP_NANOMSG_EVFDS *_ptUdpNanomsgEvFds)
{
	T_UDP_NET_EVCTL *ptUdpNetEventCtl = _ptUdpNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer     = ptUdpNetEventCtl->ptServer;
	//zlog_debug(ptMainServer->ptZlogCategory,"++++++++++++++%s:%d\n",__func__,__LINE__);
	#if 1
	uint8_t *dat = NULL;
	uint32_t bytes = nn_recv(_ptUdpNanomsgEvFds->iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) {
			zlog_debug(ptMainServer->ptZlogCategory,"Main->UDP Nanomsg+++++++++++++++%s\n",dat);
			nn_freemsg(dat);
	}
	#endif
}

static void _UDP_SERVICE_UDPMainNsg_CallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_UDP_NANOMSG_EVFDS *ptUdpNanoMsgEvFds = (T_UDP_NANOMSG_EVFDS*)_pvArg;
	T_UDP_NET_EVCTL *ptUdpNetEventCtl 	   = ptUdpNanoMsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer          = ptUdpNetEventCtl->ptServer;

    switch (_eType) {
	    case E_EV_READ:
	        ptUdpNanoMsgEvFds->pfEventCallBack(_ptEventCtl, ptUdpNanoMsgEvFds);
	        break;
		case E_EV_WRITE:
			zlog_warn(ptMainServer->ptZlogCategory,"Unexpected write event");
			break;
		case E_EV_ERROR:
			zlog_error(ptMainServer->ptZlogCategory, "Unexpected error event");
			break;
	    default:
			zlog_warn(ptMainServer->ptZlogCategory, "Unexpected event");
	        break;
    }
}

static int32_t _UDP_SERVICE_UdpEventCtlInit(T_UDP_NET_EVCTL *_ptUdpNetEventCtl)
{
	T_MAINSERVER    *ptMainServer     = _ptUdpNetEventCtl->ptServer;
	size_t size = sizeof(size_t);
    _ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == _ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_socket failed\n");
		return -1;
	}
	if (-1 == nn_connect(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd, "inproc://udp<->main"))
	{
	  zlog_error(ptMainServer->ptZlogCategory,"++++nn_connect failed\n");
      nn_close(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd);
	  return -1;
	}
	zlog_debug(ptMainServer->ptZlogCategory,"--------------------------%d\n",_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd);
	
	if (-1 == nn_getsockopt(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd, NN_SOL_SOCKET, NN_RCVFD,\
							 (char *)&_ptUdpNetEventCtl->tNanoMsgUDPNet.iSysFd, &size))
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_getsockopt failed\n");
		nn_close(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd);
		return -1;	 
	}


	_ptUdpNetEventCtl->ptEventCtl = EVIO_EventCtl_Create();
	
	if (NULL == _ptUdpNetEventCtl->ptEventCtl)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++EVIO_EventCtl_Create failed\n");
		close(_ptUdpNetEventCtl->tNanoMsgUDPNet.iSysFd);
		nn_close(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd);
		return -1;
	}
    _ptUdpNetEventCtl->tNanoMsgUDPNet.pfEventCallBack = _UDP_SERVICE_HandleUDPMainNsg;
    _ptUdpNetEventCtl->tNanoMsgUDPNet.pvArg = _ptUdpNetEventCtl;
    _ptUdpNetEventCtl->tNanoMsgUDPNet.ptEventFd = EVIO_EventFd_Add(_ptUdpNetEventCtl->ptEventCtl, \
													 _ptUdpNetEventCtl->tNanoMsgUDPNet.iSysFd,\
													 _UDP_SERVICE_UDPMainNsg_CallBack, 
													 &_ptUdpNetEventCtl->tNanoMsgUDPNet);//add the event fd 
	if (NULL == _ptUdpNetEventCtl->tNanoMsgUDPNet.ptEventFd)
	{
		EVIO_EventCtl_Free(_ptUdpNetEventCtl->ptEventCtl);
		close(_ptUdpNetEventCtl->tNanoMsgUDPNet.iSysFd);
		nn_close(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd);
		zlog_error(ptMainServer->ptZlogCategory,"tNanoMsgUDPNet.ptEventFd failed\n");
		return -1;
	}
	EVIO_Event_Watch_Read(_ptUdpNetEventCtl->ptEventCtl, _ptUdpNetEventCtl->tNanoMsgUDPNet.ptEventFd);
	return 0;
	
}

void *UDP_SERVICE_Thread_Handle(void *_pvArg)
{
	T_UDP_NET_EVCTL tUDPNetEventCtl;
     pthread_t tPthread_udpbrodcast;
	 T_MAINSERVER *ptMainServer = (T_MAINSERVER *) _pvArg;
	 tUDPNetEventCtl.ptServer = ptMainServer;
    if(0 != pthread_create(&tPthread_udpbrodcast, NULL,_UDP_SERVICE_ThreadBroadCastInit,&tUDPNetEventCtl))
	{
		zlog_error(ptMainServer->ptZlogCategory,"udp broadcast Init Failed\n");
		abort();
	}
	if(-1 == _UDP_SERVICE_UdpEventCtlInit(&tUDPNetEventCtl))
	{
		zlog_error(ptMainServer->ptZlogCategory,"_UdpEvent Init Failed\n");
		abort();
	}

    pthread_mutex_lock(&ptMainServer->tThread_StartMutex);
	++ptMainServer->iThread_bStartCnt;
	pthread_cond_signal(&ptMainServer->tThread_StartCond);
	pthread_mutex_unlock(&ptMainServer->tThread_StartMutex);

     while(1)
     { 
        EVIO_EventCtlLoop_Start(tUDPNetEventCtl.ptEventCtl);
     }
	EVIO_EventFd_Del(tUDPNetEventCtl.ptEventCtl, tUDPNetEventCtl.tNanoMsgUDPNet.ptEventFd);
	EVIO_EventCtl_Free(tUDPNetEventCtl.ptEventCtl);
	close(tUDPNetEventCtl.tNanoMsgUDPNet.iSysFd);
	nn_close(tUDPNetEventCtl.tNanoMsgUDPNet.iNanoMsgFd);
	return NULL;
}