/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 17:04:16
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-21 15:35:00
 */

#ifndef MP3_DECODER_H
#define MP3_DECODER_H

#include "../include/general.h"
#include "../evio/evio.h"
#include "../main.h"

#define  FILE_EOF_TRUE   (1)
#define  FILE_EOF_FALSE  (0)

#define  DECODING       (1)
#define  NODECODING     (0)
//文件列表最多文件个数
#define FILE_LIST_NUM_MAX		    64

//文件列表最大文件名长度
#define FILE_NAME_LENGTH_MAX		256



int      MP3_Decoder_SetDecodeList(uint8_t _u8Channel,uint8_t * _pcPath,uint16_t _u16ListNum);
void     MP3_Decoder_Stop(uint8_t _u8Channel);


void     MP3_Decoder_Set_PCM_Data(uint16_t *_pu16Buffer,uint16_t _u16Len);
uint8_t  MP3_Decoder_Get_PCM_Data(uint16_t *_pu16Buffer,uint16_t _u16Len);

void     MP3_Decoder_Set_RunFlag(uint8_t _u8Status);

int      MP3_Decoder_Get_IsDecoding(void);



void*   MP3_Decoder_Service_ThreadHandle(void *_pvParam);
#endif
