#ifndef _PINPADHY_H_
#define _PINPADHY_H_

#define SIGNFILENAME "sign.png"

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



#define PINPAD_9606	0
#define PINPAD_X99	1
#define PINPAD_ECB	2
#define PINPAD_X919	3

extern int PubResetPinpad_SP100(const int nPort, const int nTimeOut);
extern int PubLoadKey_SP100(int nKeyType, int nGroupNo, const char *psKeyValue, int nKeyLen, char* psCheckValue);
extern int PubGetPinBlock_SP100(char *pszPin, int *pnPinLen, int nMode, int nKeyIndex, const char *pszCardno, int nPanLen, int nMaxLen);
extern int PubCalcMac_SP100(char *psMac, int nMode, int nKeyIndex, const char *psData, int nDataLen);
extern int PubClrPinPad_SP100(void);
extern int PubDispPinPad_SP100(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4);
extern int PubDesPinpad_SP100(const char *psSrc, int nSrcLen, char *psDest, int nKeyIndex, int nDesMode);
extern int PubGetPinpadVer_SP100(char *pszVer);
extern int PubGetErrCode_SP100(void);
extern int PubReadString_SP100(char *pstString, int *pnStringLen, int nMaxLen, int nMinLen);
extern int PubClearKey_SP100(void);
extern int PubPinpadBeep_SP100(int nDuration, int nType);
extern int PubGenAndShowQr_SP100(int nVersion, char *pszBuffer);
extern int PubCancelPIN_SP100(void);
extern int PubDoScan_SP100(char *pszBuffer);
extern int PubEsignature_SP100(char *pszCharaterCode, char *pszSignName, int nTimeOut);
extern int PubInjectKey_SP100(int nKeyType, int nSrcIndex, int nDstIndex,const char *psKeyValue, int nKeyLen, char *psCheckValue);
extern int PubCalcKcv_SP100( int nKeyIndex,int nKeyType, char *psKcv);
extern int PubSetFontColor_SP100(unsigned short usColor, char cObject);
extern int PubLoadImg_SP100(unsigned char ucImgID, int nImgLen, char *psImgData);
extern int PubDispImg_SP100(unsigned char ucImgID, int nX, int nY);
extern int PubSetAlignMode_SP100(char cMode);
extern int PubSetClrsMode_SP100(char cMode);
extern int PubSetFontSize_SP100(char cSize);
extern int PubCreateFile_SP100(char *pszFileName);
extern int PubLoadFile_SP100(char *pszPath);
extern int PubUpdateFile_SP100(char *pszFileName, char cRebootFlag);

#endif

/**< End of lpindpad.h */
