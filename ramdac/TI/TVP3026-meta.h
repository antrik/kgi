/* ----------------------------------------------------------------------------
**	TI TVP3026 ramdac meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: TVP3026-meta.h,v $
**	Revision 1.2  2003/02/05 19:09:30  foske
**	Update the driver to match current kgi core.
**	
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/04/26 14:07:08  seeger_s
**	- minor cleanups
**	
**	Revision 1.1.1.1  2000/04/18 08:51:04  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_TI_TVP3026_meta_h
#define	_ramdac_TI_TVP3026_meta_h

typedef kgim_chipset_io_t tvp3026_ramdac_io_t;
#define	TVP3026_DAC_OUTS8(ctx, r, b, c)		KGIM_DAC_OUTS8(ctx, r, b, c)
#define	TVP3026_DAC_INS8(ctx, r, b, c)		KGIM_DAC_INS8(ctx, r, b, c)
#define	TVP3026_DAC_OUT8(ctx, val, reg)		KGIM_DAC_OUT8(ctx, val, reg)
#define	TVP3026_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)

typedef struct tvp3026_ramdac_mode_record_s tvp3026_ramdac_mode_record_t;

typedef struct
{
	kgim_ramdac_mode_t		kgim;

	const tvp3026_ramdac_mode_record_t	*rec;

	kgi_u8_t	CursorCtrl, PixelMask;
	kgi_u8_t	ln, lm, lp;
	kgi_u8_t	MClkLoopControl, CursorControl, LatchControl, 
			TrueColorControl, MultiplexControl, ClockSelect,
			MiscControl, GIOData, GeneralControl, PalettePage,
			ColKeyOverlayL, ColKeyOverlayH, ColKeyRedL, ColKeyRedH,
			ColKeyGreenL, ColKeyGreenH, ColKeyBlueL, ColKeyBlueH,
			ColKeyControl;

	kgi_marker_t	ptr_ctrl;
	kgi_ucoord_t	ptr_shift;
	kgi_ucoord_t	ptr_hot;
	
	kgi_u8_t	cursor_clut[4*4];
	kgi_u8_t	cursor_data[1024];

	kgi_clut_t	clut_ctrl;
	kgi_u8_t	clut[3*256];

} tvp3026_ramdac_mode_t;

typedef struct
{
	kgim_ramdac_t		ramdac;

	kgi_u_t	rev;

	/*	initial state
	*/
	kgi_u8_t
		CursorCtrl, CursorControl, LatchControl, TrueColorControl, 
		MultiplexControl, ClockSelect, PalettePage, GeneralControl, 
		MiscControl, GIOControl, GIOData, 
		ColKeyOverlayL, ColKeyOverlayH, ColKeyRedL, ColKeyRedH,
		ColKeyGreenL, ColKeyGreenH, ColKeyBlueL, ColKeyBlueH,
		ColKeyControl;

	kgi_u8_t	clut[3*256];
	kgi_u8_t	cursor_clut[4*4];
	kgi_u8_t	cursor_data[1024];

} tvp3026_ramdac_t;

KGIM_META_INIT_FN(tvp3026_ramdac)
KGIM_META_DONE_FN(tvp3026_ramdac)
KGIM_META_MODE_CHECK_FN(tvp3026_ramdac)
KGIM_META_MODE_ENTER_FN(tvp3026_ramdac)
KGIM_META_IMAGE_RESOURCE_FN(tvp3026_ramdac)

#endif	/* #ifndef _ramdac_TI_TVP3026_meta_h	*/
