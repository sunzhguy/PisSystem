/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 15:43:55
 * @LastEditor: sunzhguy
 * @LastEditTime: 2021-01-12 10:41:51
 */
#include "occ.h"
#include "../include/pis_config.h"
#include "../driver/soundcard.h"
#include "../driver/led.h"
#include "../include/commnanomsg.h"
#include "../manage/ioinput_manage.h"
#include "../manage/dev_status.h"
#include "../manage/broadcast.h"
#include "../teminal_device/pisc_local.h"
#define CHANNEL_OCC	(CHANNEL_LEFT)

void OCC_Init(void)
{

    
}
void OCC_OCCStatusSet(void * _ptIOInPutEvCtl,uint8_t _u8Flag)
{
  T_IOINPUT_EVCTL* ptIOInPutEvCtl = _ptIOInPutEvCtl;
  LED_Ctrl(LED_OCC,_u8Flag);//0-->OCC_ON->LED_ON   1->OCC_STOP-->LED_OFF
   uint8_t *dat = nn_allocmsg(8, 0);
  if (NULL != dat)
  {
   dat[0] = TO_BROADCAST_NS;
   dat[1] = MSG_TYPE_OCCBD;  
   if(!_u8Flag)
   {
     dat[2] = BROADCAST_PLAY;
   }else
     dat[2] = BROADCAST_STOP;
     
     dat[3] = DEV_TYPE_PISC;
     dat[4] = PISC_LOCAL_GetDevId();
     nn_send(ptIOInPutEvCtl->tNanomsgEvFds.iNanoMsgFd, &dat, NN_MSG, NN_DONTWAIT);
  }
   
}

void OCC_AudioSend(void)
{

    printf("################OCC_AUDIO_SEND......\r\n");
    
}