/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 15:48:23
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-13 18:52:17
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
    gtSoundCardInfo.ptRingBuffer_LEFT  = KRingBuffer_Init(1*1024*1024);//左声道2M数据
    gtSoundCardInfo.ptRingBuffer_RIGHT = KRingBuffer_Init(1*1024*1024);//右声道2M数据
 
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

 #include <semaphore.h>
 uint16_t acTempBuffer[10240]={0x00};
 uint16_t acTempBuffer2[10240]={0x00};
 T_KRING_BUFFER *ptRingBuffer_Play;

 
sem_t bin_sem;
void  *SOUNDCARD_PlayPCMService_ThreadHandle(void *_pvArg)
{
    int iDevStatus = 0;
    int iRet       = 0;
    int iChannes   = 0;
    T_MAINSERVER *ptMainServer         = (T_MAINSERVER *) _pvArg;
    int iPeriodFrameSize = 0;
    int iFramesSize  = 0;
    int iSrcAudioChannel = 1;
    //char *pcFile = "/home/root/48000_16bits.pcm";
    char *pcFile = "../testdata/out.pcm";
     FILE * fp     = fopen(pcFile , "r");///
     iDevStatus   = ADENC_AudioOutPutInit();//打开音频设备,初始化采样参数

  
    iPeriodFrameSize  = ADENC_GetPeriodFrameSize();
    iFramesSize       = ADENC_GetFramesSize();
    iChannes          = ADENC_GetChannels();
    printf("%p,iPeriodFrameSize:%d,iFrameCnt:%d,iDevStatus:%d\r\n",fp,iPeriodFrameSize,iFramesSize,iDevStatus);

    ptRingBuffer_Play = KRingBuffer_Init(4*1024*1024);//左声道1M数据
    int iFlag = 0;
    sem_init(&bin_sem, 1024, 0);
    while(1)
    {

        #if 1
        if(iChannes == iSrcAudioChannel)
            {
                
                //sem_wait(&bin_sem);
                if(iFlag == 0)
                {
                    if((KRingBuffer_GetLength(ptRingBuffer_Play) < 100*1024))
                    {
                           printf("xxxxxxx%d\r\n",KRingBuffer_GetLength(ptRingBuffer_Play));
                           sleep(2);
                           continue;
                    
                        
                    }else
                    {
                        if(iFlag == 0)
                        {
                             iFlag =1;
                             printf("++++++++\r\n");
                        }
                         
                    }
                }
                if(iFlag == 1)
                    {
                        int rdSize = KRingBuffer_GetLength(ptRingBuffer_Play);
                        printf("rdSize==%d\r\n",rdSize);
                        if(rdSize >= 2048)
                        {
                            rdSize = 2048;
                        }
                        if(rdSize != 0)
                        {
                         KRingBuffer_Get(ptRingBuffer_Play,acTempBuffer2,rdSize);
                         ADENC_AudioPlayOut(acTempBuffer2,rdSize/(iChannes*2));
                        }
                       
                        usleep(1000);
                    }
                
           }



        #else



        if(fp != NULL)
        {
            int iReadSize = iPeriodFrameSize/2;//iPeriodFrameSize/4;
            iRet = fread(acTempBuffer2, iReadSize, 1, fp);

            if(iChannes != iSrcAudioChannel)
            {
                printf("iRet====%d,%p,iFramesize:%d\r\n",iRet,fp,iPeriodFrameSize);
                {
                    memset(acTempBuffer,0,iReadSize*2);
                    int i ,j = 0;
                    for(i = 0; i< iReadSize/2 ;++i)
                    {
                        acTempBuffer[j]     = acTempBuffer2[i];
                        acTempBuffer[j+1]   = acTempBuffer2[i];
                        j+=2;
                        if(i== 1023)
                        printf("i===%d,j=%d\r\n",i,j);
                    }
                    printf("i====%d,j===%d\r\n",i,j);
                    memset(acTempBuffer2,0,iReadSize*2);
                    memcpy(acTempBuffer2,acTempBuffer,iReadSize*2);
                }
            }
            if(iRet <1)
            {
                usleep(500000);
            }else
            {
             
              if(iChannes != iSrcAudioChannel)
              {
                   printf("iPeriodFrameSize%d = iFramesSize%d----%d\r\n",iPeriodFrameSize,iFramesSize,iPeriodFrameSize/iFramesSize);
                  ADENC_AudioPlayOut(acTempBuffer2,iReadSize*2/(iChannes*2));
              }else
              {
                  ADENC_AudioPlayOut(acTempBuffer2,iReadSize/(iChannes*2));
              }
            }
           
        }
         #endif
        
    }

    if(!iDevStatus)
    {
      ADENC_AudioOutPutClose();
    }

    return NULL;
}

void  SOUNDCARD_PlayPCM_Put(uint8_t *_pvdata,uint16_t _datlen)
{
    
    sem_post(&bin_sem);
    KRingBuffer_Put(ptRingBuffer_Play,_pvdata,_datlen);
}