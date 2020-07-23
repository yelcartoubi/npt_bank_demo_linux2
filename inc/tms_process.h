#ifndef _TMS_H_
#define _TMS_H_

#ifdef USE_TMS

// TMS param
//basic param
#define TMSTAG_MERCHANTID			"merchantId"
#define TMSTAG_TERMINALID			"terminalId"
#define TMSTAG_MERCHANTNAMEEN		"merchantName_EN"
#define TMSTAG_MERCHANTADDR1		"merchantAddr1"
#define TMSTAG_MERCHANTADDR2		"merchantAddr2"
#define TMSTAG_MERCHANTADDR3		"merchantAddr3"

// transaction control
#define	TMSTAG_SUPPORTSALE			"consumeYN"
#define	TMSTAG_SUPPORTVOIDSALE		"cousumeVoidYN"
#define	TMSTAG_SUPPORTREFUND		"refundYN"
#define	TMSTAG_SUPPORTBALANCE		"balanceYN"
#define	TMSTAG_SUPPORTPREAUTH		"preAuthYN"
#define	TMSTAG_SUPPORTVOIDPEAUTH	"preAuthVoidYN"
#define	TMSTAG_SUPPORTAUTHSALE		"authConsumeYN"
#define	TMSTAG_SUPPORTVOIDAUTHSALE	"authConsumeVoidYN"
#define	TMSTAG_SUPPORTTIP			"tipYN"
#define	TMSTAG_ISSALEVOIDPIN		"consumeVoidPwd"
#define	TMSTAG_ISPREAUTHVOIDPIN		"preAuthVoidPwd"
#define	TMSTAG_ISAUTHSALEVOIDPIN	"authConsumeVoidPwd"
#define	TMSTAG_ISSALEVOIDCARD		"consumeVoidCard"
#define	TMSTAG_ISAUTHSALEVOIDCARD	"authConsumeVoidCard"
#define	TMSTAG_ISCARDINPUT			"handInCard"
#define	TMSTAG_DEFAULTTRANS			"defaultTransaction"
#define	TMSTAG_REFUNDMAXAMT			"refundMaxAmount"
#define	TMSTAG_TIPRATE				"tipRate"
#define	TMSTAG_SUPPORTADJUST		"adjust"
#define	TMSTAG_SUPPORTSWIPE			"supportswipeYN"

// system param
#define	TMSTAG_TRANS_TRACENO		"traceNo"
#define	TMSTAG_TRANS_BATCH			"batchNo"
#define	TMSTAG_ISPINPAD				"pinpadOutSideYN"
#define	TMSTAG_MAXTRANSCNT			"maxTransaction"

// communication param
#define	TMSTAG_COMM_TYPE			"commType"
#define	TMSTAG_COMM_TYPE2			"commType2"
#define	TMSTAG_COMM_TIMEOUT			"commTimeOut"
#define	TMSTAG_COMM_TPDU			"tpdu"
#define	TMSTAG_COMM_TELNO1			"tel1"
#define	TMSTAG_COMM_TELNO2			"tel2"
#define	TMSTAG_COMM_TELNO3			"tel3"
#define	TMSTAG_COMM_PREDIALNO		"preDialNum"
#define	TMSTAG_COMM_ISPREDIAL		"predialYN"
#define	TMSTAG_COMM_WIRELESSDIAL_G	"gprsNo"
#define	TMSTAG_COMM_MODE_G			"gprsHangup"
#define	TMSTAG_COMM_APN1			"gprsAPN"
#define	TMSTAG_COMM_IP1_G			"gprsHost1"
#define	TMSTAG_COMM_PORT1_G			"gprsPort1"
#define	TMSTAG_COMM_IP2_G			"gprsHost2"
#define	TMSTAG_COMM_PORT2_G			"gprsPort2"
#define	TMSTAG_COMM_USER_G			"gprsUser"
#define	TMSTAG_COMM_PWD_G			"gprsPwd"
#define	TMSTAG_COMM_WIRELESSDIAL_C	"cdmaNo"
#define	TMSTAG_COMM_MODE_C			"cdmaHangup"
#define	TMSTAG_COMM_IP1_C			"cdmaHost1"
#define	TMSTAG_COMM_PORT1_C			"cdmaPort1"
#define	TMSTAG_COMM_IP2_C			"cdmaHost2"
#define	TMSTAG_COMM_PORT2_C			"cdmaPort2"
#define	TMSTAG_COMM_USER_C			"cdmaUser"
#define	TMSTAG_COMM_PWD_C			"cdmaPwd"
#define	TMSTAG_COMM_ISDHCP_E		"ethDHCP"
#define	TMSTAG_COMM_IPADDR_E		"ethIP"
#define	TMSTAG_COMM_MASK_E			"ethMask"
#define	TMSTAG_COMM_GATE_E			"ethGateway"
#define	TMSTAG_COMM_IP1_E			"ethHost1"
#define	TMSTAG_COMM_PORT1_E			"ethPort1"
#define	TMSTAG_COMM_IP2_E			"ethHost2"
#define	TMSTAG_COMM_PORT2_E			"ethPort2"
#define	TMSTAG_COMM_ISDHCP_W		"wifiDHCP"
#define	TMSTAG_COMM_IPADDR_W		"wifiIP"
#define	TMSTAG_COMM_MASK_W			"wifiMask"
#define	TMSTAG_COMM_GATE_W			"wifiGateway"
#define	TMSTAG_COMM_IP1_W			"wifiHost1"
#define	TMSTAG_COMM_PORT1_W			"wifiPort1"
#define	TMSTAG_COMM_IP2_W			"wifiHost2"
#define	TMSTAG_COMM_PORT2_W			"wifiPort2"
#define	TMSTAG_COMM_WIFISSID		"wifiSID"
#define	TMSTAG_COMM_WIFIKEY			"wifiPwd"
#define	TMSTAG_COMM_WIFIMODE		"wifiEncrypt"
#define	TMSTAG_COMM_NII				"NII"

//key passwd
#define	TMSTAG_MAINKEYNO			"keyIndex"
#define	TMSTAG_ENCRYMODE			"desType"
#define	TMSTAG_ADMINPWD				"systemPwd"
#define	TMSDEFAULT_FUNCTION			"safePwd"

// print settings
#define TMSTAG_PNTTITLE				"printHead"
#define TMSTAG_PNTPAGECNT			"printNum"
#define TMSTAG_FONTSIZE				"printFont"
#define TMSTAG_PNTDETAIL			"printDetail"
#define TMSTAG_ISPREAUTHSHIELDPAN	"preauthprintMaskCardNo"





typedef enum
{
    UPTMODE_INITIATIVE,
    UPTMODE_PASSIVE,
    UPTMODE_EXIST
}EM_UTPMODE;
		

extern int TmsCheckUpdate(EM_UTPMODE , YESORNO );


#endif

#endif

