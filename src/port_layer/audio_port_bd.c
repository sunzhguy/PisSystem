/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-21 10:40:22
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-08 17:33:58
 */
#include "audio_port_bd.h"
#include "../include/general.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

T_BDAUDIO_NET_EVCTL sgtBDAudioNetCtl;

void _BDAUDIO_HandleNanoMsg(T_EVENT_CTL *_ptEventCtl, T_BDAUDIONANOMSG_EVFDS *_ptNanomsgEvFds)
{
	T_BDAUDIO_NET_EVCTL *ptBDAudioNetCtl = _ptNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer     = ptBDAudioNetCtl->ptServer;

	uint8_t *dat = NULL;
	uint32_t bytes = nn_recv(_ptNanomsgEvFds->iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) {
			zlog_debug(ptMainServer->ptZlogCategory,"########FepSYNAudio->BDAudio Nanomsg+++++++++++++++%s\n",dat);
            
			nn_freemsg(dat);
	}
}

static void _BDAUDIO_NanoMsg_CallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_BDAUDIONANOMSG_EVFDS *ptNanomsgEvFds          = (T_BDAUDIONANOMSG_EVFDS*)_pvArg;
	T_BDAUDIO_NET_EVCTL *ptBDAudioNetCtl     = ptNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer              = ptBDAudioNetCtl->ptServer;

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

static int32_t _BDAUDIO_NanoMsgAndEventCtlInit(T_BDAUDIO_NET_EVCTL *_ptBDAudioNetEventCtl)
{
	T_MAINSERVER    *ptMainServer     = _ptBDAudioNetEventCtl->ptServer;
	size_t size = sizeof(size_t);
    _ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == _ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_socket failed\n");
		return -1;
	}
	if (-1 == nn_bind(_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd, "inproc://fep_synaudio<->bd_audio"))
	{
	  zlog_error(ptMainServer->ptZlogCategory,"++++BDAudio nn_bind failed\n");
      nn_close(_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd);
	  return -1;
	}
	zlog_debug(ptMainServer->ptZlogCategory,"--------------------------%d\n",_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd);
	
	if (-1 == nn_getsockopt(_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd, NN_SOL_SOCKET, NN_RCVFD,\
							 (char *)&_ptBDAudioNetEventCtl->tNanomsgEvFds.iSysFd, &size))
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_getsockopt failed\n");
		nn_close(_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd);
		return -1;	 
	}


	_ptBDAudioNetEventCtl->ptEventCtl = EVIO_EventCtl_Create();
	
	if (NULL == _ptBDAudioNetEventCtl->ptEventCtl)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++EVIO_EventCtl_Create failed\n");
		close(_ptBDAudioNetEventCtl->tNanomsgEvFds.iSysFd);
		nn_close(_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd);
		return -1;
	}
    _ptBDAudioNetEventCtl->tNanomsgEvFds.pfEventCallBack = _BDAUDIO_HandleNanoMsg;
    _ptBDAudioNetEventCtl->tNanomsgEvFds.pvArg = _ptBDAudioNetEventCtl;
    _ptBDAudioNetEventCtl->tNanomsgEvFds.ptEventFd = EVIO_EventFd_Add(_ptBDAudioNetEventCtl->ptEventCtl, \
													 _ptBDAudioNetEventCtl->tNanomsgEvFds.iSysFd,\
													 _BDAUDIO_NanoMsg_CallBack, 
													 &_ptBDAudioNetEventCtl->tNanomsgEvFds);//add the event fd 
	if (NULL == _ptBDAudioNetEventCtl->tNanomsgEvFds.ptEventFd)
	{
		EVIO_EventCtl_Free(_ptBDAudioNetEventCtl->ptEventCtl);
		close(_ptBDAudioNetEventCtl->tNanomsgEvFds.iSysFd);
		nn_close(_ptBDAudioNetEventCtl->tNanomsgEvFds.iNanoMsgFd);
		zlog_error(ptMainServer->ptZlogCategory,"_ptBDAudioNetEventCtl tNanoMsgUDPNet.ptEventFd failed\n");
		return -1;
	}
	EVIO_Event_Watch_Read(_ptBDAudioNetEventCtl->ptEventCtl, _ptBDAudioNetEventCtl->tNanomsgEvFds.ptEventFd);
	return 0;
	
}


static void _BDAUDIO_NanoMsgClose(T_BDAUDIONANOMSG_EVFDS *_ptNanomsgEvFds)
{
	if(_ptNanomsgEvFds)
	{
		close(_ptNanomsgEvFds->iSysFd);
		nn_close(_ptNanomsgEvFds->iNanoMsgFd);
	}
}


void  _BDAudio_ReadAFrameData(uint8_t *_pcBufer,uint32_t _u32BufLen)
{

  if(_u32BufLen >0)//进行FEP 音频同步发出数据
  {
      printf("xxxxxxxxxxxx-BDAUDIO-xxxxxxxxxxx\r\n");
  }
}


void _BDAUDIO_Recive_EventCallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_UDP * _ptEventUDP , void *_pvArg)
 {
     uint32_t beg = 0;
	 uint32_t left_len =0;
     T_EVNET_BUFFER *ptEvNetBuffer = _ptEventUDP->ptEvNetBuffer;
	 //T_FEP_AUDIO_NET_EVCTL *ptUdpNetEventCtl = (T_FEP_AUDIO_NET_EVCTL *) _pvArg;

	 if(ptEvNetBuffer->iReadLen >0)
	 { 
		  if(ptEvNetBuffer->iReadLen >=1024)
		  {
			  _BDAudio_ReadAFrameData((uint8_t*)ptEvNetBuffer->acReadBuffer,1024);//PIS 过程数据读取1K数据
			  left_len = ptEvNetBuffer->iReadLen -1024;
			  beg = 1024;
		  }else
		  {
			 
			  _BDAudio_ReadAFrameData((uint8_t*)ptEvNetBuffer->acReadBuffer,ptEvNetBuffer->iReadLen);//PIS 过程数据读取1K数据
			  left_len =0;
		  }
		  
		  ptEvNetBuffer->iReadOffset = left_len;
		  if(ptEvNetBuffer->iReadOffset > 0)
		  {
			  memmove(ptEvNetBuffer->acReadBuffer, ptEvNetBuffer->acReadBuffer + beg, ptEvNetBuffer->iReadOffset);
		  }
		  
	 }

 }


void *BDAudio_Thread_Handle(void *_pvArg)
{
    T_MAINSERVER *ptMainServer   = (T_MAINSERVER *) _pvArg;
   

    sgtBDAudioNetCtl.ptServer    = ptMainServer;
    sgtBDAudioNetCtl.ptEventCtl  = NULL;
    sgtBDAudioNetCtl.ptEventUdp  = NULL;
	if(-1 == _BDAUDIO_NanoMsgAndEventCtlInit(&sgtBDAudioNetCtl))
	{
		zlog_error(ptMainServer->ptZlogCategory,"BD_Audio_SyncThread_Handle Event Ctl Init Failed\n");
		abort();
	}
    
    if(sgtBDAudioNetCtl.ptEventCtl == NULL)
	{
	  zlog_error(ptMainServer->ptZlogCategory,"BroadCast Audio ptEventCtl Create error\n");
	  abort();
	}
   
	//功率放大器同步音频
	T_EVENT_UDP *ptEventBDAudioUdp = EV_NET_EventUDP_CreateAndStart(sgtBDAudioNetCtl.ptEventCtl,NULL,BROADCAST_AUDIO_LOCAL_PORT,BROADCAST_AUDIO_REMOTE_IP,BROADCAST_AUDIO_REMOTE_PORT,_BDAUDIO_Recive_EventCallBack,ptMainServer);
	 if(ptEventBDAudioUdp == NULL)
     {
          zlog_error(ptMainServer->ptZlogCategory,"BroadCast Audio Event UDP create failed\r\n");
		  EVIO_EventCtl_Free(sgtBDAudioNetCtl.ptEventCtl);
          abort();
     }
   
     sgtBDAudioNetCtl.ptEventUdp = ptEventBDAudioUdp;
    while(1)
     {
        EVIO_EventCtlLoop_Start(sgtBDAudioNetCtl.ptEventCtl);
     }
    EV_NET_EventUDP_Close(sgtBDAudioNetCtl.ptEventCtl,ptEventBDAudioUdp);
    _BDAUDIO_NanoMsgClose(&sgtBDAudioNetCtl.tNanomsgEvFds);
	EVIO_EventCtl_Free(sgtBDAudioNetCtl.ptEventCtl);
	return NULL;  
}


void AudioPortBD_SendData(uint8_t* _pcBuf,uint32_t _u32DatLen)
{
	if(sgtBDAudioNetCtl.ptEventUdp !=NULL && sgtBDAudioNetCtl.ptEventCtl!= NULL)
	{
      EV_NET_EventUDP_WriteData(sgtBDAudioNetCtl.ptEventCtl,sgtBDAudioNetCtl.ptEventUdp,_pcBuf,_u32DatLen);
	}
	
}
