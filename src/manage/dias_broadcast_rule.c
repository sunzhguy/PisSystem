/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-11 11:29:11
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 14:38:20
 */
#include "mp3_decoder.h"
#include "broadcast.h"
#include "../include/pis_config.h"
#include "../teminal_device/pisc_local.h"
#include "dias_broadcast_rule.h"

#define  DIAS_BROADCAST_URGENT_SHOW_NUM_MAX  (FILE_LIST_NUM_MAX)

//预到站
#define DIAS_BROADCAST_RULE_PRE_WELCOME_INDEX		(000) //欢迎词，中文，预到站
#define DIAS_BROADCAST_RULE_PRE_DIR_INDEX			(100) //中文，方向，预到站
#define DIAS_BROADCAST_RULE_PRE_NON_DES_INDEX		(200) //中文，非目的站，预到站
#define DIAS_BROADCAST_RULE_PRE_DES_INDEX			(300) //中文，目的地，预到站
#define DIAS_BROADCAST_RULE_PRE_END_INDEX			(400) //中文，终点站，预到站

//到站
#define DIAS_BROADCAST_RULE_ARR_NON_DES_INDEX		(000) //中文，非目的地，到站
#define DIAS_BROADCAST_RULE_ARR_DES_INDEX			(100) //中文，目的地，到站



void DiasBroadCastRule_GetPreList(uint16_t* _pu16List,uint8_t* _pcLanuageList,uint16_t * _pu16ListNum)
{

	uint8_t index = 0;
	T_STATIONINFO tStationInfo;
	PISC_LOCAL_GetStationInfo(&tStationInfo);
	
	_pu16List[index]          = DIAS_BROADCAST_RULE_PRE_WELCOME_INDEX + tStationInfo.u8NexStation;
	_pcLanuageList[index]     = LANGUAGE_C;
	index++;

	_pu16List[index]          = DIAS_BROADCAST_RULE_PRE_DIR_INDEX    +  tStationInfo.u8EndStation;
	_pcLanuageList[index]     = LANGUAGE_C;
	index++;

	if(tStationInfo.u8NexStation != tStationInfo.u8EndStation)
	{
		_pu16List[index]      = DIAS_BROADCAST_RULE_ARR_NON_DES_INDEX +  tStationInfo.u8NexStation;
		_pcLanuageList[index] = LANGUAGE_C;
		index++;
	}
	else
	{
		_pu16List[index]      = DIAS_BROADCAST_RULE_PRE_DES_INDEX + tStationInfo.u8NexStation;
		_pcLanuageList[index] = LANGUAGE_C;
		index++;
	}
	_pu16List[index]          = DIAS_BROADCAST_RULE_PRE_END_INDEX + tStationInfo.u8NexStation;
	_pcLanuageList[index]     = LANGUAGE_C;
	index++;

	//拼接段数
	*_pu16ListNum = index;
}

void DiasBroadCastRule_GetArrList(uint16_t* _pu16List,uint8_t* _pcLanuageList,uint16_t * _pu16ListNum)
{

	uint8_t index=0;
	T_STATIONINFO tStationInfo;
	PISC_LOCAL_GetStationInfo(&tStationInfo);
	
	if(tStationInfo.u8NexStation != tStationInfo.u8EndStation)
	{
		_pu16List[index]      = DIAS_BROADCAST_RULE_ARR_NON_DES_INDEX + tStationInfo.u8NexStation;
		_pcLanuageList[index] = LANGUAGE_C;
		index++;
	}
	else
	{
		_pu16List[index]      = DIAS_BROADCAST_RULE_ARR_DES_INDEX + tStationInfo.u8NexStation;
		
		_pcLanuageList[index] = LANGUAGE_C;
		index++;
	}

	//拼接段数
	*_pu16ListNum = index;
}

void DiasBroadCastRule_GetUrgentList(uint16_t* _pu16List,uint8_t* _pcLanuageList,uint16_t * _pu16ListNum)
{
	_pu16List[0]       =  BROADCAST_GetUrgentCode();
	_pcLanuageList[0]  = LANGUAGE_C;
	_pu16List[1]      = BROADCAST_GetUrgentCode();
	_pcLanuageList[1] =LANGUAGE_E;
	*_pu16ListNum=1;
}

