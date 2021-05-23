/******************************************************************************

	emumain.c

	エミュレーションコア

******************************************************************************/

#ifndef EMUMAIN_H
#define EMUMAIN_H

#include "psp/psp.h"
#include "include/osd_cpu.h"
#include "include/cpuintrf.h"
#include "include/memory.h"
#include "zip/zfile.h"

#ifdef ADHOC
#define send_byte(v, n)		{ memcpy(buf, v, 1 * n); buf += 1 * n; }
#define send_word(v, n)		{ memcpy(buf, v, 2 * n); buf += 2 * n; }
#define send_long(v, n)		{ memcpy(buf, v, 4 * n); buf += 4 * n; }
#define send_double(v, n)	{ memcpy(buf, v, 8 * n); buf += 8 * n; }
#define recv_byte(v, n)		{ memcpy(v, buf, 1 * n); buf += 1 * n; }
#define recv_word(v, n)		{ memcpy(v, buf, 2 * n); buf += 2 * n; }
#define recv_long(v, n)		{ memcpy(v, buf, 4 * n); buf += 4 * n; }
#define recv_double(v, n)	{ memcpy(v, buf, 8 * n); buf += 8 * n; }
#define ADHOC_SEND(name)	void adhoc_send_##name(u8 *buffer)
#define ADHOC_RECV(name)	void adhoc_recv_##name(u8 *buffer)
#endif

#if (CPS_VERSION == 2)
#include "cps2/cps2.h"
#else
#include "cps1/cps1.h"
#endif

extern char game_name[16];
extern char parent_name[16];
extern char game_dir[MAX_PATH];

#if (CPS_VERSION == 2)
extern char cache_parent_name[16];
extern char cache_dir[MAX_PATH];
#endif

extern int option_showfps;
extern int option_autoframeskip;
extern int option_frameskip;
extern int option_speedlimit;
extern int option_vsync;
extern int option_stretch;

extern u32 frames_displayed;
extern int fatal_error;

#ifdef ADHOC
extern int adhoc_enable;
extern int adhoc_server;
#endif


void emu_main(void);

void reset_frameskip(void);

u8 skip_this_frame(void);
void update_screen(void);

void fatalerror(const char *text, ...);
void show_fatal_error(void);

void save_snapshot(void);

#endif /* EMUMAIN_H */
