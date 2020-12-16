/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 14:47:26
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 14:37:27
 */

//超时次数

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../manage/sys_service.h"
#include "dev_status.h"
#include "dev_master.h"
#include "teminal_device/pisc_local.h"
#include "lib/ini_file.h"
#include "include/pis_config.h"
#include "process_matrix/pis_datproc_matrix.h"

#define DEV_STATUS_TIMEOUT_MAXCNT		(5)
#define DEV_VALID	                    (1)
#define DEV_UNVALID	                    (0)



typedef struct
{
	uint8_t u8DevStatus;
	uint8_t acDevStatusName[32];
}T_DEV_STATUS_NAME;


static const T_DEV_STATUS_NAME gatDevStatus_NameList[]=
{
	{DEV_STATUS_OK,"正常"},
	{DEV_STATUS_ERROR,"错误"},
	{DEV_STATUS_NO_DEV,"无此设备"},
	{0,""},
};

//令牌巡检表结构
typedef struct
{
	uint8_t u8DevType;
	uint8_t u8DevId;			//设备号
	uint8_t acDevType_EngName[16];
	uint8_t u8DevName[32];
	uint8_t u8ValidFlag;
	uint8_t u8DevStatus; //设备状态
	uint8_t acDevStatus_Name[32];
	uint8_t u8DevTimeoutCnt;
}__attribute((packed))T_DEVSTATUS;
//设备状态


static T_DEVSTATUS gatDevStatusList[]=
{	
	{DEV_TYPE_PISC, 1, "pisc", "中央控制器1",DEV_VALID,DEV_STATUS_OK,"正常",0},
	{DEV_TYPE_PISC, 2, "pisc", "中央控制器2",DEV_VALID,DEV_STATUS_OK,"正常",0},
	{DEV_TYPE_DCP,  1,  "dcp",  "广播控制盒1",DEV_VALID,DEV_STATUS_OK,"正常",0},
	{DEV_TYPE_TMS,  1,  "tms",  "列车接口单元1",DEV_VALID,DEV_STATUS_OK,"正常",0},
    {DEV_TYPE_EHP,  3,  "ehp",  "紧急报警器",DEV_VALID,DEV_STATUS_OK,"正常",0},
	{DEV_TYPE_AMP,  15, "amp",  "功率放大器",DEV_VALID,DEV_STATUS_OK,"正常",0},
	{0,             0,  "",    "",          0,          0,          "正常",0},
};

//初始化设备有效状态
void DEV_STATUS_InitDevValid(uint8_t _u8DevType)
{
	uint8_t i				=0;
	uint8_t u8iDev			=0;
	uint8_t u8TempDevId		=0;
	uint8_t u8DevValidFlag  =0;
	uint8_t u8DevTempStaus  =0;
	uint8_t acTempStr[32]   ={0};
	char    acValstr[4]     ="";
	uint8_t acDevTypeEnNameTemp[16] ="";
	uint8_t u8DevMaxCnt    =0;

	while(gatDevStatusList[i].u8DevType)
	{
		if(_u8DevType == gatDevStatusList[i].u8DevType)
		{
			memcpy(acDevTypeEnNameTemp,gatDevStatusList[i].acDevType_EngName,strlen((const char*)gatDevStatusList[i].acDevType_EngName));
			break;
		}
		i++;
	}

	//设备最大数目
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,(const char*)acDevTypeEnNameTemp,"dev_num_max",(char *)acValstr,sizeof(acValstr)-1)==0)
	{
		u8DevMaxCnt=atoi((const char*)acValstr);
	}	
	for(u8iDev = 0;u8iDev <u8DevMaxCnt; u8iDev++)
	{
		u8TempDevId=u8iDev+1;
		sprintf((char *)acTempStr,"%d_valid",u8TempDevId);
		memset(acValstr,0,sizeof(acValstr));
		//查找设备是否有效
		if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,(const char*)acDevTypeEnNameTemp,(const char*)acTempStr,(char *)acValstr,sizeof(acValstr)-1)==0)
		{
			u8DevValidFlag=atoi((const char*)acValstr);
		}		
		//设备是否有效
		gatDevStatusList[i].u8ValidFlag=u8DevValidFlag;
		sprintf((char *)acTempStr,"%d_status",u8TempDevId);
		//查找设备状态
		if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,(const char*)acDevTypeEnNameTemp,(const char*)acTempStr,(char *)acValstr,sizeof(acValstr)-1)==0)
		{
			u8DevTempStaus = atoi((const char*)acValstr);
		}		
		//设备有效,添加到设备状态表
		if(u8DevValidFlag)
		{
			//注册数据处理函数
			PisDataProc_MatrixInit_ProcessData(_u8DevType,u8TempDevId);
		}			
	}
}

//获得设备状态
uint8_t DEV_STATUS_GetDevStatus(uint8_t _u8DevType,uint8_t _u8DevId)
{
	uint8_t i = 0;
	while(gatDevStatusList[i].u8DevType)
	{
		if(_u8DevType  == gatDevStatusList[i].u8DevType
			&& _u8DevId == gatDevStatusList[i].u8DevId)
		{
			return gatDevStatusList[i].u8DevStatus;
		}
		i++;
	}
	//没有设备
	return DEV_STATUS_NO_DEV;
}
static void _DEV_STATUS_GetDevStatusName(uint8_t _u8DevStatus, uint8_t *pcDevStaName)
{
	uint8_t i=0;
	while(gatDevStatus_NameList[i].u8DevStatus)
	{
		if(_u8DevStatus == gatDevStatus_NameList[i].u8DevStatus)
		{
			memcpy(pcDevStaName,gatDevStatus_NameList[i].acDevStatusName,strlen((const char*)gatDevStatus_NameList[i].acDevStatusName));
			break;
		}
		i++;
	}
}

//设置设备状态
void DEV_STATUS_SetDevStatus(uint8_t _u8DevType,uint8_t _u8DevId, uint8_t _u8Status)
{
	uint8_t i=0;
	//printf("+++++++++dev_type:%d,dev_id:%d++++++\r\n",dev_type,dev_id);
	while(gatDevStatusList[i].u8DevType)
	{
		//printf("table_type:%d-->%d,--table--id:%d-->%d,status:%d\r\n",gatDevStatusList[i].u8DevType,_u8DevType,gatDevStatusList[i].u8DevId,_u8DevId,_u8Status);

		if(_u8DevType == gatDevStatusList[i].u8DevType &&  _u8DevId == gatDevStatusList[i].u8DevId)
		{	
			//清设备超时计数
			if(DEV_STATUS_OK == _u8Status)
			{
				gatDevStatusList[i].u8DevTimeoutCnt=0;
			}

			if(_u8Status != gatDevStatusList[i].u8DevStatus)
			{
				//更新设备状态
				gatDevStatusList[i].u8DevStatus = _u8Status;
				//获得设备状态的名称
				_DEV_STATUS_GetDevStatusName(gatDevStatusList[i].u8DevStatus,gatDevStatusList[i].acDevStatus_Name);
				//写入日志
				//log_write_dev_status(dev_status_table[i].dev_name,dev_status_table[i].dev_status_name);
				printf("dev_status log---->%s:%s\n",gatDevStatusList[i].acDevType_EngName,gatDevStatusList[i].acDevStatus_Name);
				
			}
			return;
		}
		i++;
	}
}


static void* ThreadDevStatusHandle(void* param)
{	
	printf("Dev Status...Thread. Start.......\r\n");

	while(1)
	{
		uint8_t i=0;
		while(gatDevStatusList[i].u8DevType)
		{
			gatDevStatusList[i].u8DevTimeoutCnt++;
			if(gatDevStatusList[i].u8DevTimeoutCnt >= DEV_STATUS_TIMEOUT_MAXCNT)
			{
				//填充设备状态
				DEV_STATUS_SetDevStatus(gatDevStatusList[i].u8DevType,gatDevStatusList[i].u8DevId,DEV_STATUS_ERROR);
				//printf("+++++++table_dev_type:%d,id:%d,PisLocal_devid:%d\r\n",dev_status_table[i].dev_type,dev_status_table[i].dev_id,pisc_local_get_my_dev_id());
				if(DEV_TYPE_PISC == gatDevStatusList[i].u8DevType && (PISC_LOCAL_GetDevId() != gatDevStatusList[i].u8DevId))
				{
					//对端故障，处理主备
					DEV_MASTER_MasterProcess();
				}
			}
			i++;
		}
		sleep(1);
	}
	return NULL;
}


void	DEV_STATUS_Thread_Create(void)
{
	static pthread_t tThreadDevStatus;
	//创建线程
	pthread_create(&tThreadDevStatus, NULL, ThreadDevStatusHandle,NULL); 	//PA内部协议数据采集线程创建
	return;
}




void    DEV_STATUS_TimerOut_Handle(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
   uint8_t i=0;
   T_TRAIN_SYSTEM* ptTrainSystem = _pvArg;
   T_EVENT_CTL * ptEventCtl = (T_EVENT_CTL * )_pvEventCtl;
   //printf("Dev Status...TimeroutThread. Start.......\r\n");
  
   while(gatDevStatusList[i].u8DevType)
	{
		gatDevStatusList[i].u8DevTimeoutCnt++;
		if(gatDevStatusList[i].u8DevTimeoutCnt >= DEV_STATUS_TIMEOUT_MAXCNT)
		{
				//填充设备状态
			DEV_STATUS_SetDevStatus(gatDevStatusList[i].u8DevType,gatDevStatusList[i].u8DevId,DEV_STATUS_ERROR);
			if(DEV_TYPE_PISC == gatDevStatusList[i].u8DevType && (PISC_LOCAL_GetDevId() != gatDevStatusList[i].u8DevId))
			{
					//对端故障，处理主备
				DEV_MASTER_MasterProcess();
			}
		}
		i++;
	}
	EVIO_EventTimer_Init(_ptEventTimer,1000,DEV_STATUS_TimerOut_Handle,ptTrainSystem);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);
}
