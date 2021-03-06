/******************************************************************************

	help.c

	PSP ヘルプ表示関数

******************************************************************************/

#ifndef PSP_HELP_H
#define PSP_HELP_H

enum
{
	HELP_FILEBROWSER = 0,
	HELP_GAMECONFIG,
	HELP_KEYCONFIG,
#if (CPS_VERSION == 1)
	HELP_DIPSWITCH,
#endif
#ifdef SAVE_STATE
	HELP_STATE,
#endif
#ifdef SOUND_TEST
	HELP_SOUNDTEST,
#endif
	HELP_NUM_MAX
};

int help(int no);

#endif /* PSP_HELP */
