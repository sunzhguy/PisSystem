/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 11:19:20
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 13:38:27
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "head_led.h"
#include "../include/pis_config.h"
#include "../manage/dev_status.h"
#include "../operate_device/pisc.h"
#include "../teminal_device/pisc_local.h"
#include "../lib/file_operator.h"
#include "../lib/utils.h"
#include "port_layer/ctrl_udport.h"

//显示命令字
#define HEAD_LED_SHOW_CMD	           (0x0008)
#define HEAD_LED_PACK_CONTENT_MAX		(1024)
#define HEAD_LED_CONTENT_MAX		    (10240)


typedef struct 
{
    uint8_t  u8Languge;
    uint8_t  u8Color;
    uint32_t u8ContentLen;
    uint8_t  acContent[HEAD_LED_CONTENT_MAX];
    /* data */
}T_HEADLED_CONTENT;

typedef struct
{
	uint8_t u8PacketCnt;
	uint8_t u8CurPackcetNum;
	uint8_t u8Color;
	uint8_t u8Language;
	uint8_t acContent[HEAD_LED_PACK_CONTENT_MAX];
}T_HEADLED_PACKET;

static T_HEADLED_CONTENT gtHeadLedContentSendTable[]=
{
	{LANGUAGE_C,COLOR_R,0,{0x00}},
	{LANGUAGE_E,COLOR_G,0,{0x00}},
	{0,0,0,"",},
};



void HEAD_LED_Init()
{
   DEV_STATUS_InitDevValid(DEV_TYPE_HEAD_LED);
}

void HEAD_LED_ContentSet(uint8_t _u8Language)
{
    
    T_STATIONINFO tStationInfo;
    uint8_t i                    = 0;   
    int iFile_Len                = 0;
    uint16_t u16HexLen           = 0;
    uint8_t acFileName[128]      = {""};
    uint8_t acTempBuffer[1280*4] = {0x00};
    uint8_t acHexBuffer[1280*4]  = {0x00};
 
    PISC_LOCAL_GetStationInfo(&tStationInfo);
    //查找显示内容
	sprintf((char *)acFileName,"%s%d/%03d.conf",HEAD_LED_CONF_PATH,LANGUAGE_C,tStationInfo.u8EndStation);
	
    while(gtHeadLedContentSendTable[i].u8Languge)
    {
        if(_u8Language == gtHeadLedContentSendTable[i].u8Languge)
        {
			
            gtHeadLedContentSendTable[i].u8ContentLen = 0;
            FileOperator_Read((const char *)acFileName,(char *)acTempBuffer,&iFile_Len);
            Utils_BufferStrToHex(acHexBuffer,acTempBuffer,iFile_Len,&u16HexLen);
            gtHeadLedContentSendTable[i].u8Color = acHexBuffer[0];//get color
            memcpy((uint8_t*)&gtHeadLedContentSendTable[i].acContent[0],&acHexBuffer[1],u16HexLen -1);
			gtHeadLedContentSendTable[i].u8ContentLen += u16HexLen;
			printf("++++++i=%d++FileName:%s:Language%d\n",i,acFileName,_u8Language);
            break;
        }
        i++;
    }
    
}
void HEAD_LED_ContentSend(void)
{
	uint8_t i=0;
	uint8_t j=0;
	T_HEADLED_PACKET tHeadLedPacket;
	uint16_t u16PacketLen    = 0;
	uint16_t u16SendPaketLen = 0;
	for(i = 0; i< 2; i++)
	{
		uint8_t u8PacketNum = (gtHeadLedContentSendTable[i].u8ContentLen/HEAD_LED_PACK_CONTENT_MAX+1);
		for(j = 0; j < u8PacketNum; j++)
		{
			if(j < (u8PacketNum - 1))
            u16PacketLen = HEAD_LED_PACK_CONTENT_MAX;
			else 
            u16PacketLen = (gtHeadLedContentSendTable[i].u8ContentLen % HEAD_LED_PACK_CONTENT_MAX);
			//printf("pack_len_tmp: %d\r\n",pack_len_tmp);
			//填充数据包
			tHeadLedPacket.u8CurPackcetNum  = u8PacketNum;
			tHeadLedPacket.u8CurPackcetNum  = (j+1);
			tHeadLedPacket.u8Color          = gtHeadLedContentSendTable[i].u8Color; 	
			tHeadLedPacket.u8Language       = gtHeadLedContentSendTable[i].u8Languge; 	
			memcpy(tHeadLedPacket.acContent,(uint8_t *)&gtHeadLedContentSendTable[i].acContent,u16PacketLen);
			//发送长度
			u16SendPaketLen                = sizeof(T_HEADLED_PACKET) - HEAD_LED_PACK_CONTENT_MAX + u16PacketLen;
            printf("Languge:%d,Color:%d,Send HEAD_LED BUFFER:%d\r\n",tHeadLedPacket.u8Language,tHeadLedPacket.u8Color,u16PacketLen);
			//发送显示数据
			//ctrl_port_send_cmd(0xffff,DEV_TYPE_HEAD_LED,0xff,HEAD_LED_SHOW_CMD,(uint8 *)&head_led_pack_tmp,send_len_tmp);
			//FiPrint("head cont:\r\n");
			//print_buf((uint8 *)&head_led_pack_tmp,send_len_tmp);
			CTRL_UDPORT_SendAFrameData(0xffff,DEV_TYPE_HEAD_LED,0xff,HEAD_LED_SHOW_CMD,(uint8_t *)&tHeadLedPacket,u16SendPaketLen);
			
		}
	}


}