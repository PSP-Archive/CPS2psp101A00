/******************************************************************************

	timer.c

	Timer functions.

******************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#define QSOUND_INTERRUPT		0
#define CPUSPIN_TIMER			1
#define VBLANK_INTERRUPT		2
#define RASTER_INTERRUPT1		3
#define RASTER_INTERRUPT2		4
#define MAX_TIMER				5

#define TIME_NOW				(0.0)
#define TIME_NEVER				(1.0e30)
#define TIME_IN_HZ(hz)			(1000000.0/(float)hz)

#define USECS_PER_SCANLINE		((1000000.0/FPS)/(float)RASTER_LINES)

#define SUSPEND_REASON_HALT		0x0001
#define SUSPEND_REASON_RESET	0x0002
#define SUSPEND_REASON_SPIN		0x0004
#define SUSPEND_REASON_TRIGGER	0x0008
#define SUSPEND_REASON_DISABLE	0x0010
#define SUSPEND_ANY_REASON		((u32)-1)

void timer_reset(void);
//void timer_suspend_cpu0(/*int cpunum,*/ int state, int reason);
void timer_suspend_cpu1(/*int cpunum,*/ int state, int reason);
int timer_enable(int which, int enable);
void timer_adjust(int which, float duration, int param, void (*callback)(int raram));
void timer_set(int which, float duration, int param, void (*callback)(int param));
double timer_get_time(void);
u32 timer_getcurrentframe(void);
void timer_update_cpu(void);

#ifdef SAVE_STATE
STATE_SAVE( timer );
STATE_LOAD( timer );
#endif

#endif /* TIMER_H */
