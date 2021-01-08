/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-07 08:32:42
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-07 11:11:31
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gpio.h"

#define  MAX_GPIO_CNT  30




/*gpio init 将相关gpio export  初始化为输入输出 默认值等等*/
const char *const GPIOSysfsPath = "/sys/class/gpio";


/*export gpio*/
static int _GPIO_Export(const int _iGpio)
{
  char acValue[5];
  int iRet, iFd;
  char acFileName[100];
  char acGpioName[30];
  memset(acFileName,0,30);
  memset(acGpioName,0,30);
  sprintf(acFileName, "%s/export", GPIOSysfsPath);
  iFd = open(acFileName, O_WRONLY);
  if (iFd < 0)
  {
    return -1;
  }
  sprintf(acGpioName, "%s/gpio%d", GPIOSysfsPath, _iGpio);
  if (!access(acGpioName, 0))
    return 0;
  sprintf(acValue, "%d", _iGpio);
  iRet = write(iFd, acValue, strlen(acValue));
  if (iRet < 0)
  {
    close(iFd);
    return -1;
  }
  fsync(iFd);
  close(iFd);
  return 0;
}

/**
 * @descripttion: set gpio direction
 * @param {gpio:gpio num value: "in","out"} 
 * @return: success :0 failed <0
 */
static int _GPIO_SetDirection(const int _iGpio, const char *_pcValue)
{
  int iRet, iFd;
  char acFileName[100];
  sprintf(acFileName, "%s/gpio%d/direction", GPIOSysfsPath, _iGpio);
  iFd = open(acFileName, O_WRONLY);
  if (iFd < 0)
  {
    return -1;
  }
  iRet = write(iFd, _pcValue, strlen(_pcValue));
  if (iRet < 0)
  {
    close(iFd);
    return -1;
  }
  fsync(iFd);
  close(iFd);
  return 0;
}


static int _GPIO_WriteValue(const int _iGpio, const int _iValue)
{
  int iRet, iFd;
  char acFileName[100];
  char chrValue;
  sprintf(acFileName, "%s/gpio%d/value", GPIOSysfsPath, _iGpio);
  iFd = open(acFileName, O_WRONLY);
  if (iFd < 0)
  {
    return -1;
  }
  chrValue = (_iValue == 1) ? '1' : '0';
  iRet = write(iFd, &chrValue, 1);
  if (iRet < 0)
  {
    close(iFd);
    return -1;
  }
  fsync(iFd);
  close(iFd);
  return 0;
}

static int _GPIO_GetValue(const int _iGpio)
{
    int iFd  = -1;
    int iRet = -1;
    unsigned char u8Value;
    char acGPIOPath[60];
    sprintf(acGPIOPath, "%s/gpio%d/value", GPIOSysfsPath, _iGpio);
    iFd = open(acGPIOPath, O_RDONLY | O_NONBLOCK);
    if (iFd < 0)
    {
        return -1;
    }
    iRet = read(iFd, &u8Value, 1);
    if (iRet < 0)
    {
        close(iFd);
        return -1;
    }
    fsync(iFd);
    close(iFd);
    return (int)(u8Value - '0');
}

int  GPIO_Init(const int _iGpio,const int _iDir)
{ 
 int iRet = -1;
 if (_GPIO_Export(_iGpio) == 0)
    {
      if (_iDir == 0)
      {
        if (_GPIO_SetDirection(_iGpio, "out") != 0)
        {
          
          printf("[gpio]: error init gpio:%d,set_direction:%d\r\n", _iGpio,_iDir);
        }else
        {
            iRet = 0;
        }
        
      }
      else
      {
        if (_GPIO_SetDirection(_iGpio, "in") != 0)
        {
          iRet = -1;
          printf("[gpio]:error init gpio:%d,set_direction:%d\r\n", _iGpio, _iDir);
        }else
        {
              iRet = 0;
        }
        
      }
    }
    return iRet;
}

int  GPIO_IO_Ctrl(const int _iGpio,const int _iValue)
{

    return _GPIO_WriteValue(_iGpio, _iValue);
    
}

int  GPIO_IO_Value(const int _iGpio)
{

    return _GPIO_GetValue(_iGpio);

}
int  GPIO_IO_Toggle(const int _iGpio)
{
    int ivalue =  _GPIO_GetValue(_iGpio);
    printf("_iGpio[%d]ivalue:%d\r\n",_iGpio,ivalue);
    if(ivalue >= 0)
    {
     return _GPIO_WriteValue(_iGpio, !ivalue);
    }
    return -1;
}