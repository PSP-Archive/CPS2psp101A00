/******************************************************************************

OUTPUT PORTS
0x00-0x01     OBJ RAM base (/256)
0x02-0x03     Scroll1 (8x8) RAM base (/256)
0x04-0x05     Scroll2 (16x16) RAM base (/256)
0x06-0x07     Scroll3 (32x32) RAM base (/256)
0x08-0x09     rowscroll RAM base (/256)
0x0a-0x0b     Palette base (/256)
0x0c-0x0d     Scroll 1 X
0x0e-0x0f     Scroll 1 Y
0x10-0x11     Scroll 2 X
0x12-0x13     Scroll 2 Y
0x14-0x15     Scroll 3 X
0x16-0x17     Scroll 3 Y
0x18-0x19     Starfield 1 X
0x1a-0x1b     Starfield 1 Y
0x1c-0x1d     Starfield 2 X
0x1e-0x1f     Starfield 2 Y
0x20-0x21     start offset for the rowscroll matrix
0x22-0x23     unknown but widely used - usually 0x0e. bit 0 enables rowscroll
              on layer 2. bit 15 is flip screen.


Some registers move from game to game.. following example strider
0x66-0x67   Layer control register
            bits 14-15 seem to be unused
                ghouls sets bits 15 in service mode when you press button 2 in
                the input test
            bits 6-13 (4 groups of 2 bits) select layer draw order
            bits 1-5 enable the three tilemap layers and the two starfield
                layers (the bit order changes from game to game).
                Only Forgotten Worlds and Strider use the starfield.
            bit 0 could be rowscroll related. It is set by captain commando,
                varth, mtwins, mssword, cawing while rowscroll is active. However
                kodj and sf2 do NOT set this bit while they are using rowscroll.
                Games known to use row scrolling:
                SF2
                Mega Twins (underwater, cave)
                Carrier Air Wing (hazy background at beginning of mission 8, put 07 at ff8501 to jump there)
                Magic Sword (fire on floor 3; screen distort after continue)
                Varth (title screen)
                Captain Commando (end game sequence)
0x68-0x69   Priority mask \   Tiles in the layer just below sprites can have
0x6a-0x6b   Priority mask |   four priority levels, each one associated with one
0x6c-0x6d   Priority mask |   of these masks. The masks indicate pens in the tile
0x6e-0x6f   Priority mask /   that have priority over sprites.
0x70-0x71   Control register (usually 0x003f). The details of how this register
            works are unknown, but it definitely affects the palette; experiments
            on the real board show that values different from 0x3f in the low 6
            bits cause wrong colors. The other bits seem to be unused.
            There is one CPS2 game (Slammasters II) setting this to 0x2f: the
            purpose is unknown.
            The only other places where this register seems to be set to a value
            different from 0x3f is during startup tests. Examples:
            ghouls  0x02
            strider 0x02
            unsquad 0x0f
            kod     0x0f
            mtwins  0x0f

Fixed registers
0x80-0x81     Sound command
0x88-0x89     Sound fade

Known Bug List
==============
CPS2:
* CPS2 can do raster effects, certainly used by ssf2 (Cammy, DeeJay, T.Hawk levels),
  msh (lava level, early in attract mode) and maybe others (xmcotaj, vsavj).
  IRQ4 is some sort of scanline interrupt used for that purpose.

* Its unknown what CPS2_OBJ_BASE register (0x400000) does but it is not a object base
  register. All games use 0x7000 even if 0x7080 is used at this register (checked on
  real HW). Maybe it sets the object bank used when cps2_objram_bank is set.

* Sprites are currently lagged by one frame to keep sync with backgrounds. This causes
  sprites to stay on screen one frame longer (visable in VSAV attract mode).

Marvel Vs. Capcom
* Sometimes currupt gfx are displayed on the 32x32 layer as the screen flashes at the
  start of super combo moves. The problem seems to be due to tiles being fetched before
  the first 32x32 tile offset and results in data coming from 16x16 or 8x8 tiles instead.

Unknown issues
==============

There are often some redundant high bits in the scroll layer's attributes.
I think that these are spare bits that the game uses for to store additional
information, not used by the hardware.
The games seem to use them to mark platforms, kill zones and no-go areas.

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

int cps_flip_screen;
int cps_rotate_screen;
int cps_raster_enable;

int cps2_objram_bank;
int scanline1;
int scanline2;


/******************************************************************************
	ローカル変数
******************************************************************************/

#define cps2_scroll_size		0x4000		/* scroll1, scroll2, scroll3 */
#define cps2_other_size			0x0800		/* line scroll offset etc. */
#define cps2_palette_align		0x0800		/* can't be larger than this, breaks ringdest & batcirc otherwise */
#define cps2_palette_size		4*32*16*2	/* Size of palette RAM */

#define cps2_scroll_mask		(((~0x3fff) & 0x3ffff) >> 1)
#define cps2_other_mask			(((~0x07ff) & 0x3ffff) >> 1)
#define cps2_palette_mask		(((~0x07ff) & 0x3ffff) >> 1)

static u16 *cps_scroll1;
static u16 *cps_scroll2;
static u16 *cps_scroll3;
static u16 *cps_other;
static u16 *cps2_palette;
static u16 cps2_old_palette[cps2_palette_size >> 1];

static int cps_layer_enabled[4];		/* Layer enabled [Y/N] */
static int cps_scroll1x, cps_scroll1y;
static int cps_scroll2x, cps_scroll2y;
static int cps_scroll3x, cps_scroll3y;
static u32 cps_total_elements[3];		/* total sprites num */
static u8  *cps_pen_usage[3];			/* sprites pen usage */

#define cps2_obj_size			0x2000
#define cps2_max_obj			(cps2_obj_size >> 3)
static u16 cps2_buffered_obj[cps2_obj_size >> 1];
static u16 cps2_buffered_palette[1*32*16];
static u8 cps2_obj_priority[cps2_max_obj];

static int cps2_last_sprite_offset;	/* Offset of the last sprite */
static int cps2_pri_ctrl;			/* Sprite layer priorities */
static int cps2_scanline_start;
static int cps2_scanline_end;
static int cps2_scroll3_base;
static u8 cps2_kludge;

static int cps2_mask_number;
static int cps2_mask_max;
static int cps2_mask_pri[3];
static int cps2_obj_pri[3];
static int cps2_mask_flag[3];

static u16 video_clut16[65536];
u16 video_palette[cps2_palette_size >> 1];


/* CPS1 output port */
#define CPS1_OBJ_BASE			0x00    /* Base address of objects */
#define CPS1_SCROLL1_BASE		0x02    /* Base address of scroll 1 */
#define CPS1_SCROLL2_BASE		0x04    /* Base address of scroll 2 */
#define CPS1_SCROLL3_BASE		0x06    /* Base address of scroll 3 */
#define CPS1_OTHER_BASE			0x08    /* Base address of other video */
#define CPS1_PALETTE_BASE		0x0a    /* Base address of palette */
#define CPS1_SCROLL1_SCROLLX	0x0c    /* Scroll 1 X */
#define CPS1_SCROLL1_SCROLLY	0x0e    /* Scroll 1 Y */
#define CPS1_SCROLL2_SCROLLX	0x10    /* Scroll 2 X */
#define CPS1_SCROLL2_SCROLLY	0x12    /* Scroll 2 Y */
#define CPS1_SCROLL3_SCROLLX	0x14    /* Scroll 3 X */
#define CPS1_SCROLL3_SCROLLY	0x16    /* Scroll 3 Y */
#define CPS1_VIDEO_CONTROL		0x22    /* Video control */

#define CPS2_PROT_FACTOR1		0x40	/* Multiply protection (factor1) */
#define CPS2_PROT_FACTOR2		0x42	/* Multiply protection (factor2) */
#define CPS2_PROT_RESULT_LO		0x44	/* Multiply protection (result low) */
#define CPS2_PROT_RESULT_HI		0x46	/* Multiply protection (result high) */
#define CPS2_SCANLINE1			0x50	/* Scanline interrupt 1 */
#define CPS2_SCANLINE2			0x52	/* Scanline interrupt 2 */
#define CPS2_LAYER_CONTROL		0x66	/* Layer control */

/* CPS1 other RAM */
#define CPS1_ROWSCROLL_OFFS		0x20    /* base of row scroll offsets in other RAM */
#define CPS1_SCROLL2_WIDTH		0x40
#define CPS1_SCROLL2_HEIGHT		0x40

/* CPS2 object RAM */
#define CPS2_OBJ_BASE			0x00	/* Unknown (not base address of objects). Could be bass address of bank used when object swap bit set? */
#define CPS2_OBJ_UK1			0x02	/* Unknown (nearly always 0x807d, or 0x808e when screen flipped) */
#define CPS2_OBJ_PRI			0x04	/* Layers priorities */
#define CPS2_OBJ_UK2			0x06	/* Unknown (usually 0x0000, 0x1101 in ssf2, 0x0001 in 19XX) */
#define CPS2_OBJ_XOFFS			0x08	/* X offset (usually 0x0040) */
#define CPS2_OBJ_YOFFS			0x0a	/* Y offset (always 0x0010) */

/* pen usage flag */
#define SPR_MASK1				0x01
#define SPR_MASK2				0x02
#define SPR_MASK3				0x04
#define SPR_MASKED_OBJ			0x40
#define SPR_NOT_EMPTY			0x80

/*------------------------------------------------------
	CPSポート読み込み
------------------------------------------------------*/

#define cps1_port(offset)	cps1_output[(offset) >> 1]
#define cps2_port(offset)	cps2_output[(offset) >> 1]


void (*cps2_build_palette)(void);
static void cps2_build_palette_normal(void);
static void cps2_build_palette_delay(void);

/*------------------------------------------------------
	CPS2スクロール2更新
------------------------------------------------------*/

static void (*cps2_render_scroll2_raster_disabled)(int otheroffs);
static void cps2_render_scroll2_raster_disabled1(int otheroffs);
static void cps2_render_scroll2_raster_disabled2(int otheroffs);


/******************************************************************************
	CPS2 メモリハンドラ
******************************************************************************/

READ16_O1_HANDLER( cps1_output_r )
{
	offset &= 0x7f;

	switch (offset/*<<1*/)
	{
	case (CPS2_PROT_RESULT_LO>>1):
		return (cps1_port(CPS2_PROT_FACTOR1) * cps1_port(CPS2_PROT_FACTOR2)) & 0xffff;

	case (CPS2_PROT_RESULT_HI>>1):
		return (cps1_port(CPS2_PROT_FACTOR1) * cps1_port(CPS2_PROT_FACTOR2)) >> 16;
	}
	return cps1_output[offset];
}

WRITE16_HANDLER( cps1_output_w )
{
	offset &= 0x7f;
	data = COMBINE_DATA(&cps1_output[offset]);

	switch (offset/*<<1*/)
	{
	case (CPS2_SCANLINE1>>1):
		cps1_port(CPS2_SCANLINE1) &= 0x1ff;
		scanline1 = data & 0x1ff;
		break;

	case (CPS2_SCANLINE2>>1):
		cps1_port(CPS2_SCANLINE2) &= 0x1ff;
		scanline2 = data & 0x1ff;
		break;
	}
}


/******************************************************************************
	CPS2 ビデオ描画処理
******************************************************************************/

/*------------------------------------------------------
	カラーテーブル作成
------------------------------------------------------*/

static void cps2_init_tables(void)
{
	int r, g, b, bright;

	for (bright = 0; bright < 16; bright++)
	{
		for (r = 0; r < 16; r++)
		{
			for (g = 0; g < 16; g++)
			{
				for (b = 0; b < 16; b++)
				{
					u16 pen;
					int r2, g2, b2, bright2;
					float fr, fg, fb;

					pen = (bright << 12) | (r << 8) | (g << 4) | b;

					bright2 = bright + 16;

					fr = (float)(r * bright2) / (15.0 * 31.0);
					fg = (float)(g * bright2) / (15.0 * 31.0);
					fb = (float)(b * bright2) / (15.0 * 31.0);

					r2 = (int)(fr * 255.0) - 15;
					g2 = (int)(fg * 255.0) - 15;
					b2 = (int)(fb * 255.0) - 15;

					if (r2 < 0) r2 = 0;
					if (g2 < 0) g2 = 0;
					if (b2 < 0) b2 = 0;

					video_clut16[pen] = MAKECOL15(r2, g2, b2);
				}
			}
		}
	}
}


/*------------------------------------------------------
	CPS1ベースオフセット取得
------------------------------------------------------*/

static u16 *cps1_base(int offset, int address_mask)
{
	return &cps1_gfxram[((cps1_port(offset) << 7) & address_mask)];
}


/*------------------------------------------------------
	CPS1ビデオ関連の値を更新
------------------------------------------------------*/

void cps1_get_video_base(void)
{
	int layer_ctrl;

	/* Re-calculate the VIDEO RAM base */
	cps_scroll1 = cps1_base(CPS1_SCROLL1_BASE, cps2_scroll_mask);
	cps_scroll2 = cps1_base(CPS1_SCROLL2_BASE, cps2_scroll_mask);
	cps_scroll3 = cps1_base(CPS1_SCROLL3_BASE, cps2_scroll_mask);
	cps_other   = cps1_base(CPS1_OTHER_BASE, cps2_other_mask);

	/* Get scroll values */
	cps_scroll1x = cps1_port(CPS1_SCROLL1_SCROLLX);
	cps_scroll1y = cps1_port(CPS1_SCROLL1_SCROLLY);
	cps_scroll2x = cps1_port(CPS1_SCROLL2_SCROLLX);
	cps_scroll2y = cps1_port(CPS1_SCROLL2_SCROLLY);
	cps_scroll3x = cps1_port(CPS1_SCROLL3_SCROLLX);
	cps_scroll3y = cps1_port(CPS1_SCROLL3_SCROLLY);

	/* Get layer enable bits */
	layer_ctrl = cps1_port(CPS2_LAYER_CONTROL);
	cps_layer_enabled[1] = layer_ctrl & 2;
	cps_layer_enabled[2] = layer_ctrl & 4;
	cps_layer_enabled[3] = layer_ctrl & 8;
}


/*------------------------------------------------------
	CPS2ビデオ初期化
------------------------------------------------------*/

int cps2_video_init(void)
{
	int i;

	cps2_mask_max = 0;

	for (i = 0; i < 3; i++)
	{
		cps2_mask_flag[i] = driver->mask[i].flag;
		cps2_mask_pri[i]  = driver->mask[i].mask_priority;
		cps2_obj_pri[i]   = driver->mask[i].obj_priority;

		if (cps2_mask_pri[i] >= 0) cps2_mask_max++;
	}
	cps2_kludge       = driver->kludge;
	cps2_scroll3_base = (driver->kludge & (CPS2_KLUDGE_SSF2 | CPS2_KLUDGE_SSF2T)) ? 0x0000 : 0x4000;

	if (driver->flags & 1)
		cps2_render_scroll2_raster_disabled = cps2_render_scroll2_raster_disabled2;
	else
		cps2_render_scroll2_raster_disabled = cps2_render_scroll2_raster_disabled1;

	if (driver->flags & 2)
		cps2_build_palette = NULL;
	else
		cps2_build_palette = cps2_build_palette_normal;

	cps_total_elements[TILE08] = gfx_total_elements[TILE08] + 0x20000;
	cps_total_elements[TILE16] = gfx_total_elements[TILE16];
	cps_total_elements[TILE32] = gfx_total_elements[TILE32] + 0x4000;

	cps_pen_usage[TILE08] = gfx_pen_usage[TILE08] - 0x20000;
	cps_pen_usage[TILE16] = gfx_pen_usage[TILE16];
	cps_pen_usage[TILE32] = gfx_pen_usage[TILE32] - 0x4000;

	cps2_init_tables();

	return 1;
}


/*------------------------------------------------------
	CPS2ビデオ終了
------------------------------------------------------*/

void cps2_video_exit(void)
{
}


/*------------------------------------------------------
	CPS2ビデオリセット
------------------------------------------------------*/

void  cps2_video_reset(void)
{
	int i;

	memset(cps1_gfxram, 0, sizeof(cps1_gfxram));
	memset(cps1_output, 0, sizeof(cps1_output));

	memset(cps2_buffered_obj, 0, cps2_obj_size);
	memset(cps2_objram[0], 0, cps2_obj_size);
	memset(cps2_objram[1], 0, cps2_obj_size);

	memset(cps2_old_palette, 0, sizeof(cps2_old_palette));
	memset(video_palette, 0, sizeof(video_palette));
	memset(cps2_buffered_palette, 0, sizeof(cps2_buffered_palette));

	for (i = 0; i < 2048; i += 16)
		video_palette[i + 15] = 0x8000;

	/* Put in some defaults */
	cps1_port(CPS1_OBJ_BASE)     = 0x9200;
	cps1_port(CPS1_SCROLL1_BASE) = 0x9000;
	cps1_port(CPS1_SCROLL2_BASE) = 0x9040;
	cps1_port(CPS1_SCROLL3_BASE) = 0x9080;
	cps1_port(CPS1_OTHER_BASE)   = 0x9100;
	cps1_port(CPS1_PALETTE_BASE) = 0x90c0;

	/* Set up old base */
	cps1_get_video_base();   /* Calculate base pointers */
	cps1_get_video_base();   /* Calculate old base pointers */

	scanline1 = 262;
	scanline2 = 262;

	blit_reset();
}


/*------------------------------------------------------
	パレット
------------------------------------------------------*/

static void cps2_build_palette_normal(void)
{
	int offset;
	u16 palette;

	cps2_palette = cps1_base(CPS1_PALETTE_BASE, cps2_palette_mask);

	for (offset = 0; offset < 2048; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps2_palette[offset];

			if (palette != cps2_old_palette[offset])
			{
				cps2_old_palette[offset] = palette;
				video_palette[offset] = video_clut16[palette];
				blit_palette_mark_dirty(offset >> 4);
			}
		}
	}
}


/*------------------------------------------------------
	パレット(xmcota用 objectのみ1フレーム遅らせて反映)
------------------------------------------------------*/

static void cps2_build_palette_delay(void)
{
	int offset;
	u16 palette;

	cps2_palette = cps1_base(CPS1_PALETTE_BASE, cps2_palette_mask);

	for (offset = 0; offset < 512; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps2_buffered_palette[offset];

			if (palette != video_palette[offset])
			{
				video_palette[offset] = palette;
				blit_palette_mark_dirty(offset >> 4);
			}

			palette = cps2_palette[offset];

			if (palette != cps2_old_palette[offset])
			{
				cps2_old_palette[offset] = palette;
				cps2_buffered_palette[offset] = video_clut16[palette];
			}
		}
	}

	for (; offset < 2048; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps2_palette[offset];

			if (palette != cps2_old_palette[offset])
			{
				cps2_old_palette[offset] = palette;
				video_palette[offset] = video_clut16[palette];
				blit_palette_mark_dirty(offset >> 4);
			}
		}
	}
}


/******************************************************************************
  Sprites (16x16)
******************************************************************************/

/*------------------------------------------------------
	objectテーブルチェック
------------------------------------------------------*/

static void cps2_find_last_sprite(void)
{
	int offset = 0;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;

	while (offset < cps2_obj_size >> 1)
	{
		if (base[offset] | base[offset + 3])
			*pri = (base[offset] >> 13) & 0x07;
		else
			*pri = 0xff;

		if (base[offset + 1] >= 0x8000 || base[offset + 3] >= 0xff00)
		{
			cps2_last_sprite_offset = offset;
			return;
		}
		offset += 4;
		pri++;
	}
	cps2_last_sprite_offset = cps2_obj_size >> 1;
}


/*------------------------------------------------------
	object描画
------------------------------------------------------*/

#define SCAN_SPRITE(blit_func)										\
	if (attr & 0x80)												\
	{																\
		sx += (64/*-screen_xoffs*/)/*cps2_port(CPS2_OBJ_XOFFS)*/;	\
		sy += (16/*-screen_yoffs*/)/*cps2_port(CPS2_OBJ_YOFFS)*/;	\
/*	sx += screen_xoffs;*/											\
/*	sy += screen_yoffs;*/											\
	}else{															\
	sx += screen_xoffs;												\
	sy += screen_yoffs;												\
	}																\
	if (!(attr & 0xff00))											\
	{																\
		if (cps_pen_usage[TILE16][code])							\
		{	blit_func(sx & 0x3ff, sy & 0x3ff, code, attr);	}		\
	}else															\
	{unsigned char nx,ny;											\
		nx = ((attr>> 8)&0xf);										\
		ny = ((attr>>12)&0xf);										\
		{unsigned char  y;											\
			for (y = 0; y <= ny; y++)								\
			{unsigned char  x;										\
				for (x = 0; x <= nx; x++)							\
				{int ncode;											\
				ncode = (code & ~0xf) + ((code + x) & 0xf) + (y << 4);		\
				ncode &= 0x3ffff;											\
					if (cps_pen_usage[TILE16][ncode])						\
					{														\
						blit_func(											\
							(sx+(((attr & 0x20)?(nx-x):(x)) << 4)) & 0x3ff,	\
							(sy+(((attr & 0x40)?(ny-y):(y)) << 4)) & 0x3ff,	\
						ncode, attr);										\
					}	\
				}		\
			}			\
		}				\
	}

static unsigned char screen_xoffs;
static unsigned char screen_yoffs;
/*------------------------------------------------------
	通常描画
------------------------------------------------------*/

static void cps2_render_sprites(int start_pri, int end_pri)
{
	int i;//, /*x, y,*/ /*sx, sy,*/ code, attr;
//	int /*nx, ny,*/ /*nsx, nsy,*/ ncode;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;
//	int screen_xoffs = 64 - cps2_port(CPS2_OBJ_XOFFS);
//	int screen_yoffs = 16 - cps2_port(CPS2_OBJ_YOFFS);

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri < start_pri || *pri > end_pri) continue;
	{int sx,sy,code, attr;
		sx   = base[i + 0];
		sy   = base[i + 1];
		code = base[i + 2] + ((sy & 0x6000) << 3);
		attr = base[i + 3];
		SCAN_SPRITE(blit_draw_object)
	}
	}
	blit_finish_object();
}


/*------------------------------------------------------
	マスク処理描画
------------------------------------------------------*/

static void cps2_render_masked_sprites(int obj_pri, int mask_pri, int mask_flag)
{
	int i, /*x, y,*/ sx, sy, code, attr;
//	int /*nx, ny,*/ /*nsx, nsy,*/ ncode;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;
//	int screen_xoffs = 64 - cps2_port(CPS2_OBJ_XOFFS);
//	int screen_yoffs = 16 - cps2_port(CPS2_OBJ_YOFFS);

	blit_masked_object_start(mask_flag);

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != obj_pri) continue;

		sy   = base[i + 1];
		code = base[i + 2] + ((sy & 0x6000) << 3);

		if ((mask_flag & MASK_CHECK_OBJCODE) && !(cps_pen_usage[TILE16][code] & SPR_MASKED_OBJ))
			continue;

		sx   = base[i + 0];
		attr = base[i + 3];

		cps2_obj_priority[i >> 2] = 8;

		SCAN_SPRITE(blit_masked_object)
	}

	pri = cps2_obj_priority;

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != mask_pri) continue;

		sy   = base[i + 1];
		code = base[i + 2] + ((sy & 0x6000) << 3);

		if (!(cps_pen_usage[TILE16][code] & (SPR_MASK1 | SPR_MASK2 | SPR_MASK3)))
			continue;

		sx   = base[i + 0];
		attr = base[i + 3];

		SCAN_SPRITE(blit_mask)
	}

	blit_masked_object_finish();
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

void cps2_scan_sprites_callback(void)
{
	int i;//, /*x, y,*/ /*sx, sy,*/ code, attr;
//	int /*nx, ny,*/ /*nsx, nsy,*/ ncode;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;
//	int screen_xoffs = 64 - cps2_port(CPS2_OBJ_XOFFS);
//	int screen_yoffs = 16 - cps2_port(CPS2_OBJ_YOFFS);

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri == 0xff) continue;
	{int sx,sy,code, attr;
		sx   = base[i + 0];
		sy   = base[i + 1];
		code = base[i + 2] + ((sy & 0x6000) << 3);
		attr = base[i + 3];

		SCAN_SPRITE(blit_update_object)
	}
	}
}


/******************************************************************************
  Scroll 1 (8x8 layer)
******************************************************************************/

/*------------------------------------------------------
	スプライトのオフセット取得
------------------------------------------------------*/

INLINE u32 scroll1_offset(u32 col, u32 row)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((col & 0x3f) << 5) + ((row & 0x20) << 6);
}

#define SCAN_SCROLL1(blit_func)												\
	int x, y, sx, sy, offs, code, attr;										\
	int logical_col = cps_scroll1x >> 3;									\
	int logical_row = cps_scroll1y >> 3;									\
	int scroll_col  = cps_scroll1x & 0x07;									\
	int scroll_row  = cps_scroll1y & 0x07;									\
	int min_x, max_x, min_y, max_y;											\
																			\
	min_x = ( 64 + scroll_col) >> 3;										\
	max_x = (448 + scroll_col) >> 3;										\
																			\
	min_y = cps2_scanline_start & ~0x07;									\
	min_y = (min_y + scroll_row) >> 3;										\
	max_y = cps2_scanline_end;												\
	max_y = (max_y + scroll_row) >> 3;										\
																			\
	for (y = min_y; y <= max_y; y++)										\
	{																		\
		sy = (y << 3) - scroll_row;											\
																			\
		for (x = min_x; x <= max_x; x++)									\
		{																	\
			offs = scroll1_offset(logical_col + x, logical_row + y) << 1;	\
			code = 0x20000 + cps_scroll1[offs];								\
																			\
			if (cps_pen_usage[TILE08][code])								\
			{																\
				attr = cps_scroll1[offs + 1];								\
				sx = (x << 3) - scroll_col;									\
				blit_func(sx, sy, code, attr);								\
			}																\
		}																	\
	}


/*------------------------------------------------------
	描画
------------------------------------------------------*/

static void cps2_render_scroll1(void)
{
	SCAN_SCROLL1(blit_draw_scroll1)

	blit_finish_scroll1();
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

void cps2_scan_scroll1_callback(void)
{
	if (cps_layer_enabled[LAYER_SCROLL1])
	{
		SCAN_SCROLL1(blit_update_scroll1)
	}
}


/******************************************************************************
  Scroll 2 (16x16 layer)
******************************************************************************/

/*------------------------------------------------------
	スプライトのオフセット取得
------------------------------------------------------*/

INLINE u32 scroll2_offset(u32 col, u32 row)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x30) << 6);
}

/********************************************************
	ラスタオペレーションなし
********************************************************/

#define SCAN_SCROLL2(blit_func)												\
	int x, y, sx, sy, offs, code, attr;										\
	int logical_col = cps_scroll2x >> 4;									\
	int logical_row = cps_scroll2y >> 4;									\
	int scroll_col  = cps_scroll2x & 0x0f;									\
	int scroll_row  = cps_scroll2y & 0x0f;									\
	int min_x, max_x, min_y, max_y;											\
																			\
	min_x = ( 64 + scroll_col) >> 4;										\
	max_x = (448 + scroll_col) >> 4;										\
																			\
	min_y = cps2_scanline_start & ~0x0f;									\
	min_y = (min_y + scroll_row) >> 4;										\
	max_y = cps2_scanline_end;												\
	max_y = (max_y + scroll_row) >> 4;										\
																			\
	for (y = min_y; y <= max_y; y++)										\
	{																		\
		sy = (y << 4) - scroll_row;											\
																			\
		for (x = min_x; x <= max_x; x++)									\
		{																	\
			offs = scroll2_offset(logical_col + x, logical_row + y) << 1;	\
			code = 0x10000 + cps_scroll2[offs];								\
																			\
			if (cps_pen_usage[TILE16][code])								\
			{																\
				attr = cps_scroll2[offs + 1];								\
				sx = (x << 4) - scroll_col;									\
				blit_func(sx, sy, code, attr);								\
			}																\
		}																	\
	}


/*------------------------------------------------------
	通常描画
------------------------------------------------------*/

static void cps2_render_scroll2(void)
{
	SCAN_SCROLL2(blit_draw_scroll2)

	blit_finish_scroll2(cps2_scanline_start, cps2_scanline_end + 1);
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

static void cps2_scan_scroll2(void)
{
	SCAN_SCROLL2(blit_update_scroll2)
}


/********************************************************
	ラスタオペレーション(line scroll)あり
********************************************************/

#define SCAN_SCROLL2_DISTORT(blit_func)											\
	int x, y, sx, sy, offs, code, attr;											\
	int logical_col, scroll_col;												\
	int logical_row = cps_scroll2y >> 4;										\
	int scroll_row  = cps_scroll2y & 0x0f;										\
	int min_x, max_x, min_y, max_y;												\
	int line, scroll2x[256];													\
																				\
	for (line = cps2_scanline_start; line <= cps2_scanline_end; line++)			\
		scroll2x[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];	\
																				\
	for (line = cps2_scanline_start; line <= cps2_scanline_end; )				\
	{																			\
		int start_line = line;													\
		int end_line = line + 1;												\
																				\
		while (scroll2x[end_line] == scroll2x[start_line])						\
		{																		\
			end_line++;															\
			if (end_line == cps2_scanline_end + 1) break;						\
		}																		\
																				\
		logical_col = scroll2x[start_line] >> 4;								\
		scroll_col = scroll2x[start_line] & 0x0f;								\
																				\
		min_x = ( 64 + scroll_col) >> 4;										\
		max_x = (448 + scroll_col) >> 4;										\
																				\
		start_line &= ~0x0f;													\
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;						\
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;							\
																				\
		for (y = min_y; y <= max_y; y++)										\
		{																		\
			sy = (y << 4) - scroll_row;											\
																				\
			for (x = min_x; x <= max_x; x++)									\
			{																	\
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;	\
				code = 0x10000 + cps_scroll2[offs];								\
																				\
				if (cps_pen_usage[TILE16][code])								\
				{																\
					attr = cps_scroll2[offs + 1];								\
					sx = (x << 4) - scroll_col;									\
					blit_func(sx, sy, code, attr);								\
				}																\
			}																	\
		}																		\
		BLIT_FINISH_FUNC														\
		line = end_line;														\
	}


/*------------------------------------------------------
	ラインスクロール描画
------------------------------------------------------*/

static void cps2_render_scroll2_distort(void)
{
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);

	if (cps_raster_enable)
	{
#define BLIT_FINISH_FUNC	blit_finish_scroll2(line, end_line);
		SCAN_SCROLL2_DISTORT(blit_draw_scroll2)
#undef BLIT_FINISH_FUNC
	}
	else
	{
		(*cps2_render_scroll2_raster_disabled)(otheroffs);
	}
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

static void cps2_scan_scroll2_distort(void)
{
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);

	if (cps_raster_enable)
	{
#define BLIT_FINISH_FUNC
		SCAN_SCROLL2_DISTORT(blit_update_scroll2)
#undef BLIT_FINISH_FUNC
	}
	else
	{
		cps2_scan_scroll2();
	}
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

void cps2_scan_scroll2_callback(void)
{
	if (cps_layer_enabled[LAYER_SCROLL2])
	{
		int video_ctrl = cps1_port(CPS1_VIDEO_CONTROL);

		if (video_ctrl & 0x0001)
			cps2_scan_scroll2_distort();
		else
			cps2_scan_scroll2();
	}
}


/********************************************************
	Scroll 2 その他の処理
********************************************************/

/*------------------------------------------------------
	Raster Effects Off時の描画
------------------------------------------------------*/

static void cps2_render_scroll2_raster_disabled1(int otheroffs)
{
	int line;

	if (cps2_scanline_start == FIRST_VISIBLE_LINE)
		line = ((cps2_scanline_end + 1) - cps2_scanline_start) >> 1;
	else
		line = 256/2;

	cps_scroll2x += cps_other[(line + otheroffs) & 0x3ff];
	cps2_render_scroll2();
}


/*------------------------------------------------------
	Raster Effects Off時の描画 (格闘ゲーム用)
------------------------------------------------------*/

static void cps2_render_scroll2_raster_disabled2(int otheroffs)
{
	int line, first_scroll2x, scroll2x[256];

	line = cps2_scanline_start;
	first_scroll2x = scroll2x[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];
	line++;

	for (; line <= cps2_scanline_end; line++)
	{
		scroll2x[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];
		if (scroll2x[line] != first_scroll2x)
			break;
	}

	if (line != cps2_scanline_end && line - cps2_scanline_start > 16)
	{
		// 途中まで同じ場合は、2回に分けて描画
		int save_start, save_end;

		save_start = cps2_scanline_start;
		save_end = cps2_scanline_end;

		cps_scroll2x = first_scroll2x;
		cps2_scanline_end = line - 1;
		cps2_render_scroll2();

		cps2_scanline_start = line;
		cps2_scanline_end = save_end;

		cps_scroll2x = scroll2x[line];
		cps2_render_scroll2();

		cps2_scanline_start = save_start;
	}
	else
	{
		// 全て同じか、ライン毎に変化がある場合
		cps2_render_scroll2_raster_disabled1(otheroffs);
	}
}


/******************************************************************************
  Scroll 3 (32x32 layer)
******************************************************************************/

/*------------------------------------------------------
	スプライトのオフセット取得
------------------------------------------------------*/

INLINE u32 scroll3_offset(u32 col, u32 row)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x07) + ((col & 0x3f) << 3) + ((row & 0x38) << 6);
}

#define SCAN_SCROLL3(blit_func)					\
	int x, y, sx, sy, offs, code, attr;			\
/*	int base = cps2_scroll3_base;*/				\
	int logical_col = cps_scroll3x >> 5;		\
	int logical_row = cps_scroll3y >> 5;		\
	int scroll_col  = cps_scroll3x & 0x1f;		\
	int scroll_row  = cps_scroll3y & 0x1f;		\
	int min_x, max_x, min_y, max_y;				\
												\
	min_x = ( 64 + scroll_col) >> 5;			\
	max_x = (448 + scroll_col) >> 5;			\
												\
	min_y = cps2_scanline_start & ~0x1f;		\
	min_y = (min_y + scroll_row) >> 5;			\
	max_y = cps2_scanline_end;					\
	max_y = (max_y + scroll_row) >> 5;			\
	if(cps2_kludge){							\
		for (y = min_y; y <= max_y; y++)				\
		{												\
			sy = (y << 5) - scroll_row;					\
														\
			for (x = min_x; x <= max_x; x++)			\
			{											\
				offs = scroll3_offset(logical_col + x, logical_row + y) << 1;	\
				code = cps_scroll3[offs];				\
														\
				switch (cps2_kludge)					\
				{										\
				case CPS2_KLUDGE_SSF2T:					\
					if (code < 0x5600) code += 0x4000;	\
					break;								\
														\
				case CPS2_KLUDGE_XMCOTA:				\
					if (code >= 0x5800) code -= 0x4000;	\
					break;								\
														\
				case CPS2_KLUDGE_DIMAHOO:				\
					code &= 0x3fff;						\
					break;								\
				}										\
				code += cps2_scroll3_base;				\
														\
				if (cps_pen_usage[TILE32][code])		\
				{										\
					attr = cps_scroll3[offs + 1];		\
					sx = (x << 5) - scroll_col;			\
					blit_func(sx, sy, code, attr);		\
				}										\
			}											\
		}												\
	}else{												\
		for (y = min_y; y <= max_y; y++)				\
		{												\
			sy = (y << 5) - scroll_row;					\
														\
			for (x = min_x; x <= max_x; x++)			\
			{											\
				offs = scroll3_offset(logical_col + x, logical_row + y) << 1;	\
				code = cps_scroll3[offs];				\
				code += cps2_scroll3_base;				\
														\
				if (cps_pen_usage[TILE32][code])		\
				{										\
					attr = cps_scroll3[offs + 1];		\
					sx = (x << 5) - scroll_col;			\
					blit_func(sx, sy, code, attr);		\
				}										\
			}											\
		}												\
	}
/*------------------------------------------------------
	描画
------------------------------------------------------*/
static void cps2_render_scroll3(void)
{
	SCAN_SCROLL3(blit_draw_scroll3)
	blit_finish_scroll3();
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/
void cps2_scan_scroll3_callback(void)
{
	if (cps_layer_enabled[LAYER_SCROLL3])
	{
		SCAN_SCROLL3(blit_update_scroll3)
	}
}


/******************************************************************************
	画面更新処理
******************************************************************************/

/*------------------------------------------------------
	レイヤーを描画
------------------------------------------------------*/

static void cps2_render_layer(int layer, int distort)
{
	if (cps_layer_enabled[layer])
	{
		switch (layer)
		{
		case 1:
			cps2_render_scroll1();
			break;

		case 2:
			if (distort)
				cps2_render_scroll2_distort();
			else
				cps2_render_scroll2();
			break;

		case 3:
			cps2_render_scroll3();
			break;
		}
	}
}


/*------------------------------------------------------
	マスク処理チェック(msh)
------------------------------------------------------*/

static void cps2_check_mask_msh(int obj_pri, int mask_pri)
{
	int i, code, found = 0;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != mask_pri) continue;

		code = base[i + 2] + ((base[i + 1] & 0x6000) << 3);

		if (cps_pen_usage[TILE16][code] & SPR_MASK1)
		{
			found = 1;
			break;
		}
	}
	if (!found) return;

	pri = cps2_obj_priority;
	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri == obj_pri)
			*pri = 6;
		else if (*pri == mask_pri)
			*pri = 7;
	}
}


/*------------------------------------------------------
	マスク処理チェック(sfa3)
------------------------------------------------------*/

static void cps2_check_mask_sfa3(int obj_pri)
{
	int i, code, count = 0;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != obj_pri) continue;

		code = base[i + 2] + ((base[i + 1] & 0x6000) << 3);

		if (!(cps_pen_usage[TILE16][code] & SPR_MASKED_OBJ))
			continue;

		count++;
	}

	if (count == 24) cps2_mask_number = 0;
}


/*------------------------------------------------------
	マスク処理チェック(gigawing / オープニングデモ)
------------------------------------------------------*/

static void cps2_check_mask_gigawing(int obj_pri, int mask_pri)
{
	int i, code, found;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;

	// object(Betting...の文字)検索
	found = 0;
	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != obj_pri) continue;

		code = base[i + 2] + ((base[i + 1] & 0x6000) << 3);

		if (cps_pen_usage[TILE16][code] & SPR_MASKED_OBJ)
		{
			found = 1;
			break;
		}
	}
	if (!found) return;

	// mask(メダリオン)検索
	pri = cps2_obj_priority;
	found = 0;
	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != mask_pri) continue;

		code = base[i + 2] + ((base[i + 1] & 0x6000) << 3);

		if (cps_pen_usage[TILE16][code] & SPR_MASK3)
		{
			found = 1;
			break;
		}
	}
	if (!found) return;

	// プライオリティを入れ替える
	pri = cps2_obj_priority;
	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri == obj_pri)
			*pri = mask_pri;
		else if (*pri == mask_pri)
			*pri = obj_pri;
	}
}


/*------------------------------------------------------
	マスク処理チェック(共通)
------------------------------------------------------*/

static void cps2_check_mask(int obj_pri, int mask_pri, int mask_flag, int mask_number)
{
	int i, code, mask;
	volatile u16 *base = cps2_buffered_obj;
	u8 *pri = cps2_obj_priority;

	if (mask_flag & MASK_MSH)
	{
		cps2_check_mask_msh(obj_pri, mask_pri);
		return;
	}
	else if (mask_flag & MASK_SFA3)
	{
		cps2_check_mask_sfa3(obj_pri);
		return;
	}
	else if (mask_flag & MASK_GIGAWING)
	{
		cps2_check_mask_gigawing(obj_pri, mask_pri);
		return;
	}

	if (mask_flag & MASK_CHECK_OBJEXIST)
	{
		int found = 0;

		for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
		{
			if (*pri != obj_pri) continue;

			code = base[i + 2] + ((base[i + 1] & 0x6000) << 3);

			if (cps_pen_usage[TILE16][code] & SPR_MASKED_OBJ)
			{
				found = 1;
				break;
			}
		}
		if (!found) return;
	}

	mask = 1 << mask_number;

	for (i = 0; i < cps2_last_sprite_offset; i += 4, pri++)
	{
		if (*pri != mask_pri) continue;

		code = base[i + 2] + ((base[i + 1] & 0x6000) << 3);

		if (cps_pen_usage[TILE16][code] & mask)
		{
			cps2_mask_number = mask_number;
			return;
		}
	}
}


/*------------------------------------------------------
	画面更新
------------------------------------------------------*/

void cps2_screenrefresh(int start, int end)
{
	int layer_ctrl = cps1_port(CPS2_LAYER_CONTROL);
	int video_ctrl = cps1_port(CPS1_VIDEO_CONTROL);
	int i, distort_scroll2, highest_prio, prev_prio, curr_prio;
	int layer[4], prio[4] = {0,};


	screen_xoffs = (64 - cps2_port(CPS2_OBJ_XOFFS));
	screen_yoffs = (16 - cps2_port(CPS2_OBJ_YOFFS));

	cps2_scanline_start = start;
	cps2_scanline_end   = end;

	cps_flip_screen = video_ctrl & 0x8000;
	distort_scroll2 = video_ctrl & 0x0001;

	cps1_get_video_base();
	cps2_find_last_sprite();

	blit_partial_start(start, end);

	layer[3] = (layer_ctrl >> 12) & 3;
	layer[2] = (layer_ctrl >> 10) & 3;
	layer[1] = (layer_ctrl >>  8) & 3;
	layer[0] = (layer_ctrl >>  6) & 3;

	cps2_mask_number = -1;
	cps2_pri_ctrl = cps2_port(CPS2_OBJ_PRI);
	prio[3] = (cps2_pri_ctrl >> 12) & 7;
	prio[2] = (cps2_pri_ctrl >>  8) & 7;
	prio[1] = (cps2_pri_ctrl >>  4) & 7;

	highest_prio = 8;
	for (i = 3; i >= 0; i--)
	{
		if (layer[i] > 0)
		{
			if (prio[layer[i]] > highest_prio)
				prio[layer[i]] = highest_prio;
			else
				highest_prio = prio[layer[i]];
		}
	}

	if (cps2_mask_max)
	{
		for (i = 0; i < cps2_mask_max; i++)
		{
			cps2_check_mask(cps2_obj_pri[i], cps2_mask_pri[i], cps2_mask_flag[i], i);
			if (cps2_mask_number >= 0) break;
		}
	}

	if (cps2_mask_number < 0)
	{
		prev_prio = -1;
		for (curr_prio = 0; curr_prio < 8; curr_prio++)
		{
			for (i = 0; i < LAYER_MAX; i++)
			{
				if (prio[layer[i]] == curr_prio)
				{
					if (prev_prio < curr_prio)
					{
						cps2_render_sprites(prev_prio + 1, curr_prio);
						prev_prio = curr_prio;
					}
					cps2_render_layer(layer[i], distort_scroll2);
				}
			}
		}
		if (prev_prio < 7)
		{
			cps2_render_sprites(prev_prio + 1, 7);
		}
	}
	else
	{
		int obj_pri, mask_pri, flag;

		obj_pri  = cps2_obj_pri[cps2_mask_number];
		mask_pri = cps2_mask_pri[cps2_mask_number];
		flag     = cps2_mask_flag[cps2_mask_number];

		if (flag & MASK_SFA2)
		{
			prev_prio = mask_pri;
			curr_prio = mask_pri + 1;
		}
		else
		{
			prev_prio = -1;
			curr_prio = 0;
		}

		for (; curr_prio < obj_pri; curr_prio++)
		{
			for (i = 0; i < LAYER_MAX; i++)
			{
				if (prio[layer[i]] == curr_prio)
				{
					if (prev_prio < curr_prio)
					{
						cps2_render_sprites(prev_prio + 1, curr_prio);
						prev_prio = curr_prio;
					}
					cps2_render_layer(layer[i], distort_scroll2);
				}
			}
		}

		cps2_render_masked_sprites(obj_pri, mask_pri, flag);

		for (; curr_prio < 8; curr_prio++)
		{
			for (i = 0; i < LAYER_MAX; i++)
			{
				if (prio[layer[i]] == curr_prio)
				{
					if (prev_prio < curr_prio)
					{
						cps2_render_sprites(prev_prio + 1, curr_prio);
						prev_prio = curr_prio;
					}
					cps2_render_layer(layer[i], distort_scroll2);
				}
			}
		}
		if (prev_prio < 7)
		{
			cps2_render_sprites(prev_prio + 1, 7);
		}
	}
}


/*------------------------------------------------------
	object RAM更新
------------------------------------------------------*/

void cps2_objram_latch(void)
{
	memcpy(cps2_buffered_obj, cps2_objram[cps2_objram_bank], cps2_obj_size);
	if (!cps2_build_palette) cps2_build_palette_delay();
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( video )
{
	state_save_long(&cps2_objram_bank, 1);
	state_save_word(cps2_buffered_obj, 0x1000);
	state_save_word(cps2_old_palette, 2048);
	state_save_word(video_palette, 2048);
	state_save_word(cps2_buffered_palette, 512);
}

STATE_LOAD( video )
{
	state_load_long(&cps2_objram_bank, 1);
	state_load_word(cps2_buffered_obj, 0x1000);
	state_load_word(cps2_old_palette, 2048);
	state_load_word(video_palette, 2048);
	state_load_word(cps2_buffered_palette, 512);
}

#endif /* SAVE_STATE */
