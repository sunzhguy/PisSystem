/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-11 10:02:22
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-11 17:35:53
 */


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include "ioinput_manage.h"
#include "../driver/gpio.h"

T_IOINPUT_EVCTL sgtIOInputEvCtl;


#define IO_INPUT_MAXNUM    6

typedef void (*PF_IOINPUT_CALLBACK)(void *,uint32_t,uint8_t);

typedef struct
{
    int      iFd;
    uint32_t iGpioPin;
    uint32_t iTrigMode;
    uint8_t  u8IOLevel;
    uint8_t  u8IOLevelBakup;
    uint8_t  u8CheckCnt;//防抖检测次数
    uint8_t  u8CheckMax;//防抖检测最大值
    PF_IOINPUT_CALLBACK pfCallBack;

}T_IOINPUT_MANAGE;

T_IOINPUT_MANAGE gtIOInputMangeTable[IO_INPUT_MAXNUM] ={{0x00}};

static uint8_t sgU8IOTableiNum = 0;

static void _IOInput_AddManageTableInit(
  struct pollfd *_ptPollFdSet,
  uint32_t _iGpioPin,
  uint32_t _iTrigMode,
  uint8_t  _u8IOlevel,
  uint8_t  _u8IOLevelBackup,
  uint8_t  _u8CheckCnt,
  uint8_t  _u8CheckMax,
  void * _pfCallBack
)
{
  int iFd = 0;
  if(sgU8IOTableiNum >= IO_INPUT_MAXNUM)
  {
      printf("To many IO manage,return");
      return;
  }
   GPIO_Init(_iGpioPin,1);
   GPIO_IO_TrigCtl(_iGpioPin,_iTrigMode);
   iFd = GPIO_IO_GetReadFd(_iGpioPin);
   if(iFd <0)
   {
        printf("GPIO read Fd...Error\r\n");
        return ;
   }

  _ptPollFdSet[sgU8IOTableiNum].fd                    = iFd;
  _ptPollFdSet[sgU8IOTableiNum].events                = POLLPRI;
  gtIOInputMangeTable[sgU8IOTableiNum].iFd            = iFd;
  gtIOInputMangeTable[sgU8IOTableiNum].iGpioPin       = _iGpioPin;
  gtIOInputMangeTable[sgU8IOTableiNum].iTrigMode      = _iTrigMode;
  gtIOInputMangeTable[sgU8IOTableiNum].u8IOLevel      = _u8IOlevel;
  gtIOInputMangeTable[sgU8IOTableiNum].u8IOLevelBakup = _u8IOLevelBackup;
  gtIOInputMangeTable[sgU8IOTableiNum].u8CheckCnt     = _u8CheckCnt;
  gtIOInputMangeTable[sgU8IOTableiNum].u8CheckMax     = _u8CheckMax;
  gtIOInputMangeTable[sgU8IOTableiNum].pfCallBack     = _pfCallBack;
  sgU8IOTableiNum++;
    
}



void _IOInput_HandleNanoMsg(T_EVENT_CTL *_ptEventCtl, T_IONANOMSG_EVFDS *_ptNanomsgEvFds)
{
	T_IOINPUT_EVCTL *ptIOInputNetCtl = _ptNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer     = ptIOInputNetCtl->ptServer;

	uint8_t *dat = NULL;
	uint32_t bytes = nn_recv(_ptNanomsgEvFds->iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) {
			zlog_debug(ptMainServer->ptZlogCategory,"########IOInput->Main Nanomsg+++++++++++++++%s\n",dat);
            
			nn_freemsg(dat);
	}
}

static void _IOInput_NanoMsg_CallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_IONANOMSG_EVFDS *ptNanomsgEvFds          = (T_IONANOMSG_EVFDS*)_pvArg;
	T_IOINPUT_EVCTL  *ptIOInputEvCtl           = ptNanomsgEvFds->pvArg;
	T_MAINSERVER    *ptMainServer              = ptIOInputEvCtl->ptServer;

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

static int32_t _IOInput_NanoMsgAndEventCtlInit(T_IOINPUT_EVCTL *_ptIOInputEventCtl)
{
	T_MAINSERVER    *ptMainServer     = _ptIOInputEventCtl->ptServer;
	size_t size = sizeof(size_t);
    _ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd= nn_socket(AF_SP, NN_PAIR);
	if (-1 == _ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_socket failed\n");
		return -1;
	}
	if (-1 == nn_connect(_ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd, "inproc://main<->ioinput"))
	{
	  zlog_error(ptMainServer->ptZlogCategory,"++++BDAudio nn_bind failed\n");
      nn_close(_ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd);
	  return -1;
	}
	
	if (-1 == nn_getsockopt(_ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd, NN_SOL_SOCKET, NN_RCVFD,\
							 (char *)&_ptIOInputEventCtl->tNanomsgEvFds.iSysFd, &size))
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++nn_getsockopt failed\n");
		nn_close(_ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd);
		return -1;	 
	}


	_ptIOInputEventCtl->ptEventCtl = EVIO_EventCtl_Create();
	
	if (NULL == _ptIOInputEventCtl->ptEventCtl)
	{
		zlog_error(ptMainServer->ptZlogCategory,"++++EVIO_EventCtl_Create failed\n");
		close(_ptIOInputEventCtl->tNanomsgEvFds.iSysFd);
		nn_close(_ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd);
		return -1;
	}
    _ptIOInputEventCtl->tNanomsgEvFds.pfEventCallBack = _IOInput_HandleNanoMsg;
    _ptIOInputEventCtl->tNanomsgEvFds.pvArg           = _ptIOInputEventCtl;
    _ptIOInputEventCtl->tNanomsgEvFds.ptEventFd       = EVIO_EventFd_Add(_ptIOInputEventCtl->ptEventCtl, \
													 _ptIOInputEventCtl->tNanomsgEvFds.iSysFd,\
													 _IOInput_NanoMsg_CallBack, 
													 &_ptIOInputEventCtl->tNanomsgEvFds);//add the event fd 
	if (NULL == _ptIOInputEventCtl->tNanomsgEvFds.ptEventFd)
	{
		EVIO_EventCtl_Free(_ptIOInputEventCtl->ptEventCtl);
		close(_ptIOInputEventCtl->tNanomsgEvFds.iSysFd);
		nn_close(_ptIOInputEventCtl->tNanomsgEvFds.iNanoMsgFd);
		zlog_error(ptMainServer->ptZlogCategory,"_ptIOInputEventCtl tNanoMsgUDPNet.ptEventFd failed\n");
		return -1;
	}
	EVIO_Event_Watch_Read(_ptIOInputEventCtl->ptEventCtl, _ptIOInputEventCtl->tNanomsgEvFds.ptEventFd);
	return 0;
	
}


static void _IOInput_NanoMsgClose(T_IONANOMSG_EVFDS *_ptNanomsgEvFds)
{
	if(_ptNanomsgEvFds)
	{
		close(_ptNanomsgEvFds->iSysFd);
		nn_close(_ptNanomsgEvFds->iNanoMsgFd);
	}
}



void IOInput_IOInputHandle(T_IOINPUT_EVCTL *ptIOInputCtl,uint32_t _iGpioPin,uint8_t _u8IOLevel)
{
    printf("_iGpioPin:%d,_u8IOLevel:%d\r\n",_iGpioPin,_u8IOLevel);
    
}


void *IOInput_InputThreadCheck(void *_pvArg)
{
    int iRet = 0;
    int i;
    uint8_t ioLevel;
    char acReadBuf[8]={0x00};
    struct pollfd tIOPollFdSet[IO_INPUT_MAXNUM];
    T_IOINPUT_EVCTL *ptIOInputEvCtl = (T_IOINPUT_EVCTL*)_pvArg;
    memset((void*)tIOPollFdSet,0,sizeof(tIOPollFdSet));
    _IOInput_AddManageTableInit(&tIOPollFdSet,IO_OCC_ACT6,MODE_BOTH,1,1,0,1,IOInput_IOInputHandle);

    while(1)
    {
        iRet = poll(tIOPollFdSet,sgU8IOTableiNum,50);//50ms
        if(iRet > 0) //0or -1 error
        {
            for(i = 0; i< sgU8IOTableiNum;++i)
            {
                if(tIOPollFdSet[i].revents & POLLPRI)
                {
                    memset(acReadBuf,0,8);
                    int len = read(tIOPollFdSet[i].fd,acReadBuf,8);
                    acReadBuf[7] = '\0';
                    lseek(tIOPollFdSet[i].fd,0,SEEK_SET);
                    ioLevel = (acReadBuf[0]=='1')? 1 : 0;
                    printf("ioLevel:%d\r\n",ioLevel);
                    if(len)
                    {
                        if(gtIOInputMangeTable[i].iTrigMode == MODE_BOTH)
                        {
                            usleep(20000);
                            if(gtIOInputMangeTable[i].u8IOLevelBakup != ioLevel)
                            {
                                //防抖在读一次
                                #if 1
                                memset(acReadBuf,0,8);
                                len = read(tIOPollFdSet[i].fd,acReadBuf,8);
                                acReadBuf[7] = '\0';
                                lseek(tIOPollFdSet[i].fd,0,SEEK_SET);
                                ioLevel = (acReadBuf[0]=='1')? 1 : 0;
                                printf("ioLevelxxx:%d\r\n",ioLevel);
                                #endif
                                if(ioLevel != gtIOInputMangeTable[i].u8IOLevelBakup)
                                {
                                    gtIOInputMangeTable[i].u8IOLevelBakup = ioLevel;
                                    gtIOInputMangeTable[i].pfCallBack(_pvArg,gtIOInputMangeTable->iGpioPin,ioLevel);
                                }
                            } 
                        }
                    }
                }
            }
        }
    }
}


void *IOInput_Thread_Handle(void *_pvArg)
{

    int iRet = 0;
    pthread_t tPthreadIOInput;
    T_MAINSERVER *ptMainServer   = (T_MAINSERVER *) _pvArg;
    sgtIOInputEvCtl.ptServer     =  ptMainServer;
    sgtIOInputEvCtl.ptEventCtl   = NULL;
    T_EVENT_FD *ptEventFd        =NULL;
    int io_fd;
    zlog_info(ptMainServer->ptZlogCategory,"IOInput_Thread start....\r\n");
	if(-1 == _IOInput_NanoMsgAndEventCtlInit(&sgtIOInputEvCtl))
	{
		zlog_error(ptMainServer->ptZlogCategory,"IOInput_Thread_Handle Event Ctl Init Failed\n");
		abort();
	}
    
    if(sgtIOInputEvCtl.ptEventCtl == NULL)
	{
	  zlog_error(ptMainServer->ptZlogCategory,"IOInput Thread ptEventCtl Create error\n");
	  abort();
	}
   
    pthread_create(&tPthreadIOInput,NULL,IOInput_InputThreadCheck,&sgtIOInputEvCtl);
    pthread_mutex_lock(&ptMainServer->tThread_StartMutex);
	++ptMainServer->iThread_bStartCnt;
    
	pthread_cond_signal(&ptMainServer->tThread_StartCond);
	pthread_mutex_unlock(&ptMainServer->tThread_StartMutex);
    
    while(1)
     {
        EVIO_EventCtlLoop_Start(sgtIOInputEvCtl.ptEventCtl);
     }
    _IOInput_NanoMsgClose(&sgtIOInputEvCtl.tNanomsgEvFds);
	EVIO_EventCtl_Free(sgtIOInputEvCtl.ptEventCtl);
	return NULL;  


}