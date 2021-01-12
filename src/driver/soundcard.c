/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 15:48:23
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 17:41:26
 */
#include "soundcard.h"
#include "../main.h"
#include "../lib/kring_buffer.h"
#include "aenc.h"
#include "adenc.h"
typedef struct{
 T_KRING_BUFFER *ptRingBuffer_LEFT;
 T_KRING_BUFFER *ptRingBuffer_RIGHT;
}T_SNDCARDINFO;

T_SNDCARDINFO  gtSoundCardInfo ={NULL,NULL};

void  *SOUNDCARD_RecordService_ThreadHandle(void *_pvArg)
{
    int iDevStatus = 0;
    int iRet       = 0;
    T_MAINSERVER *ptMainServer         = (T_MAINSERVER *) _pvArg;
    gtSoundCardInfo.ptRingBuffer_LEFT  = KRingBuffer_Init(2*1024*1024);//左声道2M数据
    gtSoundCardInfo.ptRingBuffer_RIGHT = KRingBuffer_Init(2*1024*1024);//右声道2M数据
 
    if(NULL == gtSoundCardInfo.ptRingBuffer_LEFT ||NULL == gtSoundCardInfo.ptRingBuffer_RIGHT)
    {
        zlog_error(ptMainServer->ptZlogCategory,"Left or Right RingBuffer init error");
    }

    iDevStatus = AENC_AudioInputInit();//打开音频设备,初始化采样参数

    if(!iDevStatus)
    {
        AENC_StartEnc();//设置设备采样参数 启动采样
    }
   
    while(1)
    {
         
         T_AENC_STREAM tAencStream;
         iRet = 0;

         //等待获取OCC 音频 接收相关信息
         if(!iDevStatus)
         {
             iRet = AENC_GetStream_FromSnd(&tAencStream);
         }
         if(iRet > 0)
         {
            uint16_t u16PCMChannelDat = 0;
            int iFrame_byte =tAencStream.u32Len/tAencStream.u32FrameNum;//默认针对双通道 每个通道16bit ==2字节 取出左声道
            for(int i = 0; i< tAencStream.u32FrameNum ;i++)  //默认针对双通道 每个通道16bit ==2字节 取出左声道
			{
			   if(i % 2 == 0x00)
			   {
				u16PCMChannelDat = tAencStream.ps16Addr[i];
                if(gtSoundCardInfo.ptRingBuffer_LEFT != NULL)
                {
                  KRingBuffer_Put(gtSoundCardInfo.ptRingBuffer_LEFT,&u16PCMChannelDat,2);
                }
                
			   }else
               {
                 u16PCMChannelDat = tAencStream.ps16Addr[i];
                 if(gtSoundCardInfo.ptRingBuffer_LEFT != NULL)
                 {
                   KRingBuffer_Put(gtSoundCardInfo.ptRingBuffer_RIGHT,&u16PCMChannelDat,2);
                 }
               }
			}

         }else
         {
             usleep(500000);
         }
         
    }

    if(!iDevStatus)
    {
        AENC_StopEnc();
        AENC_AudioInputClose();
    }

    KRingBuffer_Free(gtSoundCardInfo.ptRingBuffer_LEFT);
    KRingBuffer_Free(gtSoundCardInfo.ptRingBuffer_RIGHT);

    return NULL;
}



void  *SOUNDCARD_PlayPCMService_ThreadHandle(void *_pvArg)
{
    int iDevStatus = 0;
    int iRet       = 0;
    T_MAINSERVER *ptMainServer         = (T_MAINSERVER *) _pvArg;
    uint8_t **buffer = NULL;
    int iFramesize = 0;
    int iFrameCnt  = 0;

    FILE * fp     = fopen("/home/root/48000_16bits.pcm" , "r");
     iDevStatus   = ADENC_AudioOutPutInit(&buffer);//打开音频设备,初始化采样参数
   
  
    iFramesize  = ADENC_GetFrameSize();
    iFrameCnt    = ADENC_GetFrameCnt();
    printf("iFrameSize:%d,iFrameCnt:%d,iDevStatus:%d\r\n",iFramesize,iFrameCnt,iDevStatus);
    while(1)
    {
        if(fp != NULL)
        iRet = fread(buffer, iFramesize, 1, fp);
        printf("iRet====%d,%p,%p\r\n",iRet,fp,buffer);
        if(iRet <1)
        {
            printf("￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥￥\r\n");
            usleep(500000);
        }else
        {
            printf("xxxxxxxxxxxxxxxxxxx\r\n");
           ADENC_AudioPlayOut(buffer,iFrameCnt);
        }
    }

    if(!iDevStatus)
    {
      ADENC_AudioOutPutClose();
    }

    return NULL;
}