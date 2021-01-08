/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-07 16:22:19
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-08 16:13:26
 */

#include "fep_audio_port.h"
#include "../include/general.h"
#include "../include/commnanomsg.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


T_FEP_AUDIO_NET_EVCTL gtFepAudioNetCtl;




void _FEPAUDIO_HandleNanoMsg(T_EVENT_CTL *_ptEventCtl, T_NANOMSG_EVFDS *_ptNanomsgEvFds)
{
	T_FEP_AUDIO_NET_EVCTL *ptFepAudioNetCtl = _ptNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer     = ptFepAudioNetCtl->ptServer;
	//zlog_debug(ptMainServer->ptZlogCategory,"++++++++++++++%s:%d\n",__func__,__LINE__);
	#if 1
	uint8_t *dat = NULL;
	uint32_t bytes = nn_recv(_ptNanomsgEvFds->iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) {
			zlog_debug(ptMainServer->ptZlogCategory,"Main->UDP Nanomsg+++++++++++++++%s\n",dat);
			nn_freemsg(dat);
	}
	#endif
}

static void _FEPAUDIO_NanoMsg_CallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_NANOMSG_EVFDS *ptNanomsgEvFds          = (T_NANOMSG_EVFDS*)_pvArg;
	T_FEP_AUDIO_NET_EVCTL *ptFepAudioNetCtl     = ptNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer              = ptFepAudioNetCtl->ptServer;

    switch (_eType) {
	    case E_EV_READ:
	        ptNanomsgEvFds->pfEventCallBack(_ptEventCtl, ptNanomsgEvFds);
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

static int32_t _FEPAUDIO_NanoMsgAndEventCtlInit(T_FEP_AUDIO_NET_EVCTL *_ptFepAudioNetEventCtl)
{
	T_MAINSERVER    *ptMainServer     = _ptFepAudioNetEventCtl->ptServer;
	size_t size = sizeof(size_t);
    _ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == _ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_socket failed\n");
		return -1;
	}
	if (-1 == nn_connect(_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd, "inproc://fep_synaudio<->bd_audio"))
	{
	  zlog_error(ptMainServer->ptZlogCategory,"++++nn_connect failed\n");
      nn_close(_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd);
	  return -1;
	}
	zlog_debug(ptMainServer->ptZlogCategory,"------------FEP--------------<%d>\n",_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd);
	
	if (-1 == nn_getsockopt(_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd, NN_SOL_SOCKET, NN_RCVFD,\
							 (char *)&_ptFepAudioNetEventCtl->tNanoMsg.iSysFd, &size))
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_getsockopt failed\n");
		nn_close(_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd);
		return -1;	 
	}


	_ptFepAudioNetEventCtl->ptEventCtl = EVIO_EventCtl_Create();
	
	if (NULL == _ptFepAudioNetEventCtl->ptEventCtl)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++EVIO_EventCtl_Create failed\n");
		close(_ptFepAudioNetEventCtl->tNanoMsg.iSysFd);
		nn_close(_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd);
		return -1;
	}
    _ptFepAudioNetEventCtl->tNanoMsg.pfEventCallBack = _FEPAUDIO_HandleNanoMsg;
    _ptFepAudioNetEventCtl->tNanoMsg.pvArg = _ptFepAudioNetEventCtl;
    _ptFepAudioNetEventCtl->tNanoMsg.ptEventFd = EVIO_EventFd_Add(_ptFepAudioNetEventCtl->ptEventCtl, \
													 _ptFepAudioNetEventCtl->tNanoMsg.iSysFd,\
													 _FEPAUDIO_NanoMsg_CallBack, 
													 &_ptFepAudioNetEventCtl->tNanoMsg);//add the event fd 
	if (NULL == _ptFepAudioNetEventCtl->tNanoMsg.ptEventFd)
	{
		EVIO_EventCtl_Free(_ptFepAudioNetEventCtl->ptEventCtl);
		close(_ptFepAudioNetEventCtl->tNanoMsg.iSysFd);
		nn_close(_ptFepAudioNetEventCtl->tNanoMsg.iNanoMsgFd);
		zlog_error(ptMainServer->ptZlogCategory,"tNanoMsgUDPNet.ptEventFd failed\n");
		return -1;
	}
	EVIO_Event_Watch_Read(_ptFepAudioNetEventCtl->ptEventCtl, _ptFepAudioNetEventCtl->tNanoMsg.ptEventFd);
	return 0;
	
}


static void _FEPAUDIO_NanoMsgClose(T_NANOMSG_EVFDS *_ptNanomsgEvFds)
{
	if(_ptNanomsgEvFds)
	{
		close(_ptNanomsgEvFds->iSysFd);
		nn_close(_ptNanomsgEvFds->iNanoMsgFd);
	}
}


static void  _FEP_Audio_SendNanoMsg(T_FEP_AUDIO_NET_EVCTL *_ptFepAudioNetCtl)
{
	uint8_t *dat = nn_allocmsg(4, 0);
    if (NULL != dat) 
	{
		dat[0] = NANOMSG_FEPSYN2BDAUDIO;
	    int bytes = nn_send(_ptFepAudioNetCtl->tNanoMsg.iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
		//printf("{%d}--send.........byte...%d\r\n",_ptFepAudioNetCtl->tNanoMsg.iNanoMsgFd,bytes);
	}
}



static void  _FEP_Audio_ReadAFrameData(uint8_t *_pcBufer,uint32_t _u32BufLen)
{

  if(_u32BufLen >0)//进行FEP 音频同步发出数据
  {
      _FEP_Audio_SendNanoMsg(&gtFepAudioNetCtl);
	  //此出进行音频广播发送信号给 Aduio 广播进程
  }
}


static void _FEP_AUDIO_Recive_EventCallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_UDP * _ptEventUDP , void *_pvArg)
 {
     uint32_t beg = 0;
	 uint32_t left_len =0;
     T_EVNET_BUFFER *ptEvNetBuffer           = _ptEventUDP->ptEvNetBuffer;
	 T_MAINSERVER *ptMainServer 			 = (T_MAINSERVER *) _pvArg;//网络为了兼容传递了MainServer

	 if(ptEvNetBuffer->iReadLen >0)
	 { 
		  if(ptEvNetBuffer->iReadLen >=1024)
		  {
			  _FEP_Audio_ReadAFrameData((uint8_t*)ptEvNetBuffer->acReadBuffer,1024);//PIS 过程数据读取1K数据
			  left_len = ptEvNetBuffer->iReadLen -1024;
			  beg = 1024;
		  }else
		  {
			 
			  _FEP_Audio_ReadAFrameData((uint8_t*)ptEvNetBuffer->acReadBuffer,ptEvNetBuffer->iReadLen);//PIS 过程数据读取1K数据
			  left_len =0;
		  }
		  
		  ptEvNetBuffer->iReadOffset = left_len;
		  if(ptEvNetBuffer->iReadOffset > 0)
		  {
			  memmove(ptEvNetBuffer->acReadBuffer, ptEvNetBuffer->acReadBuffer + beg, ptEvNetBuffer->iReadOffset);
		  }
		  
	 }

 }


void *FEP_Audio_SyncThread_Handle(void *_pvArg)
{
    T_MAINSERVER *ptMainServer = (T_MAINSERVER *) _pvArg;
    gtFepAudioNetCtl.ptServer = ptMainServer;
    gtFepAudioNetCtl.ptEventCtl = NULL;
	gtFepAudioNetCtl.ptEventUdp  = NULL;
	if(-1 == _FEPAUDIO_NanoMsgAndEventCtlInit(&gtFepAudioNetCtl))
	{
		zlog_error(ptMainServer->ptZlogCategory,"FEP_Audio_SyncThread_Handle Event Ctl Init Failed\n");
		abort();
	}
	
	printf("##########%d######################\r\n",gtFepAudioNetCtl.tNanoMsg.iNanoMsgFd);
	//功率放大器同步音频
	T_EVENT_UDP *ptEventFepAudioUdp = EV_NET_EventUDP_CreateAndStart(gtFepAudioNetCtl.ptEventCtl,NULL,FEP_AUDIO_LOCAL_PORT,NULL,FEP_AUDIO_REMOTE_PORT,_FEP_AUDIO_Recive_EventCallBack,ptMainServer);
	 if(ptEventFepAudioUdp == NULL)
     {
          zlog_error(ptMainServer->ptZlogCategory,"FepAudio Event UDP create failed\r\n");
		  EVIO_EventCtl_Free(gtFepAudioNetCtl.ptEventCtl );
          abort();
     }

     gtFepAudioNetCtl.ptEventUdp = ptEventFepAudioUdp;

    while(1)
     {
        EVIO_EventCtlLoop_Start(gtFepAudioNetCtl.ptEventCtl);
     }
	EV_NET_EventUDP_Close(gtFepAudioNetCtl.ptEventCtl,ptEventFepAudioUdp);
	_FEPAUDIO_NanoMsgClose(&gtFepAudioNetCtl.tNanoMsg);
	EVIO_EventCtl_Free(gtFepAudioNetCtl.ptEventCtl);
	return NULL;  
}
