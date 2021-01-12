/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-12 11:18:05
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 14:28:37
 */
#ifndef _AENC_H_
#define _AENC_H_
#include "../include/general.h"

/* AENC stream data struct */
typedef struct _T_AENC_STREAM 
{ 
    uint64_t  u64TimeStamp;   /* Time stamp                      */
    uint32_t  u32FrameNum;    /* Encode frame number             */
    uint32_t  u32Len;         /* Encode data length              */
    int16_t  *ps16Addr;       /* Encode data buffer address      */
    //S32  s32TimeUnit;    /* The time spaned counted as (ms) */
} T_AENC_STREAM ;


typedef enum _AUDIO_SAMPLE_RATE_E
{
    AUDIO_SAMPLE_RATE_8     = 0,   /* 8K Sample rate     */
    AUDIO_SAMPLE_RATE_11025 = 1,   /* 11.025K Sample rate*/
    AUDIO_SAMPLE_RATE_16    = 2,   /* 16K Sample rate    */
    AUDIO_SAMPLE_RATE_22050 = 3,   /* 22.050K Sample rate*/
    AUDIO_SAMPLE_RATE_24    = 4,   /* 24K Sample rate    */
    AUDIO_SAMPLE_RATE_32    = 5,   /* 32K Sample rate    */
    AUDIO_SAMPLE_RATE_441   = 6,   /* 44.1K Sample rate  */
    AUDIO_SAMPLE_RATE_48    = 7,   /* 48K Sample rate    */
    AUDIO_SAMPLE_RATE_64    = 8,   /* 64K Sample rate    */
    AUDIO_SAMPLE_RATE_BUTT
} AUDIO_SAMPLE_RATE_E;

typedef enum _AUDIO_BIT_WIDTH_E
{
    AUDIO_BIT_WIDTH_8   = 0,   /* Bit width is 8 bits   */
    AUDIO_BIT_WIDTH_16  = 1,   /* Bit width is 16 bits  */
    AUDIO_BIT_WIDTH_BUTT
}AUDIO_BIT_WIDTH_E;

typedef enum _AUDIO_CODEC_FORMAT_E
{
    AUDIO_CODEC_FORMAT_G711A   = 1,   /* G.711 A            */
    AUDIO_CODEC_FORMAT_G711MU  = 2,   /* G.711 Mu           */
    AUDIO_CODEC_FORMAT_ADPCM   = 3,   /* ADPCM              */
    AUDIO_CODEC_FORMAT_G726    = 4,   /* G.726              */
    AUDIO_CODEC_FORMAT_AMR     = 5,   /* AMR encoder format */
    AUDIO_CODEC_FORMAT_AMRDTX  = 6,   /* AMR encoder formant and VAD1 enable */
    AUDIO_CODEC_FORMAT_AAC     = 7,   /* AAC encoder        */   
    AUDIO_CODEC_FORMAT_BUTT
}AUDIO_CODEC_FORMAT_E;

/* The audio channel attribute */
typedef struct _T_AUDIO_CH_ATTR
{   
    AUDIO_SAMPLE_RATE_E  enSampleRate;  /* Sample rate      */
    AUDIO_BIT_WIDTH_E    enBitWidth;    /* Sample bit width */
} T_AUDIO_CH_ATTR;

/* The audio enc attribute */
typedef struct _T_AUDIO_ENC_ATTR
{
    AUDIO_CODEC_FORMAT_E enEncType;  /* audio encode formate    */
    uint8_t bFlag;		  /* the flag indicate block */
}T_AUDIO_ENC_ATTR;

/* The raw data struct for audio */
typedef struct _T_AUDIO_RAWDATA
{
    AUDIO_BIT_WIDTH_E    enBitDepth;    /* Sample bit width              */
    AUDIO_SAMPLE_RATE_E  enSampleRate;  /* Sample rate                   */
    uint64_t             u64TimeStamp;  /* The time stamp                */
    uint32_t             u32FrameNum;   /* The frame number              */
    int16_t              *ps16Addr;      /* The buffer address            */
    uint32_t              u32Length;     /* The available data length     */
} T_AUDIO_RAWDATA;

// audio input attribute
int AENC_SetAIAttr(T_AUDIO_CH_ATTR *pAIAttr);
int AENC_GetAIAttr(T_AUDIO_CH_ATTR *pAIAttr);

// audio enc attribute
int AENC_SetEncAttr(T_AUDIO_ENC_ATTR *pEncAttr);
int AENC_GetEncAttr(T_AUDIO_ENC_ATTR *pEncAttr);

int  AENC_AudioInputInit(void);
void AENC_AudioInputClose(void);

int  AENC_StartEnc(void);
void AENC_StopEnc(void);

//int  AENC_GetStream(T_AENC_STREAM *pStream, int iBlockFlag);
int  AENC_GetStream_FromSnd(T_AENC_STREAM *pStream);// add by sunzhguy
void AENC_ReleaseStream(T_AENC_STREAM *pStream);

#endif