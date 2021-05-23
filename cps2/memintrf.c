/******************************************************************************

	memintrf.c

	CPS2メモリインタフェース関数

******************************************************************************/

#include "cps2.h"


#define M68K_AMASK 0x00ffffff
//#define Z80_AMASK 0x0000ffff

#define READ_BYTE(mem, offset)			mem[offset ^ 1]
#define READ_WORD(mem, offset)			*(u16 *)&mem[offset]
#define WRITE_BYTE(mem, offset, data)	mem[offset ^ 1] = data
#define WRITE_WORD(mem, offset, data)	*(u16 *)&mem[offset] = data

#define str_cmp(s1, s2)		strnicmp(s1, s2, strlen(s2))

enum
{
	REGION_CPU1 = 0,
	REGION_CPU2,
	REGION_GFX1,
	REGION_SOUND1,
	REGION_USER1,
	REGION_SKIP
};

enum
{
	ROM_LOAD = 0,
	ROM_CONTINUE,
	ROM_WORDSWAP,
	MAP_MAX
};

#define MAX_CPU1ROM		8
#define MAX_CPU2ROM		8
#define MAX_SND1ROM		8
#define MAX_USR1ROM		8


/******************************************************************************
	グローバル構造体/変数
******************************************************************************/

u8 *memory_region_cpu1;
u8 *memory_region_cpu2;
u8 *memory_region_gfx1;
u8 *memory_region_sound1;
u8 *memory_region_user1;

u32 memory_length_cpu1;
u32 memory_length_cpu2;
u32 memory_length_gfx1;
u32 memory_length_sound1;
u32 memory_length_user1;

u32 gfx_total_elements[3];
u8 *gfx_pen_usage[3];

u8  cps1_ram[0x10000];
u8  cps2_ram[0x4000 + 2];
u16 cps1_gfxram[0x30000 >> 1];
u16 cps1_output[0x100 >> 1];

u16 cps2_objram[2][0x2000 >> 1];
u16 cps2_output[0x10 >> 1];

u8 *qsound_sharedram1;
u8 *qsound_sharedram2;

int cps_machine_type;
int cps_input_type;
int cps_init_type;
int cps_screen_type;


/******************************************************************************
	ローカル構造体/変数
******************************************************************************/

struct rom_t
{
	u32 type;
	u32 offset;
	u32 length;
	u32 crc;
	int group;
	int skip;
};

static int rom_f = -1;

static struct rom_t cpu1rom[MAX_CPU1ROM];
static struct rom_t cpu2rom[MAX_CPU2ROM];
static struct rom_t snd1rom[MAX_SND1ROM];
static struct rom_t usr1rom[MAX_USR1ROM];

static int num_cpu1rom;
static int num_cpu2rom;
static int num_snd1rom;
static int num_usr1rom;

static u8 *static_ram1;
static u8 *static_ram2;
static u8 *static_ram3;
static u8 *static_ram4;
static u8 *static_ram5[2];


/******************************************************************************
	エラーメッセージ表示
******************************************************************************/

/*------------------------------------------------------
	メモリ確保エラーメッセージ表示
------------------------------------------------------*/

static void error_memory(const char *mem_name)
{
	zip_close();
	msg_printf("ERROR: Could not allocate %s memory.\n", mem_name);
	msg_printf("Press any button.\n");
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	ファイルオープンエラーメッセージ表示
------------------------------------------------------*/

static void error_file(const char *file_name)
{
	zip_close();
	msg_printf("ERROR: Could not open file. \"%s\"\n", file_name);
	msg_printf("Press any button.\n");
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	ROMファイルエラーメッセージ表示
------------------------------------------------------*/

static void error_rom(const char *rom_name)
{
	zip_close();
	msg_printf("ERROR: File not found or CRC32 not correct. \"%s\"\n", rom_name);
	msg_printf("Press any button.\n");
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/******************************************************************************
	ファイルI/O
******************************************************************************/

/*--------------------------------------------------------
	ZIPファイルからファイルを検索し、開く
--------------------------------------------------------*/

int file_open(const char *fname1, const char *fname2, const u32 crc, char *fname)
{
	int found = 0;
	struct zip_find_t file;
	char path[MAX_PATH];

	file_close();

	sprintf(path, "%s/%s.zip", game_dir, fname1);

	if (zip_open(path) != -1)
	{
		if (zip_findfirst(&file))
		{
			if (file.crc32 == crc)
			{
				found = 1;
			}
			else
			{
				while (zip_findnext(&file))
				{
					if (file.crc32 == crc)
					{
						found = 1;
						break;
					}
				}
			}
		}
		if (!found) zip_close();
	}

	if (!found && fname2 != NULL)
	{
		sprintf(path, "%s/%s.zip", game_dir, fname2);

		if (zip_open(path) != -1)
		{
			if (zip_findfirst(&file))
			{
				if (file.crc32 == crc)
				{
					found = 2;
				}
				else
				{
					while (zip_findnext(&file))
					{
						if (file.crc32 == crc)
						{
							found = 2;
							break;
						}
					}
				}
			}
			if (!found) zip_close();
		}
	}

	if (found)
	{
		if (fname) strcpy(fname, file.name);
		rom_f = zopen(file.name);
		return rom_f;
	}

	return -1;
}


/*--------------------------------------------------------
	ファイルを閉じる
--------------------------------------------------------*/

void file_close(void)
{
	if (rom_f != -1)
	{
		zclose(rom_f);
		zip_close();
		rom_f = -1;
	}
}


/*--------------------------------------------------------
	ファイルから指定バイト読み込む
--------------------------------------------------------*/

int file_read(void *buf, size_t length)
{
	if (rom_f != -1)
		return zread(rom_f, buf, length);
	return -1;
}


/*--------------------------------------------------------
	ファイルから1文字読み込む
--------------------------------------------------------*/

int file_getc(void)
{
	if (rom_f != -1)
		return zgetc(rom_f);
	return -1;
}


/*--------------------------------------------------------
	キャッシュファイルを開く
--------------------------------------------------------*/

int cachefile_open(const char *fname)
{
	char path[MAX_PATH];

	sprintf(path, "%s/%s_cache.zip", cache_dir, game_name);
	if (zip_open(path) != -1)
	{
		if ((rom_f = zopen(fname)) != -1)
			return rom_f;
		zip_close();
	}

	sprintf(path, "%s/%s_cache.zip", cache_dir, cache_parent_name);
	if (zip_open(path) != -1)
	{
		if ((rom_f = zopen(fname)) != -1)
			return rom_f;
		zip_close();
	}

	return -1;
}


/******************************************************************************
	ROM読み込み
******************************************************************************/

/*--------------------------------------------------------
	ROMをロードする
--------------------------------------------------------*/

static int rom_load(struct rom_t *rom, u8 *mem, int f, int idx, int max)
{
	int offset, length;

_continue:
	offset = rom[idx].offset;

	if (rom[idx].skip == 0)
	{
		file_read(&mem[offset], rom[idx].length);

		if (rom[idx].type == ROM_WORDSWAP)
			swab(&mem[offset], &mem[offset], rom[idx].length);
	}
	else
	{
		int c;
		int skip = rom[idx].skip + rom[idx].group;

		length = 0;

		if (rom[idx].group == 1)
		{
			if (rom[idx].type == ROM_WORDSWAP)
				offset ^= 1;

			while (length < rom[idx].length)
			{
				if ((c = file_getc()) == EOF) break;
				mem[offset] = c;
				offset += skip;
				length++;
			}
		}
		else
		{
			while (length < rom[idx].length)
			{
				if ((c = file_getc()) == EOF) break;
				mem[offset + 0] = c;
				if ((c = file_getc()) == EOF) break;
				mem[offset + 1] = c;
				offset += skip;
				length += 2;
			}
		}
	}

	if (++idx != max)
	{
		if (rom[idx].type == ROM_CONTINUE)
		{
			goto _continue;
		}
	}

	return idx;
}


/*--------------------------------------------------------
	CPU1 (M68000 program ROM / encrypted)
--------------------------------------------------------*/

static int load_rom_cpu1(void)
{
	int i, f;
	char fname[32], *parent;

	if ((memory_region_cpu1 = calloc(1, memory_length_cpu1)) == NULL)
	{
		error_memory("REGION_CPU1");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_cpu1rom; )
	{
		if ((f = file_open(game_name, parent, cpu1rom[i].crc, fname)) == -1)
		{
			error_rom("CPU1");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

		i = rom_load(cpu1rom, memory_region_cpu1, f, i, num_cpu1rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	CPU2 (Z80 program ROM)
--------------------------------------------------------*/

static int load_rom_cpu2(void)
{
	int i, f;
	char fname[32], *parent;

	if ((memory_region_cpu2 = calloc(1, memory_length_cpu2)) == NULL)
	{
		error_memory("REGION_CPU2");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_cpu2rom; )
	{
		if ((f = file_open(game_name, parent, cpu2rom[i].crc, fname)) == -1)
		{
			error_rom("CPU2");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

		i = rom_load(cpu2rom, memory_region_cpu2, f, i, num_cpu2rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	GFX1 (graphic ROM)
--------------------------------------------------------*/

static int load_rom_gfx1(void)
{
	int f;

	msg_printf("Loading cache information data...\n");

	gfx_total_elements[TILE08] = (memory_length_gfx1 - 0x800000) >> 6;
	gfx_total_elements[TILE16] = memory_length_gfx1 >> 7;
	gfx_total_elements[TILE32] = (memory_length_gfx1 - 0x800000) >> 9;

	if (gfx_total_elements[TILE08] > 0x10000) gfx_total_elements[TILE08] = 0x10000;
	if (gfx_total_elements[TILE32] > 0x10000) gfx_total_elements[TILE32] = 0x10000;

	if ((gfx_pen_usage[TILE08] = calloc(1, gfx_total_elements[TILE08])) == NULL)
	{
		error_memory("GFX_PEN_USAGE (tile8)");
		return 0;
	}
	if ((gfx_pen_usage[TILE16] = calloc(1, gfx_total_elements[TILE16])) == NULL)
	{
		error_memory("GFX_PEN_USAGE (tile16)");
		return 0;
	}
	if ((gfx_pen_usage[TILE32] = calloc(1, gfx_total_elements[TILE32])) == NULL)
	{
		error_memory("GFX_PEN_USAGE (tile32)");
		return 0;
	}

	if ((f = cachefile_open("tile8_usage")) == -1)
	{
		error_file("tile8_usage (scroll1 pen usage)");
		return 0;
	}
	file_read(gfx_pen_usage[TILE08], gfx_total_elements[TILE08]);
	file_close();

	if ((f = cachefile_open("tile16_usage")) == -1)
	{
		error_file("tile16_usage (scroll2 / object pen usage)");
		return 0;
	}
	file_read(gfx_pen_usage[TILE16], gfx_total_elements[TILE16]);
	file_close();

	if ((f = cachefile_open("tile32_usage")) == -1)
	{
		error_file("tile32_usage (scroll3 pen usage)");
		return 0;
	}
	file_read(gfx_pen_usage[TILE32], gfx_total_elements[TILE32]);
	file_close();

	if ((f = cachefile_open("block_empty")) == -1)
	{
		error_file("block_empty (cache block skip flags)");
		return 0;
	}
	file_read(block_empty, MAX_CACHE_BLOCKS);
	file_close();

	memory_length_gfx1 = driver->cache_size;

	if (cache_start() == 0)
	{
		msg_printf("Press any button.\n");
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	return 1;
}


/*--------------------------------------------------------
	SOUND1 (Q-SOUND PCM ROM)
--------------------------------------------------------*/

static int load_rom_sound1(void)
{
	int i, f;
	char fname[32], *parent;

	if ((memory_region_sound1 = calloc(1, memory_length_sound1)) == NULL)
	{
		error_memory("REGION_SOUND1");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_snd1rom; )
	{
		if ((f = file_open(game_name, parent_name, snd1rom[i].crc, fname)) == -1)
		{
			error_rom("SOUND1");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

		i = rom_load(snd1rom, memory_region_sound1, f, i, num_snd1rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	USER1 (MC68000 ROM xor table)
--------------------------------------------------------*/

static int load_rom_user1(void)
{
	int i, f;
	char fname[32], *parent;

	if ((memory_region_user1 = calloc(1, memory_length_user1)) == NULL)
	{
		error_memory("REGION_USER1");
		return 0;
	}
	memset(memory_region_user1, 0, memory_length_user1);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_usr1rom; )
	{
		if ((f = file_open(game_name, parent, usr1rom[i].crc, fname)) == -1)
		{
			error_rom("USER1");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

		i = rom_load(usr1rom, memory_region_user1, f, i, num_usr1rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	ROM情報をデータベースで解析
--------------------------------------------------------*/

static int load_rom_info(const char *game_name)
{
	FILE *fp;
	char path[MAX_PATH];
	char buf[256];
	int rom_start = 0;
	int region = 0;

	num_cpu1rom = 0;
	num_cpu2rom = 0;
	num_snd1rom = 0;
	num_usr1rom = 0;

	cps_machine_type = 0;
	cps_input_type   = 0;
	cps_init_type    = 0;
	cps_screen_type  = 0;

	sprintf(path, "%srominfo.dat", launchDir);

	if ((fp = fopen(path, "r")) != NULL)
	{
		while (fgets(buf, 255, fp))
		{
			if (buf[0] == '/' && buf[1] == '/')
				continue;

			if (buf[0] != '\t')
			{
				if (buf[0] == '\r' || buf[0] == '\n')
				{
					// 改行
					continue;
				}
				else if (str_cmp(buf, "FILENAME(") == 0)
				{
					char *name, *parent;
					char *machine, *input, *init, *rotate;

					strtok(buf, " ");
					name    = strtok(NULL, " ,");
					parent  = strtok(NULL, " ,");
					machine = strtok(NULL, " ,");
					input   = strtok(NULL, " ,");
					init    = strtok(NULL, " ,");
					rotate  = strtok(NULL, " ");

					if (stricmp(name, game_name) == 0)
					{
						if (str_cmp(parent, "cps2") == 0)
							parent_name[0] = '\0';
						else
							strcpy(parent_name, parent);

						sscanf(machine, "%d", &cps_machine_type);
						sscanf(input, "%d", &cps_input_type);
						sscanf(init, "%d", &cps_init_type);
						sscanf(rotate, "%d", &cps_screen_type);
						rom_start = 1;
					}
				}
				else if (rom_start && str_cmp(buf, "END") == 0)
				{
					fclose(fp);
					return 0;
				}
			}
			else if (rom_start)
			{
				if (str_cmp(&buf[1], "REGION(") == 0)
				{
					char *size, *type, *flag;

					strtok(&buf[1], " ");
					size = strtok(NULL, " ,");
					type = strtok(NULL, " ,");
					flag = strtok(NULL, " ");

					if (strcmp(type, "CPU1") == 0)
					{
						sscanf(size, "%x", &memory_length_cpu1);
						region = REGION_CPU1;
					}
					else if (strcmp(type, "CPU2") == 0)
					{
						sscanf(size, "%x", &memory_length_cpu2);
						region = REGION_CPU2;
					}
					else if (strcmp(type, "GFX1") == 0)
					{
						sscanf(size, "%x", &memory_length_gfx1);
						region = REGION_SKIP;
					}
					else if (strcmp(type, "SOUND1") == 0)
					{
						sscanf(size, "%x", &memory_length_sound1);
						region = REGION_SOUND1;
					}
					else if (strcmp(type, "USER1") == 0)
					{
						sscanf(size, "%x", &memory_length_user1);
						region = REGION_USER1;
					}
					else
					{
						region = REGION_SKIP;
					}
				}
				else if (str_cmp(&buf[1], "ROM(") == 0)
				{
					char *type, *offset, *length, *crc;

					strtok(&buf[1], " ");
					type   = strtok(NULL, " ,");
					offset = strtok(NULL, " ,");
					length = strtok(NULL, " ,");
					crc    = strtok(NULL, " ");

					switch (region)
					{
					case REGION_CPU1:
						sscanf(type, "%x", &cpu1rom[num_cpu1rom].type);
						sscanf(offset, "%x", &cpu1rom[num_cpu1rom].offset);
						sscanf(length, "%x", &cpu1rom[num_cpu1rom].length);
						sscanf(crc, "%x", &cpu1rom[num_cpu1rom].crc);
						cpu1rom[num_cpu1rom].group = 0;
						cpu1rom[num_cpu1rom].skip = 0;
						num_cpu1rom++;
						break;

					case REGION_CPU2:
						sscanf(type, "%x", &cpu2rom[num_cpu2rom].type);
						sscanf(offset, "%x", &cpu2rom[num_cpu2rom].offset);
						sscanf(length, "%x", &cpu2rom[num_cpu2rom].length);
						sscanf(crc, "%x", &cpu2rom[num_cpu2rom].crc);
						cpu2rom[num_cpu2rom].group = 0;
						cpu2rom[num_cpu2rom].skip = 0;
						num_cpu2rom++;
						break;

					case REGION_SOUND1:
						sscanf(type, "%x", &snd1rom[num_snd1rom].type);
						sscanf(offset, "%x", &snd1rom[num_snd1rom].offset);
						sscanf(length, "%x", &snd1rom[num_snd1rom].length);
						sscanf(crc, "%x", &snd1rom[num_snd1rom].crc);
						snd1rom[num_snd1rom].group = 0;
						snd1rom[num_snd1rom].skip = 0;
						num_snd1rom++;
						break;

					case REGION_USER1:
						sscanf(type, "%x", &usr1rom[num_usr1rom].type);
						sscanf(offset, "%x", &usr1rom[num_usr1rom].offset);
						sscanf(length, "%x", &usr1rom[num_usr1rom].length);
						sscanf(crc, "%x", &usr1rom[num_usr1rom].crc);
						usr1rom[num_usr1rom].group = 0;
						usr1rom[num_usr1rom].skip = 0;
						num_usr1rom++;
						break;
					}
				}
				else if (str_cmp(&buf[1], "ROMX(") == 0)
				{
					char *type, *offset, *length, *crc;
					char *group, *skip;

					strtok(&buf[1], " ");
					type   = strtok(NULL, " ,");
					offset = strtok(NULL, " ,");
					length = strtok(NULL, " ,");
					crc    = strtok(NULL, " ,");
					group  = strtok(NULL, " ,");
					skip   = strtok(NULL, " ");

					switch (region)
					{
					case REGION_CPU1:
						sscanf(type, "%x", &cpu1rom[num_cpu1rom].type);
						sscanf(offset, "%x", &cpu1rom[num_cpu1rom].offset);
						sscanf(length, "%x", &cpu1rom[num_cpu1rom].length);
						sscanf(crc, "%x", &cpu1rom[num_cpu1rom].crc);
						sscanf(group, "%x", &cpu1rom[num_cpu1rom].group);
						sscanf(skip, "%x", &cpu1rom[num_cpu1rom].skip);
						num_cpu1rom++;
						break;

					case REGION_CPU2:
						sscanf(type, "%x", &cpu2rom[num_cpu2rom].type);
						sscanf(offset, "%x", &cpu2rom[num_cpu2rom].offset);
						sscanf(length, "%x", &cpu2rom[num_cpu2rom].length);
						sscanf(crc, "%x", &cpu2rom[num_cpu2rom].crc);
						sscanf(group, "%x", &cpu2rom[num_cpu2rom].group);
						sscanf(skip, "%x", &cpu2rom[num_cpu2rom].skip);
						num_cpu2rom++;
						break;

					case REGION_SOUND1:
						sscanf(type, "%x", &snd1rom[num_snd1rom].type);
						sscanf(offset, "%x", &snd1rom[num_snd1rom].offset);
						sscanf(length, "%x", &snd1rom[num_snd1rom].length);
						sscanf(crc, "%x", &snd1rom[num_snd1rom].crc);
						sscanf(group, "%x", &snd1rom[num_snd1rom].group);
						sscanf(skip, "%x", &snd1rom[num_snd1rom].skip);
						num_snd1rom++;
						break;

					case REGION_USER1:
						sscanf(type, "%x", &usr1rom[num_usr1rom].type);
						sscanf(offset, "%x", &usr1rom[num_usr1rom].offset);
						sscanf(length, "%x", &usr1rom[num_usr1rom].length);
						sscanf(crc, "%x", &usr1rom[num_usr1rom].crc);
						sscanf(group, "%x", &usr1rom[num_usr1rom].group);
						sscanf(skip, "%x", &usr1rom[num_usr1rom].skip);
						num_usr1rom++;
						break;
					}
				}
			}
		}
		fclose(fp);
		return 2;
	}
	return 3;
}


/******************************************************************************
	メモリインタフェース関数
******************************************************************************/

/*------------------------------------------------------
	メモリインタフェース初期化
-----------------------------------------------------*/

int memory_init(void)
{
	int i, res;

	memory_region_cpu1   = NULL;
	memory_region_cpu2   = NULL;
	memory_region_gfx1   = NULL;
	memory_region_sound1 = NULL;
	memory_region_user1  = NULL;

	memory_length_cpu1   = 0;
	memory_length_cpu2   = 0;
	memory_length_gfx1   = 0;
	memory_length_sound1 = 0;
	memory_length_user1  = 0;

	gfx_pen_usage[TILE08] = NULL;
	gfx_pen_usage[TILE16] = NULL;
	gfx_pen_usage[TILE32] = NULL;

	cache_init();
	pad_wait_clear();
	video_clear_screen();
	msg_screen_init("Load ROM");

	msg_printf("Checking ROM info...\n");

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: msg_printf("ERROR: This game not supported.\n"); break;
		case 2: msg_printf("ERROR: ROM not found. (zip file name incorrect)\n"); break;
		case 3: msg_printf("ERROR: rominfo.dat not found.\n"); break;
		}
		msg_printf("Press any button.\n");
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (!strcmp(game_name, "ssf2ta")
	||	!strcmp(game_name, "ssf2tu")
	||	!strcmp(game_name, "ssf2tur1")
	||	!strcmp(game_name, "ssf2xj"))
	{
		strcpy(cache_parent_name, "ssf2t");
	}
	else if (!strcmp(game_name, "ssf2t"))
	{
		cache_parent_name[0] = '\0';
	}
	else
	{
		strcpy(cache_parent_name, parent_name);
	}

	i = 0;
	driver = NULL;
	while (CPS2_driver[i].name)
	{
		if (!strcmp(game_name, CPS2_driver[i].name) || !strcmp(cache_parent_name, CPS2_driver[i].name))
		{
			driver = &CPS2_driver[i];
			break;
		}
		i++;
	}

	if (parent_name[0])
		msg_printf("ROM set \"%s\" (parent: %s).\n", game_name, parent_name);
	else
		msg_printf("ROM set \"%s\".\n", game_name);

	if (cache_parent_name[0])
		msg_printf("Cache file \"%s_cache.zip\" (parent: %s).\n", cache_parent_name, cache_parent_name);
	else
		msg_printf("Cache file \"%s_cache.zip\".\n", game_name);

	load_gamecfg(game_name);

	if (load_rom_cpu1() == 0) return 0;
	if (load_rom_user1() == 0) return 0;
	if (load_rom_cpu2() == 0) return 0;
	if (option_sound_enable)
	{
		if (load_rom_sound1() == 0) return 0;
	}
	if (load_rom_gfx1() == 0) return 0;

	static_ram1    = (u8 *)cps1_ram    - 0xff0000;
	static_ram2    = (u8 *)cps1_gfxram - 0x900000;
	static_ram3    = (u8 *)cps2_ram    - 0x660000;
	static_ram4    = (u8 *)cps2_output - 0x400000;
	static_ram5[0] = (u8 *)cps2_objram[0];
	static_ram5[1] = (u8 *)cps2_objram[1];

	qsound_sharedram1 = &memory_region_cpu2[0xc000];
	qsound_sharedram2 = &memory_region_cpu2[0xf000];

	msg_printf("Done.\n");

	msg_screen_clear();
	video_clear_screen();

	return 1;
}


/*------------------------------------------------------
	メモリインタフェース終了
------------------------------------------------------*/

void memory_shutdown(void)
{
	cache_shutdown();

	if (gfx_pen_usage[TILE08]) free(gfx_pen_usage[TILE08]);
	if (gfx_pen_usage[TILE16]) free(gfx_pen_usage[TILE16]);
	if (gfx_pen_usage[TILE32]) free(gfx_pen_usage[TILE32]);

	if (memory_region_cpu1)   free(memory_region_cpu1);
	if (memory_region_cpu2)   free(memory_region_cpu2);
	if (memory_region_gfx1)   free(memory_region_gfx1);
	if (memory_region_sound1) free(memory_region_sound1);
	if (memory_region_user1)  free(memory_region_user1);
}


/******************************************************************************
	M68000 メモリリード/ライト関数
******************************************************************************/

/*------------------------------------------------------
	M68000メモリリード (byte)
------------------------------------------------------*/

u8 m68000_read_memory_8(u32 offset)
{
//	int shift;
//	u16 mem_mask;

	offset &= M68K_AMASK;

//	switch (offset >> 20)
//	{
//	case 0x0:
//	case 0x1:
//	case 0x2:
//	case 0x3:
	if(offset<0x00400000){
		return READ_BYTE(memory_region_cpu1, offset);
	}

//	if(0==(offset & 1))
//	{		shift = 8;	mem_mask = 0x00ff;	}
//	else{	shift = 0;	mem_mask = 0xff00;	}

	switch (offset >> 16)
	{
	case 0x40:
		return READ_BYTE(static_ram4, offset);

	case 0x61:
		if(0==(offset & 1))
		{		return ((qsound_sharedram1[((offset>>1)&0x0fff)] | 0xff00)) >> 8;}
		else{	return ((qsound_sharedram1[((offset>>1)&0x0fff)] | 0xff00))     ;}

	case 0x66:
		return READ_BYTE(static_ram3, offset);

	case 0x70:
		{
		//	int bank = (offset & 0x8000) >> 15;
		//	offset &= 0x1fff;
			return READ_BYTE(static_ram5[((offset & 0x8000) >> 15)/*bank*/], (offset&0x1fff));
		}
		break;

	case 0x80:
		switch ((offset >> 8) & 0xff)
		{
		case 0x01:
		case 0x41:
			if(0==(offset & 1))
			{		return cps1_output_r(offset >> 1/*, 0x00ff*/) >> 8;}
			else{	return cps1_output_r(offset >> 1/*, 0xff00*/)     ;}

		case 0x40:
			if(0==(offset & 1)){
				switch (offset & 0xfe)
				{
				case 0x00: return cps2_inputport0_r(	/*offset >> 1, 0x00ff*/) >> 8;
				case 0x10: return cps2_inputport1_r(	/*offset >> 1, 0x00ff*/) >> 8;
				case 0x20: return cps2_eeprom_port_r(	/*offset >> 1, 0x00ff*/) >> 8;
				case 0x30: return cps2_qsound_volume_r(	/*offset >> 1, 0x00ff*/) >> 8;
				}
			}
			else{
				switch (offset & 0xfe)
				{
				case 0x00: return cps2_inputport0_r(	/*offset >> 1, 0xff00*/) ;
				case 0x10: return cps2_inputport1_r(	/*offset >> 1, 0xff00*/) ;
				case 0x20: return cps2_eeprom_port_r(	/*offset >> 1, 0xff00*/) ;
				case 0x30: return cps2_qsound_volume_r(	/*offset >> 1, 0xff00*/) ;
				}
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		return READ_BYTE(static_ram2, offset);

	case 0xff:
		return READ_BYTE(static_ram1, offset);
	}

	return 0xff;
}


/*------------------------------------------------------
	M68000リードメモリ (word)
------------------------------------------------------*/

u16 m68000_read_memory_16(u32 offset)
{
	offset &= M68K_AMASK;
	//switch (offset >> 20)
	//{
	//case 0x0:
	//case 0x1:
	//case 0x2:
	//case 0x3:
	if(offset<0x00400000){
		return READ_WORD(memory_region_cpu1, offset);
	}

	switch (offset >> 16)
	{
	case 0x40:
		return READ_WORD(static_ram4, offset);

	case 0x61:
		//return qsound_sharedram1_r(offset >> 1, 0);
		return ((qsound_sharedram1[((offset>>1)&0x0fff)] | 0xff00));

	case 0x66:
		return READ_WORD(static_ram3, offset);

	case 0x70:
		{
			//int bank = (offset & 0x8000) >> 15;
			//offset &= 0x1fff;
			return READ_WORD(static_ram5[((offset & 0x8000) >> 15)/*bank*/], (offset&0x1fff) );
		}
		break;

	case 0x80:
		switch ((offset >> 8) & 0xff)
		{
		case 0x01:
		case 0x41:
			return cps1_output_r(offset >> 1/*, 0*/);

		case 0x40:
			switch (offset & 0xfe)
			{
			case 0x00: return cps2_inputport0_r(	/*offset >> 1, 0*/);
			case 0x10: return cps2_inputport1_r(	/*offset >> 1, 0*/);
			case 0x20: return cps2_eeprom_port_r(	/*offset >> 1, 0*/);
			case 0x30: return cps2_qsound_volume_r(	/*offset >> 1, 0*/);
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		return READ_WORD(static_ram2, offset);

	case 0xff:
		return READ_WORD(static_ram1, offset);
	}

	return 0xffff;
}


/*------------------------------------------------------
	M68000リードメモリ (long)
------------------------------------------------------*/
//u32 m68000_read_memory_32(u32 offset)
//{
//	return (m68000_read_memory_16(offset) << 16) | m68000_read_memory_16(offset + 2);
//}


/*------------------------------------------------------
	M68000ライトメモリ (byte)
------------------------------------------------------*/

void m68000_write_memory_8(u32 offset, u8 data)
{
//	int shift;
//	u16 mem_mask;

//	shift = (~offset & 1) << 3;
//	mem_mask = ~(0xff << shift);

//	if(0==(offset & 1))
//	{		shift = 8;	mem_mask = 0x00ff;}
//	else{	shift = 0;	mem_mask = 0xff00;}

	offset &= M68K_AMASK;

	switch (offset >> 16)
	{
	case 0x40:
		WRITE_BYTE(static_ram4, offset, data);
		return;

	case 0x61:
		if(0==(offset & 1))
		{		qsound_sharedram1_w(offset >> 1, data << 8, 0x00ff);}
		else{	qsound_sharedram1_w(offset >> 1, data     , 0xff00);}
		return;

	case 0x66:
		WRITE_BYTE(static_ram3, offset, data);
		return;

	case 0x70:
		{
			int bank = (offset & 0x8000) >> 15;
			offset &= 0x1fff;
			WRITE_BYTE(static_ram5[bank], offset, data);
		}
		return;

	case 0x80:
		switch ((offset >> 8) & 0xff)
		{
		case 0x01:
		case 0x41:
			if(0==(offset & 1))
			{		cps1_output_w(offset >> 1, data << 8, 0x00ff);}
			else{	cps1_output_w(offset >> 1, data     , 0xff00);}
			return;

		case 0x40:
			switch (offset & 0xfe)
			{
			case 0x40:
				if(0==(offset & 1))
				{		cps2_eeprom_port_w(offset >> 1, data << 8, 0x00ff);}
				else{	cps2_eeprom_port_w(offset >> 1, data     , 0xff00);}
				return;
			case 0xe0:
				if (offset & 1)
				{
					cps2_objram_bank = data & 1;
					static_ram5[1] = (u8 *)cps2_objram[cps2_objram_bank ^ 1];
				}
				return;
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		WRITE_BYTE(static_ram2, offset, data);
		return;

	case 0xff:
		WRITE_BYTE(static_ram1, offset, data);
		return;
	}
}


/*------------------------------------------------------
	M68000ライトメモリ (word)
------------------------------------------------------*/

void m68000_write_memory_16(u32 offset, u16 data)
{
	offset &= M68K_AMASK;

	switch (offset >> 16)
	{
	case 0x40:
		WRITE_WORD(static_ram4, offset, data);
		return;

	case 0x61:
		qsound_sharedram1_w(offset >> 1, data, 0);
		return;

	case 0x66:
		WRITE_WORD(static_ram3, offset, data);
		return;

	case 0x70:
		{
			int bank = (offset & 0x8000) >> 15;
			offset &= 0x1fff;
			WRITE_WORD(static_ram5[bank], offset, data);
			return;
		}
		break;

	case 0x80:
		switch ((offset >> 8) & 0xff)
		{
		case 0x01:
		case 0x41:
			cps1_output_w(offset >> 1, data, 0);
			return;

		case 0x40:
			switch (offset & 0xfe)
			{
			case 0x40: cps2_eeprom_port_w(offset >> 1, data, 0); return;
			case 0xe0:
				cps2_objram_bank = data & 1;
				static_ram5[1] = (u8 *)cps2_objram[cps2_objram_bank ^ 1];
				return;
			}
			break;
		}
		break;

	case 0x90:
	case 0x91:
	case 0x92:
		WRITE_WORD(static_ram2, offset, data);
		return;

	case 0xff:
		WRITE_WORD(static_ram1, offset, data);
		return;
	}
}


/*------------------------------------------------------
	M68000ライトメモリ (long)
------------------------------------------------------*/
//void m68000_write_memory_32(u32 offset, u32 data)
//{
//	m68000_write_memory_16(offset, data >> 16);
//	m68000_write_memory_16(offset + 2, data);
//}


/******************************************************************************
	Z80 メモリリード/ライト関数
******************************************************************************/

/*------------------------------------------------------
	Z80リードメモリ (byte)
------------------------------------------------------*/
u8 z80_read_memory_8(u16 offset)
{
#if 000
//	offset &= Z80_AMASK;
	switch (offset >> 12)
	{
	case 0x00:	case 0x01:	case 0x02:	case 0x03:
	case 0x04:	case 0x05:	case 0x06:	case 0x07:
	case 0x08:	case 0x09:	case 0x0a:	case 0x0b:
	case 0x0c:							case 0x0f:
		// 0000-7fff: ROM
		// 8000-bfff: banked ROM
		// c000-cfff: QSOUND shared RAM 1
		// f000-ffff: QSOUND shared RAM 2
		return memory_region_cpu2[offset];
	case 0x0d:
		if (offset == 0xd007) return 0x80;
		break;
	}
	return 0;
#else
	volatile unsigned short iii=(offset&0xf000);
	switch(iii){
	default:		return memory_region_cpu2[offset];
	case 0xd000:	if (offset == 0xd007){ return 0x80;}
	case 0xe000:	return 0;
	}
#endif
}


/*------------------------------------------------------
	Z80ライトメモリ (byte)
------------------------------------------------------*/

void z80_write_memory_8(u16 offset, u8 data)
{
//	offset &= Z80_AMASK;
	switch (offset & 0xf000/*>> 12*/)
	{
	case 0xc000:	// c000-cfff: QSOUND shared RAM 1
	case 0xf000:	// f000-ffff: QSOUND shared RAM 2
		memory_region_cpu2[offset] = data;
		break;

	case 0xd000:
		switch (offset)
		{
		case 0xd000: qsound_data_h_w(0, data);	break;
		case 0xd001: qsound_data_l_w(0, data);	break;
		case 0xd002: qsound_cmd_w(0, data); 	break;
		case 0xd003: qsound_banksw_w(0, data);	break;
		}
		break;
	}
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( memory )
{
	state_save_byte(cps1_ram,		   0x10000);
	state_save_byte(cps1_gfxram,	   0x30000);
	state_save_byte(cps1_output,		 0x100);
	state_save_byte(cps2_ram,			0x4002);
	state_save_byte(cps2_objram[0], 	0x2000);
	state_save_byte(cps2_objram[1], 	0x2000);
	state_save_byte(cps2_output,		  0x10);
	state_save_byte(qsound_sharedram1,	0x1000);
	state_save_byte(qsound_sharedram2,	0x1000);
}

STATE_LOAD( memory )
{
	state_load_byte(cps1_ram,		   0x10000);
	state_load_byte(cps1_gfxram,	   0x30000);
	state_load_byte(cps1_output,		 0x100);
	state_load_byte(cps2_ram,			0x4002);
	state_load_byte(cps2_objram[0], 	0x2000);
	state_load_byte(cps2_objram[1], 	0x2000);
	state_load_byte(cps2_output,		  0x10);
	state_load_byte(qsound_sharedram1,	0x1000);
	state_load_byte(qsound_sharedram2,	0x1000);
}

#endif /* SAVE_STATE */
