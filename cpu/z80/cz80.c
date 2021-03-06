/********************************************************************************/
/*                                                                              */
/* CZ80 (Z80 CPU emulator) version 0.9                                          */
/* Compiled with Dev-C++                                                        */
/* Copyright 2004-2005 St?phane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cz80.h"

#define CF					0x01
#define NF					0x02
#define PF					0x04
#define VF					PF
#define XF					0x08
#define HF					0x10
#define YF					0x20
#define ZF					0x40
#define SF					0x80

// include macro file
//////////////////////

#include "cz80macro.h"

// shared global variable
//////////////////////////
#if CZ80_BIG_FLAGS_ARRAY
static u8 SZHVC_add[2*256*256];
static u8 SZHVC_sub[2*256*256];
#endif
static u8 SZ[256] /*__attribute__((aligned(64)))*/ ;            // zero and sign flags
static u8 SZP[256];           // zero, sign and parity flags
static u8 SZ_BIT[256];        // zero, sign and parity/overflow (=zero) flags for BIT opcode
static u8 SZHV_inc[256];      // zero, sign, half carry and overflow flags INC R8
static u8 SZHV_dec[256];      // zero, sign, half carry and overflow flags DEC R8


static cz80_struc CZ80 /*__attribute__((aligned(64)))*/ ;
int z80_ICount;
static int z80_ExtraCycles;

static u8 *pzR8[8];
static union16 *pzR16[4];

// core main functions
///////////////////////


static cz80_struc *ZZ;

int z80_execute(int cycles)
//{
//	return Cz80_Exec(&CZ80, cycles);
//}
//s32 Cz80_Exec(/*cz80_struc *cpu,*/ s32 cycles)
{
#if CZ80_USE_JUMPTABLE
#include "cz80jmp.c"
#endif

//	cz80_struc *CPU;
	u32 PC;
	u32 Opcode;
	u32 adr;
	u32 res;
	u32 val;
	int afterEI = 0;

//	CPU = cpu;
	PC = ZZ->PC;
	z80_ICount = cycles - z80_ExtraCycles;
	z80_ExtraCycles = 0;

	if (!ZZ->HaltState)
	{
Cz80_Exec:
		if (z80_ICount > 0)
		{
			union16 *data = pzHL;
			Opcode = FETCH_BYTE;
#if CZ80_EMULATE_R_EXACTLY
			zR++;
#endif
			#include "cz80_op.c"
		}

		if (afterEI)
		{
			z80_ICount += z80_ExtraCycles;
			z80_ExtraCycles = 0;
			afterEI = 0;
Cz80_Check_Interrupt:
			CHECK_INT
			goto Cz80_Exec;
		}
	}
	else z80_ICount = 0;

Cz80_Exec_End:
	ZZ->PC = PC;
	cycles -= z80_ICount;
#if (CZ80_EMULATE_R_EXACTLY == 0)
	zR = (zR + (cycles >> 2)) & 0x7f;
#endif

	return cycles;
}



//static void Cz80_Clear_IRQ(void/*cz80_struc *CPU*/)
//{
//	ZZ->IRQState = 0;
//}

// setting core functions
//////////////////////////


// externals main functions
////////////////////////////


/******************************************************************************
	z80.c
	Handling CZ80 core.
******************************************************************************/

#include "emumain.h"
//#include "cz80.h"


static u8 irq_state;

int z80_irq_callback(int line)
{
	if (irq_state == HOLD_LINE)
	{
		irq_state = CLEAR_LINE;
		ZZ->IRQState = 0;//Cz80_Clear_IRQ(/*&CZ80*/);
	}
	return 0xff;
}

static void Cz80_Init(void/*cz80_struc *CPU*/)
{
	u32 i, j, p;
	ZZ=&CZ80;
#if CZ80_BIG_FLAGS_ARRAY
	int oldval, newval, val;
	u8 *padd, *padc, *psub, *psbc;
#endif

	memset(&CZ80/*CPU*/, 0, sizeof(cz80_struc));

	// flags tables initialisation
	for (i = 0; i < 256; i++)
	{
		SZ[i] = i & (SF | YF | XF);
		if (!i) SZ[i] |= ZF;

		SZ_BIT[i] = i & (SF | YF | XF);
		if (!i) SZ_BIT[i] |= ZF | PF;

		for (j = 0, p = 0; j < 8; j++) if (i & (1 << j)) p++;
		SZP[i] = SZ[i];
		if (!(p & 1)) SZP[i] |= PF;

		SZHV_inc[i] = SZ[i];
		if(i == 0x80) SZHV_inc[i] |= VF;
		if((i & 0x0f) == 0x00) SZHV_inc[i] |= HF;

		SZHV_dec[i] = SZ[i] | NF;
		if (i == 0x7f) SZHV_dec[i] |= VF;
		if ((i & 0x0f) == 0x0f) SZHV_dec[i] |= HF;
	}

#if CZ80_BIG_FLAGS_ARRAY
	padd = &SZHVC_add[  0*256];
	padc = &SZHVC_add[256*256];
	psub = &SZHVC_sub[  0*256];
	psbc = &SZHVC_sub[256*256];

	for (oldval = 0; oldval < 256; oldval++)
	{
		for (newval = 0; newval < 256; newval++)
		{
			/* add or adc w/o carry set */
			val = newval - oldval;
			*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padd |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) < (oldval & 0x0f)) *padd |= HF;
			if (newval < oldval ) *padd |= CF;
			if ((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padd |= VF;
			padd++;

			/* adc with carry set */
			val = newval - oldval - 1;
			*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) <= (oldval & 0x0f)) *padc |= HF;
			if (newval <= oldval) *padc |= CF;
			if ((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padc |= VF;
			padc++;

			/* cp, sub or sbc w/o carry set */
			val = oldval - newval;
			*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psub |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) > (oldval & 0x0f)) *psub |= HF;
			if (newval > oldval) *psub |= CF;
			if ((val^oldval) & (oldval^newval) & 0x80) *psub |= VF;
			psub++;

			/* sbc with carry set */
			val = oldval - newval - 1;
			*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psbc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) >= (oldval & 0x0f)) *psbc |= HF;
			if (newval >= oldval) *psbc |= CF;
			if ((val ^ oldval) & (oldval^newval) & 0x80) *psbc |= VF;
			psbc++;
		}
	}
#endif

	pzR8[0] = &zB;
	pzR8[1] = &zC;
	pzR8[2] = &zD;
	pzR8[3] = &zE;
	pzR8[4] = &zH;
	pzR8[5] = &zL;
	pzR8[6] = &zF;	// ?????̓s?????AA?Ɠ????ւ?
	pzR8[7] = &zA;	// ?????̓s?????AF?Ɠ????ւ?

	pzR16[0] = pzBC;
	pzR16[1] = pzDE;
	pzR16[2] = pzHL;
	pzR16[3] = pzAF;

	zIX = zIY = 0xffff;
	zF = ZF;
}
static void Cz80_Set_Fetch(/*cz80_struc *cpu, u32 low_adr, u32 high_adr,*/ u32 fetch_adr)
{
	u32 i, j;

	i = /*low_adr*/ 0x0000 >> CZ80_FETCH_SFT;
	j = /*high_adr*/0xffff >> CZ80_FETCH_SFT;
	fetch_adr -= i << CZ80_FETCH_SFT;
	while (i <= j) /*cpu->*/ZZ->Fetch[i++] = (u8 *)fetch_adr;
}
//static void Cz80_Set_ReadB(cz80_struc *cpu, CZ80_READ8 *Func)
//{
//	cpu->Read_Byte = Func;
//}
//static void Cz80_Set_WriteB(cz80_struc *cpu, CZ80_WRITE8 *Func)
//{
//	cpu->Write_Byte = Func;
//}
//static void Cz80_Set_INPort(cz80_struc *cpu, CZ80_READ8 *Func)
//{
//	cpu->IN_Port = Func;
//}
//static void Cz80_Set_OUTPort(cz80_struc *cpu, CZ80_WRITE8 *Func)
//{
//	cpu->OUT_Port = Func;
//}
//static void Cz80_Set_IRQ_Callback(cz80_struc *cpu, CZ80_INT_CALLBACK *Func)
//{
//	cpu->Interrupt_Ack = Func;
//}

void Cz80_Set_PC(/*cz80_struc *CPU,*/ u32 val)
{
	ZZ->BasePC = (u32)ZZ->Fetch[val >> CZ80_FETCH_SFT];
	ZZ->PC = val + ZZ->BasePC;
}
void z80_reset(void)
//{
//	Cz80_Reset(/*&CZ80*/);
//}
//void Cz80_Reset(void/*cz80_struc *CPU*/)
{
	zI  = 0;
	zR  = 0;
	zR2 = 0;

	z80_ICount = 0;
	z80_ExtraCycles = 0;

	Cz80_Set_PC(/*CPU,*/ 0);
}
void z80_init(void)
{
	Cz80_Init(/*&CZ80*/);
	Cz80_Set_Fetch(/*&CZ80, 0x0000, 0xffff,*/ (u32)memory_region_cpu2);
	ZZ->Read_Byte		=z80_read_memory_8;	//Cz80_Set_ReadB(&CZ80, &z80_read_memory_8);
	ZZ->Write_Byte		=z80_write_memory_8;//Cz80_Set_WriteB(&CZ80, &z80_write_memory_8);
	ZZ->Interrupt_Ack	=z80_irq_callback;	//Cz80_Set_IRQ_Callback(&CZ80, &z80_irq_callback);
	z80_reset();//Cz80_Reset(/*&CZ80*/);
}





void z80_exit(void)
{
	/* nothing to do ? */
}



//static void Cz80_Set_NMI(void/*cz80_struc *CPU*/)
//{
//	zIFF1 = 0;
//	z80_ExtraCycles += 11;
//	ZZ->IRQState = 0;
//	ZZ->HaltState = 0;
//	PUSH_16(ZZ->PC - ZZ->BasePC)
//	Cz80_Set_PC(/*CPU,*/ 0x66);
//}
//INLINE void z80_assert_irq(int irqline)
//{
//	if (irqline == IRQ_LINE_NMI)
//		Cz80_Set_NMI(/*&CZ80*/);
//	else
//		Cz80_Set_IRQ(/*&CZ80,*/ irqline);
//}


INLINE void z80_clear_irq(int irqline)
{
	ZZ->IRQState = 0;//Cz80_Clear_IRQ(/*&CZ80*/);
}



void z80_set_irq_line0_HOLD(/*int irqline=0, int state=HOLD_LINE*/)
{
	irq_state = HOLD_LINE;//state;
//	switch (state)
//	{
//	case CLEAR_LINE:	z80_clear_irq(irqline);		return;
//	case ASSERT_LINE:	z80_assert_irq(irqline);	return;
//	default:			z80_assert_irq(irqline);	return;
//	}
//Cz80_Set_IRQ(0);//z80_assert_irq(0);
//static void Cz80_Set_IRQ(/*cz80_struc *CPU,*/ s32 vector)
{
//s32 vector;
//vector=0;
	u32 PC = ZZ->PC;

	ZZ->IRQState = 1;
	CHECK_INT
	ZZ->PC = PC;
}

}


#ifdef SAVE_STATE
static u32 Cz80_Get_PC(void/*cz80_struc *CPU*/)
{
	u32 PC = ZZ->PC;
	return zRealPC;
}
void state_save_z80(FILE *fp)
{
	u32 pc = Cz80_Get_PC(/*&CZ80*/);

	state_save_word(&ZZ->BC.W, 1);
	state_save_word(&ZZ->DE.W, 1);
	state_save_word(&ZZ->HL.W, 1);
	state_save_word(&ZZ->AF.W, 1);
	state_save_word(&ZZ->IX.W, 1);
	state_save_word(&ZZ->IY.W, 1);
	state_save_word(&ZZ->SP.W, 1);
	state_save_long(&pc, 1);
	state_save_word(&ZZ->BC2.W, 1);
	state_save_word(&ZZ->DE2.W, 1);
	state_save_word(&ZZ->HL2.W, 1);
	state_save_word(&ZZ->AF2.W, 1);
	state_save_word(&ZZ->R.W, 1);
	state_save_word(&ZZ->IFF.W, 1);
	state_save_byte(&ZZ->I, 1);
	state_save_byte(&ZZ->IM, 1);
	state_save_byte(&ZZ->IRQState, 1);
	state_save_byte(&ZZ->HaltState, 1);
	state_save_byte(&irq_state, 1);
}
void state_load_z80(FILE *fp)
{
	u32 pc;

	state_load_word(&ZZ->BC.W, 1);
	state_load_word(&ZZ->DE.W, 1);
	state_load_word(&ZZ->HL.W, 1);
	state_load_word(&ZZ->AF.W, 1);
	state_load_word(&ZZ->IX.W, 1);
	state_load_word(&ZZ->IY.W, 1);
	state_load_word(&ZZ->SP.W, 1);
	state_load_long(&pc, 1);
	state_load_word(&ZZ->BC2.W, 1);
	state_load_word(&ZZ->DE2.W, 1);
	state_load_word(&ZZ->HL2.W, 1);
	state_load_word(&ZZ->AF2.W, 1);
	state_load_word(&ZZ->R.W, 1);
	state_load_word(&ZZ->IFF.W, 1);
	state_load_byte(&ZZ->I, 1);
	state_load_byte(&ZZ->IM, 1);
	state_load_byte(&ZZ->IRQState, 1);
	state_load_byte(&ZZ->HaltState, 1);
	state_load_byte(&irq_state, 1);

	Cz80_Set_PC(/*&CZ80,*/ pc);
}
#endif

