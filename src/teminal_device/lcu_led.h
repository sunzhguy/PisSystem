/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-11 11:13:16
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 14:03:03
 */

#ifndef  LCU_LED_H
#define  LCU_LED_H
#include "../include/general.h"
void LCU_LED_Init(void);
void LCU_LED_RollSecondSet();
void LCU_LED_CoontentSet(uint8_t *_pcPath,uint16_t *_pu16List,uint16_t _u16ListNum);
void LCU_LED_ContentSend();

#endif