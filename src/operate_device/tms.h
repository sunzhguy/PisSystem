/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 11:05:56
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 10:37:07
 */

#ifndef PISC_TMS_H
#define PISC_TMS_H
#include "../include/general.h"


typedef struct{

  uint16_t u16Year;
  uint8_t  u8Month;
  uint8_t  u8Day;
  uint8_t  u8Hour;
  uint8_t  u8Min;
  uint8_t  u8Sec;
}__attribute((packed))T_TMSTIME;




//接收数据包结构,60字节
typedef struct
{
	uint16_t    u16TrainNo;//列车编号
	uint8_t     u8LineNo; //线号
	uint8_t     u8RunDir; // 10-11 上下行方案
	uint8_t     u8CurStation; //当前站号
	uint8_t     u8NexStation; //12-13 
	uint8_t     u8StartStation; // 14-15
	uint8_t     u8EndStation; // 16-17
	uint16_t    u16DistToCurStation; //12-13
	uint16_t    u16DistToNextStation; // 14-15
	struct//28-29
	{
		 uint8_t    b8CurStatnValid:1; //28	 
		 uint8_t    b8NextStatnValid:1; //上行
		 uint8_t    b8StatStatnValid:1; //28	 
		 uint8_t    b8EndStatnValid:1; //上行
		 uint8_t    b8DistToCurStatnValid:1; //12-13
		 uint8_t    b8DistToNextStatnValid:1; // 14-15
		 uint8_t    b8ATCValid:1; //
		 uint8_t    b8TimeValid:1; //29
	}__attribute((packed))tValidSignal;

	T_TMSTIME   tTmsTime;
	uint8_t     u8DoorSide; // 21-23
	uint8_t     u8DoorOpen; // 21-23
	uint8_t     u8ClosedDoorSta; // 21-23
	uint8_t     u8TrigSig_30km; // 24-25
	uint8_t     u8TrigSig_5km; // 24-25
	uint8_t     u8Sig_TrainStop; // 24-25
	uint16_t    u16GpsTranPostion; // 24-25
	uint8_t     u8ManulFlag;    // 24-25
	uint8_t     u8PreBrdCastFlag; // 24-25
	uint8_t     u8ArrBrdcastFlag; // 24-25
	uint8_t     u8ManulStopBrdCastFlag; // 24-25
	uint16_t 	u8UrgentCode; // 18-19
	uint8_t 	u8UrgentTrigFlag; // 24-25
	uint16_t 	u16TrainSpeed; // 24-25
	uint8_t 	u16CycleBrdCastFlag; // 18-19
	uint8_t 	u8ProjectID;
	uint8_t 	u8SkipBrdCastTrigFlag;
	uint8_t 	acSkipStaionCode[8];

	uint8_t 	u8CabDoor1Release;
	//uint8 lcu_door_release[LCU_NUM_MAX];
	uint8_t 	u8CabDoor2Release;
	struct//60
	{
		 uint8_t bTestBrdCastFlag:1; 
		 uint8_t bTestHeadLedFlag:1;
		 uint8_t bTestLCULedFlag:1;
		 uint8_t bTestDmpFlag:1;
		 uint8_t bReserve:4;
	}__attribute((packed))tTestFlag;
}__attribute((packed))T_TMS_RECVPACKT;




void  TMS_Init(void );

#endif