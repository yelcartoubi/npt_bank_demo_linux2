/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
* @file  toms_process.h
* @brief
* @version 1.0
* @author Shawn Lian
* @date 2020-7-1
**************************************************************************/

#ifndef _TOMS_PROCESS_H_
#define _TOMS_PROCESS_H_

#ifdef USE_TOMS

/*******************************
 * Macro Definition
 *******************************/
/* Param Type String */
#define TOMSTYPE_STR_STRING			"STRING"
#define TOMSTYPE_STR_BOOLEAN		"BOOLEAN"
#define TOMSTYPE_STR_NUMBER			"NUMBER"
#define TOMSTYPE_STR_HEX			"HEX"
#define TOMSTYPE_STR_REFERENCE		"REFERENCE"
#define TOMSTYPE_STR_FILE           "FILE"

	/* BOOL_VALUE */
#define TOMS_BOOL_TURE				"true"
#define TOMS_BOOL_FALSE				"false"

/* CFG param */
#define TOMSTAG_BODY				"body"
#define TOMSTAG_PARAMVALUES			"paramValues"
#define TOMSTAG_TYPE				"type"
#define TOMSTAG_VALUE				"value"
#define TOMSTAG_SUBVALUE			"value"

/* basic param */
#define TOMSTAG_VERSION				"appVersion"

/* basic param */
#define TOMSTAG_MERCHANTID			"merchantId"
#define TOMSTAG_TERMINALID			"terminalId"
#define TOMSTAG_MERCHANTNAMEEN		"merchantName_EN"
#define TOMSTAG_MERCHANTADDR1		"merchantAddr1"
#define TOMSTAG_MERCHANTADDR2		"merchantAddr2"
#define TOMSTAG_MERCHANTADDR3		"merchantAddr3"

/* transaction control */
#define	TOMSTAG_SUPPORTSALE			"consumeYN"
#define	TOMSTAG_SUPPORTVOIDSALE		"cousumeVoidYN"
#define	TOMSTAG_SUPPORTREFUND		"refundYN"
#define	TOMSTAG_SUPPORTBALANCE		"balanceYN"
#define	TOMSTAG_SUPPORTPREAUTH		"preAuthYN"
#define	TOMSTAG_SUPPORTVOIDPEAUTH	"preAuthVoidYN"
#define	TOMSTAG_SUPPORTAUTHSALE		"authConsumeYN"
#define	TOMSTAG_SUPPORTVOIDAUTHSALE	"authConsumeVoidYN"
#define	TOMSTAG_SUPPORTADJUST		"adjustYN"

/* system param */
#define	TOMSTAG_TRANS_TRACENO		"traceNo"
#define	TOMSTAG_TRANS_BATCHNO		"batchNo"
#define	TOMSTAG_ISPINPAD			"pinpadOutSideYN"
#define	TOMSTAG_MAXTRANSCNT			"maxTransaction"

/* communication param */
#define	TOMSTAG_COMM_NII			"NII"
#define	TOMSTAG_COMM_TYPE			"commType"
#define	TOMSTAG_COMM_TIMEOUT		"commTimeOut"
#define TOMSTAG_COMM_TPDU           "tpdu"
	/* LINE(ETH) */
#define	TOMSTAG_COMM_IP1_E			"ethHost1"
#define	TOMSTAG_COMM_PORT1_E		"ethPort1"
#define	TOMSTAG_COMM_IP2_E			"ethHost2"
#define	TOMSTAG_COMM_PORT2_E		"ethPort2"
#define	TOMSTAG_COMM_IPADDR_E		"ethIP"
#define	TOMSTAG_COMM_MASK_E			"ethMask"
#define	TOMSTAG_COMM_GATE_E			"ethGateway"
#define TOMSTAG_COMM_ISDHCP_E		"ethDHCP"

	/* DIAL */
#define	TOMSTAG_COMM_PREDIALNO		"preDialNum"
#define	TOMSTAG_COMM_PREPHONE		"prePhone"
#define	TOMSTAG_COMM_TELNO1			"tel1"
#define	TOMSTAG_COMM_TELNO2			"tel2"
#define	TOMSTAG_COMM_TELNO3			"tel3"
#define	TOMSTAG_COMM_ISPREDIAL      "predialYN"

	/* GPRS */
#define TOMSTAG_COMM_WIRELESSDIAL_G "gprsNo"
#define	TOMSTAG_COMM_IP1_G			"gprsHost1"
#define	TOMSTAG_COMM_PORT1_G		"gprsPort1"
#define	TOMSTAG_COMM_IP2_G			"gprsHost2"
#define	TOMSTAG_COMM_PORT2_G		"gprsPort2"
#define	TOMSTAG_COMM_APN1			"gprsAPN"
#define	TOMSTAG_COMM_USER_G			"gprsUser"
#define	TOMSTAG_COMM_PWD_G			"gprsPwd"
#define	TOMSTAG_COMM_MODE_G			"gprsHangup"
	/* CDMA */
#define	TOMSTAG_COMM_IP1_C			"cdmaHost1"
#define	TOMSTAG_COMM_PORT1_C		"cdmaPort1"
#define	TOMSTAG_COMM_IP2_C			"cdmaHost2"
#define	TOMSTAG_COMM_PORT2_C		"cdmaPort2"
#define	TOMSTAG_COMM_USER_C			"cdmaUser"
#define	TOMSTAG_COMM_PWD_C			"cdmaPwd"
#define	TOMSTAG_COMM_MODE_C			"cdmaHangup"
	/* WIFI */
#define	TOMSTAG_COMM_WIFISSID		"wifiSSID"
#define	TOMSTAG_COMM_WIFIKEY		"wifiPwd"
#define	TOMSTAG_COMM_WIFIMODE		"wifiEncrypt"
#define	TOMSTAG_COMM_IP1_W			"wifiHost1"
#define	TOMSTAG_COMM_PORT1_W		"wifiPort1"
#define	TOMSTAG_COMM_IP2_W			"wifiHost2"
#define	TOMSTAG_COMM_PORT2_W		"wifiPort2"
#define	TOMSTAG_COMM_IPADDR_W		"wifiIP"
#define	TOMSTAG_COMM_MASK_W			"wifiMask"
#define	TOMSTAG_COMM_GATE_W			"wifiGateway"
#define TOMSTAG_COMM_ISDHCP_W       "wifiDHCP"
    /* KEY */
#define TOMSTAG_MAINKEYNO           "keyIndex"
#define TOMSTAG_ENCRYMODE           "desType"
#define TOMSTAG_ADMINPWD            "systemPwd"
#define TOMSDEFAULT_FUNCTION        "safePwd"
    /* PRINT */
#define TOMSTAG_PNTTITLE            "printHead"
#define TOMSTAG_PNTPAGECNT          "printNum"
#define TOMSTAG_FONTSIZE            "printFont"
#define TOMSTAG_PNTDETAIL           "printDetail"
#define TOMSTAG_ISPREAUTHSHIELDPAN  "preauthprintMaskCardNo"

#define TOMSTAG_TESTFILE            "test.file"

/*******************************
 * Type Definition
 *******************************/
typedef enum
{
	TOMSTYPE_STRING = 0,
	TOMSTYPE_BOOLEAN,
	TOMSTYPE_NUMBER,
	TOMSTYPE_HEX,
	TOMSTYPE_REFERENCE,
	TOMSTYPE_FILE,
}EM_TOMS_PARAM_TYPE;

typedef enum
{
	TOMS_COMMTYPE_DAIL = 0,
	TOMS_COMMTYPE_ETH,
	TOMS_COMMTYPE_CDMA,
	TOMS_COMMTYPE_GPRS,
	TOMS_COMMTYPE_RS232,
	TOMS_COMMTYPE_WIFI,
}EM_TOMS_PARAM_COMMTYPE;

typedef enum
{
	TOMS_WIFITYPE_OPEN = 1,
	TOMS_WIFITYPE_WEP,
	TOMS_WIFITYPE_WPA_PSK,
	TOMS_WIFITYPE_WPA2_PSK,
	TOMS_WIFITYPE_WPA_CCMK,
}EM_TOMS_PARAM_WIFITYPE;

/*******************************
 * Global Function Declaration
 *******************************/
extern int TOMS_ProInit(void);
extern int TOMS_SetDebugSwitch(char cSwitch);
extern int TOMS_UpdateDomain(EM_TOMS_OPT emOpt, char *pszDomain);

#endif /* USE_TOMS */

#endif

