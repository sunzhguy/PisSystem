/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-03 15:17:09
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-03 16:59:26
 */

#include <stdint.h>
#include "pis_datproc_matrix.h"

typedef struct
{
	uint8_t u8DevType;
	uint8_t u8DevId;
	uint16_t u16DatLen;
	uint8_t acDataBackUp[128];
}/*__attribute((packed))*/T_DEV_PACK_BACKUPDATA;//打包备份数据


static T_DEV_PACK_BACKUPDATA gtDevPackBackUpDataTable[20]=
{
    {0,0,0,{0}},

}

static void _PiscDataProc_GetPreDefaultData(uint8_t _u8DevType,uint8_t _u8DevId,uint8_t *pcBuf,uint16_t *_u16Len)
{
  //T_DEV_PACK_BACKUPDATA *ptDevPackBackUpdata = &gtDevPackBackUpDataTable[0];	//临时状态指令表		
  T_DEV_PACK_BACKUPDATA data;
 
 #if 0
	while(ptDevPackBackUpdata.)
	{
		if(dev_type==dev_pack_tmp->dev_type && dev_id==dev_pack_tmp->dev_id)
		{
			*len=dev_pack_tmp->data_len;
 			memcpy(buf,(uint8 *)&dev_pack_tmp->data_bak[0],dev_pack_tmp->data_len);
			break;
		}		
		dev_pack_tmp++;		
	}	
#endif
    
}