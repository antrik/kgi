/* -----------------------------------------------------------------------------
**	S3 ViRGE ramdac register definitions
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	Jon Taylor
**			2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: ViRGE.h,v $
**	Revision 1.1.1.1  2000/08/04 11:57:50  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/08/04 11:57:50  seeger_s
**	- merged code posted by Jos Hulzink to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- driver status 30% is reported by Jos Hulzink
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_S3_VIRGE_h
#define	_ramdac_S3_VIRGE_h

#if 0

#define	VIRGE_DAC_PW_INDEX	0x0
#define	VIRGE_DAC_P_DATA		0x1
#define	VIRGE_DAC_PIXEL_MASK	0x2
#define	VIRGE_DAC_PR_INDEX	0x3

#define	VIRGE_DAC_EXT_ADDR	0x0
#define	VIRGE_DAC_CW_INDEX	0x4
#define	VIRGE_DAC_C_DATA		0x5
/*
**	reserved	0x6..0x9
*/
#define	VIRGE_DAC_EXT_DATA	0xA
#define	VIRGE_DAC_CURSOR_RAM_DATA	0xB
#define	VIRGE_DAC_CURSOR_XL	0xC
#define	VIRGE_DAC_CURSOR_XH	0xD
#define	VIRGE_DAC_CURSOR_YL	0xE
#define	VIRGE_DAC_CURSOR_YH	0xF

/*
**	indexed registers
*/

/*
** registers reserved				0x00 ... 0x05
*/
#define	VIRGE_EDAC_CursorControl		0x06
#	define	VIRGE_EDAC06_CursorOff			0x00
#	define	VIRGE_EDAC06_CursorThreeColor		0x01
#	define	VIRGE_EDAC06_CursorXGA			0x02
#	define	VIRGE_EDAC06_CursorX			0x03
#	define	VIRGE_EDAC06_CursorMask			0x03
	
#	define	VIRGE_EDAC06_CursorAddr8			0x04
#	define	VIRGE_EDAC06_CursorAddr9			0x08

#	define	VIRGE_EDAC06_CursorSelectMask		0x30
#	define	VIRGE_EDAC06_CursorSelectShift		4

#	define	VIRGE_EDAC06_CursorSizeMask		0x40
#	define	VIRGE_EDAC06_Cursor32x32			0x00
#	define	VIRGE_EDAC06_Cursor64x64			0x40
/*
**	registers reservd			0x07 ... 0x17
*/
#define	VIRGE_EDAC_ColorMode			0x18
#	define	VIRGE_EDAC18_PixelFormatMask		0x0F
#	define	VIRGE_EDAC18_I8				0x00
#	define	VIRGE_EDAC18_R3G3B2			0x01
#	define	VIRGE_EDAC18_R2G3B2_Offset		0x02
#	define	VIRGE_EDAC18_R2G3B2A1			0x03
#	define	VIRGE_EDAC18_R5G5B5A1			0x04
#	define	VIRGE_EDAC18_R4G4B4A4			0x05
#	define	VIRGE_EDAC18_R5G6B5			0x06
#	define	VIRGE_EDAC18_R8G8B8A8			0x08
#	define	VIRGE_EDAC18_R8G8B8			0x09

#	define	VIRGE_EDAC18_GUI				0x10
#	define	VIRGE_EDAC18_SVGA				0x00

#	define	VIRGE_EDAC18_RGB				0x20
#	define	VIRGE_EDAC18_TrueColor			0x80
#	define	VIRGE_EDAC18_DirectColor			0x00

#define	VIRGE_EDAC_ModeControl			0x19
#	define	VIRGE_EDAC19_FrontBuffer			0x00
#	define	VIRGE_EDAC19_BackBuffer			0x01
#	define	VIRGE_EDAC19_StaticDoubleBuffer		0x02
#	define	VIRGE_EDAC19_DynamicDoubleBuffer		0x04
/*
**	register reserved			0x1A ... 0x1B
*/
#define	VIRGE_EDAC_PalettePage			0x1C
/*	register reserved			0x1D
*/
#define	VIRGE_EDAC_MiscControl			0x1E
#	define	VIRGE_EDAC1E_PowerDown			0x01
#	define	VIRGE_EDAC1E_8BitEnable			0x02
#	define	VIRGE_EDAC1E_HSyncInvert			0x04
#	define	VIRGE_EDAC1E_VSyncInvert			0x08
#	define	VIRGE_EDAC1E_BlankPedestral		0x10
#	define	VIRGE_EDAC1E_SyncOnGreen			0x20


#define	VIRGE_EDAC_PClkA_M			0x20
#define	VIRGE_EDAC_PClkA_N			0x21
#define	VIRGE_EDAC_PClkA_P			0x22
#	define	VIRGE_EDAC_PClkP_Enable			0x08

#define	VIRGE_EDAC_PClkB_M			0x23
#define	VIRGE_EDAC_PClkB_N			0x24
#define	VIRGE_EDAC_PClkB_P			0x25

#define	VIRGE_EDAC_PClkC_M			0x26
#define	VIRGE_EDAC_PClkC_N			0x27
#define	VIRGE_EDAC_PClkC_P			0x28

#define	VIRGE_EDAC_PClkStatus			0x29
#	define	VIRGE_EDAC29_PLLlocked			0x10

#define	VIRGE_EDAC_MClk_M				0x30
#define	VIRGE_EDAC_MClk_N				0x31
#define	VIRGE_EDAC_MClk_P				0x32

#define	VIRGE_EDAC_MClkStatus			0x33
#	define	VIRGE_EDAC33_PLLlocked			0x10

#define	VIRGE_EDAC_ColorKeyControl		0x40
#	define	VIRGE_EDAC40_OverlayEnable		0x01
#	define	VIRGE_EDAC40_RedEnable			0x02
#	define	VIRGE_EDAC40_GreenEnable			0x04
#	define	VIRGE_EDAC40_BlueEnable			0x08
#	define	VIRGE_EDAC40_TestTrue			0x00
#	define	VIRGE_EDAC40_TestFalse			0x10

#define	VIRGE_EDAC_OverlayKey			0x41
#define	VIRGE_EDAC_RedKey				0x42
#define	VIRGE_EDAC_GreenKey			0x43
#define	VIRGE_EDAC_BlueKey			0x44

#endif

#endif	/* #ifdef _ramdac_S3_VIRGE_h */
