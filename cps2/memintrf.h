/******************************************************************************

	memintrf.c

	Memory interace

******************************************************************************/

#ifndef CPS2_MEMORY_INTERFACE_H
#define CPS2_MEMORY_INTERFACE_H

extern u8 *memory_region_cpu1;
extern u8 *memory_region_cpu2;
extern u8 *memory_region_gfx1;
extern u8 *memory_region_sound1;
extern u8 *memory_region_user1;

extern u32 memory_length_cpu1;
extern u32 memory_length_cpu2;
extern u32 memory_length_gfx1;
extern u32 memory_length_sound1;
extern u32 memory_length_user1;

extern u32 gfx_total_elements[3];
extern u8 *gfx_pen_usage[3];

extern u8 cps1_ram[0x10000];
extern u8 cps2_ram[0x4000 + 2];
extern u16 cps1_gfxram[0x30000 >> 1];
extern u16 cps1_output[0x100 >> 1];

extern int cps2_objram_bank;
extern u16 cps2_objram[2][0x2000 >> 1];
extern u16 cps2_output[0x10 >> 1];

extern u8 *qsound_sharedram1;
extern u8 *qsound_sharedram2;

extern int cps_machine_type;
extern int cps_input_type;
extern int cps_init_type;
extern int cps_screen_type;

int  file_open(const char *fname1, const char *fname2, const u32 crc, char *fname);
void file_close(void);
int  file_read(void *buf, size_t length);
int  file_getc(void);

int memory_init(void);
void memory_shutdown(void);

u8 m68000_read_memory_8(u32 offset);
u16 m68000_read_memory_16(u32 offset);
//u32 m68000_read_memory_32(u32 offset);
void m68000_write_memory_8(u32 offset, u8 data);
void m68000_write_memory_16(u32 offset, u16 data);
//void m68000_write_memory_32(u32 offset, u32 data);

u8 z80_read_memory_8(u16 offset);
void z80_write_memory_8(u16 offset, u8 data);

#ifndef RELEASE
void cheat_callback(void);
#endif

#ifdef SAVE_STATE
STATE_SAVE( memory );
STATE_LOAD( memory );
#endif

#endif /* CPS2_MEMORY_INTERFACE_H */
