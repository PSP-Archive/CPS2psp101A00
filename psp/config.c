/******************************************************************************

	config.c

	アプリケーション設定ファイル管理

******************************************************************************/

#include "psp.h"
#include "emumain.h"

#define LINEBUF_SIZE	256


enum
{
	CFG_NONE = 0,
	CFG_INT,
	CFG_BOOL,
	CFG_PAD,
	CFG_STR
};

enum
{
	PAD_NONE = 0,
	PAD_UP,
	PAD_DOWN,
	PAD_LEFT,
	PAD_RIGHT,
	PAD_CIRCLE,
	PAD_CROSS,
	PAD_SQUARE,
	PAD_TRIANGLE,
	PAD_LTRIGGER,
	PAD_RTRIGGER,
	PAD_START,
	PAD_SELECT,
	PAD_MAX
};

typedef struct cfg_t
{
	int type;
	const char *name;
	int *value;
	int def;
	int max;
} cfg_type;

typedef struct cfg2_t
{
	int type;
	const char *name;
	char *value;
	int max_len;
} cfg2_type;


/******************************************************************************
	グローバル変数
******************************************************************************/

#define INIVERSION	3

static const char *inifile_name = "cps2psp.ini";
static int ini_version;


/******************************************************************************
	ローカル構造体/変数
******************************************************************************/

static cfg_type default_options[] =
{
	{ CFG_NONE,	"[System Settings]", },
	{ CFG_INT,	"INIFileVersion",	&ini_version,	INIVERSION,	INIVERSION		},
#ifdef ADHOC
	{ CFG_INT,	"PSPClock",			&psp_cpuclock,	PSPCLOCK_222,PSPCLOCK_222	},
#else
	{ CFG_INT,	"PSPClock",			&psp_cpuclock,	PSPCLOCK_333,PSPCLOCK_333	},
#endif
	{ CFG_NONE, NULL, }
};

static cfg2_type default_options2[] =
{
	{ CFG_NONE,	"[Directory Settings]", 				},
	{ CFG_STR,	"StartupDir", startupDir,	MAX_PATH	},
	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_2buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_2buttons_rot[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"RotateScreen",			&cps_rotate_screen,		1,	1	},
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_3buttons_rot[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"RotateScreen",			&cps_rotate_screen,		1,	1	},
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_3buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_4buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",	&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_6buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",	&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button5",				&input_map[P1_BUTTON5],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Button6",				&input_map[P1_BUTTON6],	PSP_CTRL_RTRIGGER,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_PAD,	"Autofire4",			&input_map[P1_AF_4],	0,	0	},
	{ CFG_PAD,	"Autofire5",			&input_map[P1_AF_5],	0,	0	},
	{ CFG_PAD,	"Autofire6",			&input_map[P1_AF_6],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_qndream[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_progear[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Start2",				&input_map[P2_START],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_puzloop2[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"WaitVsync",			&option_vsync,			0,	0	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
//	{ CFG_INT,	"SampleRate",			&option_samplerate,		1,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[CPS Analog Input Settings]", },
	{ CFG_PAD,	"PaddleLeft",			&input_map[P1_DIAL_L],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"PaddleRight",			&input_map[P1_DIAL_R],	PSP_CTRL_RTRIGGER,	0	},
	{ CFG_INT,	"Sensitivity",			&analog_sensitivity,	1,					2	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

typedef struct padname_t
{
	int code;
	const char name[16];
} PADNAME;

static const PADNAME pad_name[13] =
{
	{ 0,					"PAD_NONE"		},
	{ PSP_CTRL_UP,			"PAD_UP"		},
	{ PSP_CTRL_DOWN,		"PAD_DOWN"		},
	{ PSP_CTRL_LEFT,		"PAD_LEFT"		},
	{ PSP_CTRL_RIGHT,		"PAD_RIGHT"		},
	{ PSP_CTRL_CROSS,		"PAD_CROSS"		},
	{ PSP_CTRL_CIRCLE,		"PAD_CIRCLE"	},
	{ PSP_CTRL_SQUARE,		"PAD_SQUARE"	},
	{ PSP_CTRL_TRIANGLE,	"PAD_TRIANGLE"	},
	{ PSP_CTRL_START,		"PAD_START"		},
	{ PSP_CTRL_SELECT,		"PAD_SELECT"	},
	{ PSP_CTRL_LTRIGGER,	"PAD_LTRIGGER"	},
	{ PSP_CTRL_RTRIGGER,	"PAD_RTRIGGER"	}
};


/******************************************************************************
	ローカル関数
******************************************************************************/

/*------------------------------------------------------
	CFG_BOOLの値を読み込む
------------------------------------------------------*/

static int get_config_bool(char *str)
{
	if (!stricmp(str, "yes"))
		return 1;
	else
		return 0;
}


/*------------------------------------------------------
	CFG_INTの値を読み込む
------------------------------------------------------*/

static int get_config_int(char *str, int maxval)
{
	int value = atoi(str);

	if (value < 0) value = 0;
	if (value > maxval) value = maxval;
	return value;
}


/*------------------------------------------------------
	CFG_PADの値を読み込む
------------------------------------------------------*/

static int get_config_pad(char *str)
{
	int i;

	for (i = 0; i < PAD_MAX; i++)
	{
		if (strcmp(str, pad_name[i].name) == 0)
			return pad_name[i].code;
	}

	return pad_name[PAD_NONE].code;
}


/*------------------------------------------------------
	CFG_BOOLの値を保存する
------------------------------------------------------*/

static const char *set_config_bool(int value)
{
	if (value)
		return "yes";
	else
		return "no";
}


/*------------------------------------------------------
	CFG_INTの値を保存する
------------------------------------------------------*/

static char *set_config_int(int value, int maxval)
{
	static char buf[16];

	if (value < 0) value = 0;
	if (value > maxval) value = maxval;

	sprintf(buf, "%d", value);

	return buf;
}


/*------------------------------------------------------
	CFG_PADの値を保存する
------------------------------------------------------*/

static const char *set_config_pad(int value)
{
	int i;

	for (i = 0; i < PAD_MAX; i++)
	{
		if (value == pad_name[i].code)
			return pad_name[i].name;
	}

	return pad_name[PAD_NONE].name;
}


/*------------------------------------------------------
	.iniファイルから設定を読み込む
------------------------------------------------------*/

static int load_inifile(const char *path, cfg_type *cfg, cfg2_type *cfg2)
{
	FILE *fp;

	if ((fp = fopen(path, "r")) != NULL)
	{
		int i;
		char linebuf[LINEBUF_SIZE];

		while (1)
		{
			char *name, *value;

			memset(linebuf, LINEBUF_SIZE, 0);
			if (fgets(linebuf, LINEBUF_SIZE - 1, fp) == NULL)
				break;

			if (linebuf[0] == ';' || linebuf[0] == '[')
				continue;

			name = strtok(linebuf, " =\r\n");
			if (name == NULL)
				continue;

			value = strtok(NULL, " =\r\n");
			if (value == NULL)
				continue;

			/* check name and value */
			for (i = 0; cfg[i].name; i++)
			{
				if (!strcmp(name, cfg[i].name))
				{
					switch (cfg[i].type)
					{
					case CFG_INT:  *cfg[i].value = get_config_int(value, cfg[i].max); break;
					case CFG_BOOL: *cfg[i].value = get_config_bool(value); break;
					case CFG_PAD:  *cfg[i].value = get_config_pad(value); break;
					}
				}
			}
		}

		if (cfg2)
		{
			fseek(fp, 0, SEEK_SET);

			while (1)
			{
				char *name, *value;
				char *p1, *p2, temp[LINEBUF_SIZE];

				memset(linebuf, LINEBUF_SIZE, 0);
				if (fgets(linebuf, LINEBUF_SIZE - 1, fp) == NULL)
					break;

				strcpy(temp, linebuf);

				if (linebuf[0] == ';' || linebuf[0] == '[')
					continue;

				name = strtok(linebuf, " =\r\n");
				if (name == NULL)
					continue;

				value = strtok(NULL, " =\r\n");
				if (value == NULL)
					continue;

				p1 = strchr(temp, '\"');
				if (p1)
				{
					p2 = strchr(p1 + 1, '\"');
					if (p2)
					{
						value = p1 + 1;
						*p2 = '\0';
					}
				}

				/* check name and value */
				for (i = 0; cfg2[i].name; i++)
				{
					if (!strcmp(name, cfg2[i].name))
					{
						if (cfg2[i].type == CFG_STR)
						{
							memset(cfg2[i].value, 0, cfg2[i].max_len);
							strncpy(cfg2[i].value, value, cfg2[i].max_len - 1);
						}
					}
				}
			}
		}

		fclose(fp);

		return 1;
	}

	return 0;
}


/*------------------------------------------------------
	.iniファイルに設定を保存
------------------------------------------------------*/

static int save_inifile(const char *path, cfg_type *cfg, cfg2_type *cfg2)
{
	FILE *fp;

	if ((fp = fopen(path, "w")) != NULL)
	{
		int i;

		fprintf(fp, ";-------------------------------------------\r\n");
		fprintf(fp, "; " APPNAME_STR " " VERSION_STR "\r\n");
		fprintf(fp, ";-------------------------------------------\r\n");

		for (i = 0; cfg[i].name; i++)
		{
			switch (cfg[i].type)
			{
			case CFG_NONE: if (cfg[i].name) fprintf(fp, "\r\n%s\r\n", cfg[i].name); break;
			case CFG_INT:  fprintf(fp, "%s = %s\r\n", cfg[i].name, set_config_int(*cfg[i].value, cfg[i].max)); break;
			case CFG_BOOL: fprintf(fp, "%s = %s\r\n", cfg[i].name, set_config_bool(*cfg[i].value)); break;
			case CFG_PAD:  fprintf(fp, "%s = %s\r\n", cfg[i].name, set_config_pad(*cfg[i].value)); break;
			}
		}

		if (cfg2)
		{
			for (i = 0; cfg2[i].name; i++)
			{
				switch (cfg2[i].type)
				{
				case CFG_NONE: if (cfg2[i].name) fprintf(fp, "\r\n%s\r\n", cfg2[i].name); break;
				case CFG_STR:  fprintf(fp, "%s = \"%s\"\r\n", cfg2[i].name, cfg2[i].value); break;
				}
			}
		}

		fclose(fp);

		return 1;
	}

	return 0;
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*------------------------------------------------------
	アプリケーションの設定を読み込む
------------------------------------------------------*/

void load_settings(void)
{
	int i;
	char path[MAX_PATH];

	for (i = 0; default_options[i].name; i++)
	{
		if (default_options[i].value)
			*default_options[i].value = default_options[i].def;
	}

	sprintf(path, "%s%s", launchDir, inifile_name);

	if (load_inifile(path, default_options, default_options2) == 0)
	{
		save_settings();
	}
	else if (ini_version != INIVERSION)
	{
		for (i = 0; default_options[i].name; i++)
		{
			if (default_options[i].value)
				*default_options[i].value = default_options[i].def;
		}

		remove("cps2psp.ini");
		delete_files("nvram","nv");
		delete_files("config","ini");

		save_settings();
	}
}


/*------------------------------------------------------
	アプリケーションの設定を保存する
------------------------------------------------------*/

void save_settings(void)
{
#ifdef ADHOC
	if (!adhoc_enable)
	{
#endif
		char path[MAX_PATH];

		sprintf(path, "%s%s", launchDir, inifile_name);

		save_inifile(path, default_options, default_options2);
#ifdef ADHOC
	}
#endif
}


/*------------------------------------------------------
	ゲームの設定を読み込む
------------------------------------------------------*/

void load_gamecfg(const char *name)
{
	int i;
	char path[MAX_PATH];
	cfg_type *gamecfg;

	sprintf(path, "%sconfig/%s.ini", launchDir, name);

	memset(input_map, 0, sizeof(input_map));

	switch (cps_input_type)
	{
	case INPTYPE_19xx:
		gamecfg = gamecfg_2buttons_rot;
		break;

	case INPTYPE_daimahoo:
		gamecfg = gamecfg_3buttons_rot;
		break;

	case INPTYPE_batcir:
		gamecfg = gamecfg_2buttons;
		break;

	case INPTYPE_cybots:
	case INPTYPE_ddtod:
		gamecfg = gamecfg_4buttons;
		break;

	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		gamecfg = gamecfg_6buttons;
		break;

	case INPTYPE_qndream:
		gamecfg = gamecfg_qndream;
		break;

	case INPTYPE_puzloop2:
		gamecfg = gamecfg_puzloop2;
		break;

	default:
		gamecfg = gamecfg_3buttons;
		break;
	}

	if (!strcmp(driver->name, "progear"))
		gamecfg = gamecfg_progear;

	for (i = 0; gamecfg[i].name; i++)
	{
		if (gamecfg[i].value)
			*gamecfg[i].value = gamecfg[i].def;
	}

	if (!cps_screen_type) cps_rotate_screen = 0;

	if (load_inifile(path, gamecfg, NULL) == 0)
		save_gamecfg(name);
}


/*------------------------------------------------------
	ゲームの設定を保存する
------------------------------------------------------*/

void save_gamecfg(const char *name)
{
	char path[MAX_PATH];
	cfg_type *gamecfg;

	sprintf(path, "%sconfig/%s.ini", launchDir, name);

	switch (cps_input_type)
	{
	case INPTYPE_19xx:
		gamecfg = gamecfg_2buttons_rot;
		break;

	case INPTYPE_daimahoo:
		gamecfg = gamecfg_3buttons_rot;
		break;

	case INPTYPE_batcir:
		gamecfg = gamecfg_2buttons;
		break;

	case INPTYPE_cybots:
	case INPTYPE_ddtod:
		gamecfg = gamecfg_4buttons;
		break;

	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		gamecfg = gamecfg_6buttons;
		break;

	case INPTYPE_qndream:
		gamecfg = gamecfg_qndream;
		break;

	case INPTYPE_puzloop2:
		gamecfg = gamecfg_puzloop2;
		break;

	default:
		gamecfg = gamecfg_3buttons;
		break;
	}

	if (!strcmp(driver->name, "progear"))
		gamecfg = gamecfg_progear;

	save_inifile(path, gamecfg, NULL);
}
