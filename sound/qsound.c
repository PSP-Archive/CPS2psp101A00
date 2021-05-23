/***************************************************************************

  Capcom System QSound(tm)
  ========================

  Driver by Paul Leaman (paul@vortexcomputing.demon.co.uk)
		and Miguel Angel Horna (mahorna@teleline.es)

  A 16 channel stereo sample player.

  QSpace position is simulated by panning the sound in the stereo space.

  Register
  0	 xxbb	xx = unknown bb = start high address
  1	 ssss	ssss = sample start address
  2	 pitch
  3	 unknown (always 0x8000)
  4	 loop offset from end address
  5	 end
  6	 master channel volume
  7	 not used
  8	 Balance (left=0x0110  centre=0x0120 right=0x0130)
  9	 unknown (most fixed samples use 0 for this register)

  Many thanks to CAB (the author of Amuse), without whom this probably would
  never have been finished.

  If anybody has some information about this hardware, please send it to me
  to mahorna@teleline.es or 432937@cepsz.unizar.es.
  http://teleline.terra.es/personal/mahorna

***************************************************************************/

#include <math.h>
#include "cps2.h"
#include "psp/sound.h"

#define FORCE_44100 1


#define MAXOUT		(+32767)
#define MINOUT		(-32768)

#define Limit(val, max, min)			\
{										\
	if (val > max) val = max;			\
	else if (val < min) val = min;		\
}


#define QSOUND_CLOCK    4000000		/* default 4MHz clock */
#define QSOUND_CLOCKDIV 166			/* Clock divider */
#define QSOUND_CHANNELS 16

typedef s8  QSOUND_SRC_SAMPLE;
typedef s16 QSOUND_SAMPLE;
typedef s32 QSOUND_SAMPLE_MIX;

struct QSOUND_CHANNEL
{
	int bank;			/* bank (x16)	*/
	int address;		/* start address */
	int pitch;			/* pitch */
	int loop;			/* loop address */
	int end;			/* end address */
	int vol;			/* master volume */
	int pan;			/* Pan value */

	/* Work variables */
	int key;			/* Key on / key off */

	int lvol;			/* left volume */
	int rvol;			/* right volume */
	int lastdt;			/* last sample value */
	int offset;			/* current offset counter */
};


/* ローカル変数 */
static struct QSOUND_CHANNEL qsound_channel[QSOUND_CHANNELS];
static int qsound_data;					/* register latch data */
QSOUND_SRC_SAMPLE *qsound_sample_rom;	/* Q sound sample ROM */

static int qsound_pan_table[33];		/* Pan volume table */
static float qsound_frq_ratio;			/* Frequency ratio */
static int qsound_volume_shift;			/* Volume shift */
#if (0==FORCE_44100)
static int qsound_stream_type;			/* Stream type */
#endif
/* Function prototypes */
void qsound_set_command(int data, int value);


void qsound_set_samplerate(void)
{
	int samplerate = 736 * 60;
#if (0==FORCE_44100)
	if (!qsound_stream_type)
	{
		samplerate >>= (2 - option_samplerate/*2==44100*/);
	}
#endif
	qsound_frq_ratio = ((float)QSOUND_CLOCK / (float)QSOUND_CLOCKDIV) / (float)samplerate;
	qsound_frq_ratio *= 16.0;
}


void qsound_sh_start(void)
{
#if (0==FORCE_44100)
	if (!strcmp(driver->name, "dstlk")
	||	!strcmp(driver->name, "nwarr")
	||	!strcmp(driver->name, "sfa2")
	||	!strcmp(driver->name, "vsav")
	||	!strcmp(driver->name, "vhunt2")
	||	!strcmp(driver->name, "vsav2"))
	{
		qsound_stream_type = 1;
	}
	else
	{
		qsound_stream_type = 0;
	}
#endif
	if (!strcmp(driver->name, "csclub"))
	{
		qsound_volume_shift = 4;
	}
	else
	if (!strcmp(driver->name, "ddsom")
	||	!strcmp(driver->name, "vsav")
	||	!strcmp(driver->name, "vsav2"))
	{
		qsound_volume_shift = 5;
	}
	else
	if (!strcmp(driver->name, "batcir")
	||	!strcmp(driver->name, "spf2t")
	||	!strcmp(driver->name, "gigawing")
	||	!strcmp(driver->name, "mpangj")
	||	!strcmp(driver->name, "pzloop2j"))
	{
		qsound_volume_shift = 7;
	}
	else
	{
		qsound_volume_shift = 6;
	}

	qsound_sample_rom = (QSOUND_SRC_SAMPLE *)memory_region_sound1;

	memset(qsound_channel, 0, sizeof(qsound_channel));

	qsound_set_samplerate();

	/* Create pan table */
	{int i;
		for (i = 0; i < 33; i++)
		{	qsound_pan_table[i] = (int)((256 / sqrt(32)) * sqrt(i));}
	}
}

//void qsound_sh_stop(void){}



WRITE8_HANDLER( qsound_data_h_w )
{
	qsound_data = (qsound_data & 0xff) | (data << 8);
}

WRITE8_HANDLER( qsound_data_l_w )
{
	qsound_data = (qsound_data & 0xff00) | data;
}

WRITE8_HANDLER( qsound_cmd_w )
{
	qsound_set_command(data, qsound_data);
}

READ8_HANDLER( qsound_status_r )
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}

void qsound_set_command(int data, int value)
{
	int ch, reg;

	if (data < 0x80)
	{
		ch = data >> 3;
		reg = data & 0x07;
	}
	else if (data < 0x90)
	{
		ch = data - 0x80;
		reg = 8;
	}
	else
	{
		/* Unknown registers */
		return;
	}

	switch (reg)
	{
	case 0: /* Bank */
		ch = (ch + 1) & 0x0f;	/* strange ... */
		qsound_channel[ch].bank = (value & 0x7f) << 16;
		break;

	case 1: /* start */
		qsound_channel[ch].address = value;
		break;

	case 2: /* pitch */
		qsound_channel[ch].pitch = (long)((float)value * qsound_frq_ratio);
		if (!value)
		{
			/* Key off */
			qsound_channel[ch].key = 0;
		}
		break;

	case 4: /* loop offset */
		qsound_channel[ch].loop = value;
		break;

	case 5: /* end */
		qsound_channel[ch].end = value;
		break;

	case 6: /* master volume */
		if (!value)
		{
			/* Key off */
			qsound_channel[ch].key = 0;
		}
		else if (!qsound_channel[ch].key)
		{
			/* Key on */
			qsound_channel[ch].key = 1;
			qsound_channel[ch].offset = 0;
			qsound_channel[ch].lastdt = 0;
		}
		qsound_channel[ch].vol = value;
		break;

	case 8: /* pan and L/R volume */
		qsound_channel[ch].pan = value;
		value = (value - 0x10) & 0x3f;
		if (value > 32) value = 32;
		qsound_channel[ch].rvol = qsound_pan_table[value];
		qsound_channel[ch].lvol = qsound_pan_table[32 - value];
		break;
	}
}

static QSOUND_SAMPLE_MIX qsound_buffer/*[2]*/[SOUND_SAMPLES*2];

#if (0==FORCE_44100)
static void qsound_update_11KHz(s16 *buffer, int length)
{
	int i, ch;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC = &qsound_channel[0];
	QSOUND_SRC_SAMPLE *pST;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		if (pC->key)
		{
			//QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
			//QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX *bufLR = qsound_buffer;

			pST = qsound_sample_rom + pC->bank;
			rvol = (pC->rvol * pC->vol) >> qsound_volume_shift;
			lvol = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < length; i++)
			{
				count = (pC->offset) >> 16;
				pC->offset &= 0xffff;

				if (count)
				{
					pC->address += count;

					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							/* Reached the end of a non-looped sample */
							pC->key = 0;
							break;
						}
						/* Reached the end, restart the loop */
						pC->address = (pC->end - pC->loop) & 0xffff;
					}

					pC->lastdt = pST[pC->address];
				}

			//	*bufL++ += (pC->lastdt * lvol) >> 6;
			//	*bufR++ += (pC->lastdt * rvol) >> 6;
				(*bufLR++) += (pC->lastdt * lvol) >> 6;
				(*bufLR++) += (pC->lastdt * rvol) >> 6;
				pC->offset += pC->pitch;
			}
		}
		pC++;
	}

	for (i = 0; i < (length*2); /*i++*/)
	{
	QSOUND_SAMPLE_MIX lt, rt;
	//	lt = qsound_buffer[0][i];	Limit(lt, MAXOUT, MINOUT);
	//	rt = qsound_buffer[1][i];	Limit(rt, MAXOUT, MINOUT);
	lt = qsound_buffer/*[0]*/[i++];	Limit(lt, MAXOUT, MINOUT);
	rt = qsound_buffer/*[1]*/[i++];	Limit(rt, MAXOUT, MINOUT);
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
	}
}

static void qsound_update_11KHz_resample(s16 *buffer, int length)
{
	int i, j, ch;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC = &qsound_channel[0];
	QSOUND_SRC_SAMPLE *pST;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		if (pC->key)
		{
		//	QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
		//	QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX *bufLR = qsound_buffer;

			pST = qsound_sample_rom + pC->bank;
			rvol = (pC->rvol * pC->vol) >> qsound_volume_shift;
			lvol = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < length; i++)
			{
				for (j = 0; j < 4; j++)
				{
					count = (pC->offset) >> 16;
					pC->offset &= 0xffff;

					if (count)
					{
						pC->address += count;

						if (pC->address >= pC->end)
						{
							if (!pC->loop)
							{
								/* Reached the end of a non-looped sample */
								pC->key = 0;
								break;
							}
							/* Reached the end, restart the loop */
							pC->address = (pC->end - pC->loop) & 0xffff;
						}

						if (pC->lastdt)
							pC->lastdt = (pC->lastdt + pST[pC->address]) >> 1;
						else
							pC->lastdt = pST[pC->address];
					}

					pC->offset += pC->pitch;
				}

			//	*bufL++ += (pC->lastdt * lvol) >> 6;
			//	*bufR++ += (pC->lastdt * rvol) >> 6;
				(*bufLR++) += (pC->lastdt * lvol) >> 6;
				(*bufLR++) += (pC->lastdt * rvol) >> 6;

				if (!pC->key) break;
			}
		}
		pC++;
	}

	for (i = 0; i < (length*2); /*i++*/)
	{
	QSOUND_SAMPLE_MIX lt, rt;
		lt = qsound_buffer/*[0]*/[i++];	Limit(lt, MAXOUT, MINOUT);
		rt = qsound_buffer/*[1]*/[i++];	Limit(rt, MAXOUT, MINOUT);
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
	}
}

static void qsound_update_22KHz(s16 *buffer, int length)
{
	int i, ch;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC = &qsound_channel[0];
	QSOUND_SRC_SAMPLE *pST;
	QSOUND_SAMPLE_MIX lt, rt;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		if (pC->key)
		{
		//	QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
		//	QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX *bufLR = qsound_buffer;

			pST = qsound_sample_rom + pC->bank;
			rvol = (pC->rvol * pC->vol) >> qsound_volume_shift;
			lvol = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < length; i++)
			{
				count = (pC->offset) >> 16;
				pC->offset &= 0xffff;

				if (count)
				{
					pC->address += count;

					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							/* Reached the end of a non-looped sample */
							pC->key = 0;
							break;
						}
						/* Reached the end, restart the loop */
						pC->address = (pC->end - pC->loop) & 0xffff;
					}

					pC->lastdt = pST[pC->address];
				}

			//	*bufL++ += (pC->lastdt * lvol) >> 6;
			//	*bufR++ += (pC->lastdt * rvol) >> 6;
				(*bufLR++) += (pC->lastdt * lvol) >> 6;
				(*bufLR++) += (pC->lastdt * rvol) >> 6;
				pC->offset += pC->pitch;
			}
		}
		pC++;
	}

	for (i = 0; i < (length*2); /*i++*/)
	{
		lt = qsound_buffer/*[0]*/[i++];	Limit(lt, MAXOUT, MINOUT);
		rt = qsound_buffer/*[1]*/[i++];	Limit(rt, MAXOUT, MINOUT);
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
	}
}

static void qsound_update_22KHz_resample(s16 *buffer, int length)
{
	int i, j, ch;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC = &qsound_channel[0];
	QSOUND_SRC_SAMPLE *pST;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		if (pC->key)
		{
		//	QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
		//	QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX *bufLR = qsound_buffer;

			pST = qsound_sample_rom + pC->bank;
			rvol = (pC->rvol * pC->vol) >> qsound_volume_shift;
			lvol = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < length; i++)
			{
				for (j = 0; j < 2; j++)
				{
					count = (pC->offset) >> 16;
					pC->offset &= 0xffff;

					if (count)
					{
						pC->address += count;

						if (pC->address >= pC->end)
						{
							if (!pC->loop)
							{
								/* Reached the end of a non-looped sample */
								pC->key = 0;
								break;
							}
							/* Reached the end, restart the loop */
							pC->address = (pC->end - pC->loop) & 0xffff;
						}

						if (pC->lastdt)
							pC->lastdt = (pC->lastdt + pST[pC->address]) >> 1;
						else
							pC->lastdt = pST[pC->address];
					}

					pC->offset += pC->pitch;
				}

			//	*bufL++ += (pC->lastdt * lvol) >> 6;
			//	*bufR++ += (pC->lastdt * rvol) >> 6;
				(*bufLR++) += (pC->lastdt * lvol) >> 6;
				(*bufLR++) += (pC->lastdt * rvol) >> 6;
				if (!pC->key) break;
			}
		}
		pC++;
	}

	for (i = 0; i < (length*2); /*i++*/)
	{
	QSOUND_SAMPLE_MIX lt, rt;
		lt = qsound_buffer/*[0]*/[i++];	Limit(lt, MAXOUT, MINOUT);
		rt = qsound_buffer/*[1]*/[i++];	Limit(rt, MAXOUT, MINOUT);
		*buffer++ = lt;		*buffer++ = rt;
		*buffer++ = lt;		*buffer++ = rt;
	}
}
#endif//(0==FORCE_44100)

static void qsound_update_44KHz(s16 *buffer, int length)
{
	int i, ch;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC = &qsound_channel[0];
	QSOUND_SRC_SAMPLE *pST;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		if (pC->key)
		{
		//	QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
		//	QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX *bufLR = qsound_buffer;

			pST = qsound_sample_rom + pC->bank;
			rvol = (pC->rvol * pC->vol) >> qsound_volume_shift;
			lvol = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < length; i++)
			{
				count = (pC->offset) >> 16;
				pC->offset &= 0xffff;

				if (count)
				{
					pC->address += count;

					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							/* Reached the end of a non-looped sample */
							pC->key = 0;
							break;
						}
						/* Reached the end, restart the loop */
						pC->address = (pC->end - pC->loop) & 0xffff;
					}

					pC->lastdt = pST[pC->address];
				}

			//	*bufL++ += (pC->lastdt * lvol) >> 6;
			//	*bufR++ += (pC->lastdt * rvol) >> 6;
				(*bufLR++) += (pC->lastdt * lvol) >> 6;
				(*bufLR++) += (pC->lastdt * rvol) >> 6;
				pC->offset += pC->pitch;
			}
		}
		pC++;
	}

	for (i = 0; i < (length*2); i++)
	{
	//	Limit(qsound_buffer[0][i], MAXOUT, MINOUT);	*buffer++ = qsound_buffer[0][i];
	//	Limit(qsound_buffer[1][i], MAXOUT, MINOUT);	*buffer++ = qsound_buffer[1][i];
		Limit(qsound_buffer/*[1]*/[i], MAXOUT, MINOUT);	*buffer++ = qsound_buffer/*[1]*/[i];
	}
}

void qsound_update(s16 *buffer, int length)
{
	memset(qsound_buffer, 0, sizeof(qsound_buffer));

#if (0==FORCE_44100)
	if (qsound_stream_type)
	{
		switch (option_samplerate)
		{
		case 0: qsound_update_11KHz_resample(buffer, length >> 2); return;
		case 1: qsound_update_22KHz_resample(buffer, length >> 1); return;
		case 2: qsound_update_44KHz(buffer, length); return;
		}
	}
	switch (option_samplerate)
	{
	case 0: qsound_update_11KHz(buffer, length >> 2); return;
	case 1: qsound_update_22KHz(buffer, length >> 1); return;
	case 2: qsound_update_44KHz(buffer, length); return;
	}
#else
	qsound_update_44KHz(buffer, length);
#endif
}

/*------------------------------------------------------
	セーブ/ロード ステート
 -----------------------------------------------------*/

#ifdef SAVE_STATE

STATE_SAVE( qsound )
{
int i;
	for (i = 0; i < QSOUND_CHANNELS; i++)
	{
		state_save_long(&qsound_channel[i].bank, 1);
		state_save_long(&qsound_channel[i].address, 1);
		state_save_long(&qsound_channel[i].pitch, 1);
		state_save_long(&qsound_channel[i].loop, 1);
		state_save_long(&qsound_channel[i].end, 1);
		state_save_long(&qsound_channel[i].vol, 1);
		state_save_long(&qsound_channel[i].pan, 1);
		state_save_long(&qsound_channel[i].key, 1);
		state_save_long(&qsound_channel[i].lvol, 1);
		state_save_long(&qsound_channel[i].rvol, 1);
		state_save_long(&qsound_channel[i].lastdt, 1);
		state_save_long(&qsound_channel[i].offset, 1);
	}
}

STATE_LOAD( qsound )
{
int i;
	for (i = 0; i < QSOUND_CHANNELS; i++)
	{
		state_load_long(&qsound_channel[i].bank, 1);
		state_load_long(&qsound_channel[i].address, 1);
		state_load_long(&qsound_channel[i].pitch, 1);
		state_load_long(&qsound_channel[i].loop, 1);
		state_load_long(&qsound_channel[i].end, 1);
		state_load_long(&qsound_channel[i].vol, 1);
		state_load_long(&qsound_channel[i].pan, 1);
		state_load_long(&qsound_channel[i].key, 1);
		state_load_long(&qsound_channel[i].lvol, 1);
		state_load_long(&qsound_channel[i].rvol, 1);
		state_load_long(&qsound_channel[i].lastdt, 1);
		state_load_long(&qsound_channel[i].offset, 1);
	}
}

#endif /* SAVE_STATE */
