/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 11:54:10
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 13:50:35
 */
#include "utils.h"
//***********************************************************************************************************************
//函数作用:把缓冲区内容转化成为hex字符串
//参数说明:buf
//注意事项:
//返回说明:
//***********************************************************************************************************************

void Utils_BufferStrToHex(uint8_t *_pcHex, uint8_t *pcBuf, uint16_t _u16BuferLen, uint16_t *pu16HexLen)
{
    uint16_t u16BufLen = _u16BuferLen;
	uint8_t  u8Chr='0';
	while(u16BufLen--)
	{
		if((*pcBuf)>='a' && (*pcBuf)<='f')
		{
			u8Chr = 'a';
			
			*_pcHex=((*pcBuf++)-u8Chr+10)<<4;
		}
		else if((*pcBuf)>='A' && (*pcBuf)<='F')
		{
			u8Chr = 'A';
			
			*_pcHex=((*pcBuf++)-u8Chr+10)<<4;
		}
		else
		{
			u8Chr = '0';
			
			*_pcHex=((*pcBuf++)-u8Chr)<<4;
		}
		
		if((*pcBuf)>='a' && (*pcBuf)<='f')
			{
			u8Chr = 'a';
			
			*_pcHex|=((*pcBuf++)-u8Chr+10)&0x0F;
			}
		else if((*pcBuf)>='A' && (*pcBuf)<='F')
			{
			u8Chr = 'A';
			
			*_pcHex|=((*pcBuf++)-u8Chr+10)&0x0F;
			}		
		else
			{
			u8Chr = '0';
			
			*_pcHex|=((*pcBuf++)-u8Chr)&0x0F;
			}		
		
		pcBuf++;
		_pcHex++;
	}
	*_pcHex=0;
	*pu16HexLen=(_u16BuferLen/3);
}