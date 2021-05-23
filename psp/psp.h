/******************************************************************************

	psp.c

	Initialize PSP and control threads.

******************************************************************************/

#ifndef PSP_MAIN_H
#define PSP_MAIN_H

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#include "emucfg.h"

#include <psptypes.h>
#include <pspaudio.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspiofilemgr.h>
#include <pspkernel.h>
#include <psppower.h>
#include <psprtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "psp/config.h"
#include "psp/filer.h"
#include "psp/help.h"
#include "psp/input.h"
#include "psp/menu.h"
#include "psp/mesbox.h"
#include "psp/misc.h"
#include "psp/sound.h"
#include "psp/ticker.h"
#include "psp/usrintrf.h"
#include "psp/video.h"
#ifdef ADHOC
#include "psp/adhoc.h"
#endif

enum
{
	LOOP_EXIT = 0,
	LOOP_BROWSER,
	LOOP_RESTART,
	LOOP_RESET,
	LOOP_EXEC
};

extern volatile int Loop;
extern volatile int Sleep;
extern char launchDir[MAX_PATH];

#endif /* PSP_MAIN_H */
