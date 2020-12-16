/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 17:03:14
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 10:16:24
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "dev_master.h"
#include "dev_status.h"
#include "../operate_device/pisc.h"
#include "../teminal_device/pisc_local.h"

static uint8_t gu8MasterSlaveType = 0;
void DEV_MASTER_Init_SlaveType(void)
{

    gu8MasterSlaveType = SLAVE_TYPE_INIT;///配置成从设备
    printf("++DEV_MASTER  TYPE:%x\n",gu8MasterSlaveType);
    PISC_LOCAL_SetMasterFlag(gu8MasterSlaveType&0x01);
}

static void _DEV_MASTER_Set_MasterSlaveType(uint8_t _u8Type)
{
	//printf("DEV_MASTER_Set_MasterSlaveType Tp:0x%x,Tp:0x%x\r\n",gu8MasterSlaveType,_u8Type);
	if(_u8Type != gu8MasterSlaveType)
	{
		printf("DEV_MASTER_Set_MasterSlaveType: 0x%x\r\n",_u8Type);
		gu8MasterSlaveType = _u8Type;
		PISC_LOCAL_SetMasterFlag(gu8MasterSlaveType&0x01);
	}
	
}

uint8_t DEV_MASTER_Get_MasterSlaveType(void)
{
    return gu8MasterSlaveType;
}

void DEV_MASTER_MasterProcess(void)
{
	uint8_t devModeType=DEV_MASTER_Get_MasterSlaveType();
	//printf("DEV_MASTER_MasterProcess:%d\r\n",devModeType);
	//对端中央控制器故障
	if(DEV_STATUS_OK != DEV_STATUS_GetDevStatus(DEV_TYPE_PISC,PISC_LOCAL_GetOtherDevId()))
	{
		devModeType = MASTER_TYPE_OTHER_PISC_ERROR;
	}		
	else
	{
		//对方有钥匙，降备
		if(PISC_GetOtherPisc_KeyStatus())
		{
			devModeType = SLAVE_TYPE_OTHER_KEY;
		}
		//对方没钥匙
		else
		{
			//本地有钥匙
			if(PISC_LOCAL_GetKeyStatus())
			{
				devModeType = MASTER_TYPE_KEY;			
			}
			else
			{
				if(PISC_LOCAL_GetOtherDevId() == 1)
				{
					if(SLAVE_STATUS == PISC_GetOtherPisc_MasterStatus())
					{
						devModeType = MASTER_TYPE_DEV_ID_LOW;
					}
				}
				else
				{
					devModeType = SLAVE_TYPE_DEV_ID_HIGH;
				}				
			}
		}
	}

	_DEV_MASTER_Set_MasterSlaveType(devModeType);
	

}
