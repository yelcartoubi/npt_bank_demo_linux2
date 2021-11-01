/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  ltoms.h
** File indentifier:
** Brief:
** Current Verion:  v1.0
** Auther: Tom chen
** Complete date: 2020-8-21
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/

#ifndef _LTOMS_H_
#define _LTOMS_H_
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TOMS_SUCC = 0,
    TOMS_FAIL = -1,
    TOMS_ERR_CREATEDIR = -2,
} TOMS_ERRCODE;

typedef enum
{
    TOMS_PROC_UPLOADAPPINFO,         //Upload App informations               [5.1, Passive]
    TOMS_PROC_UPLOADAPPLOADRESULT,   //Upload downloading result             [5.2, Passive]
    TOMS_PROC_QUERYAPPINFO,          //Query application information         [5.3, Initiative, Terminal timing trigger]
    TOMS_PROC_QUERYAPPDETAIL,        //Query application detail              [5.4, Initiative, Terminal timing trigger]
    TOMS_PROC_QUERYFIRMWAREINFO,     //Query firmware informaion             [5.5, Initiative, Terminal timing trigger]
    TOMS_PROC_OBTAINPARAMLIST,       //Obtain the application information list of the configured parameters of the terminal
    TOMS_PROC_DOWNLOADPARAM,         //Download online application parameter [5.6, Passive]
    TOMS_PROC_UPLOADPARAMRESULT,     //Upload parameter update result
    TOMS_PROC_GETRESOURCEPATH,       //Get resource download path            [5.7, Initiative]
    TOMS_PROC_GETCMD,                //Check command failure                 [5.8, Initiative, Query once every 30 minutes]
    TOMS_PROC_UPLOADCMDSTATUS,       //Upload command result                 [5.9, Initiative, Triggered when a command is received and after command execution]
    TOMS_PROC_UPLOADLOGFILES,        //Upload log file                       [5.11,Passive]
    TOMS_PROC_UPLOADLOGFILESRESULT,  //Upload log file result
    TOMS_PROC_BIND,                  //Bind terminal                         [5.12,Initiative Scan QR Code]
    TOMS_PROC_UNBIND,                //Unbind terminal                       [5.13,Passive]
    TOMS_PROC_GETAPPLOADURL,         //Get application download path         [5.14,Initiative According to the result of EXEC_TOMS_QUERYAPPINFO]
    TOMS_PROC_UPLOADTERMINALINFO,    //Upload Terminal status and infomation [6.1, Initiative, Terminal timing trigger]
    TOMS_PROC_GETDEVICEAUTHTOKEN,    //Get device authentication token       [7.1, Initiative, Triggered when a third-party application is invoked]
    TOMS_PROC_DOWNLOADAPP,           //Download app
    TOMS_PROC_DOWNLOADAPPLIST,       //Download app list
    TOMS_PROC_TERMINALBIND,          //Terminal bind
    TOMS_PROC_OBTAINRESOURCES,       //Get resources addr: boot logo , power on animation , power off animation
    TOMS_PROC_UPLOADTID,             //Upload TID
    TOMS_PROC_EXECCMD,               //Exec cmd
    TOMS_PROC_UPLOADTERMINALSTATUS,  //Upload Terminal Status
    TOMS_PROC_UPLOAD_PUBLICKEY,      //Upload Public Key
    TOMS_PROC_VERIFY_PUBLICKEY,      //Verify Public Key
    TOMS_PROC_AUTHTERMINAL,          //Authenticate terminal
} EM_TOMS_PROCEDURE;

typedef enum
{
    TOMS_UI_DOWNLOADING,
    TOMS_UI_LOW_BATTERY,
    TOMS_UI_ILLEGAL_CHARACTER,
    TOMS_UI_NO_UPDATE,
    TOMS_UI_PROCESSING,
    TOMS_UI_COMM_ERROR,
    TOMS_UI_NETWORK_ERROR,
    TOMS_UI_INSTALL_APP,
    TOMS_UI_INSTALL_PATCH,
    TOMS_UI_REBOOT,
    TOMS_UI_PROGRESS,
    TOMS_UI_RESULT,
    TOMS_UI_IS_DOWNLOAD_APP,
    TOMS_UI_IS_DOWNLOAD_PATCH,
    TOMS_UI_DNS_RESOLUTION_ERR,
    TOMS_UI_DOWNLOADING_APP,
    TOMS_UI_READCERT_FAIL,
    TOMS_UI_PARSINGPARAM,
    TOMS_UI_CFGINCORRECT,
    TOMS_UI_DOWNLOADING_RESOURCE,
    TOMS_UI_UPDATE_RESOURCE,
    TOMS_UI_UNINSTALL_APP,
    TOMS_UI_LOCK_TERMINAL,
    TOMS_UI_UNLOCK_TERMINAL,
    TOMS_UI_SET_SLEEP_DUR,
    TOMS_UI_GENERATE_KEYPAIR,
    TOMS_UI_PLS_AUTH_TERMINAL
} EM_TOMS_UI_ID;

typedef enum
{
    TOMS_OPR_DOWNLOAD_UPDATE,
    TOMS_OPR_EXIST_INSTALL
} EM_TOMS_OPERATE;

typedef enum {
	TOMS_SYS_MODEL,       /**<Model*/
	TOMS_SYS_SN,          /**<USN.*/
	TOMS_SYS_OS_VERSION,  /**<OS version*/
	TOMS_SYS_HW           /**<All hardware info*/
} EM_TOMS_SYSINFO_ID;

typedef enum {
    TOMS_VER_APP,
    TOMS_VER_PATCH,
    TOMS_VER_BOOT_LOGO,
    TOMS_VER_POWERON_ANIMATION,
    TOMS_VER_POWEROFF_ANIMATION,
} EM_TOMS_VERSION_TYPE;

typedef enum {
    TOMS_RES_BOOT_LOGO,
    TOMS_RES_POWERON_ANIMATION,
    TOMS_RES_POWEROFF_ANIMATION,
} EM_TOMS_RES_ID;

typedef enum {
    TOMS_FLOCK_LOCK,
    TOMS_FLOCK_UNLOCK
} EM_TOMS_FLOCK;

typedef enum {
    TOMS_SS_POWER_OPERATION_COUNTS,
    TOMS_SS_PRINT_MITERS_COUNTS,
    TOMS_SS_USB_OPERATION_COUNTS,
    TOMS_SS_STORAGE_AVAILABLE,
    TOMS_SS_STORATE_BRUSH_COUNTS,
    TOMS_SS_POWERBUTTON_PRESSED_COUNTS,
    TOMS_SS_SCREEN_TOUCHED_COUNTS,
    TOMS_SS_CAMERA_OPENED_COUNTS,
    TOMS_SS_BATTERY_OPERATION_COUNTS,
    TOMS_SS_IMEI,
    TOMS_SS_WIFI_STATUS,
    TOMS_SS_SSID,
    TOMS_SS_BLUETOOTH_STATUS,
    TOMS_SS_DEVICE_MANUFACTURER,
    TOMS_SS_HARDWARE_CONFIG,
    TOMS_SS_HARDWARE_ID,
    TOMS_SS_PATCH_LIST,
    TOMS_SS_CAMERA_BACK_MODEL,
    TOMS_SS_CAMERA_FRONT_MODEL,
    TOMS_SS_SECURE_APP_VERSION,
    TOMS_SS_SECURE_FW_VERSION,
    TOMS_SS_SECURE_BOOT_VERSION,
    TOMS_SS_SYS_FW_ID,
} EM_TOMS_TERMSTATUS_INFO_ID;

typedef enum {
    TOMS_APP_VERSION_NAME,
} EM_TOMS_APP_INFO_ID;

typedef enum {
    TOMS_NETTYPE_UNKNOWN,
    TOMS_NETTYPE_WIFI,
    TOMS_NETTYPE_2G,
    TOMS_NETTYPE_3G,
    TOMS_NETTYPE_4G,
} EM_TOMS_NETWORK_TYPE;

typedef enum {
    TOMS_PRINTER_NORMAL,
    TOMS_PRINTER_EXCEPTION,
    TOMS_PRINTER_OUT_OF_PAPER,
} EM_TOMS_PRINTER_STATUS;

typedef enum {
    TOMS_SIM_SIGNAL_STRENGTH,
    TOMS_SIM_SIGNAL_LEVEL
} EM_TOMS_SIM_STATUS;

typedef enum {
    TOMS_DEBUG_CLOSE,
    TOMS_DEBUG_OPEN,
} EM_TOMS_DEBUG_SWITCH;


#define TOMSOPT(n, b, o) n = (b) + (o)
#define TOMS_OPTTYPE_CONFIG_BASE         (0)
#define TOMS_OPTTYPE_SYSAPI_BASE         (10000)
#define TOMS_OPTTYPE_EXECCB_BASE         (20000)  // exec callback

typedef enum {
    //TOMS_OPTTYPE_CONFIG_BASE
    TOMSOPT(TOMS_OPT_CONF_APP_DOMAIN,               TOMS_OPTTYPE_CONFIG_BASE,    1),         // string
    TOMSOPT(TOMS_OPT_CONF_PARAM_DOMAIN,             TOMS_OPTTYPE_CONFIG_BASE,    2),         // string
    TOMSOPT(TOMS_OPT_CONF_KEYPOS_DOMAIN,            TOMS_OPTTYPE_CONFIG_BASE,    3),         // string
    TOMSOPT(TOMS_OPT_CONF_FILESERVER_DOMAIN,        TOMS_OPTTYPE_CONFIG_BASE,    4),         // string
    TOMSOPT(TOMS_OPT_CONF_TDAS_DOMAIN,              TOMS_OPTTYPE_CONFIG_BASE,    5),         // string
    TOMSOPT(TOMS_OPT_CONF_DEBUG_SWITCH,             TOMS_OPTTYPE_CONFIG_BASE,    6),         // EM_TOMS_DEBUG_SWITCH
    TOMSOPT(TOMS_OPT_CONF_USER_OID,                 TOMS_OPTTYPE_CONFIG_BASE,    7),         // string

    //TOMS_OPTTYPE_SYSAPI_BASE
    TOMSOPT(TOMS_OPT_SYSAPI_GET_DATETIME,           TOMS_OPTTYPE_SYSAPI_BASE,    1),         // toms_get_datetime_cb
    TOMSOPT(TOMS_OPT_SYSAPI_SYS_REBOOT,             TOMS_OPTTYPE_SYSAPI_BASE,    2),         // toms_reboot_cb
    TOMSOPT(TOMS_OPT_SYSAPI_APP_INSTALL,            TOMS_OPTTYPE_SYSAPI_BASE,    3),         // toms_app_install_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_SYSINFO,            TOMS_OPTTYPE_SYSAPI_BASE,    4),         // toms_get_sysinfo_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_VOLPERCENT,         TOMS_OPTTYPE_SYSAPI_BASE,    5),         // toms_get_volpercent_cb
    TOMSOPT(TOMS_OPT_SYSAPI_APP_UNINSTALL,          TOMS_OPTTYPE_SYSAPI_BASE,    6),         // toms_app_uninstall_cb
    TOMSOPT(TOMS_OPT_SYSAPI_SET_SLEEP_DUR,          TOMS_OPTTYPE_SYSAPI_BASE,    7),         // toms_set_sleep_duration_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_APPNAME_BY_INDEX,   TOMS_OPTTYPE_SYSAPI_BASE,    8),         // toms_get_appname_by_index_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_APPINFO_BY_NAME,    TOMS_OPTTYPE_SYSAPI_BASE,    9),         // toms_get_appinfo_by_name_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_NETWORK_TYPE,       TOMS_OPTTYPE_SYSAPI_BASE,    10),        // toms_get_network_type_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_SIM_STATUS,         TOMS_OPTTYPE_SYSAPI_BASE,    11),        // toms_get_sim_status_cb
    TOMSOPT(TOMS_OPT_SYSAPI_GET_PRINER_STATUS,      TOMS_OPTTYPE_SYSAPI_BASE,    12),        // toms_get_printer_status_cb
    TOMSOPT(TOMS_OPT_SYSAPI_PARSE_DNS_IP,           TOMS_OPTTYPE_SYSAPI_BASE,    13),        // toms_parse_dns_ip_cb

    //TOMS_OPTTYPE_EXECCB_BASE
    TOMSOPT(TOMS_OPT_EXECCB_COMM_CONNECT,           TOMS_OPTTYPE_EXECCB_BASE,    1),        // toms_comm_connect_cb
    TOMSOPT(TOMS_OPT_EXECCB_COMM_CLOSE,             TOMS_OPTTYPE_EXECCB_BASE,    2),        // toms_comm_close_cb
    TOMSOPT(TOMS_OPT_EXECCB_UI_DISPLAY,             TOMS_OPTTYPE_EXECCB_BASE,    3),        // toms_ui_display_cb
    TOMSOPT(TOMS_OPT_EXECCB_LOG_OUTPUT,             TOMS_OPTTYPE_EXECCB_BASE,    4),        // toms_log_output_cb
    TOMSOPT(TOMS_OPT_EXECCB_PARAM_PARSING,          TOMS_OPTTYPE_EXECCB_BASE,    5),        // toms_param_parsing_cb
    TOMSOPT(TOMS_OPT_EXECCB_CHECK_VERSION,          TOMS_OPTTYPE_EXECCB_BASE,    6),        // toms_check_version_cb
    TOMSOPT(TOMS_OPT_EXECCB_UPDATE_RES,             TOMS_OPTTYPE_EXECCB_BASE,    7),        // toms_update_res_cb
    TOMSOPT(TOMS_OPT_EXECCB_ISLOCK_TERMINAL,        TOMS_OPTTYPE_EXECCB_BASE,    8),        // toms_lock_terminal_cb
    TOMSOPT(TOMS_OPT_EXECCB_EXTRACT_LOG,            TOMS_OPTTYPE_EXECCB_BASE,    9),        // toms_extract_log_cb
    TOMSOPT(TOMS_OPT_EXECCB_GET_TERMSTATUS_INFO,    TOMS_OPTTYPE_EXECCB_BASE,    10),       // toms_get_terminal_status_cb
    TOMSOPT(TOMS_OPT_EXECCB_GET_ISLOCK_TERMINAL,    TOMS_OPTTYPE_EXECCB_BASE,    12),       // toms_get_lock_status_cb
} EM_TOMS_OPT;

//TOMS_OPTTYPE_SYSAPI_BASE
typedef TOMS_ERRCODE (*toms_get_datetime_cb)(struct tm *pstDateTime);
typedef TOMS_ERRCODE (*toms_get_sysinfo_cb)(EM_TOMS_SYSINFO_ID InfoID, char *pszOutBuf, int *pnOutBufLen);
typedef TOMS_ERRCODE (*toms_get_volpercent_cb)(int *nVolPercent);
typedef TOMS_ERRCODE (*toms_get_appname_by_index_cb)(int nIndex, char *pszOutAppName, int nMaxLen);
typedef TOMS_ERRCODE (*toms_get_appinfo_by_name_cb)(const char *pszAppName, EM_TOMS_APP_INFO_ID emID, char *pszOutInfo, int nMaxLen);
typedef TOMS_ERRCODE (*toms_get_network_type_cb)(EM_TOMS_NETWORK_TYPE *pemOutType);
typedef TOMS_ERRCODE (*toms_get_sim_status_cb)(EM_TOMS_SIM_STATUS emType, int *pnOutResult);
typedef TOMS_ERRCODE (*toms_get_printer_status_cb)(EM_TOMS_PRINTER_STATUS *pemOutType);
typedef void         (*toms_reboot_cb)(void);
typedef TOMS_ERRCODE (*toms_set_sleep_duration_cb)(int nSleepDur);
typedef TOMS_ERRCODE (*toms_app_install_cb)(const char *FilePath);
typedef TOMS_ERRCODE (*toms_app_uninstall_cb)(const char *pszAppName);

// TOMS_OPTTYPE_EXECCB_BASE callback
typedef TOMS_ERRCODE (*toms_comm_connect_cb)(int nIsUpdateUI, EM_TOMS_PROCEDURE emTrantpye);
typedef TOMS_ERRCODE (*toms_comm_close_cb)(int nHangUpFlag);
typedef TOMS_ERRCODE (*toms_ui_display_cb)(EM_TOMS_UI_ID emUIID, unsigned char* uszUIData1, unsigned char* uszUIData2, unsigned char* uszUIData3);
typedef TOMS_ERRCODE (*toms_param_parsing_cb)(const char *szFilePath, const char *pszStoragePath, const char* szAppName);
typedef TOMS_ERRCODE (*toms_check_version_cb)(EM_TOMS_VERSION_TYPE emType, const char *pszAppName, const char* pszVer);
typedef void         (*toms_log_output_cb)(const char* pszLog, int nLen);
typedef TOMS_ERRCODE (*toms_update_res_cb)(EM_TOMS_RES_ID emRes, char* pszResPath, char *pszMD5);
typedef TOMS_ERRCODE (*toms_extract_log_cb)(const char *pszPerFileNameFmt, const char *pszStartTime, const char *pszEndTime, char *pszOutDir);
typedef TOMS_ERRCODE (*toms_lock_terminal_cb)(EM_TOMS_FLOCK emlock, const char *pszTipsInfo);
typedef TOMS_ERRCODE (*toms_get_lock_status_cb)(EM_TOMS_FLOCK *pemlock);
typedef TOMS_ERRCODE (*toms_get_terminal_status_cb)(EM_TOMS_TERMSTATUS_INFO_ID emSS, char *pszOutInfo, int nMaxLen);

typedef TOMS_ERRCODE (*toms_parse_dns_ip_cb)(const char *pszDns, char *pszOutIp, int nMaxLen);

/**
* @brief set toms options such as configures, callbacks and so on.
*        Please strictly follow the instruction of 'EM_TOMS_OPT'
* @param [in, in...]
* @return
* @li TOMS_FAIL Fail
* @li TOMS_SUCC Success
* @date 2020-08-21
*/
TOMS_ERRCODE TOMS_SetOption(EM_TOMS_OPT emOpt, ...);

/**
* @brief Init TOMS module
* @param
* @return
* @li \ref TOMS_ERRCODE
* @li TOMS_SUCC Success
* @date 2020-08-21
*/
TOMS_ERRCODE TOMS_Initialize(void);

/**
* @brief Get the version of toms library
* @param
* @return
* @li toms libary version like: x.x.xx
* @date 2020-08-21
*/
char *TOMS_GetLibVersion(void);

/**
* @brief Check if there is update and download apps and parameter files.
* @param emOpr	Type of operation
* @return
* @li TOMS_FAIL Fail
* @li TOMS_SUCC Success
* @date 2020-08-21
*/
TOMS_ERRCODE TOMS_CheckUpdate(EM_TOMS_OPERATE emOpr);

/**
* @brief Check if there is commands to be executed.
* @param
* @return
* @li TOMS_FAIL Fail
* @li TOMS_SUCC Success
* @date 2020-08-21
*/
TOMS_ERRCODE TOMS_CheckRemoteCmds(void);

/**
* @brief Authenticate terminal. we should call it first.
* @param
* @return
* @li TOMS_FAIL Fail
* @li TOMS_SUCC Success
* @date 2020-08-21
*/
TOMS_ERRCODE TOMS_AuthTerminal(void);

#ifdef __cplusplus
}
#endif


#endif /* _LTOMS_H_ */

