/* ----------------------------------------------------------------------------
**	ATI RADEON ramdac meta definition
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: RADEON-meta.h,v $
**	Revision 1.2  2002/09/26 02:32:04  redmondp
**	Added palette support to ramdac
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#ifndef	_ramdac_ATI_RADEON_meta_h
#define	_ramdac_ATI_RADEON_meta_h

#include "chipset/ATI/RADEON-meta.h"

typedef radeon_chipset_io_t radeon_ramdac_io_t;

#define	RADEON_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	RADEON_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	RADEON_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	RADEON_DAC_IN8(ctx, reg)	KGIM_DAC_IN8(ctx, reg)

typedef struct
{
	kgim_ramdac_mode_t		kgim;

	// FIXME: should this be in chipset?
	struct {
		struct { kgi_u_t x,y; }
		pos, hot, shift;
	} ptr;

	kgi_clut_t			clut_ctrl;
	/* using 16 bits because Radeon has 10 bits ber component */
	kgi_u16_t			clut[3*256];

} radeon_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t		ramdac;

	/*	initial state
	*/
	kgi_u16_t	clut[3*256];
	kgi_u8_t	cursor_clut[3*4];
	kgi_u8_t	cursor_data[1];

} radeon_ramdac_t;

KGIM_META_INIT_FN(radeon_ramdac)
KGIM_META_DONE_FN(radeon_ramdac)
KGIM_META_MODE_CHECK_FN(radeon_ramdac)
KGIM_META_MODE_ENTER_FN(radeon_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(radeon_ramdac);

#endif	/* #ifndef _ramdac_ATI_RADEON_meta_h	*/
