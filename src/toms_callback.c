/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
* @file  toms_callback.c
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
#include "napi_wlm.h"
#include "napi_printer.h"
#include "napi_app.h"

/*******************************
 * Macro Definition
 *******************************/
#define TOMS_RESULT_CODE_SUCC		"00"
#define TOMS_SUCC_CONTENT			"SUCCESS"
#define TOMS_FAIL_CONTENT			"FAIL"

#define TOMS_RESOURCE_FILE          "toms_resource_file"
/*******************************
 * Type Definition
 *******************************/
typedef struct
{
    EM_TOMS_PROCEDURE nTomsFuncType;
    char* pszPrompt;
}ST_TOMS_UIPROMPT;

/*******************************
 * Global Variable Definition
 *******************************/
//static char gszBackupDomain[50+1] = {0};
//static int  gnBackupPort = 0;

const static ST_TOMS_UIPROMPT gszTransPrompt[] = {
    {TOMS_PROC_UPLOADAPPINFO,        "Upload App Info"},             //Upload App informations               [5.1, Passive]
    {TOMS_PROC_UPLOADAPPLOADRESULT,  "Upload Downloading Result"},   //Upload downloading result             [5.2, Passive]
    {TOMS_PROC_QUERYAPPINFO,         "Query App Info"},              //Query application information         [5.3, Initiative, Terminal timing trigger]
    {TOMS_PROC_QUERYAPPDETAIL,       "Query App Detail"},            //Query application detail              [5.4, Initiative, Terminal timing trigger]
    {TOMS_PROC_QUERYFIRMWAREINFO,    "Query Firmware Info"},         //Query firmware informaion             [5.5, Initiative, Terminal timing trigger]
    {TOMS_PROC_OBTAINPARAMLIST,      "Obtain Param List"},           //Obtain the application information list of the configured parameters of the terminal
    {TOMS_PROC_DOWNLOADPARAM,        "DownLoad Param"},              //Download online application parameter [5.6, Passive]
    {TOMS_PROC_UPLOADPARAMRESULT,    "Upload Param Result"},         //Upload parameter update result
    {TOMS_PROC_GETRESOURCEPATH,      "Upload App Infor Result"},     //Get resource download path            [5.7, Initiative]
    {TOMS_PROC_GETCMD,               "Check Command"},               //Check command failure                 [5.8, Initiative, Query once every 30 minutes]
    {TOMS_PROC_UPLOADCMDSTATUS,      "Upload Command Status"},       //Upload command result                 [5.9, Initiative, Triggered when a command is received and after command execution]
    {TOMS_PROC_UPLOADLOGFILES,       "Upload Log File"},             //Upload log file                       [5.11,Passive]
    {TOMS_PROC_UPLOADLOGFILESRESULT, "Upload Log File Result"},      //Upload log file result
    {TOMS_PROC_BIND,                 "Bind terminal"},               //Bind terminal                         [5.12,Initiative Scan QR Code]
    {TOMS_PROC_UNBIND,               "Upload App Info"},             //Unbind terminal                       [5.13,Passive]
    {TOMS_PROC_GETAPPLOADURL,        "Get AppLoadUrl"},              //Get application download path         [5.14,Initiative According to the result of EXEC_TOMS_QUERYAPPINFO]
    {TOMS_PROC_UPLOADTERMINALINFO,   "Upload Terminal Info"},        //Upload Terminal status and infomation [6.1, Initiative, Terminal timing trigger]
    {TOMS_PROC_GETDEVICEAUTHTOKEN,   "Get Device AuthToken"},        //Get device authentication token       [7.1, Initiative, Triggered when a third-party application is invoked]
    {TOMS_PROC_DOWNLOADAPP,          "Download App"},                //Download App                          [7.1, Initiative, Triggered when a third-party application is invoked]
    {TOMS_PROC_DOWNLOADAPPLIST,      "Download App List"},           //
    {TOMS_PROC_TERMINALBIND,         "Terminal Bind"},               //
    {TOMS_PROC_OBTAINRESOURCES,      "Obtain Resources"},
    {TOMS_PROC_UPLOADTID,            "Upload TID"},
    {TOMS_PROC_EXECCMD,              "Exec Commands"},
    {TOMS_PROC_UPLOADTERMINALSTATUS, "Upload Terminal status info"},
    {TOMS_PROC_UPLOAD_PUBLICKEY,     "Upload Public Key"},
    {TOMS_PROC_VERIFY_PUBLICKEY,     "Verify Public Key"},
    {TOMS_PROC_AUTHTERMINAL,         "Authenticate Terminal"},
};

/*******************************
 * Local Function Declaration
 *******************************/
/**
 * @brief  Display progress of current downloading
 * @param
 * @return
 * @li 0 SUCC
 * @author Shawn Lian
 * @date 2020-7-1
 */
static void DispProgress(char cFlag, unsigned char* uszUIData2, unsigned char* uszUIData3);

/**
 * @brief  Get prompt content
 * @param
 * @return	String	Prompt content
 * @author Shawn Lian
 * @date 2020-7-1
 */
static char* getPromptByType(char cTransType);

static int DispPrompt(unsigned char* pszTitle, unsigned char* pszData);

static int DispPromptNotTitle(unsigned char* pszData);

static int DispResultMsg(char* pszTitle, char* pszCode, char* pszMsg, int nTimeout);

static inline int __get_resource_from_file(EM_TOMS_RES_ID emRes, char*pszResPath, char *pszMD5);

/*******************************
 * Local Function Definition
 *******************************/
/**
 * @brief  Display progress of current downloading
 * @param
 * @return
 * @li 0 SUCC
 * @author Shawn Lian
 * @date 2020-7-1
 */
static void DispProgress(char cFlag, unsigned char* uszUIData2, unsigned char* uszUIData3)
{
	static uint unWidth=0, unHeight=0;
	static uint unX=0, unY=0;
	static uint unFHeight=0;
	int nTemp;
	float fSchedule;
	static int nTempOld = 1234;//randon value
	static float fScheduleOld = 101.0;//randon value
	char cIsUpdateWindow = NO;
	char szBuf[20] = {0};
	uint nCur = 0;
	uint nTotal = 0;

	PubC4ToInt(&nCur, uszUIData2);
	PubC4ToInt(&nTotal, uszUIData3);

	if(1 == cFlag)
	{
		PubGetDispForm(NULL, NULL, (int*)&unFHeight);

		NAPI_ScrGetViewPort(&unX, &unY, &unWidth, &unHeight);
		NAPI_ScrRectangle(1, unHeight - 32, unWidth - 2, 32, RECT_PATTERNS_NO_FILL, 0x00);

		nTempOld = 1234;		//randon value
		fScheduleOld = 101.0;	//randon value
		cIsUpdateWindow = YES;
	}
	nTemp = (unWidth - 4) * nCur / nTotal;
	if(nTemp != nTempOld)
	{
		NAPI_ScrRectangle(3, unHeight - 31, nTemp, 31, RECT_PATTERNS_SOLID_FILL, 0x3384);
		nTempOld = nTemp;
		cIsUpdateWindow = YES;
	}
	fSchedule = nCur * 100.0 / nTotal;
	if(fSchedule != fScheduleOld)
	{
		sprintf(szBuf, "Completed:%4.1f%%", fSchedule);
		NAPI_ScrClrLine(unHeight - 32 - unFHeight - 1, unHeight - 32 - 1);
		NAPI_ScrDispTrueTypeFontText(3, unHeight - 32 - unFHeight - 1, szBuf, strlen(szBuf));
		fScheduleOld = fSchedule;
		cIsUpdateWindow = YES;
	}
	if(YES == cIsUpdateWindow)
	{
		PubUpdateWindow();
	}
}

/**
* @brief  Establish a link
* @param in nIsUpdateWindow	1:[Display prompt info], 0:[No prompt]
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
TOMS_ERRCODE TOMS_CommConnect(int nIsUpdateUI, EM_TOMS_PROCEDURE emTrantpye)
{
	int nRet;
	STAPPCOMMPARAM stAppCommParam;
	STSHOWINFOXY stShowInfoxy;

    memset(&stAppCommParam, 0, sizeof(stAppCommParam));
	 GetAppCommParam(&stAppCommParam);
	if(stAppCommParam.cCommType == COMM_DIAL || stAppCommParam.cCommType == COMM_RS232)
	{
		PubMsgDlg("Info", "Currnet comm type does not support HTTP", 3, 3);
		return TOMS_FAIL;
	}

	if(nIsUpdateUI == 1)
	{
		PubClear2To4();
		PubDisplayTitle(getPromptByType(emTrantpye));
		PubDisplayStrInline(0, 3, "Connect to network...");
		//PubDisplayStrInline(0, 4, "<Cancel> to quit");
		PubUpdateWindow();
	}
	memset(&stShowInfoxy, 0, sizeof(stShowInfoxy));
	stShowInfoxy.nColumn = 9;
	stShowInfoxy.nRow = 2;
	stShowInfoxy.nType = 0;
	PubSetShowXY(stShowInfoxy);
	CommSetSuspend(0);

	nRet = PubCommDialNet();
	if (nRet != APP_SUCC )
	{
		PubDispErr("fail to open ppp");
		return TOMS_FAIL;
	}

	return TOMS_SUCC;
}

/**
* @brief  Close link
* @param in nHangFlag    0: Non-block; 1: Block
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
TOMS_ERRCODE TOMS_CommClose(int nHangFlag)
{
	STAPPCOMMPARAM stAppCommParam;
	memset(&stAppCommParam, 0, sizeof(stAppCommParam));

	GetAppCommParam(&stAppCommParam);
	CommSetSuspend(1);
	if (COMM_GPRS == stAppCommParam.cCommType || COMM_CDMA == stAppCommParam.cCommType)
	{
		if (stAppCommParam.cMode == 1 || stAppCommParam.cMode == '1' )
		{
			return TOMS_SUCC;
		}
		else/**<Short link, need to close PPP*/
		{
			return CommHungUp(nHangFlag);
		}
	}
	return TOMS_SUCC;
}

/**
 * @brief  Get prompt content
 * @param
 * @return	String	Prompt content
 * @author Shawn Lian
 * @date 2020-7-1
 */
static char* getPromptByType(char cTransType)
{
	int i;

	for (i = 0; i < sizeof(gszTransPrompt)/sizeof(gszTransPrompt[0]); i++)
	{
	    if (cTransType == gszTransPrompt[i].nTomsFuncType)
	    {
			return gszTransPrompt[i].pszPrompt;
			break;
	    }
	}
	return "";
}

/**
* @brief  Display operation prompt information
* @param in pszTitle
* @param in pszPromptMsg  			Prompt message
* @param in cIsClrAll  				1: clear screen  other: Clear from the second line to the last line
* @param in cIsUpdate  				1: Update & display screen other: No operation
* @param in cIsWait  				i: wait 0: Do not wait
* @param in nTimout  				Wait Time, Timeout(>0) can be set, set 0 for blocking unless any key is pressed.
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
void ShowPromptMsg(char* pszTitle, char* pszPromptMsg, char cIsClrAll, char cIsUpdate, char cIsWait, int nTimout)
{
	if(cIsClrAll == 1)
	{
		PubClearAll();
	}
	else
	{
		PubClear2To4();
	}

	if(pszTitle != NULL)
	{
		PubDisplayTitle(pszTitle);
	}

	if(pszPromptMsg != NULL)
	{
		PubDisplayStr(0, 3, 1, "%s", tr(pszPromptMsg));
	}

	if(cIsUpdate == 1)
	{
		PubUpdateWindow();
	}

	if(cIsWait == 1)
	{
		PubGetKeyCode(nTimout);
	}
}

/**
* @brief  Prompt a menu with 2 options
* @param in pszTitle
* @param in pszPromptMsg  			Prompt message
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
int ShowSelectPrompt(unsigned char* pszTitle, unsigned char* pszPromptMsg)
{
	char cSelect = '1', nRet = -1;

	ASSERT_QUIT(PubSelectYesOrNo(tr((char*)pszTitle), (char*)pszPromptMsg, NULL, &cSelect));
	switch(cSelect)
	{
	case '0':
		nRet = 0;
		break;
	case '1':
		nRet = 1;
		break;
	default:
		return nRet;
	}
	return nRet;
}


static int DispPrompt(unsigned char* pszTitle, unsigned char* pszData)
{
	PubClear2To4();
	PubDisplayTitle(( char*)pszTitle);
	PubDisplayStr(0, 3, 1, "%s", pszData);
	PubUpdateWindow();
	return APP_SUCC;
}

static int DispPromptNotTitle(unsigned char* pszData)
{
	PubClear2To4();
	PubDisplayStr(0, 3, 1, "%s", pszData);
	PubUpdateWindow();
	return APP_SUCC;
}

static int DispResultMsg(char* pszTitle, char* pszCode, char* pszMsg, int nTimeout)
{
	char szTmp[512+64] = {0};
	char szMsg[512] = {0};

	if(strcmp(pszCode, TOMS_RESULT_CODE_SUCC) == 0)
	{
		strcpy(szMsg, TOMS_SUCC_CONTENT);
		sprintf(szTmp, "\n\n%s", szMsg);
	}
	else if(pszMsg == NULL || strlen(pszMsg) == 0)
	{
		strcpy(szMsg, TOMS_FAIL_CONTENT);
		sprintf(szTmp, "\n%s\n%s", pszCode, szMsg);
	}
	else
	{
		strcpy(szMsg, pszMsg);
		sprintf(szTmp, "\n%s\n%s", pszCode, pszMsg);
	}

	PubMsgDlg(pszTitle, szTmp, 0, nTimeout);

	return APP_SUCC;
}

TOMS_ERRCODE TOMS_DispUIEvent(EM_TOMS_UI_ID emUIID, unsigned char* uszUIData1, unsigned char* uszUIData2, unsigned char* uszUIData3)
{
	char szContent[128] = {0};


	switch(emUIID)
	{
   	case TOMS_UI_DOWNLOADING:
		DispPrompt((uchar *)"Downloading", uszUIData1);
		break;
    case TOMS_UI_LOW_BATTERY:
		DispPrompt((uchar *) "Warning", (uchar *) "LOW BATTERY\nPLS CHARGE");
		break;
    case TOMS_UI_ILLEGAL_CHARACTER:
		if(uszUIData1[0] == 0)
		{
			sprintf(szContent, "AppId [\"%s\"] contains illegal character[\'%s\']", uszUIData2, uszUIData3);
		}
		else
		{
			sprintf(szContent, "Version [\"%s\"] contains illegal character[\'%s\']", uszUIData2, uszUIData3);
		}
		DispPrompt((uchar *) "Warning", (uchar *)szContent);
		PubWaitConfirm(60);
		break;
    case TOMS_UI_NO_UPDATE:
		DispPrompt(NULL, (uchar *)"No Update");
		PubWaitConfirm(2);
		break;
    case TOMS_UI_PROCESSING:
		DispPrompt((uchar *) getPromptByType(uszUIData1[0]),(uchar *) "Processing...");
		break;
    case TOMS_UI_COMM_ERROR:
		sprintf(szContent, "%s:%s\n %s", "Comm Err", uszUIData2, "Pls try again later");
		DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar *)szContent);
		PubWaitConfirm(60);
		break;
    case TOMS_UI_NETWORK_ERROR:
		sprintf(szContent, "%s:%s\n %s", "Network Busy", uszUIData2, "Pls try again later");
		DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar *)szContent);
		PubWaitConfirm(60);
		break;
    case TOMS_UI_INSTALL_APP:
		DispPrompt((uchar *) "APP Installing", (uchar *) "Do not power off!!!");
		break;
    case TOMS_UI_INSTALL_PATCH:
		DispPrompt((uchar *) "PATCH Installing",(uchar *) "Do not power off!!!");
		break;
    case TOMS_UI_REBOOT:
		DispPrompt((uchar *)"Rebooting",(uchar *) "Do not power off!!!");
		break;
    case TOMS_UI_PROGRESS:
		DispProgress(uszUIData1[0], uszUIData2, uszUIData3);
		break;
    case TOMS_UI_RESULT:
		DispResultMsg(getPromptByType(uszUIData1[0]), (char*)uszUIData2, (char*)uszUIData3, 3);
		break;
    case TOMS_UI_IS_DOWNLOAD_APP:
		return ShowSelectPrompt((uchar *)"UPDATE APP", uszUIData1);
		break;
    case TOMS_UI_IS_DOWNLOAD_PATCH:
		return ShowSelectPrompt((uchar *)"UPDATE PATCH", uszUIData1);
		break;
	case TOMS_UI_DNS_RESOLUTION_ERR:
		DispPrompt((uchar *) getPromptByType(uszUIData1[0]),(uchar *) "DNS Resolution failure");
		PubWaitConfirm(60);
		break;
	case TOMS_UI_DOWNLOADING_APP:
		DispPrompt((uchar *) getPromptByType(uszUIData1[0]), uszUIData2);
		break;
    case TOMS_UI_DOWNLOADING_RESOURCE:
        DispPromptNotTitle((uchar *)"Downloading Resource");
        break;
    case TOMS_UI_UPDATE_RESOURCE:
        DispPromptNotTitle((uchar *)"Update Resource");
        break;
	case TOMS_UI_READCERT_FAIL:
		sprintf(szContent,"unable to parse certificate in: %s\n",uszUIData1);
		DispPromptNotTitle((uchar *)szContent);
		break;
    case TOMS_UI_UNINSTALL_APP:
        DispResultMsg(getPromptByType(uszUIData1[0]),(char *)"uninstall" , (char*)uszUIData2, 3);
        break;
    case TOMS_UI_LOCK_TERMINAL:
        DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar*)"lock terminal");
        break;
    case TOMS_UI_UNLOCK_TERMINAL:
        DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar*)"unlock terminal");
        break;
    case TOMS_UI_SET_SLEEP_DUR:
        DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar*)"set sleep duration");
        break;
    case TOMS_UI_GENERATE_KEYPAIR:
        DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar*)"generate key pair");
        break;
    case TOMS_UI_PLS_AUTH_TERMINAL:
        DispPrompt((uchar *) getPromptByType(uszUIData1[0]), (uchar*)"please authenticate terminal");
        TxnWaitAnyKey(1);
        break;
	default:
		break;
	}

    return TOMS_SUCC;
}

/**
* @brief  Output libtoms debug log
* @param in log data
* @param in log len
* @return
* @author kyle
* @date 2019-06-23
*/
void TOMS_LogOutput(const char* pszLog, int nLen)
{
	PubBufToAux(pszLog, nLen);
	return ;
}

#define ASSERT_TOMSPARAM_FAIL(e, m) \
if (e != APP_SUCC) { \
	PubMsgDlg(tr("TOMS PARAM"), m, 1, 3); \
	return APP_FAIL; \
}

/**
* @brief	Return COMM_TYPE according to intput flag
* @param [in] pszFlag		Input flag
* @return
* @li Ref to type COMM_DIAL
* @date 2020-07-10
*/
static int GetCommTypeByFlag(EM_TOMS_PARAM_COMMTYPE emFlag)
{
	switch(emFlag)
	{
		case TOMS_COMMTYPE_DAIL:
			return COMM_DIAL;
			break;

		case TOMS_COMMTYPE_ETH:
			return COMM_ETH;
			break;

		case TOMS_COMMTYPE_CDMA:
			return COMM_CDMA;
			break;

		case TOMS_COMMTYPE_GPRS:
			return COMM_GPRS;
			break;

		case TOMS_COMMTYPE_RS232:
			return COMM_RS232;
			break;

		case TOMS_COMMTYPE_WIFI:
			return COMM_WIFI;
			break;

		default:
			return COMM_NONE;
			break;
	}
}

/**
* @brief	Return COMM_TYPE according to intput flag
* @param [in] pszFlag		Input flag
* @return
* @li Ref to type COMM_DIAL
* @date 2020-07-10
*/
static int GetWifiTypeByFlag(EM_TOMS_PARAM_COMMTYPE emFlag)
{
	switch(emFlag)
	{
		case TOMS_WIFITYPE_OPEN:
			return WIFI_AUTH_OPEN;
			break;

		case TOMS_WIFITYPE_WEP:
			return WIFI_AUTH_WEP_PSK;
			break;

		case TOMS_WIFITYPE_WPA_PSK:
			return WIFI_AUTH_WPA_PSK;
			break;

		case TOMS_WIFITYPE_WPA2_PSK:
			return WIFI_AUTH_WPA2_PSK;
			break;

		case TOMS_WIFITYPE_WPA_CCMK:
			return WIFI_AUTH_WPA_WPA2_MIXED_PSK;
			break;

		default:
			return 5;
			break;
	}
}

/**
* @brief	Check if JSON node is valid.
* @param [in] CheckNode		cJSON node to be checked
* @param [in] nType			Type cJSON node should be
* @return
* @li APP_FAIL JSON node is invalid
* @li APP_SUCC JSON node is valid
* @date 2020-07-10
*/
static int CheckJsonNode(cJSON *CheckNode, int nType)
{
	if(CheckNode == NULL)
	{
		//TRACE("JSON node is NULL.");
		return APP_FAIL;
	}
	else if(CheckNode->type != nType)
	{
		//TRACE("JSON node type mismatching. expect_type=%d, actual_type = %d", nType, CheckNode->type);
		return APP_FAIL;
	}
	else
	{
		switch(CheckNode->type)
		{
			case cJSON_String:
				if(CheckNode->valuestring == NULL)
				{
					//TRACE("JSON node has no target value. CheckNode->type=", CheckNode->type);
					return APP_FAIL;
				}else{/* do nothing */}
				break;

			case cJSON_Array:
			case cJSON_Object:
				if(CheckNode->child == NULL)
				{
					//TRACE("JSON node has no target value. CheckNode->type=", CheckNode->type);
					return APP_FAIL;
				}else{/* do nothing */}
				break;

			default:
				break;
		}
	}

	return APP_SUCC;
}

/**
* @brief	Get parameter's value from TOMS Json
* @param [in] ParamNode    "paramValues" node
* @param [in] pszInStr		Key string
* @param [in] emOutType		Output value type
* @param [out] pOut			Output value
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2020-07-10
*/
static int GetTomsParamValueFromJson(cJSON *ParamNode, char * pszInStr, void *pOut, EM_TOMS_PARAM_TYPE emOutType)
{
	cJSON *TargetNode = NULL, *TypeNode = NULL, *ValueNode = NULL, *SubValueNode = NULL;
	char szOutTypeStr[10+1] = {0};
	int nType = cJSON_NULL;

	//TRACE("=== Start parse param <%s> ===", pszInStr);
	TargetNode = cJSON_GetObjectItem(ParamNode, pszInStr);
	ASSERT_FAIL(CheckJsonNode(TargetNode, cJSON_Object));

	TypeNode = cJSON_GetObjectItem(TargetNode, TOMSTAG_TYPE);
	ASSERT_FAIL(CheckJsonNode(TypeNode, cJSON_String));
	switch(emOutType)
	{
		case TOMSTYPE_STRING:
			strcpy(szOutTypeStr, TOMSTYPE_STR_STRING);
			nType = cJSON_String;
			break;

		case TOMSTYPE_BOOLEAN:
			strcpy(szOutTypeStr, TOMSTYPE_STR_BOOLEAN);
			nType = cJSON_String;
			break;

		case TOMSTYPE_NUMBER:
			strcpy(szOutTypeStr, TOMSTYPE_STR_NUMBER);
			nType = cJSON_String;
			break;

		case TOMSTYPE_HEX:
			strcpy(szOutTypeStr, TOMSTYPE_STR_HEX);
			nType = cJSON_String;
			break;

		case TOMSTYPE_REFERENCE:
			strcpy(szOutTypeStr, TOMSTYPE_STR_REFERENCE);
			nType = cJSON_Object;
			break;
        case TOMSTYPE_FILE:
            strcpy(szOutTypeStr, TOMSTYPE_STR_FILE);
            nType = cJSON_String;
			break;
		default:
			//TRACE("Output type out of range. [emOutType=%d]", emOutType);
			return APP_FAIL;
			break;
	}
	if(strcmp(TypeNode->valuestring,szOutTypeStr) != 0)
	{
		//TRACE("Output type mismatch. [TypeNode->valuestring=%s] [szOutTypeStr=%s]", TypeNode->valuestring, szOutTypeStr);
		return APP_FAIL;
	}else{/* do nothing */}

	ValueNode = cJSON_GetObjectItem(TargetNode, TOMSTAG_VALUE);
	ASSERT_FAIL(CheckJsonNode(ValueNode, cJSON_Array));

	SubValueNode = cJSON_GetArrayItem(ValueNode->child, 1);
	ASSERT_FAIL(CheckJsonNode(SubValueNode, nType));
	switch(emOutType)
	{
		case TOMSTYPE_BOOLEAN:
		case TOMSTYPE_STRING:
		case TOMSTYPE_HEX:
		case TOMSTYPE_FILE:
			strcpy(pOut, SubValueNode->valuestring);
			//TRACE("Param parsed. [Value=%s].", pOut);
			break;

		case TOMSTYPE_NUMBER:
			*((int*)pOut) = atoi(SubValueNode->valuestring);
			//TRACE("Param parsed. [Value=%d].", *((int*)pOut));
			break;

		case TOMSTYPE_REFERENCE:
			pOut = SubValueNode->child;
			//TRACE("Param parsed. Value is an pointer.");
			break;

		default:
			//TRACE("Output type out of range. [emOutType=%d]", emOutType);
			return APP_FAIL;
			break;
	}

	return APP_SUCC;
}

/**
* @brief	Parse all TOMS parameters and store them.
* @param [in] MainNode    JSON node contains parameter values
* @return
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @date 2020-07-10
*/
static int ParseTomsParam(cJSON *MainNode, const char *pszStoragePath, const char *szAppName)
{
	STAPPCOMMPARAM stAppCommParam;
	STAPPPOSPARAM stAppPosParam;
	char szLabelValue[10+1] = {0};
	char szInfo[128] = {0};
	char szCmpVer[10] = {0};
	cJSON *BodyNode = NULL, *ParamNode = NULL;
	char *pszFileRelativePath = NULL;
    char szFilePath[256] = {0};

	BodyNode = cJSON_GetObjectItem(MainNode, TOMSTAG_BODY);
	if(CheckJsonNode(BodyNode, cJSON_Object) != APP_SUCC)
	{
		//TRACE("Fail to parse body node.");
	}
	ParamNode = cJSON_GetObjectItem(BodyNode, TOMSTAG_PARAMVALUES);
	if(CheckJsonNode(ParamNode, cJSON_Object) != APP_SUCC)
	{
		//TRACE("Fail to parse param node.");
	}

	GetTomsParamValueFromJson(ParamNode, TOMSTAG_VERSION, szCmpVer, TOMSTYPE_STRING);

    strcpy(szFilePath, pszStoragePath);
	pszFileRelativePath = szFilePath + strlen(szFilePath);

	GetAppCommParam(&stAppCommParam);
	GetAppPosParam(&stAppPosParam);

	/* basic param */
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MERCHANTID, stAppPosParam.szMerchantId, TOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_TERMINALID, stAppPosParam.szTerminalId, TOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MERCHANTNAMEEN, stAppPosParam.szMerchantNameEn, TOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MERCHANTADDR1, stAppPosParam.szMerchantAddr[0], TOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MERCHANTADDR2, stAppPosParam.szMerchantAddr[1], TOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MERCHANTADDR3, stAppPosParam.szMerchantAddr[2], TOMSTYPE_STRING);

	/* transaction control */
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTSALE, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_SALE, YES) : SetTransSwitchOnoff(TRANS_SALE, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTVOIDSALE, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_VOID, YES) : SetTransSwitchOnoff(TRANS_VOID, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTREFUND, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_REFUND, YES) : SetTransSwitchOnoff(TRANS_REFUND, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTBALANCE, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_BALANCE, YES) : SetTransSwitchOnoff(TRANS_BALANCE, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTPREAUTH, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_PREAUTH, YES) : SetTransSwitchOnoff(TRANS_PREAUTH, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTVOIDPEAUTH, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_VOID_PREAUTH, YES) : SetTransSwitchOnoff(TRANS_VOID_PREAUTH, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTAUTHSALE, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_AUTHCOMP, YES) : SetTransSwitchOnoff(TRANS_AUTHCOMP, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTVOIDAUTHSALE, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_VOID_AUTHSALE, YES) : SetTransSwitchOnoff(TRANS_VOID_AUTHSALE, NO);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_SUPPORTADJUST, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		(strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? SetTransSwitchOnoff(TRANS_ADJUST, YES) : SetTransSwitchOnoff(TRANS_ADJUST, NO);
	}else{/* do nothing */}
	GetTransSwitchValue(stAppPosParam.sTransSwitch);

	/* system param */
	if(GetTomsParamValueFromJson(ParamNode, TOMSTAG_TRANS_TRACENO, szLabelValue, TOMSTYPE_STRING) == APP_SUCC)
	{
		SetVarTraceNo(szLabelValue);
	}else{/* do nothing */}
	if(GetTomsParamValueFromJson(ParamNode, TOMSTAG_TRANS_BATCHNO, szLabelValue, TOMSTYPE_STRING) == APP_SUCC)
	{
		SetVarBatchNo(szLabelValue);
	}else{/* do nothing */}
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_ISPINPAD, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
		stAppPosParam.cIsPinPad = (strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? YES : NO;
	}else{/* do nothing */}
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MAXTRANSCNT, stAppPosParam.szMaxTransCount, TOMSTYPE_NUMBER);

	/* communication param */
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_NII, stAppCommParam.szNii, TOMSTYPE_STRING);
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_TYPE, szLabelValue, TOMSTYPE_NUMBER) == APP_SUCC) {
		stAppCommParam.cCommType = GetCommTypeByFlag(szLabelValue[0]);
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_TPDU, szLabelValue, TOMSTYPE_NUMBER) == APP_SUCC) {
		PubAscToHex((uchar *)szLabelValue, 10, 0, (uchar *)stAppCommParam.sTpdu);
	}

	switch (stAppCommParam.cCommType)
	{
		case COMM_ETH:
			ASSERT_TOMSPARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_ETH, NULL), "No Support ETH");
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP1_E, stAppCommParam.szIp1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT1_E, stAppCommParam.szPort1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP2_E, stAppCommParam.szIp2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT2_E, stAppCommParam.szPort2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IPADDR_E, stAppCommParam.szIpAddr, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_MASK_E, stAppCommParam.szMask, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_GATE_E, stAppCommParam.szGate, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_TIMEOUT, &(stAppCommParam.cTimeOut), TOMSTYPE_NUMBER);
			if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_ISDHCP_E, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC)
			{
				stAppCommParam.cIsDHCP = (strcmp(szLabelValue, TOMS_BOOL_TURE) == 0) ? 1 : 0;
			}
			break;

		case COMM_DIAL:
			ASSERT_TOMSPARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_MODEM, NULL), "No Support DIAL");
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PREDIALNO, &(stAppCommParam.cReDialNum), TOMSTYPE_NUMBER);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PREPHONE, stAppCommParam.szPreDial, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_TELNO1, stAppCommParam.szTelNum1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_TELNO2, stAppCommParam.szTelNum2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_TELNO3, stAppCommParam.szTelNum3, TOMSTYPE_STRING);
			if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_ISPREDIAL, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
			    stAppCommParam.cPreDialFlag = (szLabelValue[0] == 'Y') ? 1 : 0;
		    }
			break;

		case COMM_GPRS:
			ASSERT_TOMSPARAM_FAIL((PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo) || memcmp(szInfo, "GPRS", 4)), "No Support GPRS");
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_WIRELESSDIAL_G, stAppCommParam.szWirelessDialNum, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP1_G, stAppCommParam.szIp1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT1_G, stAppCommParam.szPort1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP2_G, stAppCommParam.szIp2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT2_G, stAppCommParam.szPort2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_APN1, stAppCommParam.szAPN1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_USER_G, stAppCommParam.szUser, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PWD_G, stAppCommParam.szPassWd, TOMSTYPE_STRING);
			if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_MODE_G, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
				stAppCommParam.cMode = (strcmp(szLabelValue, TOMS_BOOL_FALSE) == 0) ? 1 : 0;  //N - alive
			}
			break;

		case COMM_CDMA:
			ASSERT_TOMSPARAM_FAIL((PubGetHardwareSuppot(HARDWARE_SUPPORT_WIRELESS, szInfo) || memcmp(szInfo, "CDMA", 4)), "No Support CDMA");
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP1_C, stAppCommParam.szIp1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT1_C, stAppCommParam.szPort1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP2_C, stAppCommParam.szIp2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT2_C, stAppCommParam.szPort2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_USER_C, stAppCommParam.szUser, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PWD_C, stAppCommParam.szPassWd, TOMSTYPE_STRING);
			if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_MODE_G, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
				stAppCommParam.cMode = (strcmp(szLabelValue, TOMS_BOOL_FALSE) == 0) ? 1 : 0;  //N - alive
			}
			break;

		case COMM_RS232:
			ASSERT_TOMSPARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_COMM_NUM, NULL), "No Support SERIAL");
			break;

		case COMM_WIFI:
			ASSERT_TOMSPARAM_FAIL(PubGetHardwareSuppot(HARDWARE_SUPPORT_WIFI, NULL), "No Support WIFI");
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP1_W, stAppCommParam.szIp1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT1_W, stAppCommParam.szPort1, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IP2_W, stAppCommParam.szIp2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_PORT2_W, stAppCommParam.szPort2, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_IPADDR_W, stAppCommParam.szIpAddr, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_MASK_W, stAppCommParam.szMask, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_GATE_W, stAppCommParam.szGate, TOMSTYPE_STRING);
			memset(szLabelValue, 0, sizeof(szLabelValue));
			if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_WIFIMODE, szLabelValue, TOMSTYPE_NUMBER) == APP_SUCC) {
				stAppCommParam.cWifiMode = GetWifiTypeByFlag(szLabelValue[0]);
			}
			memset(szLabelValue, 0, sizeof(szLabelValue));
			if (GetTomsParamValueFromJson(MainNode, TOMSTAG_COMM_ISDHCP_W, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
				stAppCommParam.cIsDHCP = (szLabelValue[0] == 'Y') ? 1 : 0;
			}

			switch(stAppCommParam.cWifiMode)
			{
			case 1:
				stAppCommParam.cWifiMode = WIFI_AUTH_OPEN;
				break;
			case 2:
				stAppCommParam.cWifiMode = WIFI_AUTH_WEP_PSK;
				break;
			case 3:
				stAppCommParam.cWifiMode = WIFI_AUTH_WPA_PSK;
				break;
			case 4:
				stAppCommParam.cWifiMode = WIFI_AUTH_WPA2_PSK;
				break;
			case 5:
				stAppCommParam.cWifiMode = WIFI_AUTH_WPA_WPA2_MIXED_PSK; // ?
				break;
			default:
				ASSERT_TOMSPARAM_FAIL(APP_FAIL, "Wifi mode error");
				return APP_FAIL;
			}
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_WIFISSID, stAppCommParam.szWifiSsid, TOMSTYPE_STRING);
			GetTomsParamValueFromJson(ParamNode, TOMSTAG_COMM_WIFIKEY, stAppCommParam.szWifiKey, TOMSTYPE_STRING);
			break;
		default:
			ASSERT_TOMSPARAM_FAIL(APP_FAIL, "Comm Type INVALID");
			return APP_FAIL;
	}

	//key passwd
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_MAINKEYNO, stAppPosParam.szMainKeyNo, TOMSTYPE_STRING);
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_ENCRYMODE, szLabelValue, TOMSTYPE_STRING) == APP_SUCC) {
		if (szLabelValue[0] == '1')
		{
			stAppPosParam.cEncyptMode = DESMODE_3DES;
		}
		else
		{
			stAppPosParam.cEncyptMode = DESMODE_DES;
		}
	}
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_ADMINPWD, stAppPosParam.szAdminPwd, TOMSTYPE_STRING);
	GetTomsParamValueFromJson(ParamNode, TOMSDEFAULT_FUNCTION, stAppPosParam.szFuncPwd, TOMSTYPE_STRING);

	// print settings
	GetTomsParamValueFromJson(ParamNode, TOMSTAG_PNTTITLE, stAppPosParam.szPntTitleEn, TOMSTYPE_STRING);
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_PNTPAGECNT, szLabelValue, TOMSTYPE_STRING) == APP_SUCC) {
		stAppPosParam.cPrintPageCount = szLabelValue[0];
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_FONTSIZE, szLabelValue, TOMSTYPE_STRING) == APP_SUCC) {
		if (szLabelValue[0] == 'S') {
			stAppPosParam.cFontSize = 16;
		} else if (szLabelValue[0] == 'M') {
			stAppPosParam.cFontSize = 24;
		} else if (szLabelValue[0] == 'B') {
			stAppPosParam.cFontSize = 32;
		}
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_PNTDETAIL, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
		stAppPosParam.cIsPntDetail = (szLabelValue[0] == 'Y') ? YES : NO;
	}
	memset(szLabelValue, 0, sizeof(szLabelValue));
	if (GetTomsParamValueFromJson(ParamNode, TOMSTAG_ISPREAUTHSHIELDPAN, szLabelValue, TOMSTYPE_BOOLEAN) == APP_SUCC) {
		stAppPosParam.cIsPreauthShieldPan = (szLabelValue[0] == 'Y') ? YES : NO;
	}

	if(GetTomsParamValueFromJson(ParamNode, TOMSTAG_TESTFILE, pszFileRelativePath, TOMSTYPE_BOOLEAN) == APP_SUCC)
	{
		PubDebug("pszFileRelativePath=%s", pszFileRelativePath);
		PubDebug("szFilePath=%s", szFilePath);
        char szContent[256] = {0};
		FILE* fd = fopen(szFilePath, "r");
		fread(szContent, sizeof(char), 256, fd);
		fclose(fd);
		PubMsgDlg("TEST FILE", szContent, 0, 60);
	}

   /// SaveCommParamToFile(&stAppCommParam);
	UpdateAppPosParam(FILE_APPPOSPARAM, stAppPosParam);
	UpdateAppCommParam(FILE_APPCOMMPARAM, stAppCommParam);
	CommInit();

	return APP_SUCC;
}

TOMS_ERRCODE TOMS_UpdateAppParam(const char *szFilePath, const char *pszStoragePath, const char* szAppName)
{
	int nRet = APP_FAIL;
	int nFd = 0;
	int nFileSize = 0;
	char IsUpdate = NO;
	char* pFileBuffer = NULL;
	cJSON *MainNode = NULL;

	PubClearAll();
	PubDisplayStrInline(0, 2, tr("Parsing Parameters..."));
	PubDisplayStrInline(0, 4, tr("Please wait..."));
	PubUpdateWindow();

	//TRACE("Start Update Param. szFilePath=%s", szFilePath);
	PubFsFileSize(szFilePath, (uint *)&nFileSize);
	if(nFileSize <= 0)
	{
		//TRACE("File size error. nFileSize=%d", nFileSize);
		return TOMS_FAIL;
	}

	nRet = TOMS_FAIL;
	pFileBuffer = (char*)malloc(sizeof(char) * nFileSize);
	if((nFd=PubFsOpen (szFilePath, "r")) < 0)
	{
		//TRACE("Fail to open file. [%s]", szFilePath);
		goto FAIL;
	}

	PubFsSeek(nFd, 0L, SEEK_SET);
	if((nFileSize = PubFsRead(nFd, pFileBuffer, nFileSize)) < 0)
	{
		//TRACE("Fail to read file.");
		PubFsClose(nFd);
		goto FAIL;
	}
	PubFsClose(nFd);

	MainNode = cJSON_Parse(pFileBuffer);
	if(CheckJsonNode(MainNode, cJSON_Object) != APP_SUCC)
	{
		//TRACE("Fail to parse file buffer.");
		goto FAIL;
	}

	if (ParseTomsParam(MainNode, pszStoragePath, szAppName) != APP_SUCC)
	{
		//TRACE("Fail to parse TOMS param.");
		goto FAIL;
	}
	nRet = TOMS_SUCC;
	//TRACE("TOMS param parse succ.");

	IsUpdate = YES;
FAIL:
	if (MainNode != NULL)
	{
		cJSON_Delete(MainNode);
	}
	if (pFileBuffer != NULL)
	{
		free(pFileBuffer);
	}
	if (IsUpdate == YES)
	{
		PubFsDel(szFilePath);
	}

	return nRet;
}

/**
 * TOMS_SUCC: needn't deal anything
 * TOMS_FAIL: need to download
 */
TOMS_ERRCODE TOMS_CheckVersion(EM_TOMS_VERSION_TYPE emType, const char *pszAppName, const char* pszParamVer)
{
    static APP_INFO stAppInfo;
	char szOSVer[60+1];
	int nLen = 60;
    char szBootLogoMd5[32+1] = {0};
    char szBootLogoPath[128] = {0};

    switch (emType)
    {
    case TOMS_VER_APP:
        memset(&stAppInfo, 0, sizeof(APP_INFO));
        if(NAPI_OK != NAPI_AppGetInfoByName(pszAppName, &stAppInfo))
        {
            return TOMS_FAIL;
        }
        PubDebug("[%s]system app version: [%s], compared [%s]", pszAppName, stAppInfo.VerBuf, pszParamVer);
        if(strcmp(pszParamVer, (char*)stAppInfo.VerBuf) <= 0)
        {
            return TOMS_SUCC;
        }
        break;
    case TOMS_VER_PATCH:
        memset(&stAppInfo, 0, sizeof(APP_INFO));
        if(NAPI_OK != NAPI_SysGetInfo(OS_VERSION, szOSVer, &nLen))
        {
            return TOMS_FAIL;
        }
        PubDebug("szOSVer [%s] pszParamVer [%s]\n", szOSVer, pszParamVer);

        if(strcmp(pszParamVer, szOSVer) <= 0)
        {
            return TOMS_SUCC;
        }
        break;
    case TOMS_VER_BOOT_LOGO:
        if (__get_resource_from_file(TOMS_RES_BOOT_LOGO, szBootLogoPath, szBootLogoMd5) != APP_SUCC)
        {
            return TOMS_FAIL;
        }
        if (strcmp(pszParamVer, szBootLogoMd5) == 0)
        {
            return TOMS_SUCC;
        }
        break;
    default:
        break;
    }

    return TOMS_FAIL;
}

static inline int __save_resource_to_file(EM_TOMS_RES_ID emRes, char*pszResPath, char *pszMD5)
{
    int nFd, nRet = APP_SUCC;
    char szContent[1024] = {0};

    if ((nFd = PubFsOpen(TOMS_RESOURCE_FILE, "w")) < 0)
    {
        return APP_FAIL;
    }

    switch (emRes)
    {
    case TOMS_RES_BOOT_LOGO:
        sprintf(szContent, "%d %s %s", emRes, pszResPath, pszMD5);
        PubFsSeek(nFd, 0, SEEK_SET);
        if (PubFsWrite(nFd, szContent, sizeof(szContent)) != sizeof(szContent)) {
            nRet = APP_FAIL;
        }
        break;
    default:
        nRet = APP_FAIL;
        break;
    }
    PubFsClose(nFd);

    return nRet;
}

static inline int __get_resource_from_file(EM_TOMS_RES_ID emRes, char*pszResPath, char *pszMD5)
{
    int nFd, nRet = APP_SUCC;
    char szContent[1024] = {0};

    if ((nFd = PubFsOpen(TOMS_RESOURCE_FILE, "r")) < 0)
    {
        return APP_FAIL;
    }

    switch (emRes)
    {
    case TOMS_RES_BOOT_LOGO:
        PubFsSeek(nFd, 0, SEEK_SET);
        if (PubFsRead(nFd, szContent, sizeof(szContent)) == sizeof(szContent))
        {
            sscanf(szContent, "%d %s %s", (int *)&emRes, pszResPath, pszMD5);
        }
        else
        {
            nRet = APP_FAIL;
        }
        break;
    default:
        nRet = APP_FAIL;
        break;
    }
    PubFsClose(nFd);

    return nRet;
}

TOMS_ERRCODE TOMS_UpdateResource(EM_TOMS_RES_ID emRes, char*pszResPath, char *pszMD5)
{
    int nRet = APP_SUCC;

    PubDebug("recieve resid [%d]  Path:[%s]  MD5 [%s]", emRes, pszResPath, pszMD5);
    switch (emRes)
    {
    case TOMS_RES_BOOT_LOGO:
        if ((nRet = NAPI_SysUpdateBootLogo(pszResPath)) != NAPI_OK && nRet != NAPI_ALREADY_DONE)
        {
            return TOMS_FAIL;
        }
        nRet = __save_resource_to_file(emRes, pszResPath, pszMD5);
        break;
    default:
        return TOMS_FAIL;
        break;
    }

    return nRet;
}

void ProDealLockTerminal()
{
    char szPasswd[20] = {0};
    int nOutLen;

    if (!GetLockTerminal())
    {
        return;
    }

    while(PubInputDlg(tr("Lock"), GetLockPromptInfo(), szPasswd, &nOutLen, 1, 20, 60, INPUT_MODE_PASSWD) != APP_SUCC
        || strcmp(szPasswd, "1234") != 0);
}

TOMS_ERRCODE TOMS_IsLockTerminal(EM_TOMS_FLOCK bIsLock, const char *pszTipsInfo)
{
    char szPromptInfo[32];

    PubDebug("lock flag: %d tips: [%s]\n", bIsLock, pszTipsInfo == NULL ? "NULL" : pszTipsInfo);
    if (bIsLock == TOMS_FLOCK_LOCK)
    {
        SetLockTerminal(1);
        if (pszTipsInfo && strlen(pszTipsInfo)) {
            strncpy(szPromptInfo, pszTipsInfo, sizeof(szPromptInfo));
        } else {
            strcpy(szPromptInfo, "Enter passwd");
        }
        SetLockPromptInfo(szPromptInfo);
    }
    else
    {
        SetLockTerminal(0);
        SetLockPromptInfo("");
    }

    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_GetIsLockTerminal(EM_TOMS_FLOCK *pemlock)
{
    if (GetLockTerminal())
    {
        *pemlock = TOMS_FLOCK_LOCK;
    }
    else
    {
        *pemlock = TOMS_FLOCK_UNLOCK;
    }

    return TOMS_SUCC;
}

static inline void __create_test_log(const char *pszPerFileNameFmt, const char *pszStartTime, const char *pszEndTime)
{
    char szFileName[1024] = {0};
    char szBuffer[4096] = {0};
    char szLogFileName[50] = {0}, szOldLogFileName[50] = {0};

    strcat(szFileName, "rm  /tmp/");
    sprintf(szFileName+ strlen(szFileName), pszPerFileNameFmt, "*", "*");
    system(szFileName);

    ProGetLogFileAttr(NULL, szLogFileName, szOldLogFileName);

    memset(szFileName, 0, sizeof(szFileName));
    strcpy(szFileName, "/tmp/new_");
    sprintf(szFileName+strlen(szFileName), pszPerFileNameFmt, pszStartTime, pszEndTime);
    sprintf(szBuffer, "cp %s %s", szLogFileName, szFileName);
    system(szBuffer);

    memset(szFileName, 0, sizeof(szFileName));
    strcpy(szFileName, "/tmp/old_");
    sprintf(szFileName+strlen(szFileName), pszPerFileNameFmt, pszStartTime, pszEndTime);
    sprintf(szBuffer, "cp %s %s", szOldLogFileName, szFileName);
    system(szBuffer);
}

TOMS_ERRCODE TOMS_ExtractLog(const char *pszPerFileNameFmt, const char *pszStartTime, const char *pszEndTime, char *pszOutDir)
{
    __create_test_log(pszPerFileNameFmt, pszStartTime, pszEndTime);
    strcpy(pszOutDir, "/tmp");

    return APP_SUCC;
}

static inline int __get_storage_available(char *pszOutInfo, int nMaxLen)
{
    ulong ulSpace;
    if (PubFsGetDiskSpace(1, &ulSpace) != NAPI_OK) {
        return APP_FAIL;
    }
    sprintf(pszOutInfo, "%lu", ulSpace);
    return APP_SUCC;
}

static inline int __get_imei_info(char *pszOutInfo, int nMaxLen)
{
    if (NAPI_WlmGetInfo(WLM_INFO_IMEI, pszOutInfo, nMaxLen) == NAPI_OK) {
        return APP_SUCC;
    }

    if (NAPI_WlmGetInfo(WLM_INFO_IMSI, pszOutInfo, nMaxLen) == NAPI_OK) {
        return APP_SUCC;
    }

    return APP_FAIL;
}

static inline int __get_wifi_status(char *pszOutInfo, int nMaxLen)
{
    EM_WIFI_STATE_T state;

    if (NAPI_WifiGetState(&state) != NAPI_OK) {
        return APP_FAIL;
    }

    if (state == WIFI_STATE_ENABLED || state == WIFI_STATE_ENABLING) {
        sprintf(pszOutInfo, "%d", 1);
    } else {
        sprintf(pszOutInfo, "%d", 0);
    }

    return APP_SUCC;
}

static inline int __get_wifi_ssid(char *pszOutInfo, int nMaxLen)
{
    ST_WIFI_INFO_T stInfo;

    if (NAPI_WifiGetConnectionInfo(&stInfo) != APP_SUCC) {
        return APP_FAIL;
    }
    snprintf(pszOutInfo, nMaxLen, "%s", stInfo.stApInfo.szSsid);

    return APP_SUCC;
}

static inline int __get_device_manufacturer(char *pszOutInfo, int nMaxLen)
{
    snprintf(pszOutInfo, nMaxLen, "%s", "C1C3"); // POS manufacturer identification(NEWLAND: C1C3)
    return APP_SUCC;

}

TOMS_ERRCODE TOMS_GetTerminalStatus(EM_TOMS_TERMSTATUS_INFO_ID emSS, char *pszOutInfo, int nMaxLen)
{
    int nRet = APP_SUCC;

    switch (emSS) {
    case TOMS_SS_STORAGE_AVAILABLE:
        nRet = __get_storage_available(pszOutInfo, nMaxLen);
        break;
    case TOMS_SS_IMEI:
        nRet = __get_imei_info(pszOutInfo, nMaxLen);
        break;
    case TOMS_SS_WIFI_STATUS:
        nRet = __get_wifi_status(pszOutInfo, nMaxLen);
        break;
    case TOMS_SS_SSID:
        nRet = __get_wifi_ssid(pszOutInfo, nMaxLen);
        break;
    case TOMS_SS_DEVICE_MANUFACTURER:
        nRet = __get_device_manufacturer(pszOutInfo, nMaxLen);
        break;
    default:
        nRet = APP_FAIL;
        break;
    }

    return nRet != APP_SUCC ? TOMS_FAIL : TOMS_SUCC;
}

TOMS_ERRCODE TOMS_SysGetDatetime(struct tm *pstDateTime)
{
    if (NAPI_SysGetDateTime(pstDateTime) != NAPI_OK)
    {
        return TOMS_FAIL;
    }

    return TOMS_SUCC;
}

void TOMS_SysReboot(void)
{
    NAPI_SysReboot();
}

TOMS_ERRCODE TOMS_SysAppInstall(const char *FilePath)
{
    if (strstr(FilePath, ".NLD") == NULL || NAPI_AppInstall(FilePath) != NAPI_OK)
    {
        return TOMS_FAIL;
    }

    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_SysAppUninstall(const char *pszAppName)
{
    int nRet;

    nRet = NAPI_AppDelete(pszAppName);
    PubDebug("unstall app : [%s] nRet = %d", pszAppName, nRet);
    if (nRet != NAPI_OK && nRet != NAPI_ERR_APP_NOT_EXIST)
    {
        return TOMS_FAIL;
    }

    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_SysSetSleepDuration(int nSleepDur)
{
    PubDebug("sleep duration is :[%d]", nSleepDur);
    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_GetAppNameByIndex(int nIndex, char *pszOutAppName, int nMaxLen)
{
    int nRet;
    APP_INFO stAppInfo;

    nRet = NAPI_AppGetInfoByIndex(nIndex, &stAppInfo);
    if (nRet != NAPI_OK) {
        return TOMS_FAIL;
    }
    snprintf(pszOutAppName, nMaxLen, "%s", stAppInfo.AppName);

    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_GetAppInfoByName(const char *pszAppName, EM_TOMS_APP_INFO_ID emID, char *pszOutInfo, int nMaxLen)
{
    int nRet;
    APP_INFO stAppInfo;

    switch (emID) {
    case TOMS_APP_VERSION_NAME:
        nRet = NAPI_AppGetInfoByName(pszAppName, &stAppInfo);
        if (nRet != NAPI_OK) {
            return TOMS_FAIL;
        }
        snprintf(pszOutInfo, nMaxLen, "%s", stAppInfo.VerBuf);
        break;
    default:
        return TOMS_FAIL;
        break;
    }

    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_GetNetworkType(EM_TOMS_NETWORK_TYPE *pemOutType)
{
    STAPPCOMMPARAM stComm;
    int nSq;

    memset(&stComm, 0, sizeof(stComm));
    GetAppCommParam(&stComm);

    switch (stComm.cCommType) {
    case COMM_WIFI:
        *pemOutType = TOMS_NETTYPE_WIFI;
        break;
    case COMM_GPRS:
    case COMM_CDMA:
        {
            if (NAPI_WlmGetSQ(&nSq) != NAPI_OK)
            {
                *pemOutType = TOMS_NETTYPE_UNKNOWN;
            }
            else
            {
                switch (nSq & 0xFF000000) {
                case WLM_NET_TYPE_GPRS:
                case WLM_NET_TYPE_CDMA:
                    *pemOutType = TOMS_NETTYPE_2G;
                    break;
                case WLM_NET_TYPE_3G:
                    *pemOutType = TOMS_NETTYPE_3G;
                    break;
                case WLM_NET_TYPE_4G:
                    *pemOutType = TOMS_NETTYPE_4G;
                    break;
                default:
                    *pemOutType = TOMS_NETTYPE_UNKNOWN;
                    break;
                }
            }
        }
        break;
    default:
        *pemOutType = TOMS_NETTYPE_UNKNOWN;
        break;
    }
    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_GetSimStatus(EM_TOMS_SIM_STATUS emType, int *pnOutResult)
{
    if (NAPI_WlmGetSQ(pnOutResult) != NAPI_OK) {
        return TOMS_FAIL;
    }

    if (emType == TOMS_SIM_SIGNAL_LEVEL) {
        *pnOutResult = *pnOutResult & 0xff;
    }

    return TOMS_SUCC;
}

TOMS_ERRCODE TOMS_GetPrinterStatus(EM_TOMS_PRINTER_STATUS *pemOutType)
{
    PRN_STATUS PrnStatus;

    if (NAPI_PrnGetStatus(&PrnStatus) != NAPI_OK) {
        return TOMS_FAIL;
    }

    if (PrnStatus == NAPI_PRN_STATUS_OK || PrnStatus == NAPI_PRN_STATUS_BUSY) {
        *pemOutType = TOMS_PRINTER_NORMAL;
    }
    else if (PrnStatus == NAPI_PRN_STATUS_NOPAPER) {
        *pemOutType = TOMS_PRINTER_OUT_OF_PAPER;
    }
    else {
        *pemOutType = TOMS_PRINTER_EXCEPTION;
    }

    return TOMS_SUCC;
}


TOMS_ERRCODE TOMS_SysGetInfo(EM_TOMS_SYSINFO_ID InfoID, char *pszOutBuf, int *pnOutBufLen)
{
    int nRet = APP_FAIL;

    switch (InfoID)
    {
    case TOMS_SYS_MODEL:
        nRet = NAPI_SysGetInfo(MODEL, pszOutBuf, pnOutBufLen);
        break;
    case TOMS_SYS_SN:
        nRet = NAPI_SysGetInfo(SN, pszOutBuf, pnOutBufLen);
        break;
    case TOMS_SYS_OS_VERSION:
        nRet = NAPI_SysGetInfo(OS_VERSION, pszOutBuf, pnOutBufLen);
        break;
    case TOMS_SYS_HW:
        nRet = NAPI_SysGetInfo(HW, pszOutBuf, pnOutBufLen);
        break;
    default:
        break;
    }

    return nRet != APP_SUCC ? TOMS_FAIL : TOMS_SUCC;
}

TOMS_ERRCODE TOMS_SysGetVolPercent(int *nVolPercent)
{
    if (NAPI_SysGetVolPercent(nVolPercent) != NAPI_OK)
    {
        return TOMS_FAIL;
    }

    return TOMS_SUCC;
}

#endif /* USE_TOMS */

