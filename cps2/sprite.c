/******************************************************************************

	sprite.c

	CPS2 スプライトマネージャ

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	定数/マクロ等
******************************************************************************/

#define OBJECT_MAX_HEIGHT	512
#define SCROLL1_MAX_HEIGHT	224
#define SCROLL2_MAX_HEIGHT	416
#define SCROLL3_MAX_HEIGHT	416

#define CNVCOL15TO32(c)	((GETB15(c) << 16) | (GETG15(c) << 8) | GETR15(c))


/******************************************************************************
	ローカル変数/構造体
******************************************************************************/

typedef struct sprite_t SPRITE __attribute__((aligned(16)));

struct sprite_t
{
	u32 key;
	int used;
	u16 pal;
	u16 index;
	SPRITE *next;
};

static RECT cps_src_clip =
	{  64, 16,  64 + 384, 16 + 224 };

static RECT cps_clip[5] =
{
	{  48, 24,  48 + 384, 24 + 224 },	// option_stretch = 0  (384x224)
	{  60,  0,  60 + 360,      272 },	// option_stretch = 1  (360x270  4:3)
	{  48,  0,  48 + 384,      272 },	// option_stretch = 2  (384x272 24:17)
	{   0,  0,       480,      272 },	// option_stretch = 3  (480x270 16:9)
	{ 138,  0, 138 + 204,      272 }	// option_stretch = 4  (204x272  3:4 vertical)
};

static int clip_min_y;
static int clip_max_y;

static int min_y_8;
static int min_y_16;
static int min_y_32;


/*------------------------------------------------------------------------
	パレットは0～255番(うち、127番まで使用)まであり、各16色と
	なっている。パレットが更新された場合、テクスチャも登録し
	直す必要がある。
------------------------------------------------------------------------*/

static u8 palette_dirty_marks[128];


/*------------------------------------------------------------------------
	OBJECT: キャラクタ等

	size:    16x16 pixels
	palette: 0～31番を使用
------------------------------------------------------------------------*/

#define OBJECT_HASH_SIZE		0x200
#define OBJECT_HASH_MASK		0x1ff
#define OBJECT_CACHE_SIZE		((512/16)*(OBJECT_MAX_HEIGHT/16))
#define OBJECT_MAX_SPRITES		0x1000

static SPRITE *object_head[OBJECT_HASH_SIZE];
static SPRITE object_data[OBJECT_CACHE_SIZE];
static SPRITE *object_free_head;

static u16 *tex_object;
static s16 object_x[OBJECT_MAX_SPRITES];
static s16 object_y[OBJECT_MAX_SPRITES];
static u16 object_idx[OBJECT_MAX_SPRITES];
static u16 object_atr[OBJECT_MAX_SPRITES];
static u16 object_num;
static u16 object_cached_num;
static u16 object_max;
static u8 object_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLL1: スクロール面1(テキスト等)

	size:    8x8 pixels
	palette: 32～63番を使用
------------------------------------------------------------------------*/

#define SCROLL1_HASH_SIZE		0x200
#define SCROLL1_HASH_MASK		0x1ff
#define SCROLL1_CACHE_SIZE		((512/8)*(SCROLL1_MAX_HEIGHT/8))
#define SCROLL1_MAX_SPRITES		0x2000

static SPRITE *scroll1_head[SCROLL1_HASH_SIZE];
static SPRITE scroll1_data[SCROLL1_CACHE_SIZE];
static SPRITE *scroll1_free_head;

static u16 *tex_scroll1;
static s16 scroll1_x[SCROLL1_MAX_SPRITES];
static s16 scroll1_y[SCROLL1_MAX_SPRITES];
static u16 scroll1_idx[SCROLL1_MAX_SPRITES];
static u16 scroll1_atr[SCROLL1_MAX_SPRITES];
static u16 scroll1_num;
static u16 scroll1_cached_num;
static u16 scroll1_max;
static u8 scroll1_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLL2: スクロール面2

	size:    16x16 pixels
	palette: 64～95番を使用
------------------------------------------------------------------------*/

#define SCROLL2_HASH_SIZE		0x100
#define SCROLL2_HASH_MASK		0xff
#define SCROLL2_CACHE_SIZE		((512/16)*(SCROLL2_MAX_HEIGHT/16))
#define SCROLL2_MAX_SPRITES		0x800

static SPRITE *scroll2_head[SCROLL2_HASH_SIZE];
static SPRITE scroll2_data[SCROLL2_CACHE_SIZE];
static SPRITE *scroll2_free_head;

static u16 *tex_scroll2;
static s16 scroll2_x[SCROLL2_MAX_SPRITES];
static s16 scroll2_y[SCROLL2_MAX_SPRITES];
static u16 scroll2_idx[SCROLL2_MAX_SPRITES];
static u16 scroll2_atr[SCROLL2_MAX_SPRITES];
static u16 scroll2_num;
static u16 scroll2_cached_num;
static u16 scroll2_max;
static u8 scroll2_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLL3: スクロール面3

	size:    32x32 pixels
	palette: 96～127番を使用
------------------------------------------------------------------------*/

#define SCROLL3_HASH_SIZE		0x80
#define SCROLL3_HASH_MASK		0x7f
#define SCROLL3_CACHE_SIZE		((512/32)*(SCROLL3_MAX_HEIGHT/32))
#define SCROLL3_MAX_SPRITES		0x200

static SPRITE *scroll3_head[SCROLL3_HASH_SIZE];
static SPRITE scroll3_data[SCROLL3_CACHE_SIZE];
static SPRITE *scroll3_free_head;

static u16 *tex_scroll3;
static s16 scroll3_x[SCROLL3_MAX_SPRITES];
static s16 scroll3_y[SCROLL3_MAX_SPRITES];
static u16 scroll3_idx[SCROLL3_MAX_SPRITES];
static u16 scroll3_atr[SCROLL3_MAX_SPRITES];
static u16 scroll3_num;
static u16 scroll3_cached_num;
static u16 scroll3_max;
static u8 scroll3_palette_is_dirty;


/******************************************************************************
	OBJECTスプライト管理
******************************************************************************/

/*----------------------------------------------------------------
	OBJECTキャッシュをリセットする
----------------------------------------------------------------*/

static void object_reset_cache(void)
{
	int i;

	for (i = 0; i < object_max - 1; i++)
		object_data[i].next = &object_data[i + 1];

	object_data[i].next = NULL;
	object_free_head = &object_data[0];

	memset(object_head, 0, sizeof(SPRITE *) * OBJECT_HASH_SIZE);

	object_cached_num = 0;
	object_palette_is_dirty = 0;
}


/*----------------------------------------------------------------
	OBJECTキャッシュからテクスチャ番号を取得
----------------------------------------------------------------*/

static int object_get_cache(int key)
{
	SPRITE *p = object_head[key & OBJECT_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			p->used = frames_displayed;
			if (update_cache) update_cache((key & 0xffffff) << 7);
			return p->index;
	 	}
		p = p->next;
	}
	return -1;
}


/*----------------------------------------------------------------
	OBJECTキャッシュにテクスチャを登録
----------------------------------------------------------------*/

static int object_insert_cache(u32 key)
{
	u16 hash = key & OBJECT_HASH_MASK;
	SPRITE *p = object_head[hash];
	SPRITE *q = object_free_head;

	if (!q) return -1;

	object_free_head = object_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 24;
	q->used = frames_displayed;

	if (!p)
	{
		object_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	object_cached_num++;

	return q->index;
}


/*----------------------------------------------------------------
	OBJECTキャッシュから一定時間を経過したテクスチャを削除
----------------------------------------------------------------*/

static void object_clean_cache(int delay)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < OBJECT_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = object_head[i];

		while (p)
		{
			if (frames_displayed - p->used > delay)
			{
				object_cached_num--;

				if (!prev_p)
				{
					object_head[i] = p->next;
					p->next = object_free_head;
					object_free_head = p;
					p = object_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = object_free_head;
					object_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/*----------------------------------------------------------------
	OBJECTキャッシュからパレットが変更されたスプライトを削除
----------------------------------------------------------------*/

static void object_clear_dirty_palette(void)
{
	SPRITE *p, *prev_p;
	int i, palno;

	for (palno = 0; palno < 32; palno++)
	{
		if (!palette_dirty_marks[palno]) continue;

		for (i = 0; i < OBJECT_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = object_head[i];

			while (p)
			{
				if (p->pal == palno)
				{
					object_cached_num--;

					if (!prev_p)
					{
						object_head[i] = p->next;
						p->next = object_free_head;
						object_free_head = p;
						p = object_head[i];
					}
					else
					{
						prev_p->next = p->next;
						p->next = object_free_head;
						object_free_head = p;
						p = prev_p->next;
					}
				}
				else
				{
					prev_p = p;
					p = p->next;
				}
			}
		}
	}

	object_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLL1スプライト管理
******************************************************************************/

/*----------------------------------------------------------------
	SCROLL1キャッシュをリセットする
----------------------------------------------------------------*/

static void scroll1_reset_cache(void)
{
	int i;

	for (i = 0; i < scroll1_max - 1; i++)
		scroll1_data[i].next = &scroll1_data[i + 1];

	scroll1_data[i].next = NULL;
	scroll1_free_head = &scroll1_data[0];

	memset(scroll1_head, 0, sizeof(SPRITE *) * SCROLL1_HASH_SIZE);

	scroll1_cached_num = 0;
	scroll1_palette_is_dirty = 0;
}


/*----------------------------------------------------------------
	SCROLL1キャッシュからテクスチャ番号を取得
----------------------------------------------------------------*/

static int scroll1_get_cache(int key)
{
	SPRITE *p = scroll1_head[key & SCROLL1_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
				if (update_cache) update_cache((key & 0xffffff) << 6);
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*----------------------------------------------------------------
	SCROLL1キャッシュにテクスチャを登録
----------------------------------------------------------------*/

static int scroll1_insert_cache(u32 key)
{
	u16 hash = key & SCROLL1_HASH_MASK;
	SPRITE *p = scroll1_head[hash];
	SPRITE *q = scroll1_free_head;

	if (!q) return -1;

	scroll1_free_head = scroll1_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 24;
	q->used = frames_displayed;

	if (!p)
	{
		scroll1_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scroll1_cached_num++;

	return q->index;
}


/*----------------------------------------------------------------
	SCROLL1キャッシュから一定時間を経過したテクスチャを削除
----------------------------------------------------------------*/

static void scroll1_clean_cache(int delay)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLL1_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scroll1_head[i];

		while (p)
		{
			if (frames_displayed - p->used > delay)
			{
				scroll1_cached_num--;

				if (!prev_p)
				{
					scroll1_head[i] = p->next;
					p->next = scroll1_free_head;
					scroll1_free_head = p;
					p = scroll1_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scroll1_free_head;
					scroll1_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/*----------------------------------------------------------------
	SCROLL1キャッシュからパレットが変更されたスプライトを削除
----------------------------------------------------------------*/

static void scroll1_clear_dirty_palette(void)
{
	SPRITE *p, *prev_p;
	int i, palno;

	for (palno = 32; palno < 64; palno++)
	{
		if (!palette_dirty_marks[palno]) continue;

		for (i = 0; i < SCROLL1_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = scroll1_head[i];

			while (p)
			{
				if (p->pal == palno)
				{
					scroll1_cached_num--;

					if (!prev_p)
					{
						scroll1_head[i] = p->next;
						p->next = scroll1_free_head;
						scroll1_free_head = p;
						p = scroll1_head[i];
					}
					else
					{
						prev_p->next = p->next;
						p->next = scroll1_free_head;
						scroll1_free_head = p;
						p = prev_p->next;
					}
				}
				else
				{
					prev_p = p;
					p = p->next;
				}
			}
		}
	}

	scroll1_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLL2スプライト管理
******************************************************************************/

/*----------------------------------------------------------------
	SCROLL2キャッシュをリセットする
----------------------------------------------------------------*/

static void scroll2_reset_cache(void)
{
	int i;

	for (i = 0; i < scroll2_max - 1; i++)
		scroll2_data[i].next = &scroll2_data[i + 1];

	scroll2_data[i].next = NULL;
	scroll2_free_head = &scroll2_data[0];

	memset(scroll2_head, 0, sizeof(SPRITE *) * SCROLL2_HASH_SIZE);

	scroll2_cached_num = 0;
	scroll2_palette_is_dirty = 0;
}


/*----------------------------------------------------------------
	SCROLL2キャッシュからテクスチャ番号を取得
----------------------------------------------------------------*/

static int scroll2_get_cache(int key)
{
	SPRITE *p = scroll2_head[key & SCROLL2_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
				if (update_cache) update_cache((key & 0xffffff) << 7);
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*----------------------------------------------------------------
	SCROLL2キャッシュにテクスチャを登録
----------------------------------------------------------------*/

static int scroll2_insert_cache(u32 key)
{
	u16 hash = key & SCROLL2_HASH_MASK;
	SPRITE *p = scroll2_head[hash];
	SPRITE *q = scroll2_free_head;

	if (!q) return -1;

	scroll2_free_head = scroll2_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 24;
	q->used = frames_displayed;

	if (!p)
	{
		scroll2_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scroll2_cached_num++;

	return q->index;
}


/*----------------------------------------------------------------
	SCROLL2キャッシュから一定時間を経過したテクスチャを削除
----------------------------------------------------------------*/

static void scroll2_clean_cache(int delay)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLL2_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scroll2_head[i];

		while (p)
		{
			if (frames_displayed - p->used > delay)
			{
				scroll2_cached_num--;

				if (!prev_p)
				{
					scroll2_head[i] = p->next;
					p->next = scroll2_free_head;
					scroll2_free_head = p;
					p = scroll2_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scroll2_free_head;
					scroll2_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/*----------------------------------------------------------------
	SCROLL2キャッシュからパレットが変更されたスプライトを削除
----------------------------------------------------------------*/

static void scroll2_clear_dirty_palette(void)
{
	SPRITE *p, *prev_p;
	int i, palno;

	for (palno = 64; palno < 96; palno++)
	{
		if (!palette_dirty_marks[palno]) continue;

		for (i = 0; i < SCROLL2_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = scroll2_head[i];

			while (p)
			{
				if (p->pal == palno)
				{
					scroll2_cached_num--;

					if (!prev_p)
					{
						scroll2_head[i] = p->next;
						p->next = scroll2_free_head;
						scroll2_free_head = p;
						p = scroll2_head[i];
					}
					else
					{
						prev_p->next = p->next;
						p->next = scroll2_free_head;
						scroll2_free_head = p;
						p = prev_p->next;
					}
				}
				else
				{
					prev_p = p;
					p = p->next;
				}
			}
		}
	}

	scroll2_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLL3スプライト管理
******************************************************************************/

/*----------------------------------------------------------------
	SCROLL3キャッシュをリセットする
----------------------------------------------------------------*/

static void scroll3_reset_cache(void)
{
	int i;

	for (i = 0; i < scroll3_max - 1; i++)
		scroll3_data[i].next = &scroll3_data[i + 1];

	scroll3_data[i].next = NULL;
	scroll3_free_head = &scroll3_data[0];

	memset(scroll3_head, 0, sizeof(SPRITE *) * SCROLL3_HASH_SIZE);

	scroll3_cached_num = 0;
	scroll3_palette_is_dirty = 0;
}


/*----------------------------------------------------------------
	SCROLL3キャッシュからテクスチャ番号を取得
----------------------------------------------------------------*/

static int scroll3_get_cache(int key)
{
	SPRITE *p = scroll3_head[key & SCROLL3_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
				if (update_cache) update_cache((key & 0xffffff) << 9);
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*----------------------------------------------------------------
	SCROLL3キャッシュにテクスチャを登録
----------------------------------------------------------------*/

static int scroll3_insert_cache(u32 key)
{
	u16 hash = key & SCROLL3_HASH_MASK;
	SPRITE *p = scroll3_head[hash];
	SPRITE *q = scroll3_free_head;

	if (!q) return -1;

	scroll3_free_head = scroll3_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 24;
	q->used = frames_displayed;

	if (!p)
	{
		scroll3_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scroll3_cached_num++;

	return q->index;
}


/*----------------------------------------------------------------
	SCROLL3キャッシュから一定時間を経過したテクスチャを削除
----------------------------------------------------------------*/

static void scroll3_clean_cache(int delay)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLL3_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scroll3_head[i];

		while (p)
		{
			if (frames_displayed - p->used > delay)
			{
				scroll3_cached_num--;

				if (!prev_p)
				{
					scroll3_head[i] = p->next;
					p->next = scroll3_free_head;
					scroll3_free_head = p;
					p = scroll3_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scroll3_free_head;
					scroll3_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/*----------------------------------------------------------------
	SCROLL3キャッシュからパレットが変更されたスプライトを削除
----------------------------------------------------------------*/

static void scroll3_clear_dirty_palette(void)
{
	SPRITE *p, *prev_p;
	int i, palno;

	for (palno = 96; palno < 128; palno++)
	{
		if (!palette_dirty_marks[palno]) continue;

		for (i = 0; i < SCROLL3_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = scroll3_head[i];

			while (p)
			{
				if (p->pal == palno)
				{
					scroll3_cached_num--;

					if (!prev_p)
					{
						scroll3_head[i] = p->next;
						p->next = scroll3_free_head;
						scroll3_free_head = p;
						p = scroll3_head[i];
					}
					else
					{
						prev_p->next = p->next;
						p->next = scroll3_free_head;
						scroll3_free_head = p;
						p = prev_p->next;
					}
				}
				else
				{
					prev_p = p;
					p = p->next;
				}
			}
		}
	}

	scroll3_palette_is_dirty = 0;
}


/******************************************************************************
	スプライト描画インタフェース関数
******************************************************************************/

/*----------------------------------------------------------------
	全てのキャッシュの即座にクリアする
--------------------------------------------------------*/

void blit_clear_all_cache(void)
{
	object_reset_cache();
	scroll1_reset_cache();
	scroll2_reset_cache();
	scroll3_reset_cache();
	memset(palette_dirty_marks, 0, sizeof(palette_dirty_marks));
}


/*------------------------------------------------------
	スプライト処理のリセット
------------------------------------------------------*/

void blit_reset(void)
{
	int i;
	int object_height;
	int scroll1_height;
	int scroll2_height;
	int scroll3_height;
	u16 *work_buffer;

	screen_is_dirty = 0;

	// 合計が1232pixelになるように割り当てる
	object_height  = driver->object_tex_height;
	scroll1_height = driver->scroll1_tex_height;
	scroll2_height = driver->scroll2_tex_height;
	scroll3_height = driver->scroll3_tex_height;

	object_max  = (512/16) * (object_height /16);
	scroll1_max = (512/ 8) * (scroll1_height/ 8);
	scroll2_max = (512/16) * (scroll2_height/16);
	scroll3_max = (512/32) * (scroll3_height/32);

	work_buffer = video_frame_addr(work_frame, 0, 0);
	tex_object  = work_buffer + BUF_WIDTH * 256;
	tex_scroll1 = tex_object  + BUF_WIDTH * object_height;
	tex_scroll2 = tex_scroll1 + BUF_WIDTH * scroll1_height;
	tex_scroll3 = tex_scroll2 + BUF_WIDTH * scroll2_height;

	for (i = 0; i < object_max  - 1; i++) object_data[i].index = i;
	for (i = 0; i < scroll1_max - 1; i++) scroll1_data[i].index = i;
	for (i = 0; i < scroll2_max - 1; i++) scroll2_data[i].index = i;
	for (i = 0; i < scroll3_max - 1; i++) scroll3_data[i].index = i;

	clip_min_y = FIRST_VISIBLE_LINE;
	clip_max_y = LAST_VISIBLE_LINE;

	blit_clear_all_cache();
}


/*----------------------------------------------------------------
	パレット変更フラグを立てる
----------------------------------------------------------------*/

void blit_palette_mark_dirty(int palno)
{
	palette_dirty_marks[palno] = 1;

	if (palno < 32) object_palette_is_dirty = 1;
	else if (palno < 64) scroll1_palette_is_dirty = 1;
	else if (palno < 96) scroll2_palette_is_dirty = 1;
	else if (palno < 128) scroll3_palette_is_dirty = 1;
}


/*----------------------------------------------------------------
	スプライト描画開始
----------------------------------------------------------------*/

static void blit_start(void)
{
	RECT clip = { 0, 0, 512, 256 };

	if (object_palette_is_dirty) object_clear_dirty_palette();
	if (scroll1_palette_is_dirty) scroll1_clear_dirty_palette();
	if (scroll2_palette_is_dirty) scroll2_clear_dirty_palette();
	if (scroll3_palette_is_dirty) scroll3_clear_dirty_palette();
	memset(palette_dirty_marks, 0, sizeof(palette_dirty_marks));

	if (screen_is_dirty > 0)
	{
		video_clear_frame(draw_frame);
		screen_is_dirty--;
	}
	video_clear_rect(work_frame, &clip);
}


/*----------------------------------------------------------------
	画面の一部更新開始
----------------------------------------------------------------*/

void blit_partial_start(int start, int end)
{
	clip_min_y  = start;
	clip_max_y  = end + 1;

	min_y_8  = start - 8;
	min_y_16 = start - 16;
	min_y_32 = start - 32;

	object_num  = 0;
	scroll1_num = 0;
	scroll2_num = 0;
	scroll3_num = 0;

	if (start == FIRST_VISIBLE_LINE)
		blit_start();
}


/*----------------------------------------------------------------
	スプライト描画終了
----------------------------------------------------------------*/

void blit_finish(void)
{
	if (!cps_rotate_screen)
	{
		if (!cps_flip_screen)
			video_copy_rect(work_frame, draw_frame, &cps_src_clip, &cps_clip[option_stretch]);
		else
			video_copy_rect_flip(work_frame, draw_frame, &cps_src_clip, &cps_clip[option_stretch]);
	}
	else
	{
		if (cps_flip_screen)
		{
			video_copy_rect_flip(work_frame, draw_frame, &cps_src_clip, &cps_src_clip);
			video_copy_rect(draw_frame, work_frame, &cps_src_clip, &cps_src_clip);
			video_clear_frame(draw_frame);
		}
		video_copy_rect_rotate(work_frame, draw_frame, &cps_src_clip, &cps_clip[4]);
	}
}


/*----------------------------------------------------------------
	Objectのテクスチャを更新
----------------------------------------------------------------*/

void blit_update_object(int x, int y, u32 code, u32 attr)
{
	if ((x > 48 && x < 448) && (y > min_y_16 && y < clip_max_y))
	{
		u32 key = MAKE_KEY(code, (attr & 0x1f));
		SPRITE *p = object_head[key & OBJECT_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				p->used = frames_displayed;
				if (update_cache) update_cache((key & 0xffffff) << 7);
				return;
		 	}
			p = p->next;
		}
	}
}


/*----------------------------------------------------------------
	OBJECTを描画リストに登録
----------------------------------------------------------------*/

void blit_draw_object(int x, int y, u32 code, u32 attr)
{
	if ((x > 48 && x < 448) && (y > min_y_16 && y < clip_max_y))
	{
		int idx;
		u16 color = attr & 0x1f;
		u32 key = MAKE_KEY(code, color);

		if ((idx = object_get_cache(key)) < 0)
		{
			int lines, u, v;
			u16 *dst, *pal;
			u32 *gfx, tile;
			u32 offset;

			if (object_cached_num == object_max - 1)
			{
				cps2_scan_sprites_callback();
				object_clean_cache(0);

				if (object_cached_num == object_max - 1)
					return;
			}

			idx = object_insert_cache(key);
			u = (idx & 31) << 4;
			v = (idx >> 5) << 4;

			offset = (*read_cache)(code << 7);
			gfx = (u32 *)&memory_region_gfx1[offset];

			pal = &video_palette[color << 4];
			dst = &tex_object[u + (v << 9)];
			lines = 16;

			while (lines--)
			{
				tile = *gfx++;
				dst[/* 0*/ 7] = pal[(tile & 0xf0000000)>>28];
				dst[/* 1*/ 6] = pal[(tile & 0x0f000000)>>24];
				dst[/* 2*/ 5] = pal[(tile & 0x00f00000)>>20];
				dst[/* 3*/ 4] = pal[(tile & 0x000f0000)>>16];
				dst[/* 4*/ 3] = pal[(tile & 0x0000f000)>>12];
				dst[/* 5*/ 2] = pal[(tile & 0x00000f00)>> 8];
				dst[/* 6*/ 1] = pal[(tile & 0x000000f0)>> 4];
				dst[/* 7*/ 0] = pal[(tile & 0x0000000f)    ];
				tile = *gfx++;
				dst[/* 8*/15] = pal[(tile & 0xf0000000)>>28];
				dst[/* 9*/14] = pal[(tile & 0x0f000000)>>24];
				dst[/*10*/13] = pal[(tile & 0x00f00000)>>20];
				dst[/*11*/12] = pal[(tile & 0x000f0000)>>16];
				dst[/*12*/11] = pal[(tile & 0x0000f000)>>12];
				dst[/*13*/10] = pal[(tile & 0x00000f00)>> 8];
				dst[/*14*/ 9] = pal[(tile & 0x000000f0)>> 4];
				dst[/*15*/ 8] = pal[(tile & 0x0000000f)    ];
				dst += BUF_WIDTH;
			}
		}

		object_idx[object_num] = idx;
		object_x[object_num]   = x;
		object_y[object_num]   = y;
		object_atr[object_num] = attr;
		object_num++;
	}
}

/*----------------------------------------------------------------
	OBJECT描画終了
----------------------------------------------------------------*/

void blit_finish_object(void)
{
	int i, u, v;
	struct Vertex *vertices, *vertices_tmp;

	if (!object_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * work_frame), BUF_WIDTH);
	sceGuScissor(cps_src_clip.left, clip_min_y, cps_src_clip.right, clip_max_y);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);

	sceGuTexImage(0, 512, 512, 512, tex_object);

	vertices = (struct Vertex *)sceGuGetMemory(object_num * 2 * sizeof(struct Vertex));
	if (vertices)
	{
		vertices_tmp = vertices;

		for (i = 0; i < object_num; i++)
		{
			v = (object_idx[i] >> 5) << 4;
			u = (object_idx[i] & 31) << 4;

			if (object_atr[i] & 0x20)
			{
				// flipx
				vertices_tmp[0].u = u + 16;
				vertices_tmp[1].u = u;
			}
			else
			{
				vertices_tmp[0].u = u;
				vertices_tmp[1].u = u + 16;
			}

			if (object_atr[i] & 0x40)
			{
				// flipy
				vertices_tmp[0].v = v + 16;
				vertices_tmp[1].v = v;
			}
			else
			{
				vertices_tmp[0].v = v;
				vertices_tmp[1].v = v + 16;
			}

			vertices_tmp[0].x = object_x[i];
			vertices_tmp[0].y = object_y[i];

			vertices_tmp[1].x = object_x[i] + 16;
			vertices_tmp[1].y = object_y[i] + 16;

			vertices_tmp += 2;
		}

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2 * object_num, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);

	object_num = 0;
}


/*----------------------------------------------------------------
	Scroll1のテクスチャを更新
----------------------------------------------------------------*/

void blit_update_scroll1(int x, int y, u32 code, u32 attr)
{
	if ((x > 56 && x < 448) && (y > min_y_8 && y < clip_max_y))
	{
		u32 key = MAKE_KEY(code, ((attr & 0x1f) + 32));
		SPRITE *p = scroll1_head[key & SCROLL1_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				if (p->used != frames_displayed)
				{
					if (update_cache) update_cache((key & 0xffffff) << 6);
					p->used = frames_displayed;
				}
				return;
			}
			p = p->next;
		}
	}
}


/*----------------------------------------------------------------
	Scroll1を描画リストに登録
----------------------------------------------------------------*/

void blit_draw_scroll1(int x, int y, u32 code, u32 attr)
{
	if ((x > 56 && x < 448) && (y > min_y_8 && y < clip_max_y))
	{
		int idx;
		int color = (attr & 0x1f) + 32;
		u32 key = MAKE_KEY(code, color);

		if ((idx = scroll1_get_cache(key)) < 0)
		{
			int lines, u, v;
			u16 *dst, *pal;
			u32 *gfx, tile;
			u32 offset;

			if (scroll1_cached_num == scroll1_max - 1)
			{
				cps2_scan_scroll1_callback();
				scroll1_clean_cache(0);
			}

			idx = scroll1_insert_cache(key);
			u = (idx & 63) << 3;
			v = (idx >> 6) << 3;

			offset = (*read_cache)(code << 6);
			gfx = (u32 *)&memory_region_gfx1[offset];
			pal = &video_palette[color << 4];
			dst = &tex_scroll1[u + (v << 9)];
			lines = 8;

			while (lines--)
			{
				gfx++;
				tile = *gfx++;
				dst[/* 0*/ 7] = pal[(tile & 0xf0000000)>>28];
				dst[/* 1*/ 6] = pal[(tile & 0x0f000000)>>24];
				dst[/* 2*/ 5] = pal[(tile & 0x00f00000)>>20];
				dst[/* 3*/ 4] = pal[(tile & 0x000f0000)>>16];
				dst[/* 4*/ 3] = pal[(tile & 0x0000f000)>>12];
				dst[/* 5*/ 2] = pal[(tile & 0x00000f00)>> 8];
				dst[/* 6*/ 1] = pal[(tile & 0x000000f0)>> 4];
				dst[/* 7*/ 0] = pal[(tile & 0x0000000f)    ];
				dst += BUF_WIDTH;
			}
		}

		scroll1_idx[scroll1_num] = idx;
		scroll1_x[scroll1_num]   = x;
		scroll1_y[scroll1_num]   = y;
		scroll1_atr[scroll1_num] = attr;
		scroll1_num++;
	}
}


/*----------------------------------------------------------------
	Scroll1描画終了
----------------------------------------------------------------*/

void blit_finish_scroll1(void)
{
	int i, u, v;
	struct Vertex *vertices, *vertices_tmp;

	if (!scroll1_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * work_frame), BUF_WIDTH);
	sceGuScissor(cps_src_clip.left, clip_min_y, cps_src_clip.right, clip_max_y);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);

	sceGuTexImage(0, 512, 512, 512, tex_scroll1);

	vertices = (struct Vertex *)sceGuGetMemory(scroll1_num * 2 * sizeof(struct Vertex));
	if (vertices)
	{
		vertices_tmp = vertices;

		for (i = 0; i < scroll1_num; i++)
		{
			u = (scroll1_idx[i] & 63) << 3;
			v = (scroll1_idx[i] >> 6) << 3;

			if (scroll1_atr[i] & 0x20)
			{
				// flipx
				vertices_tmp[0].u = u + 8;
				vertices_tmp[1].u = u;
			}
			else
			{
				vertices_tmp[0].u = u;
				vertices_tmp[1].u = u + 8;
			}

			if (scroll1_atr[i] & 0x40)
			{
				// flipy
				vertices_tmp[0].v = v + 8;
				vertices_tmp[1].v = v;
			}
			else
			{
				vertices_tmp[0].v = v;
				vertices_tmp[1].v = v + 8;
			}

			vertices_tmp[0].x = scroll1_x[i];
			vertices_tmp[0].y = scroll1_y[i];

			vertices_tmp[1].x = scroll1_x[i] + 8;
			vertices_tmp[1].y = scroll1_y[i] + 8;

			vertices_tmp += 2;
		}

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2 * scroll1_num, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);

	scroll1_num = 0;
}


/*----------------------------------------------------------------
	Scroll2のテクスチャを更新
----------------------------------------------------------------*/

void blit_update_scroll2(int x, int y, u32 code, u32 attr)
{
	if ((x > 48 && x < 448) && (y > min_y_16 && y < clip_max_y))
	{
		u32 key = MAKE_KEY(code, ((attr & 0x1f) + 64));
		SPRITE *p = scroll2_head[key & SCROLL2_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				if (p->used != frames_displayed)
				{
					if (update_cache) update_cache((key & 0xffffff) << 7);
					p->used = frames_displayed;
				}
				return;
			}
			p = p->next;
		}
	}
}


/*----------------------------------------------------------------
	Scroll2を描画リストに登録
----------------------------------------------------------------*/

void blit_draw_scroll2(int x, int y, u32 code, u32 attr)
{
	if ((x > 48 && x < 448) && (y > min_y_16 && y < clip_max_y))
	{
		int idx;
		u16 color = (attr & 0x1f) + 64;
		u32 key = MAKE_KEY(code, color);

		if ((idx = scroll2_get_cache(key)) < 0)
		{
			int lines, u, v;
			u16 *dst, *pal;
			u32 *gfx, tile;
			u32 offset;

			if (scroll2_cached_num == scroll2_max - 1)
			{
				cps2_scan_scroll2_callback();
				scroll2_clean_cache(0);
			}

			idx = scroll2_insert_cache(key);
			u = (idx & 31) << 4;
			v = (idx >> 5) << 4;

			offset = (*read_cache)(code << 7);
			gfx = (u32 *)&memory_region_gfx1[offset];
			pal = &video_palette[color << 4];
			dst = &tex_scroll2[u + (v << 9)];
			lines = 16;

			while (lines--)
			{
				tile = *gfx++;
				dst[/* 0*/ 7] = pal[(tile & 0xf0000000)>>28];
				dst[/* 1*/ 6] = pal[(tile & 0x0f000000)>>24];
				dst[/* 2*/ 5] = pal[(tile & 0x00f00000)>>20];
				dst[/* 3*/ 4] = pal[(tile & 0x000f0000)>>16];
				dst[/* 4*/ 3] = pal[(tile & 0x0000f000)>>12];
				dst[/* 5*/ 2] = pal[(tile & 0x00000f00)>> 8];
				dst[/* 6*/ 1] = pal[(tile & 0x000000f0)>> 4];
				dst[/* 7*/ 0] = pal[(tile & 0x0000000f)    ];
				tile = *gfx++;
				dst[/* 8*/15] = pal[(tile & 0xf0000000)>>28];
				dst[/* 9*/14] = pal[(tile & 0x0f000000)>>24];
				dst[/*10*/13] = pal[(tile & 0x00f00000)>>20];
				dst[/*11*/12] = pal[(tile & 0x000f0000)>>16];
				dst[/*12*/11] = pal[(tile & 0x0000f000)>>12];
				dst[/*13*/10] = pal[(tile & 0x00000f00)>> 8];
				dst[/*14*/ 9] = pal[(tile & 0x000000f0)>> 4];
				dst[/*15*/ 8] = pal[(tile & 0x0000000f)    ];
				dst += BUF_WIDTH;
			}
		}

		scroll2_idx[scroll2_num] = idx;
		scroll2_x[scroll2_num]   = x;
		scroll2_y[scroll2_num]   = y;
		scroll2_atr[scroll2_num] = attr;
		scroll2_num++;
	}
}


/*----------------------------------------------------------------
	Scroll2描画終了
----------------------------------------------------------------*/

void blit_finish_scroll2(int min_y, int max_y)
{
	int i, u, v;
	struct Vertex *vertices, *vertices_tmp;

	if (!scroll2_num) return;

	if (min_y < clip_min_y) min_y = clip_min_y;
	if (max_y > clip_max_y) max_y = clip_max_y;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * work_frame), BUF_WIDTH);
	sceGuScissor(cps_src_clip.left, min_y, cps_src_clip.right, max_y);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);

	sceGuTexImage(0, 512, 512, 512, tex_scroll2);

	vertices = (struct Vertex *)sceGuGetMemory(scroll2_num * 2 * sizeof(struct Vertex));
	if (vertices)
	{
		vertices_tmp = vertices;

		for (i = 0; i < scroll2_num; i++)
		{
			u = (scroll2_idx[i] & 31) << 4;
			v = (scroll2_idx[i] >> 5) << 4;

			if (scroll2_atr[i] & 0x20)
			{
				// flipx
				vertices_tmp[0].u = u + 16;
				vertices_tmp[1].u = u;
			}
			else
			{
				vertices_tmp[0].u = u;
				vertices_tmp[1].u = u + 16;
			}

			if (scroll2_atr[i] & 0x40)
			{
				// flipy
				vertices_tmp[0].v = v + 16;
				vertices_tmp[1].v = v;
			}
			else
			{
				vertices_tmp[0].v = v;
				vertices_tmp[1].v = v + 16;
			}

			vertices_tmp[0].x = scroll2_x[i];
			vertices_tmp[0].y = scroll2_y[i];

			vertices_tmp[1].x = scroll2_x[i] + 16;
			vertices_tmp[1].y = scroll2_y[i] + 16;

			vertices_tmp += 2;
		}

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2 * scroll2_num, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);

	scroll2_num = 0;
}


/*----------------------------------------------------------------
	Scroll3のテクスチャを更新
----------------------------------------------------------------*/

void blit_update_scroll3(int x, int y, u32 code, u32 attr)
{
	if ((x > 32 && x < 448) && (y > min_y_32 && y < clip_max_y))
	{
		u32 key = MAKE_KEY(code, ((attr & 0x1f) + 96));
		SPRITE *p = scroll3_head[key & SCROLL3_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				if (p->used != frames_displayed)
				{
					if (update_cache) update_cache((key & 0xffffff) << 9);
					p->used = frames_displayed;
				}
				return;
			}
			p = p->next;
		}
	}
}


/*----------------------------------------------------------------
	Scroll3を描画リストに登録
----------------------------------------------------------------*/

void blit_draw_scroll3(int x, int y, u32 code, u32 attr)
{
	if ((x > 32 && x < 448) && (y > min_y_32 && y < clip_max_y))
	{
		int idx;
		int color = (attr & 0x1f) + 96;
		u32 key = MAKE_KEY(code, color);

		if ((idx = scroll3_get_cache(key)) < 0)
		{
			int lines, u, v;
			u16 *dst, *pal;
			u32 *gfx, tile;
			u32 offset;

			if (scroll3_cached_num == scroll3_max - 1)
			{
				cps2_scan_scroll3_callback();
				scroll3_clean_cache(0);
			}

			idx = scroll3_insert_cache(key);
			u = (idx & 15) << 5;
			v = (idx >> 4) << 5;

			offset = (*read_cache)(code << 9);
			gfx = (u32 *)&memory_region_gfx1[offset];
			pal = &video_palette[color << 4];
			dst = &tex_scroll3[u + (v << 9)];
			lines = 32;

			while (lines--)
			{
				tile = *gfx++;
				dst[/* 0*/ 7] = pal[(tile & 0xf0000000)>>28];
				dst[/* 1*/ 6] = pal[(tile & 0x0f000000)>>24];
				dst[/* 2*/ 5] = pal[(tile & 0x00f00000)>>20];
				dst[/* 3*/ 4] = pal[(tile & 0x000f0000)>>16];
				dst[/* 4*/ 3] = pal[(tile & 0x0000f000)>>12];
				dst[/* 5*/ 2] = pal[(tile & 0x00000f00)>> 8];
				dst[/* 6*/ 1] = pal[(tile & 0x000000f0)>> 4];
				dst[/* 7*/ 0] = pal[(tile & 0x0000000f)    ];
				tile = *gfx++;
				dst[/* 8*/15] = pal[(tile & 0xf0000000)>>28];
				dst[/* 9*/14] = pal[(tile & 0x0f000000)>>24];
				dst[/*10*/13] = pal[(tile & 0x00f00000)>>20];
				dst[/*11*/12] = pal[(tile & 0x000f0000)>>16];
				dst[/*12*/11] = pal[(tile & 0x0000f000)>>12];
				dst[/*13*/10] = pal[(tile & 0x00000f00)>> 8];
				dst[/*14*/ 9] = pal[(tile & 0x000000f0)>> 4];
				dst[/*15*/ 8] = pal[(tile & 0x0000000f)    ];
				tile = *gfx++;
				dst[/*16*/23] = pal[(tile & 0xf0000000)>>28];
				dst[/*17*/22] = pal[(tile & 0x0f000000)>>24];
				dst[/*18*/21] = pal[(tile & 0x00f00000)>>20];
				dst[/*19*/20] = pal[(tile & 0x000f0000)>>16];
				dst[/*20*/19] = pal[(tile & 0x0000f000)>>12];
				dst[/*21*/18] = pal[(tile & 0x00000f00)>> 8];
				dst[/*22*/17] = pal[(tile & 0x000000f0)>> 4];
				dst[/*23*/16] = pal[(tile & 0x0000000f)    ];
				tile = *gfx++;
				dst[/*24*/31] = pal[(tile & 0xf0000000)>>28];
				dst[/*25*/30] = pal[(tile & 0x0f000000)>>24];
				dst[/*26*/29] = pal[(tile & 0x00f00000)>>20];
				dst[/*27*/28] = pal[(tile & 0x000f0000)>>16];
				dst[/*28*/27] = pal[(tile & 0x0000f000)>>12];
				dst[/*29*/26] = pal[(tile & 0x00000f00)>> 8];
				dst[/*30*/25] = pal[(tile & 0x000000f0)>> 4];
				dst[/*31*/24] = pal[(tile & 0x0000000f)    ];
				dst += BUF_WIDTH;
			}
		}

		scroll3_idx[scroll3_num] = idx;
		scroll3_x[scroll3_num]   = x;
		scroll3_y[scroll3_num]   = y;
		scroll3_atr[scroll3_num] = attr;
		scroll3_num++;
	}
}


/*----------------------------------------------------------------
	Scroll3描画終了
----------------------------------------------------------------*/

void blit_finish_scroll3(void)
{
	int i, u, v;
	struct Vertex *vertices, *vertices_tmp;

	if (!scroll3_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, (void *)(FRAMESIZE * work_frame), BUF_WIDTH);
	sceGuScissor(cps_src_clip.left, clip_min_y, cps_src_clip.right, clip_max_y);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);

	sceGuTexImage(0, 512, 512, 512, tex_scroll3);

	vertices = (struct Vertex *)sceGuGetMemory(scroll3_num * 2 * sizeof(struct Vertex));
	if (vertices)
	{
		vertices_tmp = vertices;

		for (i = 0; i < scroll3_num; i++)
		{
			u = (scroll3_idx[i] & 15) << 5;
			v = (scroll3_idx[i] >> 4) << 5;

			if (scroll3_atr[i] & 0x20)
			{
				// flipx
				vertices_tmp[0].u = u + 32;
				vertices_tmp[1].u = u;
			}
			else
			{
				vertices_tmp[0].u = u;
				vertices_tmp[1].u = u + 32;
			}

			if (scroll3_atr[i] & 0x40)
			{
				// flipy
				vertices_tmp[0].v = v + 32;
				vertices_tmp[1].v = v;
			}
			else
			{
				vertices_tmp[0].v = v;
				vertices_tmp[1].v = v + 32;
			}

			vertices_tmp[0].x = scroll3_x[i];
			vertices_tmp[0].y = scroll3_y[i];

			vertices_tmp[1].x = scroll3_x[i] + 32;
			vertices_tmp[1].y = scroll3_y[i] + 32;

			vertices_tmp += 2;
		}

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2 * scroll3_num, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);

	scroll3_num = 0;
}


/*----------------------------------------------------------------
	メモリキャッシュを更新する
----------------------------------------------------------------*/

void blit_update_all_cache(void)
{
	cache_set_update_func(1);
	cps2_scan_scroll1_callback();
	cps2_scan_scroll2_callback();
	cps2_scan_scroll3_callback();
	cps2_scan_sprites_callback();
	cache_set_update_func(0);
}


/*----------------------------------------------------------------
	マスク処理付きobjectの描画開始
----------------------------------------------------------------*/

static int mask_flag;
static int mask_min_x;
static int mask_max_x;
static int mask_min_y;
static int mask_max_y;
static int obj_min_x;
static int obj_max_x;
static int obj_min_y;
static int obj_max_y;

void blit_masked_object_start(int flag)
{
	if (flag & MASK_CLEAR_SCREEN)
	{
		int x, y;
		u32 *dst = (u32 *)video_frame_addr(draw_frame, 64, 16);

		for (y = 0; y < 224; y++)
		{
			for (x = 0; x < (384 >> 1); x++)
				dst[x] = 0x80008000;
			dst += BUF_WIDTH >> 1;
		}
	}

	flag &= MASK_COPYRECT_BIT;

	if (flag)
	{
		mask_min_x = 1024;
		mask_min_y = 1024;
		mask_max_x = 0;
		mask_max_y = 0;
		obj_min_x  = 1024;
		obj_min_y  = 1024;
		obj_max_x  = 0;
		obj_max_y  = 0;
	}

	mask_flag = flag;
}


/*----------------------------------------------------------------
	マスク処理付きobjectの描画終了
----------------------------------------------------------------*/

void blit_masked_object_finish(void)
{
	RECT clip = { 64, 16, 448, 248 };

	if (mask_flag)
	{
		switch (mask_flag)
		{
		case MASK_COPYRECT_MINIMUM:
			if (obj_min_x > mask_min_x) mask_min_x = obj_min_x;
			if (obj_max_x < mask_max_x) mask_max_x = obj_max_x;
			if (obj_min_y > mask_min_y) mask_min_y = obj_min_y;
			if (obj_max_y < mask_max_y) mask_max_y = obj_max_y;
			break;

		case MASK_COPYRECT_MAXIMUM:
			if (obj_min_x < mask_min_x) mask_min_x = obj_min_x;
			if (obj_max_x > mask_max_x) mask_max_x = obj_max_x;
			if (obj_min_y < mask_min_y) mask_min_y = obj_min_y;
			if (obj_max_y > mask_max_y) mask_max_y = obj_max_y;
			break;
		}

		if (mask_min_x < 64)  mask_min_x = 64;
		if (mask_max_x > 448) mask_max_x = 448;
		if (mask_min_y < clip_min_y) mask_min_y = clip_min_y;
		if (mask_max_y > clip_max_y) mask_max_y = clip_max_y;

		clip.top    = mask_min_y;
		clip.bottom = mask_max_y;
		clip.left   = mask_min_x;
		clip.right  = mask_max_x;
	}

	video_copy_rect_alpha(draw_frame, work_frame, &clip, &clip);
	video_clear_frame(draw_frame);
}


/*----------------------------------------------------------------
	マスク処理付きobjectを描画
----------------------------------------------------------------*/

void blit_masked_object(int x, int y, u32 code, u32 attr)
{
	if ((x > 48 && x < 448) && (y > min_y_16 && y < clip_max_y))
	{
		u16 *dst, *pal;
		u32 *gfx, tile, col, offset;
		int lines = 16;

		if (mask_flag)
		{
			if (x < obj_min_x) obj_min_x = x;
			if (y < obj_min_y) obj_min_y = y;
			if (x + 16 > obj_max_x) obj_max_x = x + 16;
			if (y + 16 > obj_max_y) obj_max_y = y + 16;
		}

		offset = (*read_cache)(code << 7);
		gfx = (u32 *)&memory_region_gfx1[offset];
		pal = &video_palette[(attr & 0x1f) << 4];

		if (attr & 0x40)
		{
			dst = video_frame_addr(draw_frame, x, y + 15);

			if (attr & 0x20)
			{
				while (lines--)
				{
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/*15*/ 8] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/*14*/ 9] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/*13*/10] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/*12*/11] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/*11*/12] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/*10*/13] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/* 9*/14] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/* 8*/15] = pal[(col    )];
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/* 7*/ 0] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/* 6*/ 1] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/* 5*/ 2] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/* 4*/ 3] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/* 3*/ 4] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/* 2*/ 5] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/* 1*/ 6] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/* 0*/ 7] = pal[(col    )];
					dst -= BUF_WIDTH;
				}
			}
			else
			{
				while (lines--)
				{
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/* 0*/ 7] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/* 1*/ 6] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/* 2*/ 5] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/* 3*/ 4] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/* 4*/ 3] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/* 5*/ 2] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/* 6*/ 1] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/* 7*/ 0] = pal[(col    )];
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/* 8*/15] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/* 9*/14] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/*10*/13] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/*11*/12] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/*12*/11] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/*13*/10] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/*14*/ 9] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/*15*/ 8] = pal[(col    )];
					dst -= BUF_WIDTH;
				}
			}
		}
		else
		{
			dst = video_frame_addr(draw_frame, x, y);

			if (attr & 0x20)
			{
				while (lines--)
				{
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/*15*/ 8] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/*14*/ 9] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/*13*/10] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/*12*/11] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/*11*/12] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/*10*/13] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/* 9*/14] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/* 8*/15] = pal[(col    )];
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/* 7*/ 0] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/* 6*/ 1] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/* 5*/ 2] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/* 4*/ 3] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/* 3*/ 4] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/* 2*/ 5] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/* 1*/ 6] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/* 0*/ 7] = pal[(col    )];
					dst += BUF_WIDTH;
				}
			}
			else
			{
				while (lines--)
				{
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/* 0*/ 7] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/* 1*/ 6] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/* 2*/ 5] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/* 3*/ 4] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/* 4*/ 3] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/* 5*/ 2] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/* 6*/ 1] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/* 7*/ 0] = pal[(col    )];
					tile = *gfx++;
					col = tile & 0xf0000000; if (col != 0xf0000000) dst[/* 8*/15] = pal[(col>>28)];
					col = tile & 0x0f000000; if (col != 0x0f000000) dst[/* 9*/14] = pal[(col>>24)];
					col = tile & 0x00f00000; if (col != 0x00f00000) dst[/*10*/13] = pal[(col>>20)];
					col = tile & 0x000f0000; if (col != 0x000f0000) dst[/*11*/12] = pal[(col>>16)];
					col = tile & 0x0000f000; if (col != 0x0000f000) dst[/*12*/11] = pal[(col>>12)];
					col = tile & 0x00000f00; if (col != 0x00000f00) dst[/*13*/10] = pal[(col>> 8)];
					col = tile & 0x000000f0; if (col != 0x000000f0) dst[/*14*/ 9] = pal[(col>> 4)];
					col = tile & 0x0000000f; if (col != 0x0000000f) dst[/*15*/ 8] = pal[(col    )];
					dst += BUF_WIDTH;
				}
			}
		}
	}
}


/*----------------------------------------------------------------
	マスク処理付きobjectのマスクを描画
----------------------------------------------------------------*/

void blit_mask(int x, int y, u32 code, u32 attr)
{
	if ((x > 48 && x < 448) && (y > min_y_16 && y < clip_max_y))
	{
		u16 *dst;
		u32 *gfx, tile, offset;
		int lines = 16;

		if (mask_flag)
		{
			if (x < mask_min_x) mask_min_x = x;
			if (y < mask_min_y) mask_min_y = y;
			if (x + 16 > mask_max_x) mask_max_x = x + 16;
			if (y + 16 > mask_max_y) mask_max_y = y + 16;
		}

		offset = (*read_cache)(code << 7);
		gfx = (u32 *)&memory_region_gfx1[offset];

		if (attr & 0x40)
		{
			dst = video_frame_addr(draw_frame, x, y + 15);

			if (attr & 0x20)
			{
				while (lines--)
				{
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/*15*/ 8] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/*14*/ 9] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/*13*/10] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/*12*/11] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/*11*/12] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/*10*/13] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/* 9*/14] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/* 8*/15] = 0x8000;
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/* 7*/ 0] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/* 6*/ 1] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/* 5*/ 2] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/* 4*/ 3] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/* 3*/ 4] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/* 2*/ 5] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/* 1*/ 6] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/* 0*/ 7] = 0x8000;
					dst -= BUF_WIDTH;
				}
			}
			else
			{
				while (lines--)
				{
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/* 0*/ 7] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/* 1*/ 6] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/* 2*/ 5] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/* 3*/ 4] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/* 4*/ 3] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/* 5*/ 2] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/* 6*/ 1] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/* 7*/ 0] = 0x8000;
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/* 8*/15] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/* 9*/14] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/*10*/13] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/*11*/12] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/*12*/11] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/*13*/10] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/*14*/ 9] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/*15*/ 8] = 0x8000;
					dst -= BUF_WIDTH;
				}
			}
		}
		else
		{
			dst = video_frame_addr(draw_frame, x, y);

			if (attr & 0x20)
			{
				while (lines--)
				{
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/*15*/ 8] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/*14*/ 9] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/*13*/10] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/*12*/11] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/*11*/12] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/*10*/13] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/* 9*/14] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/* 8*/15] = 0x8000;
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/* 7*/ 0] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/* 6*/ 1] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/* 5*/ 2] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/* 4*/ 3] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/* 3*/ 4] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/* 2*/ 5] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/* 1*/ 6] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/* 0*/ 7] = 0x8000;
					dst += BUF_WIDTH;
				}
			}
			else
			{
				while (lines--)
				{
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/* 0*/ 7] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/* 1*/ 6] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/* 2*/ 5] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/* 3*/ 4] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/* 4*/ 3] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/* 5*/ 2] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/* 6*/ 1] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/* 7*/ 0] = 0x8000;
					tile = *gfx++;
					if ((tile & 0xf0000000) != 0xf0000000) dst[/* 8*/15] = 0x8000;
					if ((tile & 0x0f000000) != 0x0f000000) dst[/* 9*/14] = 0x8000;
					if ((tile & 0x00f00000) != 0x00f00000) dst[/*10*/13] = 0x8000;
					if ((tile & 0x000f0000) != 0x000f0000) dst[/*11*/12] = 0x8000;
					if ((tile & 0x0000f000) != 0x0000f000) dst[/*12*/11] = 0x8000;
					if ((tile & 0x00000f00) != 0x00000f00) dst[/*13*/10] = 0x8000;
					if ((tile & 0x000000f0) != 0x000000f0) dst[/*14*/ 9] = 0x8000;
					if ((tile & 0x0000000f) != 0x0000000f) dst[/*15*/ 8] = 0x8000;
					dst += BUF_WIDTH;
				}
			}
		}
	}
}
