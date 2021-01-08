/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-07 11:12:05
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-07 14:09:50
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "led.h"
#include "../include/general.h"


uint8_t  gu8LED_Status = 0x00;



static int  _LED_IO_CTRL(const int _iled,const int _iValue)
{
	int iGpio = 0;
	if(_iValue == LED_ON)
	{
		gu8LED_Status &= ~(1<<_iled);
	}else
	{
		gu8LED_Status |= (1<<_iled);
	}
	
	switch (_iled)
	  {
		
		case LED_SYS_RUN:
			iGpio = IO_SYS_RUN;
		  	break;
		case LED_MANUAL:
		    iGpio = IO_MANUAL;
			break;
		case LED_TMS:
		    iGpio = IO_TMS;
			break;
		case LED_ATC:
			iGpio = IO_ATC;
			break;
		case LED_OCC:
			iGpio = IO_OCC;
			break;
		case LED_ACTIVE:
		    iGpio = IO_ACTIVE;
			break;
		case LED_MIC:
			iGpio = IO_MIC;
			break;
		case LED_ERR:
			iGpio = IO_ERR;
			break;
	   default:
	   	     break;
	  }
	  return GPIO_IO_Ctrl(iGpio,_iValue);
}

static int  _LED_GetValue(const int _iled)
{
	
	return (gu8LED_Status >>_iled)&0x01;

}



void LED_Init(void)
{


    GPIO_Init(IO_SYS_RUN,0);
	GPIO_Init(IO_MANUAL,0);
    GPIO_Init(IO_TMS,0);
	GPIO_Init(IO_ATC,0);
	GPIO_Init(IO_OCC,0);
	GPIO_Init(IO_ACTIVE,0);
	GPIO_Init(IO_MIC,0);
	GPIO_Init(IO_ERR,0);
   
    _LED_IO_CTRL(LED_SYS_RUN,LED_OFF);
	_LED_IO_CTRL(LED_MANUAL,LED_OFF);
	_LED_IO_CTRL(LED_TMS,LED_OFF);
	_LED_IO_CTRL(LED_ATC,LED_OFF);
	_LED_IO_CTRL(LED_OCC,LED_OFF);
	_LED_IO_CTRL(LED_ACTIVE,LED_OFF);
	_LED_IO_CTRL(LED_MIC,LED_OFF);
	_LED_IO_CTRL(LED_ERR,NOR_STA);
    

}

int  LED_Ctrl(const int _iLed,const int _iStatus)
{

	return 	_LED_IO_CTRL(_iLed,_iStatus);
    
}

int  LED_Toggle(const int _iLed)
{
	int value = _LED_GetValue(_iLed);

	value  = !value;
    _LED_IO_CTRL(_iLed,value);
	
}