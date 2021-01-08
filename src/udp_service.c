/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 08:40:25
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-08 14:56:09
 */ 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include "udp_service.h"
#include "port_layer/ctrl_udport.h"
#include "port_layer/fep_audio_port.h"




 void _UDP_SERVICE_UdpRecive_EventCallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_UDP * _ptEventUDP , void *_pvArg)
 {
     uint32_t beg = 0;
	 uint32_t left_len =0;
     T_EVNET_BUFFER *ptEvNetBuffer = _ptEventUDP->ptEvNetBuffer;
	 T_UDP_NET_EVCTL *ptUdpNetEventCtl = (T_UDP_NET_EVCTL *) _pvArg;

	 if(ptEvNetBuffer->iReadLen >0)
	 { 
		  if(ptEvNetBuffer->iReadLen >=1024)
		  {
			  CTRL_UDPORT_ReadAFrameData(ptEvNetBuffer->acReadBuffer,1024);//PIS 过程数据读取1K数据
			  left_len = ptEvNetBuffer->iReadLen -1024;
			  beg = 1024;
		  }else
		  {
			 
			  CTRL_UDPORT_ReadAFrameData(ptEvNetBuffer->acReadBuffer,ptEvNetBuffer->iReadLen);//PIS 过程数据读取1K数据
			  left_len =0;
		  }
		  
		  ptEvNetBuffer->iReadOffset = left_len;
		  if(ptEvNetBuffer->iReadOffset > 0)
		  {
			  memmove(ptEvNetBuffer->acReadBuffer, ptEvNetBuffer->acReadBuffer + beg, ptEvNetBuffer->iReadOffset);
		  }
		  
	 }

 }





void *_UDP_SERVICE_ThreadBroadCastInit(void *_pvArg)
{
     T_EVENT_CTL * ptEventCtl = EVIO_EventCtl_Create();
	 T_UDP_NET_EVCTL *ptUdpNetEventCtl = (T_UDP_NET_EVCTL *) _pvArg;
	 T_MAINSERVER *ptMainServer = ptUdpNetEventCtl->ptServer;
	 


    if(ptEventCtl == NULL)
	{
		 zlog_error(ptMainServer->ptZlogCategory,"ThreadBroadCastInit ptEventCtl Create error\n");
		 abort();
	}
	//TMS 网络接收 以及网络发送
     T_EVENT_UDP *ptEventUdp = EV_NET_EventUDP_CreateAndStart(ptEventCtl,BROADCAST_IP,UDP_LOCAL_PORT,BROADCAST_IP,UDP_REMOTE_PORT,_UDP_SERVICE_UdpRecive_EventCallBack,ptMainServer);
	  
	 if(ptEventUdp == NULL)
     {
          zlog_error(ptMainServer->ptZlogCategory,"Event UDP create failed\r\n");
		  EVIO_EventCtl_Free(ptEventCtl);
          abort();
     }

	#if 0
	 T_EVENT_UDP *ptEventFepAudioUdp = EV_NET_EventUDP_CreateAndStart(ptEventCtl,NULL,FEP_AUDIO_LOCAL_PORT,NULL,FEP_AUDIO_REMOTE_PORT,_UDP_FEP_AUDIO_Recive_EventCallBack,_pvArg);
	  
	 if(ptEventFepAudioUdp == NULL)
     {
          zlog_error(ptMainServer->ptZlogCategory,"Event UDP FEP Audio create failed\r\n");
		  EVIO_EventFd_Del(ptEventCtl,ptEventUdp->ptEventFd);
		  EVIO_EventCtl_Free(ptEventCtl);
          abort();
     }
	 ptUdpNetEventCtl->ptEventFepAudioUdp = ptEventFepAudioUdp;
	#endif


	 ptUdpNetEventCtl->ptUDPEventCtl      = ptEventCtl;
	 ptUdpNetEventCtl->ptEventUdp         = ptEventUdp;
	 
     while(1)
     {
        EVIO_EventCtlLoop_Start(ptEventCtl);
     }
    //EVIO_EventFd_Del(ptEventCtl,ptEventUdp->ptEventFd);
	//EVIO_EventFd_Del(ptEventCtl,ptEventFepAudioUdp->ptEventFd);
	EV_NET_EventUDP_Close(ptEventCtl,ptEventUdp);
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
	if (-1 == nn_connect(_ptUdpNetEventCtl->tNanoMsgUDPNet.iNanoMsgFd, "inproc://udp<->broadcast"))
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

T_UDP_NET_EVCTL tUDPNetEventCtl;

void *UDP_SERVICE_Thread_Handle(void *_pvArg)
{
	
     pthread_t tPthread_udpbrodcast;
	 
	 T_MAINSERVER *ptMainServer = (T_MAINSERVER *) _pvArg;
	 tUDPNetEventCtl.ptServer = ptMainServer;
	 tUDPNetEventCtl.ptUDPEventCtl = NULL;
	 tUDPNetEventCtl.ptEventUdp = NULL;
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
	close(tUDPNetEventCtl.tNanoMsgUDPNet.iSysFd);
	nn_close(tUDPNetEventCtl.tNanoMsgUDPNet.iNanoMsgFd);
	EVIO_EventCtl_Free(tUDPNetEventCtl.ptEventCtl);
	return NULL;
}


void UDP_SERVICE_SendData(uint8_t* _pcBuf,uint32_t _u32DatLen)
{
	if(tUDPNetEventCtl.ptUDPEventCtl !=NULL && tUDPNetEventCtl.ptEventCtl!= NULL)
	{
	  //printf("write data....%d\r\n",_u32DatLen);
      EV_NET_EventUDP_WriteData(tUDPNetEventCtl.ptUDPEventCtl,tUDPNetEventCtl.ptEventUdp,_pcBuf,_u32DatLen);
	}
	
}

void  UDP_SERVICE_SendNanoMsg(uint8_t *_pcMsg)
{
	if(_pcMsg != NULL)
	{
		printf("22222222222\r\n");
	    int bytes = 	nn_send(tUDPNetEventCtl.tNanoMsgUDPNet.iNanoMsgFd, &_pcMsg, NN_MSG, NN_DONTWAIT);
	    printf("222222bytes===%d\r\n",bytes);
	}
}