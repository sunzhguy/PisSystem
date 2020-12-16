/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 09:14:09
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 14:52:57
 */

#ifndef _PISC_LOCALDEV_H
#define _PISC_LOCALDEV_H
#include <stdint.h>

#include "../operate_device/pisc.h"

uint8_t   PISC_LOCAL_GetDevId(void);


uint8_t  PISC_LOCAL_GetMasterFlag(void);
void     PISC_LOCAL_SetMasterFlag(uint8_t _u8Status);

uint8_t  PISC_LOCAL_SetLeftDoorOpeneStatus(uint8_t _u8Status);
uint8_t  PISC_LOCAL_SetRightDoorOpeneStatus(uint8_t _u8Status);

uint32_t PISC_LOCAL_GetDevIp(void);
uint8_t  PISC_LOCAL_GetOtherDevId(void);//PISC 获取另外一个PISC设备ID

void     PISC_LOCAL_SetKeyStatus(uint8_t _u8Flag);
uint8_t  PISC_LOCAL_GetKeyStatus(void);

void     PISC_LOCAL_SetStationInfo(T_STATIONINFO _tStationInfo);
void     PISC_LOCAL_GetStationInfo(T_STATIONINFO *_ptStationInfo);

void     PISC_LOCAL_SetRunDir(uint8_t _u8UpDown);
uint8_t  PISC_LOCAL_GetRunDir(void);

void     PISC_LOCAL_ProcessDataInit(void);

void     PISC_LOCAL_SetWorkMode(uint8_t _u8Mode);
uint8_t  PISC_LOCAL_GetWorkMode(void);


void     PISC_LOCAL_SetPreFlag(uint8_t _u8Flag);
uint8_t  PISC_LOCAL_GetPreFlag(void);

void     PISC_LOCAL_SetArrFlag(uint8_t _u8Flag);
uint8_t  PISC_LOCAL_GetArrFlag(void);

void     PISC_LOCAL_SetLCURollSec(uint8_t _u8RollSec);
uint8_t  PISC_LOCAL_GetLCURollSec(void);

uint8_t  PISC_LOCAL_GetOpenLeftDoorFlag(void);
void     PISC_LOCAL_SetOpenLeftDoorFlag(uint8_t _u8Flag);

uint8_t  PISC_LOCAL_GetOpenRightDoorFlag(void);
void     PISC_LOCAL_SetOpenRightDoorFlag(uint8_t _u8Flag);

uint8_t  PISC_LOCAL_GeCloseLeftDoorFlag(void);
uint8_t  PISC_LOCAL_GeCloseRightDoorFlag(void);

void     PISC_LOCAL_SetCloseDoorFlag(uint8_t _u8Flag);

void     PISC_LOCAL_SendProcessData(void);
void     PISC_LOCAL_SetTime(uint8_t *_pcTimeBuf, uint16_t _u16TimeLen);
void     PISC_LOCAL_GetTime(uint8_t *_pcTimeBuf);
void     PISC_LOCAL_SendTime(void);
uint32_t PISC_LOCAL_GetJumpStation(void);
void     PISC_LOCAL_SetJumpStation(uint32_t _u32JumpStations);
#endif