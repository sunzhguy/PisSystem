/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-12 11:18:10
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 15:00:35
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "aenc.h"
typedef struct _T_AUDIO_INFO {
	snd_pcm_t            *ptHandle;
	snd_pcm_uframes_t    tFrame_num;
	snd_pcm_format_t     tFormat;
	AUDIO_SAMPLE_RATE_E  enSampleRate;
	AUDIO_BIT_WIDTH_E    enBitWidth;
	uint16_t             u16Channels;                        //通道数
	size_t               iChunk_bytes;                       //周期字节数
	size_t               iBits_per_sample;                   //样本长度  8bit  16  bit等
	size_t               iBits_per_frame;                    //帧   记录了一个声音单元的长度   等于通道数乘以 样本长度
	uint8_t              *pcDataBuf;                        //数据的缓冲区
}T_AUDIO_INFO, *PT_AUDIO_INFO;


typedef struct _T_AUDIO_ENC
{
    AUDIO_CODEC_FORMAT_E enEncType;
    uint8_t bFlag;
    uint8_t *pcEncBuf;
    size_t  iChunk_bytes;                       //周期字节数
}T_AUDIO_ENC, *PT_AUDIO_ENC;

#define AUDIO_CHANNEL_DOUBLE 2
#define AUDIO_CHANNEL_SINGLE 1

#define VOICE_PACAKGE_10MS 160000


static T_AUDIO_INFO gtAudioInfo;
static T_AUDIO_ENC  gtAudioEnc = {AUDIO_CODEC_FORMAT_ADPCM, 0, NULL, 0};
//static char * gpcAudioBuffer = NULL;

static int _AENC_AudioInputGetFormat(AUDIO_BIT_WIDTH_E _eBitWidth,  snd_pcm_format_t *_ptFormat)
{
    switch (_eBitWidth)
    {
        case AUDIO_BIT_WIDTH_16:
            *_ptFormat = SND_PCM_FORMAT_S16_LE;
            break;
        case AUDIO_BIT_WIDTH_8:
            *_ptFormat = SND_PCM_FORMAT_U8;
            break;
        default:
            *_ptFormat = SND_PCM_FORMAT_UNKNOWN;
            break;
    }
    
    return 0;
}

static int _AENC_AudioInputGetSampleRate(AUDIO_SAMPLE_RATE_E _eSampleRate, uint32_t *_u32SampleRate)
{
    switch(_eSampleRate)
    {
        case AUDIO_SAMPLE_RATE_8:
            *_u32SampleRate = 8000;
            break;
        case AUDIO_SAMPLE_RATE_11025:
            *_u32SampleRate = 11025;
            break;
        case AUDIO_SAMPLE_RATE_16:
            *_u32SampleRate = 16000;
            break;
        case AUDIO_SAMPLE_RATE_22050:
            *_u32SampleRate = 22050;
            break;
        case AUDIO_SAMPLE_RATE_24:
            *_u32SampleRate = 24000;
            break;
        case AUDIO_SAMPLE_RATE_32:
            *_u32SampleRate = 32000;
            break;
        case AUDIO_SAMPLE_RATE_441:
            *_u32SampleRate = 44100;
            break;
        case AUDIO_SAMPLE_RATE_48:
            *_u32SampleRate = 48000;
            break;
        case AUDIO_SAMPLE_RATE_64:
            *_u32SampleRate = 64000;
            break;
        default:
            *_u32SampleRate = 8000;
            break;
    }
    return 0;
}


#if 0
static ssize_t _AENC_ReadAudioData(PT_AUDIO_INFO _ptAudioInfo, size_t _iRCount)
{
    int iRet;
    int iResult = 0;
    int iRdCount = _iRCount;
    uint8_t *pcDataBuf = _ptAudioInfo->pcDataBuf;

    if (iRdCount != _ptAudioInfo->tFrame_num) {
        iRdCount = _ptAudioInfo->tFrame_num;
    }

    while (iRdCount > 0)
    {
        iRet = snd_pcm_readi(ptAudioInfo->handle, pcDataBuf, iRdCount);

        if (iRet == -EAGAIN || (iRet >= 0 && (size_t)iRet < count))
        {
            snd_pcm_wait(_ptAudioInfo->ptHandle, 1000);
        }
        else if (iRet == -EPIPE)
        {
            snd_pcm_prepare(_ptAudioInfo->ptHandle);
            fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>/n");
        }
        else if (iRet == -ESTRPIPE)
        {
            fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>/n");
        }
        else if (iRet < 0)
        {
            fprintf(stderr, "Error snd_pcm_writei: [%s]", snd_strerror(iRet));
           // exit(-1);
        }
		
        if (iRet > 0)
        {
            iResult += iRet;
            iRdCount -= iRet;
            pcDataBuf += iRet * _ptAudioInfo->iBits_per_frame / 8;
        }
    }
    
    return iRdCount;
}
#endif
int AENC_AudioInputInit(void)
{
    char *pcDeviceName = "default";

    if (snd_pcm_open(&gtAudioInfo.ptHandle, pcDeviceName, SND_PCM_STREAM_CAPTURE, 0) < 0)
    {
        fprintf(stderr, "Error snd_pcm_open [ %s]/n", pcDeviceName);
        return -1;
    }

    gtAudioInfo.enBitWidth   = AUDIO_BIT_WIDTH_16;
    gtAudioInfo.enSampleRate = AUDIO_SAMPLE_RATE_441;//AUDIO_SAMPLE_RATE_8;
    gtAudioInfo.u16Channels = AUDIO_CHANNEL_DOUBLE;

    return 0;
}


void AENC_AudioInputClose(void)
{

    snd_pcm_close(gtAudioInfo.ptHandle);

}

static int _AENC_SetAudioChAttr(void)
{
    snd_pcm_hw_params_t *ptHwparams;
    snd_pcm_format_t tFormat;
    uint32_t u32SampleRate;
    uint32_t u32BufferTime, u32PeriodTime;
    snd_pcm_uframes_t tBufferSize;

    /* Allocate the snd_pcm_hw_params_t structure on the stack. */
    snd_pcm_hw_params_alloca(&ptHwparams);
    /* Init hwparams with full configuration space */
    if (snd_pcm_hw_params_any(gtAudioInfo.ptHandle, ptHwparams) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_any\r\n");
        goto ERR_SET_PARAMS;
    }
	/*
		设置数据为交叉模式，并判断是否设置成功
		interleaved/non interleaved:交叉/非交叉模式。
		表示在多声道数据传输的过程中是采样交叉的模式还是非交叉的模式。
		对多声道数据，如果采样交叉模式，使用一块buffer即可，其中各声道的数据交叉传输；
		如果使用非交叉模式，需要为各声道分别分配一个buffer，各声道数据分别传输。
	*/
    if (snd_pcm_hw_params_set_access(gtAudioInfo.ptHandle, ptHwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_set_access\r\n");
        goto ERR_SET_PARAMS;
    }

    /* Set sample format */
    if (_AENC_AudioInputGetFormat(gtAudioInfo.enBitWidth,  &tFormat) < 0)
    {
        fprintf(stderr, "Error get_snd_pcm_format\r\n");
        goto ERR_SET_PARAMS;
    }
    
	/*if (SNDWAV_P_GetFormat(wav, &format) < 0) {
		fprintf(stderr, "Error get_snd_pcm_format/n");
		goto ERR_SET_PARAMS;
	}*/

    if (snd_pcm_hw_params_set_format(gtAudioInfo.ptHandle, ptHwparams, tFormat) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_set_format/n");
        goto ERR_SET_PARAMS;
    }
    gtAudioInfo.tFormat = tFormat;

    /* Set number of channels */
   if (snd_pcm_hw_params_set_channels(gtAudioInfo.ptHandle, ptHwparams, gtAudioInfo.u16Channels) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_set_channels/n");
        goto ERR_SET_PARAMS;
    }

    /* Set sample rate. If the exact rate is not supported */
    /* by the hardware, use nearest possible rate.         */ 
    _AENC_AudioInputGetSampleRate(gtAudioInfo.enSampleRate, &u32SampleRate);

    if (snd_pcm_hw_params_set_rate_near(gtAudioInfo.ptHandle, ptHwparams, &u32SampleRate, 0) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_set_rate_near/n");
        goto ERR_SET_PARAMS;
    }
    /*if (LE_INT(wav->format.sample_rate) != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware./n ==> Using %d Hz instead./n", 
			LE_INT(wav->format.sample_rate), exact_rate);
	}*/


    if (snd_pcm_hw_params_get_buffer_time_max(ptHwparams, &u32BufferTime, 0) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max/n");
        goto ERR_SET_PARAMS;
    }
    printf("&&&&&&&&&&&&&&&AudioBufferTime:%d&&&&&&&&&&&&&&&\r\n",u32BufferTime);
    u32BufferTime = VOICE_PACAKGE_10MS;
    if (u32BufferTime > 500000)
    {
        u32BufferTime = 500000;
    }
    u32PeriodTime = u32BufferTime / 4;

    if (snd_pcm_hw_params_set_buffer_time_near(gtAudioInfo.ptHandle, ptHwparams, &u32BufferTime, 0) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near\r\n");
        goto ERR_SET_PARAMS;
    }

    if (snd_pcm_hw_params_set_period_time_near(gtAudioInfo.ptHandle, ptHwparams, &u32PeriodTime, 0) < 0)
    {
		    fprintf(stderr, "Error snd_pcm_hw_params_set_period_time_near\r\n");
		    goto ERR_SET_PARAMS;
    }

    /* Set hw params */
    if (snd_pcm_hw_params(gtAudioInfo.ptHandle, ptHwparams) < 0)
    {
        fprintf(stderr, "Error snd_pcm_hw_params(handle, params)\r\n");
        goto ERR_SET_PARAMS;
    }
    
    snd_pcm_hw_params_get_period_size(ptHwparams, &gtAudioInfo.tFrame_num, 0);
    snd_pcm_hw_params_get_buffer_size(ptHwparams, &tBufferSize);
    if (gtAudioInfo.tFrame_num == tBufferSize)
    {		
        fprintf(stderr, ("Can't use period equal to buffer size (%lu == %lu)/n"), gtAudioInfo.tFrame_num, tBufferSize);		
        goto ERR_SET_PARAMS;
    }
    
    //获取样本长度
    gtAudioInfo.iBits_per_sample =  snd_pcm_format_physical_width(tFormat);
    gtAudioInfo.iBits_per_frame  =  gtAudioInfo.iBits_per_sample *gtAudioInfo.u16Channels;
    gtAudioInfo.iChunk_bytes     =  gtAudioInfo.tFrame_num * gtAudioInfo.iBits_per_frame / 8;
	
    printf("++++++++++iBitPerSample :%d\r\n",gtAudioInfo.iBits_per_sample);
    printf("++++++++++iBitPerFrame :%d\r\n",gtAudioInfo.iBits_per_frame);
    printf("++++++++++iChunk_bytes :%d\r\n",gtAudioInfo.iChunk_bytes);
    printf("+++++++++++tFrame_num  :%d\r\n",gtAudioInfo.tFrame_num);
	
  //3、通过snd_pcm_hw_params_get_period_size()取得peroid_size，注意在ALSA中peroid_size是以frame为单位的。
  //	The configured buffer and period sizes are stored in “frames” in the runtime. 1 frame = channels * sample_size. 
  //	所以要对peroid_size进行转换：chunk_bytes = peroid_size * sample_length / 8。chunk_bytes就是我们单次从WAV读PCM数据的大小。
  //  之后的过程就乏善可陈了。唯一要留意的是snd_pcm_writei()和snd_pcm_readi()的第三个参数size也是以frame为单位，不要忘记frames和bytes的转换。

    return 0;

ERR_SET_PARAMS:
    return -1;
}


int AENC_SetAIAttr(T_AUDIO_CH_ATTR *pAIAttr)
{
    gtAudioInfo.enBitWidth = pAIAttr->enBitWidth;
    gtAudioInfo.enSampleRate = pAIAttr->enSampleRate;
    return 0;
}

int AENC_GetAIAttr(T_AUDIO_CH_ATTR *pAIAttr)
{
    pAIAttr->enSampleRate = gtAudioInfo.enSampleRate;
    pAIAttr->enBitWidth   = gtAudioInfo.enBitWidth;
    return 0;
}

int AENC_SetEncAttr(T_AUDIO_ENC_ATTR *pEncAttr)
{
    if (pEncAttr)
    {
        gtAudioEnc.enEncType = pEncAttr->enEncType;
        gtAudioEnc.bFlag     = pEncAttr->bFlag;
    }
    return 0;
}

int AENC_GetEncAttr(T_AUDIO_ENC_ATTR *pEncAttr)
{
    if (pEncAttr)
    {
        pEncAttr->enEncType = gtAudioEnc.enEncType;
        pEncAttr->bFlag     = gtAudioEnc.bFlag;
    }
    return 0;
}

int AENC_StartEnc(void)
{
    int  iRet = _AENC_SetAudioChAttr();
    printf("setAudioChAttr:iRet:%d\r\n",iRet);
    printf("g_tAudioInfo.chunk_bytes===%d\r\n",gtAudioInfo.iChunk_bytes);
        /* Allocate audio data buffer */
    gtAudioInfo.pcDataBuf = (uint8_t *)malloc(gtAudioInfo.iChunk_bytes);
    if (NULL ==  gtAudioInfo.pcDataBuf)
    {
        fprintf(stderr, "Error malloc: [data_buf]/n");
        return -1;
    }
    
    #if 0
    gtAudioEnc.pcEncBuf = (uint8_t *)malloc(gtAudioInfo.chunk_bytes);
    if (NULL == gtAudioEnc.pcEncBuf)
    {
        fprintf(stderr, "Error malloc: [pcEncBuf]/n");
        return -1;
    }
    
    gtAudioEnc.iChunk_bytes = gtAudioInfo.iChunk_bytes;
    #endif
    return iRet;
}

void AENC_StopEnc(void)
{
    usleep(100000);
    snd_pcm_drain(gtAudioInfo.ptHandle);
    free(gtAudioInfo.pcDataBuf);
    gtAudioInfo.pcDataBuf = NULL;
    #if 0
    free(gtAudioEnc.pcEncBuf);
    gtAudioEnc.pcEncBuf = NULL;
    #endif
}

/*待确定........*/
#if 0
static int AENC_StartAudioEnc(AUDIO_CODEC_FORMAT_E eAudioEncType, void *pRawData, void *pEncAddr, int len)
{

    switch(gtAudioEnc.enEncType)
    {
        case AUDIO_CODEC_FORMAT_G711A:
        {
            //G711_linear2alaw(len/2, pRawData, pEncAddr);
            return len/2;
        }
        case AUDIO_CODEC_FORMAT_G711MU:
        {
            //G711_linear2ulaw(len/2, pRawData, pEncAddr);
            return len/2;
        }
        case AUDIO_CODEC_FORMAT_G726:	
        default:
            memcpy(pEncAddr, pRawData, len);
            return len;
        	break;
    }
    return 0;
}
#endif

int AENC_GetStream_FromSnd(T_AENC_STREAM *pStream)
{
    int iRet,iFrameCnt;
    iFrameCnt = gtAudioInfo.tFrame_num;
    pStream->ps16Addr = (int16_t*)gtAudioInfo.pcDataBuf;
    pStream->u32FrameNum = iFrameCnt;
    iRet =snd_pcm_readi(gtAudioInfo.ptHandle,gtAudioInfo.pcDataBuf,iFrameCnt);
    //printf("iFrameCnt:%d,iRet:%d\r\n",iFrameCnt,iRet);
    if (iRet == -EAGAIN || (iRet >= 0 && (size_t)iRet < iFrameCnt))
        {
            snd_pcm_wait(gtAudioInfo.ptHandle, 1000);
        }
        else if (iRet == -EPIPE)
        {
            snd_pcm_prepare(gtAudioInfo.ptHandle);
            fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>/n");
        }
        else if (iRet == -ESTRPIPE)
        {
            fprintf(stderr, "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>/n");
        }
        else if (iRet < 0)
        {
            fprintf(stderr, "Error snd_pcm_writei: [%s]", snd_strerror(iRet));
          
        }
        pStream->u32Len =  gtAudioInfo.iChunk_bytes;
    return iRet;
}