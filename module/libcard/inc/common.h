#ifndef _COMMON_H_
#define _COMMON_H_

#define CARDLIBVER "ALCARD0117052301"

#define READER_MAX_DATA_LEN 240

#define	SEPERATOR_CMD	( 0x2F )

#define	BPS300		0
#define	BPS1200		1
#define	BPS2400		2
#define	BPS4800		3
#define	BPS7200		4
#define	BPS9600		5
#define	BPS19200	6
#define	BPS38400	7
#define	BPS57600	8
#define	BPS76800	9
#define	BPS115200	10


#define FIAL_READCARD_LIB_BASE				(-4000)
#define FAIL_READCARD_INVAILIDPARAM		    (FIAL_READCARD_LIB_BASE - 1)
#define FAIL_READCARD_INIT					(FIAL_READCARD_LIB_BASE - 2)
#define FAIL_READCARD_WRITE				    (FIAL_READCARD_LIB_BASE - 3)
#define FAIL_READCARD_READ					(FIAL_READCARD_LIB_BASE - 4)
#define FAIL_READCARD_TIMEOUT				(FIAL_READCARD_LIB_BASE - 5)
#define FAIL_READCARD_RXOOVER				(FIAL_READCARD_LIB_BASE - 6)

#define ERR_RFIDCARDLIB_BASE		(-4100)
#define ERR_INITPORT_FAIL		(ERR_RFIDCARDLIB_BASE - 1)
#define ERR_UNKNOWREADER_FAIL	(ERR_RFIDCARDLIB_BASE - 2)
#define ERR_USERCANCEL_FAIL		(ERR_RFIDCARDLIB_BASE - 3)
#define ERR_NO_RECVDATA			(ERR_RFIDCARDLIB_BASE - 4)
#define ERR_RECVDATA_LEN			(ERR_RFIDCARDLIB_BASE - 5)
#define ERR_RECVDATA_LRC		(ERR_RFIDCARDLIB_BASE - 6)
#define ERR_SUPPORT_CARD		(ERR_RFIDCARDLIB_BASE - 7)
#define ERR_SUPPORT_DOUBLE_RF	(ERR_RFIDCARDLIB_BASE - 8)
#define ERR_INITINSIDE_FAIL		(ERR_RFIDCARDLIB_BASE - 9)
#define ERR_INITOUTSIDE_FAIL		(ERR_RFIDCARDLIB_BASE - 10)
#define ERR_READERPARAM_FAIL	(ERR_RFIDCARDLIB_BASE - 11)
#define ERR_FUN_POINT_NULL_FAIL	(ERR_RFIDCARDLIB_BASE - 12)
#define ERR_INITOUTSIDE_SH_FAIL		(ERR_RFIDCARDLIB_BASE - 13)
#define ERR_INITOUTSIDE_ZX_FAIL		(ERR_RFIDCARDLIB_BASE - 14)
#define ERR_SEEKCARD_SH_FAIL		(ERR_RFIDCARDLIB_BASE - 15)
#define ERR_GETCONNECT_SH_FAIL		(ERR_RFIDCARDLIB_BASE - 16)

void Card_IntToC2 (unsigned char* pszBuf, const unsigned int nNum );
int Card_GetVarReaderPort(int *pnPort);
int Card_SetVarReaderPort(int nPort);
int Card_SetVarInitReader(char cInit);
int Card_SetVarFirst(char cFirst);
int Card_GetVarFirst(char *pcFirst);
int Card_SetVarReaderTimeOut(int nTimeOut);
void Card_LibDebug(const char *pszFile, const char *pszFunc, const int nLine, char *pszBufer, int nLen, char* lpszFormat, ...);
void Card_SetErrorCode(int nErrCode, const char* pFileName, const char* pFunName, int nLin);
int Card_InitReaderAux(const int nPort, const int nBps);
int Card_RfReader_CommByAsyn(const char * pszData,int nLen,char * pszOutBuf,int * pnOutLen);
int Reader_SendRecv(unsigned char *psSend, unsigned int nSendLen, unsigned char *psRecv, unsigned int *pnRecvLen);

#endif/*End of common.h*/

