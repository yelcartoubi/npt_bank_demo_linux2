/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
* @file  toms_callback.c
* @brief
* @version 1.0
* @author Shawn Lian
* @date 2020-7-1
**************************************************************************/

#ifndef _TOMS_CALLBACK_H_
#define _TOMS_CALLBACK_H_

#ifdef USE_TOMS

/*******************************
 * Global Function Declaration
 *******************************/
extern TOMS_ERRCODE TOMS_CommConnect(int nIsUpdateUI, EM_TOMS_PROCEDURE emTrantpye);
extern TOMS_ERRCODE TOMS_CommClose(int );
extern void         ShowPromptMsg(char* , char* , char , char , char , int );
extern int          ShowSelectPrompt(unsigned char* pszTitle, unsigned char* pszPromptMsg);
extern TOMS_ERRCODE TOMS_DispUIEvent(EM_TOMS_UI_ID emUIID, unsigned char* uszUIData1, unsigned char* uszUIData2, unsigned char* uszUIData3);
extern void         TOMS_LogOutput(const char* pszLog, int nLen);
extern TOMS_ERRCODE TOMS_UpdateAppParam(const char *szFilePath, const char *pszStoragePath, const char* szAppName);
extern TOMS_ERRCODE TOMS_CheckVersion(EM_TOMS_VERSION_TYPE emType, const char *pszAppName, const char* pszParamVer);
extern TOMS_ERRCODE TOMS_UpdateResource(EM_TOMS_RES_ID emRes, char*pszResPath, char *pszMD5);

extern TOMS_ERRCODE TOMS_IsLockTerminal(EM_TOMS_FLOCK bIsLock, const char *pszTipsInfo);
extern TOMS_ERRCODE TOMS_GetIsLockTerminal(EM_TOMS_FLOCK *pemlock);
extern TOMS_ERRCODE TOMS_ExtractLog(const char *pszPerFileNameFmt, const char *pszStartTime, const char *pszEndTime, char *pszOutDir);
extern TOMS_ERRCODE TOMS_GetTerminalStatus(EM_TOMS_TERMSTATUS_INFO_ID emSS, char *pszOutInfo, int nMaxLen);
extern TOMS_ERRCODE TOMS_SysGetDatetime(struct tm *pstDateTime);
extern void         TOMS_SysReboot(void);
extern TOMS_ERRCODE TOMS_SysAppInstall(const char *FilePath);
extern TOMS_ERRCODE TOMS_SysAppUninstall(const char *pszAppName);
extern TOMS_ERRCODE TOMS_SysSetSleepDuration(int nSleepDur);
extern TOMS_ERRCODE TOMS_GetAppNameByIndex(int nIndex, char *pszOutAppName, int nMaxLen);
extern TOMS_ERRCODE TOMS_GetAppInfoByName(const char *pszAppName, EM_TOMS_APP_INFO_ID emID, char *pszOutInfo, int nMaxLen);
extern TOMS_ERRCODE TOMS_GetNetworkType(EM_TOMS_NETWORK_TYPE *pemOutType);
extern TOMS_ERRCODE TOMS_GetSimStatus(EM_TOMS_SIM_STATUS emType, int *pnOutResult);
extern TOMS_ERRCODE TOMS_GetPrinterStatus(EM_TOMS_PRINTER_STATUS *pemOutType);

extern TOMS_ERRCODE TOMS_SysGetVolPercent(int *nVolPercent);
extern TOMS_ERRCODE TOMS_SysGetInfo(EM_TOMS_SYSINFO_ID InfoID, char *pszOutBuf, int *pnOutBufLen);

extern void ProDealLockTerminal();

#endif /* USE_TOMS */

#endif

