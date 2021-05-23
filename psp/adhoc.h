/***************************************************************************

	adhoc.c

	PSP AdHoc functions.

***************************************************************************/

#ifndef PSP_ADHOC_INTERFACE_H
#define PSP_ADHOC_INTERFACE_H

#define ADHOC_USE_GUI

#ifdef ADHOC_USE_GUI
#define ScreenInit()	msg_screen_init("AdHoc");
#define ScreenClear		msg_screen_clear
#define Printf			msg_printf
#define SetTextColor	msg_set_text_color
#else
#define ScreenInit()	pspDebugScreenInit()
#define ScreenClear		pspDebugScreenClear
#define Printf			pspDebugScreenPrintf
#define SetTextColor	pspDebugScreenSetTextColor
#endif

int pspSdkLoadAdhocModules(void);

int adhocInit(char *matchingData);
int adhocTerm(void);
int adhocSelect(void);
int adhocReconnect(char *ssid);

int adhocSend(void *buffer, int length);
int adhocRecv(void *buffer, int *length);
int adhocRecvBlocked(void *buffer, int *length);

int adhocSendRecvAck(void *buffer, int length);
int adhocRecvSendAck(void *buffer, int *length);

#endif /* PSP_ADHOC_INTERFACE_H */
