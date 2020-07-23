/**************************************************************************
* Copyright (C) 2019 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file callback.h
* @brief  
* @version 1.0
* @author 
* @date 2019-9-2
**************************************************************************/


#ifndef _EMV_CALLBACK_H_
#define _EMV_CALLBACK_H_
extern void SaveInputAMT(char* );
extern void InitInputAMT(void);
extern void SetupCallbackFunc(void);
extern void GetCardEventData(char *pszOut);
extern void GetManualExpiryDate(char *pszOut);
extern void GetManualCVV(char *pszOut);
extern void EmvGetOnlinePin(char *);
extern void SetEmvDebugPort(int );
extern int CtrlCardPan(uchar *, const uint, const uchar, const uint);
extern int DispPan(char *);
extern void EmvClrOnlinePin();

#endif

