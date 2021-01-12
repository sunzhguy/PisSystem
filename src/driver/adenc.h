/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-12 15:05:11
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 17:30:29
 */
#ifndef _AUDIO_DENC_H
#define _AUDIO_DENC_H




int  ADENC_AudioOutPutInit(char **pcBuffer);
void ADENC_AudioOutPutClose(void);
int  ADENC_GetFrameSize(void);
int  ADENC_GetFrameCnt(void);
int  ADENC_AudioPlayOut(const void* _pvData, int _iDataSize);





#endif