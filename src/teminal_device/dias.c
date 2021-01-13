/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 16:22:33
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-13 18:44:30
 */
#include "dias.h"
#include "../include/general.h"
#include "../manage/mp3_decoder.h"
#include "../manage/broadcast.h"
#include "../driver/soundcard.h"
#include <stdio.h>

#define MP3_DECODE_TEST   0
void DIAS_Init(void)
{
   

}
void DIAS_PlayList(uint16_t *pu16List,uint16_t _u16ListNum)
{



}
void DIAS_Stop(void)
{
    


}
FILE *ptFile = NULL;
void DIAS_AudioSend(void)
{
    unsigned char acAudioBuffer[1024*2]={0x00}; 
      
    //printf("Audio send..................\r\n");
    if(MP3_Decoder_Get_PCM_Data((uint16_t*)acAudioBuffer,1024) != 0)
    {
        
        #if MP3_DECODE_TEST
            if(ptFile == NULL )
            {
                ptFile = fopen("./out.pcm","wb"); 
                printf("++++++++++++++++++++++++++++++++++++++++++++++++++++%p\n",ptFile);
            }
     
            if(ptFile)
            {
                int iRet =  fwrite(acAudioBuffer,1024,1,ptFile);
                printf("iRet == %d\r\n",iRet);
            }

       
        #endif    
         SOUNDCARD_PlayPCM_Put(acAudioBuffer,2048);
        BROADCAST_AudioSend(acAudioBuffer,2048);

    }
    #if MP3_DECODE_TEST
    else
    {
            if(ptFile)
            {
                printf("send.**************************..over\r\n");
                fclose(ptFile);
                ptFile = NULL;
            }
    }
    #endif
    
    
}
