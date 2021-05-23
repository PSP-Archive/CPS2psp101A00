/******************************************************************************

	icon_s.c
	icon_l.c
	Small icon data.
	Large icon data.

******************************************************************************/

#include "psp/psp.h"

#define NUM_FONTS	0x0e
#define NUM_FONTL	0x07

#define v (0x0)
#define x (0x1)
#define i (0x2)
#define j (0x3)
#define c (0x4)
#define o (0x5)
#define u (0x6)
#define d (0x7)
#define I (0x8)
#define T (0x9)
#define J (0xa)
#define U (0xb)
#define H (0xc)
#define A (0xd)
#define N (0xe)
#define M (0xf)

#define f(aa,bb) (aa+(bb<<4))
#define w20(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20)  \
f(a01,a02),f(a03,a04),f(a05,a06),f(a07,a08),f(a09,a10),f(a11,a12),f(a13,a14),f(a15,a16),f(a17,a18),f(a19,a20),
#define w22(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22)  \
f(a01,a02),f(a03,a04),f(a05,a06),f(a07,a08),f(a09,a10),f(a11,a12),f(a13,a14),f(a15,a16),f(a17,a18),f(a19,a20),f(a21,a22),
#define w24(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24)  \
f(a01,a02),f(a03,a04),f(a05,a06),f(a07,a08),f(a09,a10),f(a11,a12),f(a13,a14),f(a15,a16),f(a17,a18),f(a19,a20),f(a21,a22),f(a23,a24),

#define w28(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28)  \
f(a01,a02),f(a03,a04),f(a05,a06),f(a07,a08),f(a09,a10),f(a11,a12),f(a13,a14),f(a15,a16),f(a17,a18),f(a19,a20),f(a21,a22),f(a23,a24),f(a25,a26),f(a27,a28),
#define w30(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30)  \
f(a01,a02),f(a03,a04),f(a05,a06),f(a07,a08),f(a09,a10),f(a11,a12),f(a13,a14),f(a15,a16),f(a17,a18),f(a19,a20),f(a21,a22),f(a23,a24),f(a25,a26),f(a27,a28),f(a29,a30),
#define w32(a01,a02,a03,a04,a05,a06,a07,a08,a09,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32)  \
f(a01,a02),f(a03,a04),f(a05,a06),f(a07,a08),f(a09,a10),f(a11,a12),f(a13,a14),f(a15,a16),f(a17,a18),f(a19,a20),f(a21,a22),f(a23,a24),f(a25,a26),f(a27,a28),f(a29,a30),f(a31,a32),

/*------------------------------------------------------
	gryph data
------------------------------------------------------*/

static const u8 icon_s[] = {
// ICONS0: configuretion icon_S(24x18)
#define S0W 24
#define S0H 18
w24(v,v,v,v,x,i,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(v,v,i,I,H,A,U,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(v,v,I,A,A,N,N,N,d,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(v,v,v,x,j,o,U,N,N,j,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(v,v,v,v,v,v,o,N,M,d,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(x,v,v,v,v,v,j,N,N,U,x,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(x,d,I,o,j,j,U,N,N,N,A,u,v,v,v,v,v,v,v,v,v,v,v,v)
w24(v,c,N,N,A,A,N,N,N,N,N,M,H,c,v,v,v,v,v,v,v,v,v,v)
w24(v,v,o,H,N,M,N,H,N,M,N,N,M,N,J,j,v,j,o,o,x,v,v,v)
w24(v,v,v,x,o,u,c,v,j,U,M,M,N,N,M,N,U,A,N,N,H,o,v,v)
w24(v,v,v,v,v,v,v,v,v,v,o,H,M,N,N,N,N,N,A,A,N,N,o,v)
w24(v,v,v,v,v,v,v,v,v,v,v,x,d,A,N,N,N,H,c,j,u,T,I,x)
w24(v,v,v,v,v,v,v,v,v,v,v,v,v,x,J,N,N,c,v,v,v,v,v,x)
w24(v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,M,N,o,v,v,v,v,v,v)
w24(v,v,v,v,v,v,v,v,v,v,v,v,v,v,j,A,N,U,o,j,v,v,v,v)
w24(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,d,N,N,N,A,A,I,v,v)
w24(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,U,A,H,J,j,v,v)
w24(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,j,i,v,v,v,v)
// ICONS1: control pad icon_S(22x16)
#define S1W 24
#define S1H 13
w24(v,o,M,M,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(o,M,M,M,M,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(M,M,M,M,M,N,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w24(M,M,M,M,N,N,v,v,v,v,v,v,v,v,v,v,v,v,v,v,o,o,v,v)
w24(o,M,M,N,N,o,v,v,v,v,v,v,v,v,v,c,c,v,v,o,M,M,o,v)
w24(v,o,N,N,o,v,v,v,v,v,j,j,v,v,c,N,N,c,v,v,o,o,v,v)
w24(v,v,v,v,v,v,v,v,v,j,A,A,j,v,v,c,c,v,v,v,v,v,v,v)
w24(v,v,o,o,v,v,v,v,v,v,j,j,v,v,v,v,v,v,v,v,o,M,M,o)
w24(v,v,J,I,v,v,v,v,v,v,v,v,v,v,v,c,N,N,c,v,M,M,M,M)
w24(v,v,J,I,v,v,v,v,v,v,j,A,A,j,v,N,N,N,N,v,o,M,M,o)
w24(v,v,J,I,v,v,v,v,v,v,A,A,A,A,v,c,N,N,c,v,v,v,v,v)
w24(v,v,J,I,v,v,v,v,v,v,j,A,A,j,v,v,v,v,v,v,v,v,v,v)
w24(v,v,J,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
// ICONS2: folder icon_S(20x15)
#define S2W 20
#define S2H 15
w20(v,v,j,o,o,o,o,i,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,j,H,U,U,U,U,H,x,v,v,v,v,v,v,v,v,v,v,v)
w20(i,U,c,v,v,v,v,d,J,x,v,v,v,v,v,v,v,v,v,v)
w20(I,d,v,v,v,v,v,v,T,U,J,J,J,J,J,J,U,c,v,v)
w20(I,o,v,v,v,v,v,v,v,j,j,j,i,i,i,i,T,d,v,v)
w20(I,o,v,T,U,J,J,U,J,J,J,J,J,J,J,J,H,H,J,c)
w20(I,c,i,N,N,N,N,M,M,M,M,M,N,N,N,N,A,A,N,j)
w20(I,c,o,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,H,x)
w20(I,o,I,N,N,N,N,N,N,N,N,N,N,N,N,A,A,N,J,v)
w20(I,u,U,N,N,N,N,N,N,N,N,N,N,N,N,A,A,N,d,v)
w20(I,I,A,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,c,v)
w20(I,H,A,A,A,N,N,N,N,N,N,N,N,N,A,A,A,H,x,v)
w20(I,N,A,A,A,A,N,N,N,N,N,N,N,A,A,A,N,J,v,v)
w20(I,N,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,I,v,v)
w20(u,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,c,v,v)
// ICONS3: CPS2 icon_S(24x12)
#define S3W 20
#define S3H 15
//0x10,0x54,0x66,0x56,0x55,0x55,0x56,0x45,0x52,0x66,0x56,0x04,
//0x51,0xc8,0xbb,0xcc,0xa8,0xcc,0xcb,0x8c,0xa6,0xbd,0xcc,0x07,
//0x95,0xaf,0x77,0xe7,0x6a,0xcf,0x76,0xee,0xf8,0x69,0xe7,0x08,
//0xf7,0x4a,0x02,0x95,0x58,0xbe,0x54,0xfb,0xec,0x45,0x95,0x07,
//0xfa,0x27,0x00,0x41,0x54,0xbe,0x66,0xbd,0xfa,0x8a,0x67,0x05,
//0xfb,0x17,0x00,0x00,0x50,0xce,0xdb,0x6c,0xd6,0xff,0xdf,0x08,
//0xfa,0x37,0x00,0x00,0x50,0xbe,0x76,0x46,0x65,0x87,0xfa,0x0c,
//0xf7,0x5c,0x03,0x31,0x54,0xbe,0x04,0x30,0x77,0x23,0xd5,0x0d,
//0xa5,0xbf,0x57,0x96,0x58,0xcf,0x35,0x20,0xcb,0x45,0xf8,0x09,
//0x52,0xe9,0xcd,0xcc,0xa8,0xde,0x59,0x30,0xe9,0xbc,0xac,0x05,
//0x10,0x65,0x87,0x67,0x76,0x66,0x57,0x20,0x66,0x77,0x56,0x01,
//0x00,0x10,0x11,0x01,0x10,0x00,0x01,0x00,0x10,0x11,0x01,0x00,
//
w20(v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v)
w20(v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v)
w20(v,v,v,v,v,M,o,v,v,v,M,v,v,v,o,M,v,v,v,v)
w20(v,v,v,v,v,o,M,o,v,v,M,v,v,o,M,o,v,v,v,v)
w20(v,v,v,v,v,v,o,M,o,v,o,v,o,M,o,v,v,v,v,v)
w20(v,v,v,v,v,v,v,o,M,v,v,v,M,o,v,v,v,v,v,v)
w20(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,o,M,M,M,o,v,v,v,v,v,v,v,o,M,M,M,o,v)
w20(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,v,v,v,v,v,o,M,v,v,v,M,o,v,v,v,v,v,v)
w20(v,v,v,v,v,v,o,M,o,v,o,v,o,M,o,v,v,v,v,v)
w20(v,v,v,v,v,o,M,o,v,v,M,v,v,o,M,o,v,v,v,v)
w20(v,v,v,v,v,M,o,v,v,v,M,v,v,v,o,M,v,v,v,v)
w20(v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v)
w20(v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v)

// ICONS4: Return to game icon_S(22x15)
#define S4W 22
#define S4H 15
w22(v,v,u,I,I,I,I,I,I,I,I,I,I,I,I,I,u,j,v,v,v,v)
w22(v,v,U,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,I,x,v,v)
w22(v,v,J,A,A,A,A,A,A,A,A,A,A,A,A,A,A,N,N,J,v,v)
w22(v,v,x,x,x,x,x,x,x,x,x,x,x,x,x,x,c,U,A,N,u,v)
w22(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,i,H,N,U,v)
w22(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,T,N,A,i)
w22(v,v,v,v,v,i,v,v,v,v,v,v,v,v,v,v,v,v,I,N,A,i)
w22(v,v,v,v,T,I,v,v,v,v,v,v,v,v,v,v,v,x,H,N,H,v)
w22(v,v,i,J,M,I,v,v,v,v,v,v,v,v,v,v,i,J,N,N,d,v)
w22(v,c,H,N,N,A,U,U,U,U,U,U,U,U,U,U,A,N,N,U,x,v)
w22(x,U,N,A,A,N,M,M,M,M,M,M,M,M,M,M,N,N,J,i,v,v)
w22(v,x,J,N,N,H,J,J,J,J,J,J,J,J,J,J,I,c,v,v,v,v)
w22(v,v,v,I,M,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w22(v,v,v,v,d,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w22(v,v,v,v,v,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
// ICONS5: exit to psp icon_S(22x12)
#define S5W 22
#define S5H 12
w22(v,v,v,j,c,c,c,c,o,o,o,o,o,c,c,c,c,c,j,v,v,v)
w22(v,x,J,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,J,x,v)
w22(v,T,N,N,u,v,v,v,v,v,v,v,v,v,v,v,v,c,A,N,T,v)
w22(j,A,A,N,o,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,A,c)
w22(d,N,A,N,u,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,N,d)
w22(T,N,A,N,u,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,A,T)
w22(T,N,A,N,u,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,A,J)
w22(I,N,A,N,u,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,N,T)
w22(u,N,A,N,u,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,N,u)
w22(i,H,A,N,o,v,v,v,v,v,v,v,v,v,v,v,v,j,A,A,A,i)
w22(v,u,N,N,I,c,c,c,c,c,c,c,c,c,c,c,c,d,N,N,u,v)
w22(v,v,o,U,H,H,H,H,H,H,H,H,H,H,H,H,H,H,U,u,v,v)
// ICONS6: dip switch icon_S(20x15)
#define S6W 20
#define S6H 15
w20(U,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,H,J)
w20(A,A,H,H,A,N,A,A,A,N,N,A,H,A,N,A,H,H,A,H)
w20(A,I,u,u,d,A,j,x,x,U,U,u,d,u,A,d,d,u,T,H)
w20(A,I,U,A,I,A,i,v,v,U,J,U,N,I,H,I,A,U,T,H)
w20(A,I,U,H,I,A,i,v,v,U,U,J,A,I,H,I,H,J,T,H)
w20(A,I,v,v,o,N,i,v,v,U,J,v,v,j,N,o,v,v,I,H)
w20(A,I,v,v,o,N,i,v,v,U,U,v,v,j,N,c,v,v,I,H)
w20(A,I,v,v,o,N,i,v,v,U,U,v,v,j,N,c,v,v,I,H)
w20(A,I,v,v,o,N,i,v,v,U,J,v,v,j,N,c,v,v,I,H)
w20(A,I,v,v,o,A,d,U,I,U,J,v,v,j,N,c,v,v,I,H)
w20(A,d,v,v,o,A,T,M,U,U,J,v,v,j,N,c,v,v,I,H)
w20(A,I,v,v,u,A,u,T,u,U,J,v,v,j,N,o,v,v,T,H)
w20(A,A,H,H,A,N,H,U,H,A,A,H,H,H,A,H,H,H,A,H)
w20(H,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,U)
w20(x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x)
// ICONS7: up folder icon(20x14)
#define S7W 20
#define S7H 14
w20(v,v,v,T,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,u,N,A,j,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,c,A,A,N,H,x,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(j,A,N,A,N,M,J,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(I,J,H,N,A,J,U,J,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,d,M,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,d,M,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,d,M,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,d,M,A,i,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,o,N,N,u,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,i,A,N,A,c,v,v,v,v,v,v,v,v,v,v,v,v,v)
w20(v,v,v,u,N,N,N,H,U,U,U,U,J,x,d,J,i,d,i,j)
w20(v,v,v,v,I,N,N,N,N,N,N,N,N,i,T,A,j,J,j,o)
w20(v,v,v,v,v,c,J,H,A,A,A,A,H,i,I,H,i,T,j,c)
// ICONS8: memory stick duo icon(20x13)
#define S8W 20
#define S8H 13
w20(M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,M,U,v)
w20(N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,H,x)
w20(N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,H,x)
w20(N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,H,x)
w20(N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,H,x)
w20(N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,H,x)
w20(N,N,N,N,o,H,N,N,N,N,N,N,N,N,N,A,A,A,H,x)
w20(N,N,N,o,v,H,N,N,N,N,N,N,N,N,N,A,A,A,H,x)
w20(N,N,o,v,v,H,N,N,N,N,N,N,N,N,N,A,A,A,H,x)
w20(c,N,N,o,v,H,N,N,N,N,N,N,N,N,A,A,A,A,H,x)
w20(v,o,N,N,o,H,N,N,N,N,N,N,N,N,A,A,A,A,H,x)
w20(v,v,o,N,N,N,N,N,N,N,N,N,A,A,A,A,A,A,H,x)
w20(v,v,v,i,j,j,j,j,j,j,j,j,j,j,j,j,j,j,j,v)
// ICONS9: zip icon(20x15)
#define S9W 20
#define S9H 15
w20(v,d,H,U,H,H,H,H,H,H,H,H,H,J,u,u,u,o,v,v)
w20(c,U,J,J,J,J,J,J,J,J,J,J,U,A,N,A,N,A,v,v)
w20(v,v,v,i,i,i,i,i,i,i,i,i,v,i,o,T,I,c,v,v)
w20(v,v,T,N,N,N,N,N,N,N,N,N,c,v,x,A,J,v,v,v)
w20(v,v,o,J,J,J,J,J,J,J,J,T,x,v,i,N,J,v,v,v)
w20(v,v,T,A,A,A,A,A,A,A,A,A,c,v,i,N,J,v,v,v)
w20(v,v,o,J,J,J,J,J,J,J,J,T,x,v,i,N,J,v,v,v)
w20(v,v,T,N,A,A,N,N,N,N,N,N,c,v,i,N,J,v,v,v)
w20(v,v,i,o,o,o,o,o,o,o,o,o,v,x,j,I,d,i,v,v)
w20(o,T,T,I,I,I,T,T,T,T,T,T,T,A,N,A,A,H,v,v)
w20(x,J,N,A,A,N,N,N,N,N,N,N,N,H,I,T,T,u,v,v)
w20(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,U,I,v,v,v)
w20(v,v,v,v,v,v,v,v,v,v,v,v,v,o,u,I,I,o,c,v)
w20(v,v,v,v,v,v,v,v,v,v,v,v,x,N,N,A,A,N,U,v)
w20(v,v,v,v,v,v,v,v,v,v,v,v,v,c,c,c,c,c,j,v)
// ICONSA: battery 100% icon(22x14)
#define SAW 22
#define SAH 14
w22(v,v,u,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,u,x)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,x,A,T,x,v,v,v,x,x,v,v,v,x,x,v,v,v,x,d,A,i)
w22(v,j,A,I,v,u,u,d,i,v,u,u,u,v,i,d,u,o,v,u,A,i)
w22(u,A,N,I,v,N,M,M,o,i,M,M,M,i,u,M,M,A,v,u,A,i)
w22(d,N,N,I,v,A,N,N,o,i,N,N,N,i,o,N,N,H,v,u,A,i)
w22(u,N,N,I,v,A,N,N,o,i,N,N,N,i,o,N,N,H,v,u,A,i)
w22(u,N,N,I,v,A,N,N,o,i,N,N,N,i,o,N,N,H,v,u,A,i)
w22(d,N,N,I,v,A,N,N,o,i,N,N,N,i,o,N,N,H,v,u,A,i)
w22(u,A,N,I,v,A,M,M,o,i,M,M,N,i,u,M,N,H,v,u,A,i)
w22(v,j,A,I,v,o,o,o,x,v,o,o,o,v,i,u,o,c,v,o,A,i)
w22(v,x,A,T,i,x,x,x,i,i,x,x,x,i,i,x,x,x,i,d,A,i)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,v,o,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,o,x)
// ICONSB: battery  75% icon(22x14)
#define SBW 22
#define SBH 14
w22(v,v,u,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,u,x)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,x,A,T,x,x,x,x,x,x,v,v,v,x,x,v,v,v,x,d,A,i)
w22(v,j,A,I,v,v,v,v,v,x,u,u,u,v,i,d,u,o,v,u,A,i)
w22(u,A,N,I,v,v,v,v,v,j,M,M,M,i,u,M,M,A,v,u,A,i)
w22(d,N,N,I,v,v,v,v,v,j,N,N,N,i,o,N,N,H,v,u,A,i)
w22(u,N,N,I,v,v,v,v,v,j,N,N,N,i,o,N,N,H,v,u,A,i)
w22(u,N,N,I,v,v,v,v,v,j,N,N,N,i,o,N,N,H,v,u,A,i)
w22(d,N,N,I,v,v,v,v,v,j,N,N,N,i,o,N,N,H,v,u,A,i)
w22(u,A,N,I,v,v,v,v,v,j,M,M,N,i,u,M,N,H,v,u,A,i)
w22(v,j,A,I,v,v,v,v,v,v,o,o,o,v,i,u,o,c,v,u,A,i)
w22(v,x,A,T,i,i,i,i,i,i,x,x,x,i,i,x,x,x,i,d,A,i)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,v,o,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,o,x)
// ICONSC: battery  50% icon(22x14)
#define SCW 22
#define SCH 14
w22(v,v,u,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,u,x)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,x,A,T,x,x,x,x,x,x,x,x,x,x,x,v,v,v,x,d,A,i)
w22(v,j,A,I,v,v,v,v,v,v,v,v,v,v,i,d,u,o,v,u,A,i)
w22(u,A,N,I,v,v,v,v,v,v,v,v,v,v,u,M,M,A,v,u,A,i)
w22(d,N,N,I,v,v,v,v,v,v,v,v,v,v,u,N,N,H,v,u,A,i)
w22(u,N,N,I,v,v,v,v,v,v,v,v,v,v,u,N,N,H,v,u,A,i)
w22(u,N,N,I,v,v,v,v,v,v,v,v,v,v,u,N,N,H,v,u,A,i)
w22(d,N,N,I,v,v,v,v,v,v,v,v,v,v,u,N,N,H,v,u,A,i)
w22(u,A,N,I,v,v,v,v,v,v,v,v,v,v,u,M,N,H,v,u,A,i)
w22(v,j,A,I,v,v,v,v,v,v,v,v,v,v,i,u,o,c,v,u,A,i)
w22(v,x,A,T,i,i,i,i,i,i,i,i,i,i,i,x,x,x,i,d,A,i)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,v,o,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,o,x)
// ICONSD: battery  25% icon(22x14)
#define SDW 22
#define SDH 14
w22(v,v,u,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,u,x)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,x,A,T,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,d,A,i)
w22(v,j,A,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(u,A,N,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(d,N,N,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(u,N,N,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(u,N,N,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(d,N,N,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(u,A,N,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(v,j,A,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,i)
w22(v,x,A,T,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,d,A,i)
w22(v,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,A,i)
w22(v,v,o,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,u,o,x)
//(.align)
//0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/*------------------------------------------------------
	gryph data
------------------------------------------------------*/

static const u8 icon_l[] = {
// ICONL0: configuretion icon_L(32x24)
#define L0W 32
#define L0H 24
w32(v,v,v,v,x,i,i,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,x,d,U,H,H,U,u,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,j,H,N,A,A,A,N,N,J,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,c,I,J,H,A,N,A,A,N,J,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,v,v,x,j,u,H,N,A,N,c,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,v,v,v,v,v,u,N,A,N,T,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,v,v,v,v,v,x,A,N,N,U,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,v,v,v,v,v,i,A,N,N,A,u,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(I,T,u,j,x,v,i,H,N,N,N,N,M,U,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(o,N,N,A,H,J,H,N,N,N,N,N,N,M,M,J,j,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,T,N,A,N,N,N,N,N,N,N,N,N,N,N,M,N,I,i,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,I,A,N,N,N,M,A,H,M,N,N,N,N,N,N,M,A,d,x,v,i,o,u,o,j,v,v,v,v,v)
w32(v,v,v,j,I,T,T,d,i,v,d,A,M,N,N,N,N,N,N,M,H,T,A,N,N,N,A,T,x,v,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,i,T,N,M,N,N,N,N,N,N,N,N,A,A,A,A,N,U,x,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,j,J,N,M,N,N,N,N,N,A,N,A,A,N,N,N,T,x,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,o,H,M,N,N,N,N,N,T,x,j,u,I,J,H,j,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,A,N,N,N,T,v,v,v,v,v,v,x,i,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,N,N,N,o,v,v,v,v,v,v,v,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,c,N,N,N,J,v,v,v,v,v,v,v,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,H,N,A,A,o,x,v,v,v,v,v,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,N,A,A,A,H,J,I,u,c,v,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,I,N,A,A,A,A,N,N,J,x,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,u,H,A,A,A,H,d,v,v,v,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,c,u,o,i,v,v,v,v,v)
// ICONL1: control pad icon_L(30x22)
#define L1W 32
#define L1H 17
w32(v,o,M,M,M,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(o,M,M,M,M,M,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(M,M,M,M,M,M,N,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(M,M,M,M,M,M,N,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(M,M,M,M,M,N,N,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(o,M,M,M,N,N,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,o,o,o,v,v)
w32(v,o,N,N,N,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,c,c,c,v,v,v,o,M,M,M,o,v)
w32(v,v,v,v,v,v,v,v,v,v,v,v,v,j,j,j,v,v,v,c,N,N,N,c,v,v,o,M,M,M,o,v)
w32(v,v,o,o,o,v,v,v,v,v,v,v,j,A,A,A,j,v,v,c,N,N,N,c,v,v,v,o,o,o,v,v)
w32(v,v,J,T,I,v,v,v,v,v,v,v,j,A,A,A,j,v,v,v,c,c,c,v,v,v,v,v,v,v,v,v)
w32(v,v,J,T,I,v,v,v,v,v,v,v,v,j,j,j,v,v,v,v,v,v,v,v,v,v,v,o,M,M,M,o)
w32(v,v,J,T,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,c,N,N,N,c,v,v,M,M,M,M,M)
w32(v,v,J,T,I,v,v,v,v,v,v,v,v,j,A,A,A,j,v,v,N,N,N,N,N,v,v,M,M,M,M,M)
w32(v,v,J,T,I,v,v,v,v,v,v,v,v,A,A,A,A,A,v,v,N,N,N,N,N,v,v,o,M,M,M,o)
w32(v,v,J,T,I,v,v,v,v,v,v,v,v,A,A,A,A,A,v,v,c,N,N,N,c,v,v,v,v,v,v,v)
w32(v,v,J,T,I,v,v,v,v,v,v,v,v,j,A,A,A,j,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w32(v,v,u,d,o,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
// ICONL2: folder icon_L(28x20)
#define L2W 28
#define L2H 20
w28(v,v,v,I,A,A,A,A,A,A,U,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,u,A,I,d,d,d,d,d,A,T,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,c,N,o,v,v,v,v,v,v,j,N,u,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(x,H,d,v,v,v,v,v,v,v,v,u,N,T,I,I,I,I,I,I,I,I,I,I,j,v,v,v)
w28(i,A,i,v,v,v,v,v,v,v,v,v,d,J,J,J,J,J,J,J,J,J,J,N,u,v,v,v)
w28(i,H,i,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,U,u,v,v,v)
w28(i,H,i,v,c,U,J,J,J,J,J,J,J,J,J,J,J,J,J,J,J,J,J,A,H,J,J,j)
w28(i,H,i,v,I,M,N,N,N,N,N,M,M,M,M,M,N,N,N,N,N,N,N,A,A,N,A,i)
w28(i,H,i,v,U,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,A,J,v)
w28(i,A,i,x,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,N,I,v)
w28(i,A,x,o,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,N,o,v)
w28(i,H,i,I,N,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,A,i,v)
w28(i,H,j,U,N,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,U,v,v)
w28(i,H,o,A,A,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,N,T,v,v)
w28(i,H,T,A,A,A,A,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,N,u,v,v)
w28(i,H,H,A,A,A,A,A,N,N,N,N,N,N,N,N,N,N,N,A,A,A,A,A,A,j,v,v)
w28(i,A,A,A,A,A,A,A,A,N,N,N,N,N,N,N,N,N,A,A,A,A,A,A,H,v,v,v)
w28(i,A,A,A,A,A,A,A,A,A,A,N,N,N,N,N,A,A,A,A,A,A,A,N,T,v,v,v)
w28(i,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,M,d,v,v,v)
w28(v,u,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,x,v,v,v)
// ICONL3: CPS2 icon_L(32x15)
#define L3W 28
#define L3H 20
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,o,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,M,o,v,v,v,v,v,M,v,v,v,v,v,o,M,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,o,M,o,v,v,v,v,M,v,v,v,v,o,M,o,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,o,M,o,v,v,v,o,v,v,v,o,M,o,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,o,M,v,v,v,v,v,v,v,M,o,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,o,M,M,M,M,o,v,v,v,v,v,v,v,v,v,o,M,M,M,M,o,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,o,M,v,v,v,v,v,v,v,M,o,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,o,M,o,v,v,v,o,v,v,v,o,M,o,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,o,M,o,v,v,v,v,M,v,v,v,v,o,M,o,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,M,o,v,v,v,v,v,M,v,v,v,v,v,o,M,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,M,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,o,v,v,v,v,v,v,v,v,v,v,v,v,v)
w28(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
// ICONL4: Return to game icon_L(30x20)
#define L4W 30
#define L4H 20
w30(v,v,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,v,v,v,v,v,v,v,v,v)
w30(v,x,H,A,H,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,H,T,c,v,v,v,v,v,v)
w30(v,x,A,A,A,A,A,A,A,N,N,N,N,N,N,N,N,N,A,A,A,A,N,N,J,i,v,v,v,v)
w30(v,i,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,A,N,H,i,v,v,v)
w30(v,x,I,I,I,I,I,I,T,T,T,T,T,T,T,T,T,T,T,I,T,U,A,A,A,N,U,v,v,v)
w30(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,c,H,A,A,N,u,v,v)
w30(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,c,N,A,A,U,v,v)
w30(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,U,A,A,H,i,v)
w30(v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,I,N,A,A,j,v)
w30(v,v,v,v,v,v,c,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,T,N,A,A,j,v)
w30(v,v,v,v,v,T,J,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,H,A,A,H,x,v)
w30(v,v,v,i,U,M,J,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,d,N,A,N,J,v,v)
w30(v,v,c,H,N,N,J,v,v,v,v,v,v,v,v,v,v,v,v,v,x,j,I,N,A,A,A,c,v,v)
w30(v,u,A,N,A,N,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,A,N,A,A,N,I,v,v,v)
w30(d,N,A,A,A,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,N,T,v,v,v,v)
w30(v,U,N,A,A,A,N,N,N,N,N,M,M,M,M,M,N,N,N,N,N,N,N,H,o,v,v,v,v,v)
w30(v,v,J,N,A,A,H,T,T,T,T,T,T,T,T,T,T,T,T,T,I,d,o,x,v,v,v,v,v,v)
w30(v,v,v,I,N,N,T,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w30(v,v,v,v,u,N,J,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
w30(v,v,v,v,v,o,I,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v)
// ICONL5: exit to psp icon_L(30x17)
#define L5W 30
#define L5H 17
w30(v,v,v,v,x,j,j,j,j,c,c,c,c,c,c,c,c,c,c,c,j,j,j,j,j,x,v,v,v,v)
w30(v,v,v,d,A,A,N,N,N,N,N,N,N,M,M,M,N,N,N,N,N,N,N,N,A,A,d,v,v,v)
w30(v,v,T,N,A,A,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,I,A,A,N,T,v,v)
w30(v,o,N,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,N,o,v)
w30(v,U,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,U,v)
w30(j,A,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,A,j)
w30(u,N,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,N,u)
w30(I,N,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,N,I)
w30(T,N,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,N,T)
w30(I,N,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,N,I)
w30(u,N,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,N,u)
w30(c,A,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,A,c)
w30(v,U,A,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,A,U,v)
w30(v,u,N,A,A,A,x,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,x,A,A,A,N,u,v)
w30(v,v,J,N,A,A,u,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,u,A,A,N,J,v,v)
w30(v,v,x,T,A,N,N,N,N,N,M,M,M,M,M,M,M,M,M,N,N,N,N,N,N,A,T,x,v,v)
w30(v,v,v,v,i,c,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,o,c,i,v,v,v,v)
// ICONL6: dip switch icon_L(28x20)
#define L6W 28
#define L6H 20
w28(c,d,u,u,u,u,d,d,d,d,d,d,d,d,d,d,d,d,d,d,d,u,u,u,u,u,d,u)
w28(I,M,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,H)
w28(I,N,A,A,A,A,A,A,A,N,N,N,N,N,A,N,N,A,A,A,A,A,A,A,A,A,A,U)
w28(I,N,T,o,o,o,d,N,H,c,c,c,o,H,N,d,o,o,o,T,N,T,o,o,o,d,A,U)
w28(I,M,u,I,H,U,o,N,U,v,v,v,v,H,N,o,U,A,T,I,M,u,I,H,U,o,A,U)
w28(I,M,d,J,N,A,u,N,U,v,v,v,v,H,N,u,A,M,J,I,M,d,J,N,H,u,A,U)
w28(I,M,d,J,N,A,u,N,U,v,v,v,v,H,N,u,A,M,J,I,M,d,J,N,H,u,A,U)
w28(I,M,d,j,o,c,c,N,U,v,v,v,v,H,N,c,c,o,j,I,M,d,j,o,c,c,A,U)
w28(I,M,d,v,v,v,j,N,U,v,v,v,v,H,N,j,v,v,v,I,M,u,v,v,v,j,A,U)
w28(I,M,d,v,v,v,j,N,U,v,v,v,v,H,N,j,v,v,v,I,M,d,v,v,v,j,A,U)
w28(I,M,d,v,v,v,j,N,U,v,v,v,v,H,N,j,v,v,v,I,M,d,v,v,v,j,A,U)
w28(I,M,d,v,v,v,j,N,U,v,v,v,v,H,N,j,v,v,v,I,M,d,v,v,v,j,A,U)
w28(I,M,d,v,v,v,j,N,U,v,v,v,v,H,N,j,v,v,v,I,M,u,v,v,v,j,A,U)
w28(I,M,d,v,v,v,j,N,J,u,A,H,o,U,N,j,v,v,v,I,M,u,v,v,v,j,A,U)
w28(I,M,u,v,v,v,j,N,J,d,M,M,u,U,N,j,v,v,v,I,M,u,v,v,v,j,A,U)
w28(I,M,u,v,v,v,j,N,J,d,M,M,u,U,N,j,v,v,v,I,M,u,v,v,v,j,A,U)
w28(I,M,d,v,v,v,c,N,U,j,u,u,j,H,N,j,v,v,v,I,M,d,v,v,v,c,A,U)
w28(I,N,H,U,U,H,H,A,A,U,U,U,U,A,N,H,H,H,H,A,N,H,U,U,U,H,A,U)
w28(I,N,A,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,A,N,H)
w28(u,U,J,J,J,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,J,J,J,J,J,U,T)
//(.align)
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

#undef v
#undef x
#undef i
#undef j
#undef c
#undef o
#undef u
#undef d
#undef I
#undef T
#undef J
#undef U
#undef H
#undef A
#undef N
#undef M

#undef f
#undef w20
#undef w22
#undef w24
#undef w28
#undef w30
#undef w32

#define ICONS0 (0)
#define ICONS1 (ICONS0+((S0W*S0H)/2))
#define ICONS2 (ICONS1+((S1W*S1H)/2))
#define ICONS3 (ICONS2+((S2W*S2H)/2))
#define ICONS4 (ICONS3+((S3W*S3H)/2))
#define ICONS5 (ICONS4+((S4W*S4H)/2))
#define ICONS6 (ICONS5+((S5W*S5H)/2))
#define ICONS7 (ICONS6+((S6W*S6H)/2))
#define ICONS8 (ICONS7+((S7W*S7H)/2))
#define ICONS9 (ICONS8+((S8W*S8H)/2))
#define ICONSA (ICONS9+((S9W*S9H)/2))
#define ICONSB (ICONSA+((SAW*SAH)/2))
#define ICONSC (ICONSB+((SBW*SBH)/2))
#define ICONSD (ICONSC+((SCW*SCH)/2))

static const int icon_s_pos[NUM_FONTS] = {
ICONS0,//0x000000,	// ICONS0:
ICONS1,//0x0000d8,	// ICONS1: //((24*18)/2)
ICONS2,//0x000188,	// ICONS2:
ICONS3,//0x00021e,	// ICONS3:
ICONS4,//0x0002ae,	// ICONS4:
ICONS5,//0x000353,	// ICONS5:
ICONS6,//0x0003d7,	// ICONS6:
ICONS7,//0x00046d,	// ICONS7:
ICONS8,//0x0004f9,	// ICONS8:
ICONS9,//0x00057b,	// ICONS9:
ICONSA,//0x000611,	// ICONSA:
ICONSB,//0x0006ab,	// ICONSB:
ICONSC,//0x000745,	// ICONSC:
ICONSD //0x0007df	// ICONSD:
};

#define ICONL0 (0)
#define ICONL1 (ICONL0+((L0W*L0H)/2))
#define ICONL2 (ICONL1+((L1W*L1H)/2))
#define ICONL3 (ICONL2+((L2W*L2H)/2))
#define ICONL4 (ICONL3+((L3W*L3H)/2))
#define ICONL5 (ICONL4+((L4W*L4H)/2))
#define ICONL6 (ICONL5+((L5W*L5H)/2))

static const int icon_l_pos[NUM_FONTL] = {
ICONL0,//0x000000,	// ICONL0: configuretion icon_L(32x24)
ICONL1,//0x000180,	// ICONL1: control pad icon_L(30x22)
ICONL2,//0x0002ca,	// ICONL2: folder icon_L(28x20)
ICONL3,//0x0003e2,	// ICONL3: CPS2 icon_L(32x15)
ICONL4,//0x0004d2,	// ICONL4: Return to game icon_L(30x20)
ICONL5,//0x0005fe,	// ICONL5: exit to psp icon_L(30x17)
ICONL6 //0x0006fd	// ICONL6: dip switch icon_L(28x20)
};




static const s8 icon_s_width[NUM_FONTS] = {
//	24,22,20,24,22,22,20,20,20,20,22,22,22,22
S0W,S1W,S2W,S3W,S4W,S5W,S6W,S7W,S8W,S9W,SAW,SBW,SCW,SDW
};

static const s8 icon_s_height[NUM_FONTS] = {
//	18,16,15,12,15,12,15,14,13,15,14,14,14,14
S0H,S1H,S2H,S3H,S4H,S5H,S6H,S7H,S8H,S9H,SAH,SBH,SCH,SDH
};

static const s8 icon_s_skipx[NUM_FONTS] = {
	 0, /*1*/0, 2, /*0*/2, 1, 1, 2, 2, 2, 3, 1, 1, 1, 1
};

static const s8 icon_s_skipy[NUM_FONTS] = {
	 0, /*1*/3, 1, /*3*/1, 2, 3, 2, 2, 3, 2, 2, 2, 2, 2
};

static const s8 icon_l_width[NUM_FONTL] = {
//	32,30,28,32,30,30,28
L0W,L1W,L2W,L3W,L4W,L5W,L6W
};

static const s8 icon_l_height[NUM_FONTL] = {
//	24,22,20,15,20,17,20
L0H,L1H,L2H,L3H,L4H,L5H,L6H
};

static const s8 icon_l_skipx[NUM_FONTL] = {
	 1, /*1*/1, 2, /*0*/2, 2, 1, 2
};

static const s8 icon_l_skipy[NUM_FONTL] = {
	 0, /*1*/4, 2, /*5*/2, 2, 4, 2
};


/*------------------------------------------------------
	functions
------------------------------------------------------*/

int icon_s_get_gryph(struct font_t *font, u16 code)
{
	if (code < NUM_FONTS)
	{
		font->data   = &icon_s[icon_s_pos[code]];
		font->width  = icon_s_width[code];
		font->height = icon_s_height[code];
		font->pitch  = 32;
		font->skipx  = icon_s_skipx[code];
		font->skipy  = icon_s_skipy[code];
		return 1;
	}
	return 0;
}

int icon_l_get_gryph(struct font_t *font, u16 code)
{
	if (code < NUM_FONTL)
	{
		font->data   = &icon_l[icon_l_pos[code]];
		font->width  = icon_l_width[code];
		font->height = icon_l_height[code];
		font->pitch  = 32;
		font->skipx  = icon_l_skipx[code];
		font->skipy  = icon_l_skipy[code];
		return 1;
	}
	return 0;
}


#undef ICONS0
#undef ICONS1
#undef ICONS2
#undef ICONS3
#undef ICONS4
#undef ICONS5
#undef ICONS6
#undef ICONS7
#undef ICONS8
#undef ICONS9
#undef ICONSA
#undef ICONSB
#undef ICONSC
#undef ICONSD

#undef ICONL0
#undef ICONL1
#undef ICONL2
#undef ICONL3
#undef ICONL4
#undef ICONL5
#undef ICONL6

