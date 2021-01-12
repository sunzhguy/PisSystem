/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-07 08:32:49
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 09:39:09
 */


#ifndef _GPIO_H
#define _GPIO_H

#define   GPIO(m)        (m)
#define   PIN(n)         (n)
#define   GPIO_PIN(x,y)  ((GPIO(x)-1)*32 +PIN(y))

#define  IO_SYS_RUN        GPIO_PIN(GPIO(3),PIN(0))
#define  IO_MANUAL         GPIO_PIN(GPIO(5),PIN(0))
#define  IO_TMS            GPIO_PIN(GPIO(5),PIN(1))
#define  IO_ATC            GPIO_PIN(GPIO(5),PIN(2))
#define  IO_OCC            GPIO_PIN(GPIO(5),PIN(3))
#define  IO_ACTIVE         GPIO_PIN(GPIO(5),PIN(4))
#define  IO_MIC            GPIO_PIN(GPIO(5),PIN(7))
#define  IO_ERR            GPIO_PIN(GPIO(5),PIN(8))


#define  MODE_NONE      0
#define  MODE_RISING    1  //
#define  MODE_FALLING   2
#define  MODE_BOTH      3



#define  IO_R_OPEN4         GPIO_PIN(GPIO(4),PIN(24)) //---
#define  IO_L_OPEN3         GPIO_PIN(GPIO(4),PIN(23)) //--

#define  IO_SPEED2          GPIO_PIN(GPIO(1),PIN(30))
#define  IO_KEY1            GPIO_PIN(GPIO(1),PIN(31))
#define  IO_CTRL            GPIO_PIN(GPIO(1),PIN(28)) //司机占有
#define  IO_CL_CTRL         GPIO_PIN(GPIO(1),PIN(29)) //重连

#define  IO_CLOSE5          GPIO_PIN(GPIO(1),PIN(1)) //
#define  IO_OCC_ACT6        GPIO_PIN(GPIO(1),PIN(2)) //
#define  IO_POWER_DWN       GPIO_PIN(GPIO(1),PIN(18)) //掉电检测

#define  IO_CPU_ADDR1       GPIO_PIN(GPIO(1),PIN(8)) //ADDR1
#define  IO_CPU_ADDR2       GPIO_PIN(GPIO(1),PIN(9)) //ADDR2
#define  IO_CPU_ADDR3       GPIO_PIN(GPIO(1),PIN(5)) //ADDR3
#define  IO_CPU_ADDR4       GPIO_PIN(GPIO(1),PIN(3)) //ADDR4

int  GPIO_Init(const int _iGpio,const int _iDir);

int  GPIO_IO_Ctrl(const int _iGpio,const int _iValue);

int  GPIO_IO_TrigCtl(const int _iGpio,const int _mode);

int  GPIO_IO_Value(const int _iGpio);

int  GPIO_IO_GetReadFd(const int _iGpio);

int  GPIO_IO_Toggle(const int _iGpio);


#endif