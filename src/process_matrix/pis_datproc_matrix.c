/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-03 15:17:09
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 17:11:40
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "pis_datproc_matrix.h"
#include "../teminal_device/pisc_local.h"
#include "../lib/pis_pack.h"

#define DEV_MAXCNT   20

typedef struct _T_DEV_PACK_BACKUPDATA
{
	uint8_t  u8DevType;
	uint8_t  u8DevId;
	uint16_t u16DatLen;
	uint8_t  acDataBackUp[128];
}__attribute((packed))T_DEV_PACK_BACKUPDATA;//打包备份数据

static uint8_t gu8DevIndex=0;
static T_DEV_PACK_BACKUPDATA gatDevPackBackUpDataTable[DEV_MAXCNT]=
{
    {0,0,0,{0}},

};

static T_DEV_MATRIX gatDevMatrixTable[DEV_MAXCNT]=
{
	{0,NULL},
};

static void _PisDataProc_GetPreBackUpProcessData(uint8_t _u8DevType,uint8_t _u8DevId,uint8_t *pucBuf,uint16_t *_p16Len)
{
  T_DEV_PACK_BACKUPDATA *ptDevPackBackUpdata = gatDevPackBackUpDataTable;	//临时状态指令表		

	while(ptDevPackBackUpdata->u8DevId)
	{
		if(_u8DevType == ptDevPackBackUpdata->u8DevType && _u8DevId == ptDevPackBackUpdata->u8DevId)
		{
			printf("$$$$$$$$$$$$$DevType:%d DevId:%d\n",ptDevPackBackUpdata->u8DevType,ptDevPackBackUpdata->u8DevId);
			*_p16Len = ptDevPackBackUpdata->u16DatLen;
 			memcpy(pucBuf,(uint8_t *)&ptDevPackBackUpdata->acDataBackUp[0],ptDevPackBackUpdata->u16DatLen);
			break;
		}		
		ptDevPackBackUpdata++;		
	}
    
}

void PisDataProc_MatrixInit(void)
{
	memset((uint8_t *)&gatDevMatrixTable,0,sizeof(gatDevMatrixTable));
}

void PisDataProc_MatrixInit_ProcessData(uint8_t _u8DevType, uint8_t _u8DevId)
{
	static uint8_t i=0;
	//添加tms设备
	if(i < DEV_MAXCNT)
	{
		gatDevPackBackUpDataTable[i].u8DevType=_u8DevType;
		gatDevPackBackUpDataTable[i].u8DevId=_u8DevId;
		gatDevPackBackUpDataTable[i].u16DatLen=80;
		i++;	
	}
	memset((uint8_t *)gatDevPackBackUpDataTable[i].acDataBackUp, 0,sizeof(T_DEV_PACK_BACKUPDATA));
	
}
void PisDataProc_Matrix_AddDeviceToMatrixTable(uint8_t _u8DevType, T_CMD_MATRIX *_ptCmdMatrixTable)
{
	if(gu8DevIndex>=(sizeof(gatDevMatrixTable)/sizeof(T_DEV_MATRIX)))
		{
			printf("data_proc_matrix_add_device_to_matrix_table, too many dev,exit...\r\n");
			exit(0);
		}
	gatDevMatrixTable[gu8DevIndex].u8DevType=_u8DevType;
	gatDevMatrixTable[gu8DevIndex].ptCmdMatrixTable=_ptCmdMatrixTable;
	gu8DevIndex++;
}


void PisDataProc_Matrix_CompareUpdateProcessData(T_PIS_PACKDATAFRAME *_ptPiscReciveDataFrame)
{
	T_PIS_PACKDATAFRAME tPisPackDataFrame;
	T_DEV_PACK_BACKUPDATA *ptDevPackBackUpData = &gatDevPackBackUpDataTable[0];	
	memcpy((uint8_t *)&tPisPackDataFrame,(uint8_t *)_ptPiscReciveDataFrame,_ptPiscReciveDataFrame->u16DatLen+6);	

	while(ptDevPackBackUpData->u8DevId)
	{		
		if(tPisPackDataFrame.u8SrcDevType == ptDevPackBackUpData->u8DevType && tPisPackDataFrame.u8SrcDevId == ptDevPackBackUpData->u8DevId)
		{	
			ptDevPackBackUpData->u16DatLen=_ptPiscReciveDataFrame->u16DatLen+6;
			memcpy((uint8_t *)ptDevPackBackUpData->acDataBackUp,(uint8_t *)&tPisPackDataFrame,_ptPiscReciveDataFrame->u16DatLen+6);
			break;
		}
		ptDevPackBackUpData++;
	}
	
}


static uint8_t  gucTempBufBackup[256]={0};	
void PisDataProc_Matrix_Dispatch(uint8_t _u8DevType, uint8_t _u8DevId,uint16_t  _u16Cmd, uint8_t *_pucBuf, uint16_t _u16Len, uint8_t _u8MatixIndexlen)
{
	uint8_t aucRecvPackDataFrameTempBuf[sizeof(T_PIS_PACKDATAFRAME)]={0};
	T_DEV_MATRIX  const *ptDevMatixTb = &gatDevMatrixTable[0];	//临时状态指令表	
	if((_pucBuf == NULL)) return ; //判断过程数据		
	if(_u16Len > sizeof(aucRecvPackDataFrameTempBuf))
	{
		printf("data_proc_matrix_Dispatch,dev_type: %d, too large len: 0x%x, pack_size_max: %d\r\n",_u8DevType,_u16Len,sizeof(aucRecvPackDataFrameTempBuf));
		//print_buf(aucRecvPackDataFrameTempBuf,30);
		return;
	}
	memcpy(aucRecvPackDataFrameTempBuf, &_pucBuf[_u8MatixIndexlen], _u16Len);	
	//FiPrint("data_proc_matrix_Dispatch,dev_type: 0x%x,dev_id: %d, cmd: 0x%x",dev_type,dev_id,cmd);
	//printf("dev_type:%d,dev_id: %d, cmd: 0x%x,dev_index:%d,matrix_index_len:%d\r\n",dev_type,dev_id,dev_index,matrix_index_len);
	while(ptDevMatixTb->u8DevType)
	{		
		if(ptDevMatixTb ->u8DevType == _u8DevType) //查找设备
		{
			
			T_CMD_MATRIX const *ptCmdMatrixTb=ptDevMatixTb->ptCmdMatrixTable;		
			while(ptCmdMatrixTb->u16Cmd)
			{
				//命令字不符合，返回
				if(_u16Cmd == ptCmdMatrixTb->u16Cmd)
				{			
					T_MODE_MATRIX const *ptModeMatrixTb=ptCmdMatrixTb->ptModeMatrixTable;		
					while(NULL != ptModeMatrixTb)
					{
						uint8_t u8MasterStatus = 0;
						u8MasterStatus = PISC_LOCAL_GetMasterFlag();			

							
						if(u8MasterStatus == ptModeMatrixTb->u8PiscMasterMode)
						{
							uint16_t len_tmp = 0;
							//printf("master..._status:%d--%d\r\n",u8MasterStatus,ptModeMatrixTb->u8PiscMasterMode);
							T_DEV_BUSMSGDATA_PROC const *ptDevBusMsgDataProcTb = ptModeMatrixTb->ptDevBusDatMsgProcTb;
							memset(gucTempBufBackup,0,sizeof(gucTempBufBackup));
							_PisDataProc_GetPreBackUpProcessData(_u8DevType,_u8DevId, gucTempBufBackup,&len_tmp);
							while(ptDevBusMsgDataProcTb->pfMsgHandleCallBack != NULL)
							{
								if(ptDevBusMsgDataProcTb->u8Byte_Index <= _u16Len)
								{
									if(ptDevBusMsgDataProcTb->u8JugeFlag == UNEQUAL)
									{
										if(ptDevBusMsgDataProcTb->u8Bit_Index > B_8BIT)
										{
											//判断命令字并且CALLBACK函数不为空
											if((((aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index+1]<<8|aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index]) \
												^  (gucTempBufBackup[_u8MatixIndexlen+ptDevBusMsgDataProcTb->u8Byte_Index]|(gucTempBufBackup[_u8MatixIndexlen+ptDevBusMsgDataProcTb->u8Byte_Index+1]<<8))) 
												& (ptDevBusMsgDataProcTb->u16BitNum << ptDevBusMsgDataProcTb->u8Bit_Index)) != 0)  
												{	
													ptDevBusMsgDataProcTb->pfMsgHandleCallBack((T_PIS_PACKDATAFRAME *)_pucBuf);
												}								
										}
										else
										{
											//判断命令字并且CALLBACK函数不为空
											if(((aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index] ^  (gucTempBufBackup[_u8MatixIndexlen+ptDevBusMsgDataProcTb->u8Byte_Index]))\
												& (ptDevBusMsgDataProcTb->u16BitNum << ptDevBusMsgDataProcTb->u8Bit_Index)) != 0)  
											{	
												if(_u8DevType == 3)
												printf("+++++++++++++++_u8DevType:%d--->byteIndex:%d\n",_u8DevType,ptDevBusMsgDataProcTb->u8Byte_Index);
												ptDevBusMsgDataProcTb->pfMsgHandleCallBack((T_PIS_PACKDATAFRAME *)_pucBuf);
											}
										}
									}
									else if(ptDevBusMsgDataProcTb->u8JugeFlag == EQUAL)
									{		
										if(ptDevBusMsgDataProcTb->u16BitNum > B_8BIT)
										{
											if(((aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index+1]<<8|aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index])\
											& (ptDevBusMsgDataProcTb->u16BitNum << ptDevBusMsgDataProcTb->u8Bit_Index))
											== (ptDevBusMsgDataProcTb->u16CompValue << ptDevBusMsgDataProcTb->u8Bit_Index))
											{		
												ptDevBusMsgDataProcTb->pfMsgHandleCallBack((T_PIS_PACKDATAFRAME *)_pucBuf); //调用CALLBACK函数
											}
										}
										else
										{
											if((aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index] & (ptDevBusMsgDataProcTb->u16BitNum  << ptDevBusMsgDataProcTb->u8Bit_Index))\
											== (ptDevBusMsgDataProcTb->u16CompValue << ptDevBusMsgDataProcTb->u8Bit_Index))
											{		
												ptDevBusMsgDataProcTb->pfMsgHandleCallBack((T_PIS_PACKDATAFRAME *)_pucBuf); //调用CALLBACK函数
											}
										}
									}
									else if(ptDevBusMsgDataProcTb->u8JugeFlag == WHATEVER)
									{
										ptDevBusMsgDataProcTb->pfMsgHandleCallBack((T_PIS_PACKDATAFRAME *)_pucBuf); //调用CALLBACK函数
									}					
									else if(ptDevBusMsgDataProcTb->u8JugeFlag == BIGGER)
									{		
										if((aucRecvPackDataFrameTempBuf[ptDevBusMsgDataProcTb->u8Byte_Index] & (ptDevBusMsgDataProcTb->u16BitNum << ptDevBusMsgDataProcTb->u8Bit_Index))\
										 > (ptDevBusMsgDataProcTb->u16CompValue << ptDevBusMsgDataProcTb->u8Bit_Index))
										{		
											ptDevBusMsgDataProcTb->pfMsgHandleCallBack((T_PIS_PACKDATAFRAME *)_pucBuf); //调用CALLBACK函数
										}
									}		
								}
								ptDevBusMsgDataProcTb++; 
							}	
							return;
						}
						ptModeMatrixTb++;				
					}	
				}
				ptCmdMatrixTb++; 	
			}
		}
		ptDevMatixTb++;
	}
	return;
}