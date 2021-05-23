/******************************************************************************

	cpuintrf.h

	Core CPU interface definitions. (compatible for M.A.M.E.)

******************************************************************************/

#ifndef CPUINTRF_H
#define CPUINTRF_H


/*************************************
 *
 *	Enum listing all the CPUs
 *
 *************************************/
//enum
//{
//	CPU _M68000 = 0,
//	CPU _Z80=1,
//	MAX _CPU=2
//};



/*************************************
 *
 *	Interrupt line constants
 *
 *************************************/

enum
{
	/* line states */
	CLEAR_LINE = 0,				/* clear (a fired, held or pulsed) line */
	ASSERT_LINE,				/* assert an interrupt immediately */
	HOLD_LINE,					/* hold interrupt line until acknowledged */
	PULSE_LINE,					/* pulse interrupt line for one instruction */
	IRQ_LINE_NMI = 127			/* IRQ line for NMIs */
};

#endif /* CPUINTRF_H */
