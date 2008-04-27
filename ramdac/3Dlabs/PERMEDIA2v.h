/* -----------------------------------------------------------------------------
**	3Dlabs PERMEDIA2v integrated DAC register definitions
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: PERMEDIA2v.h,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:55:50  seeger_s
**	- typo fixed
**	
**	Revision 1.1.1.1  2000/04/18 08:51:04  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_3Dlabs_PERMEDIA2v_h
#define	_ramdac_3Dlabs_PERMEDIA2v_h

#define	PGC2v_DAC_PaletteWriteAddress	0x00
#define	PGC2v_DAC_PaletteData		0x01
#define	PGC2v_DAC_PixelMask		0x02
#define	PGC2v_DAC_PaletteReadAddress	0x03
#define	PGC2v_DAC_IndexLow		0x04
#define	PGC2v_DAC_IndexHigh		0x05
#define	PGC2v_DAC_IndexData		0x06
#define	PGC2v_DAC_IndexControl		0x07
#	define	PGC2v_DAC07_NoIncrement		0x00
#	define	PGC2v_DAC07_AutoIncrement	0x01


#define	PGC2v_EDAC_MiscControl		0x000
#	define	PGC2v_E000_HighColorResolution	0x01
#	define	PGC2v_E000_PixelDouble		0x02
#	define	PGC2v_E000_LastReadAddress	0x04
#	define	PGC2v_E000_DirectColor		0x08
#	define	PGC2v_E000_OverlayEnable	0x10
#	define	PGC2v_E000_PixelDoubleBuffer	0x20
#	define	PGC2v_E000_StereoDoubleBuffer	0x40
#define	PGC2v_EDAC_SyncControl		0x001
#	define	PGC2v_E001_VSyncActiveLow	0x00
#	define	PGC2v_E001_VSyncActiveHigh	0x01
#	define	PGC2v_E001_VSyncTristate	0x02
#	define	PGC2v_E001_VSyncForceActive	0x03
#	define	PGC2v_E001_VSyncForceInactive	0x04
#	define	PGC2v_E001_HSyncActiveLow	0x00
#	define	PGC2v_E001_HSyncActiveHigh	0x08
#	define	PGC2v_E001_HSyncTristate	0x10
#	define	PGC2v_E001_HSyncForceActive	0x18
#	define	PGC2v_E001_HSyncForceInactive	0x20
#define	PGC2v_EDAC_DACControl		0x002
#	define	PGC2v_E002_DACLowPower		0x07
#	define	PGC2v_E002_BlankRedDAC		0x10
#	define	PGC2v_E002_BlankGreeDAC		0x20
#	define	PGC2v_E002_BlankBlueDAC		0x40
#	define	PGC2v_E002_BlankPedestral	0x80
#define	PGC2v_EDAC_PixelSize		0x003
#	define	PGC2v_E003_8bpp			0x00
#	define	PGC2v_E003_16bpp		0x01
#	define	PGC2v_E003_32bpp		0x02
#	define	PGC2v_E003_24bpp		0x04
#define	PGC2v_EDAC_ColorFormat		0x004
#	define	PGC2v_E004_ColorFormat		0x1F
#	define	PGC2v_E004_BGR			0x20
#	define	PGC2v_E004_LinearColorExtension	0x40
#define	PGC2v_EDAC_CursorMode		0x005
#	define	PGC2v_E005_CursorEnable		0x01
#	define	PGC2v_E005_CursorDisable	0x00
#	define	PGC2v_E005_CursorMode0		0x00
#	define	PGC2v_E005_CursorMode1		0x02
#	define	PGC2v_E005_CursorMode2		0x04
#	define	PGC2v_E005_CursorMode3		0x06
#	define	PGC2v_E005_CursorMode4		0x08
#	define	PGC2v_E005_CursorMode5		0x0A
#	define	PGC2v_E005_CursorMode6		0x0C
#	define	PGC2v_E005_CursorWindows	0x00
#	define	PGC2v_E005_CursorX		0x10
#	define	PGC2v_E005_Cursor3Color		0x20
#	define	PGC2v_E005_Cursor15Color	0x30
#	define	PGC2v_E005_ReversePixelOrder	0x40
#define	PGC2v_EDAC_CursorControl	0x006
#	define	PGC2v_E006_DoubleX		0x01
#	define	PGC2v_E006_DoubleY		0x02
#	define	PGC2v_E006_ReadbackLastPosition	0x00
#	define	PGC2v_E006_ReadbackUsedPosition	0x04
#define	PGC2v_EDAC_CursorXLow		0x007
#define	PGC2v_EDAC_CursorXHigh		0x008
#define	PGC2v_EDAC_CursorYLow		0x009
#define	PGC2v_EDAC_CursorYHigh		0x00A
#define	PGC2v_EDAC_CursorHotSpotX	0x00B
#define	PGC2v_EDAC_CursorHotSpotY	0x00C
#define	PGC2v_EDAC_OverlayKey		0x00D
#define	PGC2v_EDAC_Pan			0x00E
#	define	PGC2v_E00E_PanEnable		0x01
#define	PGC2v_EDAC_Sense		0x00F
#	define	PGC2v_E00F_RedSense		0x01
#	define	PGC2v_E00F_GreenSense		0x02
#	define	PGC2v_E00F_BlueSense		0x04
#define	PGC2v_EDAC_CheckControl		0x018
#	define	PGC2v_E018_Pixel		0x01
#	define	PGC2v_E018_LUT			0x02
#define	PGC2v_EDAC_CheckPixelRed	0x019
#define	PGC2v_EDAC_CheckPixelGreen	0x01A
#define	PGC2v_EDAC_CheckPixelBlue	0x01B
#define	PGC2v_EDAC_CheckLUTRed		0x01C
#define	PGC2v_EDAC_CheckLUTGreen	0x01D
#define	PGC2v_EDAC_CheckLUTBlue		0x01E
#define	PGC2v_EDAC_DClkSetup1		0x1F0
#define	PGC2v_EDAC_DClkSetup2		0x1F1
#define	PGC2v_EDAC_MClkSetup1		0x1F2
#define	PGC2v_EDAC_MClkSetup2		0x1F3
#define	PGC2v_EDAC_DClkControl		0x200
#	define	PGC2v_E200_DClkDisable		0x00
#	define	PGC2v_E200_DClkEnable		0x01
#	define	PGC2v_E200_DClkLocked		0x02
#define	PGC2v_EDAC_DClk0PreScale	0x201
#define	PGC2v_EDAC_DClk0FeedbackScale	0x202
#define	PGC2v_EDAC_DClk0PostScale	0x203
#define	PGC2v_EDAC_DClk1PreScale	0x204
#define	PGC2v_EDAC_DClk1FeedbackScale	0x205
#define	PGC2v_EDAC_DClk1PostScale	0x206
#define	PGC2v_EDAC_DClk2PreScale	0x207
#define	PGC2v_EDAC_DClk2FeedbackScale	0x208
#define	PGC2v_EDAC_DClk2PostScale	0x209
#define	PGC2v_EDAC_DClk3PreScale	0x20A
#define	PGC2v_EDAC_DClk3FeedbackScale	0x20B
#define	PGC2v_EDAC_DClk3PostScale	0x20C
#define	PGC2v_EDAC_MClkControl		0x20D
#	define	PGC2v_E200_Enable		0x01
#	define	PGC2v_E200_Locked		0x02
#define	PGC2v_EDAC_MClkPreScale		0x20E
#define	PGC2v_EDAC_MClkFeedBackScale	0x20F
#define	PGC2v_EDAC_MClkPostScale	0x210
#define	PGC2v_EDAC_CursorPaletteBase	0x303
#define	PGC2v_EDAC_CursorPaletteSize	45
#define	PGC2v_EDAC_CursorPatternBase	0x400
#define	PGC2v_EDAC_CursorPatternSize	1024

#endif	/* #ifdef _ramdac_3Dlabs_PERMEDIA2_h */
