/* ----------------------------------------------------------------------------
**	ATI Rage 128 ramdac meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Paul_Redmond
**
**	$Log: R128-meta.h,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#ifndef	_ramdac_ATI_R128_meta_h
#define	_ramdac_ATI_R128_meta_h

#include "chipset/ATI/R128-meta.h"
typedef r128_chipset_io_t r128_ramdac_io_t;

typedef struct
{
	kgim_ramdac_mode_t		kgim;

	kgi_clut_t	clut_ctrl;
	kgi_u8_t	clut[3*256];

	kgi_marker_t	ptr_ctrl;
	kgi_ucoord_t	ptr_hot;
	kgi_ucoord_t	ptr_shift;

} r128_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t		ramdac;

	/* initial state */
	kgi_u32_t	clut[256];
	kgi_u32_t	palette_index, cur_offset,
			cur_horz_vert_posn, cur_horz_vert_off,
			cur_clr0, cur_clr1;

} r128_ramdac_t;

KGIM_META_INIT_FN(r128_ramdac)
KGIM_META_DONE_FN(r128_ramdac)
KGIM_META_MODE_CHECK_FN(r128_ramdac)
KGIM_META_MODE_ENTER_FN(r128_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(r128_ramdac)

#endif	/* #ifndef _ramdac_ATI_R128_meta_h	*/
