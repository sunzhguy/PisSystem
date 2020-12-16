/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 15:48:14
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 15:50:56
 */

//头文件中有SOUNDCARD_H的宏定义
#ifndef SOUNDCARD_H
#define SOUNDCARD_H
#include "../include/general.h"

#define AUDIO_SAMPLE_SPEED	(44100)
//音频包样本数量
#define AUDIO_SAMPLE_NUM	512
//音频采样精度
#define AUDIO_SAMPLE_BITS	16
//音频队列长度
#define AUDIO_FIFO_SIZE		10

//音频接收端口
#define AUDIO_RECV_PORT		3000
//每一路音频一包数据的缓冲区大小 单位/short
#define AUDIO_CHANNEL_SIZE_SHORT	(AUDIO_SAMPLE_NUM) //AUDIO_SAMPLE_NUM*10*2 字节
//网络接收一包数据的大小 单位/字节
#define AUDIO_PACKET_SIZE_BYTE (AUDIO_SAMPLE_NUM*AUDIO_SAMPLE_BITS/8) //网络接收数据包 字节数


//通道号定义
#define CHANNEL_LEFT	   1
#define CHANNEL_RIGHT	   2



#endif
