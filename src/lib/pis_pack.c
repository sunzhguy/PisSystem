/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 08:35:14
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 09:59:54
 */
#include "pis_pack.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>





void PIS_PACK_DataPacket(uint16_t _u16DstTrainId, uint8_t _u8DstDevType, uint8_t _u8DstDevId,uint32_t _u32DstDevIp,
						  uint16_t _u16SrcTrainId, uint8_t _u8SrcDevType, uint8_t _u8SrcDevId,uint32_t _u32SrcDevIp,
						  uint16_t _u16Cmd,
						  uint8_t  *_pcSrcData, uint16_t _u16SrcDatLen, uint8_t* pcDstData, uint16_t* pusDstDatLen)
{
    
	T_PIS_PACKDATAFRAME *ptPisPackDataFrame=(T_PIS_PACKDATAFRAME *)pcDstData;
	ptPisPackDataFrame->u8Head         = PIS_HEAD;
	ptPisPackDataFrame->u16DstTrainId  = _u16DstTrainId;
	ptPisPackDataFrame->u8DstDevType   = _u8DstDevType;
	ptPisPackDataFrame->u8DstDevId     = _u8DstDevId;	
	ptPisPackDataFrame->u32DstDevIp    = _u32DstDevIp;
	ptPisPackDataFrame->u16SrcTrainID  = _u16SrcTrainId;
	ptPisPackDataFrame->u8SrcDevType   = _u8SrcDevType;
	ptPisPackDataFrame->u8SrcDevId     = _u8SrcDevId;	
	ptPisPackDataFrame->u32SrcDevIp    = _u32SrcDevIp;	
	ptPisPackDataFrame->u16Cmd         = _u16Cmd;	
	ptPisPackDataFrame->u16DatLen      = _u16SrcDatLen;	
	memcpy((uint8_t *)ptPisPackDataFrame->acData,_pcSrcData,_u16SrcDatLen);
	*pusDstDatLen=(sizeof(T_PIS_PACKDATAFRAME)-PIS_DATA_MAX_SIZE+_u16SrcDatLen);
    
}


