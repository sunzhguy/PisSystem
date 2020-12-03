/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 11:02:55
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-03 10:35:45
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



void Main_TimerOut_Handle(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
	T_MAINSERVER *ptMainServer = (T_MAINSERVER *)_pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;
    printf("++++++++++++++++++++timout->index:%ld tm:%ld,%p\r\n",_ptEventTimer->u64Index,_ptEventTimer->u64Expire,ptEventCtl);
	EVIO_EventTimer_Init(_ptEventTimer,1000,Main_TimerOut_Handle,ptMainServer);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);

	#if 1
	char *str = "Main Timer Timout SEND OK";
    uint8_t *dat = nn_allocmsg(100, 0);
	printf("dat===%p,%p\n",dat,ptMainServer);
    if (NULL != dat) {
         memcpy(dat, str, strlen(str));
	 	 int ret = nn_send(ptMainServer->tMainServerEventLoop.tNanoMsgFdsUDP.iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
		 printf("ret====%d\n",ret);
	}
	#endif
	
}



void Main_Handle_UDPNanomsgEvent(T_EVENT_CTL *_ptEventCtl, T_MAIN_NANOMSGFDS *_ptMainNanoMsgFds)
{ 
	uint8_t *dat = NULL;
	//uint32_t bytes = 0;
	T_MAINSERVER *ptMainServer = _ptMainNanoMsgFds->pvArg;
	//printf("handle udp event....\r\n");
	uint32_t bytes = nn_recv(_ptMainNanoMsgFds->iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) {
			printf("+++++++++++++++++++++%s\n",dat);
			nn_freemsg(dat);
	}
}

void Main_EventLoop_Handle(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_MAIN_NANOMSGFDS *ptMainNanoMsgFds = _pvArg;
	//T_MAINSERVER *ptMainServer = _ptMainNanoMsgFds->pvArg;
    printf("+++++++++++++++++++++++++++++++Main_EventLoop_Handle\n");
    switch (_eType) {
	    case E_EV_READ:
			printf("ev.....read\n");
	        ptMainNanoMsgFds->pfCallBack(_ptEventCtl, ptMainNanoMsgFds);
	        break;
		case E_EV_WRITE:
			printf("Main loop write event, unexpected\n");
			break;
		case E_EV_ERROR:
			printf("Main loop error event, unexpected\n");
			break;
	    default:
			printf("Main loop unknow event, unexpected\n");
    }
}

int32_t Main_NanomsgSocket_Init(T_MAINSERVER *_ptMainServer)
{
	size_t size = sizeof(size_t);
	T_MAINSERVEREVENTLOOP *ptMainServerEventLoop = &_ptMainServer->tMainServerEventLoop;
	
	ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd)
	{
		printf("+++main_loop nn_socket error...\n");
		return -1;
	}
	if (-1 == nn_bind(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd, "inproc://udp<->main"))
	{
		nn_close(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd);
		printf("+++main_loop nn_bind error...\n");
		return -1;
	}
	
	if (-1 == nn_getsockopt(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd, NN_SOL_SOCKET, NN_RCVFD, (char *)&ptMainServerEventLoop->tNanoMsgFdsUDP.iSysFd, &size))
	{
     	nn_close(ptMainServerEventLoop->tNanoMsgFdsUDP.iNanomsgFd);
		printf("+++main_loop nn_getsockopt error...\n");
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
		 printf("+++ptMainServerEventLoop->ptEventCtl error\n");
		 Main_NanomsgSocket_Close(_ptMainServer);
		 return -1;
	 }
	
	#if 1
    ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd = EVIO_EventFd_Add(ptMainServerEventLoop->ptEventCtl,\
													  ptMainServerEventLoop->tNanoMsgFdsUDP.iSysFd, \
													  Main_EventLoop_Handle, &ptMainServerEventLoop->tNanoMsgFdsUDP);//add the event fd 
	if (NULL == ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd)
	{
		printf("+++ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd error\n");
		EVIO_EventCtl_Free(ptMainServerEventLoop->ptEventCtl);
		Main_NanomsgSocket_Close(_ptMainServer);
		return -1;
	}
	ptMainServerEventLoop->tNanoMsgFdsUDP.pfCallBack= Main_Handle_UDPNanomsgEvent;//UDP enventloop main server
    ptMainServerEventLoop->tNanoMsgFdsUDP.pvArg = _ptMainServer;
	
	 EVIO_Event_Watch_Read(ptMainServerEventLoop->ptEventCtl, ptMainServerEventLoop->tNanoMsgFdsUDP.ptEventFd);
	 #endif
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
int main( void )
{
     int i =0;
	 int ret =0;
	 T_MAINSERVER tmainServer;
	 pthread_t tThreadUDP;
	 
	 ret = Main_Loop_Init(&tmainServer);
	 if(-1 == ret)
	 {
		printf("+++main loop init... error\n");
		return -1;
	 }
	 sleep(1);
     tmainServer.iThread_bStartCnt = 0;
	 pthread_mutex_init(&tmainServer.tThread_StartMutex, NULL);
	 pthread_cond_init(&tmainServer.tThread_StartCond, NULL);
	 ret =pthread_create(&tThreadUDP,NULL,UDP_SERVICE_Thread_Handle,&tmainServer);

	 if(-1 == ret)
	 {
		printf("+++udp pthread create... error\n");
		Main_Loop_Del(&tmainServer);
		return -1; 
	 }

	 while (tmainServer.iThread_bStartCnt < 1) {
        pthread_mutex_lock(&tmainServer.tThread_StartMutex);
        pthread_cond_wait(&tmainServer.tThread_StartCond, &tmainServer.tThread_StartMutex);
        pthread_mutex_unlock(&tmainServer.tThread_StartMutex);
        printf("pthread isstart cnt is %d\n", tmainServer.iThread_bStartCnt);    
  	}
	  printf("++++++++tmainServer.tMainServerEventLoop.tNanoMsgFdsUDP.iNanomsgFd:%d\n",tmainServer.tMainServerEventLoop.tNanoMsgFdsUDP.iNanomsgFd);
     while(1)
     {
		
        EVIO_EventCtlLoop_Start(tmainServer.tMainServerEventLoop.ptEventCtl);
		#if 0
		 uint8_t *dat = NULL;
		uint32_t bytes = nn_recv(tmainServer.tMainServerEventLoop.tNanoMsgFdsUDP.iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
		if (-1 != bytes) {
			printf("++++++++++++++++++++++111111111111111111111%s\n",dat);
		}
		#endif
     }
  return 0;
}