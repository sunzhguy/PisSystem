/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-15 10:43:27
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-15 11:03:26
 */

#include "systime.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <linux/rtc.h>

#if 0
struct rtc_time   
{   
    int tm_sec; //当前秒    
    int tm_min; //当前分钟    
    int tm_hour; //当前小时    
    int tm_mday; //当前在本月中的天，如11月1日，则为1    
    int tm_mon; //当前月，范围是0~11    
    int tm_year; //当前年和1900的差值，如2006年则为36    
    int tm_wday; //当前在本星期中的天，范围0~6    
    int tm_yday; //当前在本年中的天，范围0~365    
    int tm_isdst; //这个我也不清楚    
};
#endif

int SysTime_DateTimeSet(char *_pcDataTime)
{  
    struct rtc_time tm;  
    struct tm _tm;  
    struct timeval tv;  
    time_t timep;  
    sscanf(_pcDataTime, "%d-%d-%d %d:%d:%d", &tm.tm_year,  
        &tm.tm_mon, &tm.tm_mday,&tm.tm_hour,  
        &tm.tm_min, &tm.tm_sec);  
    _tm.tm_sec = tm.tm_sec;  
    _tm.tm_min = tm.tm_min;  
    _tm.tm_hour = tm.tm_hour;  
    _tm.tm_mday = tm.tm_mday;  
    _tm.tm_mon = tm.tm_mon - 1;  
    _tm.tm_year = tm.tm_year - 1900;  
  
    timep = mktime(&_tm);  
    tv.tv_sec = timep;  
    tv.tv_usec = 0;  
    if(settimeofday (&tv, (struct timezone *) 0) < 0)  
    {  
      printf("Set system datatime error!/n");  
      return -1;  
    }  
    return 0;  
}