/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-07 14:38:10
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 11:24:22
 */

#include "ctrl_udport.h"
#include "../udp_service.h"
#include "../manage/dev_status.h"
#include "../lib/pis_pack.h"
#include "../process_matrix/pis_datproc_matrix.h"
#include "../teminal_device/pisc_local.h"
#include <unistd.h>
#include <stdio.h>

#define CTRL_REMOTE_IP_HEX		(0xc0a865ff)

void CTRL_UDPORT_ReadAFrameData(uint8_t *_pcBufer,uint32_t _u32BufLen)
{
    T_PIS_PACKDATAFRAME *ptPisPacketDataFrame = (T_PIS_PACKDATAFRAME *)_pcBufer;
    DEV_STATUS_SetDevStatus(ptPisPacketDataFrame->u8SrcDevType,ptPisPacketDataFrame->u8SrcDevId,DEV_STATUS_OK);
    PisDataProc_Matrix_Dispatch(ptPisPacketDataFrame->u8SrcDevType,ptPisPacketDataFrame->u8SrcDevId,ptPisPacketDataFrame->u16Cmd,
        _pcBufer,PIS_PACK_HEADINFO_SIZE+ptPisPacketDataFrame->u16DatLen,0);
}



void CTRL_UDPORT_SendAFrameData(uint16_t  _u16DstTrainId,uint8_t _u8DstDevType,uint8_t _u8DstDevId,uint16_t _u16Cmd,uint8_t *_pcBuf,uint16_t _u16Len)
{
    uint8_t acTempSendBuffer[PIS_DATA_MAX_SIZE];
    uint16_t u16SendLen;
    PIS_PACK_DataPacket(_u16DstTrainId,_u8DstDevType,_u8DstDevId,CTRL_REMOTE_IP_HEX,
                        0x001,DEV_TYPE_PISC,PISC_LOCAL_GetDevId(),PISC_LOCAL_GetDevIp(),
                        _u16Cmd,_pcBuf,_u16Len,acTempSendBuffer,&u16SendLen);
                        
    /*{
        int i =0;
        do{
            printf("%x ",acTempSendBuffer[i]);
            i++;
        }while(i< u16SendLen);
        printf("\r\n");
    }*/
   UDP_SERVICE_SendData(acTempSendBuffer,u16SendLen);

}