/* ----------------------------------------------------------------------------
**	ATI MACH64 ramdac meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Filip_Spacek
**
**	$Log: MACH64-meta.h,v $
**	Revision 1.2  2002/06/07 01:22:18  fspacek
**	Export a clut image resource since at least the console layer can use it
**	
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:35  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:17:59  phil
**	Template generated code for ATI ramdac
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_ATI_MACH64_meta_h
#define	_ramdac_ATI_MACH64_meta_h

typedef kgim_chipset_io_t mach64_ramdac_io_t;
#define MACH64_PCIDEV(ctx)		KGIM_PCIDEV(ctx)
#define MACH64_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define MACH64_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define MACH64_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define MACH64_DAC_IN8(ctx, reg)	KGIM_DAC_IN8(ctx, reg)

typedef struct mach64_ramdac_mode_record_s mach64_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t kgim;

	const mach64_ramdac_mode_record_t *rec;

	kgi_clut_t			clut_ctrl;
	kgi_u8_t			clut[3*256];

} mach64_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t		ramdac;

	kgi_u_t	rev;

	/*	initial state
	*/
	kgi_u8_t	clut[3*256];
	kgi_u8_t	cursor_clut[3*4];
	kgi_u8_t	cursor_data[1];

} mach64_ramdac_t;

KGIM_META_INIT_FN(mach64_ramdac)
KGIM_META_DONE_FN(mach64_ramdac)
KGIM_META_MODE_CHECK_FN(mach64_ramdac)
KGIM_META_MODE_ENTER_FN(mach64_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(mach64_ramdac)

#endif	/* #ifndef _ramdac_ATI_MACH64_meta_h	*/
