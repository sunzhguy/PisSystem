/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 11:44:53
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 11:49:32
 */

#ifndef FILE_OPERATOR_H
#define FILE_OPERATOR_H

#define MAX_FILE_SIZE 1024*64  //64K

int FileOperator_Read(const char *_pcfileName,char *_pcBuf,int * _piFileSize);
#endif