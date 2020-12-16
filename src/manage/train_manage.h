/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-10 09:32:33
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-11 11:09:55
 */
#ifndef TRAIN_MANAGE_H
#define TRAIN_MANAGE_H


#define BROADCAST_SET_VOL  (0x0002)
#define BROADCAST_SET_PRI  (0X0007)
#define BROADCAST_SET_TIME (0x000A)


#define  CAB_NUM_MAX  (2)
#define  LCU_NUM_MAX  (8)
#include "include/general.h"
#include "../timer/evtimer.h"


void   TrainManage_Init(void);
void   TrainManage_Timer500msCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg);
void   TrainManage_Timer1sCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg);
void   TrainManage_Timer5sCallBack(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg);

#endif