/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-07 14:38:50
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 14:46:59
 */

#ifndef _CTRL_UDPORT_H
#define _CTRL_UDPPORT_H
#include "../include/general.h"

void CTRL_UDPORT_ReadAFrameData(uint8_t *_pcBufer,uint32_t _u32BufLen);
void CTRL_UDPORT_SendAFrameData(uint16_t  _u16DstTrainId,uint8_t _u8DstDevType,uint8_t _u8DstDevId,uint16_t _u16Cmd,uint8_t *_pcBuf,uint16_t _u16Len);

#endif