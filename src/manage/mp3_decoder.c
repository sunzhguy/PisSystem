/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 17:06:24
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-13 17:28:53
 */

#include <unistd.h>
#include <time.h>
#include "broadcast.h"
#include "mp3_decoder.h"
#include "../driver/soundcard.h"

//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"

//缩放
#include "libswscale/swscale.h"

#include "libswresample/swresample.h"

#include "../include/pis_config.h"
#include "../port_layer/audio_port_bd.h"
#include "../lib/kring_buffer.h"



#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000   //192K

//#define AUDIO_PACKET_SIZE		(300)
#define AUDIO_PACKET_SIZE		(50)

//#define AUDIO_PACK_INIT_SIZE			(1024*5)
#define AUDIO_PACK_INIT_SIZE			(10)


//音频通道播放结构
typedef struct
{
	char acFileList[FILE_LIST_NUM_MAX][FILE_NAME_LENGTH_MAX];//文件列表
	int iCurFileIndex;//当前正在播放的文件索引
	int iIsCurFileEOF;//当前文件播放完毕
	int iPauseFlag;//暂停标志
	int iIsDecoding;//正在播放
	
	AVFormatContext *pFormatCtx;//媒体格式指针
	AVCodecContext  *pAudioCodecCtx;//音频解码器指针
	AVCodecContext  *pVideoCodecCtx;//视频解码器指针
	int iAudioStream;//音频流编号
	int iVideoStream;//视频流编号
	
	short as16Audio_PlayBuffer[(AVCODEC_MAX_AUDIO_FRAME_SIZE*3)/2];//音频播放缓冲区
	int   iAudio_WriteLenth;//音频数据长度
}T_AUDIAO_CHANNEL;


static T_AUDIAO_CHANNEL gtAudioChannel_Left=
{
	{""},//文件列表
	0,//当前正在播放的文件索引
	1,//当前文件播放完毕
	0,//暂停标志
	0,//正在播放
	
	NULL,//媒体格式指针
	NULL,//音频解码器指针
	NULL,//视频解码器指针
	-1,//音频流编号
	-1,//视频流编号
	
	{0},//音频播放缓冲区
	0,//音频数据长度
};

//static uint16_t au16AudioPCM_Buffer[AUDIO_PACKET_SIZE*BROADCAST_AUDIO_DATA_SIZE] = {0};
//static uint32_t u32AudioPack_FIFO_In  = 0;
//static uint32_t u32AudioPack_FIFO_Out = 0;

typedef struct {
    //T_EVENT_CTL  *ptEventCtl;//事件控制器
    //T_EV_TIMER   tEventTimer;//添加一个定时器
    uint8_t  u8MP3DecoderRunFlag;
    T_KRING_BUFFER *ptKRingBuffer;
    pthread_mutex_t tThread_Mutex;
    pthread_cond_t  tThread_DecodingRunCond;
    T_MAINSERVER *ptMainServer;
}T_MP3DECODER_SERVICE;


 T_MP3DECODER_SERVICE  gtMP3Decoder_Serivce;

 
static T_KRING_BUFFER * gptKRingBuffer = NULL;


//***********************************************************************************************************************
//函数作用:打印媒体信息
//参数说明:
//注意事项:
//返回说明:无
//***********************************************************************************************************************
static void _MP3_Decoder_Display_AVCodecContext(AVCodecContext *pCodecCtx)
{	
	printf("pCodecCtx->bit_rate:%d \n", pCodecCtx->bit_rate);
	printf("pCodecCtx->sample_rate:%d \n", pCodecCtx->sample_rate);
	printf("pCodecCtx->channels:%d \n", pCodecCtx->channels);
	printf("pCodecCtx->frame_size:%d \n", pCodecCtx->frame_size);
	printf("pCodecCtx->frame_number:%d \n", pCodecCtx->frame_number);
	printf("pCodecCtx->delay:%d \n", pCodecCtx->delay);
	printf("pCodecCtx->frame_bits:%d \n", pCodecCtx->frame_bits);
	printf("\n");
}

void     MP3_Decoder_Set_RunFlag(uint8_t _u8Status)
{
    pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    printf("Mp3 Decoding   set....flag.........\r\n");
    gtMP3Decoder_Serivce.u8MP3DecoderRunFlag = _u8Status;
    pthread_cond_signal(&gtMP3Decoder_Serivce.tThread_DecodingRunCond);
    pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
}
static void _MP3_Decoder_Set_CurrentFileEOF(int _iFlag)
{

  pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    if(_iFlag != gtAudioChannel_Left.iIsCurFileEOF)
    {
        printf("+++++++++To Set CurrentFile EOF:%d\r\n",_iFlag);
        gtAudioChannel_Left.iIsCurFileEOF = _iFlag;
    }
  pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
}

static int _MP3_Decoder_Get_CurrentFileEOF()
{
    int iFlag = 0;
    pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    iFlag = gtAudioChannel_Left.iIsCurFileEOF;
    pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
    return iFlag;
}


static void _MP3_Decoder_Set_IsDecoding(int _iFlag)
{
   pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    if(_iFlag != gtAudioChannel_Left.iIsDecoding)
    {
        printf("+++++++Set IsDecoding....%d\n",_iFlag);
        gtAudioChannel_Left.iIsDecoding = _iFlag;
    }
   pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
}

int  MP3_Decoder_Get_IsDecoding()
{
    int iFlag = 0;
    pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    iFlag = gtAudioChannel_Left.iIsDecoding;
    pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
    return iFlag;
}


//***********************************************************************************************************************
//函数作用:打开媒体文件 并初始化解码库
//参数说明:
//注意事项:
//返回说明:无
//***********************************************************************************************************************
static int  _MP3_Decoder_AVCodecInit(char *_pcFileName, AVFormatContext **_pptFormatCtx, AVCodecContext **_pptAudioCodecCtx, int *_piAudioStream, AVCodecContext **_pptVideoCodecCtx, int *_piVideoStream)
{
	int i;
	//注册所有解码库
	av_register_all();
	printf("av_init,file_name: %s,%p\r\n",_pcFileName,_pptFormatCtx);


	//打开媒体文件文件
	if(avformat_open_input(_pptFormatCtx,_pcFileName,NULL,NULL))
	{
		printf(" Can't open %s! ...\n",_pcFileName);
		return -1;	
	}
	//查找相应的媒体格式
	if(avformat_find_stream_info(*_pptFormatCtx,NULL) <0 )
	{
		printf(" Can't find stream info! \n");
		return -1;	
	}
	
	//寻找第一桢音频和视频数据
	for(i = 0; i < (*_pptFormatCtx)->nb_streams; i++)
	{
		if((*_pptFormatCtx)->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			*_piAudioStream = i;
			printf("*p_audioStream: %d\r\n",*_piAudioStream);
		}
		else if((*_pptFormatCtx)->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			*_piVideoStream = i;
		}
	}
    
	printf("OVER Audio Stream.....\r\n");
	//判断是否有媒体数据
	if((*_piAudioStream == -1) && (*_piVideoStream==-1))
	{
		printf("Error: No Audio and Videa frame \n");
		return -1;
	}
	
	//判断是否有音频数据 查找音频解码器
	if(*_piAudioStream != -1)
	{
		AVCodec *pAudioCodec;
		
		*_pptAudioCodecCtx = (*_pptFormatCtx)->streams[*_piAudioStream]->codec;
		pAudioCodec       = avcodec_find_decoder((*_pptAudioCodecCtx)->codec_id);

		if(pAudioCodec == NULL)
         return -1;

		if(avcodec_open2((*_pptAudioCodecCtx), pAudioCodec,NULL) <0 )
		{
			printf("Open Audio codec failed\n");
			return -1;
		}
		//打印媒体音频信息
		_MP3_Decoder_Display_AVCodecContext(*_pptAudioCodecCtx);
	}
	else
	{
		printf("Error: No Audio \n");
		return -1;
	}
	return 0;
}


uint8_t _MP3_Decoder_AVCodec_Decode(AVFormatContext * _ptFormatCtx,AVCodecContext * _ptAudioCodecCtx, int _iAudioStream, AVCodecContext * _ptVideoCodecCtx, int _iVideoStream)
{
	AVPacket tAvPkt;
	uint32_t u32Len;
	uint16_t au16DecomPressed_AudioBuffer[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	uint32_t au16DecomPresseed_AudioSize = 0;

    while(1)
    {
        //read frame and play
        if ((av_read_frame(_ptFormatCtx, (AVPacket *)&tAvPkt) >= 0))
        {
            //Audio frame
            if(tAvPkt.stream_index == _iAudioStream)
            {
                AVFrame tAvFrame;
                int   iGotAFrame = 0;
                u32Len = avcodec_decode_audio4 (_ptAudioCodecCtx,&tAvFrame,&iGotAFrame,&tAvPkt);
                if(u32Len<0)
                {
                    printf("++++++----- error in decoding audio frame\n");
                    av_free_packet((AVPacket *)&tAvPkt);
                    continue;
                }

                if(iGotAFrame)//如果读取到一帧数据
                {
                    //int iDataSize = 0;
                    if(_ptAudioCodecCtx->channels == 0x01)//单通道
                    {
                        int bytes = av_get_bytes_per_sample(_ptAudioCodecCtx->sample_fmt & 0xFF);
                        au16DecomPresseed_AudioSize = tAvFrame.linesize[0];
                        #if DEBUG_MP3_DECODE
                        printf("bytePerSample:%d,frame.nb_samples:%d,sample_fmt:%d--decomPressedSize:%d\r\n",bytes,tAvFrame.nb_samples,_ptAudioCodecCtx->sample_fmt,au16DecomPresseed_AudioSize);
                        #endif
                        memcpy((unsigned char *)au16DecomPressed_AudioBuffer, (unsigned char *)tAvFrame.data[0],au16DecomPresseed_AudioSize);
                    }
                    
                }

                //如果是立体声 把两个通道合成 送通道缓冲区
                if(_ptAudioCodecCtx->channels == 2)
                {
                    uint16_t au16TempBuffer[4096]={0x00};
                    uint16_t i = 0;
                    uint16_t u16DecodeLen = 0;
                    uint16_t u16HalfLen   = 0;
                    //printf("decompressed_audio_buf_size: %d\r\n",decompressed_audio_buf_size);

                    //采样率展宽
                    if(_ptAudioCodecCtx->sample_rate == 44100)
                    {
                        u16HalfLen          = au16DecomPresseed_AudioSize/2;
                        u16DecodeLen        = au16DecomPresseed_AudioSize/4;

                        for(i=0 ; i < u16HalfLen; i++)
                        {
                            au16TempBuffer[i] = au16DecomPressed_AudioBuffer[ i*2 ];
                        }
                        MP3_Decoder_Set_PCM_Data(au16TempBuffer,u16DecodeLen);
                    }					
                    else if(_ptAudioCodecCtx->sample_rate == 22050)
                    {
                        u16DecodeLen = au16DecomPresseed_AudioSize/2;
                        memcpy(au16TempBuffer,au16DecomPressed_AudioBuffer,au16DecomPresseed_AudioSize);
                        
                        MP3_Decoder_Set_PCM_Data(au16TempBuffer,u16DecodeLen);
                    }
                }		
                else
                {
                    MP3_Decoder_Set_PCM_Data(au16DecomPressed_AudioBuffer,au16DecomPresseed_AudioSize/2);
                }	
                av_free_packet(&tAvPkt);
                
                return 0;
            }
            else
            {
                //Free frame (that was allocated by av_read_frame)
                av_free_packet(&tAvPkt);
            }
        }     
        else //播放完毕
        {
            return 1;
        }
    }//while
}


//***********************************************************************************************************************
//函数作用:关闭解码器和文件
//参数说明:
//注意事项:
//返回说明:无
//***********************************************************************************************************************
void _MP3_Decoder_AVCodec_Close(AVFormatContext **_pptFormatCtx, AVCodecContext * _pptAudioCodecCtx, int _iAudioStream, AVCodecContext * _ptVideoCodecCtx,  int _iVideoStream)
{
	//如果有音频 关闭音频解码器
	if(_iAudioStream != -1) 
     avcodec_close(_pptAudioCodecCtx);

	//如果有视频 关闭视频解码器
	if(_iVideoStream != -1)
    avcodec_close(_ptVideoCodecCtx);

	//关闭媒体文件
    avformat_close_input(_pptFormatCtx);

}

int      MP3_Decoder_SetDecodeList(uint8_t _u8Channel,uint8_t * _pcPath,uint16_t _u16ListNum)
{

 
    int     i                                   = 0;
    int     iFileNum                            = _u16ListNum;
    //uint8_t acTempBuf[FILE_NAME_LENGTH_MAX]     = {0};
    T_AUDIAO_CHANNEL *ptAudioChannel             = &gtAudioChannel_Left;
    uint8_t  *pcPath                             = _pcPath;

    printf("Mp3DecoderPlayList, File_Num: %d\r\n",iFileNum);

    if(_u8Channel == CHANNEL_LEFT)
    {
        ptAudioChannel = &gtAudioChannel_Left;
    }
    //拷贝文件列表
    for(i = 0; i < iFileNum; ++i)
    {
        memcpy(ptAudioChannel->acFileList[i],pcPath,FILE_NAME_LENGTH_MAX);
        printf("-----%s\r\n",ptAudioChannel->acFileList[i]);
        pcPath += FILE_NAME_LENGTH_MAX;
    }
    //添加结束符
    ptAudioChannel->acFileList[iFileNum][0]   = 0x00;
    ptAudioChannel->acFileList[iFileNum+1][0] = 0x00;

    //当前正在播放的文件回0
    ptAudioChannel->iCurFileIndex = 0;

    //送MP3播放标志
    _MP3_Decoder_Set_CurrentFileEOF(FILE_EOF_TRUE);
    _MP3_Decoder_Set_IsDecoding(DECODING);
    
    return  0;
}


void     MP3_Decoder_Stop(uint8_t _u8Channel)
{
    #if 0
    T_AUDIAO_CHANNEL *ptAudioChannel  = &gtAudioChannel_Left;
    printf("+++++++++MP3 Decoder Stop ....\r\n");
    //通道
    if(_u8Channel == CHANNEL_LEFT)
        {
            ptAudioChannel = &gtAudioChannel_Left;
        }
    #endif
    //已经关闭则退出
    if(MP3_Decoder_Get_IsDecoding() == NODECODING)
    {
        return ;
    }
    //设置关闭标志
    _MP3_Decoder_Set_IsDecoding(NODECODING);
    
    //等待关闭
    while(_MP3_Decoder_Get_CurrentFileEOF() == FILE_EOF_FALSE)
    {
        usleep(10*1000);
    }
    
}


void     MP3_Decoder_Set_PCM_Data(uint16_t *_pu16Buffer,uint16_t _u16Len)
{
    KRingBuffer_Put(gptKRingBuffer,(void*)_pu16Buffer,_u16Len*sizeof(uint16_t));
}

uint8_t  MP3_Decoder_Get_PCM_Data(uint16_t *_pu16Buffer,uint16_t _u16Len)
{

  if(KRingBuffer_GetLength(gptKRingBuffer)!=0)
	 {
		KRingBuffer_Get(gptKRingBuffer,_pu16Buffer,_u16Len*sizeof(uint16_t));
       // printf("++++++++++%d++++++++++%d+\r\n",KRingBuffer_GetLength(gptKRingBuffer),_u16Len);
		return 1;
	 }else 
	    return 0;
    
}


#define DEBUG_MP3_DECODE  0

void  *MP3_Decoder_Service_ThreadHandle(void *_pvParam)
{
  
   T_MAINSERVER *ptMainServer     = (T_MAINSERVER *) _pvParam;
   
    printf("MP3_Decoding  Thread Start............\r\n");
    pthread_detach(pthread_self());
    gptKRingBuffer = KRingBuffer_Init(1024*1024);
    gtMP3Decoder_Serivce.ptKRingBuffer  = gptKRingBuffer;
    pthread_mutex_init(&gtMP3Decoder_Serivce.tThread_Mutex, NULL);
	pthread_cond_init(&gtMP3Decoder_Serivce.tThread_DecodingRunCond, NULL);
    
    pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    gtMP3Decoder_Serivce.u8MP3DecoderRunFlag = 0;
    pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
   while(1)
   {

    pthread_mutex_lock(&gtMP3Decoder_Serivce.tThread_Mutex);
    if(gtMP3Decoder_Serivce.u8MP3DecoderRunFlag != DECODING)
    {
        printf("wating...to MP3Decoding.....ERROR\r\n");
        pthread_cond_wait(&gtMP3Decoder_Serivce.tThread_DecodingRunCond, &gtMP3Decoder_Serivce.tThread_Mutex);
    }
     //gtMP3Decoder_Serivce.u8MP3DecoderRunFlag = DECODING;
     pthread_mutex_unlock(&gtMP3Decoder_Serivce.tThread_Mutex);
     #if DEBUG_MP3_DECODE
     printf(" MP3Decoding.....starting......\r\n");
     #endif

    //判断全部播放完毕 退出 最后一个文件为空则视为全面解码完成
     if(gtAudioChannel_Left.acFileList[gtAudioChannel_Left.iCurFileIndex][0] == 0)
     {
         MP3_Decoder_Set_RunFlag(NODECODING);
         continue;
     }
     //如果有文件才能播放
     if(gtAudioChannel_Left.acFileList[gtAudioChannel_Left.iCurFileIndex][0])
     {
         //判断打开文件是否解码完毕
         if(_MP3_Decoder_Get_CurrentFileEOF() == FILE_EOF_TRUE)
         {
             //打开媒体
             int iCurFileIndex = gtAudioChannel_Left.iCurFileIndex;
             if(_MP3_Decoder_AVCodecInit(gtAudioChannel_Left.acFileList[iCurFileIndex],&gtAudioChannel_Left.pFormatCtx,\
                                         &gtAudioChannel_Left.pAudioCodecCtx,&gtAudioChannel_Left.iAudioStream,\
                                         &gtAudioChannel_Left.pVideoCodecCtx,&gtAudioChannel_Left.iVideoStream) < 0)
             {
                 //指向下一个文件
                 gtAudioChannel_Left.iCurFileIndex++;
                 if(gtAudioChannel_Left.acFileList[gtAudioChannel_Left.iCurFileIndex][0] == 0)
                 {
                     printf("++++++++++++++++++++++++++++++++\r\n");
                     _MP3_Decoder_Set_IsDecoding(NODECODING);
                      MP3_Decoder_Set_RunFlag(NODECODING);
                 }
             }else
             {
                 //清当前文件播放完毕标志
                 _MP3_Decoder_Set_CurrentFileEOF(FILE_EOF_FALSE);
             }
             
         }else  //进行解码
         {
             /* 设置解码文件标识 */
             int iFlag = _MP3_Decoder_AVCodec_Decode(gtAudioChannel_Left.pFormatCtx,gtAudioChannel_Left.pAudioCodecCtx,gtAudioChannel_Left.iAudioStream,
                                                     gtAudioChannel_Left.pVideoCodecCtx,gtAudioChannel_Left.iVideoStream);
            #if DEBUG_MP3_DECODE
             printf("Decoding................................OK\r\n");
            #endif
            
             _MP3_Decoder_Set_CurrentFileEOF(iFlag);
             //判断是否播放完毕 指向下一个文件
             if(_MP3_Decoder_Get_CurrentFileEOF() == FILE_EOF_TRUE || (MP3_Decoder_Get_IsDecoding() == NODECODING))
             {
                 
                printf("Decoding................................OVER...OK\r\n");
                _MP3_Decoder_AVCodec_Close(&gtAudioChannel_Left.pFormatCtx,gtAudioChannel_Left.pAudioCodecCtx,gtAudioChannel_Left.iAudioStream,\
                                            gtAudioChannel_Left.pVideoCodecCtx,gtAudioChannel_Left.iVideoStream); 

                 int bExistSleep = 0;
                 while(KRingBuffer_GetLength(gptKRingBuffer) != 0x00)
                 {
                     usleep(100000);
                     #if 0
                     if(bExistSleep%10 == 0x00)
						printf("------%d,%d\r\n",bExistSleep,KRingBuffer_GetLength(gptKRingBuffer));
						usleep(100000);
						bExistSleep++;
						if(bExistSleep >500)//大约5秒退出
					    {
							break;
						}
                        #endif
                 }  
                 //KRingBuffer_Reset(gptKRingBuffer);         
                _MP3_Decoder_Set_CurrentFileEOF(FILE_EOF_TRUE);
                _MP3_Decoder_Set_IsDecoding(NODECODING);
                MP3_Decoder_Set_RunFlag(NODECODING);

                if(BROADCAST_NONE == BROADCAST_GetBroadCastType())
                {
                    printf("CLEAR KRING.....BUFFER:%d\r\n",KRingBuffer_GetLength(gptKRingBuffer));
                }
             }
         }
         
     }
   
   }
    KRingBuffer_Free(gptKRingBuffer);

}