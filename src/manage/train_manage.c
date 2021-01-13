/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-10 09:32:25
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-13 08:59:16
 */


#include "train_manage.h"
#include "sys_service.h"
#include "../teminal_device/pisc_local.h"
#include "teminal_device/head_led.h"
#include "teminal_device/lcu_led.h"
#include "broadcast.h"
#include "driver/led.h"
void   TrainManage_Timer500msCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
    T_TRAIN_SYSTEM* ptTrainSystem = _pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;
    int iRet = 0;
    //printf("Send ProcessData.....\r\n");
    PISC_LOCAL_SendProcessData();
    BROADCAST_GetConfigPriority();
	EVIO_EventTimer_Init(_ptEventTimer,500,TrainManage_Timer500msCallBack,ptTrainSystem);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);
    LED_Toggle(LED_SYS_RUN);
    #if 0
    LED_Toggle(LED_MANUAL);
    LED_Toggle(LED_TMS);
    LED_Toggle(LED_ATC);
    LED_Toggle(LED_OCC);
    LED_Toggle(LED_ACTIVE);
    LED_Toggle(LED_MIC);
    LED_Toggle(LED_ERR);
    #endif

}


void   TrainManage_Timer1sCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
   
    T_TRAIN_SYSTEM* ptTrainSystem = _pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;

    //printf("Send Local Time.....\r\n");
    if(PISC_LOCAL_GetMasterFlag())
    {
        PISC_LOCAL_SendTime();
    }
	EVIO_EventTimer_Init(_ptEventTimer,1000,TrainManage_Timer1sCallBack,ptTrainSystem);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);

    
}

void   TrainManage_Timer5sCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{

    uint8_t i=0;
    T_TRAIN_SYSTEM* ptTrainSystem = _pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;
    if(PISC_LOCAL_GetMasterFlag())
    {
        
        //发送车头屏数据显示
        HEAD_LED_ContentSend();

        //发送客室数据显示
        PISC_LOCAL_SendTime();

        //客室屏滚动速度
        LCU_LED_RollSecondSet();

        //广播优先级设置

        //BROADCAST_SendPriority(); need complete.....
        //printf("5ms train mange....\r\n");
    }
	EVIO_EventTimer_Init(_ptEventTimer,5000,TrainManage_Timer5sCallBack,ptTrainSystem);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);


}
