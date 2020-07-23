#ifndef _TMS_CALLBACK_H_
#define _TMS_CALLBACK_H_

#ifdef USE_TMS
extern int Tms_CommConnect(int );
extern int Tms_CommClose(int );
extern int DispResultMsg(char* , char* , char* , int);
extern int SetCommDomain(char* , int , int );
extern void DispProgress(char , uint , uint );
extern void ShowPromptMsg(char* , char* , char , char , char , int );
extern int ShowSelectPrompt(char* , char* );
extern void LibTmsDebug(const char* , int );

#endif
#endif

