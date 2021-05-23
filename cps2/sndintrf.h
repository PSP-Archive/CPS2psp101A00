/******************************************************************************

	sndintrf.c

	CPS2 サウンドインタフェース

******************************************************************************/

#ifndef CPS2_SNDINTRF_H
#define CPS2_SNDINTRF_H

extern int option_sound_enable;
//extern int option_samplerate;
extern int option_sound_volume;

int cps2_sound_init(void);
void cps2_sound_exit(void);
void cps2_sound_reset(void);
void cps2_sound_mute(int mute);

#endif /* CPS2_SNDINTRF_H */
