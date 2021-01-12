/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-07 15:35:18
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 16:24:44
 */
#include "sys_service.h"
#include "evio/evio.h"
#include "../manage/dev_status.h"
#include "main.h"
#include "../process_matrix/pis_datproc_matrix.h"
#include "../teminal_device/pisc_local.h"
#include "../operate_device/tms.h"
#include "train_manage.h"
#include "broadcast.h"
#include <pthread.h>
#include "mp3_decoder.h"
#include "../port_layer/fep_audio_port.h"
#include "../port_layer/audio_port_bd.h"
#include "../manage/ioinput_manage.h"
#include "driver/soundcard.h"


T_TRAIN_SYSTEM  tTrainSystem;
void *TrainSystemService_ThreadHandle(void *_pvArg)
{
    int ret = 0;
    pthread_t tThreadBroadCast;
    pthread_t tThreadMP3Decoder;
    T_MAINSERVER *ptMainServer = (T_MAINSERVER *) _pvArg;
    tTrainSystem.ptEventCtl    =  EVIO_EventCtl_Create();  //创建一个事件控制器 creator event contoler
    tTrainSystem.ptMainServer  = ptMainServer;
    if(NULL == tTrainSystem.ptEventCtl)
	 {
		 zlog_error(ptMainServer->ptZlogCategory,"+++Train System creator EventCtl Failed\n");
		 return NULL;
	 }
	
    PisDataProc_MatrixInit();
    PISC_LOCAL_ProcessDataInit();//本地数据初始化,设备号可能在后面的初始化中用到
    PISC_Init();
    TMS_Init();



    //启动声卡录音线程
    ret =pthread_create(&tThreadBroadCast,NULL,SOUNDCARD_RecordService_ThreadHandle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"soundcard thread pthread create error\n");
	 }
    
      //启动声卡播放线程
    ret =pthread_create(&tThreadBroadCast,NULL,SOUNDCARD_PlayPCMService_ThreadHandle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"soundcard thread pthread create error\n");
	 }
    

    //状态检测
    EVIO_EventTimer_Init(&tTrainSystem.tEventTimer,1000,DEV_STATUS_TimerOut_Handle,&tTrainSystem);
	EVIO_EventTimer_Start(tTrainSystem.ptEventCtl,&tTrainSystem.tEventTimer);

    //列车管理 显示屏 动态地图 广播优先级  音量控制 越战控制
    EVIO_EventTimer_Init(&tTrainSystem.tTtrain500msEvTimer,500,TrainManage_Timer500msCallBack,&tTrainSystem);
	EVIO_EventTimer_Start(tTrainSystem.ptEventCtl,&tTrainSystem.tTtrain500msEvTimer);

    EVIO_EventTimer_Init(&tTrainSystem.tTtrain1sEvTimer,1000,TrainManage_Timer1sCallBack,&tTrainSystem);
	EVIO_EventTimer_Start(tTrainSystem.ptEventCtl,&tTrainSystem.tTtrain1sEvTimer);

    EVIO_EventTimer_Init(&tTrainSystem.tTtrain5sEvTimer,5000,TrainManage_Timer5sCallBack,&tTrainSystem);
	EVIO_EventTimer_Start(tTrainSystem.ptEventCtl,&tTrainSystem.tTtrain5sEvTimer);

    //启动广播线程
    ret =pthread_create(&tThreadBroadCast,NULL,BROADCAST_Service_ThreadHandle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"SYS service thread pthread create error\n");
	 }

    //启动MP3解码线程
    ret =pthread_create(&tThreadMP3Decoder,NULL,MP3_Decoder_Service_ThreadHandle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"SYS service thread pthread create error\n");
	 }

  
    //功放音频同步UDP接收线程

     ret =pthread_create(&tThreadMP3Decoder,NULL,FEP_Audio_SyncThread_Handle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"SYS service thread pthread create error\n");
	 }

     //广播音频网络层初始化
    ret =pthread_create(&tThreadMP3Decoder,NULL,BDAudio_Thread_Handle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"SYS service thread pthread create error\n");
	 }

    //IOInput初始化
    ret =pthread_create(&tThreadMP3Decoder,NULL,IOInput_Thread_Handle,ptMainServer);
	 if(-1 == ret)
	 {
		zlog_error(ptMainServer->ptZlogCategory,"SYS service thread pthread create error\n");
	 }
     
    pthread_mutex_lock(&ptMainServer->tThread_StartMutex);
	++ptMainServer->iThread_bStartCnt;
    zlog_info(ptMainServer->ptZlogCategory,"+++++++++++++++++<%d>\r\n",ptMainServer->iThread_bStartCnt);
	pthread_cond_signal(&ptMainServer->tThread_StartCond);
	pthread_mutex_unlock(&ptMainServer->tThread_StartMutex);


    while(1)
     { 
        EVIO_EventCtlLoop_Start(tTrainSystem.ptEventCtl);
     }
    EVIO_EventCtl_Free(tTrainSystem.ptEventCtl);
    return NULL;
}


