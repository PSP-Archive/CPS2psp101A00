/***************************************************************************

	adhoc.c

	PSP AdHoc functions.

***************************************************************************/

#include "psp.h"
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhocmatching.h>
#include <pspwlan.h>

#define NUM_ENTRIES			32

#define PSP_LISTING			1
#define PSP_SELECTED		2
#define PSP_SELECTING		3
#define PSP_WAIT_EST		4
#define PSP_ESTABLISHED		5

/***************************************************************************
	関数パラメータ (長いので・・・)
***************************************************************************/

#define MATCHING_CREATE_PARAMS	\
	3,							\
	0xa,						\
	0x22b,						\
	0x800,						\
	0x2dc6c0,					\
	0x5b8d80,					\
	3,							\
	0x7a120,					\
	matchingCallback			

#define MATCHING_START_PARAMS	\
	matchingId,					\
	0x10,						\
	0x2000,						\
	0x10,						\
	0x2000,						\
	strlen(MatchingData) + 1,	\
	MatchingData				


/***************************************************************************
	ローカル変数
***************************************************************************/

static int NetInit;
static int NetAdhocInit;
static int NetAdhocctlInit;
static int NetAdhocctlConnect;
static int NetAdhocPdpCreate;
static int NetAdhocMatchingInit;
static int NetAdhocMatchingCreate;
static int NetAdhocMatchingStart;

static int Server;
static int pdpId;

static char g_mac[6];
static int  g_unk1;
static int  g_matchEvent;
static int  g_matchOptLen;
static char g_matchOptData[1000];
static int  matchChanged;
static int  matchingId;

static struct psplist_t
{
	char name[48];
	char mac[6];
} psplist[NUM_ENTRIES];

static int max;
static int pos;


/***************************************************************************
	ローカル関数
***************************************************************************/

/*--------------------------------------------------------
	リスト消去
--------------------------------------------------------*/

static void ClearPspList(void)
{
	max = 0;
	pos = 0;
	memset(&psplist, 0, sizeof(psplist));
}


/*--------------------------------------------------------
	リストに追加
--------------------------------------------------------*/

static int AddPsp(char *mac, char *name, int length)
{
	int i;

	if (max == NUM_ENTRIES) return 0;
	if (length == 1) return 0;

	for (i = 0; i < max; i++)
	{
		if (memcmp(psplist[i].mac, mac, 6) == 0)
			return 0;
	}

	memcpy(psplist[max].mac, mac, 6);

	if (length)
	{
		if (length < 47)
			strcpy(psplist[max].name, name);
		else
			strncpy(psplist[max].name, name, 47);
	}
	else
		psplist[max].name[0] = '\0';

	max++;

	return 1;
}


/*--------------------------------------------------------
	リストから削除
--------------------------------------------------------*/

static int DelPsp(char *mac)
{
	int i, j;

	for (i = 0; i < max; i++)
	{
		if (memcmp(psplist[i].mac, mac, 6) == 0)
		{
			if (i != max - 1)
			{
				for (j = i + 1; j < max; j++)
				{
					memcpy(psplist[j - 1].mac, psplist[j].mac, 6);
					strcpy(psplist[j - 1].name, psplist[j].name);
				}
			}

			if (pos == i) pos = 0;
			if (pos > i) pos--;
			max--;

			return 0;
		}
	}

	return -1;
}


/*--------------------------------------------------------
	リストを表示
--------------------------------------------------------*/

static void DisplayPspList(int top, int rows)
{
	if (max == 0)
	{
		Printf("Waiting for another PSP to join\n");
	}
	else
	{
		int i;
		char temp[20];

#ifdef ADHOC_USE_GUI
		video_copy_rect(show_frame, draw_frame, &full_rect, &full_rect);

		draw_scrollbar(470, 26, 479, 270, rows, max, pos);

		for (i = 0; i < rows; i++)
		{
			if ((top + i) >= max) break;

			sceNetEtherNtostr((u8 *)psplist[top + i].mac, temp);

			if ((top + i) == pos)
			{
				uifont_print(24, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_SELECT), temp);
				uifont_print(190, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_SELECT), psplist[top + i].name);
			}
			else
			{
				uifont_print(24, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_NORMAL), temp);
				uifont_print(190, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_NORMAL), psplist[top + i].name);
			}
		}

		video_flip_screen(1);
#else
		for (i = 0; i < rows; i++)
		{
			if ((top + i) >= max) break;

			if ((top + i) == pos)
				SetTextColor(0xffffffff);
			else
				SetTextColor(0x00aaaaaa);

			sceNetEtherNtostr((u8 *)psplist[top + i].mac, temp);

			Printf("  %s: %s\n", temp, psplist[top + i].name);
		}
#endif
	}
}


/*--------------------------------------------------------
	選択中のPSPの情報を取得
--------------------------------------------------------*/

static int GetPspEntry(char *mac, char *name)
{
	if (max == 0) return -1;

	memcpy(mac, psplist[pos].mac, 6);
	strcpy(name, psplist[pos].name);

	return 1;
}


/*--------------------------------------------------------
	Matching callback
--------------------------------------------------------*/

static void matchingCallback(int unk1, int event, char *mac2, int optLen, char *optData)
{
	switch (event)
	{
	case MATCHING_JOINED:
		AddPsp(mac2, optData, optLen);
		break;

	case MATCHING_DISCONNECT:
		DelPsp(mac2);
		break;

	default:
		g_unk1        = unk1;
		g_matchEvent  = event;
		g_matchOptLen = optLen;
		strncpy(g_matchOptData, optData, optLen);
		memcpy(g_mac, mac2, sizeof(char) * 6);
		matchChanged = 1;
		break;
	}
}


/***************************************************************************
	AdHocインタフェース関数
***************************************************************************/

/*--------------------------------------------------------
	モジュールのロード
--------------------------------------------------------*/

int pspSdkLoadAdhocModules(void)
{
	int modID;

	modID = pspSdkLoadStartModule("flash0:/kd/ifhandle.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (modID < 0)
		return modID;

	modID = pspSdkLoadStartModule("flash0:/kd/memab.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (modID < 0)
		return modID;

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhoc_auth.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (modID < 0)
		return modID;

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhoc.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhocctl.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhoc_matching.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();

	return 0;
}


/*--------------------------------------------------------
	初期化
--------------------------------------------------------*/

int adhocInit(char *matchingData)
{
	struct productStruct product;
	int error_code;
	char mac[10];
	char *MatchingData = "unknown\0";

	// 変数初期化
	NetInit                = 0;
	NetAdhocInit           = 0;
	NetAdhocctlInit        = 0;
	NetAdhocctlConnect     = 0;
	NetAdhocPdpCreate      = 0;
	NetAdhocMatchingInit   = 0;
	NetAdhocMatchingCreate = 0;
	NetAdhocMatchingStart  = 0;

	Server = 0;

	g_unk1        = 0;
	g_matchEvent  = 0;
	g_matchOptLen = 0;
	matchChanged  = 0;
	memset(g_mac, 0, sizeof(g_mac));

	strcpy(product.product, "ULJP99999");
	product.unknown = 0;

	ClearPspList();

	ScreenInit();
	SetTextColor(0xffffffff);

	if (strlen(matchingData))
	{
		MatchingData = matchingData;
	}

	Printf("[Initialize AdHoc]\n");

	Printf("sceNetInit()...");
	if ((error_code = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000)) != 0)
	{
		Printf("failed (error code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetInit = 1;

	Printf("sceNetAdhocInit()...");
	if ((error_code = sceNetAdhocInit()) != 0)
	{
		Printf("failed. (error code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocInit = 1;

	Printf("sceNetAdhocctlInit()...");
	if ((error_code = sceNetAdhocctlInit(0x2000, 0x20, &product)) != 0)
	{
		Printf("failed. (error code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocctlInit = 1;

	Printf("sceNetAdhocctlConnect()...");
	if ((error_code = sceNetAdhocctlConnect("")) != 0)
	{
		Printf("failed. (error code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocctlConnect = 1;

	Printf("sceNetApctlGetState() - Connecting...");
	while (1)
	{
		int state;

		if ((error_code = sceNetAdhocctlGetState(&state)) != 0)
		{
			Printf("failed. (error code = %08x)\n", error_code);
			sceKernelDelayThread(10 * 1000000);
			return error_code;
		}
		if (state == 1) break;

		sceKernelDelayThread(50 * 1000);
	}
	Printf("ok\n");

	sceWlanGetEtherAddr(mac);

	Printf("sceNetAdhocPdpCreate()...");
	if ((pdpId = sceNetAdhocPdpCreate(mac, 0x309, 0x400, 0)) <= 0)
	{
		Printf("failed. (pdpId = %08x)\n", pdpId);
		return -1;
	}
	Printf("ok\n");
	NetAdhocPdpCreate = 1;

	Printf("sceNetAdhocMatchingInit()...");
	if ((error_code = sceNetAdhocMatchingInit(0x20000)) != 0)
	{
		Printf("failed. (error code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocMatchingInit = 1;

	Printf("sceNetAdhocMatchingCreate()...");
	if ((matchingId = sceNetAdhocMatchingCreate(MATCHING_CREATE_PARAMS)) < 0)
	{
		Printf("failed. (error code = %08x)\n", matchingId);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocMatchingCreate = 1;

	Printf("sceNetAdhocMatchingStart()...");
	if ((error_code = sceNetAdhocMatchingStart(MATCHING_START_PARAMS)) != 0)
	{
		Printf("failed. (error_code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocMatchingStart = 1;

	Printf("Complete.\n");

	sceKernelDelayThread(1000000);

	return 0;
}


/*--------------------------------------------------------
	切断
--------------------------------------------------------*/

int adhocTerm(void)
{
	int error_code;

	ScreenInit();
	Printf("[AdHoc Terminate]\n");

	if (NetAdhocMatchingStart)
	{
		Printf("sceNetAdhocMatchingStop()...");
		if ((error_code = sceNetAdhocMatchingStop(matchingId)) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocMatchingStart = 0;
	}

	if (NetAdhocMatchingCreate)
	{
		Printf("sceNetAdhocMatchingDelete()...");
		if ((error_code = sceNetAdhocMatchingDelete(matchingId)) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocMatchingCreate = 0;
	}

	if (NetAdhocMatchingInit)
	{
		Printf("sceNetAdhocMatchingTerm()...");
		if ((error_code = sceNetAdhocMatchingTerm()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocMatchingInit = 0;
	}

	if (NetAdhocPdpCreate)
	{
		Printf("sceNetAdhocPdpDelete()...");
		if ((error_code = sceNetAdhocPdpDelete(pdpId, 0)) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocPdpCreate = 0;
	}

	if (NetAdhocctlConnect)
	{
		Printf("sceNetAdhocctlDisconnect()...");
		if ((error_code = sceNetAdhocctlDisconnect()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocctlConnect = 0;
	}

	if (NetAdhocctlInit)
	{
		Printf("sceNetAdhocctlTerm()...");
		if ((error_code = sceNetAdhocctlTerm()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocctlInit = 0;
	}

	if (NetAdhocInit)
	{
		Printf("sceNetAdhocTerm()...");
		if ((error_code = sceNetAdhocTerm()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocInit = 0;
	}

	if (NetInit)
	{
		Printf("sceNetTerm()...");
		if ((error_code = sceNetTerm()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetInit = 0;
	}

	Printf("Complete.\n");
	pad_wait_clear();

	sceKernelDelayThread(1000000);

	return 0;
}


/*--------------------------------------------------------
	SSIDを指定して再接続
--------------------------------------------------------*/

int adhocReconnect(char *ssid)
{
	int error_code = 0;
	char mac[10];
	TICKER start, end;

	start = ticker();

	ScreenInit();
	Printf("[AdHoc Reconnect]\n");

	if (NetAdhocMatchingStart)
	{
		Printf("sceNetAdhocMatchingStop()...");
		if ((error_code = sceNetAdhocMatchingStop(matchingId)) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocMatchingStart = 0;
	}

	if (NetAdhocMatchingCreate)
	{
		Printf("sceNetAdhocMatchingDelete()...");
		if ((error_code = sceNetAdhocMatchingDelete(matchingId)) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocMatchingCreate = 0;
	}

	if (NetAdhocMatchingInit)
	{
		Printf("sceNetAdhocMatchingTerm()...");
		if ((error_code = sceNetAdhocMatchingTerm()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocMatchingInit = 0;
	}

	if (NetAdhocPdpCreate)
	{
		Printf("sceNetAdhocPdpDelete()...");
		if ((error_code = sceNetAdhocPdpDelete(pdpId, 0)) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocPdpCreate = 0;
	}

	if (NetAdhocctlConnect)
	{
		Printf("sceNetAdhocctlDisconnect()...");
		if ((error_code = sceNetAdhocctlDisconnect()) != 0)
			Printf("returned %08x\n", error_code);
		else
			Printf("ok\n");
		NetAdhocctlConnect = 0;
	}

	Printf("sceNetApctlGetState() - Disconnecting...");
	while (1)
	{
		int state;

		if ((error_code = sceNetAdhocctlGetState(&state)) != 0)
		{
			Printf("failed. (error code = %08x)\n", error_code);
			sceKernelDelayThread(10 * 1000000);
			return error_code;
		}
		if (state == 0) break;

		sceKernelDelayThread(50 * 1000);
	}
	Printf("ok\n");

	Printf("sceNetAdhocctlConnect()...");
	if ((error_code = sceNetAdhocctlConnect(ssid)) != 0)
	{
		Printf("failed. (error code = %08x)\n", error_code);
		return error_code;
	}
	Printf("ok\n");
	NetAdhocctlConnect = 1;

	Printf("sceNetApctlGetState() - Connecting...");
	while (1)
	{
		int state;

		if ((error_code = sceNetAdhocctlGetState(&state)) != 0)
		{
			Printf("failed. (error code = %08x)\n", error_code);
			sceKernelDelayThread(10 * 1000000);
			return error_code;
		}
		if (state == 1) break;

		sceKernelDelayThread(50 * 1000);
	}
	Printf("ok\n");

	sceWlanGetEtherAddr(mac);

	Printf("sceNetAdhocPdpCreate()...");
	if ((pdpId = sceNetAdhocPdpCreate(mac, 0x309, 0x800, 0)) <= 0)
	{
		Printf("failed. (pdpId = %08x)\n", pdpId);
		return -1;
	}
	Printf("ok\n");
	NetAdhocPdpCreate = 1;

	Printf("Complete.\n");

	do
	{
		end = ticker();
		video_wait_vsync();
	} while ((u32)(end - start) < 12*1000000);

	return 0;
}


/*--------------------------------------------------------
	接続先の選択
--------------------------------------------------------*/

int adhocSelect(void)
{
	int top = 0;
#ifdef ADHOC_USE_GUI
	int rows = 11;
#else
	int rows = 32;
#endif
	int currentState = PSP_LISTING;
	int prev_max = 0;
	int update = 1;
	char mac[10];
	char name[64];
	char temp[64];
	char macAddr[10];
	char *tempMac;
	char ssid[10];

	ScreenInit();

	while (1)
	{
		pad_update();

		SetTextColor(0xffff);

		switch (currentState)
		{
		case PSP_LISTING:
			Server = 0;
			if (update)
			{
				ScreenInit();
#ifdef ADHOC_USE_GUI
				Printf("Select a server to connect to, or " FONT_TRIANGLE " to return.\n");
#else
				Printf("Select a server to connect to, or TRIANGLE to return.\n");
#endif
				Printf("\n");
				DisplayPspList(top, rows);
				update = 0;
			}
			if (pad_pressed(PSP_CTRL_UP))
			{
				if (pos > 0) pos--;
				update = 1;
			}
			else if (pad_pressed(PSP_CTRL_DOWN))
			{
				if (pos < max - 1) pos++;
				update = 1;
			}
			else if (pad_pressed(PSP_CTRL_CIRCLE))
			{
				if (GetPspEntry(mac, name) > 0)
				{
					currentState = PSP_SELECTING;
					sceNetAdhocMatchingSelectTarget(matchingId, mac, 0, 0);
					update = 1;
				}
			}
			else if (pad_pressed(PSP_CTRL_TRIANGLE))
			{
				SetTextColor(0xffffffff);
				return -1;
			}
			if (matchChanged)
			{
				if (g_matchEvent == MATCHING_SELECTED)
				{
					memcpy(mac, g_mac, 6);
					strcpy(name, g_matchOptData);
					currentState = PSP_SELECTED;
				}
				update = 1;
			}
			break;

		case PSP_SELECTING:
			if (update)
			{
				ScreenInit();
				sceNetEtherNtostr((u8 *)mac, temp);
				Printf("Waiting for %s to accept the connection.\n", temp);
#ifdef ADHOC_USE_GUI
				Printf("To cancel press " FONT_CROSS ".\n");
#else
				Printf("To cancel press CROSS.\n");
#endif
				update = 0;
			}
			if (pad_pressed(PSP_CTRL_CROSS))
			{
				sceNetAdhocMatchingCancelTarget(matchingId, mac);
				currentState = PSP_LISTING;
				update = 1;
			}
			if (matchChanged)
			{
				switch (g_matchEvent)
				{
				case MATCHING_SELECTED:
					sceNetAdhocMatchingCancelTarget(matchingId, mac);
					break;

				case MATCHING_ESTABLISHED:
					currentState = PSP_ESTABLISHED;
					break;

				case MATCHING_REJECTED:
					currentState = PSP_LISTING;
					break;
				}
				update = 1;
			}
			break;

		case PSP_SELECTED:
			Server = 1;
			if (update)
			{
				ScreenInit();
				sceNetEtherNtostr((u8 *)mac, temp);
				Printf("%s has requested a connection.\n", temp);
#ifdef ADHOC_USE_GUI
				Printf("To accept the connection press " FONT_CIRCLE ", to cancel press " FONT_CROSS ".\n");
#else
				Printf("To accept the connection press CIRCLE, to cancel press CROSS.\n");
#endif
				update = 0;
			}
			if (pad_pressed(PSP_CTRL_CROSS))
			{
				sceNetAdhocMatchingCancelTarget(matchingId, mac);
				currentState = PSP_LISTING;
				update = 1;
			}
			else if (pad_pressed(PSP_CTRL_CIRCLE))
			{
				sceNetAdhocMatchingSelectTarget(matchingId, mac, 0, 0);
				currentState = PSP_WAIT_EST;
				update = 1;
			}
			if (matchChanged)
			{
				if (g_matchEvent == MATCHING_CANCELED)
				{
					currentState = PSP_LISTING;
				}
				update = 1;
			}
			break;

		case PSP_WAIT_EST:
			if (matchChanged)
			{
				if (g_matchEvent == MATCHING_ESTABLISHED)
				{
					currentState = PSP_ESTABLISHED;
				}
				update = 1;
			}
			break;
		}

		matchChanged = 0;
		if (currentState == PSP_ESTABLISHED)
			break;

		if (top > max - rows) top = max - rows;
		if (top < 0) top = 0;
		if (pos >= top + rows) top = pos - rows + 1;
		if (pos < top) top = pos;

		if (max != prev_max)
		{
			prev_max = max;
			update = 1;
		}

		sceDisplayWaitVblankStart();
	}

	SetTextColor(0xffffffff);

	if (Server)
	{
		sceWlanGetEtherAddr(macAddr);
		tempMac = macAddr;
	}
	else
	{
		tempMac = mac;
	}

	sceNetEtherNtostr((u8 *)tempMac, temp);

	ssid[0] = temp[ 9];
	ssid[1] = temp[10];
	ssid[2] = temp[12];
	ssid[3] = temp[13];
	ssid[4] = temp[15];
	ssid[5] = temp[16];
	ssid[6] = '\0';
	adhocReconnect(ssid);

	Printf("\n");
	Printf("Connected.\n");

	if (Server)
		return 1;
	else
		return 0;
}


/*--------------------------------------------------------
	データ送信 (低レベル)
--------------------------------------------------------*/

int adhocSend(void *buffer, int length)
{
	return sceNetAdhocPdpSend(pdpId, g_mac, 0x309, buffer, length, 0, 1);
}


/*--------------------------------------------------------
	データ受信 (低レベル)
--------------------------------------------------------*/

int adhocRecv(void *buffer, int *length)
{
	int error_code = 0;
	int pdpStatLength = 20;
	int port = 0;
	char mac[20];
	u32 pdpRecvLength = 0;
	pdpStatStruct pspStat;

	if ((error_code = sceNetAdhocGetPdpStat(&pdpStatLength, &pspStat)) < 0)
	{
//		Printf("sceNetAdhocGetPdpStat() (error_code = %08x)\n", error_code);
		return error_code;
	}

	if (pspStat.rcvdData > 0)
	{
		pdpRecvLength = pspStat.rcvdData;
		if ((error_code = sceNetAdhocPdpRecv(pdpId, mac, &port, buffer, &length, 0, 1)) < 0)
		{
//			Printf("sceNetAdhocPdpRecv() (error_code = %08x)\n", error_code);
			return error_code;
		}
		return 1;
	}

	return 0;
}


/*--------------------------------------------------------
	データを受信するまで待つ (ack受信用)
--------------------------------------------------------*/

int adhocRecvBlocked(void *buffer, int *length)
{
	int error_code = 0;

	do
	{
		error_code = adhocRecv(buffer, length);
		sceKernelDelayThread(1000);
	} while (error_code == 0);

	return error_code;
}


/*--------------------------------------------------------
	データを送信し、ackを受信するまで待つ
--------------------------------------------------------*/

int adhocSendRecvAck(void *buffer, int length)
{
	int error_code = 0;
	int recvTemp   = 0;
	int recvLen    = 0;
	int tempLen    = length;
	int sentCount  = 0;

	do
	{
		if (tempLen > 0x400)
			tempLen = 0x400;

		if ((error_code = adhocSend(buffer, tempLen)) < 0)
			return error_code;

		if ((error_code = adhocRecvBlocked(&recvTemp, &recvLen)) < 0)
			return error_code;

		buffer += 0x400;
		sentCount += 0x400;
		tempLen = length - sentCount;
	} while (sentCount < length);

	return error_code;
}


/*--------------------------------------------------------
	ackを受信してからデータを送信する
--------------------------------------------------------*/

int adhocRecvSendAck(void *buffer, int *length)
{
	int temp       = 1;
	int error_code = 0;
	int tempLen    = *length;
	int rcvdCount  = 0;

	do
	{
		if (tempLen > 0x400)
			tempLen = 0x400;

		if ((error_code = adhocRecvBlocked(buffer, length)) < 0)
			return error_code;

		if ((error_code = adhocSend(&temp, 1)) < 0)
			return error_code;

		rcvdCount += 0x400;
		buffer += 0x400;
		tempLen = *length - rcvdCount;
	} while (rcvdCount < *length);

	return error_code;
}


/***************************************************************************
	送受信のテスト
***************************************************************************/

int adhoc_test(void)
{
	int err, server;
	int size, length;
	char data[256];

	ScreenInit();

	if ((adhocInit("AdHoc test") >= 0) && ((server = adhocSelect()) >= 0))
	{
		SetTextColor(0x0000ffff);
		Printf("\n");
		Printf("## AdHoc INITALISED ##\n");
		Printf("\n");
		SetTextColor(0xffffffff);

		if (server)
		{
			Printf("Server Assigned - Sending data to the client.\n");
			Printf("\n");

			sceKernelDelayThread(5*1000000);

			strcpy(data, "abcdefghijklmnopqrstuvwxyz");

			size = strlen(data);	// 送信データ長
			err = adhocSendRecvAck(&size, 4);
			Printf("Done sending data size. (data: %d, length: 4 bytes)\n", size);
			Printf("\n");

			sceKernelDelayThread(1000000);

			err = adhocSendRecvAck(data, size);
			Printf("Done sending data. (data: %s, length: %d bytes)\n", data, size);
		}
		else
		{
			Printf("Client Assigned - Waiting for data.\n");
			Printf("\n");

			memset(data, 0, sizeof(data));

			length = 4;	// 受信バッファのサイズ
			err = adhocRecvSendAck(&size, &length);
			Printf("Done receiving data size. (data: %d, length: %d bytes)\n", size, sizeof(size));
			Printf("\n");

			length = sizeof(data);	// 受信バッファのサイズ
			err = adhocRecvSendAck(data, &length);
			Printf("Received data. (data: %s, length: %d bytes)\n", data, strlen(data));
		}
	}

	Printf("\n");
	Printf("Press any button to exit.\n");
	pad_wait_press(-1);

	adhocTerm();
}
