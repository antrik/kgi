/* ----------------------------------------------------------------------------
**	ATI Rage 128 register definitions
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
#ifndef	_ramdac_ATI_R128_h
#define	_ramdac_ATI_R128_h

#define R128_PALETTE_INDEX					0xB0
#define   R128_B0_PALETTE_W_INDEXMask				0x000000FF
#define   R128_B0_PALETTE_W_INDEXShift				0
#define   R128_B0_PALETTE_R_INDEXMask				0x00FF0000
#define   R128_B0_PALETTE_R_INDEXShift				16

#define R128_PALETTE_DATA					0xB4
#define   R128_B4_PALETTE_DATA_BMask				0x000000FF
#define   R128_B4_PALETTE_DATA_BShift				0
#define   R128_B4_PALETTE_DATA_GMask				0x0000FF00
#define   R128_B4_PALETTE_DATA_GShift				8
#define   R128_B4_PALETTE_DATA_RMask				0x00FF0000
#define   R128_B4_PALETTE_DATA_RShift				16

#define R128_CUR_OFFSET						0x260
#define   R128_260_CUR_OFFSETMask				0x01FFFFFF
#define   R128_260_CUR_OFFSETShift				0
#define   R128_260_CUR_LOCK					0x80000000

#define R128_CUR_HORZ_VERT_POSN					0x264
#define   R128_264_CUR_VERT_POSNMask				0x000007FF
#define   R128_264_CUR_VERT_POSNShift				0
#define   R128_264_CUR_HORZ_POSNMask				0x07FF0000
#define   R128_264_CUR_HORZ_POSNShift				16
#define   R128_264_CUR_LOCK					0x80000000

#define R128_CUR_HORZ_VERT_OFF					0x268
#define   R128_268_CUR_VERT_OFFMask				0x0000003F
#define   R128_268_CUR_VERT_OFFShift				0
#define   R128_268_CUR_HORZ_OFFMask				0x003F0000
#define   R128_268_CUR_HORZ_OFFShift				16
#define   R128_268_CUR_LOCK					0x80000000

#define R128_CUR_CLR0						0x26C
#define   R128_26C_CUR_CLR0_BMask				0x000000FF
#define   R128_26C_CUR_CLR0_BShift				0
#define   R128_26C_CUR_CLR0_GMask				0x0000FF00
#define   R128_26C_CUR_CLR0_GShift				8
#define   R128_26C_CUR_CLR0_RMask				0x00FF0000
#define   R128_26C_CUR_CLR0_RShift				16

#define R128_CUR_CLR1						0x270
#define   R128_270_CUR_CLR1_BMask				0x000000FF
#define   R128_270_CUR_CLR1_BShift				0
#define   R128_270_CUR_CLR1_GMask				0x0000FF00
#define   R128_270_CUR_CLR1_GShift				8
#define   R128_270_CUR_CLR1_RMask				0x00FF0000
#define   R128_270_CUR_CLR1_RShift				16

#endif	/* #ifdef _ramdac_ATI_R128_h	*/
