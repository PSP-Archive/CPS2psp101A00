/******************************************************************************

	sound.c

	PSP �T�E���h�X���b�h

******************************************************************************/

#ifndef PSP_SOUND_H
#define PSP_SOUND_H

//#define SOUND_SAMPLES		(44100/FPS)	// �������������ǁA64�o�C�g���E�ɂ��킸
#define SOUND_SAMPLES		(736*2)
#define SOUND_BUFFER_SIZE	(SOUND_SAMPLES * 2)

void sound_init(void);
void sound_exit(void);
void sound_set_callback(void (*callback)(s16 *buffer, int length));
void sound_enable(int enable);
void sound_set_volume(void);
int sound_thread_start(int stereo);
void sound_thread_stop(void);

#endif /* PSP_SOUND_H */
