/******************************************************************************

	timer.c

	Timer functions.

******************************************************************************/

#include "cps2.h"


#define CPU_NOTACTIVE	-1





/******************************************************************************
	ローカル構造体
******************************************************************************/

typedef struct timer_t
{
	float expire;
	int enable;
	int param;
	void (*callback)(int param);
} TIMER;

//typedef struct cpuinfo_t
//{
//	int (*execute)(int cycles);
//	int *icount;
//	float usec_to_cycles;
//	float cycles_to_usec;
//	int cycles;
//	int suspended;
//} CPUINFO;

/* uSec == micro seconds. */
#define  M68_USEC_TO_CYCLES (float)(11800000/1000000.0)
#define  Z80_USEC_TO_CYCLES (float)( 8000000/1000000.0)

#define  M68_CYCLES_TO_USEC (float)(1000000.0/11800000)
#define  Z80_CYCLES_TO_USEC (float)(1000000.0/ 8000000)

static TIMER timer[MAX_TIMER];

//static CPUINFO cpu[2/*MAX _CPU*/] =
//{
//	{
//	//	m68000_execute,		// execute
//		&m68k_ICount,		// icount
//		11800000/1000000.0,	// usec_to_cycles
//		1000000.0/11800000//,	// cycles_to_usec
//	//	0//,					// cycles
//	//	0					// suspended
//	},
//	{
//	//	z80_execute,		// execute
//		&z80_ICount,		// icount
//		8000000/1000000.0,	// usec_to_cycles
//		1000000.0/8000000//,	// cycles_to_usec
//	//	0//,					// cycles
//	//	0					// suspended
//	}
//};


/******************************************************************************
	ローカル変数
******************************************************************************/

static const float time_slice = (int)(1000000.0 / FPS);

static float global_offset;
static float base_time;
static float frame_base;
static float timer_ticks;
static float timer_left;
static int active_cpu;
static u32 current_frame;

static void qsound_int_callback(int param);

static int M68_cycles;
static int Z80_cycles;

static u8 M68_suspended;
static u8 Z80_suspended;
/*------------------------------------------------------
	CPUを実行
------------------------------------------------------*/
//static void cpu_execute0(void/*int cpunum*/)
//{
//	if (!cpu[cpunum].suspended)
//	{
//		active_cpu = cpunum;
//		cpu[cpunum].cycles = (int)(timer_ticks * cpu[cpunum].usec_to_cycles);
//		cpu[cpunum].execute(cpu[cpunum].cycles);
//		active_cpu = CPU_NOTACTIVE;
//	}
//}

/*------------------------------------------------------
	現在の秒以下の時間を取得 (単位:マイクロ秒)
------------------------------------------------------*/

static float getabsolutetime(void)
{
	float time = base_time + frame_base;

	if (active_cpu != CPU_NOTACTIVE){
		//CPUの消費した時間を取得 (単位:マイクロ秒)
		//time += cpu_elapsed_time(active_cpu);
		if(/*CPU_Z80*/1==active_cpu){
		//	time += (float)(cpu[1/*active_cpu*/].cycles - *cpu[1/*active_cpu*/].icount) * cpu[1/*active_cpu*/].cycles_to_usec;
			time += (float)(Z80_cycles - z80_ICount) * Z80_CYCLES_TO_USEC;
		}else{/*if(CPU_ M68000 0==active_cpu) */
		//	time += (float)(cpu[0/*active_cpu*/].cycles - *cpu[0/*active_cpu*/].icount) * cpu[0/*active_cpu*/].cycles_to_usec;
			time += (float)(M68_cycles - m68k_ICount) * M68_CYCLES_TO_USEC;
		}
	}
	return time;
}


/*------------------------------------------------------
	タイマーをリセット
------------------------------------------------------*/

void timer_reset(void)
{
	//int cpunum;

	global_offset = 0;
	base_time = 0;
	frame_base = 0;
	current_frame = 0;

	active_cpu = CPU_NOTACTIVE;
	memset(&timer, 0, sizeof(timer));

//	for (cpunum = 0; cpunum < MAX _CPU; cpunum++)
//	{
		M68_suspended=0;
		Z80_suspended=0;
		M68_cycles = 0;
		Z80_cycles = 0;

		//cpu[0/*cpunum*/].cycles = 0;
		//cpu[0/*cpunum*/].suspended = 0;
		//cpu[1/*cpunum*/].cycles = 0;
		//cpu[1/*cpunum*/].suspended = 0;
//	}

	timer_set(QSOUND_INTERRUPT, TIME_IN_HZ(251), 0, qsound_int_callback);
}


/*------------------------------------------------------
	CPUをサスペンドする
------------------------------------------------------*/
static void timer_suspend_cpu0(/*int cpunum,*/ int state, int reason)
{
//	if (state == 0)	cpu[0/*cpunum*/].suspended |= reason;
//	else			cpu[0/*cpunum*/].suspended &= ~reason;
	if (state == 0)	M68_suspended |= reason;
	else			M68_suspended &= ~reason;
}
void timer_suspend_cpu1(/*int cpunum,*/ int state, int reason)
{
//	if (state == 0)	cpu[1/*cpunum*/].suspended |= reason;
//	else			cpu[1/*cpunum*/].suspended &= ~reason;
	if (state == 0)	Z80_suspended |= reason;
	else			Z80_suspended &= ~reason;
}
/*------------------------------------------------------
	CPUのスピンを解除(トリガ)
------------------------------------------------------*/
static void cpu_spin_trigger(int param)
{
	timer_suspend_cpu0(/*param,*/ 1, SUSPEND_REASON_SPIN);
}


/*------------------------------------------------------
	タイマーを有効/無効にする
------------------------------------------------------*/

int timer_enable(int which, int enable)
{
	int old = timer[which].enable;

	timer[which].enable = enable;
	return old;
}


/*------------------------------------------------------
	タイマーをセット
------------------------------------------------------*/

void timer_adjust(int which, float duration, int param, void (*callback)(int param))
{
	float time = getabsolutetime();

	timer[which].expire = time + duration;
	timer[which].param = param;
	timer[which].callback = callback;

	if (active_cpu != CPU_NOTACTIVE)
	{
		if(/*CPU _Z80*/1==active_cpu){
			// CPU実行中の場合は、残りサイクルを破棄
		int cycles_left;
		float time_left;
		//	cycles_left = *cpu[1/*Z80 active_cpu*/].icount;
			cycles_left = z80_ICount;
		//	time_left = cycles_left * cpu[1/*Z80 active_cpu*/].cycles_to_usec;
			time_left = cycles_left * Z80_CYCLES_TO_USEC;
			if (duration < timer_left)
			{
				timer_ticks -= time_left;
			//	cpu[1/*Z80 active_cpu*/].cycles -= cycles_left;
				Z80_cycles -= cycles_left;
			//	*cpu[1/*Z80 active_cpu*/].icount = 0;
				z80_ICount = 0;
				// /*CPU2*/Z80の場合は/*CPU1*/m68000を停止しCPU1が消費した余分なサイクルを調整する
				if (!timer[CPUSPIN_TIMER].enable)
				{
					timer_suspend_cpu0(/*CPU _M68000,*/ 0, SUSPEND_REASON_SPIN);
					timer[CPUSPIN_TIMER].enable = 1;
					timer[CPUSPIN_TIMER].expire = time + time_left;
					timer[CPUSPIN_TIMER].param = 0/*CPU _M68000*/;
					timer[CPUSPIN_TIMER].callback = cpu_spin_trigger;
				}
			}
		}else/* if(CPU _M68000 0==active_cpu) */{
			// CPU実行中の場合は、残りサイクルを破棄
		int cycles_left;
		float time_left;
		//	cycles_left = *cpu[0/*M68 active_cpu*/].icount;
			cycles_left = m68k_ICount;
		//	time_left = cycles_left * cpu[0/*M68 active_cpu*/].cycles_to_usec;
			time_left = cycles_left * M68_CYCLES_TO_USEC;
			if (duration < timer_left)
			{
				timer_ticks -= time_left;
			//	cpu[0/*M68 active_cpu*/].cycles -= cycles_left;
				M68_cycles -= cycles_left;
			//	*cpu[0/*M68 active_cpu*/].icount = 0;
				m68k_ICount = 0;
			}
		}
	}
}


/*------------------------------------------------------
	タイマーをセット
------------------------------------------------------*/

void timer_set(int which, float duration, int param, void (*callback)(int param))
{
	timer[which].enable = 1;
	timer_adjust(which, duration, param, callback);
}


/*------------------------------------------------------
	現在のエミュレーション時間を取得 (単位:秒)
------------------------------------------------------*/

double timer_get_time(void)
{
	return (double)(global_offset + (getabsolutetime() / 1000000.0));
}


/*------------------------------------------------------
	現在のフレームを取得
------------------------------------------------------*/

u32 timer_getcurrentframe(void)
{
	return current_frame;
}


/*------------------------------------------------------
	描画割り込み
------------------------------------------------------*/

static void raster_int_callback(int param)
{
	int line = param & 0xffff;
	int which = param >> 16;

	cps2_raster_interrupt(line, which);
}


static void timer_set_raster_interrupt(int which, int scanline)
{
	int param = (which << 16) | scanline;

	timer_set(which, USECS_PER_SCANLINE * scanline, param, raster_int_callback);
}


static void vblank_int_callback(int param)
{
	cps2_vblank_interrupt();
}


static void timer_set_vblank_interrupt(void)
{
	timer_set(VBLANK_INTERRUPT, USECS_PER_SCANLINE * 256, 0, vblank_int_callback);
}


/*------------------------------------------------------
	サウンド割り込み
------------------------------------------------------*/

static void qsound_int_callback(int param)
{
	z80_set_irq_line0_HOLD(/*0, HOLD_LINE*/);
	timer_set(QSOUND_INTERRUPT, TIME_IN_HZ(251), 0, qsound_int_callback);
}


/*------------------------------------------------------
	CPUを更新
------------------------------------------------------*/

void timer_update_cpu(void)
{
	int i;
	float time;
	extern int scanline1;
	extern int scanline2;

	frame_base = 0;
	timer_left = time_slice;

	if (scanline1 != RASTER_LINES) timer_set_raster_interrupt(RASTER_INTERRUPT1, scanline1);
	if (scanline2 != RASTER_LINES) timer_set_raster_interrupt(RASTER_INTERRUPT2, scanline2);
	timer_set_vblank_interrupt();

	if (cps2_build_palette) (*cps2_build_palette)();

	while (timer_left > 0)
	{
		timer_ticks = timer_left;
		time = base_time + frame_base;

		for (i = 0; i < MAX_TIMER; i++)
		{
			if (timer[i].enable)
			{
				if (timer[i].expire - time <= 0)
				{
					timer[i].enable = 0;
					timer[i].callback(timer[i].param);
				}
			}
			if (timer[i].enable)
			{
				if (timer[i].expire - time < timer_ticks)
					timer_ticks = timer[i].expire - time;
			}
		}

		//for (i = 0; i < MAX _CPU; i++)
		//cpu_execute(0/*i*/);
		//cpu_execute(1/*i*/);
		//if (!cpu[0/*cpunum*/].suspended)
		if (!M68_suspended)
		{
			active_cpu = 0/*cpunum*/;
		//	cpu[0/*cpunum*/].cycles = (int)(timer_ticks * cpu[0/*cpunum*/].usec_to_cycles);
			M68_cycles = (int)(timer_ticks * M68_USEC_TO_CYCLES);
		//	m68000_execute/*cpu[0 cpunum].execute*/(cpu[0/*cpunum*/].cycles);
			m68000_execute(M68_cycles);
			active_cpu = CPU_NOTACTIVE;
		}
		//if (!cpu[1/*cpunum*/].suspended)
		if (!Z80_suspended)
		{
			active_cpu = 1/*cpunum*/;
		//	cpu[1/*cpunum*/].cycles = (int)(timer_ticks * cpu[1/*cpunum*/].usec_to_cycles);
		//	z80_execute/*cpu[1 cpunum].execute*/(cpu[1/*cpunum*/].cycles);
			Z80_cycles = (int)(timer_ticks * Z80_USEC_TO_CYCLES);
			z80_execute(Z80_cycles);
			active_cpu = CPU_NOTACTIVE;
		}

		frame_base += timer_ticks;
		timer_left -= timer_ticks;
	}

	base_time += time_slice;
	if (base_time >= 1000000.0)
	{
		global_offset += 1.0;
		base_time -= 1000000.0;

		for (i = 0; i < MAX_TIMER; i++)
		{
			if (timer[i].enable)
				timer[i].expire -= 1000000.0;
		}
	}

	current_frame++;
}


/*------------------------------------------------------
	セーブ/ロード ステート
------------------------------------------------------*/

#ifdef SAVE_STATE

STATE_SAVE( timer )
{
	state_save_float(&global_offset, 1);
	state_save_float(&base_time, 1);
	state_save_long(&current_frame, 1);

//	state_save_long(&cpu[0/*M68  */].suspended, 1);
//	state_save_long(&cpu[1/*Z80 */].suspended, 1);
//	state_save_long(&M68_suspended, 1);
//	state_save_long(&Z80_suspended, 1);

	{ u32 M68_sss,Z80_sss;
		M68_sss=(u32)M68_suspended;	state_save_long(&M68_sss, 1);
		Z80_sss=(u32)Z80_suspended;	state_save_long(&Z80_sss, 1);
	}

	{int i;
		for (i = 0; i < MAX_TIMER; i++)
		{
			state_save_float(&timer[i].expire, 1);
			state_save_long(&timer[i].enable, 1);
			state_save_long(&timer[i].param, 1);
		}
	}
}

STATE_LOAD( timer )
{
	state_load_float(&global_offset, 1);
	state_load_float(&base_time, 1);
	state_load_long(&current_frame, 1);

//	state_load_long(&cpu[0].suspended, 1);
//	state_load_long(&cpu[1].suspended, 1);
//	state_load_long(&M68_suspended, 1);
//	state_load_long(&Z80_suspended, 1);

	{ u32 M68_sss,Z80_sss;
		state_load_long(&M68_sss, 1);	M68_suspended=(u8)M68_sss;
		state_load_long(&Z80_sss, 1);	Z80_suspended=(u8)Z80_sss;
	}
	{int i;
		for (i = 0; i < MAX_TIMER; i++)
		{
			state_load_float(&timer[i].expire, 1);
			state_load_long(&timer[i].enable, 1);
			state_load_long(&timer[i].param, 1);
		}
	}

	timer_left  = 0;
	timer_ticks = 0;
	frame_base  = 0;
	active_cpu = CPU_NOTACTIVE;

	timer[QSOUND_INTERRUPT].callback  = qsound_int_callback;
	timer[CPUSPIN_TIMER].callback     = cpu_spin_trigger;
	timer[VBLANK_INTERRUPT].callback  = vblank_int_callback;
	timer[RASTER_INTERRUPT1].callback = raster_int_callback;
	timer[RASTER_INTERRUPT2].callback = raster_int_callback;
}

#endif /* SAVE_STATE */
