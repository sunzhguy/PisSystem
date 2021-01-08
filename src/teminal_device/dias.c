/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 16:22:33
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-08 17:25:00
 */
#include "dias.h"
#include "../include/general.h"
#include "../manage/mp3_decoder.h"
#include "../manage/broadcast.h"

void DIAS_Init(void)
{

}
void DIAS_PlayList(uint16_t *pu16List,uint16_t _u16ListNum)
{

}
void DIAS_Stop(void)
{
    
}
void DIAS_AudioSend(void)
{
    unsigned char acAudioBuffer[1024];    
    if(MP3_Decoder_Get_PCM_Data(acAudioBuffer,1024) != 0)
    {
        BROADCAST_AudioSend(acAudioBuffer,1024);
    }
    
}
