/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  manage.c
** File indentifier: Manage precessing module
** Brief:  
** Current Verion:  v1.0
** Auther: sunh
** Complete date: 2016-9-19
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"
#include "napi_crypto.h"

static void ClearRevFlag(void);
static int DoClear(void);
static int KeyManage(void);
static int SetTmkByHand(void);
static int PosPrintSet(void);
static int MerchantManage(void);
static int ResendManage(void);


/**
* @brief Clear revesal flag
* @param in void
* @return void
 */
void ClearRevFlag(void)
{
	STREVERSAL stReversal;
	
	SetVarIsReversal(NO);
	SetVarHaveReversalNum(0);
	
	memset(&stReversal, 0, sizeof(STREVERSAL));
	SetReversalData(&stReversal);
	PubMsgDlg(tr("CLEAR REVERSAL"),tr("SUCC"), 0, 1);
	return;
}

/**
* @brief  Do Cleat
* @param in void
* @return     void
*/
int DoClear(void)
{
	int nRet;
	char *pszItems[] = {
		tr("1.CLEAR REVERSAL"),
		tr("2.CLEAR TRANSLOG"),
		tr("3.SET UNSIGNED"),
		tr("4.SET SIGNED"),
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("CLEAR"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
	
		switch(nSelcItem)
		{
		case 1:
			nRet = PubConfirmDlg(tr("CLEAR REVERSAL"),tr("CONFIRM?"), 0, 0);
			if( nRet == APP_SUCC )
			{
				ClearRevFlag(); 
			}
			break;
		case 2:
			nRet = PubConfirmDlg(tr("CLEAR LOG"),tr("CONFIRM?"), 0, 0);
			if( nRet == APP_SUCC )
			{
				PubMsgDlg(tr("CLEAR TRANS"),tr("PROCESSING..."), 0, 1);
				InitBatchFile();
				ClearSettle();
				EmvClearRecord();
				SetVarOfflineUnSendNum(0);
				PubMsgDlg(tr("CLEAR TRANS"),tr("CLEAR OK"), 1, 1);
			}
			break;
		case 3: 
			nRet = PubConfirmDlg(tr("SET UNSIGNED"),tr("CONFIRM?"), 0, 0);
			if( nRet == APP_SUCC )
			{
				SetVarIsLogin(NO);
			}
			break;
		case 4:
			nRet = PubConfirmDlg(tr("SET SIGNED"),tr("CONFIRM?"), 0, 0);
			if( nRet == APP_SUCC )
			{
				SetVarIsLogin(YES);
			}
			break;
		default :
			break;
		}
	}
	return APP_SUCC;
}


int ChkPinpad(void)
{
	int nRet = 0;
	int nPinpadAux = PINPAD;
	int nPinpadTimeout = 120;
	int nMode = SECRITY_MODE_INSIDE;
	STPINPADPARAM stPinpadParam;

	memset(&stPinpadParam, 0, sizeof(stPinpadParam));
	stPinpadParam.cAux = PINPAD;
	stPinpadParam.cTimeout = 60;
	if((GetVarIsPinpad() == NO) && (GetControlChkInside() == YES))
	{
		SetControlChkPinpad(YES);
		nMode = SECRITY_MODE_INSIDE;
		stPinpadParam.cTimeout = DEFAULT_PIN_INPUT_TIME;
		
		nRet = PubInitSecrity(nMode, &stPinpadParam);
		if (APP_SUCC != nRet)
		{
			PubMsgDlg(tr("DETECT DEV"),tr("LOAD SECRITY FAIL"), 3, 10);
			return APP_FAIL;
		}
		SetControlChkInside(NO);
	}
	else if ((GetVarIsPinpad() == YES) && (GetControlChkPinpad() == YES))
	{
		SetControlChkInside(YES);
		PubClearAll();
		PubDisplayStrInline(0, 2, tr("LOAD PINPAD"));
		PubDisplayStrInline(0, 4, tr("PLEASE WAIT..."));
		PubUpdateWindow();

		nMode = SECRITY_MODE_PINPAD;
		
		GetVarPinpadAuxNo(&nPinpadAux);
		stPinpadParam.cAux = nPinpadAux;
		GetVarPinpadTimeOut(&nPinpadTimeout);
		stPinpadParam.cTimeout = nPinpadTimeout;

		nRet = PubInitSecrity(nMode, &stPinpadParam);
		if (nRet != APP_SUCC)
		{
			PubMsgDlg(tr("DETECT DEV"),tr("PINPAD NOT CONNECTTED"), 3, 10);
			
			return APP_FAIL;
		}
		SetControlChkPinpad(NO);
	}
	PubSetKeySystemType(GetVarKeySystemType() == KS_MSK ? SECRITY_KEYSYSTEM_MSKEY : SECRITY_KEYSYSTEM_DUKPT);
	return APP_SUCC;
}

int ChkRF(void)
{
	return APP_SUCC;
}

/**
* @brief 				Get mainkey from Key pos
* @param in cFlag 		0x03++16Tmi+16TAK+8REFNO
* @return
* @li APP_SUCC
* @li APP_FAIL
*/
int GetTmkFromKeyPos()
{
	int nKeyIndex;
	int nRet;
	int nLen;
	char szBuffer[21];
	char szBcdKey[16+1];
	STCOMMPARAM stPubCommParam;
	char *pszTitle = tr("DOWNLOAD TMK");
	// int nKeyLen;
	char cIsPinPad=NO;
	STSHOWINFOXY stShowInfoxy;

	cIsPinPad = GetVarIsPinpad();
	if (YES == cIsPinPad && YES == GetControlChkPinpad())
	{
		if (APP_SUCC != ChkPinpad())
		{
			return APP_FAIL;
		}
		SetControlChkPinpad(NO);
	}
	memset(&stPubCommParam, 0, sizeof(stPubCommParam));
	stPubCommParam.cCommType= COMMTYPE_PORT;
	stPubCommParam.ConnInfo.stPortParam.nAux = RS232;
	stPubCommParam.ConnInfo.stPortParam.nBaudRate = BAUD9600;
	stPubCommParam.ConnInfo.stPortParam.nDataBits = DATA_8;
	stPubCommParam.ConnInfo.stPortParam.nParity = PAR_NONE;
	stPubCommParam.ConnInfo.stPortParam.nStopBits = STOP_1;
	stPubCommParam.cPreDialFlag = 0;
	stPubCommParam.nTimeOut = 3;
	stPubCommParam.cIsSupportQuitRecv = 1;

	stShowInfoxy.nType = 0;
	stShowInfoxy.nColumn = 0;
	stShowInfoxy.nRow = 0;
	PubSetShowXY(stShowInfoxy);

	nRet = PubCommInit(&stPubCommParam);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	SetControlCommInit();

	PubClearAll();
	PubDisplayTitle(pszTitle);
	PubDisplayStrInline(1, 2, tr("Connect To Key Pos"));
	PubDisplayStrInline(1, 4, tr("[Cancel] to quit"));

	PubUpdateWindow();

	PubCommConnect();
	while(1)
	{
		szBuffer[0] = EOT;
		if(PubCommWrite(szBuffer, 1)==APP_FAIL)
		{
			PubMsgDlg(pszTitle, tr("SEND FAIL"), 3, 3);
			PubCommHangUp();
			return APP_FAIL;
		}

		szBuffer[0]=0;
		nRet = PubCommRead(szBuffer, 1, &nLen);
		if(APP_QUIT == nRet)
		{
			PubCommHangUp();
			return APP_QUIT;
		}
		else if (APP_FAIL == nRet)
		{
			continue;
		}

		if(szBuffer[0]==ETX)
		{
			break;
		}
	}
	memset(szBuffer, 0, sizeof(szBuffer));
	nRet = PubCommRead(szBuffer, 2, &nLen);
	if (nRet==APP_FAIL || nLen != 2 )
	{
		PubMsgDlg(pszTitle, tr("Index Err"), 3, 3);
		PubCommHangUp();
		return APP_FAIL;
	}
	nKeyIndex = atoi(szBuffer);

	// nKeyLen = 16;

	memset(szBcdKey, 0, sizeof(szBcdKey));
	nRet = PubCommRead(szBcdKey, 16, &nLen);
	if (nRet==APP_FAIL || nLen != 16 )
	{
		PubMsgDlg(pszTitle, tr("LEN ERR"), 3, 3);
		PubCommHangUp();
		return APP_FAIL;
	}

	nRet = PubLoadMainKey(nKeyIndex, szBcdKey, NULL, nLen);
	if (nRet != APP_SUCC)
	{
		PubMsgDlg(pszTitle, tr("INSTALL KEY FAILS"), 3, 3);
		PubCommHangUp();
		return APP_FAIL;
	}

	SetVarMainKeyNo(nKeyIndex);

	sprintf(szBuffer,  tr("Download Tmk(%d) Ok"),nKeyIndex);
    PubMsgDlg(pszTitle, szBuffer, 1, 3);
	return APP_SUCC;
}

static int GetKcv(void)
{
	int nIndex, nRet;
	EM_SEC_CRYPTO_KEY_TYPE KeyType;
	EM_SEC_KEY_USAGE KeyUsage;
	uchar sKcv[50] = {0};
	int nKcvLen = 0;

	GetVarMainKeyNo(&nIndex);
	KeyType = KEY_TYPE_DES;
	if (GetVarKeySystemType() == KS_MSK)
	{

		KeyUsage = KEY_USE_KEK;
	}
	else
	{
		KeyUsage = KEY_USE_DUKPT;
	}

	nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KCV, (uchar)nIndex, KeyType, KeyUsage, NULL, 0, sKcv, &nKcvLen);
	if (nRet != NAPI_OK)
	{
		PubMsgDlg(tr("Promt"), tr("get SEC_KEY_TYPE_TMK kcv error"), 3, 3);
		return APP_FAIL;
	}
	DispTraceHex((char *)sKcv, nKcvLen, "TMK KCV: ");

	memset(sKcv, 0, sizeof(sKcv));
	nKcvLen = 0;
	KeyUsage = KEY_USE_PIN;
	nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KCV, (uchar)nIndex, KeyType, KeyUsage, NULL, 0, sKcv, &nKcvLen);
	if (nRet != NAPI_OK)
	{
		PubMsgDlg(tr("Promt"), tr("get SEC_KEY_TYPE_TPK kcv error"), 3, 3);
		return APP_FAIL;
	}
	DispTraceHex((char *)sKcv, nKcvLen, "TPK KCV: ");

	memset(sKcv, 0, sizeof(sKcv));
	nKcvLen = 0;
	KeyUsage = KEY_USE_MAC;
	nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KCV, (uchar)nIndex, KeyType, KeyUsage, NULL, 0, sKcv, &nKcvLen);
	if (nRet != NAPI_OK)
	{
		PubMsgDlg(tr("Promt"), tr("get SEC_KEY_TYPE_TAK kcv error"), 3, 3);
		return APP_FAIL;
	}
	DispTraceHex((char *)sKcv, nKcvLen, "TAK KCV: ");

	memset(sKcv, 0, sizeof(sKcv));
	nKcvLen = 0;
	KeyUsage = KEY_USE_DATA;
	nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KCV, (uchar)nIndex, KeyType, KeyUsage, NULL, 0, sKcv, &nKcvLen);
	if (nRet != NAPI_OK)
	{
		PubMsgDlg(tr("Promt"), tr("get SEC_KEY_TYPE_TDK kcv error"), 3, 3);
		return APP_FAIL;
	}
	DispTraceHex((char *)sKcv, nKcvLen, "TDK KCV: ");

	return APP_SUCC;
}


/**
* @brief Key manage
* @param in void
* @return     void
*/
int KeyManage(void)
{
	int	nRet;
	char *pszItems[] = {
		tr("1.KEY SYSTEM TYPE"),
		tr("2.INPUT TMK"),
		tr("3.KEYNO"),
		tr("4.CLEAR"),
		tr("5.CHECK KCV")
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("DOWNLOAD KEY"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		
		switch(nSelcItem)
		{
		case 1:
			SetFunctionKeySystemType();
			break;
 		case 2:
			SetTmkByHand();
			break;	
		case 3:
			SetFunctionMainKeyNo();
			break;	
		case 4:
			nRet = PubConfirmDlg(tr("KEY MANAGE"),tr("Clear all keys?"), 0, 0);
			if (nRet == APP_SUCC)
			{
				if (APP_SUCC == PubClearKey())
				{
					PubMsgDlg(tr("KEY MANAGE"),tr("Clear Ok"), 1, 1);
				}
			}
			break;
		case 5:
			GetKcv();
			break;
		default:
			break;
		}
	}

	return APP_SUCC;
}

/**
* @brief Set Tmk by hand
*/
int SetTmkByHand(void)
{
	int nKeyIndex;
	int nRet;
	int nLen = 0;
	char szKeyIndex[2+1];
	char szTmpStr[21] = {0};
	char szAscKey[32+1], sBcdKey[16];
	char szAscIPEK[32+1], sBcdIPEK[16];
	char szAscKSN[20+1], sBcdKSN[10]; 
	char *pszTitle = tr("INPUT BY MANUAL");
	YESORNO cIsPinPad = NO;

	/********************TMK**********************/
	cIsPinPad = GetVarIsPinpad();
	if (YES == cIsPinPad && YES == GetControlChkPinpad())
	{
		if (APP_SUCC != ChkPinpad())
		{
			return APP_FAIL;
		}
		SetControlChkPinpad(NO);
	}

	while (1)
	{
		memset(szKeyIndex, 0, sizeof(szKeyIndex));
		ASSERT_QUIT(PubInputDlg(tr("INPUT KEY NO.:"),"(1 ~ 9)", szKeyIndex, &nLen, 1, 1, 0, INPUT_MODE_NUMBER));
		nKeyIndex = atoi(szKeyIndex);
		if ((nKeyIndex >= 1) && (nKeyIndex <= 9))
		{
			break;
		}
	}

	if (GetVarKeySystemType() == KS_MSK)
	{
		memset(szAscKey, 0, sizeof(szAscKey));	
		ASSERT_QUIT(PubInputDlg(pszTitle, tr("INPUT TMK(32):"), szAscKey, &nLen, 32, 32, 0, INPUT_MODE_STRING));	
		PubAscToHex((uchar *)szAscKey, nLen, 0, (uchar *)sBcdKey);
		nLen >>= 1;

		nRet = PubLoadMainKey(nKeyIndex, sBcdKey, NULL, nLen);
		if (nRet != APP_SUCC)
		{
			PubDispErr(tr("LOAD MAINKEY FAIL"));
			return APP_FAIL;
		}
	}
	else
	{
		memset(szAscIPEK, 0, sizeof(szAscIPEK));	
		ASSERT_QUIT(PubInputDlg(pszTitle, tr("INPUT IPEK(32):"), szAscIPEK, &nLen, 32, 32, 0, INPUT_MODE_STRING));	
		PubAscToHex((uchar *)szAscIPEK, nLen, 0, (uchar *)sBcdIPEK);

		memset(szAscKSN, 0, sizeof(szAscKSN));	
		ASSERT_QUIT(PubInputDlg(pszTitle, tr("INPUT KSN(20):"), szAscKSN, &nLen, 20, 20, 0, INPUT_MODE_STRING));	
		PubAscToHex((uchar *)szAscKSN, nLen, 0, (uchar *)sBcdKSN);


		nRet = PubLoadMainKey(nKeyIndex, sBcdIPEK, sBcdKSN, 16);
		if (nRet != APP_SUCC)
		{
			PubDispErr("LOAD DUKPT FAIL");
			return APP_FAIL;
		}
	}
		
	sprintf(szTmpStr, tr("LOAD(%d) SUC"), nKeyIndex);	
	PubMsgDlg(pszTitle, tr("SET TMK OK"), 1, 1);
	
	return APP_SUCC;
}

static int PosPrintSet()
{
	char *pszItems[] = {
		tr("1.RECEIPT HEAD"),
		tr("2.PAGE"),
		tr("3.OTHER")
	}; 
	int nSelcItem = 1, nStartItem = 1;
	
	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("SET PRINT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		switch(nSelcItem)
		{
		case 1:
			SetFunctionPrintTitleMode();
			break;
		case 2:
			SetFunctionPrintPageCount();
			break;
		case 3:
			{
				int (*lSetFuns[])(void) = {
					SetFunctionIsPntDetail,
				   	SetFunctionIsPrintPrintMinus,
				   NULL};
				PubUpDownMenus(lSetFuns);	
			}
			break;
		default:                         
			break;
		}
	}
	return APP_SUCC;
}

static int MerchantManage(void)
{
	char *pszItems[] = {
		tr("1.MERCHANT ID"),
		tr("2.TERMINAL ID"),
		tr("3.MERCHANT NAME"),
		tr("4.MERCHANT ADDR")
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("SET TERMINAL"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		switch(nSelcItem)
		{
		case 1:
			SetFunctionMerchantId();
			break;
		case 2:
			SetFunctionTerminalId();
			break;
		case 3:
			SetFunctionMerchantName();
			break;
		case 4:
			{
				int (*lSetFuns[])(void) = {
					SetFunctionMerchantAddr,
					NULL};
				PubUpDownMenus(lSetFuns);
			}
			break;
		default:                         
			break;
		}
	}
	return APP_SUCC;
}

static int ResendManage(void)
{
	char *pszItems[] = {
		tr("1.OFFLINE RESEND"),
		tr("2.REVERSAL RESEND")
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("SET RESEND"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));

		switch(nSelcItem)
		{
		case 1:
			SetFunctionOffResendNum(); 
			break;
		case 2:
			SetFuncCommReSendNum();
			break;
		default:                         
			break;
		}
	}

	return APP_SUCC;
}

int DoPrint()
{
	char *pszItems[] = {
		tr("1.PRINT LAST"),
		tr("2.PRINT ANYONE"),
		tr("3.PRINT OLD SETTLE"),
		tr("4.PRINT SUMMRY"),
		tr("5.PRINT DETAIL"),
		tr("6.PRINT PARAM")
	}; 
	int nSelcItem = 1, nStartItem = 1;


	ASSERT_QUIT(PubShowMenuItems(tr("DO PRINT"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
	switch (nSelcItem)
	{
	case 1:
		PrnLastTrans();
		break;
	case 2:
		RePrnSpecTrans();
		break;
	case 3:
		PrintSettle(REPRINT);
		break;
	case 4:
		PrnTotal();
		break;
	case 5:
		PrintAllRecord();
		break;
	case 6:
		PrintParam();
		break;
	default:
		break;
		
	}
	return APP_SUCC;
}

int DoLookUp()
{
	char *pszItems[] = {
		tr("1.VIEW DETAIL"),
		tr("2.VIEW ANY"),
		tr("3.VIEW TOTAL")
	}; 
	int nSelcItem = 1, nStartItem = 1;


	ASSERT_QUIT(PubShowMenuItems(tr("LOOK UP"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));

	switch (nSelcItem)
	{
	case 1:
		ViewTranList();
		break;
	case 2:
		ViewSpecList();
		break;
	case 3:
		ViewTotal();
		break;
	default:
		break;
	}
	return APP_SUCC;
}

static int TransParam(void)
{
	char *pszItems[] = {
		tr("1.TRACE NO"),
		tr("2.BATCH NO"),
		tr("3.INVOICE NO"),
		tr("4.TRANS COUNT"),
		tr("5.SET PRINT"),
		tr("6.SET VOID"),
		tr("7.RESEND"),
		tr("8.DEFAULT TRANS"),
		tr("9.SET QPS LIMIT"),
		tr("10.SET TIP RATE")
	}; 
	int nSelcItem = 1, nStartItem = 1;

	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("TRANS PARAM"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0))
		switch(nSelcItem)
		{
		case 1:
			SetFunctionTraceNo();
			break;
		case 2:
			SetFunctionBatchNo();
			break;
		case 3:
			SetFunctionInvoiceNo();
			break;
		case 4:
			SetFunctionMaxTransCount();
			break;
		case 5:
			PosPrintSet();
			break;
		case 6:
			SetFunctionIsVoidPin();
			SetFunctionIsVoidStrip();
			break;
		case 7:
			ResendManage();
			break;
		case 8:
			SetFunctionDefaultTransType();
			break;
		case 9:
			SetFunctionQPSLimit();
			break;
		case 10:
			SetFunctionTipRate();
			break;
		default:                         
			break;
		}
	}
	return APP_SUCC;
}

int SetPaymentType()
{
	char *pszItems[] = {
		tr("1.SWIPE ENABLE"),
		tr("2.MANUAL ENABLE"),
		tr("3.CONTACT ENABLE"),
		tr("4.WAVE ENABLE")
	}; 
	int nSelcItem = 1, nStartItem = 1;

	while (1)
	{	
		ASSERT_QUIT(PubShowMenuItems(tr("PAYMENT TYPE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0))
		switch(nSelcItem)
		{
		case 1:
			SetFunctionIsSwipe();
			break;
		case 2:
			SetFunctionIsManul();
			break;
		case 3:
			SetFunctionIsContact();
			break;
		case 4:
			SetFunctionIsSupportRF();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

/**
* @brief Set parameters of transaction
* @param in void
* @return     void
*/
int ControlOnOf(void)
{
	char *pszItems[] = {
		tr("1.TRANS ENABLE"),
		tr("2.PAYMENT TYPE"),
		tr("3.TIP ENABLE"),
		tr("4.CVV2 ENABLE"),
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	ASSERT_QUIT(ProCheckPwd(tr("ON OFF"), EM_SYSTEM_PWD));
	
	while (1)
	{

		ASSERT_QUIT(PubShowMenuItems(tr("ON OFF"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0))
		switch(nSelcItem)
		{
		case 1:
			SetFunctionTransSwitch();
			break;
		case 2:
			SetPaymentType();
			break;
		case 3:
			SetFunctionIsTipFlag();
			break;
		case 4:
			SetFunctionIsNeedCVV2();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

void setLanguage(void)
{
	 int  nRet;
	 int nSelcItem = 1, nStartItem = 1;
	 char *pszStrs[] = {
	 	tr("1.English"),
		tr("2.Chinese"),
		tr("3.Customized")
	};

	nSelcItem = GetVarSystemLanguage() + 1;
	while(1)
	{
		nRet = PubShowMenuItems(tr("Language"), pszStrs, sizeof(pszStrs)/sizeof(char *), &nSelcItem, &nStartItem, 0);
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
		{
			return;
		}

		switch(nSelcItem)
		{
		case 1:
			SetVarSystemLanguage(LANG_EN_US);
			break;
		case 2:
			SetVarSystemLanguage(LANG_ZH_CN);
			break;
		case 3:
			SetVarSystemLanguage(LANG_CUSTOMIZED);
			break;
		default:
			return;
		}
	}

}

void setFontSize(void)
{	
	int  nRet;	
	int nSelcItem = 1, nStartItem = 1;	
	char *pszStrs[] = {
		"1.16",
		"2.24",
		"3.32"
	};	

	nSelcItem = GetVarSystemFontSize() / 8 - 1;
	while(1)	
	{		
		nRet = PubShowMenuItems(tr("Set Font"), pszStrs, sizeof(pszStrs)/sizeof(char *), &nSelcItem, &nStartItem, 0);		
		if (nRet==APP_QUIT || nRet==APP_TIMEOUT)		
		{			
			return;		
		}		
		switch(nSelcItem)		
		{			
		case 1: 
		case 2: 
		case 3:	
			SetVarSystemFontSize((nSelcItem + 1) * 8);
			PubSetDispForm((nSelcItem + 1) * 8, GetVarSystemFontSize() / 4);
			break;			
		}	
	}
}

static int InitAppDefaultParam()
{
	int nRet;

	nRet = PubConfirmDlg(tr("INITIALIZE"),tr("All datas will be init"), 0, 0);
	if (nRet==APP_QUIT || nRet==APP_TIMEOUT)
	{
		return nRet;
	}
	PubClearAll();
	ASSERT_FAIL(PubFsDel(FILE_APPCOMMPARAM));
	ASSERT_FAIL(PubFsDel(FILE_APPSETTLEMENT));
	ASSERT_FAIL(PubFsDel(FILE_APPREVERSAL));
	ASSERT_FAIL(PubFsDel(FILE_APPTRANSPARAM));
	ASSERT_FAIL(PubFsDel(FILE_APPPOSPARAM));
	PubDeReclFile(LASTSETTLEFILE);
	PubDisplayGen(2, tr("INITIALIZE"));
	PubDisplayGen(3, tr("PLEASE WAIT...."));
	PubUpdateWindow();
	InitCommParam();
	InitBatchFile();
	EmvClearRecord();
	ASSERT_FAIL(InitPosDefaultParam());
	InitExPosParam();
	PubMsgDlg(tr("INITIALIZE"),tr("init ok"), 1, 1);

	return APP_SUCC;
}

int AdminMenu()
{
	char *pszItems[] = {
		tr("1.CHANGE PASSWORD"),
		tr("2.SET TIME"),
		tr("3.INITIALIZE"),
		tr("4.PINPAD"),
		tr("5.LANGUAGE"),
		tr("6.FONT SIZE"),
		tr("7.EXTERN RF")
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	while (1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("SYSTEM"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0))
	
		switch(nSelcItem)
		{
		case 1:
			ChangePassword();
			break;
		case 2:
			SetFunctionPosDateTime();
			break;
		case 3:
			InitAppDefaultParam();
			CommInit();
			break;
		case 4:
			SetFunctionIsPinPad();
			break;
		case 5:
			setLanguage();
			break;
		case 6:
			setFontSize();
			break;
		case 7:
			SetFunctionIsExRF();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}

#ifdef USE_TMS

static void DisplayCertNum(void)
{
	char szOID[48] = {0};
	char sBuff[128] = {0};

	PubClear2To4();

	TMS_GetData(LTMS_DATATYPE_CERTNUM, szOID);
	sprintf(sBuff, "Certificate Number: \r\n\r\n%s", szOID);
	PubDisplayStrs(DISPLAY_ALIGN_BIGFONT, 0, 2, 1, "%s", sBuff);
	PubGetKeyCode(0);
	return;
}

static int TMS_Menu(void)
{
	int nSelcItem = 1, nStartItem = 1;
	char *pszItems[] = {
		tr("1.ACTIVATE"),
		tr("2.UPDATE"),
		tr("3.CERTIFICATE NUM"),
		tr("4.AUTO UPDATE"),
	}; 

	ASSERT_QUIT(ProCheckPwd(tr("TMS"), EM_SYSTEM_PWD));
	while(1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("TMS"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0));
		switch(nSelcItem)
		{
		case 1:
			TmsCheckUpdate(UPTMODE_INITIATIVE, YES);
			break;
		case 2:
			TmsCheckUpdate(UPTMODE_INITIATIVE, NO);
			break;
		case 3:
			DisplayCertNum();
			break;
		case 4:
			SetFunctionTmsAutoUpdate();
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}
#endif

int OtherMenu()
{
	char *pszItems[] = {
		tr("1.SHOW TVR"),
		tr("2.PRINT ISO"),
		tr("3.TMS"),
	}; 
	int nSelcItem = 1, nStartItem = 1;

	
	while (1)
	{
		ASSERT_QUIT(PubShowMenuItems(tr("OTHER"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0))
	
		switch(nSelcItem)
		{
		case 1:
			ViewTVR_TSI();
			break;
		case 2:
			SetFunctionIsPrintIso();
			break;
		case 3:
#ifdef USE_TMS
			TMS_Menu();
#else
			PubMsgDlg(NULL, "NONSUPPORT", 1, 5);
#endif
			break;
		default:
			break;
		}
	}
	return APP_SUCC;
}


int SystemManage()
{
	int nRet = 0;
	char *pszItems[] = {
		tr("1.MERCHANT"),
		tr("2.COMMUNICATION"),
		tr("3.ON-OF"),
		tr("4.TRANS PARAM"),
		tr("5.KEY MANAGE"),
		tr("6.CLEAR"),
		tr("7.SYSTEM"),
		tr("8.PRINT"),
		tr("9.OTHER")
		};
	int nSelcItem = 1, nStartItem = 1;
	
	ASSERT_QUIT(ProCheckPwd(tr("MANAGE"), EM_FUNC_PWD));
	
 	while(1)
	{	
		nRet = PubShowMenuItems(tr("MANAGE"), pszItems, sizeof(pszItems)/sizeof(char *), &nSelcItem, &nStartItem, 0);
		if (nRet==APP_QUIT)
		{
			if (APP_SUCC == PubConfirmDlg(tr("MANAGE"),tr("QUIT?"), 0, 30))
				return APP_QUIT;
			continue;
		}
		if (nRet == APP_TIMEOUT)
		{
			continue;
		}
		switch(nSelcItem)
		{		
		case 1:
			MerchantManage();
			break;
		case 2:
			SetComm();
			break;
		case 3:
			ControlOnOf();
			break;
		case 4:
			TransParam();
			break;
		case 5:
			KeyManage();
			break;
		case 6:
			DoClear();
			break;
		case 7:
			AdminMenu();
			break;
		case 8:
			DoPrint();
			break;
		case 9:
			OtherMenu();
			break;
		default:
			break;
		}
 	}
	return APP_SUCC;
}
