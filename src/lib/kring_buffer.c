/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-06-05 15:35:11
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-21 15:08:33
 * 参考连接:https://www.cnblogs.com/java-ssl-xy/p/7868531.html
 * http://en.wikipedia.org/wiki/Circular_buffer
 */ 
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "kring_buffer.h"
/*判断X 是否2 的次方*/
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) -1))==0))

/*判断a, b 取最小值*/
#define min(a,b)   (((a) < (b)) ? (a) :(b))




static  T_KRING_BUFFER * _KRingBuffer_AllocInit (void *_pvFBuffer, uint32_t _u32Size)
{
    assert(_pvFBuffer);
    T_KRING_BUFFER *ptKRingBuf = NULL;

    if(!is_power_of_2(_u32Size))
    {
        fprintf(stderr,"size must be powr of 2.\r\n");
        return ptKRingBuf;
    }
    ptKRingBuf = (T_KRING_BUFFER*)malloc(sizeof (T_KRING_BUFFER));
    if(!ptKRingBuf)
    {
        fprintf(stderr,"Faile to malloc kring buffer %u,reasson:%s\r\n",errno,strerror(errno));
        return ptKRingBuf;
    }
    if (pthread_mutex_init(&ptKRingBuf->ptFLock, NULL) != 0)/*需要互斥锁*/
    {
        fprintf(stderr, "Failed init mutex,errno:%u,reason:%s\n",errno, strerror(errno));
        free(ptKRingBuf);
        return NULL;
    }
    memset(ptKRingBuf, 0 ,sizeof(T_KRING_BUFFER));
    ptKRingBuf->pvFBuffer = _pvFBuffer;
    ptKRingBuf->u32Size   = _u32Size ;
    ptKRingBuf->u32In     = 0;
    ptKRingBuf->u32Out    = 0;
    return ptKRingBuf;
}


/*释放kring_buffer*/
static void _KRingBuffer_AllFree(T_KRING_BUFFER *_ptKRingBuf)
{
    if(_ptKRingBuf)
    {
        if(_ptKRingBuf->pvFBuffer)
        {
            free(_ptKRingBuf->pvFBuffer);
            _ptKRingBuf->pvFBuffer = NULL;
        }
        free(_ptKRingBuf);
        _ptKRingBuf = NULL;
    }
}

/*缓冲区长度*/
static uint32_t _KRingBuffer_Len(const T_KRING_BUFFER *_ptKRingBuf)
{
   // printf("kring_buf->in:%d,kring_buf->out:%d\r\n",kring_buf->in,kring_buf->out);
    return (_ptKRingBuf->u32In - _ptKRingBuf->u32Out);

}

/*获取数据从kring buffer*/
static uint32_t _KRingBuffer_Get(T_KRING_BUFFER *_ptKRingBuf,void * _pvFBuffer,uint32_t _u32Size)
{ 
   uint32_t u32Len = 0;
   assert (_ptKRingBuf || _pvFBuffer);
  
   u32Len = min(_u32Size,_ptKRingBuf->u32In - _ptKRingBuf->u32Out);
   /*first get the data form fifo->out unsil the end of the buffer*/
   u32Len = min(_u32Size,_ptKRingBuf->u32Size - (_ptKRingBuf->u32Out & (_ptKRingBuf->u32Size -1)));
   memcpy(_pvFBuffer,_ptKRingBuf->pvFBuffer+(_ptKRingBuf->u32Out &(_ptKRingBuf->u32Size -1)),u32Len);
   /*then get the rest (if any) from the beginning of the buffer*/
   memcpy(_pvFBuffer+u32Len,_ptKRingBuf->pvFBuffer,_u32Size-u32Len);
   _ptKRingBuf->u32Out += _u32Size;
   return _u32Size;
}
//向缓冲区中存放数据
static uint32_t _KRingBuffer_Put(T_KRING_BUFFER *_ptKRingBuf,void * _pvFBuffer,uint32_t _u32Size)
{
    uint32_t u32Len = 0;
    assert (_ptKRingBuf || _pvFBuffer);

    _u32Size = min(_u32Size, _ptKRingBuf->u32Size - _ptKRingBuf->u32In + _ptKRingBuf->u32Out);
     /* first put the data starting from fifo->in to buffer end */
    u32Len  = min(_u32Size, _ptKRingBuf->u32Size - (_ptKRingBuf->u32In & (_ptKRingBuf->u32Size - 1)));
    memcpy(_ptKRingBuf->pvFBuffer + (_ptKRingBuf->u32In & (_ptKRingBuf->u32Size - 1)),_pvFBuffer, u32Len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(_ptKRingBuf->pvFBuffer, _pvFBuffer + u32Len, _u32Size - u32Len);
    _ptKRingBuf->u32In += _u32Size;
   return _u32Size;

}

uint32_t KRingBuffer_GetLength(const T_KRING_BUFFER *_ptKRingBuffer)
{
      uint32_t len = 0;
      pthread_mutex_lock(&_ptKRingBuffer->ptFLock);
      len = _KRingBuffer_Len(_ptKRingBuffer);
      pthread_mutex_unlock(&_ptKRingBuffer->ptFLock);
      return len;
}


uint32_t KRingBuffer_Get (T_KRING_BUFFER *_ptKRingBuffer,void * _pvBuffer, uint32_t _u32Size)
{
    uint32_t ret;
    pthread_mutex_lock(&_ptKRingBuffer->ptFLock);
    ret = _KRingBuffer_Get(_ptKRingBuffer,_pvBuffer,_u32Size);
    if(_ptKRingBuffer->u32In == _ptKRingBuffer->u32Out)
    _ptKRingBuffer->u32In = _ptKRingBuffer->u32Out = 0;
    pthread_mutex_unlock(&_ptKRingBuffer->ptFLock);
    return ret;
}


uint32_t KRingBuffer_Put(T_KRING_BUFFER *_ptKRingBuffer,void * _pvBuffer, uint32_t _u32Size)
{
     uint32_t ret;
     pthread_mutex_lock(&_ptKRingBuffer->ptFLock);
     ret = _KRingBuffer_Put(_ptKRingBuffer,_pvBuffer,_u32Size);
     pthread_mutex_unlock(&_ptKRingBuffer->ptFLock);
     return ret;
}
uint32_t KRingBuffer_Reset(T_KRING_BUFFER *_ptKRingBuffer)
{

    pthread_mutex_lock(&_ptKRingBuffer->ptFLock);
    _ptKRingBuffer->u32In = _ptKRingBuffer->u32Out =0;
    pthread_mutex_unlock(&_ptKRingBuffer->ptFLock);
    return 0;

}


T_KRING_BUFFER * KRingBuffer_Init(uint32_t _u32Size)
{
    void * buffer = NULL;
    T_KRING_BUFFER *ptKRingBuffer =NULL;
    buffer = (void *)malloc(_u32Size);
    if (!buffer)
    {
      fprintf(stderr, "Failed to malloc memory.\n");
      return NULL;
    }
    ptKRingBuffer = _KRingBuffer_AllocInit(buffer, _u32Size);
    return ptKRingBuffer;
}

void     KRingBuffer_Free(T_KRING_BUFFER *_ptKRingBuffer)
{
    _KRingBuffer_AllFree(_ptKRingBuffer);
}
