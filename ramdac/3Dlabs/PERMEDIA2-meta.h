/* ----------------------------------------------------------------------------
**	3Dlabs PERMEDIA2 ramdac meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: PERMEDIA2-meta.h,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:56:15  seeger_s
**	- implemented CLUT control and hardware cursor support
**	
**	Revision 1.1.1.1  2000/04/18 08:51:03  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_3Dlabs_PERMEDIA2_meta_h
#define	_ramdac_3Dlabs_PERMEDIA2_meta_h

typedef kgim_chipset_io_t pgc_ramdac_io_t;
#define	PGC_PCIDEV(ctx)			KGIM_PCIDEV(ctx)
#define	PGC_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	PGC_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	PGC_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	PGC_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct pgc_ramdac_mode_record_s pgc_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t		kgim;

	const pgc_ramdac_mode_record_t	*rec;

	kgi_u8_t	ext1E, ext06;

	kgi_marker_t	ptr_ctrl;
	kgi_ucoord_t	ptr_hot;
	kgi_ucoord_t	ptr_shift;

	kgi_clut_t	clut_ctrl;
	kgi_u8_t	clut[3*256];

} pgc_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t		ramdac;

	kgi_u_t	rev;
	/*	initial state
	*/
	struct { kgi_u8_t

		CursorControl, ColorMode, ModeControl, PalettePage, 
		MiscControl, ColorKeyControl, OverlayKey, 
		RedKey, GreenKey, BlueKey;

	} EXT;

	kgi_u8_t	clut[3*256];
/*	kgi_u8_t	cursor_clut[3*4];
	kgi_u8_t	cursor_data[];
*/
	kgi_u8_t	CursorXLow, CursorXHigh, CursorYLow, CursorYHigh;

} pgc_ramdac_t;

KGIM_META_INIT_FN(pgc_ramdac)
KGIM_META_DONE_FN(pgc_ramdac)
KGIM_META_MODE_CHECK_FN(pgc_ramdac)
KGIM_META_MODE_ENTER_FN(pgc_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(pgc_ramdac)

#endif	/* #ifndef _dac_3Dlabs_PERMEDIA2_meta_h	*/
