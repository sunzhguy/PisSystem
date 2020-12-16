/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 14:35:27
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-07 16:51:15
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pisc.h"
#include "../manage/dev_status.h"
#include "../manage/dev_master.h"
#include "../lib/pis_pack.h"
#include "../teminal_device/pisc_local.h"
#include "../process_matrix/pis_datproc_matrix.h"





static void _PISC_Key_Handle(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static void _PISC_Master_Handle(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static void _PISC_UpateLocalProcessToMaster(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);
static void _PISC_ReciveProcesstPacket(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame);



static T_DEV_BUSMSGDATA_PROC  gtPiscMasterMsgDataProc[] =
{
	{PIS_PACK_HEADINFO_SIZE+0,		6,		B_1BIT,	UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_PISC_Key_Handle	    },	
	{PIS_PACK_HEADINFO_SIZE+0,		7,		B_1BIT,	UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_PISC_Master_Handle	},	
	{0,		                        0,		0,		0,			0,	NULL											    }
};

static  T_DEV_BUSMSGDATA_PROC gtPiscSlaveMsgDataProc[] =
{
	{PIS_PACK_HEADINFO_SIZE+0,		6,		B_1BIT,	UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_PISC_Key_Handle				},	
	{PIS_PACK_HEADINFO_SIZE+0,		7,		B_1BIT,	UNEQUAL,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_PISC_Master_Handle			},	

	{PIS_PACK_HEADINFO_SIZE+0, 		0,		B_1BIT,	WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_PISC_UpateLocalProcessToMaster  },	
	{PIS_PACK_HEADINFO_SIZE+0,		0,		B_1BIT,	WHATEVER,	0,	(PF_PROCESSMSGHANDLER_CALLBACK)_PISC_ReciveProcesstPacket		},	
	{0,		                        0,		0,		0,			0,	NULL													        }
};

static T_MODE_MATRIX   gtPiscModeMatixProcTable[]=
{
	{PISC_STATUS_MASTER, gtPiscMasterMsgDataProc},			//主状态处理矩阵
	{PISC_STATUS_SLAVE,  gtPiscSlaveMsgDataProc },		    //备状态处理矩阵
	{0, 0,},
};

//命令处理索引
static T_CMD_MATRIX  gtPiscCmdMatixProcTable[]=
{
	{CTRL_CMD_DEFAULT,  gtPiscModeMatixProcTable,},			//过程数据处理矩阵
	//{CTRL_CMD_VOLUME_CTRL,	NULL,},			      //音量控制
	//{CTRL_CMD_FEP,	NULL,}, 		
	{0, NULL,},
};

static uint8_t gu8OtherPiscKeyStatus    = 0;
static uint8_t gu8OtherPiscMasterStatus = 0;

void PISC_Init(void)
{
	//初始化设备有效性
	DEV_STATUS_InitDevValid(DEV_TYPE_PISC);
	PisDataProc_Matrix_AddDeviceToMatrixTable(DEV_TYPE_PISC, gtPiscCmdMatixProcTable);
}


static void _PISC_UpateLocalProcessToMaster(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{
	T_PISC_PROCESSTDATA *ptPiscProcessData = (T_PISC_PROCESSTDATA *)_ptPisRecvPackDataFrame->acData;
    
	if(CTRL_CMD_DEFAULT!=_ptPisRecvPackDataFrame->u16Cmd)
     return ;

	if(ptPiscProcessData->tFlag.b8MasterFlag)
	{
		if(_ptPisRecvPackDataFrame->u8DstDevId == PISC_LOCAL_GetOtherDevId())
		{
			PISC_LOCAL_SetStationInfo(ptPiscProcessData->tStationInfo);
			PISC_LOCAL_SetRunDir(ptPiscProcessData->tFlag.b8RunDir);		
		}
	}
}


static void _PISC_ReciveProcesstPacket(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{
	PisDataProc_Matrix_CompareUpdateProcessData( _ptPisRecvPackDataFrame);
}




uint8_t PISC_GetOtherPisc_KeyStatus(void)
{
	return gu8OtherPiscKeyStatus;
}
void PISC_SetOtherPisc_KeyStatus(uint8_t _u8Status)
{
	if(gu8OtherPiscKeyStatus != _u8Status)
	{
		gu8OtherPiscKeyStatus = _u8Status;
		DEV_MASTER_MasterProcess();
	}
}


uint8_t PISC_GetOtherPisc_MasterStatus(void)
{
	return gu8OtherPiscMasterStatus;
}
void PISC_SetOtherPisc_MasterStatus(uint8_t _u8Status)
{
	if(gu8OtherPiscMasterStatus != _u8Status)
	{
		gu8OtherPiscMasterStatus = _u8Status;
		DEV_MASTER_MasterProcess();
	}
}

static void _PISC_Key_Handle(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{
	T_PISC_PROCESSTDATA *ptPiscProcessData=(T_PISC_PROCESSTDATA *)_ptPisRecvPackDataFrame->acData;
	if(CTRL_CMD_DEFAULT != _ptPisRecvPackDataFrame->u16Cmd)
        return;
	if(_ptPisRecvPackDataFrame->u8SrcDevId == PISC_LOCAL_GetOtherDevId())
	{
		printf("pisc_key_proc, other key: %d\r\n",ptPiscProcessData->tFlag.b8ActiveFlag);
		PISC_SetOtherPisc_KeyStatus(ptPiscProcessData->tFlag.b8ActiveFlag);
	}
}
static void _PISC_Master_Handle(T_PIS_PACKDATAFRAME *_ptPisRecvPackDataFrame)
{
	T_PISC_PROCESSTDATA *ptPiscProcessData=(T_PISC_PROCESSTDATA *)_ptPisRecvPackDataFrame->acData;
	if(CTRL_CMD_DEFAULT!=_ptPisRecvPackDataFrame->u16Cmd)
        return;
	if(_ptPisRecvPackDataFrame->u8SrcDevId == PISC_LOCAL_GetOtherDevId())
	{
		printf("pisc_master_proc, other master: %d\r\n",ptPiscProcessData->tFlag.b8MasterFlag);
		PISC_SetOtherPisc_MasterStatus(ptPiscProcessData->tFlag.b8MasterFlag);
	}
}


