/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 15:44:00
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 09:31:07
 */

#ifndef OCC_H
#define OCC_H
#include "../include/general.h"


void OCC_Init(void);
//void OCC_OCCStatusSet(uint8_t _u8Flag);
void OCC_OCCStatusSet(void* _ptIOInPutEvCtl,uint8_t _u8Flag);
void OCC_AudioSend(void);

#endif

