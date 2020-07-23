#include "pinpad.h"
#include "lsecurity.h"
#include "ltool.h"
#include "lui.h"
#include "pinpadtool.h"
#include "unistd.h"
#include "emvconfig.h"
#include "softalg.h"
#include "napi.h"
#include "napi_app.h"
#include "napi_crypto.h"
#include "emvl3.h"
#include "napi_sysinfo.h"
#include "napi_display.h"
#include "napi_smartcard.h"

#define SECURITYLIBVER			"ALSECU0117052301"
#define PINMAXLEN 				12

#define GREENFONT				0x3384
#define REDFONT					0xFACB
#define BLACKFONT				0x0000
#define WHITEFONT				0xFFFF
#define MONEYBACKGROUNDPNG		"money_background.png"	/*<background of money display:269*64 */
#define MONEYBACKGROUNDW		269						/*<height:269 */
#define MONEYBACKGROUNDH		50						/*<width:64-->50 */
#define BORDERW					8						/*<margin:8 */

#define DUKPT_KSN_LEN (10)
#define KEY_MAX_LEN   (48)
#define MAX_TIMEOUT   (200)
#define MIN_TIMEOUT   (5)

static int gnSecurityMode = SECRITY_MODE_INSIDE;   		/**< built-in or external pinpad*/
static int gnMainKeyIndex = 0;  						/**< current key index*/

static int gnTimeOut = MAX_TIMEOUT;						/**< timeout of built-in pinpad (5-200s)*/
static int gnYesOrNoInit = 0;							/**< panpad init flag 0: uninit, 1:init*/
static char gcKeySystem = SECRITY_KEYSYSTEM_MSKEY;		/**< default key system type*/

typedef struct {
	int (*pLoadKey)(int , int , const char *, int , char*);
	int (*pGetPinBlock)(char *, int *, int , int , const char *, int , int);
	int (*pCalMac)(char*, int, int, const char*, int);
	int (*pClrPinpad)(void);
	int (*pDispPinpad)(const char *, const char *, const char *, const char *);
	int (*pDespinpad)(const char *, int , char *, int , int );
	int (*pReadString)(char*, int*, int, int);
	int (*pClrKey)(void);
	int (*pPinpadBeep)(int, int);
	int (*pPinpadQR)(int, char*);
	int (*pPinpadDoScan)(char *);
	int (*pEsignature)(char *, char *, int );
}STPINPAD;

static STPINPAD gstPinpad;					/**< structure for callback funtions*/


static int ProInitPinpadParam(const STPINPADPARAM stPinpadParam)
{
	if(PubResetPinpad_SP100(stPinpadParam.cAux, stPinpadParam.cTimeout) != APP_SUCC)
	{
		return APP_FAIL;
	}
	memset(&gstPinpad, 0, sizeof(STPINPAD));
	gstPinpad.pLoadKey = PubLoadKey_SP100;
	gstPinpad.pGetPinBlock = PubGetPinBlock_SP100;
	gstPinpad.pCalMac = PubCalcMac_SP100;
	gstPinpad.pClrPinpad = PubClrPinPad_SP100;
	gstPinpad.pDispPinpad = PubDispPinPad_SP100;
	gstPinpad.pDespinpad = PubDesPinpad_SP100;
	gstPinpad.pClrKey = PubClearKey_SP100;
	gstPinpad.pPinpadBeep = PubPinpadBeep_SP100;
	gstPinpad.pPinpadQR = PubGenAndShowQr_SP100;
	gstPinpad.pPinpadDoScan = PubDoScan_SP100;
	gstPinpad.pEsignature = PubEsignature_SP100;
	return APP_SUCC;
}

/**
* @brief Init security module
* @details Will check if the module is existed
* @param [in] nMode           ---Refer to EM_SECRITY_MODE
* @param [in] pstPinpadParam  ---Pinpad parameters
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubInitSecrity(int nMode,const STPINPADPARAM* pstPinpadParam)
{
	int nRet;
	STPINPADPARAM stPinPadParam;

	gnYesOrNoInit = 0;
	memset(&stPinPadParam,0,sizeof(STPINPADPARAM));
	if (pstPinpadParam != NULL)
	{
		memcpy(&stPinPadParam,pstPinpadParam,sizeof(STPINPADPARAM));
	}

	switch (nMode)
	{
	case SECRITY_MODE_INSIDE:
		if(pstPinpadParam != NULL)
		{
			gnTimeOut = stPinPadParam.cTimeout;
		}

		if (gnTimeOut < MIN_TIMEOUT)
		{
			gnTimeOut = MIN_TIMEOUT;
		}
		else if (gnTimeOut > MAX_TIMEOUT)
		{
			gnTimeOut = MAX_TIMEOUT;
		}
		gnSecurityMode = SECRITY_MODE_INSIDE;
		gnYesOrNoInit = 1;
		break;
	case SECRITY_MODE_PINPAD:
		if (pstPinpadParam == NULL)
		{
			ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
			PINPAD_TRACE_SECU("PubInitSecrity nMode=SECRITY_MODE_PINPAD,but pstPinpadParam is NULL");
			return APP_FAIL;
		}
		nRet = ProInitPinpadParam(stPinPadParam);
		if (nRet != APP_SUCC)
		{
			ProSetSecurityErrCode(ERR_PINPAD_PININITERROR,0);
			PINPAD_TRACE_SECU("ProInitPinpadParam error");
			return APP_FAIL;
		}
		gnSecurityMode = SECRITY_MODE_PINPAD;
		gnYesOrNoInit = 1;
		PubClrPinPad();
		break;
	default:
		PINPAD_TRACE_SECU("ProInitPinpadParam nMode error");
		ProSetSecurityErrCode(ERR_PINPAD_NOSUPPORTMODE,0);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubInitSecrity succ");

	return APP_SUCC;
}

static int ProCheckInit(void)
{
	if(gnYesOrNoInit == 0)
	{
		PubMsgDlg("Not yet Init","Please Init Pinpad", 2, 5);
		return APP_FAIL;
	}
	return APP_SUCC;
}

/**
* @brief Clear KEY
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClearKey(void)
{
	int nRet = -1;

	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		if ((nRet=NAPI_SecKeyErase()) != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_KEYERASE,nRet);
            PINPAD_TRACE_SECU("NAPI_SecKeyErase error nRet=%d",nRet);
			return APP_FAIL;
		}
		PINPAD_TRACE_SECU("PubClearKey SUCC");
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pClrKey();
	}
}

/**
* @brief Set the index of current main key.
* @param [in] nIndex Index of key
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSetCurrentMainKeyIndex(int nIndex)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(nIndex < 1 || nIndex > 250)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("nIndex error");
		return APP_FAIL;
	}
	gnMainKeyIndex = nIndex;
	PINPAD_TRACE_SECU("PubSetCurrentMainKeyIndex(%d) SUCC",nIndex);
	return APP_SUCC;
}

/**
* @brief Set the key system type.
* @param [in] nType Type of key system--EM_KEYSYSTEM
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubSetKeySystemType(int nType)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	
	if(nType != SECRITY_KEYSYSTEM_MSKEY && nType != SECRITY_KEYSYSTEM_DUKPT)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("nType error");
		return APP_FAIL;
	}
	gcKeySystem = nType;
	PINPAD_TRACE_SECU("PubSetCurrentKeySystemType(%d) SUCC", nType);
	return APP_SUCC;
}

int PubGetKeySystemType()
{
	return gcKeySystem;
}

static int LoadKeyByKLA(int nKeyType, int nKeyIndex, const char *psKey, const char *psKSN, int nKeyLen)
{
	enum sec_key_arch 
	{
		KEY_ARCH_FIXED = 0x100,
		KEY_ARCH_MKSK,
		KEY_ARCH_DUKPT,
		KEY_ARCH_RSA,
		KEY_ARCH_CUSTOM = 0x200
	};
	
	typedef struct 
	{
		int nKeyArch;
		int nKeyType;
		int nKeyIndex;
		char sKey[48];
		int nKeyLen;
		char sKsn[10];
		char szKeyOwner[32];
		char sreserve[100];
	}STKEYDATAINFO;
	
	int nRet;
	char szAppPath[255] = {0};
	char szAppName[32] = {0};
	APP_EXIT_STATUS *exitStatus;
	int nFlag = 0;
	char szPosType[64] = {0};
	char szOsVersion[64] = {0};

	nRet = PubGetPosInfo(MODEL, szPosType, sizeof(szPosType) - 1);
	if(nRet != NAPI_OK)
	{
		PINPAD_TRACE_SECU("Napi_SysGetPosInfo error nRet=%d",nRet);
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		return APP_FAIL;
	}

	if(strstr(szPosType, "SP600") != NULL || strstr(szPosType, "SP60") != NULL || strstr(szPosType, "SP50") != NULL)
	{
		nFlag = 1;
	}

	if(strstr(szPosType, "ME51") != NULL)
	{
		nRet = PubGetPosInfo(OS_VERSION, szOsVersion, sizeof(szOsVersion) - 1);
		if(nRet != NAPI_OK)
		{
			PINPAD_TRACE_SECU("Napi_SysGetPosInfo error nRet=%d",nRet);
			ProSetSecurityErrCode(ERR_LOADKEY,nRet);
			return APP_FAIL;
		}
		if(strcmp(szOsVersion, "2.0.0") < 0)
		{
			nFlag = 1;
		}
	}
		
	STKEYDATAINFO stKeyDataInfo;
	memset(&stKeyDataInfo, 0, sizeof(STKEYDATAINFO));
	if(SECRITY_KEYSYSTEM_MSKEY == gcKeySystem)
	{
		stKeyDataInfo.nKeyArch = KEY_ARCH_MKSK;
	}
	else
	{
		stKeyDataInfo.nKeyArch = KEY_ARCH_DUKPT;
		memcpy(stKeyDataInfo.sKsn, psKSN, 10);
	}
	stKeyDataInfo.nKeyType = nKeyType;
	stKeyDataInfo.nKeyIndex = nKeyIndex;
	memcpy(stKeyDataInfo.sKey, psKey, nKeyLen);
	stKeyDataInfo.nKeyLen = nKeyLen;
	getcwd(szAppPath, sizeof(szAppPath) - 1);
	strcpy(szAppName, strrchr(szAppPath, '/') + 1);
	if(nFlag == 1)
	{
		strcpy(stKeyDataInfo.szKeyOwner, szAppName);
	}
	else
	{
		strcpy(stKeyDataInfo.szKeyOwner, "*");
	}

	nRet = NAPI_AppLaunch("/usr/sbin/NKLoader", (void *)&stKeyDataInfo, sizeof(STKEYDATAINFO), &exitStatus);
	if (nRet != NAPI_OK)
	{
        PINPAD_TRACE_SECU("NAPI_AppLaunchByPath error nRet=%d",nRet);
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		return APP_FAIL;
	}
	if(exitStatus->Code == 0 && exitStatus->Data != NULL && exitStatus->Len == sizeof(int))
	{
		nRet = *((int *)exitStatus->Data);
		free((char *)exitStatus->Data);
        PINPAD_TRACE_SECU("NAPI_AppLaunchByPath nRet = %d, ",nRet);
	}
	else
	{
        PINPAD_TRACE_SECU("NAPI_AppLaunchByPath return error");
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		return APP_FAIL;
	}

	if (nRet != NAPI_OK)
	{
        PINPAD_TRACE_SECU("nRet = %d",nRet);
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		return APP_FAIL;
	}

	return NAPI_OK;
}

/**
* @brief Intall main key
* @param [in] nIndex  >= 1
* @param [in] psKey   The buffer of main key
* @param [in] psKSN   The buffer of KSN used in DUKPT mode
* @param [in] nKeyLen key length is 8 or 16
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubLoadMainKey(int nIndex, const char *psKey, const char *psKSN, int nKeyLen)
{
	int nRet;

	if(ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

	if(nKeyLen != 8 && nKeyLen != 16)
	{
		PINPAD_TRACE_SECU("nKeyLen error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	if(nIndex < 1 || nIndex > 250) /* only support 250 slots*/
	{
		PINPAD_TRACE_SECU("nIndex error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	if(psKey == NULL)
	{
		PINPAD_TRACE_SECU("psKey error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		nRet = LoadKeyByKLA(KEY_TYPE_TMK, nIndex, psKey, psKSN, nKeyLen);
		if (nRet != NAPI_OK)
		{
			PINPAD_TRACE_SECU("LoadKeyByKLA error nRet=%d",nRet);
			ProSetSecurityErrCode(ERR_LOADKEY,nRet);
			return APP_FAIL;
		}
		gnMainKeyIndex = nIndex;
		PINPAD_TRACE_SECU("PubLoadMainKey(%d) INSIDE SUCC ",gnMainKeyIndex);
		return APP_SUCC;
	}
	else
	{
		nRet = gstPinpad.pLoadKey(KEY_TYPE_TMK, nIndex, psKey, nKeyLen, NULL);
		if (nRet == APP_SUCC)
		{
			gnMainKeyIndex = nIndex;
		}
		return nRet;
	}

	return APP_FAIL;
}

/**
* @brief Install word key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in] nKeyType      Key type---EM_KEY_TYPE
* @param [in] psKey         Buffer of the working key
* @param [in] nKeyLen       key length 16
* @param [in] psCheckValue  The buffer of key check value (3 bytes) or NULL
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubLoadWorkKey(int nKeyType,const char* psKey, int nKeyLen,const char* psCheckValue)
{
	int nRet;
	ST_SEC_KEYIN_DATA stKGData;
	ST_SEC_KCV_DATA stKcvData;

	if (ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

	if (SECRITY_KEYSYSTEM_DUKPT == gcKeySystem)
	{
        PINPAD_TRACE_SECU("SECRITY_KEYSYSTEM_MSKEY error : not support workkey!");
		ProSetSecurityErrCode(ERR_PINPAD_NOSUPPORT_WK,0);
		return APP_FAIL;
	}
	if(psKey == NULL)
	{
		PINPAD_TRACE_SECU("psKey error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	if(nKeyLen != 16 && nKeyLen != 24) /* no support 8 bytes*/
	{
		PINPAD_TRACE_SECU("nKeyLen error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	if(nKeyType != KEY_TYPE_MAC && nKeyType != KEY_TYPE_PIN && nKeyType != KEY_TYPE_DATA)
	{
		PINPAD_TRACE_SECU("nKeyType error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		memset(&stKGData, 0, sizeof(stKGData));
		memset(&stKcvData, 0, sizeof(stKcvData));
		stKGData.ucKEKIdx = gnMainKeyIndex;
		stKGData.KEKType = KEY_TYPE_DES;
		stKGData.KEKUsage = KEY_USE_KEK;
		stKGData.CipherMode = SEC_CIPHER_MODE_ECB;
		stKGData.ucKeyIdx  = gnMainKeyIndex;
		stKGData.KeyType = KEY_TYPE_DES;

		switch (nKeyType)
		{
		case KEY_TYPE_PIN:
			stKGData.KeyUsage = KEY_USE_PIN;
			break;
		case KEY_TYPE_MAC:
			stKGData.KeyUsage = KEY_USE_MAC;
			break;
		case KEY_TYPE_DATA:
			stKGData.KeyUsage = KEY_USE_DATA;
			break;
		default:
			ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
            PINPAD_TRACE_SECU("nKeyType[%d] is invalid ", nKeyType);
			return APP_FAIL;
			break;
		}
		stKGData.pKeyData = (uchar *)psKey;
		stKGData.nKeyDataLen = nKeyLen;
		stKGData.nKeyLen = nKeyLen;
		stKcvData.nCheckMode = NAPI_SEC_KCV_NONE;
		stKcvData.nLen = 0;
		if (psCheckValue != NULL)
		{
			stKcvData.nCheckMode = NAPI_SEC_KCV_ZERO;
			stKcvData.nLen = 0x04;
			memcpy(stKcvData.sCheckBuf, psCheckValue, 4);
		}
		nRet = NAPI_SecGenerateKey(SEC_KIM_CIPHER, &stKGData, &stKcvData);
		if (nRet != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_LOADKEY,nRet);
            PINPAD_TRACE_SECU("NAPI_SecGenerateKey error [nRet = %d]",nRet);
			return APP_FAIL;
		}
		PINPAD_TRACE_SECU("(%d,%d) INSIDE SUCC",gnMainKeyIndex,nKeyType);
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pLoadKey(nKeyType, gnMainKeyIndex, psKey, nKeyLen,(char *)psCheckValue);
	}
}

static void ProShowStar(int nPressKey, int nMaxLen)
{
    int i;
	char szStar[32]={0};
    
    for(i = 0 ; i < nPressKey ; i++)
    {
    	szStar[i] = '*' ;
    }

	PubDisplayStrInline(DISPLAY_MODE_CLEARLINE, 3, szStar);
	NAPI_ScrRefresh();
}

int ActivePIN(L3_PIN_TYPE type, char *pszPan, publicKey *pinPK)
{
	int nRet = 0;
	EM_SEC_CRYPTO_KEY_TYPE KeyType = KEY_TYPE_DES;
	ST_NAPI_RSA_KEY stRsaKey, *pstRsaKey = NULL;
	EM_SEC_VPP_SESSION_TYPE SessionType;
	NAPI_EM_SEC_PIN emPinMode = NAPI_SEC_PIN_ISO9564_0;

	if (type == L3_PIN_ONLINE)
	{
		if (gcKeySystem == SECRITY_KEYSYSTEM_MSKEY)
		{
			SessionType = SEC_VPP_MASTER_SESSION;
		}
		else
		{
			SessionType = SEC_VPP_DUKPT;
		}
	}
	else
	{
		emPinMode = NAPI_SEC_PIN_ISO9564_2;
		pszPan = NULL;
		if (type == L3_PIN_OFFLINE)
		{
			SessionType = SEC_VPP_EMV_OFFLINE_CLEARPIN;
		}
		else
		{
		    if (pinPK == NULL)
		    {
		        return -1;
		    }

			SessionType = SEC_VPP_EMV_OFFLINE_ENCPIN;
			PINPAD_TRACE_SECU("pinPK->pk_mod_len=%#x", pinPK->modulusLen);
			//TRACE_HEX(pinPK->modulus, pinPK->modulusLen, "pstPinPK->pk_modulus:");
			//TRACE_HEX(pinPK->exponent, 3, "pstPinPK->pk_exponent:");
			stRsaKey.usBits = pinPK->modulusLen*8;
			memcpy(stRsaKey.sModulus, pinPK->modulus, pinPK->modulusLen);
			memcpy(stRsaKey.sExponent, pinPK->exponent, 3);
            pstRsaKey = &stRsaKey;
		}
	}

	nRet = NAPI_SecVPPInit(SessionType, KeyType, gnMainKeyIndex, pszPan, emPinMode, gnTimeOut, pstRsaKey, NULL, 0);
	if(SessionType == SEC_VPP_DUKPT && nRet == NAPI_ERR_SECKM_KEY_ALREADY_USED)
	{
		PubDukptIncreaseKSN();
		nRet = NAPI_SecVPPInit(SessionType, KeyType, gnMainKeyIndex, pszPan, emPinMode, gnTimeOut, pstRsaKey, NULL, 0);
	}
	PINPAD_TRACE_SECU("NAPI_SecVPPInit %d gnMainKeyIndex %d, nRet %d\n", nRet, gnMainKeyIndex, nRet);
	if (nRet != NAPI_OK)
	{
		ProSetSecurityErrCode(ERR_GETPIN,nRet);
		PINPAD_TRACE_SECU("NAPI_SecVPPInit error,nRet=%d",nRet);
		return nRet;
	}

	return NAPI_OK;
}

static int DetectIcstatus()
{
	int nRet, nStatus;

	nRet = NAPI_SCGetStatus(&nStatus);
	if (nRet != NAPI_OK)
	{
		return APP_FAIL;
	}
	PubDebug("nRet = %d nStatus = %d", nRet, nStatus);
	nRet = nStatus & 0x0B;
	switch(nRet)
	{
	case 0x01: // IC1_EXIST
	case 0x01 | 0x02:
		return APP_SUCC;
	default:
		return APP_FAIL;
	}
	return APP_FAIL;
}

static int ProGetPin(char *psPin, int *pnPinLen, int nMode, const char *pszCardno,const char *pszAmount,
              int nMaxLen, int nMinLen)
{
	int nRet,i;
	char szExpPinLenIn[100+1];
	char szDispAmt[64+1];
	int nPressKey = 0;
	int nFlag=0;
	char szConvert[30]= {0};
	char sPin[20];
	uchar szKsn[20] = {0};
	int nKsnLen = 0;
	char szCardNo[19+1] = {0};
	int nEvent, nOutPinLen;
	char cEntryMode = 0x00;
	
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		if(nMode == PINTYPE_PLAIN || nMode == PINTYPE_PLAIN_STAR)
		{
			ProSetSecurityErrCode(ERR_PINPAD_NOSUPPORTMODE,0);
			PINPAD_TRACE_SECU("nMode error");
			return APP_FAIL;
		}

		memset(szDispAmt, 0, sizeof(szDispAmt));
		if(pszAmount != NULL && 0 != strlen(pszAmount))
		{
			if (memcmp(pszAmount, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12) == 0)
			{
				;
			}
			else
			{
				PubConvAmount(	NULL,(uchar *)pszAmount,2,(uchar *)szDispAmt);
			}

			PubClearAll();
			sprintf(szConvert,"%s",szDispAmt);
			PubDisplayStrInline(0|DISPLAY_MODE_TAIL, 1, szConvert);
			PubDisplayStrInline(0, 2, "Please enter PIN:");
			PubDisplayStrInline(0, 4, "press [ENTER] to skip");
			PubUpdateWindow();
		}
		else
		{
			PubClear2To4();
			PubDisplayStrInline(0, 2, "Please enter PIN:");
			PubDisplayStrInline(0, 4, "press [ENTER] to skip");		
			PubUpdateWindow();
		}
 
		/**< Password length (Can be separated as 0,4,6) */
		strcpy(szExpPinLenIn,"0");
		if (nMinLen <= 0)
			nMinLen=1;//avoid multiple 0
		for (i=nMinLen; i<=nMaxLen; i++ )
		{
			sprintf(szExpPinLenIn+strlen(szExpPinLenIn),",%d",i);
		}

		if(nMode == PINTYPE_WITHPAN)
		{
			strcpy(szCardNo, pszCardno);
		}
		else
		{
			strcpy(szCardNo, "0000000000000000");
		}
		PubSysMsDelay(200);
		nRet = ActivePIN(L3_PIN_ONLINE, szCardNo, NULL);
		if (nRet != NAPI_OK)
		{
			return nRet;
		}

		nPressKey = 0;
		NAPI_L3GetData(L3_DATA_POS_ENTRY_MODE, &cEntryMode, 1);
		while(1)
		{
			if(cEntryMode == 0x05 && DetectIcstatus() != APP_SUCC) //ATTR_CONTACT = 0x05
			{
				PubDebug("remove card and quit");
				_FINISH_PIN_INPUT_THREAD(L3_ERR_REMOVE_INTERRUPT);
			}
			nRet = NAPI_SecVPPGetEvent(&nEvent, (uchar *)sPin, &nOutPinLen, szKsn, &nKsnLen);
			if (nRet == NAPI_ERR_SECVP_PIN_TOO_SHORT)
			{
				continue;
			}
			if (nRet != NAPI_OK)
			{
				ProSetSecurityErrCode(ERR_GETPINRESULT,nRet);
		        PINPAD_TRACE_SECU("NAPI_SecVPPGetEvent error,nRet=%d", nRet);
				if (nRet == NAPI_ERR_TIMEOUT
					|| nRet == NAPI_ERR_SECVP_TIMED_OUT)
				{
					_FINISH_PIN_INPUT_THREAD(L3_ERR_CANCEL);
				}
				_FINISH_PIN_INPUT_THREAD(L3_ERR_FAIL);
			}
			switch(nEvent)
			{
			case NAPI_SEC_VPP_KEY_PIN:
			case NAPI_SEC_VPP_KEY_BACKSPACE:
			case NAPI_SEC_VPP_KEY_CLEAR:
		        nPressKey = sPin[0] & 0x0F;
		        ProShowStar(nPressKey, PINMAXLEN);
				break;
			case NAPI_SEC_VPP_KEY_ENTER:
				PINPAD_TRACE_SECU("nStatus=SEC_VPP_KEY_ENTER,nRet = %d",nRet);
				nFlag = 1;
				break;
			case NAPI_SEC_VPP_KEY_ESC:
				PINPAD_TRACE_SECU("ESC:user quit");
				*pnPinLen = 0;
				return APP_QUIT;
				break;
			default:
				break;
			}
			if(nFlag == 1)
			{
				break;
			}
		}
        
		memcpy(psPin, sPin, 8);
		if(memcmp(psPin, "\x00\x00\x00\x00\x00\x00\x00\x00", 8) == 0)
			*pnPinLen = 0;
		else
			*pnPinLen = 8;
		PINPAD_TRACE_SECU("PubGetPin SUCC");
		return APP_SUCC;
	}
	else
	{
		nRet = gstPinpad.pGetPinBlock(psPin, pnPinLen, nMode, gnMainKeyIndex, pszCardno, strlen(pszCardno), nMaxLen);
		PubClrPinPad();
		return nRet;
	}
}



/**
* @brief Get PIN-Block(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [out] psPin         Pointer to PIN-block buffer
* @param [out] pnPinLen      Pointer to PIN-block-length buffer
* @param [in]  nMode         EM_PIN_TYPE
* @param [in]  pszCardno     Card Number/(NULL)
* @param [in]  pszAmount     Amount(used for display, e.g. buff:000000001234-->display:12.34)
* @param [in]  nMaxLen       Maximum PIN length (4-12)
* @param [in]  nMinLen       Minimun PIN length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubGetPin(char *psPin, int *pnPinLen, int nMode, const char *pszCardno,const char *pszAmount,
              int nMaxLen, int nMinLen)
{	
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if(psPin == NULL || pnPinLen == NULL || nMinLen < 0 || nMaxLen <= 0 || nMinLen > nMaxLen)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("incorrect params");
		return APP_FAIL;
	}
	if(nMode < PINTYPE_WITHPAN || nMode > PINTYPE_PLAIN)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("incorrect params");
		return APP_FAIL;
	}
	if(nMode == PINTYPE_WITHPAN && pszCardno == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("incorrect params");
		return APP_FAIL;
	}	

	return ProGetPin(psPin, pnPinLen, nMode, pszCardno, pszAmount, nMaxLen, nMinLen);
}

/**
* @brief Calculate MAC(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nMode         Method---EM_MAC_TYPE
* @param [in]  psData        Data that needs to be calculated
* @param [in]  nDataLen      Date length
* @param [out] psMac         MAC
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubCalcMac(int nMode,const char *psData, int nDataLen, char * psMac)
{
    int nMacType;
	int nMacLen;
	int nRet = 0;
	uchar sKsnOut[20];
	int nOutKsnLen;

	if (ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}

	if (psData == NULL || psMac == NULL || nDataLen < 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("PubCalcMac incorrect params");
		return APP_FAIL;
	}

	PINPAD_TRACE_SECU("gnMainKeyIndex = %d, nMode = %d,nDataLen = %d",gnMainKeyIndex,nMode,nDataLen);
	
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		if(SECRITY_KEYSYSTEM_MSKEY == gcKeySystem) //SEC_MAC_DUKPT_LAST
		{
			switch (nMode)
			{
			case MAC_TYPE_ECB:
				nMacType = SEC_MAC_TDES_UNIONPAY_ECB;
				break;
			case MAC_TYPE_X99:
				nMacType = SEC_MAC_TDES_X99;
				break;
			case MAC_TYPE_X919:
				nMacType = SEC_MAC_TDES_X919;
				break;
			case MAC_TYPE_9606:
				nMacType = SEC_MAC_TDES_LAST;
				break;
			case MAC_TYPE_AES_X99:
				nMacType = SEC_MAC_AES_X99;
				break;
			case MAC_TYPE_AES_X9606:
				nMacType = SEC_MAC_AES_X99;
				break;
			default:
                PINPAD_TRACE_SECU("nMode error ");
				ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
				return APP_FAIL;
				break;
			}
		}
		else
		{
			switch (nMode)
			{
			case MAC_TYPE_ECB:
				nMacType = SEC_MAC_DUKPT_UNIONPAY_ECB;
				break;
			case MAC_TYPE_X99:
				nMacType = SEC_MAC_DUKPT_X99;
				break;
			case MAC_TYPE_X919:
				nMacType = SEC_MAC_DUKPT_X919;
				break;
			case MAC_TYPE_9606:
				nMacType = SEC_MAC_DUKPT_LAST;
				break;
			case MAC_TYPE_AES_X99:
				nMacType = SEC_MAC_AES_DUKPT_X99;
				break;
			case MAC_TYPE_AES_X9606:
				nMacType = SEC_MAC_AES_DUKPT_LAST;
				break;
			case MAC_TYPE_ECB_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_UNIONPAY_ECB;
				break;
			case MAC_TYPE_X99_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_X99;
				break;
			case MAC_TYPE_X919_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_X919;
				break;
			case MAC_TYPE_9606_RESP:
				nMacType = SEC_MAC_DUKPT_RESP_LAST;
				break;
			case MAC_TYPE_AES_X99_RESP:
				nMacType = SEC_MAC_AES_DUKPT_RESP_X99;
				break;
			case MAC_TYPE_AES_X9606_RESP:
				nMacType = SEC_MAC_AES_DUKPT_RESP_LAST;
				break;
			default:
                PINPAD_TRACE_SECU("nMode error ");
				ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
				return APP_FAIL;
				break;
			}
		}
		nMacLen = 0;
        nRet = NAPI_SecGenerateMAC((EM_SEC_MAC_TYPE)nMacType, gnMainKeyIndex, NULL, 0, (uchar *)psData, nDataLen, NULL, 0,
					(uchar *)psMac, &nMacLen, sKsnOut, &nOutKsnLen);
		if (nRet != NAPI_OK)
		{
			ProSetSecurityErrCode(ERR_GETMAC,nRet);
            PINPAD_TRACE_SECU("NAPI_SecGenerateMAC error =%d",nRet);
			return APP_FAIL;
		}

        PINPAD_TRACE_SECU("PubCalcMac SUCC");
		return APP_SUCC;
	}
	else
	{
		switch (nMode)
		{
		case MAC_TYPE_X99:
			nMacType = PINPAD_X99;
			break;
		case MAC_TYPE_X919:/**<X919 :X99+3DES(last 8 bytes)*/
			nMacType = PINPAD_X919;
			break;
		case MAC_TYPE_ECB:
			nMacType = PINPAD_ECB;
			break;
		case MAC_TYPE_9606:
			nMacType = PINPAD_9606;
			break;	
		default:
			PINPAD_TRACE_SECU("nMode error ");
			ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
			return APP_FAIL;
			break;
		}

		return gstPinpad.pCalMac(psMac, nMacType, gnMainKeyIndex, psData, nDataLen);
	}
}

/**
* @brief Increase KSN
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubDukptIncreaseKSN(void)
{
	int nRet;
    ST_SEC_KEYIN_DATA stKGData;
    ST_SEC_KCV_DATA stKcvData;

    memset(&stKGData, 0, sizeof(stKGData));
    memset(&stKcvData, 0, sizeof(stKcvData));

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		stKGData.ucKEKIdx = gnMainKeyIndex;
		stKGData.ucKeyIdx  = gnMainKeyIndex;
		stKGData.KEKType = KEY_TYPE_DES;
		stKGData.KEKUsage = KEY_USE_DUKPT;
		stKGData.KeyType = KEY_TYPE_DES;
		stKGData.KeyUsage = KEY_USE_DUKPT;
		stKGData.CipherMode = SEC_CIPHER_MODE_ECB;
		stKcvData.nCheckMode = NAPI_SEC_KCV_NONE;
		stKcvData.nLen = 0;
		nRet = NAPI_SecGenerateKey(SEC_KIM_DUKPT_DERIVE, &stKGData, &stKcvData);
		if(nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecIncreaseDukptKsn error = %d",nRet);
			return APP_FAIL;
		}
	}
	else
	{
		return APP_FAIL;
	}
    PINPAD_TRACE_SECU("PubDukptIncreaseKSN SUCC");
	return APP_SUCC;
}

/**
* @brief Get KSN
* @param [out] psKSN    KSN data---10byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-5-25
*/
int PubGetDukptKSN(char *psKSN)
{
	int nRet, nKSNLen;
	EM_SEC_CRYPTO_KEY_TYPE KeyType;
	EM_SEC_KEY_USAGE KeyUsage;

	if(psKSN == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
        PINPAD_TRACE_SECU("psKSN == NULL");
		return APP_FAIL;
	}
	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		KeyType = KEY_TYPE_DES;
		KeyUsage = KEY_USE_DUKPT;
		nRet = NAPI_SecGetKeyInfo(SEC_KEY_INFO_KSN, (uchar)gnMainKeyIndex, KeyType, KeyUsage, NULL, 0, (uchar *)psKSN, &nKSNLen);
		if(nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecGetDukptKsn error = %d",nRet);
			return APP_FAIL;
		}
	}
	else
	{
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubGetDukptKSN SUCC");
	return APP_SUCC;
}

/**
* @brief Display string on pinpad
* @param [in]  pszLine1  String displayed in line 1
* @param [in]  pszLine2  String displayed in line 2
* @param [in]  pszLine3  String displayed in line 3
* @param [in]  pszLine4  String displayed in line 4
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDispPinPad(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(pszLine1 == NULL && pszLine2 == NULL && pszLine3 == NULL && pszLine4 == NULL)
	{
		PINPAD_TRACE_SECU("pszData error ");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pDispPinpad(pszLine1, pszLine2, pszLine3, pszLine4);
	}
}

/**
* @brief Clear the message and display default info on pinpad(for PinPad)
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubClrPinPad(void)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;

	if(gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pClrPinpad();
	}
}

/**
* @brief Do TDES encryption with specified 'nKeyType' key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  nLen          Length of data
* @param [in]  psSrc         Source data
* @param [out] psDest        Target data
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDes3(const char* psSrc, int nLen, char *psDest)
{
	int nRet;
	ST_SEC_ENCRYPTION_DATA encData;
	int encDataLen = 0;
	uchar encKsnData[20] = {0}; /**< Max Len 20 */
	int encKsnDataLen = 0;

	if(ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	if(psSrc == NULL || psDest == NULL || nLen <= 0)
	{
        PINPAD_TRACE_SECU("psSrc /psDest error ");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

    PINPAD_TRACE_SECU("gnMainKeyIndex=%d", gnMainKeyIndex);
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		memset(&encData, 0, sizeof(encData));
		encData.ucKeyID = gnMainKeyIndex;
		encData.PaddingMode = SEC_PADDING_NONE;
		encData.psDataIn = (uchar *)psSrc;
		encData.unDataInLen = nLen;

		if (gcKeySystem == SECRITY_KEYSYSTEM_MSKEY)
		{
			encData.CipherType = SEC_CIPHER_DES_ECB; /* only support 3des*/
			encData.KeyUsage = KEY_USE_DATA;
		}
		else
		{
			encData.CipherType = SEC_CIPHER_DUKPT_ECB_BOTH;
			encData.KeyUsage = KEY_USE_DUKPT;
		}

		nRet = NAPI_SecEncryption(&encData, (uchar *)psDest, &encDataLen, encKsnData, &encKsnDataLen);
		if (nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecEncryption error nRet = %d",nRet);
			ProSetSecurityErrCode(ERR_CALCDES, nRet);
			return APP_FAIL;
		}

        PINPAD_TRACE_SECU("PubDes3 SUCC");
		//TRACE_HEX((char*)encKsnData, encKsnDataLen,"MSKEY [%s] DES3 len [%d] ksnLen [%d] ksn :",
		//gcKeySystem == SECRITY_KEYSYSTEM_MSKEY ? "YES" : "NO", encDataLen, encKsnDataLen);

		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pDespinpad(psSrc, nLen, psDest, gnMainKeyIndex, DESMODE_3DES);
	}
	return APP_FAIL;
}

/**
* @brief Do TDES decryption with specified 'nKeyType' key(calling 'PubSetCurrentMainKeyIndex' to set index before calling this api)
* @param [in]  psSrc         Source data
* @param [in]  nLen          Length of data
* @param [out] psDest        Target data
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubUnDes3(const char *psSrc, int nLen, char *psDest)
{
	int nRet;
	ST_SEC_ENCRYPTION_DATA decData;
	int decDataLen = 0;
	uchar decKsnData[20] = {0}; /**< max len 20 */
	int decKsnDataLen = 0;

	if(ProCheckInit() != APP_SUCC)
	{
		return APP_FAIL;
	}
	if(psSrc == NULL || psDest == NULL)
	{
        PINPAD_TRACE_SECU("psSrc /psDest error ");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

    PINPAD_TRACE_SECU("gnMainKeyIndex=%d", gnMainKeyIndex);
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		memset(&decData, 0 ,sizeof(decData));
		decData.ucKeyID = gnMainKeyIndex;
		decData.PaddingMode = SEC_PADDING_NONE;
		decData.psDataIn = (uchar *)psSrc;
		decData.unDataInLen = nLen;
		if (gcKeySystem == SECRITY_KEYSYSTEM_MSKEY)
		{
			decData.CipherType = SEC_CIPHER_DES_ECB; /* only support 3des*/
			decData.KeyUsage = KEY_USE_DATA;
		}
		else
		{
			decData.CipherType = SEC_CIPHER_DUKPT_ECB_RESP;
			decData.KeyUsage = KEY_USE_DUKPT;
		}
		nRet = NAPI_SecDecryption(&decData, (uchar *)psDest, &decDataLen, decKsnData, &decKsnDataLen);
		if (nRet != NAPI_OK)
		{
            PINPAD_TRACE_SECU("NAPI_SecDecryption error nRet = %d",nRet);
			ProSetSecurityErrCode(ERR_CALCDES,nRet);
			return APP_FAIL;
		}
        PINPAD_TRACE_SECU("PubUnDes3 SUCC");
		//TRACE_HEX((char*)decKsnData, decKsnDataLen,"MSKEY [%s] undes declen [%d] decKsnLen [%d] ksn :",
		//	gcKeySystem == SECRITY_KEYSYSTEM_MSKEY ? "YES" : "NO", decDataLen, decKsnDataLen);
		return APP_SUCC;
	}
	else
	{
		return gstPinpad.pDespinpad(psSrc, nLen, psDest, gnMainKeyIndex, DESMODE_3UNDES);
	}
}

int PubSoftDes(const char* psKey,const char* psSrc,char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,8,SALG_TDS_MODE_ENC);
	if(nRet != SALG_OK)
	{
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftDes SUCC");
	return APP_SUCC;
}
/**
* @brief Do soft-TDES encryption with 'psKey'.
* @param [in]  psKey         Key string
* @param [in]  psSrc         Source data---8byte
* @param [out] psDest        Target data---8byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSoftDes3(const char* psKey, const char* psSrc, char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		return APP_FAIL;
	}
	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,16,SALG_TDS_MODE_ENC);
	if (nRet != SALG_OK)
	{
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftDes3 SUCC");
	return APP_SUCC;
}

int PubSoftUnDes(const char* psKey,const char* psSrc,char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}
	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,8,SALG_TDS_MODE_DEC);
	if(nRet != SALG_OK)
	{
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftUnDes SUCC");
	return APP_SUCC;
}
/**
* @brief Do soft-TDES decryption with 'psKey'.
* @param [in]  psKey         Key string
* @param [in]  psSrc         Source data---8byte
* @param [out] psDest        Target data---8byte
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author Liug
* @date 2012-5-23
*/
int PubSoftUnDes3(const char* psKey,const char* psSrc,char *psDest)
{
	int nRet;

	if(psKey == NULL || psSrc == NULL || psDest == NULL)
	{
		PINPAD_TRACE_SECU("psKey/psSrc/psDest error");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
		return APP_FAIL;
	}

	nRet = Soft_AlgTDes((uchar*)psSrc,(uchar*)psDest,(uchar*)psKey,16,SALG_TDS_MODE_DEC);
	if(nRet != SALG_OK)
	{
		ProSetSecurityErrCode(ERR_ALGDES,nRet);
        PINPAD_TRACE_SECU("NAPI_AlgTDes error = %d",nRet);
		return APP_FAIL;
	}
	PINPAD_TRACE_SECU("PubSoftUnDes3 SUCC");
	return APP_SUCC;
}

/**
* @brief Get security library version
* @param [out] pszVer  Version information, 12bytes
* @return 
* @li void
*/
void PubGetSecrityVerion(char *pszVer)
{
	if (pszVer != NULL)
	{
		strcpy(pszVer, SECURITYLIBVER);
	}
	else
	{
		PINPAD_TRACE_SECU("pszVer is NULL");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM,0);
	}
}

/**
* @brief pinpad beep
* @param [in]  nDuration     time of duration
* @param [in]  Buzzer tone   0 – Default   1 – Alert (750Hz)  2 – Success (1500Hz) 
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubPinpadBeep(int nDuration, int nType)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pPinpadBeep(nDuration, nType);
	}
}

/**
* @brief show QR Code
* @param [in]  nVersion	Version(defult 0, from 0-40)
* @param [in]  pszBuffer content of display
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubGenAndShowQr(int nVersion, char *pszBuffer)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pPinpadQR(nVersion, pszBuffer);
	}
}

/**
* @brief Scan the code through the Pinpad
* @param [out]  results of scanning.
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubDoScanByPinpad(char *pszBuffer)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pPinpadDoScan(pszBuffer);
	}
}

/**
* @brief show QR Code
* @param [in]  pszCharaterCode	feature code
* @param [in]  name of signature
* @param [in]  nTimeOut
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
*/
int PubDoSignatureByPinpad(char *pszCharaterCode, char *pszSignName, int nTimeOut)
{
	if(ProCheckInit() != APP_SUCC)
		return APP_FAIL;
	if (gnSecurityMode == SECRITY_MODE_INSIDE)
	{
		PINPAD_TRACE_SECU("NOT support");
		return APP_QUIT;
	}
	else
	{
		return gstPinpad.pEsignature(pszCharaterCode, pszSignName, nTimeOut);
	}
}

