/* ----------------------------------------------------------------------------
**	VGA ramdac meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: VGA-meta.h,v $
**	Revision 1.3  2003/03/25 03:42:43  fspacek
**	- palette support
**	
**	Revision 1.2  2003/01/25 02:27:57  aldot
**	- sync up to current API, (first part -- 20021001)
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_IBM_VGA_meta_h
#define	_ramdac_IBM_VGA_meta_h

typedef kgim_chipset_io_t vga_ramdac_io_t;

#define VGA_PCIDEV(ctx)	KGIM_PCIDEV(ctx)

#define	VGA_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	VGA_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	VGA_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	VGA_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct vga_ramdac_mode_record_s vga_ramdac_mode_record_t;

#warning UPDATE this
#if 1
typedef struct
{
	kgim_ramdac_mode_t kgim;

	const vga_ramdac_mode_record_t *rec;

	struct 
	{
		kgi_ucoord_t pos, hot, shift;
	} ptr;

	kgi_clut_t	clut_ctrl;
	kgi_u8_t	clut[3 * 256];

} vga_ramdac_mode_t;

#else
typedef struct
{
	kgim_ramdac_mode_t kgim;

	const vga_ramdac_mode_record_t *rec;

	kgi_marker_t	ptr_ctrl;
/*kgi_ucoord_t	ptr_pos;*/
	kgi_ucoord_t	ptr_hot;
	kgi_ucoord_t	ptr_shift;

	kgi_clut_t	clut_ctrl;
	kgi_u8_t	clut[3 * 256];

} vga_ramdac_mode_t;
#endif

typedef struct
{
	kgim_ramdac_t ramdac;
/*
	vga_ramdac_io_t *io;

	kgi_u_t	rev;
*/
	/*	Initial state
	*/

	kgi_u8_t clut[3 * 256];
/*	kgi_u8_t cursor_clut[3 * 4];
	kgi_u8_t cursor_data[];
*/

} vga_ramdac_t;

KGIM_META_INIT_FN(vga_ramdac)
KGIM_META_DONE_FN(vga_ramdac)
KGIM_META_MODE_CHECK_FN(vga_ramdac)
KGIM_META_MODE_ENTER_FN(vga_ramdac)
KGIM_META_MODE_LEAVE_FN(vga_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(vga_ramdac)

#endif	/* #ifndef _ramdac_IBM_VGA_meta_h */
