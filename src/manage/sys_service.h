/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-07 15:35:11
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 11:45:23
 */
#ifndef _SYS_SERVICE_H
#define _SYS_SERVICE_H
#include "../main.h"

typedef struct {
T_EVENT_CTL *ptEventCtl;//事件控制器
T_EV_TIMER   tEventTimer;//添加一个定时器
T_EV_TIMER   tTtrain500msEvTimer;//添加一个定时器
T_EV_TIMER   tTtrain1sEvTimer;//添加一个定时器
T_EV_TIMER   tTtrain5sEvTimer;//添加一个定时器
T_MAINSERVER *ptMainServer;
}T_TRAIN_SYSTEM;

void *TrainSystemService_ThreadHandle(void *_pvArg);
#endif