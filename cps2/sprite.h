/******************************************************************************

	sprite.c

	CPS2 スプライトマネージャ

******************************************************************************/

#ifndef CPS2_SPRITE_H
#define CPS2_SPRITE_H

#define MAKE_KEY(code, palno)		(code | (palno << 24))

/* マスク処理用フラグ */
#define MASK_COPYRECT_BIT			0x03
#define MASK_COPYRECT_FULLSCREEN	0x00
#define MASK_COPYRECT_MINIMUM		0x01
#define MASK_COPYRECT_MAXIMUM		0x02
#define MASK_CLEAR_SCREEN			0x04
#define MASK_CHECK_OBJCODE			0x08
#define MASK_CHECK_OBJEXIST			0x10
#define MASK_GIGAWING				0x20
#define MASK_SFA2					0x40
#define MASK_SFA3					0x80
#define MASK_MSH					0x100

void blit_palette_mark_dirty(int palno);
void blit_clear_all_cache(void);

void blit_reset(void);
void blit_partial_start(int start, int end);
void blit_finish(void);

void blit_update_object(int x, int y, u32 code, u32 attr);
void blit_draw_object(int x, int y, u32 tileno, u32 tileatr);
void blit_finish_object(void);

void blit_update_scroll1(int x, int y, u32 tileno, u32 tileatr);
void blit_draw_scroll1(int x, int y, u32 tileno, u32 tileatr);
void blit_finish_scroll1(void);

void blit_update_scroll2(int x, int y, u32 tileno, u32 tileatr);
void blit_draw_scroll2(int x, int y, u32 tileno, u32 tileatr);
void blit_finish_scroll2(int min_y, int max_y);

void blit_update_scroll3(int x, int y, u32 tileno, u32 tileatr);
void blit_draw_scroll3(int x, int y, u32 tileno, u32 tileatr);
void blit_finish_scroll3(void);

void blit_update_all_cache(void);

void blit_masked_object_start(int flag);
void blit_masked_object_finish(void);

void blit_masked_object(int x, int y, u32 code, u32 attr);
void blit_mask(int x, int y, u32 code, u32 attr);

#endif /* CPS2_SPRITE_H */
