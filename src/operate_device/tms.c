/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 11:05:48
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-09 17:23:11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tms.h"
#include "pisc.h"
#include "../manage/dev_status.h"
#include "../manage/dev_master.h"
#include "../lib/pis_pack.h"
#include "../process_matrix/pis_datproc_matrix.h"
#include "../teminal_device/pisc_local.h"
#include "../teminal_device/head_led.h"
#include "../include/pis_config.h"
#include "../manage/broadcast.h"
#include "../driver/systime.h"
#include "../udp_service.h"
#include "driver/led.h"

//TMS 上下行
#define  TMS_RUNDIR_UP     (1)
#define  TMS_RUNDIR_DOWN   (2)


//开关门  左右侧

#define   TMS_DOOR_LEFT      1
#define   TMS_DOOR_RIGHT     2
#define   TMS_DOOR_BOTH_SIDE 3



//Tms 设置时间


static  void  _TMS_TimeSet (T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_CloseDoor_StatusSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_PreBroadCast_TriggerSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_ArrBroadCast_TriggerSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_StopBroadCast_TriggerSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_RecivAFrameDataPackHandle(uint8_t *pcBuf);
static  void  _TMS_StationInfoSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_RunDirectionSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_EmergencyBroadCastTrigger(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static  void  _TMS_CycleBroadCastTrigger(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);


static T_DEV_BUSMSGDATA_PROC gtTmsMasterMsgDataProc[] = 
{
	{PIS_PACK_HEADINFO_SIZE+3,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_RunDirectionSet		},	//DCP上下行切换
	{PIS_PACK_HEADINFO_SIZE+4,		0,	B_8BIT,	    WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_StationInfoSet			},	//当前站变更
	{PIS_PACK_HEADINFO_SIZE+5,		0,	B_8BIT,	    WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_StationInfoSet			},	//下一站变更
	{PIS_PACK_HEADINFO_SIZE+6,		0,	B_8BIT,	    WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_StationInfoSet			},	//起始站变更
	{PIS_PACK_HEADINFO_SIZE+7,		0,	B_8BIT,	    WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_StationInfoSet			},	//终点站变更
    
    {PIS_PACK_HEADINFO_SIZE+13,		0,  B_16BIT,    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_TimeSet				},	//year
	{PIS_PACK_HEADINFO_SIZE+15,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_TimeSet				},	//mon
	{PIS_PACK_HEADINFO_SIZE+16,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_TimeSet				},	//day
	{PIS_PACK_HEADINFO_SIZE+17,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_TimeSet				},	//hour
	{PIS_PACK_HEADINFO_SIZE+18,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_TimeSet				},	//min
	{PIS_PACK_HEADINFO_SIZE+19,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_TimeSet				},	//sec

	{PIS_PACK_HEADINFO_SIZE+21,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_CloseDoor_StatusSet	},	//关门状态设置
	
	{PIS_PACK_HEADINFO_SIZE+29, 	0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_PreBroadCast_TriggerSet},	//预到站广播触发
	{PIS_PACK_HEADINFO_SIZE+30,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_ArrBroadCast_TriggerSet},	//到站广播触发
  //{PIS_PACK_HEADINFO_SIZE+30,		0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_StopBroadCast_TriggerSet},	//当前站变更	
	{PIS_PACK_HEADINFO_SIZE+31,     0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_StopBroadCast_TriggerSet},	//停站广播触发

	{PIS_PACK_HEADINFO_SIZE+34,	0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_EmergencyBroadCastTrigger},	//紧急广播触发
	{PIS_PACK_HEADINFO_SIZE+37,	0,	B_8BIT,	    UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)	_TMS_CycleBroadCastTrigger	  },	//循环广播触发
	{PIS_PACK_HEADINFO_SIZE, 		0,	B_8BIT,	    WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK) _TMS_RecivAFrameDataPackHandle},	
    
	{0,								0,	0,			0,			0,	NULL														  }
};

//从状态处理矩阵
static T_DEV_BUSMSGDATA_PROC gtTmsSlaveMsgDataProc[] = 
{	
	{PIS_PACK_HEADINFO_SIZE, 		0,	B_4BIT,		WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_TMS_RecivAFrameDataPackHandle},	
	{0,								0,	0,			0,			0,	NULL														 }
};
//状态基索引
static T_MODE_MATRIX  gtTmsModeMatixProcTable[]=
{
	{PISC_STATUS_MASTER,  gtTmsMasterMsgDataProc},			//主机状态处理矩阵
	{PISC_STATUS_SLAVE,   gtTmsSlaveMsgDataProc },		    //备机状态处理矩阵
	{0, 				  NULL,},
};

//状态基索引
static T_CMD_MATRIX  gtTmsCmdMatixProcTable[]=
{
	{CTRL_CMD_DEFAULT,  gtTmsModeMatixProcTable},			//主/从模式处理矩阵
	{0, NULL,},
};

//预到站播放标志
static uint8_t _gu8PreBroadCastFlag = 0;
//到站播放标志
static uint8_t _gu8ArrBroadCastFlag = 0;

void  TMS_Init(void)
{
   //初始化设备有效性
   DEV_STATUS_InitDevValid(DEV_TYPE_TMS);
   printf("Tms Init......\r\n");
   PisDataProc_Matrix_AddDeviceToMatrixTable(DEV_TYPE_TMS,gtTmsCmdMatixProcTable);
	
}

static  void  _TMS_CloseDoor_StatusSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{
	T_TMS_RECVPACKT *ptTmsPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;

	if(PISC_LOCAL_GetMasterFlag() != MASTER_STATUS)
	  return;

	if(ptTmsPacket->u8ClosedDoorSta)//关门状态---已关门
	{
		PISC_LOCAL_SetOpenLeftDoorFlag(0);
		PISC_LOCAL_SetOpenRightDoorFlag(0);
	}		
	
}
static void _TMS_StationInfoSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{


	T_STATIONINFO  tStationInfo;
	uint8_t u8StationChangeFlag    = 0;
	uint8_t u8EndStationChangeFlag = 0;
	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}
	if(PISC_LOCAL_GetMasterFlag() != MASTER_STATUS)
	{
		return;
	}

	PISC_LOCAL_GetStationInfo(&tStationInfo);
	
	//站点改变，赋值
	if(tStationInfo.u8StartStation != ptTmsRecvPacket->u8StartStation)
	{
		tStationInfo.u8StartStation = ptTmsRecvPacket->u8StartStation;
		u8StationChangeFlag         = 1;
	}
	if(tStationInfo.u8EndStation != ptTmsRecvPacket->u8EndStation)
	{
		tStationInfo.u8EndStation = ptTmsRecvPacket->u8EndStation;
		u8StationChangeFlag       = 1;
		u8EndStationChangeFlag    = 1;
	}
	if(tStationInfo.u8NexStation != ptTmsRecvPacket->u8NexStation)
	{
		tStationInfo.u8NexStation = ptTmsRecvPacket->u8NexStation;
		u8StationChangeFlag = 1;
		//清开门侧
		
	}
	if(tStationInfo.u8CurStation != ptTmsRecvPacket->u8NexStation)
	{
		tStationInfo.u8CurStation = ptTmsRecvPacket->u8NexStation;
		u8StationChangeFlag       = 1;
	}
	if(u8StationChangeFlag)
	{
		//填充站点信息
		PISC_LOCAL_SetStationInfo(tStationInfo);

		if(u8EndStationChangeFlag)
		{
			HEAD_LED_ContentSet(LANGUAGE_C);
			HEAD_LED_ContentSet(LANGUAGE_E);
			//车头屏显示
			HEAD_LED_ContentSend();
			
		}
		
		//清标志
		_gu8PreBroadCastFlag = 0;
		_gu8ArrBroadCastFlag = 0;
	}

}

static void _TMS_RunDirectionSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{
	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	if( PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}

	if( PISC_LOCAL_GetMasterFlag() != MASTER_STATUS)
	{
		return ;
	}
	if(TMS_RUNDIR_UP == ptTmsRecvPacket->u8RunDir)
	{
		PISC_LOCAL_SetRunDir(PISC_DIR_UP);
	}else
	{
		PISC_LOCAL_SetRunDir(PISC_DIR_DOWN);
	}
	

}
static void _TMS_EmergencyBroadCastTrigger(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{

	uint16_t u16UrgentCodeId=0;
	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	printf("ptTmsRecvPacket->urgent_flag: %d, u16UrgentCodeId: %d\r\n",ptTmsRecvPacket->u8UrgentTrigFlag,ptTmsRecvPacket->u8UrgentCode);
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}
	if(PISC_LOCAL_GetMasterFlag() != MASTER_STATUS)
	{
		return ;
	}
	
	u16UrgentCodeId = ptTmsRecvPacket->u8UrgentCode;

	if(!ptTmsRecvPacket->u8UrgentTrigFlag)
	{
		if(BROADCAST_URGENT == BROADCAST_GetBroadCastType())
		{
			//BROADCAST_StopProcess(BROADCAST_URGENT);
			TMS_SendNanoMsgToBroadCast(BROADCAST_STOP,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_URGENT);
		}
		
	}
	if(ptTmsRecvPacket->u8UrgentTrigFlag)
	{
		if(u16UrgentCodeId)
		{
			BROADCAST_SetUrgentCode(u16UrgentCodeId);
			TMS_SendNanoMsgToBroadCast(BROADCAST_PLAY,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_URGENT);
			//BROADCAST_Process(_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_URGENT);
		}
	}

		
}




static void _TMS_TimeSet (T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{

	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	PISC_LOCAL_SetTime((uint8_t *)&ptTmsRecvPacket->tTmsTime,sizeof(T_TMSTIME));
	

}

void TMS_SendNanoMsgToBroadCast(uint8_t _u8OpType,uint8_t _u8DevType,uint8_t _u8DevId,uint8_t _u8BdType)
{
	uint8_t *dat = nn_allocmsg(8, 0);
    if (NULL != dat) {
		 dat[0] = TO_BROADCAST_NS;
		 dat[1] = MSG_TYPE_TMSUDP;
         dat[2] = _u8OpType;
		 dat[3] = _u8DevType;
		 dat[4] = _u8DevId;
		 dat[5] = _u8BdType;
		
	 	 UDP_SERVICE_SendNanoMsg(dat);
	}

}
static void _TMS_PreBroadCast_TriggerSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{

	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}
	if(PISC_LOCAL_GetMasterFlag() != MASTER_STATUS)
	{
		return ;
	}
	//printf("++++++++++++++++++++++++++++++++++++++==%d\r\n",ptTmsRecvPacket->u8PreBrdCastFlag);
	//预到站触发
	if(ptTmsRecvPacket->u8PreBrdCastFlag)
	{
		printf("+++++++++++PPPPPPPPPPPPPPPPPPPPPPPPP+++++++++++++\r\n");
		//预到站广播
		//BROADCAST_Process(_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_PRE);
		TMS_SendNanoMsgToBroadCast(BROADCAST_PLAY,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_PRE);
		PISC_LOCAL_SetPreFlag(1);
		PISC_LOCAL_SetArrFlag(0);

		if(TMS_DOOR_LEFT == ptTmsRecvPacket->u8DoorSide)
		{
			PISC_LOCAL_SetOpenLeftDoorFlag(1);
			PISC_LOCAL_SetOpenRightDoorFlag(0);
		}
		else if(TMS_DOOR_RIGHT == ptTmsRecvPacket->u8DoorSide)
		{
			PISC_LOCAL_SetOpenLeftDoorFlag(0);
			PISC_LOCAL_SetOpenRightDoorFlag(1);
		}
		else if(TMS_DOOR_BOTH_SIDE == ptTmsRecvPacket->u8DoorSide)
		{
			PISC_LOCAL_SetOpenLeftDoorFlag(1);
			PISC_LOCAL_SetOpenRightDoorFlag(1);
		}	
		else
		{
			PISC_LOCAL_SetOpenLeftDoorFlag(0);
			PISC_LOCAL_SetOpenRightDoorFlag(0);
		}		
	}

  printf("_TMS_PreBroadCast_TriggerSet\n");
}

static void _TMS_ArrBroadCast_TriggerSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{

	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}
	if(PISC_LOCAL_GetMasterFlag() != MASTER_STATUS)
	{
		return ;
	}

	if(ptTmsRecvPacket->u8ArrBrdcastFlag)
	{
		//BROADCAST_Process(_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_ARRIVE);
		TMS_SendNanoMsgToBroadCast(BROADCAST_PLAY,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_ARRIVE);
		PISC_LOCAL_SetPreFlag(0);
		PISC_LOCAL_SetArrFlag(1);	
	}

		printf("_TMS_ArrBroadCast_TriggerSet\n");
}
 static void _TMS_StopBroadCast_TriggerSet(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{

	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}
	if(ptTmsRecvPacket->u8ManulStopBrdCastFlag)
	{
		if(BROADCAST_PRE == BROADCAST_GetBroadCastType())
		{
			//BROADCAST_StopProcess(BROADCAST_PRE);
			TMS_SendNanoMsgToBroadCast(BROADCAST_STOP,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_PRE);
		}

		if(BROADCAST_ARRIVE == BROADCAST_GetBroadCastType())
		{
			//BROADCAST_StopProcess(BROADCAST_ARRIVE);
			TMS_SendNanoMsgToBroadCast(BROADCAST_STOP,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_ARRIVE);
		}
	}
	printf("_TMS_StopBroadCast_TriggerSet\n");
}


static void _TMS_CycleBroadCastTrigger(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{	

	T_TMS_RECVPACKT *ptTmsRecvPacket = (T_TMS_RECVPACKT*)_ptPisRecvPackDataFrame->acData;
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		return ;
	}
	//测试循环广播
	if(ptTmsRecvPacket->u16CycleBrdCastFlag)
	{	
		//pisc_set_urgent_trigger(urgent_no); //默认第一段紧急广播
		//BROADCAST_Process(_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_URGENT);
		TMS_SendNanoMsgToBroadCast(BROADCAST_PLAY,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_URGENT);
	}
	else
	{
		TMS_SendNanoMsgToBroadCast(BROADCAST_STOP,_ptPisRecvPackDataFrame->u8SrcDevType,_ptPisRecvPackDataFrame->u8SrcDevId,BROADCAST_URGENT);
		//BROADCAST_StopProcess(BROADCAST_URGENT);	
	}	

	printf("_TMS_CycleBroadCastTrigger\n");
}


static void _TMS_RecivAFrameDataPackHandle(uint8_t *pcBuf)
{	
	
	printf("_TMS_RecivAFrameDataPackHandle......\r\n");
	//printf("PISC Mode:%d,status:%d\n",PISC_LOCAL_GetWorkMode(),PISC_LOCAL_GetMasterFlag());
	 LED_Toggle(LED_TMS);
	if(PISC_LOCAL_GetWorkMode() != PISC_ATC_MODE)
	{
		printf("MODE++++++++++++%d\n",PISC_LOCAL_GetWorkMode());
		return ;
	}

	if(PISC_LOCAL_GetMasterFlag() != PISC_STATUS_MASTER)
	{
		printf("FLAG++++++++++++%d\n",PISC_LOCAL_GetMasterFlag());
		return ;
	}
	PisDataProc_Matrix_CompareUpdateProcessData((T_PIS_PACKDATAFRAME*)pcBuf);

}