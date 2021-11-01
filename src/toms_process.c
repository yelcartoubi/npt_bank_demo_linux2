/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
* @file  toms_process.c
* @brief
* @version 1.0
* @author Shawn Lian
* @date 2020-7-1
**************************************************************************/
#ifdef USE_TOMS

/*******************************
 * Header File
 *******************************/
#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

/*******************************
 * Global Variable Definition
 *******************************/

static void __remove_legacy_dirs()
{
    char *plszDelDirs[] = {
        "/app/share/TMS",
        "/app/share/XTMS"
    };
    char szShellBuffer[100] = {0};
    int i;

    // 1. after testing, we found we should use them together, so that it will ignore the Permission of files.
    // 2. the Dirs owner and group should be same as this app, such as BANKDEMO:BANKDEMO
    for (i = 0; i < sizeof(plszDelDirs)/sizeof(plszDelDirs[0]); i++)
    {
        PubFsRemoveDirectory(plszDelDirs[i]);
        sprintf(szShellBuffer, "rm %s -rf", plszDelDirs[i]);
        system(szShellBuffer);
    }
}

static int __check_necessary_exec_files()
{
    int nRet = 0;

    nRet += PubFsExist("/usr/bin/zip");
    nRet += PubFsExist("/usr/bin/unzip");

    if (nRet != NAPI_OK) {
        PubMsgDlg(tr("TOMS INIT FAIL"), tr("Please install necessary relied file first !! (zip & unzip)"), 3, 3);
    }

    return nRet;
}

int ProDealTomsLimit()
{
    //1. rm legacy dirs
    __remove_legacy_dirs();

    //2. check zip & unzip
    ASSERT_FAIL(__check_necessary_exec_files());

    return 0;
}

int TOMS_UpdateDomain(EM_TOMS_OPT emOpt, char *pszDomain)
{
    if (pszDomain == NULL
        || emOpt <= TOMS_OPTTYPE_CONFIG_BASE
        || emOpt >= TOMS_OPTTYPE_SYSAPI_BASE)
    {
        return APP_FAIL;
    }

    ASSERT_FAIL(TOMS_SetOption(emOpt, pszDomain));

    return APP_SUCC;
}

/**
 * @brief  Init TOMS options
 * @param
 * @return
 * @li
 * @APP_SUCC
 * @APP_FAIL
 * @author tom chen
 * @date 2020-8-21
 */
int TOMS_ProInit(void)
{
    STAPPCOMMPARAM stCommParam = {0};

    ASSERT_FAIL(ProDealTomsLimit());

    GetAppCommParam(&stCommParam);

    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_APP_DOMAIN,                stCommParam.szTOMSAppDomain));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_PARAM_DOMAIN,              stCommParam.szTOMSParamDomain));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_KEYPOS_DOMAIN,             stCommParam.szTOMSKeyPosDomain));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_FILESERVER_DOMAIN,         stCommParam.szTOMSFileServerDomain));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_TDAS_DOMAIN,               stCommParam.szTOMSTdasDomain));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_USER_OID,                  stCommParam.szTomsUserOid));

    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_DATETIME,            TOMS_SysGetDatetime));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_SYS_REBOOT,              TOMS_SysReboot));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_APP_INSTALL,             TOMS_SysAppInstall));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_SYSINFO,             TOMS_SysGetInfo));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_VOLPERCENT,          TOMS_SysGetVolPercent));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_APP_UNINSTALL,           TOMS_SysAppUninstall));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_SET_SLEEP_DUR,           TOMS_SysSetSleepDuration));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_APPNAME_BY_INDEX,    TOMS_GetAppNameByIndex));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_APPINFO_BY_NAME,     TOMS_GetAppInfoByName));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_NETWORK_TYPE,        TOMS_GetNetworkType));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_SIM_STATUS,          TOMS_GetSimStatus));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_GET_PRINER_STATUS,       TOMS_GetPrinterStatus));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_SYSAPI_PARSE_DNS_IP,            TOMS_ParseDNSIP));

    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_COMM_CONNECT,            TOMS_CommConnect));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_COMM_CLOSE,              TOMS_CommClose));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_UI_DISPLAY,              TOMS_DispUIEvent));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_LOG_OUTPUT,              TOMS_LogOutput));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_PARAM_PARSING,           TOMS_UpdateAppParam));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_CHECK_VERSION,           TOMS_CheckVersion));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_UPDATE_RES,              TOMS_UpdateResource));

    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_ISLOCK_TERMINAL,         TOMS_IsLockTerminal));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_GET_ISLOCK_TERMINAL,     TOMS_GetIsLockTerminal));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_EXTRACT_LOG,             TOMS_ExtractLog));
    ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_EXECCB_GET_TERMSTATUS_INFO,     TOMS_GetTerminalStatus));

    ASSERT_FAIL(TOMS_Initialize());

    return APP_SUCC;
}

int TOMS_SetDebugSwitch(char cSwitch)
{
    if (cSwitch)
    {
        ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_DEBUG_SWITCH, TOMS_DEBUG_OPEN));
    }
    else
    {
        ASSERT_FAIL(TOMS_SetOption(TOMS_OPT_CONF_DEBUG_SWITCH, TOMS_DEBUG_CLOSE));
    }

    return APP_SUCC;
}

#endif /* USE_TOMS */

