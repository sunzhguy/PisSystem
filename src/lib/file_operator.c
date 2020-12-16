/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-08 11:45:15
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-08 13:48:57
 */
#include "file_operator.h"
#include <stdio.h>
#include <assert.h>


int FileOperator_Read(const char *_pcfileName,char *_pcBuf,int * _piFileSize)
{	
    int i        = 0;
	FILE *pfFile = NULL;
	*_piFileSize = 0;

	assert(_pcfileName !=NULL);
	assert(_pcBuf !=NULL);

	pfFile = fopen(_pcfileName,"r");
	if( NULL == pfFile) {
		return -1;
	}

	_pcBuf[i] = fgetc(pfFile);
	
	//load initialization file
	while( _pcBuf[i] != (char)EOF) {
		i++;
		assert( i < MAX_FILE_SIZE ); //file too big, you can redefine MAX_FILE_SIZE to fit the big file 
		_pcBuf[i] = fgetc(pfFile);
	}
	
	_pcBuf[i]='\0';
	*_piFileSize = i;
	fclose(pfFile);
	return 0;
}