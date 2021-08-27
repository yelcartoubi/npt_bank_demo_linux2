#ifndef _TAGPARAM_H_
#define _TAGPARAM_H_

//TLV format:Tag(3 Bytes of ASCII) + Length(3 Bytes of ASCII) + Value
#define TLV_TAGLEN                   3   					//length of "T"
#define TLV_LENGTHLEN                3   					//Length of "L"
#define MAX_VALUE_LEN            	 512
#define FILE_APPPOSPARAM	         APP_NAME"POSPARA"
#define FILE_APPSETTLEMENT	         APP_NAME"SETTLEPARA"
#define FILE_APPREVERSAL	         APP_NAME"REVERSALPARA"
#define FILE_APPTRANSPARAM	     	 APP_NAME"TRANSPARA"

#define DEFAULT_LEN                  -1
#define PER_TRANSRECORD_LEN	         1024

#define ASSERT_FILE_FAIL(e, nHandle) \
	if ((e) != APP_SUCC)\
	{\
		PubFsClose(nHandle);\
		return APP_FAIL;\
	}

typedef struct
{
	char *pszFileName;
	int nTagid;
	int nLen;           // if nlen = DEFAULT_LEN, then real len is strlen(value)
	char *pszValue;		// default value
} STTAGINFO;

enum EM_TAG_POSPARAM {
/* STAPPPOSPARAM */
	TAG_MERCHANTID				= 1,
	TAG_TERMINALID				= 2,
	TAG_MERCHANTNAMEEN			= 3,
	TAG_MERCHANTADDR1			= 4,
	TAG_MERCHANTADDR2			= 5,
	TAG_MERCHANTADDR3			= 6,
	TAG_ADMINPWD				= 7,
	TAG_USRWD					= 8,
	TAG_FUNCPWD					= 9,
	TAG_TRANSSWITCH				= 10,
	TAG_ACQNAME					= 11,
	TAG_ISSUERNAME				= 12,
	TAG_CURRENCYNAME			= 13,
	TAG_PNTPAGECNT				= 14,
	TAG_PNTDETAIL				= 15,
	TAG_TIPFLAG					= 16,
	TAG_TIPRATE					= 17,
	TAG_CARDINPUTMODE			= 18,
	TAG_ISSALEVOIDSTRIP			= 19,
	TAG_ISVOIDPIN				= 20,
	TAG_ISSUPPORTRF				= 21,
	TAG_ISEXRF					= 22,
	TAG_ISPINPAD				= 23,
	TAG_PINPADAUXNO				= 24,
	TAG_PINPADTIMEOUT			= 25,
	TAG_MAINKEYNO				= 26,
	TAG_ENCRYMODE				= 27,
	TAG_PINENCRYMODE			= 28,
	TAG_MAXTRANSCNT				= 29,
	TAG_ISPRINTERRREPORT		= 30,
	TAG_SUPPORTCONTACT			= 31,
	TAG_DEFAULTTRANS			= 32,
	TAG_ISSHOWTVRTSI			= 33,
	TAG_ISSHIELDPAN				= 34,
	TAG_PNTTITLEMODE			= 35,
	TAG_PNTTITLEEN				= 36,
	TAG_SHOWAPPNAME				= 37,
	TAG_ISADMINPWD				= 38,
	TAG_PNTMINUS				= 39,
	TAG_ISPREAUTHSHIELDPAN		= 40,
	TAG_ISREPNTSETTLE			= 41,
	TAG_SUPPOTRSWIPE			= 42,
	TAG_ISPNTISO				= 43,
	TAG_KEYSYSTEMTYPE			= 44,
	TAG_LANGUAGE				= 45,
	TAG_FONTSIZE				= 46,
	TAG_APPCHKVALUE				= 47,
	TAG_VERSION					= 48,
	TAG_CVV2					= 49,
	TAG_TMSAUTOUPDATE			= 50,
	TAG_PINPADUSAGE				= 51,
	TAG_PINPADCALLBACKFLAG		= 52,
	TAG_PINPADTYPE				= 53,
	TAG_L3INITSTATUS			= 54,
	TAG_LOCKTERMINAL            = 55,
	TAG_LOCKPROMPTINFO          = 56,
	TAG_TOMSOBTAINCMD			= 57,
};

enum EM_TAG_TRANSRECORD {
//STTRANSRECORD
	TAG_RECORD_TRANSTYPE		= 1,
	TAG_RECORD_TRANSATTR		= 2,
	TAG_RECORD_EMVSTATUS		= 3,
	TAG_RECORD_PAN				= 4,
	TAG_RECORD_PANLEN			= 5,
	TAG_RECORD_AMT				= 6,
	TAG_RECORD_TRACE			= 7,
	TAG_RECORD_TIME				= 8,
	TAG_RECORD_DATE				= 9,
	TAG_RECORD_EXPDATE			= 10,
	TAG_RECORD_INPUTMODE		= 11,
	TAG_RECORD_CARDSN			= 12,
	TAG_RECORD_TRACK2			= 13,
	TAG_RECORD_TRACK2LEN		= 14,
	TAG_RECORD_TRACK3			= 15,
	TAG_RECORD_TRACK3_LEN		= 16,
	TAG_RECORD_REFNUM			= 17,
	TAG_RECORD_AUTHCODE			= 18,
	TAG_RECORD_RESPONSE			= 19,
	TAG_RECORD_BATCH			= 20,
	TAG_RECORD_OLDTRACE			= 21,
	TAG_RECORD_OLDAUTHCODE		= 22,
	TAG_RECORD_OLDREFNUM		= 23,
	TAG_RECORD_STATUS			= 24,
	TAG_RECORD_TIPAMT			= 25,
	TAG_RECORD_BASEAMT			= 26,
	TAG_RECORD_SENDFLAG			= 27,
	TAG_RECORD_BATCHUPFLAG		= 28,
	TAG_RECORD_TCFLAG			= 29,
	TAG_RECORD_HOLDNAME			= 30,
	TAG_RECORD_SERVERCODE		= 31,
	TAG_RECORD_FIELD55			= 32,
	TAG_RECORD_FIELD55LEN		= 33,
	TAG_RECORD_PRINTFLAG		= 34,
	TAG_RECORD_CVV				= 35,
	TAG_RECORD_ADD				= 36,
	TAG_RECORD_ADDLEN			= 37,
	TAG_RECORD_SIG_PIN			= 38,
	TAG_RECORD_CASHBACKAMT		= 39,
	TAG_RECORD_OLDDATE			= 40,
};

enum EM_TAG_COMMPARAM {
/* STAPPPOSCOMM */
	TAG_MIN_APPCOMMPARAM		= 1,
	TAG_COMM_TYPE				= 2,
	TAG_COMM_TYPE2				= 3,
	TAG_COMM_ISPREDIAL			= 4,
	TAG_COMM_REDIALNUM			= 5,
	TAG_COMM_RESENDNUM			= 6,
	TAG_COMM_TIMEOUT			= 7,
	TAG_COMM_TPDU				= 8,
	TAG_COMM_PREDIALNO			= 9,
	TAG_COMM_TELNO1				= 10,
	TAG_COMM_TELNO2				= 11,
	TAG_COMM_TELNO3				= 12,
	TAG_COMM_IP1				= 13,
	TAG_COMM_PORT1				= 14,
	TAG_COMM_IP2				= 15,
	TAG_COMM_PORT2				= 16,
	TAG_COMM_WIRELESSDIALNUM	= 17,
	TAG_COMM_APN1				= 18,
	TAG_COMM_APN2				= 19,
	TAG_COMM_USER				= 20,
	TAG_COMM_PWD				= 21,
	TAG_COMM_SIMPWD				= 22,
	TAG_COMM_MODE				= 23,
	TAG_COMM_IPADDR				= 24,
	TAG_COMM_MASK				= 25,
	TAG_COMM_GATE				= 26,
	TAG_COMM_DNSIP1				= 27,
	TAG_COMM_DNSIP2				= 28,
	TAG_COMM_DNSIP3				= 29,
	TAG_COMM_DOMAIN1			= 30,
	TAG_COMM_DOMAIN2			= 31,
	TAG_COMM_AUXISADDTPDU		= 32,
	TAG_COMM_ISDNS				= 33,
	TAG_COMM_OFFRESENDNUM		= 34,
	TAG_COMM_ISDHCP				= 35,
	TAG_COMM_WIFISSID			= 36,
	TAG_COMM_WIFIKEY			= 37,
	TAG_COMM_WIFIKEYTYPE		= 38,
	TAG_COMM_WIFIMODE			= 39,
	TAG_COMM_NII				= 40,
	TAG_COMM_ISSSL				= 41,
	TAG_COMM_TMSDOMAIN			= 42,
    TAG_COMM_TOMSPARAMDOMAIN    = 43,
    TAG_COMM_TOMSKEYPOSDOMAIN   = 44,
    TAG_COMM_TOMSFILESERDOMAIN  = 45,
    TAG_COMM_TOMSTDASDOMAIN     = 46,
	TAG_COMM_TOMSAPPDOMAIN      = 47,
    TAG_COMM_TOMSISUSEROID      = 48,
    TAG_COMM_TOMSUSEROID        = 49
};

enum EM_TAG_TRANSPARAM {
// STTRANSPARAM
	TAG_TRANS_ISLOGIN			= 1,
	TAG_TRANS_TRACENO			= 2,
	TAG_TRANS_BATCH				= 3,
	TAG_TRANS_INVOICENO 		= 4,
	TAG_TRANS_LOGINDATETIME		= 5,
	TAG_TRANS_ONLINEUPNUM		= 6,
	TAG_TRANS_QPSLIMIT			= 7,
};

enum EM_TAG_REVERSEPARAM {
// STREVERSAL
	TAG_REVERSE_FLAG			= 1,
	TAG_REVERSE_HADSENDCNT		= 2,
	TAG_REVERSE_TRANSTYPE		= 3,
	TAG_REVERSE_TRANSATTR		= 4,
	TAG_REVERSE_PAN 			= 5,
	TAG_REVERSE_PROCESSCODE 	= 6,
	TAG_REVERSE_AMT 			= 7,
	TAG_REVERSE_TRACE			= 8,
	TAG_REVERSE_EXDATE			= 9,
	TAG_REVERSE_INPUTMODE		= 10,
	TAG_REVERSE_CARDSN			= 11,
	TAG_REVERSE_NII 			= 12,
	TAG_REVERSE_SERVERCODE		= 13,
	TAG_REVERSE_TRACK1			= 14,
	TAG_REVERSE_TRACK2			= 15,
	TAG_REVERSE_RESPONSECODE	= 16,
	TAG_REVERSE_OLDDATE 		= 17,
	TAG_REVERSE_OLDTIME 		= 18,
	TAG_REVERSE_INVOICE 		= 19,
	TAG_REVERSE_CVV2			= 20,
	TAG_REVERSE_ADD1			= 21,
	TAG_REVERSE_ADD1LEN 		= 22,
	TAG_REVERSE_CASHBACKAMT		= 23,
	TAG_REVERSE_OLDAUTHCODE		= 24,
};

enum EM_TAG_SETTLEPARAM {
// SETTLE
	TAG_SETTLE_OFFLINEUNSENDNUM	= 1,
	TAG_SETTLE_RECNUM			= 2,
	TAG_SETTLE_HADRESENDNUM		= 3,
	TAG_SETTLE_PNTSETTLEHALT	= 4,
	TAG_SETTLE_PNTDETAILHALT	= 5,
	TAG_SETTLE_PNTRECORDHALT	= 6,
	TAG_SETTLE_TOTALMATCHFLAG	= 7,
	TAG_SETTLE_BATCHHALT		= 8,
	TAG_SETTLE_CLRSETTLE		= 9,
	TAG_SETTLE_BATCHOFFLINEHALT	= 10,
	TAG_SETTLE_BATCH_SUM		= 11,
	TAG_SETTLE_DATETIME			= 12,
	TAG_SETTLE_SALENUM			= 13,
	TAG_SETTLE_SALEAMT			= 14,
	TAG_SETTLE_VOIDSALENUM		= 15,
	TAG_SETTLE_VOIDSALEAMT		= 16,
	TAG_SETTLE_AUTHSALENUM		= 17,
	TAG_SETTLE_AUTHSALEAMT		= 18,
	TAG_SETTLE_VOIDAUTHSALENUM	= 19,
	TAG_SETTLE_VOIDAUTHSALEAMT	= 20,
	TAG_SETTLE_PREAUTHNUM		= 21,
	TAG_SETTLE_PREAUTHAMT		= 22,
	TAG_SETTLE_VOIDPREAUTHNUM	= 23,
	TAG_SETTLE_VOIDPREAUTHAMT	= 24,
	TAG_SETTLE_REFUNDNUM		= 25,
	TAG_SETTLE_REFUNDAMT		= 26,
	TAG_SETTLE_ADJUSTNUM		= 27,
	TAG_SETTLE_ADJUSTAMT		= 28,
	TAG_SETTLE_OFFLINENUM		= 29,
	TAG_SETTLE_OFFLINEAMT		= 30,
};

extern int InitCommParamFile(const STAPPCOMMPARAM stAppCommParam);
extern int InitPosParamFile(const STAPPPOSPARAM stAppPosParam);
extern int InitReversalFile(STREVERSALPARAM stReversalParam);
extern int InitTransParamFile(STTRANSPARAM stTransParam);
extern int InitSettleFile(STSETTLEPARAM stSettleParam);

extern int UpdateReverseData(char *pszFileName, STREVERSAL stReversal);
extern int UpdateAppCommParam(char *pszFileName, STAPPCOMMPARAM stAppCommParam);
extern int UpdateAppPosParam(char *pszFileName, STAPPPOSPARAM stAppPosParam);

extern int GetRecordTag(char    *pszTagstr, int nTagid, int *pnLen, char *pszValue);
extern int RecordBufToStRecord(char *pszTagstr, STTRANSRECORD *pstTransRecord);
extern int StRecordToRecordBuf(const STTRANSRECORD *pstTransRecord, char *pszTagstr);

extern int InitExPosParam();
extern int LoadTagParam(char *pszFileName, void *pszParam);
extern int UpdateTagParam(const char *pszFileName, int nTagId, int nValueLen, char *pszTagValue);
extern int GetTag(char *pszFileName, int nTagId, int *pnLen, char *pszValue);

extern int ExportFileContent(char *pszFileName);
#endif

