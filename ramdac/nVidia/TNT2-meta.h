/* ----------------------------------------------------------------------------
**	nVidia TNT2 ramdac meta definition
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
**	$Log: TNT2-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:06  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:06  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_nVidia_TNT2_meta_h
#define	_ramdac_nVidia_TNT2_meta_h

typedef kgim_chipset_io_t tnt2_ramdac_io_t;

#define	TNT2_PCIDEV(ctx)		KGIM_PCIDEV(ctx)
#define	TNT2_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	TNT2_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	TNT2_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	TNT2_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct tnt2_ramdac_mode_record_s tnt2_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t kgim;
	const tnt2_ramdac_mode_record_t *rec;
	
	struct 
	{
		struct 
		{ 
			kgi_u_t x,y; 
		} pos, hot, shift;
	} ptr;

	kgi_u8_t ext1E, ext06;
	kgi_u8_t clut[3 * 256];

} tnt2_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t ramdac;
	tnt2_ramdac_io_t *io;

	kgi_u_t	rev;
	
	/* Initial state */
	struct 
	{ 
		kgi_u8_t
		CursorControl, ColorMode, ModeControl, PalettePage, 
		MiscControl, ColorKeyControl, OverlayKey, 
		RedKey, GreenKey, BlueKey;
	} EXT;

	kgi_u8_t clut[3 * 256];
/*	kgi_u8_t cursor_clut[3 * 4];
	kgi_u8_t cursor_data[];
*/
	kgi_u8_t CursorXLow, CursorXHigh, CursorYLow, CursorYHigh;

} tnt2_ramdac_t;

KGIM_META_INIT_FN(tnt2_ramdac)
KGIM_META_DONE_FN(tnt2_ramdac)
KGIM_META_MODE_CHECK_FN(tnt2_ramdac)
KGIM_META_MODE_ENTER_FN(tnt2_ramdac)

#endif	/* #ifndef _ramdac_nVidia_TNT2_meta_h */
