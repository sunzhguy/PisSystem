/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 14:15:20
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-09 17:31:28
 */
#ifndef BROADCAST_H
#define BROADCAST_H
#include "../include/general.h"
#include "../main.h"
#include "../include/commnanomsg.h"
//广播类型
#define BROADCAST_PRE	 	   0x01//预报站
#define BROADCAST_ARRIVE       0x02//到站报站
#define BROADCAST_CLOSE_DOOR   0x03//关门提示音 
#define BROADCAST_TEST         0x04//广播测试
#define BROADCAST_SKIP         0x05//送跳站广播标志
#define BROADCAST_FORCAST      0x06//广播预示音
#define BROADCAST_URGENT       0x07//紧急广播
#define BROADCAST_OCC	       0x08//OCC广播
#define BROADCAST_MEDIA	       0x09//媒体广播
#define BROADCAST_LIVE         0x0A//紧急广播
#define BROADCAST_NONE 	       0x10//无报站



#define BROADCAST_PLAY        0x01
#define BROADCAST_STOP        0x00


//优先级
#define PRI_OCC			         1
#define PRI_URGENT		         2
#define PRI_LIVE		         3
#define PRI_CLOSE_DOOR	         4
#define PRI_STATION		         5
#define PRI_MEDIA		         6
#define PRI_TEST		         7
#define PRI_NONE		        0xff
#define PRI_PLAYOVER		    0xfe



void    BROADCAST_Init(void);
void    BROADCAST_Process(uint8_t _u8OpDevType,uint8_t _u8OpDevId,uint8_t _u8TypeBroadCast);
void    BROADCAST_StopProcess(uint8_t _u8TypeBroadCast );
void    BROADCAST_SetBroadCastType(uint8_t _u8Type);
uint8_t BROADCAST_GetBroadCastType(void);
void    BROADCAST_GetConfigPriority(void);
void    BROADCAST_SetUrgentCode(uint8_t _u8CodeId);
uint8_t BROADCAST_GetUrgentCode(void);

void    BROADCAST_SetBroadCastOperateDevType(uint8_t _u8OpDevType);
uint8_t BROADCAST_GetBroadCastOperateDevType(void);
void    BROADCAST_SetBroadCastOperateDevId(uint8_t _u8OpDevId);
uint8_t BROADCAST_GetBroadCastOperateDevId(void);
void    BROADCAST_SetBroadCastCycleFlag(uint8_t _u8CycleFlag);
uint8_t BROADCAST_GetBroadCastCycleFlag(void);


void    BROADCAST_AudioSend(uint8_t *_pcBuffer,uint16_t _u16Len);


void    BROADCAST_SendPriority(void);
void    BROADCAST_SendVolume(void);
uint8_t BROADCAST_GetPriority(uint8_t _u8OpDevType,uint8_t _u8OpDevId,uint8_t _u8TypeBroadCast);

typedef struct _T_BROADCAST_NANOMSGFDS  T_BROADCAST_NANOMSGFDS;
struct _T_BROADCAST_NANOMSGFDS{
	int iSysFd;			/* sys fd*/
	int iNanomsgFd;			/* nanomsg fd */
	T_EVENT_FD *ptEventFd;
	void (*pfCallBack)(T_EVENT_CTL *, T_BROADCAST_NANOMSGFDS *);
	void *pvArg;
};
typedef struct {
    T_EVENT_CTL  *ptEventCtl;//事件控制器
    T_EV_TIMER   tEventTimer;//添加一个定时器
    T_MAINSERVER *ptMainServer;
    T_BROADCAST_NANOMSGFDS tNanoMsgFdsUdp2BroadCast;
}T_BROADCAST_SERVICE;

void  *BROADCAST_Service_ThreadHandle(void *_pvArg);





//缓冲区大小
#define AUDIO_BUF_SIZE	1024

//发送音频命令字
#define AUDIO_SEND_CMD			0x0004



//控制命令
typedef struct
{
	uint8_t u8BdType;
	uint8_t u8BdPriority;
	uint8_t acAudioBuf[AUDIO_BUF_SIZE];
}__attribute((packed))T_AUDIOBD;


#endif