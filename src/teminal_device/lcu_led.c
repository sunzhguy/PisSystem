/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-11 11:13:22
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-14 14:20:08
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcu_led.h"
#include "../lib/utils.h"
#include "../include/general.h"
#include "../include/pis_config.h"
#include "../lib/file_operator.h"
#include "../lib/ini_file.h"
#include "../manage/dev_status.h"
#include "../teminal_device/pisc_local.h"
#include "../manage/dias_broadcast_rule.h"
#include "../manage/broadcast.h"

#define LCU_LED_SHOW_CMD            (0X0009)
#define LCU_LED_PACK_NUM_MAX        (5)
#define LCU_LED_PACK_CONTENT_MAX    (1024)
#define LCU_LED_CONTENT_MAX         (10240)

#define LCU_LED_LANGUAGE_NUM_MAX    (2)

typedef struct {
  uint8_t   u8BroadCastType;
  uint8_t   u8Language;
  const uint8_t acFileName[16];
  void  (*pfGetLedShowList)(uint16_t *,uint8_t *,uint16_t *);//
}T_LCU_LED_SHOWTYPE;

static const T_LCU_LED_SHOWTYPE _gtLcuLedShowTypeTable[]=
{
    {BROADCAST_PRE,LANGUAGE_C,"1/pre/",DiasBroadCastRule_GetPreList},
    {BROADCAST_PRE,LANGUAGE_E,"2/pre/",DiasBroadCastRule_GetPreList},
    {BROADCAST_ARRIVE,LANGUAGE_C,"1/arrive/",DiasBroadCastRule_GetArrList},
    {BROADCAST_ARRIVE,LANGUAGE_E,"2/arrive/",DiasBroadCastRule_GetArrList},
    {BROADCAST_URGENT,LANGUAGE_C,"1/urgent/",NULL},
    {BROADCAST_URGENT,LANGUAGE_E,"2/urgent/",NULL},
    {0,0,"",NULL},
};

typedef struct{
    uint8_t u8PacketNum;
    uint8_t u8CurPacketNum;
    uint8_t u8Color;
    uint8_t acContent[LCU_LED_PACK_CONTENT_MAX];
}T_LCU_LED_PACKET;


typedef struct{
    uint8_t  u8Language;
    uint8_t  u8Color;
    uint32_t u32ContentLen;
    uint8_t  acContent[LCU_LED_CONTENT_MAX];
}T_LCU_LED_SEND_CONTENT;


static T_LCU_LED_SEND_CONTENT _gtLCU_LED_ContentSend =
{
    LANGUAGE_C,
    1,
    0,
    {0},
};

void LCU_LED_Init(void)
{

    //初始化设备有效性
    DEV_STATUS_InitDevValid(DEV_TYPE_LCU_LED);

}
void LCU_LED_CoontentSet(uint8_t *_pcPath,uint16_t *_pu16List,uint16_t _u16ListNum)
{
    
    uint8_t acFileNameStr[128] ={0x00};
    uint8_t i = 0;
    uint16_t u16FileLen = 0;
    int iTempLen = 0;
    uint8_t acTempBuf[10240] = {0x00};
    uint8_t acTempHexBuf[10240] = {0x00};
    //清除数据
    _gtLCU_LED_ContentSend.u32ContentLen = 0;
    printf("++++LcuLedPath:%s\r\n",_pcPath);
    
    for(i = 0; i< _u16ListNum; ++i)
    {
        //配置文件名
        if(0 == i)
        sprintf((char *)acFileNameStr,"%s/%d/%03d.conf",_pcPath,LANGUAGE_C,_pu16List[i]);
        else
        sprintf((char *)acFileNameStr,"%s/%d/%03d.conf",_pcPath,LANGUAGE_E,_pu16List[i]);
        //显示内容
        FileOperator_Read((const char *)acFileNameStr,(char*)acTempBuf,&iTempLen);
        u16FileLen = iTempLen;
        Utils_BufferStrToHex(acTempHexBuf,acTempHexBuf,u16FileLen,&u16FileLen);
        _gtLCU_LED_ContentSend.u8Color = acTempHexBuf[0];
        memcpy((uint8_t*)&_gtLCU_LED_ContentSend.acContent[_gtLCU_LED_ContentSend.u32ContentLen],(uint8_t*)&acTempHexBuf[1],u16FileLen);
        _gtLCU_LED_ContentSend.u32ContentLen += u16FileLen;
    }


}

//设置客室屏的显示内容
void LCU_LED_ContentSend()
{
    uint8_t i  = 0;
    T_LCU_LED_PACKET tLCU_LEDPacket;
    uint16_t u16SendLen    = 0;
    uint16_t u16ContentLen = 0;
    uint8_t  u8PacketNum = (_gtLCU_LED_ContentSend.u32ContentLen/(LCU_LED_CONTENT_MAX+1));

    for( i = 0; i < u8PacketNum; ++i)
    {
        if( i < (u8PacketNum -1) )
        u16ContentLen = LCU_LED_CONTENT_MAX;
        else
        u16ContentLen = (_gtLCU_LED_ContentSend.u32ContentLen % LCU_LED_CONTENT_MAX);

        //填充数据
        tLCU_LEDPacket.u8PacketNum    = u8PacketNum;
        tLCU_LEDPacket.u8CurPacketNum = (i+1);
        tLCU_LEDPacket.u8Color        = _gtLCU_LED_ContentSend.u8Color;
        memcpy(tLCU_LEDPacket.acContent,(uint8_t*)&_gtLCU_LED_ContentSend.acContent[LCU_LED_CONTENT_MAX*i],u16ContentLen);
        
        //发送数据长度
        u16SendLen = sizeof(T_LCU_LED_PACKET) - LCU_LED_CONTENT_MAX + u16ContentLen;
        
        //发送显示数据
        printf("u16SendLen =====%d\n",u16SendLen);
    }
    

    
}

void LCU_LED_RollSecondSet()
{
   uint16_t acTempStr[4]= {0x00};

   //客室滚动速度
   if(INI_FILE_GetValueFromConfig(PISC_CONF_PATH,"lcu_led","speed_time",(char*)acTempStr,sizeof(acTempStr) -1) == 0)
   {
       PISC_LOCAL_SetLCURollSec(atoi ((char *)acTempStr));
   }

}