/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-10 09:32:25
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 15:12:32
 */


#include "train_manage.h"
#include "sys_service.h"
#include "../teminal_device/pisc_local.h"
#include "teminal_device/head_led.h"
#include "teminal_device/lcu_led.h"
#include "broadcast.h"
void   TrainManage_Timer500msCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{

    uint8_t i=0;
    T_TRAIN_SYSTEM* ptTrainSystem = _pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;

    //printf("Send ProcessData.....\r\n");
    PISC_LOCAL_SendProcessData();
    BROADCAST_GetConfigPriority();
	EVIO_EventTimer_Init(_ptEventTimer,500,TrainManage_Timer500msCallBack,ptTrainSystem);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);


}


void   TrainManage_Timer1sCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
    uint8_t i=0;
    T_TRAIN_SYSTEM* ptTrainSystem = _pvArg;
    T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;

    printf("Send Local Time.....\r\n");
    if(PISC_LOCAL_GetMasterFlag())
    {
        PISC_LOCAL_SendTime();
    }
	EVIO_EventTimer_Init(_ptEventTimer,1000,TrainManage_Timer500msCallBack,ptTrainSystem);
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

        //BROADCAST_SendPriority();
        //printf("5ms train mange....\r\n");
    }
	EVIO_EventTimer_Init(_ptEventTimer,5000,TrainManage_Timer5sCallBack,ptTrainSystem);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);


}
