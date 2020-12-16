/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-12-04 15:19:43
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-12-16 09:48:40
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ini_file.h"
#define  CONFIG_LINE_MAXSIZE  255

static char * _INI_FILE_GetSectionName(char *_pcSectionLineName)
{

    char* pcCurrent = NULL;
    char* pcTail    = NULL;
    char* pcName    = NULL;

    if (!_pcSectionLineName)
        return NULL;

    pcCurrent = _pcSectionLineName;

    while (*pcCurrent == ' ' ||  *pcCurrent == '\t') pcCurrent++; 

    if (*pcCurrent == ';' || *pcCurrent == '#')
        return NULL;

    if (*pcCurrent++ == '[')
        while (*pcCurrent == ' ' ||  *pcCurrent == '\t') pcCurrent ++;
    else
        return NULL;

    pcName = pcTail = pcCurrent;
    while (*pcTail != ']' && *pcTail != '\n' &&
          *pcTail != ';' && *pcTail != '#' && *pcTail != '\0')
          pcTail++;
    *pcTail = '\0';
    while (*pcTail == ' ' || *pcTail == '\t') {
        *pcTail = '\0';
        pcTail--; 
    }

    return pcName;

}

static int _INI_FILE_GetKeyValue (char *_pcKeyLineName, char **_ppcKey, char **_ppcValue)
{
     char* pcCurrent = NULL;
     char* pcTail    = NULL;
     char* pcValue   = NULL;

    if (!_pcKeyLineName)
        return -1;

    pcCurrent = _pcKeyLineName;

    while (*pcCurrent == ' ' ||  *pcCurrent == '\t') pcCurrent++; 

    if (*pcCurrent == ';' || *pcCurrent == '#')
        return -1;

    if (*pcCurrent == '[')
        return 1;

    if (*pcCurrent == '\n' || *pcCurrent == '\0')
        return -1;

    pcTail = pcCurrent;
    while (*pcTail != '=' && *pcTail != '\n' &&
          *pcTail != ';' && *pcTail != '#' && *pcTail != '\0')
          pcTail++;

    pcValue = pcTail + 1;
    if (*pcTail != '=')
        *pcValue = '\0'; 

    *pcTail-- = '\0';
    while (*pcTail == ' ' || *pcTail == '\t') {
        *pcTail = '\0';
        pcTail--; 
    }
        
    pcTail = pcValue;
    while (*pcTail != '\n' && *pcTail != '\0') pcTail++;
    *pcTail = '\0'; 

    if (_ppcKey)
        *_ppcKey = pcCurrent;
    if (_ppcValue)
        *_ppcValue = pcValue;

    return 0;
}


/* This function locate the specified section in the config file. */
static int _INI_FILE_LocateSection(FILE* _ptFp, const char* pSection, FILE* _ptFpBak)
{
    char acBuff[CONFIG_LINE_MAXSIZE + 1];
    char *pcName;

    while (1) {
        if (!fgets(acBuff, CONFIG_LINE_MAXSIZE, _ptFp)) {
            if (feof (_ptFp))
                return -1;
            else
                return -1;
        }
        else if (_ptFpBak && fputs (acBuff, _ptFpBak) == EOF)
            return -1;
        
        pcName = _INI_FILE_GetSectionName (acBuff);
        if (!pcName)
            continue;

        if (strcmp (pcName, pSection) == 0)
            return 0; 
    }

    return -1;
}

/* This function locate the specified key in the etc file. */
static int _INI_FILE_LocalKeyValue(FILE* _ptFp, const char* _pcKeyName, 
                                         int _iCurSection, char* _pcValue, int iValueLen,
                                        FILE* _ptFpBak, char* _pcNextSection)
 {

    char  acBuff[CONFIG_LINE_MAXSIZE + 1 + 1];
    char* pcCurrent = NULL;
    char* pcValue   = NULL;
    int   ret;

    while (1) {
        int bufflen;

        if (!fgets(acBuff, CONFIG_LINE_MAXSIZE, _ptFp))
            return -1;
        bufflen = strlen (acBuff);
        if (acBuff [bufflen - 1] == '\n')acBuff [bufflen - 1] = '\0';
	if (acBuff [bufflen - 2] == '\r')acBuff [bufflen - 2] = '\0';
	
        ret = _INI_FILE_GetKeyValue (acBuff, &pcCurrent, &pcValue);
        if (ret < 0)
            continue;
        else if (ret > 0) {
            fseek (_ptFp, -bufflen, SEEK_CUR);
            return -1;
        }
            
        if (strcmp (pcCurrent, _pcKeyName) == 0) {
            if (_pcValue)
                strncpy (_pcValue, pcValue, iValueLen);

            return 0; 
        }
        else if (_ptFpBak && *pcCurrent != '\0')
            fprintf (_ptFpBak, "%s=%s\n", pcCurrent, pcValue);
    }

    return -1;                        
}
/* Function: INI_FILE_GetValueFromConfig(const char* _pcFileName, const char* _pcSection,
                                 const char* _pcKeyName, char* _pcValue, int iValueLen)
 * Parameter:
 *     _pcFileName:        etc file path name.
 *     _pcSection:         Section name.
 *     _pcKeyName:         Key name.
 *     _pcValue:           The buffer will store the value of the key.
 *     iValueLen:         The max length of value string.
 * Return:
 *     int                 meaning
 *     -1                 The conifg file not found. 
 *     -1                 The section is not found. 
 *     CONIFG_EKYNOTFOUND The Key is not found.
 *     0                   OK.
 */

int INI_FILE_GetValueFromConfig(const char* _pcFileName, const char* _pcSection,
                                 const char* _pcKeyName, char* _pcValue, int iValueLen)
{
    FILE* ptFp = NULL;
    char acTempSection [CONFIG_LINE_MAXSIZE + 2];

    if (!(ptFp = fopen(_pcFileName, "r")))
         return -1;

    if (_pcSection)
         if (_INI_FILE_LocateSection (ptFp, _pcSection, NULL) != 0) {
             fclose (ptFp);
             return -1;
         }

    if (_INI_FILE_LocalKeyValue (ptFp, _pcKeyName, _pcSection != NULL, 
                _pcValue, iValueLen, NULL, acTempSection) != 0) {
         fclose (ptFp);
         return -1;
    }

    fclose (ptFp);
    return 0;
    
}
/* Function: GetIntValueFromEtcFile(const char* pEtcFile, const char* pSection,
 *                               const char* pKey);
 * Parameter:
 *     pEtcFile: etc file path name.
 *     pSection: Section name.
 *     pKey:     Key name.
 * Return:
 *     int                      meaning
 *     -1             The etc file not found. 
 *     -1          The section is not found. 
 *     ETC_EKYNOTFOUND              The Key is not found.
 *     0                       OK.
 */
 #if 0
int GetIntValueFromEtcFile(const char* pEtcFile, const char* pSection,
                               const char* pKey, int* value)
{
    int ret;
    char szBuff [51];

    ret = GetValueFromEtcFile (pEtcFile, pSection, pKey, szBuff, 50);
    if (ret < 0)
        return ret;

    *value = strtol (szBuff, NULL, 0);
    if ((*value == LONG_MIN || *value == LONG_MAX) && errno == ERANGE)
        return -1;

    return 0;
}
#endif
static int INI_FILE_ConfigCopyAndLocation (FILE* ptFp, FILE* ptTempFp, 
                const char* ptSection, const char* _pcKeyName, char* pcTempSection)
{
    if (ptSection && _INI_FILE_LocateSection (ptFp, ptSection, ptTempFp) != 0)
        return -1;

    if (_INI_FILE_LocalKeyValue (ptFp, _pcKeyName, ptSection != NULL, 
                NULL, 0, ptTempFp, pcTempSection) != 0)
        return -1;

    return 0;
}
static int INI_FILE_ConfigFileCopy (FILE* ptSrcFp, FILE* ptDstFp)
{
    char acLine [CONFIG_LINE_MAXSIZE + 1];
    
    while (fgets (acLine, CONFIG_LINE_MAXSIZE + 1, ptSrcFp) != NULL)
        if (fputs (acLine, ptDstFp) == EOF) {
            return -1;
        }

    return 0;
}

int INI_FILE_SetValueToConfig  (const char* _pcFileName, const char* _pcSection, const char* _pcKeyName, char* _pcValue)
{
    FILE* ptFp = NULL;
    FILE* ptTempFp =NULL;
    int rc;
    char acTempSection [CONFIG_LINE_MAXSIZE + 2];

#ifndef HAVE_TMPFILE
    char acTempFileName [256];

    sprintf (acTempFileName, "/tmp/mg-etc-tmp-%x", (int)time(NULL));
    if ((ptTempFp = fopen (acTempFileName, "w+")) == NULL)
        return -1;
#else
    if ((tmp_fp = tmpfile ()) == NULL)
        return -1;
#endif

    if (!(ptFp = fopen (_pcFileName, "r+"))) {
        fclose (ptFp);
#ifndef HAVE_TMPFILE
        unlink (acTempFileName);
#endif
        if (!(ptFp = fopen (_pcFileName, "w"))) {
            return -1;
        }
        fprintf (ptFp, "[%s]\n", _pcSection);
        fprintf (ptFp, "%s=%s\n", _pcKeyName, _pcValue);
        fclose (ptFp);
        return 0;
    }

    switch (INI_FILE_ConfigCopyAndLocation (ptFp, ptTempFp, _pcSection, _pcKeyName, acTempSection))
    {
    case -1:
        fprintf (ptTempFp, "\n[%s]\n", _pcSection);
        fprintf (ptTempFp, "%s=%s\n", _pcKeyName, _pcValue);
        break;

    default:
        fprintf (ptTempFp, "%s=%s\n", _pcKeyName, _pcValue);
        break;
    }

    if ((rc = INI_FILE_ConfigFileCopy (ptFp, ptTempFp)) != 0)
        goto error;
    
    // replace etc content with tmp file content
    // truncate etc content first
    fclose (ptFp);
    if (!(ptFp = fopen (_pcFileName, "w"))) {
        fclose (ptFp);
#ifndef HAVE_TMPFILE
        unlink (acTempFileName);
#endif
        return -1;
    }
    
    rewind (ptTempFp);
    rc = INI_FILE_ConfigFileCopy (ptTempFp, ptFp);

error:
    fclose (ptFp);
    fclose (ptTempFp);
#ifndef HAVE_TMPFILE
    unlink (acTempFileName);
#endif
    return rc;
    
}