/*********************************************************

    Capcom Q-Sound system

*********************************************************/

#ifndef __QSOUND_H__
#define __QSOUND_H__

void qsound_sh_start(void);
void qsound_sh_stop(void);
void qsound_set_samplerate(void);
void qsound_update(s16 *buffer, int length);

#ifdef SAVE_STATE
STATE_SAVE( qsound );
STATE_LOAD( qsound );
#endif

WRITE8_HANDLER( qsound_data_h_w );
WRITE8_HANDLER( qsound_data_l_w );
WRITE8_HANDLER( qsound_cmd_w );
READ8_HANDLER( qsound_status_r );

#endif /* __QSOUND_H__ */
