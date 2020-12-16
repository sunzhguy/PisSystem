/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 17:02:51
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-07 14:22:48
 */
#ifndef MASTER_H
#define MASTER_H

#define MASTER_STATUS		(1)
#define SLAVE_STATUS	    (0)


//主类型
#define MASTER_TYPE_KEY	                (0x11)
#define MASTER_TYPE_OTHER_PISC_ERROR	(0x21)
#define MASTER_TYPE_DEV_ID_LOW	        (0x31)
//备类型
#define SLAVE_TYPE_INIT	                (0x10)
#define SLAVE_TYPE_OTHER_KEY	        (0x20)
#define SLAVE_TYPE_DVA_ERROR	        (0x30)
#define SLAVE_TYPE_APU_ERROR	        (0x40)
#define SLAVE_TYPE_DEV_ID_HIGH	        (0x50)


//void DEV_MASTER_DevTimeoutCntAdd(void);
//void DEV_MASTER_DevTimeoutCntClr(void);
void DEV_MASTER_Init_SlaveType(void);
void DEV_MASTER_MasterProcess(void);
#endif