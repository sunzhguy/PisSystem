/*
 * @Descripttion: 车通信总线管理器
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 14:47:13
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 09:37:27
 */


#ifndef  DEViCE_STATUS_H
#define  DEVICE_STATUS_H
#include <stdint.h>
#include "../evio/evio.h"
#include "../timer/evtimer.h"

#define DEV_STATUS_OK				(1)
#define DEV_STATUS_ERROR			(2)
#define DEV_STATUS_NO_DEV		    (3)
//设备类型
#define DEV_TYPE_PISC	   (0x01)     //中央控制器
#define DEV_TYPE_DCP	   (0x02)     //广播控制盒
#define DEV_TYPE_TMS	   (0x03)    //TMS MVB 车辆接口单元
#define DEV_TYPE_RECON	   (0x04)    //重连
#define DEV_TYPE_RECORDER  (0x05)    //录音
//#define RESERVE	       (0x06)  
#define DEV_TYPE_DMP	   (0x07)    //动态地图
#define DEV_TYPE_EHP	   (0x08)    //紧急报警器
#define DEV_TYPE_LCU_LED   (0x09)    //客室广告屏 媒体解码器用0x09
#define DEV_TYPE_HEAD_LED  (0x0A)    //车头屏
#define DEV_TYPE_AMP	   (0x0B)   //功率放大器
#define DEV_TYPE_SWITCH	   (0x0C)
#define DEV_TYPE_PTU	   (0x0D)  //PTU



void    DEV_STATUS_InitDevValid(uint8_t _u8DevType);
//void DEV_STATUS_AddDev(uint8_t _u8DevType,uint8_t _u8DevId, uint8_t _u8Status);
uint8_t DEV_STATUS_GetDevStatus(uint8_t _u8DevType,uint8_t _u8DevId);
void    DEV_STATUS_SetDevStatus(uint8_t _u8DevType,uint8_t _u8DevId, uint8_t _u8Status);

void    DEV_STATUS_TimerOut_Handle(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg);


#endif