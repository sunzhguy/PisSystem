/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 08:35:05
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 14:55:45
 */
#ifndef PIS_PACK_H
#define PIS_PACK_H
#include "../include/general.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>




#define PIS_HEAD					(0x7E)
#define PIS_DATA_MAX_SIZE		    (1280)

typedef struct
{
	uint8_t     u8Head;             //7E分隔符标识
	uint16_t    u16DstTrainId;      //目标车厢ID	
	uint8_t     u8DstDevType;       //目标设备类型
	uint8_t     u8DstDevId;	        //目标设备编号
	uint32_t    u32DstDevIp;        //目标设备IP
	uint16_t    u16SrcTrainID;      //源设备车厢
	uint8_t     u8SrcDevType;      //源设备类型号
	uint8_t     u8SrcDevId;         //源设备编号
	uint32_t    u32SrcDevIp;        //源设备IP
	uint16_t    u16Cmd;             //控制字
	uint16_t    u16DatLen;          //数据长度
	uint8_t     acData[PIS_DATA_MAX_SIZE];
}__attribute((packed))T_PIS_PACKDATAFRAME;


#define PIS_PACK_HEADINFO_SIZE		(sizeof(T_PIS_PACKDATAFRAME)-PIS_DATA_MAX_SIZE)


void PIS_PACK_DataPacket(uint16_t _u16DstTrainId, uint8_t _u8DstDevType, uint8_t _u8DstDevId,uint32_t _u32DstDevIp,
						  uint16_t _u16SrcTrainId, uint8_t _u8SrcDevType, uint8_t _u8SrcDevId,uint32_t _u32SrcDevIp,
						  uint16_t _u16Cmd,
						  uint8_t  *_pcSrcData, uint16_t _u16SrcDatLen, uint8_t* pcDstData, uint16_t* pusDstDatLen);

#endif