/* ----------------------------------------------------------------------------
**	ATI Rage 128 clock register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128.h,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#ifndef	_clock_ATI_R128_h
#define	_clock_ATI_R128_h

#define R128_PPLL_CNTL						0x02
#define   R128_02_PPLL_RESET					0x00000001
#define   R128_02_PPLL_SLEEP					0x00000002
#define   R128_02_PPLL_PC_GAINMask				0x00000700
#define   R128_02_PPLL_PC_GAINShift				0
#define   R128_02_PPLL_VC_GAINMask				0x00001800
#define   R128_02_PPLL_VC_GAINShift				11
#define   R128_02_PPLL_DCYCMask					0x00006000
#define   R128_02_PPLL_DCYCShift				13
#define   R128_02_PPLL_RANGE					0x00008000
#define   R128_02_PPLL_ATOMIC_UPDATE_EN				0x00010000
#define   R128_02_PPLL_VGA_ATOMIC_UPDATE_EN			0x00020000
#define   R128_02_PPLL_ATOMIC_UPDATE_SYNC			0x00040000

#define R128_PPLL_REF_DIV					0x03
#define   R128_03_PPLL_REF_DIVMask				0x000003FF
#define   R128_03_PPLL_REF_DIVShift				0
#define   R128_03_PPLL_ATOMIC_UPDATE_R				0x00008000
#define   R128_03_PPLL_ATOMIC_UPDATE_W				0x00008000
#define   R128_03_PPLL_REF_DIV_SRCMask				0x00030000
#define   R128_03_PPLL_REF_DIV_SRCShift				16

#define R128_PLL_DIV_0						0x04
#define   R128_04_PPLL_FB0_DIVMask				0x000007FF
#define   R128_04_PPLL_FB0_DIVShift				0
#define   R128_04_PPLL_ATOMIC_UPDATE_R				0x00008000
#define   R128_04_PPLL_ATOMIC_UPDATE_W				0x00008000
#define   R128_04_PPLL_POST0_DIVMask				0x00070000
#define   R128_04_PPLL_POST0_DIVShift				16

#define R128_PLL_DIV_1						0x05
#define   R128_05_PPLL_FB1_DIVMask				0x000007FF
#define   R128_05_PPLL_FB1_DIVShift				0
#define   R128_05_PPLL_ATOMIC_UPDATE_R				0x00008000
#define   R128_05_PPLL_ATOMIC_UPDATE_W				0x00008000
#define   R128_05_PPLL_POST1_DIVMask				0x00070000
#define   R128_05_PPLL_POST1_DIVShift				16

#define R128_PLL_DIV_2						0x06
#define   R128_06_PPLL_FB2_DIVMask				0x000007FF
#define   R128_06_PPLL_FB2_DIVShift				0
#define   R128_06_PPLL_ATOMIC_UPDATE_R				0x00008000
#define   R128_06_PPLL_ATOMIC_UPDATE_W				0x00008000
#define   R128_06_PPLL_POST2_DIVMask				0x00070000
#define   R128_06_PPLL_POST2_DIVShift				16

#define R128_PLL_DIV_3						0x07
#define   R128_07_PPLL_FB3_DIVMask				0x000007FF
#define   R128_07_PPLL_FB3_DIVShift				0
#define   R128_07_PPLL_ATOMIC_UPDATE_R				0x00008000
#define   R128_07_PPLL_ATOMIC_UPDATE_W				0x00008000
#define   R128_07_PPLL_POST3_DIVMask				0x00070000
#define   R128_07_PPLL_POST3_DIVShift				16

#define R128_HTOTAL_CNTL					0x09

#define R128_MCLK_CNTL						0x0F
#define   R128_0F_FORCE_GCP					0x00010000
#define   R128_0F_FORCE_PIPE3D_CP				0x00020000

#endif	/* #ifdef _clock_ATI_R128_h	*/
