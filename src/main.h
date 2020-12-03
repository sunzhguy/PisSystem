/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 09:04:27
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-03 11:31:20
 */ 

#ifndef _MAIN_H
#define _MAIN_H

#include "evio/evio.h"
#include "zlog/zlog.h"

#define ZLOGCONF					"/home/conf/zlog.conf"
#define SOFT_VER                    "V1.0.0"          

typedef struct _T_MAINSERVER 	  T_MAINSERVER;
typedef struct _T_MAIN_NANOMSGFDS T_MAIN_NANOMSGFDS;
struct _T_MAIN_NANOMSGFDS{
	int iSysFd;			/* sys fd*/
	int iNanomsgFd;			/* nanomsg fd */
	T_EVENT_FD *ptEventFd;
	void (*pfCallBack)(T_EVENT_CTL *, T_MAIN_NANOMSGFDS *);
	void *pvArg;
};


typedef struct  {
	T_EVENT_CTL *ptEventCtl;
	T_MAIN_NANOMSGFDS  tNanoMsgFdsUDP;//主服务循环 ---》UDP 与main loop 之间的通信
}T_MAINSERVEREVENTLOOP;

struct _T_MAINSERVER {
    T_EV_TIMER tEventTimer;//添加一个定时器
    T_MAINSERVEREVENTLOOP tMainServerEventLoop;//主服务循环
	int32_t iThread_bStartCnt; //启动server 服务线程数
	pthread_mutex_t tThread_StartMutex;
    pthread_cond_t tThread_StartCond;
	zlog_category_t *ptZlogCategory;
};

#endif