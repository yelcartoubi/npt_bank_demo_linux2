#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#ifdef USE_TMS

static char gcOldCommType = -1;
static char gszOldDomain[50+1] = {0};
static int gnIsFirstInvoke = 0;


static int InitCallbackFunc(void)
{
	TMSLIBOPER stTmsLibOper = 
	{
		Tms_CommConnect,
		Tms_CommClose,
		DispResultMsg,
		SetCommDomain,
		DispProgress,
		ShowPromptMsg,
		ShowSelectPrompt,
		LibTmsDebug
	};

	TMS_SetCallBack(&stTmsLibOper);
	return APP_SUCC;
}


static int SetTmsLibCommRelatedParam(char* pszTmsDomain, char cCommType)
{
	TMSLIBCOMMPARAM stTmsLibParam;

	//Setup Domain
	memset(&stTmsLibParam, 0, sizeof(TMSLIBCOMMPARAM));
	memcpy(stTmsLibParam.szDomain, pszTmsDomain, 50);
	strcpy(stTmsLibParam.szFirmCode, "NEWLAND");	
	stTmsLibParam.lnAppDLTimeOut = 210L;
	stTmsLibParam.lnOtherTimeOut = 30L;
	strcpy(stTmsLibParam.szSSLKeyType, "PEM");
	strcpy(stTmsLibParam.szSSLCerificateName, "server.pem");
	strcpy(stTmsLibParam.szCerificateID, "FD85E62D9BEB45428771EC688418B273");

	//Setup Communication Type
	switch (cCommType)
	{
	case COMM_ETH:
		strcpy(stTmsLibParam.szCommType, "E");
		break;
	case COMM_GPRS:
	// case COMM_CDMA:
		strcpy(stTmsLibParam.szCommType, "G");
		break;
	case COMM_WIFI:
	default:
		strcpy(stTmsLibParam.szCommType, "W");
		break;
	}
	TMS_SetCommParam(&stTmsLibParam);
	return 0;
}

static int ChkIsReachUpdateTime(void)
{
	char szNextChkVerTime[14+1] = {0};
	char szDateTime[14+1] = {0};

	ASSERT_FAIL(TMS_GetData(LTMS_DATATYPE_NEXTUPTTIME, szNextChkVerTime));
	PubGetCurrentDatetime(szDateTime);

	TRACE("Current Time = %s", szDateTime);
	TRACE("Next Update Time = %s", szNextChkVerTime);
	if(memcmp(szDateTime, szNextChkVerTime, 14) >= 0)
	{
		TRACE("Passive Update Start");
		TMS_CheckUpdate();
		SetStatusBar(STATUSBAR_OTHER_OPEN);
		return APP_SUCC;
	}
	return APP_FAIL;
}

#if 0
static int ChkIsSupportScanner(void)
{
	static char cIsNeedInitScan = YES;
	char szContent[100] = {0};
	int nRet;

	if (YES == cIsNeedInitScan)
	{
		if (APP_SUCC != PubGetHardwareSuppot(HARDWARE_SUPPORT_SCANNER, NULL))
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "|CNo Scan function\n|CPls use input");
			PubMsgDlg("Info", szContent, 0, 1);
			return APP_FAIL;
		}
		nRet = NDK_ScanInit();
		if(nRet != NAPI_OK)
		{
			PubGetStrFormat(DISPLAY_ALIGN_BIGFONT, szContent, "Init Scanner err%d\n|CPls use input",nRet);
			PubMsgDlg(NULL, szContent, 0, 1);
			return APP_FAIL;
		}
		cIsNeedInitScan = NO;
	}
	return APP_SUCC;
}


static int ScanBarCode(int nTimeOut , char * pszBarCode)
{
	int nRet;
	int nLen = 0, nScanRet = 0;
	int i;
	char szBarCode[128+1]={0};

	PubKbHit();

	for (i = 0 ; i < nTimeOut ; i++)
	{
		memset(szBarCode, 0, sizeof(szBarCode));
		nRet = NDK_ScanDoScan(1, szBarCode, &nLen);
		if(nRet == NAPI_OK)
		{
			break;
		}
		nScanRet = PubKbHit();
		if(KEY_ESC == nScanRet)
		{
			return APP_QUIT;
		}
	}
	if(i >= nTimeOut)
	{
		return APP_TIMEOUT;
	}

	PubDelSymbolFromStr((uchar*)szBarCode, 0x0D);
	PubDelSymbolFromStr((uchar*)szBarCode, 0x0A);
   	strcpy(pszBarCode, szBarCode);
	return APP_SUCC;
}
#endif


static int ActivateDevice(void)
{
	int nActiveCodeLen = 8, nLen = 0;
	char szActiveCode[8+1] = {0};

	ASSERT_FAIL(PubInputDlg("Activation", "Activation Code", szActiveCode,
							&nLen, nActiveCodeLen, nActiveCodeLen, 60, INPUT_MODE_STRING));

	ASSERT_FAIL(TMS_ActiveDevice(szActiveCode));
	return APP_SUCC;
}


static int InitTmsModule(void)
{
	int nRet = 0;

	nRet += InitCallbackFunc();
	nRet += TMS_Initialize();	
	if (nRet != APP_SUCC)
	{
		PubDebug("TMS_Module Err");
		return APP_FAIL;
	}
	return nRet;
}


int TmsCheckUpdate(EM_UTPMODE emUpdateMode, YESORNO cIsInputActiveCode)
{
	STAPPCOMMPARAM stAppCommParam;
	memset(&stAppCommParam, 0, sizeof(STAPPCOMMPARAM));

	GetAppCommParam(&stAppCommParam);
	if (stAppCommParam.cCommType == COMM_RS232 || stAppCommParam.cCommType == COMM_DIAL)
	{
		TRACE("stAppCommParam.cCommType = %d", stAppCommParam.cCommType);
		return APP_QUIT;
	}
	if(gnIsFirstInvoke == 0)
	{
		InitTmsModule();
		TMS_SetDebug(LTMS_DEBUG_OPEN);
		gnIsFirstInvoke = 1;
	}

	switch(emUpdateMode)
	{
	case UPTMODE_PASSIVE:
		ASSERT_FAIL(ChkIsReachUpdateTime());
		UpdateAppParamByTms();
		return APP_SUCC;
		break;
	case UPTMODE_EXIST:
		TMS_CheckAndInstallAll();
		UpdateAppParamByTms();
		return APP_SUCC;
		break;
	default:
		break;
	}

	if((gcOldCommType != stAppCommParam.cCommType) || (memcmp(gszOldDomain, stAppCommParam.szTMSDomain, 50) != 0))
	{
		memcpy(gszOldDomain, stAppCommParam.szTMSDomain, 50);
		gcOldCommType = stAppCommParam.cCommType;
		SetTmsLibCommRelatedParam(stAppCommParam.szTMSDomain, stAppCommParam.cCommType);
	}
	if(cIsInputActiveCode == YES)
	{
		ActivateDevice();
		return APP_SUCC;
	}
	TMS_CheckUpdate();
	SetStatusBar(STATUSBAR_OTHER_OPEN);	
	UpdateAppParamByTms();
	return APP_SUCC;
}

#endif

