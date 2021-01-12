/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-12 15:05:16
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 17:39:04
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "adenc.h"
#include "aenc.h"

typedef struct _T_AUDIODENC_INFO {
	snd_pcm_t            *ptHandle;
	snd_pcm_uframes_t    tFrame_num;
	snd_pcm_format_t     tFormat;
	uint16_t             u16Channels;                        //通道数
    uint32_t             u32SampleRate;                       //采样率
	size_t               iChunk_bytes;                       //周期字节数
	size_t               iBits_per_sample;                   //样本长度  8bit  16  bit等
	size_t               iBits_per_frame;                    //帧   记录了一个声音单元的长度   等于通道数乘以 样本长度
	uint8_t              *pcDataBuf;                        //数据的缓冲区
}T_AUDIODENC_INFO;

T_AUDIODENC_INFO  gtAudioDencInfo;




int  ADENC_AudioOutPutInit(char **pcBuffer)
{
    int iRet = 0;
    snd_pcm_hw_params_t *ptParams;

    gtAudioDencInfo.u16Channels   = 2;                 //双声道
    gtAudioDencInfo.tFormat       = SND_PCM_FORMAT_S16;//16位数据
    gtAudioDencInfo.u32SampleRate = 44100;             //44.1KHZ 采样率
    uint32_t u32Dir;

/* Open PCM device for playback. */
    iRet = snd_pcm_open(&gtAudioDencInfo.ptHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);
     if (iRet < 0) {
          fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(iRet));
         return -1;
      }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&ptParams);

    /* Fill it in with default values. */
    if((iRet = snd_pcm_hw_params_any(gtAudioDencInfo.ptHandle, ptParams))<0)
    {
        printf("cannot initialize hardware parameter structure (%s)\n",snd_strerror(iRet));
        return -1;
    }

    /* Interleaved mode */
     if((iRet = snd_pcm_hw_params_set_access(gtAudioDencInfo.ptHandle, ptParams, SND_PCM_ACCESS_RW_INTERLEAVED)) <0)
     {
        printf("cannot set access type (%s)\n",snd_strerror (iRet));
        return -1;
     }
    /* Signed 16-bit format */
    if((iRet = snd_pcm_hw_params_set_format(gtAudioDencInfo.ptHandle, ptParams, gtAudioDencInfo.tFormat)) < 0)
    {
        printf("cannot set sample format (%s)\n",snd_strerror (iRet));
        return -1;
    }
     /* Two channels (stereo) */
    if((iRet =  snd_pcm_hw_params_set_channels(gtAudioDencInfo.ptHandle, ptParams, gtAudioDencInfo.u16Channels))< 0)
    {
         printf("cannot set channel count (%s)\n",snd_strerror (iRet));
        return -1;
    }
     /* 44100 samples/second sampling rate */

    if((iRet = snd_pcm_hw_params_set_rate_near(gtAudioDencInfo.ptHandle, ptParams, &gtAudioDencInfo.u32SampleRate,0))<0 )
    {
          printf("cannot set sample rate (%s)\n", snd_strerror (iRet));
          return -1;  
    }
    if ((iRet = snd_pcm_hw_params (gtAudioDencInfo.ptHandle, ptParams)) < 0) 
    {  
      printf("cannot set parameters (%s)\n", snd_strerror (iRet)); 
       
      return -1;  
    } 
   
    snd_pcm_hw_params_get_period_size(ptParams, &gtAudioDencInfo.tFrame_num,&u32Dir);
    gtAudioDencInfo.iChunk_bytes = gtAudioDencInfo.tFrame_num*2*2;   // 2 bytes/sample, 2 channels
    gtAudioDencInfo.pcDataBuf    = malloc(gtAudioDencInfo.iChunk_bytes);
    printf("########Play PCM Dir:%d ,%d Frame:%d\r\n",u32Dir,gtAudioDencInfo.tFrame_num,gtAudioDencInfo.iChunk_bytes);
    if(gtAudioDencInfo.pcDataBuf == NULL)
    {
        snd_pcm_hw_params_free(ptParams);
        snd_pcm_close(gtAudioDencInfo.ptHandle);
        return -1;
    }
    *pcBuffer = gtAudioDencInfo.pcDataBuf;
    return iRet;
}


void  ADENC_AudioOutPutClose(void)
{
  snd_pcm_drain(gtAudioDencInfo.ptHandle);   /* 等待数据全部播放完成 */
  snd_pcm_close(gtAudioDencInfo.ptHandle);
  if( gtAudioDencInfo.pcDataBuf)
  {
      free(gtAudioDencInfo.pcDataBuf);
  }
}


int  ADENC_GetFrameSize(void)
{
    return gtAudioDencInfo.iChunk_bytes;
}
int  ADENC_GetFrameCnt(void)
{
    return gtAudioDencInfo.tFrame_num;
}
int ADENC_AudioPlayOut(const void* _pvData, int _iDataSize)
{

  snd_pcm_uframes_t tFrameSize;	  /* 一帧的大小 */
  unsigned int      iFormatSbits = 16; /* 数据格式: 有符号16位格式 */
  unsigned int iChannels    = gtAudioDencInfo.u16Channels;
  /* 获取一个播放周期的帧数 */
  tFrameSize = iFormatSbits*iChannels/8;

  for(snd_pcm_uframes_t i = 0; i < _iDataSize;)
  {
   
  	int iRet = -1;
  	/* 播放 */
   iRet = snd_pcm_writei(gtAudioDencInfo.ptHandle, _pvData, _iDataSize/*gtAudioDencInfo.tFrame_num*/);
   printf("iRet===%dgtAudioDencInfo.tFrame_num:%d _iDataSize:%d\r\n",iRet,gtAudioDencInfo.tFrame_num,_iDataSize);
   if(iRet == -EPIPE || iRet >=0 )
    {
   
      /* 让音频设备准备好接收pcm数据 */
      if(iRet  == iRet == -EPIPE )
      snd_pcm_prepare(gtAudioDencInfo.ptHandle); 
      else
      {
         i += gtAudioDencInfo.tFrame_num * tFrameSize;
      }
      
    }else
    {
        return -1;
    }
  }
    return 0;
}