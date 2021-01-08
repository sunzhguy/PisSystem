/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-07 14:38:50
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-08 11:36:06
 */

#ifndef _CTRL_UDPORT_H
#define _CTRL_UDPPORT_H
#include "../include/general.h"


#define  UDP_LOCAL_PORT    50152
#define  UDP_REMOTE_PORT   50152
#define  BROADCAST_IP      "168.168.102.255"



void CTRL_UDPORT_ReadAFrameData(uint8_t *_pcBufer,uint32_t _u32BufLen);
void CTRL_UDPORT_SendAFrameData(uint16_t  _u16DstTrainId,uint8_t _u8DstDevType,uint8_t _u8DstDevId,uint16_t _u16Cmd,uint8_t *_pcBuf,uint16_t _u16Len);

#endif