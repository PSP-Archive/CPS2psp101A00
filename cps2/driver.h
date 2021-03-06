#ifndef CPS2_DRIVER_H
#define CPS2_DRIVER_H

#define RASTER_LINES	262

/* CPS2 kludge value */
#define CPS2_KLUDGE_NONE		0x00
#define CPS2_KLUDGE_SSF2		0x01
#define CPS2_KLUDGE_SSF2T		0x02
#define CPS2_KLUDGE_XMCOTA		0x04
#define CPS2_KLUDGE_MMATRIX		0x08
#define CPS2_KLUDGE_DIMAHOO		0x10
#define CPS2_KLUDGE_PUZLOOP2	0x20

enum
{
	MACHINE_cps2 = 0,
	MACHINE_MAX
};

enum
{
	INPTYPE_cps2 = 0,
	INPTYPE_ssf2,
	INPTYPE_ddtod,
	INPTYPE_sgemf,
	INPTYPE_avsp,
	INPTYPE_cybots,
	INPTYPE_19xx,
	INPTYPE_qndream,
	INPTYPE_batcir,
	INPTYPE_puzloop2,
	INPTYPE_daimahoo,
	INPTYPE_MAX
};

enum
{
	INIT_cps2 = 0,
	INIT_puzloop2,
	INIT_MAX
};

enum
{
	SCREEN_NORMAL = 0,
	SCREEN_VERTICAL,
	SCREENTYPE_MAX
};

struct spr_mask_t
{
	u32 flag;
	u32 mask_priority;
	u32 obj_priority;
};

struct driver_t
{
	const char *name;
	u32 cache_size;
	u16 object_tex_height;
	u16 scroll1_tex_height;
	u16 scroll2_tex_height;
	u16 scroll3_tex_height;
	u16 kludge;
	u16 flags;
	struct spr_mask_t mask[3];
	u8 inp_eeprom;
	u8 inp_eeprom_value[16];
};

extern struct driver_t CPS2_driver[];
extern struct driver_t *driver;

void cps2_driver_init(void);
void cps2_driver_reset(void);

void cps2_raster_interrupt(int scanline, int which);
void cps2_vblank_interrupt(void);

READ16_VOID_HANDLER( cps2_inputport0_r );
READ16_VOID_HANDLER( cps2_inputport1_r );


READ16_HANDLER( qsound_sharedram1_r );
WRITE16_HANDLER( qsound_sharedram1_w );
READ16_HANDLER( qsound_sharedram2_r );
WRITE16_HANDLER( qsound_sharedram2_w );
WRITE8_HANDLER( qsound_banksw_w );

READ16_VOID_HANDLER( cps2_qsound_volume_r );
WRITE16_HANDLER( cps2_qsound_sharedram_w );

READ16_VOID_HANDLER( cps2_eeprom_port_r );
WRITE16_HANDLER( cps2_eeprom_port_w );

#ifdef SAVE_STATE
STATE_SAVE( driver );
STATE_LOAD( driver );
#endif

#endif /* CPS2_DRIVER_H */
