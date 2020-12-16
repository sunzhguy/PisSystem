/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 14:35:20
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-07 15:52:50
 */
#ifndef _PISC_H
#define _PISC_H
#include <stdint.h>

#define PISC_STATUS_MASTER		1
#define PISC_STATUS_SLAVE		0


//PISC工作模式
#define PISC_ATC_MODE		      0x01
#define PISC_DCP_MANUAL_MODE      0x02	
#define PISC_HALF_AUTO_MODE		  0x03


//开关门 左右侧
#define PISC_DOOR_LEFT              2
#define PISC_DOOR_RIGHT             1


//上下行
#define PISC_DIR_UP		            2
#define PISC_DIR_DOWN	            1


#define PISC_DIR_UP_GO_AHEAD_CAL(x)		(x+1)
#define PISC_DIR_DOWN_GO_AHEAD_CAL(x)	(x-1)
#define PISC_DIR_UP_GO_BACK_CAL(x)		(x-1)
#define PISC_DIR_DOWN_GO_BACK_CAL(x)	(x+1)

//第23字节，要设置的关键字
#define PISC_SETUPCMD_NONE               0x00//无效的关键字
#define PISC_SETUPCMD_MAP_INTIAL_STATION 0x01//动态地图的初始站
#define PISC_SETUPCMD_MAP_BRIGHTNESS     0x02//动态地图的亮度


typedef struct
{
	uint8_t     u8StartStation;//起始站2
	uint8_t     u8EndStation;//终点站3
	uint8_t     u8CurStation;//当前站4
	uint8_t     u8NexStation;//下一站5
}__attribute((packed))T_STATIONINFO;

//过程数据结构1
typedef struct
{
	//广播控制位
	struct //1-4
	{
		uint8_t b8ATC_ModeFlag:1;
		uint8_t b8TMS_ModeFlag:1;
		uint8_t b8DCP_ModeFlag:1;//广播控制盒
		uint8_t b8Reserve:3;	// 1
		uint8_t b8ActiveFlag:1;
		uint8_t b8MasterFlag:1;

		uint8_t b8NextOpenLeftFlag:1;// 2
        uint8_t b8NextOpenRightFlag:1;// 2
		uint8_t b8CloseRightDoorFlag:1;
		uint8_t b8CloseLeftDoorFlag:1;
		uint8_t b8OpenRightDoorFlag:1;
		uint8_t b8OpenLeftDoorFlag:1;
		uint8_t b8RunDir:2;// 1 下行，2上行

		uint8_t u8Reserve2:6;	// 3
		uint8_t b8ArrFlag:1;	//
		uint8_t b8PreFlag:1;

		uint8_t b8Reserve3:1;// 4
		uint8_t b8DcpArrBroadCasting:1;	 //
		uint8_t b8DcpPreBroadCasting:1;
		uint8_t b8DcpUrgentBroadCasting:1;	 //
		uint8_t b8Reserve4:4;
	}__attribute((packed))tFlag;

	T_STATIONINFO tStationInfo;	//5-8
	uint8_t u8Status;		// 9
	uint8_t dmp_light_level;	// 10
	uint8_t u8SecondLcuRoll; // 11
	uint32_t u32JumpStations;	//13-16
	uint32_t u32Version;	// 12
}__attribute((packed))T_PISC_PROCESSTDATA;



void    PISC_Init(void);
uint8_t PISC_GetOtherPisc_KeyStatus(void);
void    PISC_SetOtherPisc_KeyStatus(uint8_t _u8Status);
uint8_t PISC_GetOtherPisc_MasterStatus(void);
void    PISC_SetOtherPisc_MasterStatus(uint8_t _u8Status);


#endif