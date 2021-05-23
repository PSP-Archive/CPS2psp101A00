/******************************************************************************

	sound.c

	PSP �T�E���h�X���b�h

******************************************************************************/

#include "psp.h"
#include "emumain.h"


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

static volatile int sound_active;
static int sound_handle;
static SceUID sound_thread;
static int sound_volume;
static int sound_enabled;
static s16 sound_buffer[2][SOUND_BUFFER_SIZE];
static void (*sound_callback)(s16 *buffer, int length);


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*--------------------------------------------------------
	�f�t�H���g�̃R�[���o�b�N�֐�
--------------------------------------------------------*/

static void default_sound_callback(s16 *buffer, int length)
{
	memset(buffer, 0, SOUND_BUFFER_SIZE);
}


/*--------------------------------------------------------
	�T�E���h�X�V�X���b�h
--------------------------------------------------------*/

static int SoundThread(SceSize args, void *argp)
{
	int flip = 0;

	while (sound_active)
	{
		if (Sleep)
		{
			do
			{
				sceKernelDelayThread(5000000);
			} while (Sleep);
		}

		if (sound_enabled)
		{
			sound_callback(sound_buffer[flip], SOUND_SAMPLES);
			sceAudioOutputPannedBlocking(sound_handle, sound_volume, sound_volume, (char *)sound_buffer[flip]);
		}
		else
		{
			memset(sound_buffer[flip], 0, SOUND_BUFFER_SIZE);
			sceAudioOutputPannedBlocking(sound_handle, 0, 0, (char *)sound_buffer[flip]);
		}
		flip ^= 1;
	}
	return 0;
}


/******************************************************************************
	�O���[�o���֐�
******************************************************************************/

/*--------------------------------------------------------
	�T�E���h������
--------------------------------------------------------*/

void sound_init(void)
{
	sound_active = 0;
	sound_thread = -1;
	sound_handle = -1;
	sound_volume = 0;
	sound_enabled = 0;

	sound_callback = default_sound_callback;
}


/*--------------------------------------------------------
	�T�E���h�I��
--------------------------------------------------------*/

void sound_exit(void)
{
	sound_thread_stop();
}


/*--------------------------------------------------------
	�T�E���h�R�[���o�b�N�֐���ݒ�
--------------------------------------------------------*/

void sound_set_callback(void (*callback)(s16 *buffer, int length))
{
	sound_callback = callback;
}


/*--------------------------------------------------------
	�T�E���h�L��/�����؂�ւ�
--------------------------------------------------------*/

void sound_enable(int enable)
{
	if (sound_active)
	{
		sound_enabled = enable;

		if (sound_enabled)
			sound_set_volume();
		else
			sound_volume = 0;
	}
}


/*--------------------------------------------------------
	�T�E���h�̃{�����[���ݒ�
--------------------------------------------------------*/

void sound_set_volume(void)
{
	if (sound_active)
		sound_volume = PSP_AUDIO_VOLUME_MAX * (option_sound_volume * 10) / 100;
}


/*--------------------------------------------------------
	�T�E���h�X���b�h�J�n
--------------------------------------------------------*/

int sound_thread_start(int stereo)
{
	sound_active  = 0;
	sound_thread  = -1;
	sound_handle  = -1;
	sound_enabled = 0;

	memset(sound_buffer[0], 0, SOUND_BUFFER_SIZE);
	memset(sound_buffer[1], 0, SOUND_BUFFER_SIZE);

	if (stereo)
		sound_handle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, SOUND_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
	else
		sound_handle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, SOUND_SAMPLES, PSP_AUDIO_FORMAT_MONO);
	if (sound_handle < 0)
	{
		fatalerror("Could not reserve audio channel for sound.");
		return 0;
	}

	sound_thread = sceKernelCreateThread("Sound thread", SoundThread, 0x8, 0x10000, 0, NULL);
	if (sound_thread < 0)
	{
		fatalerror("Could not start sound thread.");
		sceAudioChRelease(sound_handle);
		sound_handle = -1;
		return 0;
	}

	sound_active = 1;
	sceKernelStartThread(sound_thread, 0, 0);

	sound_set_volume();

	return 1;
}


/*--------------------------------------------------------
	�T�E���h�X���b�h��~
--------------------------------------------------------*/

void sound_thread_stop(void)
{
	if (sound_thread >= 0)
	{
		sound_volume = 0;
		sound_enabled = 0;

		sound_active = 0;
		sceKernelWaitThreadEnd(sound_thread, NULL);

		sceKernelDeleteThread(sound_thread);
		sound_thread = -1;

		sceAudioChRelease(sound_handle);
		sound_handle = -1;
	}
}
