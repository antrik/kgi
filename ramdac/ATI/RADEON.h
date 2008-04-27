/* ----------------------------------------------------------------------------
**	ATI RADEON register definitions
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: RADEON.h,v $
**	Revision 1.2  2002/09/26 02:32:04  redmondp
**	Added palette support to ramdac
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#ifndef	_ramdac_ATI_RADEON_h
#define	_ramdac_ATI_RADEON_h

#define RADEON_PALETTE_INDEX 		0x000000B0
#define RADEON_PALETTE_W_INDEXMask 	0x000000FF
#define RADEON_PALETTE_R_INDEXMask	0x00FF0000
#define RADEON_PALETTE_R_INDEXShift	16

#define RADEON_PALETTE_DATA  		0x000000B4
#define RADEON_PALETTE_DATA_BMask 	0x000000FF
#define RADEON_PALETTE_DATA_BShift	0
#define RADEON_PALETTE_DATA_GMask 	0x0000FF00
#define RADEON_PALETTE_DATA_GShift 	8
#define RADEON_PALETTE_DATA_RMask 	0x00FF0000
#define RADEON_PALETTE_DATA_RShift 	16

#define RADEON_PALETTE_30_DATA		0x000000B8
#define RADEON_PALETTE_30_DATA_BMask 	0x000003FF
#define RADEON_PALETTE_30_DATA_BShift	0
#define RADEON_PALETTE_30_DATA_GMask 	0x000FFC00
#define RADEON_PALETTE_30_DATA_GShift 	10
#define RADEON_PALETTE_30_DATA_RMask 	0x3FF00000
#define RADEON_PALETTE_30_DATA_RShift 	20

#endif	/* #ifdef _ramdac_ATI_RADEON_h	*/
