/*
 * @Descripttion: Digital station announcer 数字报站器
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 16:22:38
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 13:55:33
 */

#ifndef DIAS_H
#define DIAS_H

#include "../include/general.h"
void DIAS_Init(void);
void DIAS_PlayList(uint16_t *pu16List,uint16_t _u16ListNum);
void DIAS_Stop(void);
void DIAS_AudioSend(void);

#endif