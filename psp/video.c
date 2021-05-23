/******************************************************************************

	video.c

	PSP�r�f�I����֐� (16bit�J���[�̂�)

******************************************************************************/

#include "psp.h"


/******************************************************************************
	�O���[�o���ϐ�/�\����
******************************************************************************/

u8 __attribute__((aligned(16))) gulist[GULIST_SIZE];
int video_mode  = 0;
int show_frame  = 0;
int draw_frame  = 0;
int work_frame  = 0;
int tex_frame   = 0;
int screen_is_dirty;

RECT full_rect = { 0, 0, SCR_WIDTH, SCR_HEIGHT };


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*--------------------------------------------------------
	VRAM�N���A
--------------------------------------------------------*/

static void clear_vram(int flag)
{
	int i;
	u32 *vptr = (u32 *)(u8 *)0x44000000;

	if (flag)
		i = 2048 * 1024 >> 2;	// clear all VRAM
	else
		i = (FRAMESIZE32 * 3) >> 2;	// do not clear texture frame

	while (i--) *vptr++ = 0;
}


/******************************************************************************
	�O���[�o���֐�
******************************************************************************/

/*--------------------------------------------------------
	VSYNC��҂�
--------------------------------------------------------*/

void video_wait_vsync(void)
{
	sceDisplayWaitVblankStart();
}


/*--------------------------------------------------------
	�X�N���[�����t���b�v
--------------------------------------------------------*/

void video_flip_screen(int vsync)
{
	if (vsync) sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	show_frame ^= 1;
	draw_frame ^= 1;
}


/*--------------------------------------------------------
	�r�f�I����������
--------------------------------------------------------*/

void video_init(void)
{
	video_mode  = 16;
	show_frame  = 0;
	draw_frame  = 1;
	work_frame  = 2;
	tex_frame   = 3;
	screen_is_dirty = 0;

	sceGuInit();
	sceGuDisplay(GU_FALSE);
	sceGuStart(GU_DIRECT, gulist);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void *)(FRAMESIZE * show_frame), BUF_WIDTH);
	sceGuDrawBuffer(GU_PSM_5551, (void *)(FRAMESIZE * draw_frame), BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH / 2), 2048 - (SCR_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_LEQUAL, 0, 0x01);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
	sceGuTexScale(1.0f / BUF_WIDTH, 1.0f / BUF_WIDTH);
	sceGuTexOffset(0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFlush();

	sceGuFinish();
	sceGuSync(0, 0);

	sceGuDisplay(GU_TRUE);

	clear_vram(0);

	create_small_font();
}


/*--------------------------------------------------------
	�r�f�I�����I��
--------------------------------------------------------*/

void video_exit(int flag)
{
	sceGuDisplay(GU_FALSE);
	sceGuTerm();
	clear_vram(flag);
}


/*--------------------------------------------------------
	VRAM�̃A�h���X���擾
--------------------------------------------------------*/

u16 *video_frame_addr(int frame, int x, int y)
{
	return (u16 *)(((u8 *)0x44000000) + ((x + ((y + frame * SCR_HEIGHT) << 9)) << 1));
}


/*--------------------------------------------------------
	�`��/�\���t���[�����N���A
--------------------------------------------------------*/

void video_clear_screen(void)
{
	video_clear_frame(show_frame);
	video_clear_frame(draw_frame);
}


/*--------------------------------------------------------
	�w�肵���t���[����h��Ԃ�
--------------------------------------------------------*/

void video_fill_frame(int frame, u32 color)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * frame), BUF_WIDTH);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuClearColor(color);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	�w�肵����`�͈͂�h��Ԃ�
--------------------------------------------------------*/

void video_fill_rect(int frame, u32 color, RECT *rect)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * frame), BUF_WIDTH);
	sceGuScissor(rect->left, rect->top, rect->right, rect->bottom);
	sceGuClearColor(color);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	��`�͈͂��R�s�[
--------------------------------------------------------*/

void video_copy_rect(int src, int dst, RECT *src_rect, RECT *dst_rect)
{
	int j, sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * dst), BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuTexImage(0, BUF_WIDTH, BUF_WIDTH, BUF_WIDTH, video_frame_addr(src, 0, 0));

	if (sw == dw && sh == dh)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	{
    	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->left + j + SLICE_SIZE;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	if (j < sw)
	{
		vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->right;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->right;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	��`�͈͂����E���]���ăR�s�[
--------------------------------------------------------*/

void video_copy_rect_flip(int src, int dst, RECT *src_rect, RECT *dst_rect)
{
	int j, sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * dst), BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuTexImage(0, BUF_WIDTH, BUF_WIDTH, BUF_WIDTH, video_frame_addr(src, 0, 0));

	if (sw == dw && sh == dh)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	{
    	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->right - j * dw / sw;
		vertices[0].y = dst_rect->bottom;

		vertices[1].u = src_rect->left + j + SLICE_SIZE;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->right - (j + SLICE_SIZE) * dw / sw;
		vertices[1].y = dst_rect->top;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	if (j < sw)
	{
		vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->right - j * dw / sw;
		vertices[0].y = dst_rect->bottom;

		vertices[1].u = src_rect->right;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->left;
		vertices[1].y = dst_rect->top;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	��`�͈͂�270�x��]���ăR�s�[
--------------------------------------------------------*/

void video_copy_rect_rotate(int src, int dst, RECT *src_rect, RECT *dst_rect)
{
	struct Vertex *vertices;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * dst), BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuDisable(GU_ALPHA_TEST);
	sceGuTexImage(0, BUF_WIDTH, BUF_WIDTH, BUF_WIDTH, video_frame_addr(src, 0, 0));

	sceGuTexFilter(GU_LINEAR, GU_LINEAR);

    vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	vertices[0].u = src_rect->right;
	vertices[1].v = src_rect->top;
	vertices[0].x = dst_rect->right;
	vertices[0].y = dst_rect->top;

	vertices[1].u = src_rect->left;
	vertices[0].v = src_rect->bottom;
	vertices[1].x = dst_rect->left;
	vertices[1].y = dst_rect->bottom;

	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	��`�͈͂𓧉ߏ�������ŃR�s�[
--------------------------------------------------------*/

void video_copy_rect_alpha(int src, int dst, RECT *src_rect, RECT *dst_rect)
{
	int j, sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * dst), BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexImage(0, BUF_WIDTH, BUF_WIDTH, BUF_WIDTH, video_frame_addr(src, 0, 0));

	sceGuTexFilter(GU_NEAREST, GU_NEAREST);

	for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	{
    	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->left + j + SLICE_SIZE;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	if (j < sw)
	{
		vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->right;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->right;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}
