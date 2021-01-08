/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 09:13:53
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-07 14:11:05
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "pisc_local.h"
#include "../driver/systime.h"
#include "../manage/dev_master.h"
#include "../manage/train_manage.h"
#include "../operate_device/pisc.h"
#include "../include/pis_config.h"
#include "../lib/ini_file.h"
#include "../operate_device/tms.h"
#include "../port_layer/ctrl_udport.h"
#include "../process_matrix/pis_datproc_matrix.h"
#include "../driver/led.h"


static T_PISC_PROCESSTDATA 	gtPiscProcessDataBackUp;		//过程数据帧
static uint8_t  gu8DevId    = 0x01;
static uint32_t gu32DevIp  = 0xa8a86659;
static uint16_t gu16TrinId = 0x0001;

static T_TMSTIME gtLocalTime;



//初始化设备id
static void _PISC_LOCAL_InitDevId(void);
//初始化设备ip
static void _PISC_LOCAL_InitDevIP(uint8_t _u8DevId);
static void _PISC_LOCAL_SetVersion(uint32_t _u32version);


void PISC_LOCAL_Init(void)
{	
	printf("PISC_LOCAL Init.....\n");
	//初始化设备id
	_PISC_LOCAL_InitDevId();
	//初始化设备ip
	_PISC_LOCAL_InitDevIP(PISC_LOCAL_GetDevId());
    
	//初始化过程数据
	PISC_LOCAL_ProcessDataInit();	
}

void PISC_LOCAL_ProcessDataInit(void)
{
	T_STATIONINFO tStationInfo;
	uint8_t acTmpStr[8] = {0};

	memset((uint8_t *)&gtPiscProcessDataBackUp, 0, sizeof(T_PISC_PROCESSTDATA));
	
	tStationInfo.u8StartStation = 1;
	tStationInfo.u8EndStation   = 20;
	tStationInfo.u8CurStation   = 1;
	tStationInfo.u8NexStation   = 2;	

	PISC_LOCAL_SetKeyStatus(1);
	PISC_LOCAL_SetStationInfo(tStationInfo);
	PISC_LOCAL_SetRunDir(PISC_DIR_UP);
	PISC_LOCAL_SetWorkMode(PISC_ATC_MODE);	
	DEV_MASTER_Init_SlaveType();
	_PISC_LOCAL_SetVersion(MAJOR_VERSION);//设置版本号


	//客室led滚动速度
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"lcu_led","speed_time",(char *)acTmpStr,sizeof(acTmpStr)-1)==0)
	{
		gtPiscProcessDataBackUp.u8SecondLcuRoll = atoi((char *)acTmpStr);
	}
	/*		
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"dmp","light_level",(char *)acTmpStr,sizeof(acTmpStr)-1)==0)
	{
		gtPiscProcessDataBackUp.dmp_light_level=atoi((char *)acTmpStr);
	}
	sunzhguy*/	

	//版本号 v1.2 svn430, 发送数据为0x01 0x02 0x04 0x30
	//gtPiscProcessDataBackUp.u32Version =((0<<24)|(2<<16)|(10<<8)|(1<<0));
}

uint32_t PISC_LOCAL_GetDevIp(void)
{
	return gu32DevIp;
}
uint16_t PISC_LOCAL_GetTrainId(void)
{
	return gu16TrinId;
}

static void _PISC_LOCAL_InitDevId(void)
{
	//read GPIO Get DevIP 
}

static void  _PISC_LOCAL_InitDevIP(uint8_t _u8DevId)
{
	//toughtand set /home/conf/net.conf
}

uint8_t PISC_LOCAL_GetDevId(void)
{
		return gu8DevId;
}
uint8_t PISC_LOCAL_GetOtherDevId(void)
{
	 return gu8DevId+1;//
}

void PISC_LOCAL_SetKeyStatus(uint8_t _u8Flag)
{
	if(_u8Flag != gtPiscProcessDataBackUp.tFlag.b8ActiveFlag)
	{
		printf("+++++++++++++++pisc_local_set_key_status: %d\r\n",_u8Flag);
		//led灯显示
		//led_onoff(KEY_LED_BIT,flag);
		//led_ctrl(LED5_ACTIVE,flag);
		if(_u8Flag == 0x00)
		LED_Ctrl(LED_ACTIVE,LED_OFF);
		else
		{
		LED_Ctrl(LED_ACTIVE,LED_ON);
		}
		
		
		gtPiscProcessDataBackUp.tFlag.b8ActiveFlag =_u8Flag;
		//处理主备
		DEV_MASTER_MasterProcess();
	}
}

uint8_t PISC_LOCAL_GetKeyStatus(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8ActiveFlag;		
}



static void _PISC_LOCAL_SetVersion(uint32_t _u32version)
{
	gtPiscProcessDataBackUp.u32Version = _u32version;
}


void PISC_LOCAL_SetWorkMode(uint8_t _u8Mode)
{
	if(_u8Mode == PISC_ATC_MODE)
	{
		gtPiscProcessDataBackUp.tFlag.b8ActiveFlag 	 = 1;
		gtPiscProcessDataBackUp.tFlag.b8TMS_ModeFlag = 0;
		gtPiscProcessDataBackUp.tFlag.b8DCP_ModeFlag = 0;
	}
	else if(_u8Mode == PISC_DCP_MANUAL_MODE)
	{
		gtPiscProcessDataBackUp.tFlag.b8ActiveFlag   = 0;
		gtPiscProcessDataBackUp.tFlag.b8TMS_ModeFlag = 0;
		gtPiscProcessDataBackUp.tFlag.b8DCP_ModeFlag = 1;
	}	
}

uint8_t PISC_LOCAL_GetWorkMode(void)
{
	if(gtPiscProcessDataBackUp.tFlag.b8ActiveFlag)
	{
		return PISC_ATC_MODE;
	}
	else if(gtPiscProcessDataBackUp.tFlag.b8DCP_ModeFlag)
	{
		return PISC_DCP_MANUAL_MODE;
	}
	return PISC_DCP_MANUAL_MODE;
}

uint8_t PISC_LOCAL_GetMasterFlag(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8MasterFlag;
}

void PISC_LOCAL_SetMasterFlag(uint8_t _u8Status)
{

	//printf("++++++++++++status:%d,Flag:%d\n",_u8Status,gtPiscProcessDataBackUp.tFlag.b8MasterFlag);
	if(_u8Status != gtPiscProcessDataBackUp.tFlag.b8MasterFlag)
	{
		printf("PISC_LOCAL_SetMasterFlag+++: %d\r\n",_u8Status);


		//led_onoff(MANUAL_LED_BIT,master_flag_c);
		//置过程数据标志位
		gtPiscProcessDataBackUp.tFlag.b8MasterFlag = _u8Status;

		//升主
		if(_u8Status)  //主
		{
			//初始化模块数据
			PISC_Init();
			TMS_Init();
			#if 0
			by sunzhguy
			dcp_init();
			pisc_init();
			tms_init(); 
			dva_init();//DVA设备初始化
			media_init();
			ptu_init();
			occ_init();
			#endif

		}
		//降备
		else
		{
			printf("PISC Become Slave, stop broadcasting...\r\n");
			//停止所有广播
			//bysunzhguy broadcast_stop_proc(broadcast_get_broadcast_type());
		}
	}
}

uint8_t  PISC_LOCAL_GetRunDir(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8RunDir;
}

void PISC_LOCAL_SetRunDir(uint8_t _u8UpDown)
{	
	if(gtPiscProcessDataBackUp.tFlag.b8RunDir != _u8UpDown)
	{
		gtPiscProcessDataBackUp.tFlag.b8RunDir = _u8UpDown;

		if(PISC_DIR_UP == _u8UpDown)
		{
			printf("pisc_set_updown: up\r\n");
		}
		else if(PISC_DIR_DOWN == _u8UpDown)
		{
			printf("pisc_set_updown: down\r\n");
		}		
		else
		{
			printf("pisc_set_updown: wrong dir: %d\r\n",_u8UpDown);
		}				
	}
}


void PISC_LOCAL_SetPreFlag(uint8_t _u8Flag)
{	
	if(gtPiscProcessDataBackUp.tFlag.b8PreFlag != _u8Flag)
	{
		printf("PISC_LOCAL Set PreFlag: %d\r\n",_u8Flag);
		gtPiscProcessDataBackUp.tFlag.b8PreFlag = _u8Flag;
	}

}
uint8_t PISC_LOCAL_GetPreFlag(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8PreFlag ;
}

void PISC_LOCAL_SetArrFlag(uint8_t _u8Flag)
{	
	if(gtPiscProcessDataBackUp.tFlag.b8ArrFlag != _u8Flag)
	{
		printf("PISC_LOCAL Set ArrFlag: %d\r\n",_u8Flag);
		gtPiscProcessDataBackUp.tFlag.b8ArrFlag = _u8Flag;
	}
}


uint8_t PISC_LOCAL_GetArrFlag(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8ArrFlag ;
}


/*void pisc_set_dmp_light(uint8 light_level)
{	
	if(local_pisc_data_bak.dmp_light_level!=light_level)
	{
		printf("pisc_set_dmp_light: %d\r\n",light_level);
		local_pisc_data_bak.dmp_light_level=light_level;
	}
}
uint8 pisc_get_dmp_light(void)
{
	return local_pisc_data_bak.dmp_light_level ;
}*/


void PISC_LOCAL_SetLCURollSec(uint8_t _u8RollSec)
{	
	if(gtPiscProcessDataBackUp.u8SecondLcuRoll != _u8RollSec)
	{
		printf("pisc_set_lcu_led_roll_sec: %d\r\n",_u8RollSec);
		gtPiscProcessDataBackUp.u8SecondLcuRoll = _u8RollSec;
	}
}

uint8_t PISC_LOCAL_GetLCURollSec(void)
{
	return gtPiscProcessDataBackUp.u8SecondLcuRoll ;
}

void PISC_LOCAL_SetStationInfo(T_STATIONINFO _tStationInfo)
{
	if(memcmp((uint8_t *)&gtPiscProcessDataBackUp.tStationInfo,(uint8_t *)&_tStationInfo,sizeof(T_STATIONINFO)))
	{
		gtPiscProcessDataBackUp.tStationInfo.u8StartStation = _tStationInfo.u8StartStation;
		if(gtPiscProcessDataBackUp.tStationInfo.u8EndStation != _tStationInfo.u8EndStation)
		{
			gtPiscProcessDataBackUp.tStationInfo.u8EndStation = _tStationInfo.u8EndStation;
		}
		gtPiscProcessDataBackUp.tStationInfo.u8CurStation = _tStationInfo.u8CurStation;
		gtPiscProcessDataBackUp.tStationInfo.u8NexStation = _tStationInfo.u8NexStation;

		printf("pisc_set_station_info:+++++++++++++ \r\n");
		//print_buf((uint8 *)&station_info_c, sizeof(station_info_t));

		printf("+++station_info_start:%d\r\n",gtPiscProcessDataBackUp.tStationInfo.u8StartStation);
		printf("+++station_info_current:%d\r\n",gtPiscProcessDataBackUp.tStationInfo.u8CurStation);
		printf("+++station_info_next:%d\r\n",gtPiscProcessDataBackUp.tStationInfo.u8NexStation);
		printf("+++station_info_end:%d\r\n",gtPiscProcessDataBackUp.tStationInfo.u8EndStation);
		
	}
}

void PISC_LOCAL_GetStationInfo(T_STATIONINFO *_ptStationInfo)
{
	_ptStationInfo->u8StartStation = gtPiscProcessDataBackUp.tStationInfo.u8StartStation;
	_ptStationInfo->u8EndStation   = gtPiscProcessDataBackUp.tStationInfo.u8EndStation;
	_ptStationInfo->u8CurStation   = gtPiscProcessDataBackUp.tStationInfo.u8CurStation;
	_ptStationInfo->u8NexStation   = gtPiscProcessDataBackUp.tStationInfo.u8NexStation;
}
uint8_t PISC_LOCAL_GetOpenLeftDoorFlag(void)

{
	return gtPiscProcessDataBackUp.tFlag.b8OpenLeftDoorFlag;
}
void PISC_LOCAL_SetOpenLeftDoorFlag(uint8_t _u8Flag)
{
	if(gtPiscProcessDataBackUp.tFlag.b8OpenLeftDoorFlag != _u8Flag)
	{
		printf("pisc_set_open_left_door_status: %d\r\n",_u8Flag);
		gtPiscProcessDataBackUp.tFlag.b8OpenLeftDoorFlag =_u8Flag;
	}
}

uint8_t PISC_LOCAL_GetOpenRightDoorFlag(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8OpenRightDoorFlag;
}
void PISC_LOCAL_SetOpenRightDoorFlag(uint8_t _u8Flag)
{
	if(gtPiscProcessDataBackUp.tFlag.b8OpenRightDoorFlag != _u8Flag)
	{
		printf("pisc_set_open_right_door_status: %d\r\n",_u8Flag);
		gtPiscProcessDataBackUp.tFlag.b8OpenRightDoorFlag =_u8Flag;
	}
}

uint8_t PISC_LOCAL_GeCloseLeftDoorFlag(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8CloseLeftDoorFlag;
}
uint8_t PISC_LOCAL_GeCloseRightDoorFlag(void)
{
	return gtPiscProcessDataBackUp.tFlag.b8CloseRightDoorFlag;
}
void PISC_LOCAL_SetCloseDoorFlag(uint8_t _u8Flag)
{
	gtPiscProcessDataBackUp.tFlag.b8CloseLeftDoorFlag  = _u8Flag;
	gtPiscProcessDataBackUp.tFlag.b8CloseRightDoorFlag = _u8Flag;
}

void PISC_LOCAL_SendProcessData(void)
{
	//发送显示数据
	//ctrl_port_send_cmd(0xffff,0xff,0xff,CTRL_CMD_DEFAULT,(uint8 *)&local_pisc_data_bak,sizeof(pisc_default_data_t));
	CTRL_UDPORT_SendAFrameData(0xffff,0xff,0x01,CTRL_CMD_DEFAULT,(uint8_t*)&gtPiscProcessDataBackUp,sizeof(T_PISC_PROCESSTDATA));

}
void PISC_LOCAL_SetTime(uint8_t *_pcTimeBuf, uint16_t _u16TimeLen)
{
	char acDateTimeBuf[128] ={0x00};
	if(_u16TimeLen == sizeof(T_TMSTIME))
	{
		memcpy((uint8_t *)&gtLocalTime,_pcTimeBuf,sizeof(T_TMSTIME));
		sprintf(acDateTimeBuf,"%x-%x-%x %x:%x:%x",gtLocalTime.u16Year,gtLocalTime.u8Month,gtLocalTime.u8Day,gtLocalTime.u8Hour,gtLocalTime.u8Min,gtLocalTime.u8Sec);
		SysTime_DateTimeSet(acDateTimeBuf);
		//printf("DataTime:%s\n",acDateTimeBuf);
	}
	else
	{
		printf("pisc_local_set_time len wrong: %x\r\n",_u16TimeLen);
	}
}
void PISC_LOCAL_GetTime(uint8_t *_pcTimeBuf)
{	
	struct tm *ptTm;
	time_t cTime = time(NULL);
	ptTm=localtime(&cTime);
	gtLocalTime.u16Year = 1900+ptTm->tm_year;
	gtLocalTime.u8Month = ptTm->tm_mon+1;
	gtLocalTime.u8Day   = ptTm->tm_mday;
	gtLocalTime.u8Hour  = ptTm->tm_hour;
	gtLocalTime.u8Min   = ptTm->tm_min;
	gtLocalTime.u8Sec   = ptTm->tm_sec;	
	printf("Get Local Time %d-%d-%d %d:%d:%d\r\n",gtLocalTime.u16Year,gtLocalTime.u8Month,gtLocalTime.u8Day,gtLocalTime.u8Hour,gtLocalTime.u8Min,gtLocalTime.u8Sec);
	memcpy(_pcTimeBuf,(uint8_t *)&gtLocalTime,sizeof(T_TMSTIME));
}

void PISC_LOCAL_SendTime(void)
{
	T_TMSTIME tTmsTempTime;
	PISC_LOCAL_GetTime((uint8_t *)&tTmsTempTime);
	//ctrl_port_send_cmd(0xffff,0xff,0xff,BROADCAST_SET_TIME,(uint8 *)&tms_time_tmp,sizeof(tms_time_t));
	CTRL_UDPORT_SendAFrameData(0xffff,0xff,0xff,BROADCAST_SET_TIME,(uint8_t *)&tTmsTempTime,sizeof(T_TMSTIME));
}


uint32_t  PISC_LOCAL_GetJumpStation(void)
{
	return gtPiscProcessDataBackUp.u32JumpStations;
}

void PISC_LOCAL_SetJumpStation(uint32_t _u32JumpStations)
{
   gtPiscProcessDataBackUp.u32JumpStations = _u32JumpStations;
}


void PISC_LOCAL_UpdateJumpStationFromConf(void)
{
	uint8_t acTemp_str[32] = {0};
	uint8_t i = 0;
	uint32_t skipTmpStation=0;

	for(i = 0; i < 32; i++)
	{
		sprintf(( char*)acTemp_str,"%d_skip_valid",i+1);
		//越站信息
		//if(GetValueFromEtcFile(PISC_CONF_PATH,"pisc","skip_stations",(char *)temp_str,sizeof(temp_str)-1)==0)
		if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"pisc",(const char *)acTemp_str,(char *)acTemp_str,sizeof(acTemp_str)-1)==0)
		{
			skipTmpStation|=(atoi((char *)acTemp_str)<<i);
		}
	}
	PISC_LOCAL_SetJumpStation(skipTmpStation);
}


