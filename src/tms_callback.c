#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#ifdef USE_TMS

static char gszBackupDomain[50+1] = {0};
static int gnBackupPort;


/**
* @brief  Establish a link 
* @param in nIsUpdateWindow    1: Display prompt info 0: No prompt
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
int Tms_CommConnect(int nIsUpdateWindow)
{
	int nRet;
	STAPPCOMMPARAM stAppCommParam;
	STSHOWINFOXY stShowInfoxy;
		
	GetAppCommParam(&stAppCommParam);
	if(stAppCommParam.cCommType == COMM_DIAL || stAppCommParam.cCommType == COMM_RS232)
	{
		PubMsgDlg("Info", "Currnet comm type does not support HTTP", 3, 3);
		return APP_FAIL;
	}
	
	if(nIsUpdateWindow == 1)
	{
		PubClear2To4();
		PubDisplayStrInline(0, 2, "Connect to network...");
		PubDisplayStrInline(0, 4, "<Cancel> to quit");
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
		return APP_FAIL;
	}

	return APP_SUCC;
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
int Tms_CommClose(int nHangFlag)
{
	STAPPCOMMPARAM stAppCommParam;	
	memset(&stAppCommParam, 0, sizeof(stAppCommParam));
	
	GetAppCommParam(&stAppCommParam);
	CommSetSuspend(1);
	if (COMM_GPRS == stAppCommParam.cCommType || COMM_CDMA == stAppCommParam.cCommType)
	{
		if (stAppCommParam.cMode == 1 || stAppCommParam.cMode == '1' )
		{
			return APP_SUCC;
		}
		else/**<Short link, need to close PPP*/
		{
			return PubCommClose();
		}			
	}
	return APP_SUCC;
}

/**
* @brief  Display transaction result information 
* @param in pszTitle  
* @param in pszCode  		Prompt Code
* @param in pszMsg  		Prompt message
* @param in nTimeout  
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
int DispResultMsg(char* pszTitle, char* pszCode, char* pszMsg, int nTimeout)
{
	char szTmp[256] = {0};

	sprintf(szTmp, "\r\n%s\r\n%s", pszCode, pszMsg);
	PubMsgDlg(pszTitle, szTmp, 0, nTimeout);

	return APP_SUCC;
}


/**
* @brief  Set the host domain name for communication
* @param in pszDomain	  
* @param in nPort  
* @param in nFlag  		1: Use the set domain name and save the old domain name
*						0: Roll back the old host domain name
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
int SetCommDomain(char* pszDomain, int nPort, int nFlag)
{
	STCOMMPARAM stPubCommParam;

	memset(&stPubCommParam, 0, sizeof(STCOMMPARAM));
	PubGetCommParam(&stPubCommParam);
	if(pszDomain != NULL && nFlag == 1)
	{
		memset(gszBackupDomain, 0, sizeof(gszBackupDomain));
		memcpy(gszBackupDomain, stPubCommParam.stServerAddress.szDN, 50);
		memcpy(stPubCommParam.stServerAddress.szDN, pszDomain, 50);
		if(nPort >= 0)
		{
			gnBackupPort = stPubCommParam.stServerAddress.lnPort[0];
			stPubCommParam.stServerAddress.lnPort[0] = nPort;
		}
		
	}
	else
	{
		memcpy(stPubCommParam.stServerAddress.szDN, gszBackupDomain, 50);
		if(gnBackupPort >= 0)
		{
			stPubCommParam.stServerAddress.lnPort[0] = gnBackupPort;
		}
		memset(gszBackupDomain, 0, sizeof(gszBackupDomain));
		gnBackupPort = 0;
	}
	PubCommHangUp();
	PubSetCommParam(&stPubCommParam);
	return APP_SUCC;
}


/**
* @brief  Show download progress bar  
* @param in cFlag	  		1: First time  0: Not the first time 
* @param in nCur  			Current progress
* @param in nTotal  		Total progress
* @return
* @li 0 SUCC
* @li -1 FAIL
* @author kyle
* @date 2019-06-23
*/
void DispProgress(char cFlag, uint nCur, uint nTotal)
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
int ShowSelectPrompt(char* pszTitle, char* pszPromptMsg)
{
	char cSelect = '1', nRet = -1;

	ASSERT_QUIT(PubSelectYesOrNo((char*)tr(pszTitle), (char*)pszPromptMsg, NULL, &cSelect));
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

/**
* @brief  Output libtms debug log
* @param in log data 	  	
* @param in log len  
* @return
* @author kyle
* @date 2019-06-23
*/
void LibTmsDebug(const char* pszLog, int nLen)
{
	PubBufToAux(pszLog, nLen);
}

#endif


