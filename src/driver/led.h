/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-07 11:12:01
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-07 14:11:39
 */
#ifndef _LED_H
#define _LED_H


#define  LED_ON    0x00
#define  LED_OFF   0x01

#define  ERR_STA   0x01      
#define  NOR_STA   0x00


#define  LED_SYS_RUN        (0)
#define  LED_MANUAL         (1)
#define  LED_TMS            (2)
#define  LED_ATC            (3)
#define  LED_OCC            (4)
#define  LED_ACTIVE         (5)
#define  LED_MIC            (6)
#define  LED_ERR            (7)

void LED_Init(void);

int  LED_Ctrl(const int _iLed,const int _iStatus);

int  LED_Toggle(const int _iLed);


#endif