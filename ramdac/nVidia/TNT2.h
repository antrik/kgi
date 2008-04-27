/* -----------------------------------------------------------------------------
**	nVidia Riva TNT2 ramdac register definitions
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: TNT2.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:06  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:06  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_nVidia_TNT2_h
#define	_ramdac_nVidia_TNT2_h

#warning put real TNT2 ramdac register definitions here! -- seeger_s

#define	TNT2_DAC_PW_INDEX	0x0
#define	TNT2_DAC_P_DATA		0x1
#define	TNT2_DAC_PIXEL_MASK	0x2
#define	TNT2_DAC_PR_INDEX	0x3

#define	TNT2_DAC_EXT_ADDR	0x0
#define	TNT2_DAC_CW_INDEX	0x4
#define	TNT2_DAC_C_DATA		0x5
/*
**	reserved	0x6..0x9
*/
#define	TNT2_DAC_EXT_DATA	0xA
#define	TNT2_DAC_CURSOR_RAM_DATA	0xB
#define	TNT2_DAC_CURSOR_XL	0xC
#define	TNT2_DAC_CURSOR_XH	0xD
#define	TNT2_DAC_CURSOR_YL	0xE
#define	TNT2_DAC_CURSOR_YH	0xF

/*
**	indexed registers
*/

/*
**	reserved				0x00 ... 0x05
*/
#define	TNT2_EDAC_CursorControl		0x06
#	define	TNT2_EDAC06_CursorOff			0x00
#	define	TNT2_EDAC06_CursorThreeColor		0x01
#	define	TNT2_EDAC06_CursorXGA			0x02
#	define	TNT2_EDAC06_CursorX			0x03
#	define	TNT2_EDAC06_CursorMask			0x03
	
#	define	TNT2_EDAC06_CursorAddr8			0x04
#	define	TNT2_EDAC06_CursorAddr9			0x08

#	define	TNT2_EDAC06_CursorSelectMask		0x30
#	define	TNT2_EDAC06_CursorSelectShift		4

#	define	TNT2_EDAC06_CursorSizeMask		0x40
#	define	TNT2_EDAC06_Cursor32x32			0x00
#	define	TNT2_EDAC06_Cursor64x64			0x40
/*
**	registers reservd			0x07 ... 0x17
*/
#define	TNT2_EDAC_ColorMode			0x18
#	define	TNT2_EDAC18_PixelFormatMask		0x0F
#	define	TNT2_EDAC18_I8				0x00
#	define	TNT2_EDAC18_R3G3B2			0x01
#	define	TNT2_EDAC18_R2G3B2_Offset		0x02
#	define	TNT2_EDAC18_R2G3B2A1			0x03
#	define	TNT2_EDAC18_R5G5B5A1			0x04
#	define	TNT2_EDAC18_R4G4B4A4			0x05
#	define	TNT2_EDAC18_R5G6B5			0x06
#	define	TNT2_EDAC18_R8G8B8A8			0x08
#	define	TNT2_EDAC18_R8G8B8			0x09

#	define	TNT2_EDAC18_GUI				0x10
#	define	TNT2_EDAC18_SVGA				0x00

#	define	TNT2_EDAC18_RGB				0x20
#	define	TNT2_EDAC18_TrueColor			0x80
#	define	TNT2_EDAC18_DirectColor			0x00

#define	TNT2_EDAC_ModeControl			0x19
#	define	TNT2_EDAC19_FrontBuffer			0x00
#	define	TNT2_EDAC19_BackBuffer			0x01
#	define	TNT2_EDAC19_StaticDoubleBuffer		0x02
#	define	TNT2_EDAC19_DynamicDoubleBuffer		0x04
/*
**	register reserved			0x1A ... 0x1B
*/
#define	TNT2_EDAC_PalettePage			0x1C
/*	register reserved			0x1D
*/
#define	TNT2_EDAC_MiscControl			0x1E
#	define	TNT2_EDAC1E_PowerDown			0x01
#	define	TNT2_EDAC1E_8BitEnable			0x02
#	define	TNT2_EDAC1E_HSyncInvert			0x04
#	define	TNT2_EDAC1E_VSyncInvert			0x08
#	define	TNT2_EDAC1E_BlankPedestral		0x10
#	define	TNT2_EDAC1E_SyncOnGreen			0x20


#define	TNT2_EDAC_PClkA_M			0x20
#define	TNT2_EDAC_PClkA_N			0x21
#define	TNT2_EDAC_PClkA_P			0x22
#	define	TNT2_EDAC_PClkP_Enable			0x08

#define	TNT2_EDAC_PClkB_M			0x23
#define	TNT2_EDAC_PClkB_N			0x24
#define	TNT2_EDAC_PClkB_P			0x25

#define	TNT2_EDAC_PClkC_M			0x26
#define	TNT2_EDAC_PClkC_N			0x27
#define	TNT2_EDAC_PClkC_P			0x28

#define	TNT2_EDAC_PClkStatus			0x29
#	define	TNT2_EDAC29_PLLlocked			0x10

#define	TNT2_EDAC_MClk_M				0x30
#define	TNT2_EDAC_MClk_N				0x31
#define	TNT2_EDAC_MClk_P				0x32

#define	TNT2_EDAC_MClkStatus			0x33
#	define	TNT2_EDAC33_PLLlocked			0x10

#define	TNT2_EDAC_ColorKeyControl		0x40
#	define	TNT2_EDAC40_OverlayEnable		0x01
#	define	TNT2_EDAC40_RedEnable			0x02
#	define	TNT2_EDAC40_GreenEnable			0x04
#	define	TNT2_EDAC40_BlueEnable			0x08
#	define	TNT2_EDAC40_TestTrue			0x00
#	define	TNT2_EDAC40_TestFalse			0x10

#define	TNT2_EDAC_OverlayKey			0x41
#define	TNT2_EDAC_RedKey				0x42
#define	TNT2_EDAC_GreenKey			0x43
#define	TNT2_EDAC_BlueKey			0x44

#endif	/* #ifdef _ramdac_nVidia_TNT2_h */
