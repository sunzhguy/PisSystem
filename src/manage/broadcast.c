/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 14:15:13
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-13 09:01:30
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "../nanomsg/pair.h"
#include "../nanomsg/nn.h"
#include "broadcast.h"
#include "mp3_decoder.h"
#include "../manage/dev_status.h"
#include "../include/pis_config.h"
#include "../operate_device/occ.h"
#include "../manage/dias_broadcast_rule.h"
#include "../teminal_device/dias.h"
#include "../lib/ini_file.h"
#include "driver/soundcard.h"
#include "../main.h"
#include "../teminal_device/pisc_local.h"
#include "../lib/pis_pack.h"
#include "../port_layer/audio_port_bd.h"

#define MP3_DECODER	(1)      //需要进行MP3解码
#define NO_MP3_DECODER	(0)	//不需要进行mp3解码

static void _BROADCAST_InitDias(void);
static void _BROADCAST_Media(void); 
static void _BROADCAST_OCC(void);
static void _BROADCAST_Living(void);
static void _BROADCAST_LivingStop(void);
static void _BROADCAST_Urgent(void);
static void _BROADCAST_UrgentStop(void);
static void _BROADCAST_DoorClose(void);
static void _BROADCAST_PreArrival(void);
static void _BROADCAST_Arrived(void);
static void _BROADCAST_OCCStop(void);
static void _BROADCAST_DoorCloseStop(void);
static void _BROADCAST_PreArrivalStop(void);
static void _BROADCAST_ArrivalStop(void);
static void _BROADCAST_MediaStop(void);

static void     _BROADCAST_Set_MP3DecodeFlag(uint8_t _u8Flag);
static uint8_t  _BROADCAST_Get_MP3DecodeFlag(void);

typedef struct{

    uint8_t  u8BdOpDevType;//广播操作设备类型
	uint8_t  u8BdOpDevId;//广播操作设备号
	uint8_t  u8BdType;//广播类型
	uint8_t  u8CycleFlag;
	uint8_t  u8BdPriority;//广播优先级
	uint8_t  acConfigKey[32];		//配置文件的key值
	uint16_t au16PlayList[10];
	uint16_t u16PlayListNum;
	uint8_t  u8MP3DecodeFlag;
	uint8_t  acMP3FilePath[64];
	uint8_t  acLCULedFilePath[64];
	void (*pfBoadCastCallBack)(void);//广播处理
	void (*pfGetBroadCastRuleList)(uint16_t *,uint8_t *, uint16_t *);
	void (*pfBroadCastAudioSend)(void);//广播音频发送
	void (*pfBroadCastStop)(void);//广播停止

}T_BROADCASTPROC_LIST;




static T_BROADCASTPROC_LIST _gtBroadCastProcessList[]=
{
	{DEV_TYPE_PISC,	1,BROADCAST_OCC,    0,PRI_NONE,	"pisc_1_occ",
    {0},0,NO_MP3_DECODER,"","",_BROADCAST_OCC,NULL ,                OCC_AudioSend,_BROADCAST_OCCStop},

	{DEV_TYPE_PISC,	2,BROADCAST_OCC,    0,PRI_NONE,"pisc_2_occ",
    {0},0,NO_MP3_DECODER,"","",_BROADCAST_OCC,NULL,                 OCC_AudioSend,_BROADCAST_OCCStop},


    {DEV_TYPE_TMS,	1,BROADCAST_PRE,    0,PRI_NONE,	"tms_1_pre", 
    {0},0,MP3_DECODER,  MP3_PRE_PATH,LCU_LED_PRE_PATH,
    _BROADCAST_PreArrival,DiasBroadCastRule_GetPreList,    DIAS_AudioSend, _BROADCAST_PreArrivalStop},
    
	{DEV_TYPE_TMS,	2,BROADCAST_PRE,	0,PRI_NONE,	"tms_2_pre", 
    {0},0,MP3_DECODER,  MP3_PRE_PATH,LCU_LED_PRE_PATH,
    _BROADCAST_PreArrival,DiasBroadCastRule_GetPreList,    DIAS_AudioSend, _BROADCAST_PreArrivalStop},

    
	{DEV_TYPE_TMS,	1,BROADCAST_ARRIVE, 0,PRI_NONE,	"tms_1_arr", 
    {0},0,MP3_DECODER,  MP3_ARR_PATH,LCU_LED_ARR_PATH,
    _BROADCAST_Arrived   ,DiasBroadCastRule_GetArrList,       DIAS_AudioSend, _BROADCAST_ArrivalStop},	

	{DEV_TYPE_TMS,	2,BROADCAST_ARRIVE,	0,PRI_NONE,	"tms_2_arr", 
    {0},0,MP3_DECODER,  MP3_ARR_PATH,LCU_LED_ARR_PATH,
    _BROADCAST_Arrived   ,DiasBroadCastRule_GetArrList,       DIAS_AudioSend, _BROADCAST_ArrivalStop},	
	
    {DEV_TYPE_PISC,	1,BROADCAST_PRE,	0,PRI_NONE,	"pisc_1_pre",
    {0},0,MP3_DECODER, MP3_PRE_PATH,LCU_LED_PRE_PATH,
    _BROADCAST_PreArrival,DiasBroadCastRule_GetPreList,    DIAS_AudioSend, _BROADCAST_PreArrivalStop},

	{DEV_TYPE_PISC,	2,BROADCAST_PRE,	0,PRI_NONE,	"pisc_2_pre",
    {0},0,MP3_DECODER, MP3_PRE_PATH,LCU_LED_PRE_PATH,
    _BROADCAST_PreArrival,DiasBroadCastRule_GetPreList,    DIAS_AudioSend,  _BROADCAST_PreArrivalStop},

	{DEV_TYPE_PISC,	1,BROADCAST_ARRIVE, 0,PRI_NONE,	"pisc_1_arr",
    {0},0,MP3_DECODER, MP3_ARR_PATH,LCU_LED_ARR_PATH,
    _BROADCAST_Arrived   ,DiasBroadCastRule_GetArrList,       DIAS_AudioSend,  _BROADCAST_ArrivalStop},

	{DEV_TYPE_PISC,	2,BROADCAST_ARRIVE, 0,PRI_NONE,	"pisc_2_arr",
    {0},0,MP3_DECODER, MP3_ARR_PATH,LCU_LED_ARR_PATH,
    _BROADCAST_Arrived   ,DiasBroadCastRule_GetArrList,       DIAS_AudioSend,   _BROADCAST_ArrivalStop},

    {DEV_TYPE_PISC,	1,BROADCAST_CLOSE_DOOR,0,PRI_NONE,"pisc_1_closedoor",
    {0},0,NO_MP3_DECODER,"","",	_BROADCAST_DoorClose,	NULL,       NULL,	          _BROADCAST_DoorCloseStop},

	{DEV_TYPE_PISC,	2,BROADCAST_CLOSE_DOOR,0,PRI_NONE,"pisc_2_closedoor",
    {0},0,NO_MP3_DECODER,"","",	_BROADCAST_DoorClose,	NULL ,      NULL,             _BROADCAST_DoorCloseStop},

    {0,0,0,0,PRI_NONE,"",
    {0},0,NO_MP3_DECODER,"","",NULL,NULL,NULL,NULL,},
    	
};

typedef struct
{
	uint8_t u8BdOpDevType;
	uint8_t u8BdOpDevId;
	uint8_t u8BdType;
	uint8_t u8MP3DecodeFlag;
	uint8_t u8CycleFlag;
	uint8_t u8UrgentCode;
	uint16_t au16PlayList[8];
	uint8_t u8PlayListLength;
}T_BRAODCAST;

static T_BRAODCAST  _gtBroadCastCurrent={0,0,BROADCAST_NONE,NO_MP3_DECODER,0,0,{0},0};


//广播优先级
typedef struct
{
	uint8_t u8Pisc_1_occ;
	uint8_t u8Pisc_2_occ;
	uint8_t u8DCP_1_living;
	uint8_t u8DCP_2_living;
	uint8_t u8DCP_1_urgent;
	uint8_t u8DCP_2_urgent;
	uint8_t u8TMS_1_urgent;
	uint8_t u8TMS_2_urgent;
	uint8_t u8TMS_1_doorclose;
	uint8_t u8TMS_2_doorclose;
	uint8_t u8Pisc_1_doorclose;
	uint8_t u8Pisc_2_doorclose;	
	uint8_t u8DCP_1_pre;
	uint8_t u8DCP_2_pre;	
	uint8_t u8DCP_1_arr;
	uint8_t u8DCP_2_arr;		
	uint8_t u8TMS_1_pre;
	uint8_t u8TMS_2_pre;	
	uint8_t u8TMS_1_arr;
	uint8_t u8TMS_2_arr;	
	uint8_t u8Pics_1_pre;
	uint8_t u8Pics_2_pre;
	uint8_t u8Pics_1_arr;
	uint8_t u8Pics_2_arr;	
	uint8_t u8Meida_1;
	uint8_t u8Meida_2;	
	uint8_t reserve[4];
}T_BROADCASTPRORITY;

static T_BROADCASTPRORITY _gtBroadCastPriority =
{
	1,
	2,
};

typedef struct{
	uint8_t u8DCP_living;
	uint8_t u8DCP_Monitor;
	uint8_t u8DCP_Ehp_Monitor;
	uint8_t u8Occ;
	uint8_t u8Media;
	uint8_t acPowAmp[16];
    uint8_t u8Ehp_Talk;
	uint8_t u8Ehp_Monitor;
	uint8_t u8Dias;
	uint8_t reserve[6];

}T_BROADCAST_VOL;

static T_BROADCAST_VOL _gtBroadCastVolume = {0x00};


void    BROADCAST_Init(void)
{
    //得配置文件的优先级
   BROADCAST_GetConfigPriority();
   _BROADCAST_InitDias();
   //mp3 meida mic LED init
}

void   BROADCAST_Process(uint8_t _u8OpDevType,uint8_t _u8OpDevId,uint8_t _u8TypeBroadCast)
{

    uint8_t  i=0;
	uint8_t  u8BraodCastPriority=0;
	uint8_t  acDecodeFilePathList[FILE_LIST_NUM_MAX][FILE_NAME_LENGTH_MAX]={{0}};    
	uint16_t au16PlayList[FILE_LIST_NUM_MAX]={0};
	uint8_t  acLanguageList[FILE_LIST_NUM_MAX]={0};
	uint16_t u16DecodeListNum = 0;
	
	T_BROADCASTPROC_LIST const *ptBroadCastProcessList = _gtBroadCastProcessList;	
	//当前广播优先级
	u8BraodCastPriority = BROADCAST_GetPriority(_u8OpDevType,_u8OpDevId,BROADCAST_GetBroadCastType());
	printf("broadcast_proc,broadcast_pri_tmp: %d...\r\n",u8BraodCastPriority);
	while(ptBroadCastProcessList->u8BdType)
	{
		printf("ptBroadCastProcessList->u8BdOpDevType:%d--->_u8OpDevType:%d\n",ptBroadCastProcessList->u8BdOpDevType,_u8OpDevType);
		printf("ptBroadCastProcessList->u8BdOpDevId:%d--->_u8OpDevId:%d\n",ptBroadCastProcessList->u8BdOpDevId,_u8OpDevId);
		printf("ptBroadCastProcessList->u8BdType:%d--->_u8TypeBroadCast:%d\n",ptBroadCastProcessList->u8BdType,_u8TypeBroadCast);
		printf("\r\n");
		if(ptBroadCastProcessList->u8BdOpDevType      == _u8OpDevType
			&& ptBroadCastProcessList->u8BdOpDevId    == _u8OpDevId
			&& ptBroadCastProcessList->u8BdType       == _u8TypeBroadCast)
		{
			printf("ptBroadCastProcessList->u8BdPriority: %d--->%d\r\n",ptBroadCastProcessList->u8BdPriority,u8BraodCastPriority);
			printf("ptBroadCastProcessList->acLCULedFilePath:%s\n",ptBroadCastProcessList->acLCULedFilePath);
			if(ptBroadCastProcessList->u8BdPriority <= u8BraodCastPriority || _u8TypeBroadCast == BROADCAST_NONE)
			{
				printf("++++++++++++\r\n");
				BROADCAST_StopProcess(BROADCAST_GetBroadCastType());
				BROADCAST_SetBroadCastType(_u8TypeBroadCast);
				BROADCAST_SetBroadCastOperateDevType(_u8OpDevType);
				BROADCAST_SetBroadCastOperateDevId(_u8OpDevId);
				BROADCAST_SetBroadCastCycleFlag(ptBroadCastProcessList->u8CycleFlag);
				_BROADCAST_Set_MP3DecodeFlag(ptBroadCastProcessList->u8MP3DecodeFlag);
                printf("xxxxxxxxxxxxx\r\n");

				printf("ptBroadCastProcessList->pfGetBroadCastRuleList:%p\n",ptBroadCastProcessList->pfGetBroadCastRuleList);
				//有播放列表
				if(ptBroadCastProcessList->pfGetBroadCastRuleList)
				{
					//获取播放列表，包括音频和客室屏拼接显示
					printf("pfGetBroadCastRuleList:%d\n",u16DecodeListNum);
					ptBroadCastProcessList->pfGetBroadCastRuleList(au16PlayList,acLanguageList,&u16DecodeListNum);
                    
					if(MP3_DECODER == ptBroadCastProcessList->u8MP3DecodeFlag)
					{
						printf("list->mp3_file_path: %s\r\n",ptBroadCastProcessList->acMP3FilePath);
						printf("****play_list_num: %d\r\n",u16DecodeListNum);

					
						for(i = 0; i < u16DecodeListNum; i++)
						{
							sprintf((char *)acDecodeFilePathList[i],"%s/%d/%03d.mp3",ptBroadCastProcessList->acMP3FilePath,acLanguageList[i],au16PlayList[i]);
							printf("play_file_path_list[%d]: %s\r\n",i,acDecodeFilePathList[i]);
						}	

						MP3_Decoder_SetDecodeList(CHANNEL_LEFT,(uint8_t*)acDecodeFilePathList,u16DecodeListNum*2);
						//播放音频
						//mp3_decoder_play_list(CHANNEL_LEFT,(uint8 *)play_file_path_list,play_list_num*2);
						printf("begin sleep 1s\r\n");
						//等待，很重要
						//sleep(1);
						printf("sleep 1s\r\n");
						//启动播放线程
						//mp3_decoder_set_run_flag(1);	
						MP3_Decoder_Set_RunFlag(DECODING);
						
						//亮mp3灯
						//led_onoff(MP3_LED_BIT,1);	
						//led_ctrl(LED9_MP3,1);					
					}

					//客室中文显示设置
					//lcu_led_set_content((uint8 *)list->lcu_led_file_path,(uint16 *)play_list_tmp,play_list_num);
					//客室屏显示内容发送
					//lcu_led_send_content();
				}
				//广播
				ptBroadCastProcessList->pfBoadCastCallBack();	
			}
			break;
		}
		ptBroadCastProcessList++;
	}
    
}


void    BROADCAST_StopProcess(uint8_t _u8TypeBroadCast )
{
      T_BROADCASTPROC_LIST const *ptBroadCastProcessList = _gtBroadCastProcessList;
      while(ptBroadCastProcessList->u8BdType)
      {
          if(ptBroadCastProcessList->u8BdType == _u8TypeBroadCast)
          {
              if(ptBroadCastProcessList->u8MP3DecodeFlag)
              {
                  MP3_Decoder_Stop(CHANNEL_LEFT);
              }
              if(ptBroadCastProcessList->u8CycleFlag)
              {
                  BROADCAST_SetBroadCastCycleFlag(0);
              }
              ptBroadCastProcessList->pfBroadCastStop();
              BROADCAST_SetBroadCastType(BROADCAST_NONE);
          }
		  ptBroadCastProcessList++;
      }

}
void    BROADCAST_SetBroadCastType(uint8_t _u8Type)
{

  if(_gtBroadCastCurrent.u8BdType != _u8Type)
  {
      _gtBroadCastCurrent.u8BdType = _u8Type;
  }

}
uint8_t BROADCAST_GetBroadCastType(void)
{
    return _gtBroadCastCurrent.u8BdType;
}
void    BROADCAST_GetConfigPriority(void)
{
    char acStrTmp[32] = {0x00};
    uint8_t i         = 0;
    T_BROADCASTPROC_LIST *ptBroadCastProcessList = _gtBroadCastProcessList;
    //初始化优先级
    while(ptBroadCastProcessList[i].u8BdType)
    {
        if(INI_FILE_GetValueFromConfig( PISC_CONF_PATH, "priority",
        (const char*)ptBroadCastProcessList[i].acConfigKey,(char*)acStrTmp,sizeof(acStrTmp)-1) == 0)
        {
           ptBroadCastProcessList[i].u8BdPriority = atoi((char*)acStrTmp);
        }
        ++i;
    }
    
}


void    BROADCAST_SetUrgentCode(uint8_t _u8CodeId)
{
    _gtBroadCastCurrent.u8UrgentCode = _u8CodeId;
}
uint8_t BROADCAST_GetUrgentCode(void)
{
   return _gtBroadCastCurrent.u8UrgentCode;
}


void    BROADCAST_SetBroadCastOperateDevType(uint8_t _u8OpDevType)
{
    if(_gtBroadCastCurrent.u8BdOpDevType != _u8OpDevType)
    {
        _gtBroadCastCurrent.u8BdOpDevType = _u8OpDevType;
    }

}
uint8_t BROADCAST_GetBroadCastOperateDevType(void)
{
    return _gtBroadCastCurrent.u8BdOpDevType;
}
void    BROADCAST_SetBroadCastOperateDevId(uint8_t _u8OpDevId)
{
    if(_gtBroadCastCurrent.u8BdOpDevId != _u8OpDevId)
    {
        _gtBroadCastCurrent.u8BdOpDevType = _u8OpDevId;
    }
    
}
uint8_t BROADCAST_GetBroadCastOperateDevId(void)
{
    return _gtBroadCastCurrent.u8BdOpDevId;
}
void    BROADCAST_SetBroadCastCycleFlag(uint8_t _u8CycleFlag)
{
    if(_gtBroadCastCurrent.u8CycleFlag != _u8CycleFlag)
    {
        _gtBroadCastCurrent.u8CycleFlag = _u8CycleFlag;
    }
}
uint8_t BROADCAST_GetBroadCastCycleFlag(void)
{
    return _gtBroadCastCurrent.u8CycleFlag;
    
}
/***************************************************/
static void _BROADCAST_InitDias(void)
{
    //播放一个测试音频
}

void    BROADCAST_SendPriority(void)
{

	_gtBroadCastPriority.u8DCP_1_arr       = BROADCAST_GetPriority(DEV_TYPE_DCP,1,BROADCAST_ARRIVE);
	_gtBroadCastPriority.u8DCP_2_arr       = BROADCAST_GetPriority(DEV_TYPE_DCP,2,BROADCAST_ARRIVE);
	_gtBroadCastPriority.u8DCP_1_living    = BROADCAST_GetPriority(DEV_TYPE_DCP,1,BROADCAST_LIVE);
	_gtBroadCastPriority.u8DCP_2_living    = BROADCAST_GetPriority(DEV_TYPE_DCP,2,BROADCAST_LIVE);
	_gtBroadCastPriority.u8DCP_1_pre	   = BROADCAST_GetPriority(DEV_TYPE_DCP,1,BROADCAST_PRE);
	_gtBroadCastPriority.u8DCP_2_pre       = BROADCAST_GetPriority(DEV_TYPE_DCP,2,BROADCAST_PRE);
	_gtBroadCastPriority.u8DCP_1_urgent    = BROADCAST_GetPriority(DEV_TYPE_DCP,1,BROADCAST_URGENT);
	_gtBroadCastPriority.u8DCP_2_urgent    = BROADCAST_GetPriority(DEV_TYPE_DCP,2,BROADCAST_URGENT);

	_gtBroadCastPriority.u8TMS_1_arr	   = BROADCAST_GetPriority(DEV_TYPE_TMS,1,BROADCAST_ARRIVE);
	_gtBroadCastPriority.u8TMS_1_arr	   = BROADCAST_GetPriority(DEV_TYPE_TMS,2,BROADCAST_ARRIVE);
	_gtBroadCastPriority.u8TMS_1_pre	   = BROADCAST_GetPriority(DEV_TYPE_TMS,1,BROADCAST_PRE);
	_gtBroadCastPriority.u8TMS_2_pre	   = BROADCAST_GetPriority(DEV_TYPE_TMS,2,BROADCAST_PRE);
	_gtBroadCastPriority.u8TMS_1_urgent    = BROADCAST_GetPriority(DEV_TYPE_TMS,1,BROADCAST_URGENT);
	_gtBroadCastPriority.u8TMS_2_urgent    = BROADCAST_GetPriority(DEV_TYPE_TMS,2,BROADCAST_URGENT);
	_gtBroadCastPriority.u8TMS_1_doorclose = BROADCAST_GetPriority(DEV_TYPE_TMS,1,BROADCAST_CLOSE_DOOR);
	_gtBroadCastPriority.u8TMS_2_doorclose = BROADCAST_GetPriority(DEV_TYPE_TMS,2,BROADCAST_CLOSE_DOOR);

	_gtBroadCastPriority.u8Pics_1_arr	   = BROADCAST_GetPriority(DEV_TYPE_PISC,1,BROADCAST_ARRIVE);
	_gtBroadCastPriority.u8Pics_2_arr	   = BROADCAST_GetPriority(DEV_TYPE_PISC,2,BROADCAST_ARRIVE);
	_gtBroadCastPriority.u8Pics_1_pre      = BROADCAST_GetPriority(DEV_TYPE_PISC,1,BROADCAST_PRE);
	_gtBroadCastPriority.u8Pics_1_pre      = BROADCAST_GetPriority(DEV_TYPE_PISC,2,BROADCAST_PRE);
	_gtBroadCastPriority.u8Pisc_1_doorclose= BROADCAST_GetPriority(DEV_TYPE_PISC,1,BROADCAST_CLOSE_DOOR);
	_gtBroadCastPriority.u8Pisc_2_doorclose= BROADCAST_GetPriority(DEV_TYPE_PISC,2,BROADCAST_CLOSE_DOOR);
	_gtBroadCastPriority.u8Pisc_1_occ	   = BROADCAST_GetPriority(DEV_TYPE_PISC,1,BROADCAST_OCC);
	_gtBroadCastPriority.u8Pisc_1_occ      = BROADCAST_GetPriority(DEV_TYPE_PISC,2,BROADCAST_OCC);

	_gtBroadCastPriority.u8Meida_1         = BROADCAST_GetPriority(DEV_TYPE_PISC,1,BROADCAST_MEDIA);
	_gtBroadCastPriority.u8Meida_2         = BROADCAST_GetPriority(DEV_TYPE_PISC,2,BROADCAST_MEDIA);

	//printf("send..........priority cmd \n");
}

void    BROADCAST_SendVolume(void)
{
	  uint8_t i  = 0;
	  char  acStrTemp[32] = {0x00};
	  //功放音量
	  for(i = 0; i < 16; ++i)
	  {
		  sprintf((char *)acStrTemp,"powamp_%d_vol",i+1);
		  if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"powamp",(const char*)acStrTemp,(char*)acStrTemp,sizeof(acStrTemp) -1) == 0)
		  {
			  _gtBroadCastVolume.acPowAmp[i] = atoi((char*)acStrTemp);
		  }
	  }
	  //控制盒
	  if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"DCP","live_volume",(char*)acStrTemp,sizeof(acStrTemp)-1) == 0)
	  {
		  //口播音量
		  _gtBroadCastVolume.u8DCP_living = atoi((char*)acStrTemp);
	  }

	  if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"dcp","broadcast_monitor_volume",(char*)acStrTemp,sizeof(acStrTemp)-1) == 0)
	  {
		  //口播监听音量
		  _gtBroadCastVolume.u8DCP_Monitor = atoi((char*)acStrTemp);
	  }

	  if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"dcp","talk_monitor_volume",(char*)acStrTemp,sizeof(acStrTemp)-1) == 0)
	  {
		  //司机对讲监听音量
		  _gtBroadCastVolume.u8DCP_Ehp_Monitor = atoi((char*)acStrTemp);
	  }
	  //报警器
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"ehp","talk_volume",(char *)acStrTemp,sizeof(acStrTemp)-1)==0)
	 {
		//话筒音量
		_gtBroadCastVolume.u8Ehp_Talk = atoi((char *)acStrTemp);
	 }		
	 
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"ehp","talk_monitor_volume",(char *)acStrTemp,sizeof(acStrTemp)-1)==0)
	{
		//司机乘客对讲监听音量
		_gtBroadCastVolume.u8Ehp_Monitor = atoi((char *)acStrTemp);
	}	

	//occ
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"occ","occ_volume",(char *)acStrTemp,sizeof(acStrTemp)-1)==0)
	{
		_gtBroadCastVolume.u8Occ = atoi((char *)acStrTemp);
	}	
	//dva dias
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"dva","dva_volume",(char *)acStrTemp,sizeof(acStrTemp)-1)==0)
	{
		_gtBroadCastVolume.u8Dias = atoi((char *)acStrTemp);
	}		
	//media
	if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"media","media_volume",(char *)acStrTemp,sizeof(acStrTemp)-1)==0)
	{
		_gtBroadCastVolume.u8Media = atoi((char *)acStrTemp);
	}	
   //ctrl_port_send_cmd(0xffff,0xff,0xff,BROADCAST_SET_VOL,(uint8 *)&broadcast_vol,sizeof(broadcast_vol_t));
	
}


uint8_t   BROADCAST_GetPriority(uint8_t _u8OpDevType,uint8_t _u8OpDevId,uint8_t _u8TypeBroadCast)
{
	T_BROADCASTPROC_LIST const *ptBdProcList = _gtBroadCastProcessList;
	while(ptBdProcList->u8BdOpDevType)
	{
		//printf("+++++++++++%d\n",_u8TypeBroadCast);
		if(ptBdProcList->u8BdOpDevType == _u8OpDevType && 
		   ptBdProcList->u8BdOpDevId   == _u8OpDevId   && 
		   ptBdProcList->u8BdType      == _u8TypeBroadCast)
		   {
			   printf("++++get Priority:%d----%d\n",ptBdProcList->u8BdPriority,_u8TypeBroadCast);
			   return ptBdProcList->u8BdPriority;
		   }
	ptBdProcList++;
		
	}
	return 0xff;
}

/*音频同步数据发送*/
static _BROADCAST_AudioSyncSendHandle(void)
{
  T_BROADCASTPROC_LIST const *ptBdProcList = _gtBroadCastProcessList;
  while(ptBdProcList->u8BdType)
  {
	  if(ptBdProcList->u8BdType == BROADCAST_GetBroadCastType())
	  {
		  ptBdProcList->pfBroadCastAudioSend();
		  //printf("+++++++++++#########++++++++++++\r\n");
		  break;
	  }
	  ptBdProcList++;
  }

}



static void _BROADCAST_Media(void)
{
    printf("%s:%s\n",__FILE__,__func__);
} 
static void _BROADCAST_OCC(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_Living(void)
{
   printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_LivingStop(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_Urgent(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_UrgentStop(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_DoorClose(void)
{
    printf("%s:%s\n",__FILE__,__func__);   
}
static void _BROADCAST_PreArrival(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_Arrived(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_OCCStop(void)
{
   printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_DoorCloseStop(void)
{
   printf("%s:%s\n",__FILE__,__func__); 
}
static void _BROADCAST_PreArrivalStop(void)
{
  printf("%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_ArrivalStop(void)
{
 printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s:%s\n",__FILE__,__func__);
}
static void _BROADCAST_MediaStop(void)
{
    printf("%s:%s\n",__FILE__,__func__);
}

static void     _BROADCAST_Set_MP3DecodeFlag(uint8_t _u8Flag)
{
    if(_gtBroadCastCurrent.u8MP3DecodeFlag != _u8Flag)
    {
        _gtBroadCastCurrent.u8MP3DecodeFlag = _u8Flag;
    }

}

static uint8_t  _BROADCAST_Get_MP3DecodeFlag(void)
{
    return _gtBroadCastCurrent.u8MP3DecodeFlag;
}


T_BROADCAST_SERVICE   gtBroadCastService;
void    BROADCAST_TimerOut_Process(void *_pvEventCtl, T_EV_TIMER *_ptEventTimer,  void *_pvArg)
{
   
    T_BROADCAST_SERVICE* ptBroadCastService = _pvArg;
    T_MAINSERVER *ptMainServer              = ptBroadCastService->ptMainServer;
    T_EVENT_CTL * ptEventCtl                = (T_EVENT_CTL * )_pvEventCtl;
    zlog_info(ptMainServer->ptZlogCategory,"+++++++++++BROADCAST_TimerOut_Process!!!\n");

	if(PISC_LOCAL_GetMasterFlag())//主
	{
		if(_BROADCAST_Get_MP3DecodeFlag())//解码播放MP3广播
		{
			printf("BD:+++++++++MP3DecodeBroadcast...BDType.%x.\n",BROADCAST_GetBroadCastType());

			if(MP3_Decoder_Get_IsDecoding() ==NODECODING)//
			{
				//解码完成
				if(BROADCAST_GetBroadCastCycleFlag())//循环播放标志
				{
					BROADCAST_Process(BROADCAST_GetBroadCastOperateDevType(),BROADCAST_GetBroadCastOperateDevId(),BROADCAST_GetBroadCastType());
				}else   //解码完成 就把广播设置为空，此处需要加入一个判断就是解码缓冲区 发送为空才去设置为空或者发送解码缓冲区放在解码里面进行同步操作
				{
					printf("BD:+++++++++NONE CYCLE.......%x.\n", BROADCAST_GetBroadCastType());
					if(BROADCAST_NONE != BROADCAST_GetBroadCastType())
					{
						BROADCAST_StopProcess(BROADCAST_GetBroadCastType());
					}
				}
			}
			
			
		}
	}
	
	EVIO_EventTimer_Init(_ptEventTimer,1000,BROADCAST_TimerOut_Process,ptBroadCastService);
    EVIO_EventTimer_Start(ptEventCtl,_ptEventTimer);
}


void _BROADCAST_UDPNanomsgHandle(T_EVENT_CTL *_ptEventCtl, T_BROADCAST_NANOMSGFDS *_ptBroadCastNanoMsgFd)
{ 
	uint8_t *dat = NULL;
	T_BROADCAST_SERVICE* ptBroadCastService   = _ptBroadCastNanoMsgFd->pvArg;
	T_MAINSERVER *ptMainServer = ptBroadCastService->ptMainServer;
	uint32_t bytes = nn_recv(_ptBroadCastNanoMsgFd->iNanomsgFd, &dat, NN_MSG, NN_DONTWAIT);
	if (-1 != bytes) 
	{
		zlog_info(ptMainServer->ptZlogCategory,"UDP---------XXXXXXXXXXXXXXXXXXXXX------->Broadcast++%d<type:%d>\n",bytes,dat[1]);
		switch(dat[1])
		{
			case MSG_TYPE_TMSUDP:
			{
				printf("+++++++++++++++++++++dat[2:%d \r\n",dat[2] );
				if(dat[2] == BROADCAST_PLAY)
				{
					BROADCAST_Process(dat[3],dat[4],dat[5]);
				}else if(dat[2] == BROADCAST_STOP)
				{
					//[2]: _u8OpType;[3] :_u8DevType;[4] :_u8DevId;[5] : _u8BdType;
					printf("STOPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP\r\n");
					BROADCAST_StopProcess(dat[5]);
				}
				break;
			}
			case MSG_TYPE_FEPAUDIO://Fep Audio Sync BroadCast Send
			{
				_BROADCAST_AudioSyncSendHandle();
				break;
			}
			case MSG_TYPE_OCCBD:
			{
			  if(dat[2] == BROADCAST_PLAY)
			  {
			   BROADCAST_Process(dat[3],dat[4],BROADCAST_OCC);
			  }else
			   BROADCAST_StopProcess(BROADCAST_OCC);
			  break;
			}
			

			default:
			    break;
		}
		nn_freemsg(dat);
	}
}

void _BROADCAST_EventNanoMsgCallBack(T_EVENT_CTL *_ptEventCtl, T_EVENT_FD *_ptEventFd, int _iFd, E_EV_TYPE _eType, void *_pvArg)
{
	T_BROADCAST_NANOMSGFDS *ptNanoMsgFdsUdp2BroadCast = _pvArg;
	T_MAINSERVER *ptMainServer = ptNanoMsgFdsUdp2BroadCast->pvArg;
    //zlog_debug(ptMainServer->ptZlogCategory,"+++++++++++++++++++++++++++++++Main_EventLoop_Handle\n");
    switch (_eType) {
	    case E_EV_READ:
	        ptNanoMsgFdsUdp2BroadCast->pfCallBack(_ptEventCtl, ptNanoMsgFdsUdp2BroadCast);
	        break;
		case E_EV_WRITE:
			zlog_warn(ptMainServer->ptZlogCategory,"Broadcast write event, unexpected\n");
			break;
		case E_EV_ERROR:
			zlog_error(ptMainServer->ptZlogCategory,"Broadcast error event, unexpected\n");
			break;
	    default:
			zlog_warn(ptMainServer->ptZlogCategory,"Broadcast unknow event, unexpected\n");
    }
}

static int32_t _BROADCAST_NanomsgSocket_Init(T_BROADCAST_SERVICE  *_ptBroadCastService)
{
	size_t size = sizeof(size_t);
   T_MAINSERVER *ptMainServer     = ((T_BROADCAST_SERVICE *) _ptBroadCastService)->ptMainServer;
	_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd = nn_socket(AF_SP, NN_PAIR);
	if (-1 == _ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd)
	{
		zlog_error(ptMainServer->ptZlogCategory,"+++BROADCAST nn_socket error...\n");
		return -1;
	}
	if (-1 == nn_connect(_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd, "inproc://main<->broadcast"))
	{
		nn_close(_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd);
		zlog_error(ptMainServer->ptZlogCategory,"+++BROADCAST nn_bind error...\n");
		return -1;
	}
	
	if (-1 == nn_getsockopt(_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd, NN_SOL_SOCKET, NN_RCVFD,
										 (char *)&_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iSysFd, &size))
	{
     	nn_close(_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd);
		zlog_error(ptMainServer->ptZlogCategory,"+++++BROADCAST nn_getsockopt error...\n");
		return -1;
	}
	_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.pfCallBack= _BROADCAST_UDPNanomsgHandle;//UDP enventloop broadcast 
    _ptBroadCastService->tNanoMsgFdsUdp2BroadCast.pvArg     = _ptBroadCastService;
    _ptBroadCastService->tNanoMsgFdsUdp2BroadCast.ptEventFd = EVIO_EventFd_Add(_ptBroadCastService->ptEventCtl,\
													  _ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iSysFd, \
													  _BROADCAST_EventNanoMsgCallBack, &_ptBroadCastService->tNanoMsgFdsUdp2BroadCast);//add the event fd 
	if (NULL == _ptBroadCastService->tNanoMsgFdsUdp2BroadCast.ptEventFd)
	{
	
		zlog_error(ptMainServer->ptZlogCategory,"+++_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.ptEventFd error\n");
		close(_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iSysFd);
		nn_close(_ptBroadCastService->tNanoMsgFdsUdp2BroadCast.iNanomsgFd);
		return -1;
	}
	 EVIO_Event_Watch_Read(_ptBroadCastService->ptEventCtl, _ptBroadCastService->tNanoMsgFdsUdp2BroadCast.ptEventFd);
	return 0;
}
void  *BROADCAST_Service_ThreadHandle(void *_pvArg)
{
   T_MAINSERVER *ptMainServer     = (T_MAINSERVER *) _pvArg;
   gtBroadCastService.ptEventCtl   = EVIO_EventCtl_Create();  //创建一个事件控制器
   gtBroadCastService.ptMainServer = ptMainServer;

   if(NULL == gtBroadCastService.ptEventCtl)
	 {
		 zlog_error(ptMainServer->ptZlogCategory,"+++BroadCast Service  EventCtl Failed\n");
		 return NULL;
	 }
	zlog_info(ptMainServer->ptZlogCategory,"BROADCAST Service Thread Creator Success!!!\n");
    if(_BROADCAST_NanomsgSocket_Init(&gtBroadCastService) != 0)
	{
		EVIO_EventCtl_Free(gtBroadCastService.ptEventCtl);
		return NULL;
	}
	
	EVIO_EventTimer_Init(&gtBroadCastService.tEventTimer,1000,BROADCAST_TimerOut_Process,&gtBroadCastService);
	EVIO_EventTimer_Start(gtBroadCastService.ptEventCtl,&gtBroadCastService.tEventTimer);

    pthread_mutex_lock(&ptMainServer->tThread_StartMutex);
	++ptMainServer->iThread_bStartCnt;
	pthread_cond_signal(&ptMainServer->tThread_StartCond);
	pthread_mutex_unlock(&ptMainServer->tThread_StartMutex);
	while(1)
     { 

        EVIO_EventCtlLoop_Start(gtBroadCastService.ptEventCtl);
     }
    EVIO_EventCtl_Free(gtBroadCastService.ptEventCtl);
    return NULL;
}


void    BROADCAST_AudioSend(uint8_t *_pcBuffer,uint16_t _u16Len)
{
    uint8_t acSendBuffer[PIS_DATA_MAX_SIZE];
	T_AUDIOBD tAudioBroadCast;
	uint16_t u16SendLen = 0;
	uint8_t u8OpDevId  = BROADCAST_GetBroadCastOperateDevId();
	uint8_t u8OpDevType = BROADCAST_GetBroadCastOperateDevType();
	tAudioBroadCast.u8BdType     = BROADCAST_GetBroadCastType();
	tAudioBroadCast.u8BdPriority = BROADCAST_GetPriority(u8OpDevType,u8OpDevId,tAudioBroadCast.u8BdType);
	memcpy((uint8_t*)&tAudioBroadCast.acAudioBuf,_pcBuffer,_u16Len);

	//封包
	PIS_PACK_DataPacket(0,0xff,0xff,BROADCAST_AUDIO_REMOTE_IP_HEX,
                        0x001,DEV_TYPE_PISC,PISC_LOCAL_GetDevId(),PISC_LOCAL_GetDevIp(),
                        AUDIO_SEND_CMD,(uint8_t*)&tAudioBroadCast,sizeof(T_AUDIOBD),acSendBuffer,&u16SendLen);


	AudioPortBD_SendData(acSendBuffer,u16SendLen);
}