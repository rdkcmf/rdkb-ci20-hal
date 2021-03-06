/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utctx/utctx.h"
#include "utctx/utctx_api.h"

#include "platform_hal.h"

#define MAX_BUFFER_SIZE     1024
#define TMP_BUFFER_SIZE     128
#define ONE_KILOBYTE        1024

/* Note that 0 == RETURN_OK == STATUS_OK    */
/* Note that -1 == RETURN_ERR == STATUS_NOK */
static int execute(char *command, char *result)
{
    FILE *fp = NULL;
    char output[MAX_BUFFER_SIZE] = {0};
    char *str = NULL;

    fp = popen(command, "r");
    if(NULL == fp)
    {
        printf("Failed to run command\n" );
        return RETURN_ERR;
    }

    /* only the first line of the output is of interest */
    fgets(output, sizeof(output)-1, fp);
    str = strstr(output, ":");

    if(NULL != str)
    {
        strcpy(result, (str + 1));
    }
    else
    {
        strcpy(result,output);
    }

    printf("\nresult = %s\n", result);
    pclose(fp);

    return RETURN_OK;
}


INT platform_hal_GetDeviceConfigStatus(CHAR *pValue) { strcpy(pValue, "Complete"); return RETURN_OK; }

INT platform_hal_GetTelnetEnable(BOOLEAN *pFlag) { *pFlag = FALSE; return RETURN_OK; }
INT platform_hal_SetTelnetEnable(BOOLEAN Flag) { return RETURN_ERR; }
INT platform_hal_GetSSHEnable(BOOLEAN *pFlag)
{
    char ssh_access[2] = { 0 };
    UtopiaContext ctx;

    if(NULL==pFlag)
        return RETURN_ERR;

    if (!Utopia_Init(&ctx))
        return RETURN_ERR;

    if (!Utopia_RawGet(&ctx, NULL, "mgmt_wan_sshaccess",
                       ssh_access, sizeof(ssh_access))) {
        Utopia_Free(&ctx, 0);
        return RETURN_ERR;
    }
    *pFlag = atoi(ssh_access);
    Utopia_Free(&ctx, 0);
    return RETURN_OK;
}
INT platform_hal_SetSSHEnable(BOOLEAN Flag)
{
    char ssh_access[2] = { 0 };
    UtopiaContext ctx;
    ssh_access[0] = '0' + ! !Flag;
    if (!Utopia_Init(&ctx))
        return RETURN_ERR;
    if (!Utopia_RawSet(&ctx, NULL, "mgmt_wan_sshaccess", ssh_access)) {
        Utopia_Free(&ctx, 0);
        return RETURN_ERR;
    }
    Utopia_SetEvent(&ctx, Utopia_Event_Firewall_Restart);
    Utopia_Free(&ctx, 1);
    return RETURN_OK;
}


INT platform_hal_GetSNMPEnable(CHAR* pValue) { return RETURN_ERR; }
INT platform_hal_SetSNMPEnable(CHAR* pValue) { return RETURN_ERR; }
INT platform_hal_GetWebUITimeout(ULONG *pValue) { return RETURN_ERR; }
INT platform_hal_SetWebUITimeout(ULONG value) { return RETURN_ERR; }
INT platform_hal_GetWebAccessLevel(INT userIndex, INT ifIndex, ULONG *pValue) { return RETURN_ERR; }
INT platform_hal_SetWebAccessLevel(INT userIndex, INT ifIndex, ULONG value) { return RETURN_ERR; }

INT platform_hal_PandMDBInit(void) { return RETURN_OK; }
INT platform_hal_DocsisParamsDBInit(void) { return RETURN_OK; }
INT platform_hal_GetModelName(CHAR* pValue)
{
    if(NULL == pValue)
    {
        return RETURN_ERR;
    }

    strncpy(pValue, "Creator CI20", strlen("Creator CI20"));

    return RETURN_OK;
}

INT platform_hal_GetSerialNumber(CHAR* pValue) { strcpy(pValue, ""); return RETURN_OK; }
INT platform_hal_GetHardwareVersion(CHAR* pValue) { strcpy(pValue, ""); return RETURN_OK; }
INT platform_hal_GetSoftwareVersion(CHAR* pValue, ULONG maxSize) { strcpy(pValue, "Software Version"); return RETURN_OK; }
INT platform_hal_GetBootloaderVersion(CHAR* pValue, ULONG maxSize) { strcpy(pValue, "Bootloader Version"); return RETURN_OK; }

INT platform_hal_GetFirmwareName(CHAR* pValue, ULONG maxSize)
{
    char fn[TMP_BUFFER_SIZE] = {'\0'};
    int ret = RETURN_ERR;

    if(NULL == pValue )
    {
        return RETURN_ERR;
    }
    ret = execute("grep 'imagename' /version.txt", fn);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
        strncpy(pValue, fn, strlen(fn));
    }

    return ret;
}

INT platform_hal_GetBaseMacAddress(CHAR *pValue) { strcpy(pValue, "BasMac"); return RETURN_OK; }
INT platform_hal_GetHardware(CHAR *pValue) { strcpy(pValue, ""); return RETURN_OK;}


INT platform_hal_GetTotalMemorySize(ULONG *pulSize)
{
    char totMem[TMP_BUFFER_SIZE] = {'\0'};
    int ret = RETURN_ERR;

    if(NULL == pulSize)
    {
        return RETURN_ERR;
    }

    ret = execute("grep 'MemTotal' /proc/meminfo", totMem);
    if(RETURN_OK != ret)
    {
        printf("\nError %s\n", __func__);
    }
    else
    {
	sscanf(totMem, "%d", pulSize );
	*pulSize = *pulSize/ONE_KILOBYTE;
    }

    return ret;
}

INT platform_hal_GetHardware_MemUsed(CHAR *pValue)
{
    char usedMem[TMP_BUFFER_SIZE]={'\0'};
    int ret = RETURN_ERR;
    long tmp;

    if(NULL == pValue)
    {
        return RETURN_ERR;
    }

    ret = execute("df | grep '/dev/root' | awk '{print $3}'", usedMem);
    if(RETURN_OK != ret)
    {
        printf("Error: %s", __func__);
    }
    else
    {
        tmp = atoi(usedMem)/ONE_KILOBYTE;
        sprintf(pValue, "%ld",  tmp);
    }

    return ret;
}

INT platform_hal_GetHardware_MemFree(CHAR *pValue)
{
   char freeMem[TMP_BUFFER_SIZE] = {'\0'};
   int ret = RETURN_ERR;
   long tmp;

   if(NULL == pValue)
   {
       return RETURN_ERR;
   }
   ret = execute("df | grep '/dev/root' | awk '{print $4}'", freeMem);
   if(RETURN_OK != ret)
   {
       printf("Error:%s", __func__);
   }
   else
   {
      tmp = atoi(freeMem)/ONE_KILOBYTE;
      sprintf(pValue, "%ld", tmp);
   }

   return ret;
}

INT platform_hal_GetFreeMemorySize(ULONG *pulSize)
{
    char freeMem[TMP_BUFFER_SIZE] = {'\0'};
    int ret = RETURN_ERR;
    long tmp;

    if(NULL == pulSize)
    {
        return RETURN_ERR;
    }

    ret = execute("free| grep 'Mem'| awk '{print $4}'", freeMem);
    if(RETURN_OK != ret)
    {
        printf("Error:%s", __func__);
    }
    else
    {
        sscanf(freeMem, "%d", pulSize );
        *pulSize = *pulSize/ONE_KILOBYTE;

    }

    return ret;
}

INT platform_hal_GetUsedMemorySize(ULONG *pulSize)
{
    int ret = RETURN_ERR;
    char usedMem[TMP_BUFFER_SIZE] = {'\0'};
    long tmp;

    if(NULL == pulSize)
    {
        return RETURN_ERR;
    }

    ret = execute("free| grep 'Mem'| awk '{print $3}'", usedMem);
    if(RETURN_OK != ret)
    {
        printf("Error: %s", __func__);
    }
    else
    {
	sscanf(usedMem, "%d", pulSize );
        *pulSize = *pulSize/ONE_KILOBYTE;

    }

    return ret;
}


INT platform_hal_GetFactoryResetCount(ULONG *pulSize)
{
        if (pulSize == NULL)
        {
           return RETURN_ERR;
        }
        *pulSize = 2;
        return RETURN_OK;
}

INT platform_hal_ClearResetCount(BOOLEAN bFlag)
{
        return RETURN_OK;
}

INT platform_hal_getTimeOffSet(CHAR *pValue)
{
	return RETURN_OK;
}

INT platform_hal_SetDeviceCodeImageTimeout(INT seconds)
{
	return RETURN_OK;
}

INT platform_hal_SetDeviceCodeImageValid(BOOLEAN flag)
{
	return RETURN_OK;
}

INT platform_hal_getCMTSMac(CHAR *pValue)
{
	 if (pValue == NULL)
	 {
	     return RETURN_ERR;
	 }
	strcpy(pValue,"00:00:00:00:00:00");
	return RETURN_OK;
}

//temperature and fan control
INT platform_hal_GetChipTemperature(UINT chipIndex, ULONG *pTempValue) {  //chipIndex:0 for main CPU, 1 for wifi chip.  TempValue is in degrees Celcius
	if(chipIndex==0)
		*pTempValue=40;
	else if (chipIndex==0)
		*pTempValue=41;
	else
		*pTempValue=0;
	return RETURN_OK;
}

INT platform_hal_GetFanSpeed(ULONG *pSpeedValue) {  //SpeedValue is in RPMs
	*pSpeedValue=3600;
	return RETURN_OK;
}

INT platform_hal_SetFanSpeed(ULONG SpeeddInRpms) {
	//set the fan speed
	return RETURN_OK;
}
