/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 15:21:52
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-07 17:15:05
 */

#ifndef INI_FILE_H
#define INI_FILE_H
//读取配置文件字段
int INI_FILE_GetValueFromConfig(const char* _pcFileName, const char* _pcSection, 
                                const char* _pcKeyName, char* _pcValue, int iValueLen);
int INI_FILE_SetValueToConfig  (const char* _pcFileName, const char* _pcSection, 
                                const char* _pcKeyName, char* _pValue);


#endif