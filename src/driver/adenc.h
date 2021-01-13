/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2021-01-12 15:05:11
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-13 14:44:22
 */
#ifndef _AUDIO_DENC_H
#define _AUDIO_DENC_H




int  ADENC_AudioOutPutInit();
void ADENC_AudioOutPutClose(void);
int  ADENC_GetPeriodFrameSize(void);
int  ADENC_GetFramesSize(void);
int  ADENC_GetChannels(void);
int  ADENC_AudioPlayOut(const void* _pvBuffer, int _iFramesSize);





#endif