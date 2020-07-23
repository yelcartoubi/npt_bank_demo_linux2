#include "pinpad.h"
#include "lsecurity.h"
#include "ltool.h"
#include "lsecurity.h"
#include "lui.h"
#include "pinpadtool.h"
#include "libapiinc.h"
#define PINPAD_MAX_DATA_LEN 5240


static int nPinpadBps = BPS115200;
static int nPinpadPort;	
static int nPinpadTimeOut = 5;	
static uchar sSendBuf[PINPAD_MAX_DATA_LEN*2+5];
static uchar sRecvBuf[PINPAD_MAX_DATA_LEN*2+5];


#define	 SEPERATOR_CMD					(0x2F)


#define MEMSETSENDBUF ( memset(sSendBuf, 0, sizeof(sSendBuf)) )
#define MEMSETRECVBUF ( memset(sRecvBuf, 0, sizeof(sRecvBuf)) )

static int ProInitPinpadAux(const int nPort, const int nBps);
static int ProGetDeviceAttribute(const int nPort, const int nBps, char *pszDeviceAttribute, int nWaitTime);


static int PinPad_InsertAsyn(unsigned char *psBuf, unsigned int *punLen)
{
	char sTemp[PINPAD_MAX_DATA_LEN*2+10] = {0};
	int nLen = 2;
	
	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}

	memcpy(sTemp, psBuf, *punLen);
	psBuf[0] = STX;
	PubIntToBcd((char *)psBuf + 1, &nLen, *punLen);
	memcpy(psBuf + 3, sTemp, *punLen);
	psBuf[*punLen + 3] = ETX;
	PubCalcLRC((char *)psBuf + 1, *punLen + 3, (char *)psBuf + *punLen + 4);
	*punLen += 5;
	return APP_SUCC;
}

static int PinPad_DeleteAsyn(unsigned char *psBuf, unsigned int *punLen)
{
	int nLen = 0;
	char cLRC = 0;
	char sTemp[PINPAD_MAX_DATA_LEN*2+5] = {0};
	
	if (NULL == psBuf || NULL == punLen)
	{
		return APP_FAIL;
	}
	if (psBuf[0] != STX)
	{
		return APP_FAIL;
	}
	if (*punLen < 5)
	{
		return APP_FAIL;
	}
	PubBcdToInt((char *)psBuf + 1, &nLen);
	if (*punLen != nLen + 5)
	{
		return APP_FAIL;
	}
	if (PubCalcLRC((char *)psBuf + 1, nLen + 3, &cLRC) != APP_SUCC)
	{
		return APP_FAIL;
	}	
	if (*(psBuf + *punLen - 1) != cLRC)
	{
		return APP_FAIL;
	}
	memcpy(sTemp, psBuf + 3, nLen);
	memcpy(psBuf, sTemp, nLen);
	*punLen = nLen;
	return APP_SUCC;
}

static int PinPad_SendRecv(char cFlag, uchar *psSend, uint nSendLen, uchar *psRecv, uint *pnRecvLen, int nPort, int nTimeOut)
{
	int i;
	int nRet;
	uint nLen;
	int nLen2;
	
	char sSendBuf[PINPAD_MAX_DATA_LEN*2+5] = {0};
	char sRecvBuf[PINPAD_MAX_DATA_LEN*2+5] = {0};
	uint unTimeOut=0;
	int nCode=0;
	int nSumLen=0;
	
	memcpy(sSendBuf, psSend, nSendLen);
	nLen = nSendLen;
	nRet = PinPad_InsertAsyn((uchar *)sSendBuf, &nLen);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("nRet = %d",nRet);
		return APP_FAIL;
	}
	PubDebugData("PinPad_SendRecv send: ", sSendBuf, nLen);
	unTimeOut = PubGetOverTimer(nTimeOut * 1000);
	NAPI_KbFlush();
	
	for(i = 0; i < 3; i++)
	{
		nRet = NAPI_PortWrite(nPort, (uchar *)sSendBuf, nLen);
		if (nRet != NAPI_OK)
		{
			PINPAD_TRACE_SECU("write nRet = %d", nRet);
			return APP_FAIL;
		}

		if (memcmp(psSend, "3C", 2) == 0)
		{
			nRet = APP_SUCC;
			goto SEND_ACK;
		}
		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
			PINPAD_TRACE_SECU("cancel");
			nRet = APP_QUIT;
			goto SEND_ACK;
		}

		nSumLen = 1;
		nRet = NAPI_PortRead(nPort, (uchar *)sRecvBuf, &nSumLen, 3 * 1000);
		if(nRet == NAPI_ERR_TIMEOUT)
		{
			continue;
		}
		else if(nRet == NAPI_OK && nSumLen == 1 && sRecvBuf[0] == NAK)
		{
			continue;
		}
		else if(nRet == NAPI_OK && nSumLen == 1 && sRecvBuf[0] == ACK)
		{
			break;
		}
		else
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			return APP_FAIL;
		}
	}
	if(i == 3)
	{
		PINPAD_TRACE_SECU("i = 3 and quit");
		ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
		return APP_FAIL;
	}

	//This command does not receive a response message; the PIN pad response consists of just the single [ACK] character.
	if(cFlag == 0)
	{
		return APP_SUCC;
	}
	sRecvBuf[0] = 0;
	nSumLen = 0;
	while(1)
	{		
		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
			if (memcmp(psSend, "32", 2) == 0)
			{		
				PubCancelPIN_SP100();
			}
			PINPAD_TRACE_SECU("cancel");
			nRet = APP_QUIT;
			goto SEND_ACK;
		}

		nLen2 = 1;
		nRet = NAPI_PortRead(nPort, (uchar *)sRecvBuf, &nLen2, 1000);
		nSumLen += nLen2;

		if (PubTimerIsOver(unTimeOut) == APP_SUCC && nSumLen != 1)
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			return APP_FAIL;
		}
		if (nSumLen == 1 || nRet == APP_SUCC)
		{
			break;
		}
	}
	
	if(sRecvBuf[0] != STX)
	{
		PINPAD_TRACE_SECU("sRecvBuf[0] = %d", sRecvBuf[0]);
		return APP_FAIL;
	}
	nSumLen=0;

	NAPI_KbFlush();	
	unTimeOut = PubGetOverTimer(nTimeOut * 1000);
	while(1)
	{
		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
			PINPAD_TRACE_SECU("cancel");
			nRet = APP_QUIT;
			goto SEND_ACK;
		}
		nLen2 = 2;	
		nRet = NAPI_PortRead(nPort, (uchar *)sRecvBuf + 1, (int *)&nLen2, 1000);
		nSumLen += nLen2;
		if(PubTimerIsOver(unTimeOut)==APP_SUCC && nSumLen != 2)
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			nRet = APP_FAIL;
			goto SEND_ACK;
		}
		if(nSumLen == 2 || nRet == APP_SUCC)
		{
			break;
		}
	}
	nSumLen=0;
	NAPI_KbFlush();	
	unTimeOut = PubGetOverTimer(nTimeOut*1000);
	while(1)
	{
		PubBcdToInt((char *)sRecvBuf + 1, &nLen2);
		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
			PINPAD_TRACE_SECU("cancle");
			nRet = APP_QUIT;
			goto SEND_ACK;
		}

		nLen = 	nLen2 + 2;
		nRet = NAPI_PortRead(nPort, (uchar *)sRecvBuf + 3, (int *)&nLen, 1000);
		nSumLen += nLen;

		if (PubTimerIsOver(unTimeOut)==APP_SUCC && nSumLen != nLen2 + 2)
		{
			PINPAD_TRACE_SECU("ERR_PINPAD_NOTRET");
			ProSetSecurityErrCode(ERR_PINPAD_NOTRET, 0);
			return APP_FAIL;
		}
		if ((nSumLen == nLen2 + 2) || (nRet == APP_SUCC))
		{
			nLen = nLen2 + 5;
			break;
		}
	}

	sSendBuf[0] = ACK;
	NAPI_PortWrite(nPort, (uchar *)sSendBuf, 1);
	
	PubDebugData("PinPad_SendRecv sRecvBuf: ", sRecvBuf, nLen);
	nRet = PinPad_DeleteAsyn((uchar *)sRecvBuf, &nLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	memcpy(psRecv, sRecvBuf, nLen);
	*pnRecvLen = nLen;	
	return APP_SUCC;

SEND_ACK:
	PINPAD_TRACE_SECU("send ack");
	sSendBuf[0] = ACK;
	NAPI_PortWrite(nPort, (uchar *)sSendBuf, 1);

	return nRet;
	
}

static int ProInitPinpadAux(const int nPort, const int nBps)
{
    PORT_SETTINGS stPortSetting;
	PORT_TYPE PortType = (PORT_TYPE)nPort;

    memset(&stPortSetting, 0, sizeof(stPortSetting));
	switch(nBps)
	{
	case BPS9600:
		stPortSetting.BaudRate = BAUD9600;
		break;
	case BPS19200:
		stPortSetting.BaudRate = BAUD19200;
		break;
	case BPS38400:
		stPortSetting.BaudRate = BAUD38400;
		break;
	case BPS57600:
		stPortSetting.BaudRate = BAUD57600;
		break;
	case BPS115200:
		stPortSetting.BaudRate = BAUD115200;
		break;
	default:
		return APP_FAIL;
	}
	stPortSetting.DataBits = DATA_8;
	stPortSetting.Parity = PAR_NONE;
	stPortSetting.StopBits = STOP_1;

	if(NAPI_PortOpen(PortType, stPortSetting) != NAPI_OK)
	{
		return APP_FAIL;
	}
	if(NAPI_PortFlush(PortType) != NAPI_OK)
	{
		return APP_FAIL;
	}

	return APP_SUCC;
}

/**
* @brief Reset pinpad,set baudrate and port number.
* @param [in] nPort	Port number
* @param [in] nTimeOut	Timeout
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubResetPinpad_SP100(const int nPort, const int nTimeOut)
{
	int nRet = 0;
	char szDeviceAttribute[100] = {0};
	int nDefaultBps = BPS115200;

	if (nTimeOut <= 0)
	{
		PINPAD_TRACE_SECU("PubResetPinpad_SP100:nTimeOut<=0");
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	nPinpadTimeOut = 3;

	nRet = ProGetDeviceAttribute(nPort, nDefaultBps, szDeviceAttribute, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		ProSetSecurityErrCode(ERR_PINPAD_FAIL, 0);
		return APP_FAIL;
	}
	nPinpadPort = nPort;
	nPinpadTimeOut = nTimeOut;
	nPinpadBps = nDefaultBps;
	return APP_SUCC;
}

/**
* @brief Install main and work key
* @param [in] nKeyType		EM_KEY_TYPE
* @param [in] nKeyIndex		Key index
* @param [in] psKeyValue	Buffer of the working key
* @param [in] nKeyLen		key length 8 or 16 or 24
* @param [in] psCheckValue	check value (3 bytes) used to check if work key is correct
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-21
*/
int PubLoadKey_SP100(int nKeyType, int nKeyIndex, const char *psKeyValue, int nKeyLen, char *psCheckValue)
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "50", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	if (nKeyType == KEY_TYPE_TMK)
	{
		sSendBuf[3] = 0;	// clear key
	}
	else 
	{
		sSendBuf[3] = nKeyIndex;
	}
	sSendBuf[4] = 0x00; //0 - TR31 block, 1 - AESK,2 - DES block
	switch(nKeyType)
	{
	case KEY_TYPE_PIN:
		sSendBuf[5] = 0x00;
		break;
	case KEY_TYPE_MAC:
		sSendBuf[5] = 0x01;
		break;
	case KEY_TYPE_DATA:
		sSendBuf[5] = 0x02;
		break;
	case KEY_TYPE_TMK:
		sSendBuf[5] = 0x03;
		break;
	default:
		return APP_FAIL;
	}
	sSendBuf[6] = nKeyIndex;
	PubIntToC2(sSendBuf + 7, nKeyLen);
	memcpy(sSendBuf + 9, psKeyValue, nKeyLen);
	nLen = 9 + nKeyLen;
	if(psCheckValue != NULL)
	{
		memcpy(sSendBuf + nLen, psCheckValue, 3);
		nLen = nLen + 3;
	}
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "51", 2) != 0)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}



int PubInjectKey_SP100(int nKeyType, int nSrcIndex, int nDstIndex,const char *psKeyValue, int nKeyLen, char *psCheckValue)
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "50", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nSrcIndex;
	sSendBuf[4] = 0x02; //0 - TR31 block, 1 - AESK,2 - DES block
	switch(nKeyType)
	{
	case KEY_TYPE_PIN:
		sSendBuf[5] = 0x00;
		break;
	case KEY_TYPE_MAC:
		sSendBuf[5] = 0x01;
		break;
	case KEY_TYPE_DATA:
		sSendBuf[5] = 0x02;
		break;
	case KEY_TYPE_TMK:
		sSendBuf[5] = 0x03;
		break;	
	default:
		return APP_FAIL;
	}
	sSendBuf[6] = nDstIndex;
	PubIntToC2(sSendBuf + 7, nKeyLen);
	memcpy(sSendBuf + 9, psKeyValue, nKeyLen);
	nLen = 9 + nKeyLen;
	if(psCheckValue != NULL)
	{
		memcpy(sSendBuf + nLen, psCheckValue, 3);
		nLen = nLen + 3;
	}
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "51", 2) != 0)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_LOADKEY,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Get PIN inlcuding plain and encrypted
* @param [out] pszPin	 Pointer to PIN-block buffer
* @param [out] pnPinLen  Pointer to PIN-block-length buffer
* @param [in]  nMode	 0-Encrypt with PAN, 1-Encrypt without PAN 2-Plain
* @param [in]  pszCardno Card Number/(NULL)
* @param [in]  nPanLen	 Length of card NO.
* @param [in]  nMaxLen	 Maximum PIN length (4-12)
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
int PubGetPinBlock_SP100(char *psPin, int *pnPinLen, int nMode, int nKeyIndex, const char *pszCardno, int nPanLen, int nMaxLen)
{
	int nRet = 0;
	int nLen = 0;
	char *pszLine1 = "Please Input PIN:";
	char *pszLine2 = NULL;
	char *pszLine3 = NULL;
	char *pszLine4 = NULL;
	
	if (psPin == NULL || pszCardno == NULL)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	if(nMode != PINTYPE_WITHPAN)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	if (nMaxLen < 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}	
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubGetPinBlock_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	memcpy(sSendBuf, "32", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nKeyIndex;
	sSendBuf[4] = 0x00;	//0 - Master/Session, 1 - DUKPT, 2 - AES
	sSendBuf[5] = 0x00;

	nLen = strlen(pszCardno);
	if(nLen <= 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}

	memcpy(sSendBuf + 6, pszCardno, nLen);
	nLen = 6 + nLen;
	sSendBuf[nLen++] = 0x1C;
	sSendBuf[nLen++] = 0x00;
	sSendBuf[nLen++] = 0x00;
	sSendBuf[nLen++] = nMaxLen;
	sSendBuf[nLen++] = 0x00;
	sSendBuf[nLen++] = nPinpadTimeOut;
	sSendBuf[nLen++] = 0x00;

	if(pszLine1 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine1, strlen(pszLine1));
		nLen = nLen + strlen(pszLine1);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine2 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine2, strlen(pszLine2));
		nLen = nLen + strlen(pszLine2);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine3 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine3, strlen(pszLine3));
		nLen = nLen + strlen(pszLine3);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine4 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine4, strlen(pszLine4));
		nLen = nLen + strlen(pszLine4);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;

	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		if (nRet == APP_QUIT)
		{
			ProSetSecurityErrCode(ERR_PINPAD_QUIT, 0);
			return APP_FAIL;
		}
		PINPAD_TRACE_SECU("PubGetPinBlock_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "33", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubGetPinBlock_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "04", 2) == 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_QUIT,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	else if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_PINPAD_FAIL,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	if(sRecvBuf[6] == 0)
	{
		memset(psPin, 0, 8);
	}
	else
	{
		memcpy(psPin, sRecvBuf + 7, 8);
		*pnPinLen = 8;
	}
	return APP_SUCC;
}


/**
* @brief Calculate MAC
* @param [out] psMac		 MAC
* @param [in]  nMode		 MAC9606, X99, UCB
* @param [in]  psData		 Data to be calculated
* @param [in]  nDataLen 	 Data length
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubCalcMac_SP100(char *psMac, int nMode, int nKeyIndex, const char *psData, int nDataLen)
{
	int nRet = 0;
	int nLen = 0;
	char sStr[1024] = {0};
	int nStrLen;

	nStrLen = nDataLen;
	memcpy(sStr, psData, nStrLen);
	if (nStrLen%8 != 0)
	{
		nStrLen += 8 - nStrLen%8;
	}
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "8C", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nKeyIndex;
	sSendBuf[4] = 0x00;	//0 - Master/Session, 1 - DUKPT, 2 - AES
	switch(nMode)
	{
	case PINPAD_X99:
		sSendBuf[5] = 0x00;
		break;
	case PINPAD_X919:
		sSendBuf[5] = 0x01;
		break;
	case PINPAD_ECB:
		sSendBuf[5] = 0x02;
		break;
	case PINPAD_9606:
		sSendBuf[5] = 0x03;
		break;
	default:
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	sSendBuf[6] = 0x03;

	PubIntToC2(sSendBuf + 7, nStrLen);
	memcpy(sSendBuf + 9, sStr, nStrLen);
	nLen = 9 + nStrLen;
	sSendBuf[nLen] = 0x00;
	nLen++;
	sSendBuf[nLen + 1] = 0x00;
	nLen++;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubCalcMac_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "8D", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubCalcMac_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	memcpy(psMac, sRecvBuf + 6, 8);
	return APP_SUCC;
}


/**
* @brief Clear pinpad
* @param void
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubClrPinPad_SP100(void)
{
	PubDispPinPad("______Welcome_______", "__Newland__Payment__", NULL, NULL);
	return APP_FAIL; 
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
int PubDispPinPad_SP100(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4)
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "3C", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	if(pszLine1 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine1, strlen(pszLine1));
		nLen = nLen + strlen(pszLine1);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine2 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine2, strlen(pszLine2));
		nLen = nLen + strlen(pszLine2);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine3 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine3, strlen(pszLine3));
		nLen = nLen + strlen(pszLine3);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	if(pszLine4 != NULL)
	{
		memcpy(sSendBuf + nLen, pszLine4, strlen(pszLine4));
		nLen = nLen + strlen(pszLine4);
	}
	memcpy(sSendBuf + nLen, "\x1C", 1);
	nLen++;
	
	nRet = PinPad_SendRecv(0, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispPinPad_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	return APP_SUCC;
}


/**
* @brief Do DES encryption with specified 'nDesMode' key
* @param [out] psDest		 Target data
* @param [in]  psSrc		 Source data
* @param [in] nKeyIndex		 Key index
* @param [in]  nDesMode 	 Des mode
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDesPinpad_SP100(const char *psSrc, int nSrcLen, char *psDest, int nKeyIndex, int nDesMode)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "80", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nKeyIndex;

	/*
	1 = CBC Encryption
	2 = ECB Encryption 
	3 = CBC Decryption
	4 = ECB Decryption 
	*/
	switch(nDesMode)
	{
	case DESMODE_DES:
	case DESMODE_3DES:
		sSendBuf[4] = 0x02;
		break;
	case DESMODE_UNDES:
	case DESMODE_3UNDES:
		sSendBuf[4] = 0x04;
		break;
	default:
		ProSetSecurityErrCode(ERR_PINPAD_PARAM, 0);
		return APP_FAIL;
	}
	
	PubIntToC2(sSendBuf + 5, nSrcLen);
	memcpy(sSendBuf + 7, psSrc, nSrcLen);
	nLen = 7 + nSrcLen;
	sSendBuf[nLen] = 0x00;
	nLen++;
	sSendBuf[nLen + 1] = 0x00;
	nLen++;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDesPinpad_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "81", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubDesPinpad_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_CALCDES,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	memcpy(psDest, sRecvBuf + 8, nSrcLen);
	return APP_SUCC;
}

int PubGetErrCode_SP100(void)
{
	int nErrCode=0;
	char buf[80]={0};
	
	PubGetErrorCode(&nErrCode,buf, NULL);
	return nErrCode;
}

static int ProGetDeviceAttribute(const int nPort, const int nBps, char *pszDeviceAttribute, int nWaitTime)
{
	int nRet = 0;
	int nLen = 0;

	nRet = ProInitPinpadAux(nPort, nBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProGetDeviceAttribute:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "90", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;


	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPort, nWaitTime);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProGetDeviceAttribute:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "91", 2) != 0)
	{
		PINPAD_TRACE_SECU("ProGetDeviceAttribute:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	memcpy(pszDeviceAttribute, sRecvBuf + 3, nLen);
	return APP_SUCC;
}

/**
* @brief display image
* @param [out] 		
* @param [in]  
* @param [in]  
* @param [in]  
* @return 
* @li APP_FAIL Fail
* @li APP_SUCC Success
* @author chenwu
* @date 2018-3-22
*/
int PubDisplayImage_SP100(int nX, int nY, int nWidth, int nHeight, const char *psData)
{
	int nRet = 0;
	int nLen = 0;
	int nSurplus = 0;
	int nTotalPage = 0;
	int nImageDataPos = 0;
	int nCurrentPage = 0;
	int nDataLen = 0;

	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}
	
	nDataLen = nWidth*((nHeight+7)>>3);
	nTotalPage = nDataLen/256;
	if (nTotalPage == 0)
	{
		nTotalPage++;
	}
	if (nDataLen%256 != 0)
	{
		nTotalPage++;
	}
	nTotalPage --;
	nSurplus = nDataLen;
	while (1)
	{
		MEMSETSENDBUF;
		MEMSETRECVBUF;
		nLen = 0;
		memcpy(sSendBuf, "D0", 2);
		nLen += 2;
		sSendBuf[nLen] = SEPERATOR_CMD;
		nLen ++;

		PubIntToC2(sSendBuf+nLen, nX);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nY);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nWidth);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nHeight);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nCurrentPage);
		nLen += 2;
		
		PubIntToC2(sSendBuf+nLen, nTotalPage);
		nLen += 2;

		if (nTotalPage == 0)
		{
			memcpy(sSendBuf+nLen, psData+nImageDataPos, nSurplus);
			nLen += nSurplus;
		}
		else
		{
			memcpy(sSendBuf+nLen, psData+nImageDataPos, 256);
			nLen += 256;
		}
		
		nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
		if (nRet != APP_SUCC)
		{
			PINPAD_TRACE_SECU("PubCalcMac_SP100:PinPad_SendRecv APP_FAIL");
			ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
			return APP_FAIL;
		}

		if(memcmp(sRecvBuf, "D1", 2) != 0)
		{
			PINPAD_TRACE_SECU("PubDisplayImage_SP100:CMD erro");
			ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
			return APP_FAIL;
		}
		
		if(memcmp(sRecvBuf + 3, "\x00\x00", 2) != 0)
		{
			PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
			return APP_FAIL;
		}

		if (nTotalPage == 0)
		{
			break;
		}
		
		nCurrentPage ++;
		nTotalPage --;
		nSurplus -= 256;
		nImageDataPos += 256;
			
	}

	return APP_SUCC;
}

int PubClearKey_SP100(void)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "60", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "61", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubClearKey:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	return APP_SUCC;
}

int PubPinpadBeep_SP100(int nDuration, int nType)
{
	int nRet = 0;
	int nLen = 0;
	int nBcdLen = 2;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "39", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	sSendBuf[nLen++] = nType;
	PubIntToBcd((char *)sSendBuf+nLen, &nBcdLen, nDuration);
	nLen += 2;
	
	nRet = PinPad_SendRecv(0, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubPinpadBeep_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}


int PubCancelPIN_SP100()
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "38", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubPinpadBeep_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int PubGenAndShowQr_SP100(int nVersion, char *pszBuffer)
{
	int nRet = 0;
	int nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "D2", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 1;//adjust level
	sSendBuf[nLen++]= 0;//mask no
	sSendBuf[nLen++]= nVersion;//version
	sSendBuf[nLen++]= 1;//Auto Centering
	//sSendBuf[nLen++]= 0;//x
	//sSendBuf[nLen++]= 0;//y
	sSendBuf[nLen++]= 1;//title
	sSendBuf[nLen++]= 0;//title length
	PubIntToC2(sSendBuf+nLen, strlen(pszBuffer));
	nLen += 2;
	memcpy(sSendBuf+nLen, pszBuffer, strlen(pszBuffer));
	nLen += strlen(pszBuffer);
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubPinpadBeep_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "D3", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubLoadClearKey_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	//the spec shows the BCD 00.
	if(memcmp(sRecvBuf + 3, "\x00\x00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}

	return APP_SUCC;
}

static int PubScanStart()
{
	int nRet = 0;
	uint nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "G0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0x01;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan start");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "G1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	PubDebug("#### scan start ok");
	return APP_SUCC;
}

static int PubScanGet(char *pszBuf)
{
	uint nLen = 0;
	int nRet = 0;
	int nDataLen = 0;
	int nKey;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}
GETTT:
	nLen = 0;
	MEMSETSENDBUF;
	MEMSETRECVBUF;
	NAPI_KbHit(&nKey);
	if(nKey == KEY_ESC)
	{
		return APP_FAIL;
	}
	
	memcpy(sSendBuf, "G0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0x02;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan get");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "G1", 2) != 0)
	{
		PINPAD_TRACE_SECU("get:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		if (memcmp(sRecvBuf + 4, "01", 2) == 0)
		{
			PubDebug("#### get again");
			PubSysDelay(1);
			goto GETTT;
		}
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("get failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nLen = 6;
	PubC2ToInt((uint *)&nDataLen, sRecvBuf + nLen);
	nLen += 2;
	memcpy(pszBuf, sRecvBuf + nLen, nDataLen);
	PubDebug("#### scan get ok [%s]", pszBuf);

	return APP_SUCC;
}

static int PubScanStop()
{
	int nRet = 0;
	uint nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScanStop_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;

	memcpy(sSendBuf, "G0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0x03;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan stop");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "G1", 2) != 0)
	{
		PINPAD_TRACE_SECU("stop:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("stop failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}

	PubDebug("#### scan stop ok ");

	return APP_SUCC;
}

int PubDoScan_SP100(char *pszBuf)
{
	int nRet = 0;

	if (PubScanStart() == APP_SUCC)
	{
		nRet = PubScanGet(pszBuf);
		nRet += PubScanStop();
	}
	else
	{
		return APP_FAIL;
	}
	
	return nRet;
}

// sign
static int ProEsignStart()
{
	int nRet = 0;
	uint nLen = 0;
	int nPos = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA0;
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan start");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nPos += 5;
	//state
	PubDebugData("state: ", sRecvBuf+nPos, 1);
	nPos ++;
	//version
	PubDebugData("version: ", sRecvBuf+nPos, 6);
	nPos += 6;
	//sn
	PubDebugData("sn: ", sRecvBuf+nPos, 16);
	//flag

	return APP_SUCC;
}

static int ProEsignCheck(char *psNum)
{
	int nRet = 0;
	uint nLen = 0;
	int nPos = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA1;
	memcpy(sSendBuf+nLen, psNum, 3);//BCD
	nLen += 3;
	sSendBuf[nLen++] = 0x01; //state
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("EsignCheck");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nPos += 5;
	//state
	PubDebugData("state: ", sRecvBuf+nPos, 1);

	return APP_SUCC;
}

static int SaveToFile(const char *psInBuf,int nOff, int nLen, const char *pszFileName)
{
	int fp;

	if((fp=PubFsOpen(pszFileName,"w"))<0)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	
	if(PubFsSeek (fp,(long)nOff,SEEK_SET)<0)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	
	if(PubFsWrite(fp, psInBuf,nLen)<0)
	{
		PubFsClose(fp);
		return APP_FAIL;
	}
	PubFsClose(fp);	
	return APP_SUCC;
}

static int ProEsignTransfer(char *pszCharaterCode, char *pszSignName, int nTimeOut)
{
	int nRet = 0;
	int nLen = 0;
	int nPos = 0;
	char szTimeOut[2+1] = {0};
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA2;
	sSendBuf[nLen++]= 0x3C;
	PubIntToC2((uchar *)szTimeOut, (uint)nTimeOut);
	memcpy(sSendBuf+ nLen, szTimeOut, 2);
	nLen += 2;
	if (pszCharaterCode != NULL)
	{
		memcpy(sSendBuf + nLen, pszCharaterCode, 8);
	}
	nLen += 8;

	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("nRet = %d", nRet);
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}
	nPos += 6;
	//num
	PubDebugData("num: ", sRecvBuf+nPos, 3);
	nPos+=3;
	if (pszSignName != NULL)
	{
		SaveToFile((char *)sRecvBuf+nPos, 0, nLen-nPos, pszSignName);
	}
	else
	{
		SaveToFile((char *)sRecvBuf+nPos, 0, nLen-nPos, SIGNFILENAME);
	}
	
	PubDebug("save ok: %d", nLen-nPos);

	return APP_SUCC;
}


static int ProEsignFinish()
{
	int nRet = 0;
	uint nLen = 0;
 	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubClearKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "S0", 2);
	nLen += 2;
	sSendBuf[nLen++]= SEPERATOR_CMD;
	sSendBuf[nLen++]= 0xA3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("scan start");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf, "S1", 2) != 0)
	{
		PINPAD_TRACE_SECU("star:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 4, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("start failed [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}

	return APP_SUCC;
}

int PubEsignature_SP100(char *pszCharaterCode, char *pszSignName, int nTimeOut)
{
	int nRet = 0;
	char sNum[6] = "\x00\x00\x01";
	
	nRet = ProEsignStart();
	nRet = ProEsignTransfer(pszCharaterCode, pszSignName, nTimeOut);
	nRet += ProEsignCheck(sNum);
	nRet += ProEsignFinish();

	return nRet;
}


int PubCalcKcv_SP100( int nKeyIndex,int nKeyType, char *psKcv)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("ProLoadKey_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "52", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = nKeyIndex;
	switch(nKeyType)
	{
	case KEY_TYPE_TMK:
		sSendBuf[4] = 0;
		break;		
	case KEY_TYPE_PIN:
		sSendBuf[4] = 1;
		break;
	case KEY_TYPE_MAC:
		sSendBuf[4] = 2;
		break;
	case KEY_TYPE_DATA:
		sSendBuf[4] = 3;
		break;
	default:
		return APP_FAIL;
	}

	nLen = 5;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubCalcKcv_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "53", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubCalcKcv_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	memcpy(psKcv, sRecvBuf + 4, 3);
	return APP_SUCC;
}









int PubSetFontColor_SP100( ushort usColor, char cObject)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SetFontColor:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "42", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	PubIntToC2(&sSendBuf[3], usColor);
	//memcpy(&sSendBuf[3], &usColor, 2);
	sSendBuf[5] = cObject;


	nLen = 6;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubSetFontColor_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	return APP_SUCC;
}



int PubLoadImg_SP100(uchar ucImgID, int nImgLen, char *psImgData)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubLoadImg:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "3E", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = ucImgID;
	PubIntToC2(sSendBuf+4, nImgLen);
	memcpy(sSendBuf+6,psImgData, nImgLen);


	nLen = 6 + nImgLen;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubLoadImg_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf, "3F", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubLoadImg_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}
	if(memcmp(sRecvBuf + 3, "00", 2) != 0)
	{
		ProSetSecurityErrCode(ERR_GETMAC,nRet);
		PINPAD_TRACE_SECU("Response Code [%2.2s]", sRecvBuf + 3);
		return APP_FAIL;
	}	
	return APP_SUCC;

}


int PubDispImg_SP100(uchar ucImgID, int nX, int nY)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "3D", 2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = ucImgID;
	PubIntToC2(sSendBuf+4, nX);
	PubIntToC2(sSendBuf+6, nY);


	nLen = 8;
	
	nRet = PinPad_SendRecv(0, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	return APP_SUCC;

}




int PubScrClrs_SP100(void)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "4e",2);
	sSendBuf[2] = SEPERATOR_CMD;
	nLen = 3;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "4f", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}







int PubSetAlignMode_SP100(char cMode)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubSetAlignMode:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "5c",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = cMode;
	nLen = 4;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubSetAlignMode_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "5d", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubSetAlignMode_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}


int PubSetClrsMode_SP100(char cMode)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SetClrsMode:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "d6",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = cMode;
	nLen = 4;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "d7", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("PubScrClrs_SP100:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}




int PubSetFontSize_SP100(char cSize)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;
	
	memcpy(sSendBuf, "5a",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = cSize;
	nLen = 4;
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("SetFontSize:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "5b", 2) != 0)
	{
		PINPAD_TRACE_SECU("SetFontSize:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("SetFontSize:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	
	return APP_SUCC;

}



int PubCreateFile_SP100(char *pszFileName)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;


	memcpy(sSendBuf, "46",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = strlen(pszFileName);
	strcpy((char *)sSendBuf+4, pszFileName);
	nLen = 4 + strlen(pszFileName);
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("Create File:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "47", 2) != 0)
	{
		PINPAD_TRACE_SECU("Create File:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("Create File:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	


	return APP_SUCC;	
}


int PubLoadFile_SP100(char *pszPath)
{
	int nRet = 0;
	int nLen = 0;
	int nGotLen = 0;
	int nBlockLen = 1024;
	
	FILE *fp = NULL;

	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("PubDispImg_SP100:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;


	memcpy(sSendBuf, "48",2);
	sSendBuf[2] = SEPERATOR_CMD;
	


	fp = fopen(pszPath,"r");
	if(fp == NULL)
	{
		return APP_FAIL;
	}
	
	//fseek(fp, 0, SEEK_END);
	//nFileLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	

	while(1)
	{

		nGotLen = fread(sSendBuf+5,1,nBlockLen,fp);
		if(nGotLen < 0)
		{
			fclose(fp);
			PINPAD_TRACE_SECU("Fread Error");
			ProSetSecurityErrCode(ERR_PINPAD_FAIL, 0);
			return APP_FAIL;			
		}
		else if(nGotLen == 0)//fread finished
		{
			break;
		}
		else 
		{
			PubIntToC2(sSendBuf+3, nGotLen);
			nLen = 5 + nGotLen;
			nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
			if (nRet != APP_SUCC)
			{
				fclose(fp);
				PINPAD_TRACE_SECU("Load File:PinPad_SendRecv APP_FAIL");
				ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
				return APP_FAIL;
			}
			
			
			if(memcmp(sRecvBuf, "49", 2) != 0)
			{
				fclose(fp);
				PINPAD_TRACE_SECU("Load File:CMD erro");
				ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
				return APP_FAIL;
			}
			
			if(memcmp(sRecvBuf+3, "00", 2) != 0)
			{
				fclose(fp);
				PINPAD_TRACE_SECU("Load File:Response Code Erro");
				ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
				return APP_FAIL;
			}

		
			if(nGotLen != nBlockLen) //fread finished
				break;
		}
		
		
	}

	

	fclose(fp);
	return APP_SUCC;	
}






int PubUpdateFile_SP100(char *pszFileName, char cRebootFlag)
{
	int nRet = 0;
	int nLen = 0;
	
	nRet = ProInitPinpadAux(nPinpadPort, nPinpadBps);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("UpdateFile:ProInitPinpadAux APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_INITAUX, 0);
		return APP_FAIL;
	}

	MEMSETSENDBUF;
	MEMSETRECVBUF;


	memcpy(sSendBuf, "44",2);
	sSendBuf[2] = SEPERATOR_CMD;
	sSendBuf[3] = strlen(pszFileName);
	strcpy((char *)sSendBuf+4, pszFileName);
	sSendBuf[4+strlen(pszFileName)] = cRebootFlag;
	nLen = 4 + strlen(pszFileName) + 1;
	
	
	nRet = PinPad_SendRecv(1, sSendBuf, nLen, sRecvBuf, (uint *)&nLen, nPinpadPort, nPinpadTimeOut);
	if (nRet != APP_SUCC)
	{
		PINPAD_TRACE_SECU("Update File:PinPad_SendRecv APP_FAIL");
		ProSetSecurityErrCode(ERR_PINPAD_SENDEXRF, 0);
		return APP_FAIL;
	}


	if(memcmp(sRecvBuf, "45", 2) != 0)
	{
		PINPAD_TRACE_SECU("Update File:CMD erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}

	if(memcmp(sRecvBuf+3, "00", 2) != 0)
	{
		PINPAD_TRACE_SECU("Update File:Response Code Erro");
		ProSetSecurityErrCode(ERR_PINPAD_RECEIVEEXRF, 0);
		return APP_FAIL;
	}	


	return APP_SUCC;	
}

/* End of pindpad.c */


