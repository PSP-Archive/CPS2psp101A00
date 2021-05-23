/******************************************************************************

	shadow.c

	Box shadow data

******************************************************************************/

const unsigned char shadow[9][8][4] = {
	{	//	00 - left top
		{ 0x00,0x11,0x22,0x22 },
		{ 0x10,0x22,0x33,0x44 },
		{ 0x21,0x32,0x54,0x66 },
		{ 0x21,0x53,0x76,0x88 },
		{ 0x32,0x64,0x97,0xaa },
		{ 0x32,0x75,0xa9,0xcb },
		{ 0x42,0x86,0xba,0xdc },
		{ 0x42,0x86,0xca,0xed }
	},
	{	//	01 - top
		{ 0x22,0x22,0x22,0x22 },
		{ 0x44,0x44,0x44,0x44 },
		{ 0x66,0x66,0x66,0x66 },
		{ 0x98,0x99,0x99,0x89 },
		{ 0xbb,0xbb,0xbb,0xbb },
		{ 0xcc,0xdd,0xdd,0xcc },
		{ 0xee,0xee,0xee,0xee },
		{ 0xfe,0xff,0xff,0xef }
	},
	{	//	02 - right top
		{ 0x22,0x22,0x11,0x00 },
		{ 0x44,0x33,0x22,0x01 },
		{ 0x66,0x45,0x23,0x12 },
		{ 0x88,0x67,0x35,0x12 },
		{ 0xaa,0x79,0x46,0x23 },
		{ 0xbc,0x9a,0x57,0x23 },
		{ 0xcd,0xab,0x68,0x24 },
		{ 0xde,0xac,0x68,0x24 }
	},
	{	//	03 - left
		{ 0x42,0x86,0xcb,0xee },
		{ 0x42,0x96,0xcb,0xfe },
		{ 0x42,0x96,0xdb,0xfe },
		{ 0x42,0x96,0xdb,0xfe },
		{ 0x42,0x96,0xdb,0xfe },
		{ 0x42,0x96,0xdb,0xfe },
		{ 0x42,0x96,0xcb,0xfe },
		{ 0x42,0x86,0xcb,0xee }
	},
	{	//	04 - center
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff },
		{ 0xff,0xff,0xff,0xff }
	},
	{	//	05 - right
		{ 0xee,0xbc,0x68,0x24 },
		{ 0xef,0xbc,0x69,0x24 },
		{ 0xef,0xbd,0x69,0x24 },
		{ 0xef,0xbd,0x69,0x24 },
		{ 0xef,0xbd,0x69,0x24 },
		{ 0xef,0xbd,0x69,0x24 },
		{ 0xef,0xbc,0x69,0x24 },
		{ 0xee,0xbc,0x68,0x24 }
	},
	{	//	06 - left bottom
		{ 0x42,0x86,0xca,0xed },
		{ 0x42,0x86,0xba,0xdc },
		{ 0x32,0x75,0xa9,0xcb },
		{ 0x32,0x64,0x97,0xaa },
		{ 0x21,0x53,0x76,0x88 },
		{ 0x21,0x32,0x54,0x66 },
		{ 0x10,0x22,0x33,0x44 },
		{ 0x00,0x11,0x22,0x22 }
	},
	{	//	07 - bottom
		{ 0xfe,0xff,0xff,0xef },
		{ 0xee,0xee,0xee,0xee },
		{ 0xcc,0xdd,0xdd,0xcc },
		{ 0xbb,0xbb,0xbb,0xbb },
		{ 0x98,0x99,0x99,0x89 },
		{ 0x66,0x66,0x66,0x66 },
		{ 0x44,0x44,0x44,0x44 },
		{ 0x22,0x22,0x22,0x22 }
	},
	{	//	08 - right bottom
		{ 0xde,0xac,0x68,0x24 },
		{ 0xcd,0xab,0x68,0x24 },
		{ 0xbc,0x9a,0x57,0x23 },
		{ 0xaa,0x79,0x46,0x23 },
		{ 0x88,0x67,0x35,0x12 },
		{ 0x66,0x45,0x23,0x12 },
		{ 0x44,0x33,0x22,0x01 },
		{ 0x22,0x22,0x11,0x00 }
	}
};
