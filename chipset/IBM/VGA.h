/* ----------------------------------------------------------------------------
**	VGA register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: VGA.h,v $
**	Revision 1.2  2003/03/25 03:38:25  fspacek
**	- vga driver update, todo: planar modes (4-bit, 2-bit, mode-X) will require
**	  support in display-kgi, adjust virtual res to accommodate restrictions
**	  on the stride
**	
**	Revision 1.1.1.1  2000/04/18 08:51:20  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:20  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _IBM_vga_h
#define _IBM_vga_h

#define	VGA_IO_BASE		0x3C0
#define	VGA_IO_SIZE		32
#define	VGA_MEM_BASE		0x000a0000
#define	VGA_MEM_SIZE		(128 KB)
#define	VGA_TEXT_MEM_BASE      	0x000b8000
#define	VGA_TEXT_MEM_SIZE      	(32 KB)

#define	VGA_MISCr	0x0C
#define	VGA_MISCw	0x02
#	define	VGA_MISC_COLOR_IO	0x01
#	define	VGA_MISC_ENB_RAM	0x02
#	define	VGA_MISC_CLOCK_MASK	0x0C
#	define	VGA_MISC_VIDEO_OFF	0x10
#	define	VGA_MISC_PAGE_SEL	0x20
#	define	VGA_MISC_SYNC_MASK	0xC0
#	define	VGA_MISC_NEG_HSYNC	0x40
#	define	VGA_MISC_NEG_VSYNC	0x80
#	define	VGA_MISC_25MHZ_CLK	0x00
#	define	VGA_MISC_28MHZ_CLK	0x04
#	define	VGA_MISC_350_LINES	VGA_MISC_NEG_HSYNC
#	define	VGA_MISC_400_LINES	VGA_MISC_NEG_VSYNC
#	define	VGA_MISC_480_LINES	VGA_MISC_SYNC_MASK

#define VGA_FCTRLr	0x0A
#define VGA_FCTRLw	0x1A

#define	VGA_INS0r	0x02
#	define	VGA_INS0_VRETRACE	0x80

#define	VGA_INS1r	0x01A
#	define	VGA_INS1_DISP_OFF	0x01
#	define	VGA_INS1_LIGHTPEN_T	0x02
#	define	VGA_INS1_LIGHTPEN_S	0x04
#	define	VGA_INS1_VRETRACE	0x08
#	define	VGA_INS1_DIAGNOSTIC	0x30

#define VGA_SEQ_INDEX	0x04
#define	VGA_SEQ_DATA	0x05
#	define	VGA_SEQ_RESET	0x00
#		define	VGA_SR00_ASYNCRESET	0x01
#		define	VGA_SR00_SYNCRESET	0x02
#		define	VGA_SR00_RESERVED	0xFC
#	define	VGA_SEQ_CLOCK	0x01
#		define	VGA_SR01_8DOT_CHCLK	0x01
#		define	VGA_SR01_3CPUCYCLES	0x02
#		define	VGA_SR01_SHIFTTWO	0x04
#		define	VGA_SR01_DCLKBY2	0x08
#		define	VGA_SR01_SHIFTFOUR	0x10
#		define	VGA_SR01_DISPLAY_OFF	0x20
#	define	VGA_SEQ_WPLANE	0x02
#		define	VGA_SR02_PLANE_MASK	0x0F
#		define	VGA_SR02_PLANE0		0x01
#		define	VGA_SR02_PLANE1		0x02
#		define	VGA_SR02_PLANE2		0x04
#		define	VGA_SR02_PLANE3		0x08
#	define	VGA_SEQ_FONT	0x03
#		define	VGA_SR03_FONTA_MASK	0x2C
#		define	VGA_SR03_FONTB_MASK	0x13
#	define	VGA_SEQ_MEMMODE	0x04
#		define	VGA_SR04_256K_ACCESS	0x02
#		define	VGA_SR04_NO_ODDEVEN	0x04
#		define	VGA_SR04_CHAINED	0x08

#define VGA_CRT_INDEX	0x014
#define	VGA_CRT_DATA	0x015
#	define	VGA_CRT_HTOTAL		0x00
#	define	VGA_CRT_HDISPLAYEND	0x01
#	define	VGA_CRT_HBLANKSTART	0x02
#	define	VGA_CRT_HBLANKEND	0x03
#		define	VGA_CR03_HBE_MASK		0x1F
#		define	VGA_CR03_DSKEW_MASK		0x60
#		define	VGA_CR03_IS_VGA			0x80
#	define	VGA_CRT_HSYNCSTART	0x04
#	define	VGA_CRT_HSYNCEND	0x05
#		define	VGA_CR05_HSE_MASK		0x1F
#		define	VGA_CR05_HSKEW_MASK		0x60
#		define	VGA_CR05_HBLANKEND_B5		0x80
#	define	VGA_CRT_VTOTAL		0x06
#	define	VGA_CRT_OVERFLOW	0x07
#		define	VGA_CR07_VTOTAL_B8		0x01
#		define	VGA_CR07_VDISPLAYEND_B8		0x02
#		define	VGA_CR07_VSYNCSTART_B8		0x04
#		define	VGA_CR07_VBLANKSTART_B8		0x08
#		define	VGA_CR07_LINECOMPARE_B8		0x10
#		define	VGA_CR07_VTOTAL_B9		0x20
#		define	VGA_CR07_VDISPLAYEND_B9		0x40
#		define	VGA_CR07_VSYNCSTART_B9		0x80
#	define	VGA_CRT_PRESETROW	0x08
#		define	VGA_CR08_ROWSCAN_MASK		0x1F
#		define	VGA_CR08_BYTEPAN_MASK		0x60
#	define	VGA_CRT_CHARHEIGHT	0x09
#		define	VGA_CR09_CHARHEIGHT_MASK	0x1F
#		define	VGA_CR09_VBLANKSTART_B9		0x20
#		define	VGA_CR09_LINECOMPARE_B9		0x40
#		define	VGA_CR09_DOUBLESCAN		0x80
#	define	VGA_CRT_CURSORSTART	0x0A
#		define	VGA_CR0A_CURSOR_START_MASK	0x1F
#		define	VGA_CR0A_CURSOR_OFF		0x20
#	define	VGA_CRT_CURSOREND	0x0B
#		define	VGA_CR0B_CURSOR_END_MASK	0x1F
#		define	VGA_CR0B_CURSOR_SKEW_MASK	0x60
#	define	VGA_CRT_STARTADDR_H	0x0C
#	define	VGA_CRT_STARTADDR_L	0x0D
#	define	VGA_CRT_CURSORADDR_H	0x0E
#	define	VGA_CRT_CURSORADDR_L	0x0F
#	define	VGA_CRT_VSYNCSTART	0x10
#	define	VGA_CRT_VSYNCEND	0x11
#		define	VGA_CR11_VSYNCEND_MASK		0x0F
#		define	VGA_CR11_CLEAR_VSYNC_IRQ	0x10
#		define	VGA_CR11_DISABLE_VSYNC_IRQ	0x20
#		define	VGA_CR11_5_DRAMCYCLES		0x40
#		define	VGA_CR11_LOCKTIMING		0x80
#	define	VGA_CRT_VDISPLAYEND	0x12
#	define	VGA_CRT_LOGICALWIDTH	0x13
#		define	VGA_CR13_LOGICALWIDTH_MASK	0xFF
#	define	VGA_CRT_UNDERLINE	0x14
#		define	VGA_CR14_UNDERLINE_MASK	0x1F
#		define	VGA_CR14_MEMCOUNT_BY_4	0x20
#		define	VGA_CR14_DOUBLEWORDMODE	0x40
#	define	VGA_CRT_VBLANKSTART	0x15
#	define	VGA_CRT_VBLANKEND	0x16
#	define	VGA_CRT_MODE		0x17
#		define	VGA_CR17_CGA_BANKING		0x01
#		define	VGA_CR17_HGC_BANKING		0x02
#		define	VGA_CR17_VDOUBLE_TIMING		0x04
#		define	VGA_CR17_WORDMODE		0x08
#		define	VGA_CR17_CGA_ADDR_WRAP		0x20
#		define	VGA_CR17_BYTE_MODE		0x40
#		define	VGA_CR17_ENABLE_SYNC		0x80
#	define	VGA_CRT_LINECOMPARE	0x18
#	define	VGA_CRT_CPULATCH	0x22
#	define	VGA_CRT_ATTRI		0x24

#define	VGA_GRC_INDEX	0x0E
#define	VGA_GRC_DATA	0x0F
#	define	VGA_GRC_SET_RESET	0x00
#		define	VGA_GR0_DATA_MASK	0x0F
#	define	VGA_GRC_ENABLE	0x01
#		define	VGA_GR1_DATA_MASK	0x0F
#	define	VGA_GRC_COLOR_CMP	0x02
#		define	VGA_GR2_DATA_MASK	0x0F
#	define	VGA_GRC_OPERATION	0x03
#		define	VGA_GR3_ROTATE_MASK	0x07
#		define	VGA_GR3_NO_OP	0x00
#		define	VGA_GR3_AND_OP	0x08
#		define	VGA_GR3_OR_OP	0x10
#		define	VGA_GR3_XOR_OP	0x18
#	define	VGA_GRC_READPLANE	0x04
#		define	VGA_GR4_PLANEMASK	0x03
#	define	VGA_GRC_MODECONTROL	0x05
#		define	VGA_GR5_WRITEMODE0	0x00
#		define	VGA_GR5_WRITEMODE1	0x01
#		define	VGA_GR5_WRITEMODE2	0x02
#		define	VGA_GR5_WRITEMODE3	0x03
#		define	VGA_GR5_ENABLE_CMP	0x08
#		define	VGA_GR5_ODD_EVEN	0x10
#		define	VGA_GR5_ODD_EVEN_SHIFT	0x20
#		define	VGA_GR5_8BIT_COLOR	0x40
#	define	VGA_GRC_MMAP_MODE	0x06
#		define	VGA_GR6_GRAPHMODE	0x01
#		define	VGA_GR6_CHAIN_OE	0x02
#		define	VGA_GR6_MAP_A0_128	0x00
#		define	VGA_GR6_MAP_A0_64	0x04
#		define	VGA_GR6_MAP_B0_32	0x08
#		define	VGA_GR6_MAP_B8_32	0x0C
#		define	VGA_GR6_MAP_MASK	0x0C
#	define	VGA_GRC_DONT_CARE	0x07
#		define	VGA_GR7_DATA_MASK	0x0F
#	define	VGA_GRC_BITMASK	0x08

#define	VGA_ATC_AFF	0x1A
#define	VGA_ATC_INDEX	0x00
#	define	VGA_ATCI_ENABLE_DISPLAY	0x20
#define	VGA_ATC_DATAr	0x01
#define	VGA_ATC_DATAw	0x00
#	define	VGA_ATC_LAST_PALETTE 0x0F
#	define	VGA_ATC_MODECONTROL	0x10
#		define	VGA_AR10_GRAPHICS	0x01
#		define	VGA_AR10_MONOMODE	0x02
#		define	VGA_AR10_9DOT_CHARS	0x04
#		define	VGA_AR10_ENB_BLINK	0x08
#		define	VGA_AR10_TOP_PAN	0x20
#		define	VGA_AR10_8BIT_MODE	0x40
#		define	VGA_AR10_SELECT_B54	0x80
#	define	VGA_ATC_BORDER	0x11
#	define	VGA_ATC_PLANEENABLE	0x12
#		define	VGA_AR12_PLANEMASK	0x0F
#		define	VGA_AR12_VTESTMASK	0x30
#	define	VGA_ATC_HPAN	0x13
#		define	VGA_AR13_DATA_MASK	0x0F
#	define	VGA_ATC_PIXELPAD	0x14
#		define	VGA_AR14_DATA_MASK	0x0F

#define VGA_DAC_BASE	0x06
#define	VGA_DAC_MASK	0x06
#define	VGA_DAC_RD_ADDR	0x07		/* write only */
#define	VGA_DAC_STATUS	0x07		/* read only  */
#define	VGA_DAC_WR_ADDR	0x08
#define	VGA_DAC_DATA	0x09

#endif	/* #ifndef _IBM_vga_h */
