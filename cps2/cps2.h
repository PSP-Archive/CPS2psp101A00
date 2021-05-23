/******************************************************************************

	cps2.c

	CPS2エミュレーションコア

******************************************************************************/

#ifndef CPS2_H
#define CPS2_H

#include "emumain.h"
#include "state.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "cache.h"
#include "driver.h"
#include "eeprom.h"
#include "inptport.h"
#include "memintrf.h"
#include "sndintrf.h"
#include "sprite.h"
#include "timer.h"
#include "vidhrdw.h"
#include "sound/qsound.h"

#define FPS		59.633333

void cps2_main(void);

#endif /* CPS2_H */
