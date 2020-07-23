/***************************************************************************
** Copyright (c) 2019 Newland Payment Technology Co., Ltd All right reserved   
** File name:  common.c
** File indentifier: 
** Brief:  common module for reader
** Current Verion:  v1.0
** Auther: Liugang
** Complete date: 2017-9-1
** Modify record: 
** Modify date: 
** Version: 
** Modify content: 
***************************************************************************/
#include <stdarg.h>
#include "lui.h"
#include "ltool.h"
#include "common.h"
#include "pinpad.h"
#include "libapiinc.h"
#include "napi_kb.h"

static int gnReaderPort = PINPAD;
static int gnReaderTimeOut = 5;
static char cInitReader = APP_SUCC;	
static char gcFirst = 0;


int Card_InitReaderAux(const int nPort, const int nBps)
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



void Card_SetErrorCode(int nErrCode, const char* pFileName, const char* pFunName, int nLin)
{
	int i;
	typedef struct
	{
		int nErrCOde;
		char szErrContent[48];
	}STCARDLIBERR;
	STCARDLIBERR stCardLibErr[] = {
		{FAIL_READCARD_INVAILIDPARAM, "Parameter error"},
		{ERR_INITPORT_FAIL, "Init aux error"},
		{ERR_UNKNOWREADER_FAIL, "Unknown RF reader module type"},
		{ERR_USERCANCEL_FAIL, "Press (Esc) when the reader is detected"},
		{ERR_NO_RECVDATA, "Not receiving data from the aux"},
		{ERR_RECVDATA_LEN, "From the aux to receive data length error"},
		{ERR_RECVDATA_LRC, "Received data LRC error"},
		{ERR_SUPPORT_CARD, "Card type error"},
		{ERR_SUPPORT_DOUBLE_RF, "Not support dual frequency"},
		{ERR_INITINSIDE_FAIL, "Failed to initialize the card reader"},
		{ERR_INITOUTSIDE_FAIL, "External reader init failed"},
		{ERR_READERPARAM_FAIL, "Parameter error"},
		{ERR_FUN_POINT_NULL_FAIL, "Function pointer is not initialized"},
		{ERR_INITOUTSIDE_SH_FAIL, "SH external reader init failed"},
		{ERR_INITOUTSIDE_ZX_FAIL, "ZX external reader init failed"},	
		{ERR_SEEKCARD_SH_FAIL, "SH reader card searching failure"},
		{ERR_GETCONNECT_SH_FAIL, "SH card reader connection failure data"},
		{FIAL_READCARD_LIB_BASE , "Unknown error code"}
	};

	for( i=0; i<17; i++ )
	{
		if( nErrCode == stCardLibErr[i].nErrCOde )
		{
			Card_LibDebug(pFileName, pFunName, nLin, NULL, 0, "nErrCode=%d,disp=%s", stCardLibErr[i].nErrCOde, stCardLibErr[i].szErrContent);				
			PubSetErrorCode(stCardLibErr[i].nErrCOde, stCardLibErr[i].szErrContent, 0);
			return;
		}
	}
	if( i >= 17 )
	{
		stCardLibErr[i].nErrCOde = nErrCode;
		Card_LibDebug(pFileName, pFunName, nLin, NULL, 0, "nErrCode=%d,disp=%s", stCardLibErr[i].nErrCOde, "Unknown erro");		
		PubSetErrorCode(stCardLibErr[i].nErrCOde, "Unknown error code", 0);
	}

	return;
}

static void C2ToInt(unsigned int *nNum, unsigned char *pszBuf)
{
	*nNum = ((*pszBuf) << 8) + (*(pszBuf + 1));
}

void Card_IntToC2 (unsigned char* pszBuf, const unsigned int nNum )
{
	int nTmp = nNum;
	if (nNum <= 65535)
	{
		*(pszBuf + 1) = nNum % 256;
		*pszBuf = nTmp >>8;
	}
	return ;
}

static int Reader_InsertAsyn(unsigned char *psBuf, unsigned int *punLen)
{
	char sTemp[READER_MAX_DATA_LEN*2+10];
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

static int Reader_DeleteAsyn(unsigned char *psBuf, unsigned int *punLen)
{
	int nLen = 0;
	char cLRC = 0;
	char sTemp[512];
	
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

int Reader_SendRecv(uchar *psSend, uint nSendLen, uchar *psRecv, uint *pnRecvLen)
{
	int i;
	int nRet;
	uint nLen;
	int nLen2;
	char sSendBuf[1024] = {0};
	char sRecvBuf[1024] = {0};
	uint unTimeOut=0;
	int nCode=0;
	int nSumLen=0;
	
	memcpy(sSendBuf, psSend, nSendLen);
	nLen = nSendLen;
	nRet = Reader_InsertAsyn((uchar *)sSendBuf, &nLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	if(NAPI_PortFlush(gnReaderPort) != NAPI_OK)
	{
		return APP_FAIL;
	}
	PubDebugData("Reader_SendRecv send: ", sSendBuf, nLen);
	unTimeOut = PubGetOverTimer(gnReaderTimeOut * 1000);
	NAPI_KbFlush();
	
	for(i = 0; i < 3; i++)
	{
		nRet = NAPI_PortWrite(gnReaderPort, (uchar *)sSendBuf, nLen);
		if (nRet != NAPI_OK)
		{
			PubDebug("[%s][%d]write error. nRet = %d", __func__, __LINE__, nRet);
			return APP_FAIL;
		}

		//for quit immediately when canceled, oherwise it will quit delay 3 seconds
		if (memcmp(psSend, "3C", 2) == 0)
		{
			return APP_SUCC;
		}
		
		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
			return APP_QUIT;
		}

		nSumLen = 1;
		nRet = NAPI_PortRead(gnReaderPort, (uchar *)sRecvBuf, (int *)&nSumLen, 3 * 1000);
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
			Card_SetErrorCode(FAIL_READCARD_READ, __FILE__, __FUNCTION__, __LINE__);
			return APP_FAIL;
		}
	}
	if(i == 3)
	{
		Card_SetErrorCode(FAIL_READCARD_READ, __FILE__, __FUNCTION__, __LINE__);
		return APP_FAIL;
	}

	//This command does not receive a response message; the PIN pad response consists of just the single [ACK] character.
	//if(cFlag == 0)
	//{
	//	return APP_SUCC;
	//}
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
			return APP_QUIT;
		}

		nLen2 = 1;
		nRet = NAPI_PortRead(gnReaderPort, (uchar *)sRecvBuf, &nLen2, 1000);
		nSumLen += nLen2;

		if (PubTimerIsOver(unTimeOut) == APP_SUCC && nSumLen != 1)
		{
			Card_SetErrorCode(FAIL_READCARD_TIMEOUT, __FILE__, __FUNCTION__, __LINE__);
			return APP_FAIL;
		}
		if (nSumLen == 1 || nRet == APP_SUCC)
		{
			break;
		}
	}
	
	if(sRecvBuf[0] != STX)
	{
		PubDebug("[%s][%d]sRecvBuf[0]:%d", __func__, __LINE__, sRecvBuf[0]);
		return APP_FAIL;
	}
	nSumLen=0;

	NAPI_KbFlush();	
	unTimeOut = PubGetOverTimer(gnReaderTimeOut * 1000);
	while(1)
	{
		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
			return APP_QUIT;
		}

		nLen2 = 2;
		nRet = NAPI_PortRead(gnReaderPort, (uchar *)sRecvBuf + 1, &nLen2, 1000);
		nSumLen += nLen2;

		if(PubTimerIsOver(unTimeOut)==APP_SUCC && nSumLen != 2)
		{
			Card_SetErrorCode(FAIL_READCARD_TIMEOUT, __FILE__, __FUNCTION__, __LINE__);
			return APP_FAIL;
		}
		if(nSumLen == 2 || nRet == APP_SUCC)
		{
			break;
		}
	}
	nSumLen=0;
	NAPI_KbFlush();	
	unTimeOut = PubGetOverTimer(gnReaderTimeOut*1000);
	while(1)
	{
		PubBcdToInt((char *)sRecvBuf + 1, &nLen2);

		NAPI_KbHit(&nCode);
		if(nCode == KEY_ESC)
		{
	//		ProSetSecurityErrCode(ERR_PINPAD_QUIT, 0);
			return APP_QUIT;
		}

		nLen= nLen2 + 2;
		nRet = NAPI_PortRead(gnReaderPort, (uchar *)sRecvBuf + 3, (int *)&nLen, 1000);
		nSumLen += nLen;

		if (PubTimerIsOver(unTimeOut)==APP_SUCC && nSumLen != nLen2 + 2)
		{
			Card_SetErrorCode(FAIL_READCARD_TIMEOUT, __FILE__, __FUNCTION__, __LINE__);
			return APP_FAIL;
		}
		if ((nSumLen == nLen2 + 2) || (nRet == APP_SUCC))
		{
			nLen = nLen2 + 5;
			break;
		}
	}

	sSendBuf[0] = ACK;
	NAPI_PortWrite(gnReaderPort, (uchar *)sSendBuf, 1);
	
	nRet = Reader_DeleteAsyn((uchar *)sRecvBuf, &nLen);
	PubDebugData("Reader_SendRecv recv: ", sRecvBuf, nLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	memcpy(psRecv, sRecvBuf, nLen);
	*pnRecvLen = nLen;	
	return APP_SUCC;
}

int Card_RfReader_CommByAsyn(const char * pszData,int nLen,char * pszOutBuf,int * pnOutLen)
{
	int nRet = 0;
	int nDataLen = 0, nRececeLen = 0;
	char szCommand[512];

	NAPI_PortFlush (gnReaderPort);
	memset(szCommand, 0, sizeof(szCommand));
	memcpy(szCommand, pszData, nLen);
	nDataLen = nLen;
	nRet = Reader_InsertAsyn((unsigned char *)szCommand, (uint *)&nDataLen);
	if (nRet != APP_SUCC)
	{
		return APP_FAIL;
	}
	nRet = NAPI_PortWrite(gnReaderPort, (uchar *)szCommand, nDataLen);
	if(nRet != NAPI_OK )
	{
		Card_SetErrorCode(ERR_USERCANCEL_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return APP_QUIT;
	}
	
	memset(szCommand,0,sizeof(szCommand));
	nRececeLen = 1;
	nRet = NAPI_PortRead(gnReaderPort, (uchar *)szCommand, &nRececeLen, gnReaderTimeOut);
	if(nRet != NAPI_OK )
	{	
		{
			Card_SetErrorCode(ERR_NO_RECVDATA, __FILE__, __FUNCTION__, __LINE__);
			return APP_TIMEOUT;
		}
	}
	if(szCommand[0] != 0x02)
	{
		Card_SetErrorCode(ERR_UNKNOWREADER_FAIL, __FILE__, __FUNCTION__, __LINE__);
		return APP_FAIL;
	}
	nRececeLen = 2;
	nRet = NAPI_PortRead(gnReaderPort, (uchar *)szCommand+1, &nRececeLen, 1000);
	if(nRececeLen != 2 || nRet != NAPI_OK )
	{
		{
			Card_SetErrorCode(ERR_NO_RECVDATA, __FILE__, __FUNCTION__, __LINE__);
			return APP_TIMEOUT;
		}
	}
	C2ToInt((unsigned int *)&nDataLen, (unsigned char *)szCommand+1);
	nRececeLen= nDataLen+2;
	nRet = NAPI_PortRead(gnReaderPort, (uchar *)szCommand+3, &nRececeLen, 1000);
	if(nRececeLen != (nDataLen+2) || nRet != NAPI_OK )
	{
		Card_SetErrorCode(ERR_RECVDATA_LEN, __FILE__, __FUNCTION__, __LINE__);
		return APP_TIMEOUT;
	}

	nDataLen += 5;
	nRet = Reader_DeleteAsyn((uchar *)szCommand, (uint *)&nDataLen);
	if(nRet != APP_SUCC)
	{	
		Card_SetErrorCode(ERR_RECVDATA_LRC, __FILE__, __FUNCTION__, __LINE__);
		return APP_FAIL;
	}
	*pnOutLen = nDataLen;
	memcpy(pszOutBuf, szCommand, nDataLen);
	return APP_SUCC;
}

int Card_GetVarReaderPort(int *pnPort)
{
	*pnPort = gnReaderPort;
	return APP_SUCC;
}
int Card_SetVarReaderPort(int nPort)
{
	gnReaderPort = nPort;
	return APP_SUCC;
}
int Card_SetVarInitReader(char cInit)
{
	cInitReader = cInit;
	return APP_SUCC;
}
int Card_SetVarFirst(char cFirst)
{
	gcFirst = cFirst;
	return APP_SUCC;
}
int Card_GetVarFirst(char *pcFirst)
{
	*pcFirst = gcFirst;
	return APP_SUCC;
}

int Card_SetVarReaderTimeOut(int nTimeOut)
{
	gnReaderTimeOut = nTimeOut;
	return APP_SUCC;
}

void Card_LibDebug(const char *pszFile, const char *pszFunc, const int nLine, char *pszBufer, int nLen, char* lpszFormat, ...)
{
	va_list args;
	char szTitle[100+1];
	char buf[2048];

	memset(szTitle, 0, sizeof(szTitle));
	sprintf(szTitle, "[CARD LIB][%s][%s][%d]>>>", pszFile, pszFunc, nLine);

	if( lpszFormat!=NULL )
	{
		va_start(args, lpszFormat);
		vsprintf(buf, lpszFormat, args);
		PubDebug("%s%s",szTitle, buf);	
		va_end(args);
	}
	if( pszBufer!=NULL && nLen!=0 && nLen<2048)
	{
		PubDebugData(szTitle, pszBufer, nLen);
	}
}
/*End of common.c*/

