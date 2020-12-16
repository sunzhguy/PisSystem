/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-03 15:41:54
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 08:51:52
 */
#ifndef PISC_DATA_PROC_MATRIX
#define PISC_DATA_PROC_MATRIX
#include <stdint.h>
#include "../lib/pis_pack.h"


#define		FALSE	    0
#define		TRUE	    1

#define	    EQUAL		1
#define	    UNEQUAL		2
#define	    WHATEVER	3
#define	    BIGGER		4
#define	    SMALLER		5

#define	    B_1BIT	    0x1
#define	    B_2BIT	    0x3
#define	    B_3BIT	    0x7
#define	    B_4BIT	    0xF
#define	    B_5BIT	    0x1F
#define	    B_6BIT	    0x3F
#define	    B_7BIT	    0x7F
#define	    B_8BIT	    0xFF
#define	    B_9BIT	    0x01FF
#define	    B_10BIT  	0x03FF
#define	    B_11BIT	    0x07FF
#define	    B_12BIT	    0x0FFF
#define	    B_13BIT	    0x1FFF
#define	    B_14BIT	    0x3FF
#define	    B_15BIT	    0x7FFF
#define	    B_16BIT	    0xFFFF


#define CTRL_CMD_DEFAULT			(0x0001)   //命令控制字 过程数据
#define CTRL_CMD_VOLUME_CTRL	    (0x0002)
#define CTRL_CMD_FEP		        (0x0003)




typedef  void (*PF_PROCESSMSGHANDLER_CALLBACK)(T_PIS_PACKDATAFRAME *);



typedef struct 
{
    uint8_t 				        u8Byte_Index;			//字节索引
	uint8_t 				        u8Bit_Index;			//位索引
	uint16_t 				        u16BitNum;			   //位长度
	uint8_t 				        u8JugeFlag;				//判断
	//uint8				            value;					//判断用值,equal时用
	uint16_t				        u16CompValue;		    //判断用值,equal时用
	PF_PROCESSMSGHANDLER_CALLBACK 	pfMsgHandleCallBack ; 	//Callback指针
}T_DEV_BUSMSGDATA_PROC;



typedef struct 
{
    uint8_t               u8PiscMasterMode;
    T_DEV_BUSMSGDATA_PROC *ptDevBusDatMsgProcTb;
}T_MODE_MATRIX;

typedef struct 
{
   uint16_t      u16Cmd;
   T_MODE_MATRIX *ptModeMatrixTable;
}T_CMD_MATRIX;


typedef struct 
{
   uint8_t       u8DevType;
   T_CMD_MATRIX *ptCmdMatrixTable;
}T_DEV_MATRIX;



void PisDataProc_MatrixInit(void);
void PisDataProc_MatrixInit_ProcessData(uint8_t _u8DevType, uint8_t _u8DevId);
void PisDataProc_Matrix_AddDeviceToMatrixTable(uint8_t _u8DevType, T_CMD_MATRIX *_ptCmdMatrixTable);
void PisDataProc_Matrix_Dispatch(uint8_t _u8DevType, uint8_t _u8DevId,uint16_t  _u16Cmd, uint8_t *_pucBuf, uint16_t _u16Len, uint8_t _u8MatixIndexlen);
void PisDataProc_Matrix_CompareUpdateProcessData(T_PIS_PACKDATAFRAME *_ptPiscReciveDataFrame);


#endif