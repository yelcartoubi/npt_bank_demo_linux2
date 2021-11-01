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

#define PINPAD_X99	0
#define PINPAD_X919	1
#define PINPAD_ECB	2
#define PINPAD_9606	3
#define PINPAD_AES	5



typedef struct {
	int nIndex; // Key Index used for TDK protection,129..255. 0 - clear
	int nKeyType; // 0 ¡§C DES 1 ¡§C AES
	int nEncryMode; // 0-DES_CBC  1-DES_ECB   2-AES
} STREADCARD_IN;

typedef struct {
	int nPanLen;
	char szCipherPan[32+1];
	char szMaskPan[19+1];
	int nTrack2Len;
	char szTrack2[53+1];
	int nTrack3Len;
	char szTrack3[120+1];
} STREADCARD_OUT;

typedef struct {
	char cMsgType;
	char szReq[2+1];
	char szRsp[2+1];
	char cFuncId;
	char szMsgName[32+1];
} STPINPADMSGTYPE;

typedef enum {
	FUNCID_SETTERMINALCFG		= 0x01,
	FUNCID_GETTERMINALCFG		= 0x02,
	FUNCID_UPDATEAID			= 0x03,
	FUNCID_GETAIDCFG			= 0x04,
	FUNCID_RMSPECAID			= 0x05,
	FUNCID_RMALLAID 			= 0x06,
	FUNCID_UPDATECAPK			= 0x07,
	FUNCID_GETCAPK				= 0x08,
	FUNCID_RMSPECCAPK			= 0x09,
	FUNCID_RMALLCAPK			= 0x0A,
	FUNCID_UPDATECERTLIST		= 0x0B,
	FUNCID_GETCERTLIST			= 0x0C,
	FUNCID_RMSPECCERTLIST		= 0x0D,
	FUNCID_RMALLCERTLIST		= 0x0E,
	FUNCID_UPDATEEXCEPTIONLIST	= 0x0F,
	FUNCID_GETEXCEPTIONLIST 	= 0x10,
	FUNCID_RMSPECEXCEPTIONLIST	= 0x11,
	FUNCID_RMALLEXCEPTIONLIST	= 0x12,
	FUNCID_GETAIDNUM			= 0x13,
	FUNCID_GETCAPKNUM			= 0x14,
	FUNCID_OUTPUTDEBUG			= 0X21,
	FUNCID_INITL3				= 0x22,
	FUNCID_SETTLVDATA			= 0x23,
	FUNCID_GETL3DATA			= 0x24,
	FUNCID_SETTLVLIST			= 0x25,
	FUNCID_GETTLVLIST			= 0x26,
	FUNCID_SETDEBUGMODE 		= 0x27,
	FUNCID_GETKERNELVER 		= 0x28,
	FUNCID_PERFORMTRANS 		= 0x31,
	FUNCID_COMPLETETRANS		= 0x32,
	FUNCID_TERMINATE			= 0x33,
	FUNCID_L3CALLBACK			= 0x36,
} L3_FUNCID;

typedef enum {
	PINPAD_SETTERMINALCFG,               /**< set Terminal configuration. */
	PINPAD_GETTERMINALCFG,               /**< get Terminal configuration. */
	PINPAD_UPDATEAID,		             /**< Update aid configuration */
	PINPAD_GETAIDCFG,                    /**< Get AID Configuration */
	PINPAD_RMSPECAID,		             /**< Remove the specified AID configuration. */
	PINPAD_RMALLAID,		             /**< Remove all Terminal/AID configuration */
	PINPAD_UPDATECAPK,		             /**< Update CAPK */
	PINPAD_GETCAPK,		                 /**< Update CAPK */
	PINPAD_RMSPECCAPK, 	                 /**< Update CAPK */
	PINPAD_RMALLCAPK,		             /**< Remove all CAPK configuration */
	PINPAD_GETAIDNUM,                    /**< Get Aid Num */
	PINPAD_GETCAPKNUM,                    /**< Get Capk Num */
	PINPAD_UPDATECERTLIST,               /**< Update certification revocation list */
	PINPAD_GETCERTLIST,                  /**< Get certification revocation list */
	PINPAD_RMSPECCERTLIST,               /**< Remove One certification revocation list */
	PINPAD_RMALLCERTLIST,                /**< Remove ALL certification revocation list */
	PINPAD_UPDATEEXCEPTIONLIST,          /**< Update exception list */
	PINPAD_GETEXCEPTIONLIST,             /**< GET exception list */
	PINPAD_RMSPECEXCEPTIONLIST,          /**< Remove One exception list. */
	PINPAD_RMALLEXCEPTIONLIST,           /**< Remove One exception list. */
	PINPAD_INITL3,                       /**< initializes Newland Level3 module */
	PINPAD_SETTLVDATA,                   /**< set a TLV data by the tag name */
	PINPAD_GETL3DATA,                    /**< get the EMVL3 data according L3_DATA type */
	PINPAD_SETTLVLIST,                   /**< set one or more TLV data by the tag name */
	PINPAD_GETTLVLIST,                   /**< get the TLV list data value from current kernel. */
	PINPAD_SETDEBUGMODE,                 /**< set the L2 kernel Debug mode */
	PINPAD_GETKERNELVER,                 /**< get the kernel version */
	PINPAD_CANCELREADCARD,               /**< Cancel read card */
	PINPAD_PERFORMTRANS,                 /**< performed a transaction */
	PINPAD_COMPLETETRANS,                /**< Complete the transaction */
	PINPAD_TERMINATE,                    /**< to stop the transaction, reset the internal transaction status and release resource */
	PINPAD_L3CALLBACK,
} PINPAD_MSGTYPE;

typedef struct {
	char cMsgType;
	char cCardInteface;
	char *pszInputData;
	int nTimeOut;
	int nInputDataLen;
} STPINPADL3_IN;

extern int PubResetPinpad_PINPAD(const int nPort, const int nTimeOut);
extern int PubLoadKey_PINPAD(int nKeyType, int nGroupNo, const char *psKeyValue, int nKeyLen, char* psCheckValue);
extern int PubGetPinBlock_PINPAD(char *psPin, int *pnPinLen, int nMode, int nKeyType, int nKeyIndex, const char *pszCardno, int nPanLen, int nMaxLen);
extern int PubCalcMac_PINPAD(char *psMac, int nKeyType, int nMode, int nKeyIndex, const char *psData, int nDataLen);

extern int PubClrPinPad_PINPAD(void);
extern int PubDispPinPad_PINPAD(const char *pszLine1, const char *pszLine2, const char *pszLine3, const char *pszLine4);
extern int PubDesPinpad_PINPAD(const char *psSrc, int nSrcLen, char *psDest, int nKeyIndex, int nDesMode);
extern int PubGetPinpadVer_PINPAD(char *pszVer);
extern int PubGetErrCode_PINPAD(void);
extern int PubReadString_PINPAD(char *pstString, int *pnStringLen, int nMaxLen, int nMinLen);
extern int PubClearKey_PINPAD(void);
extern int PubPinpadBeep_PINPAD(int nDuration, int nType);
extern int PubGenAndShowQr_PINPAD(int nVersion, char *pszBuffer);
extern int PubCancelPIN_PINPAD(void);
extern int PubDoScan_PINPAD(char *pszBuffer);
extern int PubEsignature_PINPAD(char *pszCharaterCode, char *pszSignName, int nTimeOut);
extern int PubInjectKey_PINPAD(int nKeyType, int nSrcIndex, int nDstIndex,const char *psKeyValue, int nKeyLen, char *psCheckValue);
extern int PubGetKcv_PINPAD( int nKeyIndex,int nKeyType, char *psKcv);
extern int PubSetFontColor_PINPAD(unsigned short usColor, char cObject);
extern int PubLoadImg_PINPAD(unsigned char ucImgID, int nImgLen, char *psImgData);
extern int PubDispImg_PINPAD(unsigned char ucImgID, int nX, int nY);
extern int PubSetAlignMode_PINPAD(char cMode);
extern int PubSetClrsMode_PINPAD(char cMode);
extern int PubSetFontSize_PINPAD(char cSize);
extern int PubCreateFile_PINPAD(char *pszFileName);
extern int PubLoadFile_PINPAD(char *pszPath);
extern int PubUpdateFile_PINPAD(char *pszFileName, char cRebootFlag);
extern int PubSwipeCard_PINPAD(char *pIn, char *pOut);

extern int PubL3OrderSet_PINPAD(char *pL3Cfg, char *pszOutPut, int *pnOutPutLen);
extern int PubL3PerformRecv_PINPAD(char *pszOutPut, int *pnOutPutLen);
extern void PubL3CancalReadCard();
extern int PubCheckIcc_PINPAD();
extern int PubBeep_PINPAD();
extern int PubReboot_PINPAD();

extern int PubIncDukptKSN_PINPAD(int nKeyIndex);
extern int PubGetDukptKSN_PINPAD(int nKeyIndex, char *pszKsn);
#endif

/**< End of lpindpad.h */
