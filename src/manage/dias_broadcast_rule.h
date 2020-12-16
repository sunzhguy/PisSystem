/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-11 11:29:26
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-11 11:49:18
 */


#ifndef DIAS_BROADCAST_RULE_H
#define DIAS_BROADCAST_RULE_H

#include "../include/general.h"

void DiasBroadCastRule_GetPreList(uint16_t* _pu16List,uint8_t* _pcLanuageList,uint16_t * _pu16ListNum);
void DiasBroadCastRule_GetArrList(uint16_t* _pu16List,uint8_t* _pcLanuageList,uint16_t * _pu16ListNum);
void DiasBroadCastRule_GetUrgentList(uint16_t* _pu16List,uint8_t* _pcLanuageList,uint16_t * _pu16ListNum);

#endif