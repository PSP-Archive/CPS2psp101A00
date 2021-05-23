/******************************************************************************

	cache.c

	CPS2メモリキャッシュインタフェース関数

******************************************************************************/

#ifndef CPS2_CACHE_H
#define CPS2_CACHE_H

#define MAX_CACHE_BLOCKS	0x200

extern u32 (*read_cache)(u32 offset);
extern void (*update_cache)(u32 offset);
extern u8 block_empty[MAX_CACHE_BLOCKS];

void cache_init(void);
int cache_start(void);
void cache_shutdown(void);
void cache_sleep(int flag);

void cahce_set_update_func(int flag);

#endif /* CPS2_CACHE_H */
