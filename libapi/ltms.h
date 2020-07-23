/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved
** File name:  ltms.h
** File indentifier:
** Brief:
** Current Verion:  v1.0
** Auther: kyle
** Complete date: 2019-6-23
** Modify record:
** Modify date:
** Version:
** Modify content:
***************************************************************************/
#ifndef _LTMS_H_
#define _LTMS_H_

/**************************global data type definitions*******************/

typedef enum
{
    LTMS_DEBUG_CLOSE,                //close debug
    LTMS_DEBUG_OPEN,                 //open debug
}EM_LTMS_DEBUGSWITCH;

/**
* @brief libtms Output data type
*/
typedef enum
{
    LTMS_DATATYPE_LIBVER,              //Get version of tms library.
    LTMS_DATATYPE_CERTNUM,             //Get Carrier certificate number
    LTMS_DATATYPE_NEXTUPTTIME,         //Next check update time (The sub-application can judge whether the current time is greater than the next update time in the standby interface, and is used to implement the function of automatically checking and updating.)
}EM_LTMS_DATATYPE;

/**
* @brief Necessary communication data
*/
typedef struct
{
    char szDomain[50+1];
    char szCommType[1+1];
    char szFirmCode[128];                //POS manufacturer identification
    unsigned long lnAppDLTimeOut;        //Application download timeout. Default: 180 second.
    unsigned long lnOtherTimeOut;        //Other interface timeout. Default: 30 second.
    char szSSLKeyType[10+1];             //Just for HTTPS protocol
    char szSSLCerificateName[50+1];      //Just for HTTPS protocol
    char szCerificateID[32+1];           //Linux Terminal Certificate Number. Format: A 32-byte string. (A top-level organization with a certificate ID)
}TMSLIBCOMMPARAM;

/**
* @brief Callback function prototype definition
*/
typedef struct
{
    int (*CommConnect)(int );                                           //Establish a link
    int (*CommClose)(int );                                             //Close link
    int (*DispResultMsg)(char* , char* , char* , int );                 //Display transaction result information
    int (*SetCommDomain)(char* , int, int );                            //Need to switch the host domain name when downloading the app.
    void (*DispProgress)(char, unsigned int, unsigned int);             //Show download progress
    void (*ShowPromptMsg)(char* , char* , char , char , char , int );   //Display operation prompt information
    int (*ShowSelectPrompt)(char* , char* );                            //Prompt a menu with 2 options
    void (*LibTmsDebug)(const char*, int );                             //the function of output debug log
}TMSLIBOPER;

/**************************global function declarations*******************/

/**
* @brief Get data from libtms library
* @param [in]   emOutType
                    0: Get version of tms library.
                    1: et Carrier certificate number
                    2: Next check update time
* @param [out]   pszOutData
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_GetData(EM_LTMS_DATATYPE emOutType, char * pszOutData);

/**
* @brief Check whether downloaded apps exist and update them.
* @return
* @li APP_FAIL Fail
* @li APP_QUIT Quit
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_CheckAndInstallAll(void);

/**
* @brief Check if there is update and download apps and files.
* @param void
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_CheckUpdate(void);

/**
* @brief Set the necessary callback function for Libtms
* @param [in,out]
* @return
* @li APP_FAIL Faile
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_SetCallBack(TMSLIBOPER* );

/**
* @brief Set the necessary communication parameter for Libtms
* @param [in,out]
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_SetCommParam(TMSLIBCOMMPARAM* );

/**
* @brief Execute terminal active transcation
* @param [in,out]   Active code
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_ActiveDevice(char* );

/**
* @brief Initialize the libtms library
* @param void
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_Initialize(void);

/**
* @brief Setup Debug Output Mode
* @param [in]   emDebugSwitch
                    0: Close Debug
                    1: Open Debug
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2019-06-23
*/
extern int TMS_SetDebug(EM_LTMS_DEBUGSWITCH emDebugSwitch);

#endif

