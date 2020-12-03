/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-03 15:41:54
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-03 16:57:51
 */
#ifndef PIS_DATA_PROC_MATRIX
#define PIS_DATA_PROC_MATRIX

#define CTRL_CMD_DEFAULT			(0x0001)
#define CTRL_CMD_VOLUME_CTRL	    (0x0002)
#define CTRL_CMD_FEP		        (0x0003)


#define PISC_DATA_MAX_SIZE		    (1280)
#define PISC_PACK_DATA_INDEX		(sizeof(T_PISC_BUS_DATAFRAM)-PISC_DATA_MAX_SIZE)


typedef struct
{
	uint8_t     u8Head;             //7E分隔符标识
	uint16_t    u16DstTrainId;      //目标车厢ID	
	uint8_t     u8DstDevType;       //目标设备类型
	uint8_t     u8DstDevId;	        //目标设备编号
	uint32_t    u32DstDevIp;        //目标设备IP
	uint16_t    u16SrcTrainID;      //源设备车厢
	uint8_t     u16SrcDevType;      //源设备类型号
	uint8_t     u8SrcDevId;         //源设备编号
	uint32_t    u32SrcDevIp;        //源设备IP
	uint16_t    u16Cmd;             //控制字
	uint16_t    u16DatLen;          //数据长度
	uint8_t     acData[PISC_DATA_MAX_SIZE];
}/*__attribute((packed))*/T_PISC_BUS_DATAFRAM;

typedef  void (*PF_PROCESSMSGHANDLER_CALLBACK)(T_PISC_BUS_DATAFRAM *);



typedef struct 
{
    uint8_t 				        u8Byte_Index;			//字节索引
	uint8_t 				        u8Bit_Index;			//位索引
	//uint8_t 				        bit_num;				//位长度
	uint16_t 				        u16BitLength;			//位长度
	uint8_t 				        u8JugeFlag;				//判断
	//uint8				            value;					//判断用值,equal时用
	uint16_t				        u16CompValue;		    //判断用值,equal时用
	PF_PROCESSMSGHANDLER_CALLBACK 	pfMsgHandleCallBack ; 	//Callback指针
}T_DEV_BUSMSGDATA_PROC;



typedef struct 
{
    uint8_t               u8PiscMasterMode;
    T_DEV_BUSMSGDATA_PROC *ptDevBusDatMsgProc;
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
}T_CMD_MATRIX;



void PiscDataProc_MatrixInit(void);
void PiscDataProc_MatrixInit_DefaultData(uint8_t _u8DevType, uint8_t _u8DevId);
void PiscDataProc_Matrix_AddDeviceToMatrixTable(uint8_t _u8DevType, T_CMD_MATRIX *ptCmdMatrixTable);
void PiscDataProc_Matrix_Dispatch(uint8_t _u8DevType, uint8_t _u8DevId,uint16_t  _u16Cmd, uint8_t *pcBuf, uint16_t _u16Len, uint8_t _u8MatixIndex);
void PiscDataProc_Matrix_SetDefaultData(T_PISC_BUS_DATAFRAM *_ptPiscReciveDataFrame);


#endif