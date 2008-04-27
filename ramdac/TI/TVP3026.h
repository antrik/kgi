/* ----------------------------------------------------------------------------
**	TI TVP3026 register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**	Copyright (C)	1997-1998	Hartmut Niemann
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: TVP3026.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:04  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:04  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_TI_TVP3026_h
#define	_ramdac_TI_TVP3026_h

#define	TVP3026_DAC_PW_INDEX		0x0
#define	TVP3026_DAC_P_DATA		0x1
#define	TVP3026_DAC_PIXEL_MASK		0x2
#define	TVP3026_DAC_PR_INDEX		0x3

#define	TVP3026_EDAC_ADDR		0x0
#define	TVP3026_DAC_CW_INDEX		0x4
#define	TVP3026_DAC_C_DATA		0x5
/* register6 reserved			0x6 */
#define	TVP3026_DAC_CR_INDEX		0x7

/* register reserved			0x8 */
#define	TVP3026_DAC_CURSOR_CTRL		0x9
#	define	TVP3026_DAC09_CursorOff		0x00
#	define	TVP3026_DAC09_CursorThreeColor	0x01
#	define	TVP3026_DAC09_CursorXGA		0x02
#	define	TVP3026_DAC09_CursorX		0x03
#	define	TVP3026_DAC09_CursorMask	0x03

#define	TVP3026_EDAC_DATA		0xA
#define	TVP3026_DAC_CURSOR_RAM_DATA	0xB

#define	TVP3026_DAC_CURSOR_XL		0xC
#define	TVP3026_DAC_CURSOR_XH		0xD
#define	TVP3026_DAC_CURSOR_YL		0xE
#define	TVP3026_DAC_CURSOR_YH		0xF

/*
**	indexed registers
*/

/* register reserved			0x00 */
#define	TVP3026_EDAC_Revision		0x01
#	define	TVP3026_EDAC01_MinorMask	0x0F
#	define	TVP3026_EDAC01_MinorShift	0
#	define	TVP3026_EDAC01_MajorMask	0xF0
#	define	TVP3026_EDAC01_MajorShift	4	
/*
**					0x02
** registers reserved			...
**					0x05
*/
#define	TVP3026_EDAC_CursorControl	0x06
#	define	TVP3026_EDAC06_CursorOff	0x00
#	define	TVP3026_EDAC06_CursorThreeColor	0x01
#	define	TVP3026_EDAC06_CursorXGA	0x02
#	define	TVP3026_EDAC06_CursorX		0x03
#	define	TVP3026_EDAC06_CursorMask	0x03
	
#	define	TVP3026_EDAC06_CursorAddr8	0x04
#	define	TVP3026_EDAC06_CursorAddr9	0x08
#	define	TVP3026_EDAC06_4096Blank	0x10
#	define	TVP3026_EDAC06_InterlacedCursor	0x20
#	define	TVP3026_EDAC06_InterlaceInvert	0x40
#	define	TVP3026_EDAC06_DirectCCR	0x80

/*
**					0x07
** registers reservd			...
**					0x0E
*/
#define	TVP3026_EDAC_LatchControl	0x0F
/*
**					0x10
** registers reserved			...
**					0x17
*/
#define	TVP3026_EDAC_TrueColorControl	0x18
#define	TVP3026_EDAC_MultiplexControl	0x19
#define	TVP3026_EDAC_ClockSelect	0x1A
#	define	TVP3026_EDAC1A_VCLKMask		0x70
#	define	TVP3026_EDAC1A_VCLKShift	4
#	define	TVP3026_EDAC1A_ClkMask		0x0F
#	define	TVP3026_EDAC1A_ClkShift		0
#	define	TVP3026_EDAC1A_Clk0		0x00
#	define	TVP3026_EDAC1A_Clk1		0x01
#	define	TVP3026_EDAC1A_Clk2		0x02
#	define	TVP3026_EDAC1A_Clk2Neg		0x03
#	define	TVP3026_EDAC1A_Clk2Ecl		0x04
#	define	TVP3026_EDAC1A_ClkPClkPLL	0x05
#	define	TVP3026_EDAC1A_ClkOff		0x06
#	define	TVP3026_EDAC1A_Clk0VGA		0x07	
		
/* register reserved			0x1B */
#define	TVP3026_EDAC_PalettePage	0x1C
#define	TVP3026_EDAC_GeneralControl	0x1D
#	define	TVP3026_EDAC1D_InvHSYNC		0x01
#	define	TVP3026_EDAC1D_InvVSYNC		0x02
#	define	TVP3026_EDAC1D_BigEndian	0x08
#	define	TVP3026_EDAC1D_BlankPedestral	0x10
#	define	TVP3026_EDAC1D_SyncEnable	0x20
#	define	TVP3026_EDAC1D_OverscanEnable	0x40

#define	TVP3026_EDAC_MiscControl	0x1E
#	define	TVP3026_EDAC1E_PowerDown	0x01
#	define	TVP3026_EDAC1E_6n8Disable	0x04
#	define	TVP3026_EDAC1E_8BitEnable	0x08
#	define	TVP3026_EDAC1E_PortSwitchEnable 0x10
#	define	TVP3026_EDAC1E_PortSwitchInvert 0x20
/*
**					0x1F
** registers reserved			...
**					0x29
*/
#define	TVP3026_EDAC_GIOControl		0x2A
#define	TVP3026_EDAC_GIOData		0x2B
#define	TVP3026_EDAC_PLLAddress		0x2C
#	define	TVP3026_EDAC2C_PClkMask		0x03
#	define	TVP3026_EDAC2C_PClkShift	0

#	define	TVP3026_EDAC2C_MClkMask		0x0C
#	define	TVP3026_EDAC2C_MClkShift	2

#	define	TVP3026_EDAC2C_LClkMask		0x30
#	define	TVP3026_EDAC2C_LClkShift	4

#define	TVP3026_EDAC_PClkData		0x2D
#define	TVP3026_EDAC_MClkData		0x2E
#define	TVP3026_EDAC_LClkData		0x2F
/*
**	generic PLL definitions
*/
#define	TVP3026_PLL_N			0x00
#define	TVP3026_PLL_M			0x01
#define	TVP3026_PLL_P			0x02
#define	TVP3026_PLL_Status		0x03

#define	TVP3026_PClkN_Bias			0xC0

#define	TVP3026_PClkM_Bias			0x00

#define	TVP3026_PClkP_PLLEnable			0x80
#define	TVP3026_PClkP_PClkEnable		0x40
#define	TVP3026_PClkP_LForce			0x08
#define	TVP3026_PClkP_PForce			0x04
#define	TVP3026_PClkP_PMask			0x03
#define	TVP3026_PClkP_PShift			0
#define	TVP3026_PClkP_Bias			0x30

#define	TVP3026_PClkS_Lock			0x40


#define	TVP3026_MClkP_PLLEnable			0x80
#define	TVP3026_MClkP_LForce			0x08
#define	TVP3026_MClkP_PForce			0x04
#define	TVP3026_MClkP_PMask			0x03
#define	TVP3026_MClkP_PShift			0
#define	TVP3026_MClkP_Bias			0x30

#define	TVP3026_MClkS_Lock			0x40


#define	TVP3026_LClkN_Bias			0xC0

#define	TVP3026_LClkM_Bias			0x00

#define	TVP3026_LClkP_PLLEnable			0x80
#define	TVP3026_LClkP_LForce			0x08
#define	TVP3026_LClkP_PForce			0x04
#define	TVP3026_LClkP_PMask			0x03
#define	TVP3026_LClkP_PShift			0
#define	TVP3026_LClkP_Bias			0x70

#define	TVP3026_LClkS_Lock			0x40


#define	TVP3026_EDAC_ColKeyOverlayL	0x30
#define	TVP3026_EDAC_ColKeyOverlayH	0x31
#define	TVP3026_EDAC_ColKeyRedL		0x32
#define	TVP3026_EDAC_ColKeyRedH		0x33
#define	TVP3026_EDAC_ColKeyGreenL	0x34
#define	TVP3026_EDAC_ColKeyGreenH	0x35
#define	TVP3026_EDAC_ColKeyBlueL	0x36
#define	TVP3026_EDAC_ColKeyBlueH	0x37
#define	TVP3026_EDAC_ColKeyControl	0x38
#	define	TVP3026_EDAC38_CompareOverlay	0x01
#	define	TVP3026_EDAC38_CompareRed	0x02
#	define	TVP3026_EDAC38_CompareGreen	0x04
#	define	TVP3026_EDAC38_CompareBlue	0x08
#	define	TVP3026_EDAC38_InverseColKey	0x10
#	define	TVP3026_EDAC38_ZoomMask		0xE0
#	define	TVP3026_EDAC38_ZoomShift	5

#define	TVP3026_EDAC_MClkLoopControl	0x39
#	define	TVP3026_EDAC39_MClkDivMask	0x07
#	define	TVP3026_EDAC39_MClkDivShift	0
#	define	TVP3026_EDAC39_MClkOutStrobe	0x08
	
#	define	TVP3026_EDAC39_MClkOutDClk	0x00
#	define	TVP3026_EDAC39_MClkOutMClk	0x10
#	define	TVP3026_EDAC39_MClkOutMask	0x10

#	define	TVP3026_EDAC39_PClkOutPClk	0x00
#	define	TVP3026_EDAC39_PClkOutLClk	0x20
#	define	TVP3026_EDAC39_PClkOutDClk	0x40	
#	define	TVP3026_EDAC39_PClkOutMask	0x60

#define	TVP3026_EDAC_SenseTest		0x3A
#define	TVP3026_EDAC_TestModeData	0x3B
#define	TVP3026_EDAC_CRCRemainder	0x3C

#define	TVP3026_EDAC_ID			0x3F
#	define	TVP3026_EDAC3F_TVP3026_ID	0x26

#endif	/* #ifdef _ramdac_TI_TVP3026_h	*/
