/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 11:02:55
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-11 11:00:49
 */ 
//__attribute__ ((unused)) #define UNUSED(x) (void)(x)

#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "evio/evio.h"
#include "net/evnet.h"
#include "hash/shash.h"
#include <assert.h>
#include "nanomsg/pair.h"
#include "nanomsg/nn.h"
#include "udp_service.h"
#include "main.h"
#include "manage/sys_service.h"
#include "include/commnanomsg.h"
#include "driver/led.h"






void Main_TimerOut_Handle(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
	T_MAINSERVER *ptMainServer = (T_MAINSERVER *)_pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;
    //zlog_debug(ptMainServer->ptZlogCategory,"1 second Timerout:%ld tm:%ld,%p\r\n",_ptEventTimer->u64Index,_ptEventTimer->u64Expire,ptEventCtl);
	EVIO_EventTimer_Init(_ptEventTimer,1000,Main_TimerOut_Handle,ptMainServer);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);

	#if 0
	char *str = "Main Timer Timout SEND OK";
    uint8_t *dat = nn_allocmsg(100, 0);
	//zlog_debug(ptMainServer->ptZlogCategory,"dat===%p,%p\n",dat,ptMainServer);
    if (NULL != dat) {
         memcpy(dat, str, strlen(str));
	 	 /*int ret = */nn_send(ptMainServer->tMainServerEventLoop.tNanoMsgFdsUDP.iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
		// zlog_debug(ptMainServer->ptZlogCategory,"ret====%d\n",ret);
	}
	#endif
	
}



void Main_Handle_NanomsgEvent(T_EVENT_CTL *_ptEventCtl, T_MAIN_NANOMSGFDS *_ptMainNanoMsgFds)
{ 
	uint8_t *dat = NULL;
	T_MAINSERVER *ptMainServer = _ptMainNanoMsgFds->pvArg;
	uint32_t bytes = nn_recv(_ptMainNanoMsgFds->iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) {
			zlog_debug(ptMainServer->ptZlogCategory,"UDP-->Mainserver++%d\n",dat[0]);
		switch(dat[0])
		{
			case TO_BROADCAST_NS:
				nn_send(ptMainServer->tMainServerEventLoop.tNanoMsgFdsBroadCast.iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
			   break;

			default:
				nn_freemsg(dat);
				break;
		}

		
	}
}

void Main_EventLoop_Handle(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_MAIN_NANOMSGFDS *ptMainNanoMsgFds = _pvArg;
	T_MAINSERVER *ptMainServer = ptMainNanoMsgFds->pvArg;
    //zlog_debug(ptMainServer->ptZlogCategory,"+++++++++++++++++++++++++++++++Main_EventLoop_Handle\n");
    switch (_eType) {
	    case E_EV_READ:
	        ptMainNanoMsgFds->pfCallBack(_ptEventCtl, ptMainNanoMsgFds);
	        break;
		case E_EV_WRITE:
			zlog_warn(ptMainServer->ptZlogCategory,"Main loop write event, unexpected\n");
			break;
		case E_EV_ERROR:
			zlog_error(ptMainServer->ptZlogCategory,"Main loop error event, unexpected\n");
			break;
	    default:
			zlog_warn(ptMainServer->ptZlogCategory,"Main loop unknow event, unexpected\n");
    }
}

int32_t Main_NanomsgSocket_Init(T_MAINSERVER *_ptMainServer)
{
	size_t size = sizeof(size_t);
	T_MAINSERVEREVENTLOOP *ptMainServerEventLoop = &_ptMainServer->tMainServerEventLoop;
	
	/*******************Main<-->UDP*************/
	ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd)
	{
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop nn_socket error...\n");
		return -1;
	}
	if (-1 == nn_bind(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd, "inproc://udp<->main"))
	{
		nn_close(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop nn_bind error...\n");
		return -1;
	}
	
	if (-1 == nn_getsockopt(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd, NN_SOL_SOCKET, NN_RCVFD, (char *)&ptMainServerEventLoop->tNanoMsgFdsUDP.iSysFd, &size))
	{
     	nn_close(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop nn_getsockopt error...\n");
		return -1;
	}

	/*******************Main<-->FeqSyncAudio*************/
	ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd)
	{
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop fepsync_audio nn_socket error...\n");
		return -1;
	}
	if (-1 == nn_bind(ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd, "inproc://main<->fepsync_audio"))
	{
		nn_close(ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop fepsync_audio nn_bind error...\n");
		return -1;
	}
	
	if (-1 == nn_getsockopt(ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd, NN_SOL_SOCKET, NN_RCVFD, (char *)&ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iSysFd, &size))
	{
     	nn_close(ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop fepsync_audio nn_getsockopt error...\n");
		return -1;
	}

	/*******************Main<-->BroadCast*************/
	ptMainServerEventLoop->tNanoMsgFdsBroadCast.iNanomsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iNanomsgFd)
	{
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop broadcast nn_socket error...\n");
		return -1;
	}
	if (-1 == nn_bind(ptMainServerEventLoop->tNanoMsgFdsBroadCast.iNanomsgFd, "inproc://main<->broadcast"))
	{
		nn_close(ptMainServerEventLoop->tNanoMsgFdsBroadCast.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop broadcast nn_bind error...\n");
		return -1;
	}
	
	if (-1 == nn_getsockopt(ptMainServerEventLoop->tNanoMsgFdsBroadCast.iNanomsgFd, NN_SOL_SOCKET, NN_RCVFD, (char *)&ptMainServerEventLoop->tNanoMsgFdsBroadCast.iSysFd, &size))
	{
     	nn_close(ptMainServerEventLoop->tNanoMsgFdsBroadCast.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop broadcast nn_getsockopt error...\n");
		return -1;
	}

   /*******************Main<--IOINPUT*************/
	ptMainServerEventLoop->tNanoMsgFdsIOInput.iNanomsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == ptMainServerEventLoop->tNanoMsgFdsIOInput.iNanomsgFd)
	{
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop ioinput nn_socket error...\n");
		return -1;
	}
	if (-1 == nn_bind(ptMainServerEventLoop->tNanoMsgFdsIOInput.iNanomsgFd, "inproc://main<->ioinput"))
	{
		nn_close(ptMainServerEventLoop->tNanoMsgFdsIOInput.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop ioinput nn_bind error...\n");
		return -1;
	}
	
	if (-1 == nn_getsockopt(ptMainServerEventLoop->tNanoMsgFdsIOInput.iNanomsgFd, NN_SOL_SOCKET, NN_RCVFD, (char *)&ptMainServerEventLoop->tNanoMsgFdsIOInput.iSysFd, &size))
	{
     	nn_close(ptMainServerEventLoop->tNanoMsgFdsIOInput.iNanomsgFd);
		zlog_error(_ptMainServer->ptZlogCategory,"+++main_loop ioinput nn_getsockopt error...\n");
		return -1;
	}
	return 0;
}
void Main_NanomsgSocket_Close(T_MAINSERVER *_ptMainServer)
{
	T_MAINSERVEREVENTLOOP *ptMainServerEventLoop = &_ptMainServer->tMainServerEventLoop;
	close(ptMainServerEventLoop->tNanoMsgFdsUDP.iSysFd);
	nn_close(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd);

}
int32_t  Main_Loop_Init(T_MAINSERVER *_ptMainServer)
{
	
	T_MAINSERVEREVENTLOOP *ptMainServerEventLoop = &_ptMainServer->tMainServerEventLoop;
	if (-1 == Main_NanomsgSocket_Init(_ptMainServer))
		return -1;

	
	ptMainServerEventLoop->ptEventCtl= EVIO_EventCtl_Create();  //创建一个事件控制器 creator event contoler
	if(NULL == ptMainServerEventLoop->ptEventCtl)
	 {
		 zlog_error(_ptMainServer->ptZlogCategory,"+++ptMainServerEventLoop->ptEventCtl error\n");
		 Main_NanomsgSocket_Close(_ptMainServer);
		 return -1;
	 }
	
	ptMainServerEventLoop->tNanoMsgFdsUDP.pfCallBack= Main_Handle_NanomsgEvent;//UDP enventloop main server
    ptMainServerEventLoop->tNanoMsgFdsUDP.pvArg     = _ptMainServer;
    ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd = EVIO_EventFd_Add(ptMainServerEventLoop->ptEventCtl,\
													  ptMainServerEventLoop->tNanoMsgFdsUDP.iSysFd, \
													  Main_EventLoop_Handle, &ptMainServerEventLoop->tNanoMsgFdsUDP);//add the UDP tms fd 

	ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.pfCallBack= Main_Handle_NanomsgEvent;//UDP Fepsync main server
    ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.pvArg     = _ptMainServer;
	ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.ptEventFd   = EVIO_EventFd_Add(ptMainServerEventLoop->ptEventCtl,\
													  ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.iSysFd, \
													  Main_EventLoop_Handle, &ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP);//add the FepSyncAudio fd 

	ptMainServerEventLoop->tNanoMsgFdsBroadCast.pfCallBack= Main_Handle_NanomsgEvent;//Broadcast enventloop main server
    ptMainServerEventLoop->tNanoMsgFdsBroadCast.pvArg     = _ptMainServer;						  
	ptMainServerEventLoop->tNanoMsgFdsBroadCast.ptEventFd = EVIO_EventFd_Add(ptMainServerEventLoop->ptEventCtl,\
													  ptMainServerEventLoop->tNanoMsgFdsBroadCast.iSysFd, \
													  Main_EventLoop_Handle, &ptMainServerEventLoop->tNanoMsgFdsBroadCast);//add the event fd 

	
	ptMainServerEventLoop->tNanoMsgFdsIOInput.pfCallBack= Main_Handle_NanomsgEvent;//IO input enventloop main server
    ptMainServerEventLoop->tNanoMsgFdsIOInput.pvArg     = _ptMainServer;						  
	ptMainServerEventLoop->tNanoMsgFdsIOInput.ptEventFd = EVIO_EventFd_Add(ptMainServerEventLoop->ptEventCtl,\
													  ptMainServerEventLoop->tNanoMsgFdsIOInput.iSysFd, \
													  Main_EventLoop_Handle, &ptMainServerEventLoop->tNanoMsgFdsIOInput);//add the event fd 


													  
	if (NULL == ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd 
		||(NULL == ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.ptEventFd)
		||(NULL == ptMainServerEventLoop->tNanoMsgFdsBroadCast.ptEventFd)
		||(NULL == ptMainServerEventLoop->tNanoMsgFdsIOInput.ptEventFd))
	{
		zlog_error(_ptMainServer->ptZlogCategory,"+++ptMainServerEventLoop->tNanoMsgxx.ptEventFd error\n");
		EVIO_EventCtl_Free(ptMainServerEventLoop->ptEventCtl);
		Main_NanomsgSocket_Close(_ptMainServer);
		return -1;
	}
	 EVIO_Event_Watch_Read(ptMainServerEventLoop->ptEventCtl, ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd);
	 EVIO_Event_Watch_Read(ptMainServerEventLoop->ptEventCtl, ptMainServerEventLoop->tNanoMsgFdsFepSyncUDP.ptEventFd);
	 EVIO_Event_Watch_Read(ptMainServerEventLoop->ptEventCtl, ptMainServerEventLoop->tNanoMsgFdsBroadCast.ptEventFd);
	 EVIO_Event_Watch_Read(ptMainServerEventLoop->ptEventCtl, ptMainServerEventLoop->tNanoMsgFdsIOInput.ptEventFd);
	 EVIO_EventTimer_Init(&_ptMainServer->tEventTimer,1000,Main_TimerOut_Handle,_ptMainServer);
	 EVIO_EventTimer_Start(ptMainServerEventLoop->ptEventCtl,&_ptMainServer->tEventTimer);
	 return 0;
}

void Main_Loop_Del(T_MAINSERVER *_ptMainServer)
{
	T_MAINSERVEREVENTLOOP *ptMainServerEventLoop = &_ptMainServer->tMainServerEventLoop;
	EVIO_EventFd_Del(ptMainServerEventLoop->ptEventCtl, ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd);
	EVIO_EventCtl_Free(ptMainServerEventLoop->ptEventCtl);
	Main_NanomsgSocket_Close(_ptMainServer);
}


static int32_t  Main_Zlog_Init(T_MAINSERVER *_ptMainServer)
{
    /* 日志 */
	if (zlog_init(ZLOGCONF)) {
		printf("zlog init failed\n");
		return -1;
	}
	_ptMainServer->ptZlogCategory = zlog_get_category("zlog");
	if (!_ptMainServer->ptZlogCategory) {
		printf("Get log category failed");
		return -1;
	}
    zlog_info(_ptMainServer->ptZlogCategory, "Current version of software is: %s", SOFT_VER);

	return 0;
	
}

int main( void )
{
     int i =0;
	 int ret =0;
	 T_MAINSERVER tmainServer;
	 pthread_t tThreadUDP;
     pthread_t tThreadTrainSys;
	 ret =Main_Zlog_Init(&tmainServer);
	 
	 if(-1 == ret)
	 {
		 return -1;
	 }
	 LED_Init();
	 ret = Main_Loop_Init(&tmainServer);
	 if(-1 == ret)
	 {
		zlog_error(tmainServer.ptZlogCategory,"main loop init... error\n");
		return -1;
	 }
	 sleep(1);
     tmainServer.iThread_bStartCnt = 0;
	 pthread_mutex_init(&tmainServer.tThread_StartMutex, NULL);
	 pthread_cond_init(&tmainServer.tThread_StartCond, NULL);
	 
	 ret =pthread_create(&tThreadTrainSys,NULL,TrainSystemService_ThreadHandle,&tmainServer);

	 if(-1 == ret)
	 {
		zlog_error(tmainServer.ptZlogCategory,"TrainSystem thread pthread create error\n");
		Main_Loop_Del(&tmainServer);
		return -1; 
	 }
	 
	 ret =pthread_create(&tThreadUDP,NULL,UDP_SERVICE_Thread_Handle,&tmainServer);

	 if(-1 == ret)
	 {
		zlog_error(tmainServer.ptZlogCategory,"UDP service thread pthread create error\n");
		Main_Loop_Del(&tmainServer);
		return -1; 
	 }


	 while (tmainServer.iThread_bStartCnt < 6) {
        pthread_mutex_lock(&tmainServer.tThread_StartMutex);
        pthread_cond_wait(&tmainServer.tThread_StartCond, &tmainServer.tThread_StartMutex);
        pthread_mutex_unlock(&tmainServer.tThread_StartMutex);
        zlog_debug(tmainServer.ptZlogCategory,"pthread isstart cnt is %d\n", tmainServer.iThread_bStartCnt);    
  	}
     while(1)
     {
        EVIO_EventCtlLoop_Start(tmainServer.tMainServerEventLoop.ptEventCtl);
     }
  return 0;
}