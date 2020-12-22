/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-06-05 15:34:51
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-21 14:40:07
 */ 

#ifndef  KRING_FIFO_BUFFER_H
#define  KRING_FIFO_BUFFER_H

#include "../include/general.h"
#include <pthread.h>
typedef struct  _kring_buffer
{
     void *     pvFBuffer;//fifo缓冲区
     uint32_t   u32Size  ;//大小
     uint32_t   u32In;    //input 
     uint32_t   u32Out;   //output
     pthread_mutex_t ptFLock;//fifo互斥锁
}T_KRING_BUFFER;




T_KRING_BUFFER * KRingBuffer_Init(uint32_t _u32Size);
void             KRingBuffer_Free(T_KRING_BUFFER *_ptKRingBuffer);
uint32_t         KRingBuffer_GetLength(const T_KRING_BUFFER *_ptKRingBuffer);
uint32_t         KRingBuffer_Put(T_KRING_BUFFER *_ptKRingBuffer,void * _pvBuffer, uint32_t _u32Size);
uint32_t         KRingBuffer_Get(T_KRING_BUFFER *_ptKRingBuffer,void * _pvBuffer, uint32_t _u32Size);
uint32_t         KRingBuffer_Reset(T_KRING_BUFFER *_ptKRingBuffer);
#endif