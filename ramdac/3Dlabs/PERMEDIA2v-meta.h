/* ----------------------------------------------------------------------------
**	3Dlabs PERMEDIA2v ramdac meta language definition
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
**	$Log: PERMEDIA2v-meta.h,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:56:15  seeger_s
**	- implemented CLUT control and hardware cursor support
**	
**	Revision 1.1.1.1  2000/04/18 08:51:04  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_3Dlabs_PERMEDIA2v_meta_h
#define	_ramdac_3Dlabs_PERMEDIA2v_meta_h

typedef kgim_chipset_io_t pgc2v_ramdac_io_t;
#define	PGC2v_PCIDEV(ctx)		KGIM_PCIDEV(ctx)
#define	PGC2v_DAC_OUTS8(ctx, r, b, c)	KGIM_DAC_OUTS8(ctx, r, b, c)
#define	PGC2v_DAC_INS8(ctx, r, b, c)	KGIM_DAC_INS8(ctx, r, b, c)
#define	PGC2v_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	PGC2v_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct pgc2v_ramdac_mode_record_s pgc2v_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t		kgim;

	const pgc2v_ramdac_mode_record_t *rec;

	struct { kgi_u8_t

		MiscControl, SyncControl, DACControl, PixelSize,
		ColorFormat, CursorMode, CursorControl, CursorXLow,
		CursorXHigh, CursorYLow, CursorYHigh, CursorHotSpotX,
		CursorHotSpotY, OverlayKey, Pan, Sense, CheckControl;

	} RD;

	kgi_marker_t	ptr_ctrl;

	kgi_clut_t	clut_ctrl;
	kgi_u8_t	clut[3*256];

/*	kgi_u8_t	cursor_clut[PGC2v_EDAC_CursorPaletteSize];
**	kgi_u8_t	cursor_data[PGC2v_EDAC_CursorPatternSize];
*/
} pgc2v_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t			ramdac;

	/*	initial state
	*/
	struct { kgi_u8_t

		MiscControl, SyncControl, DACControl, PixelSize,
		ColorFormat, CursorMode, CursorControl, CursorXLow,
		CursorXHigh, CursorYLow, CursorYHigh, CursorHotSpotX,
		CursorHotSpotY, OverlayKey, Pan, Sense, CheckControl;

	} RD;

	kgi_u8_t	IndexControl;

/*	kgi_u8_t	clut[3*256];
**	kgi_u8_t	cursor_clut[PGC2v_EDAC_CursorPaletteSize];
**	kgi_u8_t	cursor_data[PGC2v_EDAC_CursorPatternSize];
*/
} pgc2v_ramdac_t;

KGIM_META_INIT_FN(pgc2v_ramdac)
KGIM_META_DONE_FN(pgc2v_ramdac)
KGIM_META_MODE_CHECK_FN(pgc2v_ramdac)
KGIM_META_MODE_ENTER_FN(pgc2v_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(pgc2v_ramdac)

#endif	/* #ifndef _dac_3Dlabs_PERMEDIA2v_meta_h	*/
