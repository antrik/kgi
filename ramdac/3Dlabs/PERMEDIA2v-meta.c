/* -----------------------------------------------------------------------------
**	PERMEDIA2v integrated DAC meta language implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2v-meta.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:56:15  seeger_s
**	- implemented CLUT control and hardware cursor support
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:04  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#include "ramdac/3Dlabs/PERMEDIA2v.h"
#include "ramdac/3Dlabs/PERMEDIA2v-meta.h"

static inline void pgc2v_ramdac_set_index(pgc2v_ramdac_io_t *pgc2v_io,
	kgi_u16_t reg)
{
	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_DAC07_AutoIncrement, 0x07);

	PGC2v_DAC_OUT8(pgc2v_io, reg & 0xFF, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, reg >> 8, PGC2v_DAC_IndexHigh);
}

#define	PGC2v_MAGIC	0x0101
#define	PGC2v_SHIFT	8

struct pgc2v_ramdac_mode_record_s
{
	kgi_u8_t	ColorFormat;
	kgi_u8_t	MiscControl;
	kgi_u8_t	bpp;

	kgi_u8_t	ilut;
	kgi_u8_t	alut;

	kgi_ratio_t		lclk;
	kgi_attribute_mask_t	fam;
	const kgi_u8_t		*bpfa;
	const kgi_u8_t		*bpda;
};

/*	normal 'linear' modes
*/
static const pgc2v_ramdac_mode_record_t pgc2v_ramdac_linear[] =
{ /*	    CF    MC bpp il al  LCLK   fam          bpfa         bpda	*/
	{ 0x00, 0x01, 32, 0, 1, {1,2}, KGI_AM_RGBA, KGI_AS_8888, KGI_AS_8888 },
	{ 0x41, 0x01, 16, 0, 1, {1,4}, KGI_AM_RGBA, KGI_AS_5551, KGI_AS_5551 },
	{ 0x42, 0x01, 16, 0, 1, {1,4}, KGI_AM_RGBA, KGI_AS_4444, KGI_AS_4444 },
	{ 0x49, 0x01,  8, 0, 1, {1,4}, KGI_AM_RGBA, KGI_AS_2321, KGI_AS_2321 },

	{ 0x00, 0x01, 32, 0, 1, {1,2}, KGI_AM_RGBX, KGI_AS_8888, KGI_AS_8888 },
	{ 0x41, 0x01, 16, 0, 1, {1,4}, KGI_AM_RGBX, KGI_AS_5551, KGI_AS_5551 },
	{ 0x42, 0x01, 16, 0, 1, {1,4}, KGI_AM_RGBX, KGI_AS_4444, KGI_AS_4444 },
	{ 0x49, 0x01,  8, 0, 1, {1,4}, KGI_AM_RGBX, KGI_AS_2321, KGI_AS_2321 },

	{ 0x00, 0x19, 32, 1, 0, {1,2}, KGI_AM_RGBI, KGI_AS_8888, KGI_AS_8888 },
	{ 0x41, 0x19, 16, 1, 0, {1,4}, KGI_AM_RGBI, KGI_AS_5551, KGI_AS_5551 },
	{ 0x42, 0x19, 16, 1, 0, {1,4}, KGI_AM_RGBI, KGI_AS_4444, KGI_AS_4444 },
	{ 0x49, 0x19,  8, 1, 0, {1,4}, KGI_AM_RGBI, KGI_AS_2321, KGI_AS_2321 },

	{ 0x00, 0x01, 24, 0, 1, {3,8}, KGI_AM_RGB,  KGI_AS_888,  KGI_AS_888 },
	{ 0x50, 0x01, 16, 0, 1, {1,4}, KGI_AM_RGB,  KGI_AS_565,  KGI_AS_565 },
	{ 0x45, 0x01,  8, 0, 1, {1,4}, KGI_AM_RGB,  KGI_AS_332,  KGI_AS_332 },
 
	{ 0x0E, 0x01,  8, 1, 0, {1,4}, KGI_AM_I,    KGI_AS_8,    KGI_AS_8 },
	{ 0x0E, 0x01,  8, 1, 0, {1,4}, KGI_AM_TEXT, KGI_AS_448,  KGI_AS_448 }
};

/*	per-pixel double buffered modes
*/
static const pgc2v_ramdac_mode_record_t pgc2v_ramdac_double[] =
{ /*	    CF    MC bpp il al  LCLK   fam          bpfa		*/
	{ 0x41, 0x21, 32, 0, 1, {1,2}, KGI_AM_RGBP, KGI_AS_5551, KGI_AS_AAA2 },
	{ 0x49, 0x21, 16, 0, 1, {1,4}, KGI_AM_RGBP, KGI_AS_2321, KGI_AS_4642 },

	{ 0x50, 0x21, 32, 0, 1, {1,2}, KGI_AM_RGB,  KGI_AS_565,  KGI_AS_ACA },
	{ 0x45, 0x21, 32, 0, 1, {1,4}, KGI_AM_RGB,  KGI_AS_332,  KGI_AS_664 }
};

/*	'per-pixel' stereo modes
*/
static const pgc2v_ramdac_mode_record_t pgc2v_ramdac_stereo[] =
{ /*	    CF    MC bpp il al  LCLK   fam          bpfa		*/
	{ 0x42, 0x41, 32, 0, 1, {1,2}, KGI_AM_RGBA, KGI_AS_4444, KGI_AS_8888 },
	{ 0x41, 0x41, 32, 0, 1, {1,2}, KGI_AM_RGBA, KGI_AS_5551, KGI_AS_AAA2 },

	{ 0x42, 0x41, 32, 0, 1, {1,2}, KGI_AM_RGBX, KGI_AS_4444, KGI_AS_8888 },
	{ 0x41, 0x41, 32, 0, 1, {1,2}, KGI_AM_RGBX, KGI_AS_5551, KGI_AS_AAA2 },

	{ 0x42, 0x59, 32, 1, 0, {1,2}, KGI_AM_RGBI, KGI_AS_4444, KGI_AS_8888 },
	{ 0x41, 0x59, 32, 1, 0, {1,2}, KGI_AM_RGBI, KGI_AS_5551, KGI_AS_AAA2 },

	{ 0x50, 0x41, 32, 0, 1, {1,2}, KGI_AM_RGB,  KGI_AS_565,  KGI_AS_ACA }
};

static inline const pgc2v_ramdac_mode_record_t *pgc2v_ramdac_mode_record(
	const kgi_image_mode_t *img)
{
	kgi_u_t i = 0;

#define	PGC2v_RAMDAC_MODE_MATCH(img, modes)				\
	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {	\
									\
		if ((modes[i].fam == img->fam) && 			\
			(0 == kgim_strcmp(modes[i].bpfa, img->bpfa))) {	\
									\
			KRN_DEBUG(2, #modes " mode %i matches", i);	\
			return modes + i;				\
		}							\
	}

	switch (img->frames) {

	case 1:
		if (img->flags & KGI_IF_STEREO) {

			/*	For single-buffered stereo modes, try to match
			**	per-pixel stereo modes first. If that fails,
			**	try to match a normal 'linear' modes.
			*/
			PGC2v_RAMDAC_MODE_MATCH(img, pgc2v_ramdac_stereo);
		}

		PGC2v_RAMDAC_MODE_MATCH(img, pgc2v_ramdac_linear);

		KRN_DEBUG(1, "no mode matched");
		return NULL;

	case 2:
		/*	For double-buffered modes, try to match per-pixel 
		**	double-buffered modes first. If that fails, try
		**	to match normal 'linear' modes.
		*/
		PGC2v_RAMDAC_MODE_MATCH(img, pgc2v_ramdac_double);
		/*	fall through
		*/
	default:
		/*	For all others try to match linear modes.
		*/
		PGC2v_RAMDAC_MODE_MATCH(img, pgc2v_ramdac_linear);

		KRN_DEBUG(1, "no mode matched");
		return NULL;
	}
#undef	PGC2v_RAMDAC_MODE_MATCH
}

/* ---------------------------------------------------------------------------
**	image resources
** ---------------------------------------------------------------------------
*/

/*	CLUT control
*/
static void pgc2v_ramdac_clut_set(kgi_clut_t *clut,
		kgi_u_t table, kgi_u_t index, kgi_u_t count,
		kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	pgc2v_ramdac_mode_t *pgc2v_mode = clut->meta;
	pgc2v_ramdac_io_t   *pgc2v_io   = clut->meta_io;

	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(table == 0);
	KRN_ASSERT(count > 0);

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {

		if (am & KGI_AM_COLOR1) {

			pgc2v_mode->clut[dst+0] = data[src++] >> PGC2v_SHIFT;
		}
		if (am & KGI_AM_COLOR2) {

			pgc2v_mode->clut[dst+1] = data[src++] >> PGC2v_SHIFT;
		}
		if (am & KGI_AM_COLOR3) {

			pgc2v_mode->clut[dst+2] = data[src++] >> PGC2v_SHIFT;
		}
		dst += 3;
	}

	PGC2v_DAC_OUT8(pgc2v_io, index, PGC2v_DAC_PaletteWriteAddress);
	PGC2v_DAC_OUTS8(pgc2v_io, PGC2v_DAC_PaletteData,
		pgc2v_mode->clut + 3*index, 3*count);
}


void pgc2v_ramdac_ptr_set_mode(kgi_marker_t *ptr, kgi_marker_mode_t mode)
{
	pgc2v_ramdac_mode_t *pgc2v_mode = ptr->meta;
	pgc2v_ramdac_io_t   *pgc2v_io	= ptr->meta_io;

	pgc2v_mode->RD.CursorMode = PGC2v_E005_CursorMode0 |
		PGC2v_E005_CursorEnable;

	switch (mode) {

	case KGI_MM_WINDOWS:
		pgc2v_mode->RD.CursorMode |= PGC2v_E005_CursorWindows;
		break;

	case KGI_MM_X11:
		pgc2v_mode->RD.CursorMode |= PGC2v_E005_CursorX;
		break;

	case KGI_MM_3COLOR:
		pgc2v_mode->RD.CursorMode |= PGC2v_E005_Cursor3Color;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	pgc2v_ramdac_set_index(pgc2v_io, PGC2v_EDAC_CursorMode);
	PGC2v_DAC_OUT8(pgc2v_io,
			pgc2v_mode->RD.CursorMode, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io,
			pgc2v_mode->RD.CursorControl, PGC2v_DAC_IndexData);
}

void pgc2v_ramdac_ptr_set_shape(kgi_marker_t *ptr, kgi_u_t shape,
	kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
	const kgi_rgb_color_t *color)
{
	pgc2v_ramdac_mode_t *pgc2v_mode = ptr->meta;
	pgc2v_ramdac_io_t   *pgc2v_io	= ptr->meta_io;

	const kgi_u8_t *and_mask, *xor_mask;
	kgi_u_t i;

	pgc2v_mode->RD.CursorHotSpotX = hot_x;
	pgc2v_mode->RD.CursorHotSpotY = hot_y;

	pgc2v_ramdac_set_index(pgc2v_io, PGC2v_EDAC_CursorHotSpotX);
	PGC2v_DAC_OUT8(pgc2v_io,
			pgc2v_mode->RD.CursorHotSpotX, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io,
			pgc2v_mode->RD.CursorHotSpotY, PGC2v_DAC_IndexData);

	/*	set pattern
	*/
	and_mask = data;
	xor_mask = and_mask + 512;
        for (i = 0; i < 512; i++) {

		kgi_u16_t and = and_mask[i];
		kgi_u16_t xor = xor_mask[i];
		kgi_u16_t pattern;

		xor += (xor & ~0x0001);
		and += (and & ~0x0001);
		xor += (xor & ~0x0007);
		and += (and & ~0x0007);
		xor += (xor & ~0x001F);
		and += (and & ~0x001F);
		xor += (xor & ~0x007F);
		and += (and & ~0x007F);
		xor += (xor & ~0x01FF);
		and += (and & ~0x01FF);
		xor += (xor & ~0x07FF);
		and += (and & ~0x07FF);
		xor += (xor & ~0x1FFF);
		and += (and & ~0x1FFF);

		pattern = (and << 1) | xor;

		PGC2v_DAC_OUT8(pgc2v_io, pattern, PGC2v_DAC_IndexData);
		PGC2v_DAC_OUT8(pgc2v_io, pattern >> 8, PGC2v_DAC_IndexData);
        }

	/*	set colors
	*/
	pgc2v_ramdac_set_index(pgc2v_io, PGC2v_EDAC_CursorPaletteBase);
	for (i = 0; i < 3; i++) {

		PGC2v_DAC_OUT8(pgc2v_io, color[i].r >> PGC2v_SHIFT,
			PGC2v_DAC_IndexData);
		PGC2v_DAC_OUT8(pgc2v_io, color[i].g >> PGC2v_SHIFT,
			PGC2v_DAC_IndexData);
		PGC2v_DAC_OUT8(pgc2v_io, color[i].b >> PGC2v_SHIFT,
			PGC2v_DAC_IndexData);
	}
}

static void pgc2v_ramdac_ptr_show(kgi_marker_t *ptr, kgi_u_t x, kgi_u_t y)
{
	pgc2v_ramdac_mode_t	*pgc2v_mode = ptr->meta;
	pgc2v_ramdac_io_t	*pgc2v_io   = ptr->meta_io;

	pgc2v_ramdac_set_index(pgc2v_io, PGC2v_EDAC_CursorXLow);
	PGC2v_DAC_OUT8(pgc2v_io, x,      PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, x >> 8, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, y,      PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, y >> 8, PGC2v_DAC_IndexData);
}

static void pgc2v_ramdac_ptr_hide(kgi_marker_t *ptr)
{
	pgc2v_ramdac_mode_t	*pgc2v_mode = ptr->meta;
	pgc2v_ramdac_io_t	*pgc2v_io   = ptr->meta_io;

	pgc2v_ramdac_set_index(pgc2v_io, PGC2v_EDAC_CursorXLow);
	PGC2v_DAC_OUT8(pgc2v_io, 0xFF, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, 0x7F, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, 0xFF, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, 0x7F, PGC2v_DAC_IndexData);
}


kgi_error_t pgc2v_ramdac_init(pgc2v_ramdac_t *pgc2v,
	pgc2v_ramdac_io_t *pgc2v_io, const kgim_options_t *options)
{
	pgc2v->IndexControl = PGC2v_DAC_IN8(pgc2v_io, 0x07);
	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_DAC07_AutoIncrement, 0x07);

	PGC2v_DAC_OUT8(pgc2v_io, 0x00, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, 0x00, PGC2v_DAC_IndexHigh);

	pgc2v->RD.MiscControl	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.SyncControl	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.DACControl	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.PixelSize	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.ColorFormat	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorMode	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorControl	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorXLow	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorXHigh	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorYLow	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorYHigh	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorHotSpotX= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.CursorHotSpotY= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.OverlayKey	= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.Pan		= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
	pgc2v->RD.Sense		= PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);

	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_EDAC_CheckControl, PGC2v_DAC_IndexLow);
	pgc2v->RD.CheckControl  = PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);

/*	kgi_u8_t cursor_clut[PGC2v_EDAC_CursorPaletteSize];
**	kgi_u8_t cursor_data[PGC2v_EDAC_CursorPatternSize];
**	kgi_u8_t clut[3*256];
*/
	return KGI_EOK;
}

void pgc2v_ramdac_done(pgc2v_ramdac_t *pgc2v, pgc2v_ramdac_io_t *pgc2v_io, 
	const kgim_options_t *options)
{
	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_DAC07_AutoIncrement, 0x07);
	PGC2v_DAC_OUT8(pgc2v_io, 0x00, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, 0x00, PGC2v_DAC_IndexHigh);

	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.MiscControl,    PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.SyncControl,    PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.DACControl,     PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.PixelSize,      PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.ColorFormat,    PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorMode,     PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorControl,  PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorXLow,     PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorXHigh,    PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorYLow,     PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorYHigh,    PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorHotSpotX, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CursorHotSpotY, PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.OverlayKey,     PGC2v_DAC_IndexData);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.Pan,            PGC2v_DAC_IndexData);

	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_EDAC_CheckControl,  PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->RD.CheckControl,   PGC2v_DAC_IndexData);

/*	kgi_u8_t cursor_clut[PGC2v_EDAC_CursorPaletteSize];
**	kgi_u8_t cursor_data[PGC2v_EDAC_CursorPatternSize];
**	kgi_u8_t clut[3*256];
*/
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v->IndexControl, PGC2v_DAC_IndexControl);
}

kgi_error_t pgc2v_ramdac_mode_check(pgc2v_ramdac_t *pgc2v,
	pgc2v_ramdac_io_t *pgc2v_io, pgc2v_ramdac_mode_t *pgc2v_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const pgc2v_ramdac_mode_record_t *rec = NULL;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;

	switch (cmd) {

	case KGI_TC_PROPOSE:

		if (0 == img[0].frames) {

			img[0].frames = 1;
		}

		if (NULL == (rec = pgc2v_ramdac_mode_record(img))) {

			KRN_DEBUG(1, "could not handle frame attributes %.8x",
				img[0].fam);
			return -KGI_ERRNO(RAMDAC, UNKNOWN);
		}

		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;

		dpm->lclk = rec->lclk;
		dpm->rclk.mul = dpm->rclk.div = 1;
		dpm->dam  = rec->fam;
		dpm->bpda = rec->bpda;

		KRN_DEBUG(2, " lclk %i:%i, dclk %i:%i", dpm->lclk.mul,
			dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk < pgc2v->ramdac.lclk.min) ||
			(dpm->dclk < pgc2v->ramdac.dclk.min)) {

			KRN_DEBUG(1, "%i Hz LCLK is too low", lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk > pgc2v->ramdac.lclk.max) || 
			(dpm->dclk > pgc2v->ramdac.dclk.max)) {

			register kgi_u_t dclk = pgc2v->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < pgc2v->ramdac.dclk.max)
				? dclk : pgc2v->ramdac.dclk.max;
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > pgc2v->ramdac.lclk.max) || 
			(dpm->dclk > pgc2v->ramdac.dclk.max)) {

			KRN_DEBUG(1, "%i Hz LCLK is too high", lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk < pgc2v->ramdac.lclk.min) ||
			(dpm->dclk < pgc2v->ramdac.dclk.min)) {

			register kgi_u_t dclk = pgc2v->ramdac.dclk.min *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > pgc2v->ramdac.dclk.min)
				? dclk : pgc2v->ramdac.dclk.min;
		}
		return KGI_EOK;

     case KGI_TC_CHECK:

		if (NULL == (rec = pgc2v_ramdac_mode_record(img))) {

			KRN_DEBUG(1, "could not handle frame attributes %.8x",
				img[0].fam);
			return -KGI_ERRNO(RAMDAC, UNKNOWN);
		}

		if (! ((dpm->lclk.mul == rec->lclk.mul) &&
			(dpm->lclk.div == rec->lclk.div))) {

			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		if ((dpm->dclk < pgc2v->ramdac.dclk.min) ||
			(pgc2v->ramdac.dclk.max < dpm->dclk) ||
			(lclk < pgc2v->ramdac.lclk.min) ||
			(pgc2v->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(1, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (pgc2v_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
			KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		pgc2v_mode->rec = rec;

		pgc2v_mode->RD.MiscControl = rec->MiscControl;
		pgc2v_mode->RD.SyncControl =
			((pgc2v_mode->kgim.crt->x.polarity > 0)
				? PGC2v_E001_HSyncActiveHigh
				: PGC2v_E001_HSyncActiveLow) |
			((pgc2v_mode->kgim.crt->y.polarity > 0)
				? PGC2v_E001_VSyncActiveHigh
				: PGC2v_E001_VSyncActiveLow);
		if (pgc2v_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

			pgc2v_mode->RD.DACControl |=
				PGC2v_E002_BlankPedestral;
		}

		switch (rec->bpp) {

		case  8: pgc2v_mode->RD.PixelSize = PGC2v_E003_8bpp; break;
		case 16: pgc2v_mode->RD.PixelSize = PGC2v_E003_16bpp; break;
		case 32: pgc2v_mode->RD.PixelSize = PGC2v_E003_32bpp; break;
		case 24: pgc2v_mode->RD.PixelSize = PGC2v_E003_24bpp; break;
		default:
			KRN_INTERNAL_ERROR;
			return -KGI_ERRNO(RAMDAC, UNKNOWN);
		}

		pgc2v_mode->RD.ColorFormat = rec->ColorFormat;

		/*	cursor setup
		*/
		pgc2v_mode->RD.CursorMode = PGC2v_E005_CursorMode0 |
			PGC2v_E005_CursorDisable;
		pgc2v_mode->RD.CursorControl = PGC2v_E006_ReadbackLastPosition;

		pgc2v_mode->RD.CursorXLow = pgc2v_mode->RD.CursorYLow = 0xFF;
		pgc2v_mode->RD.CursorXHigh = pgc2v_mode->RD.CursorYHigh = 0x7F;

		pgc2v_mode->RD.CursorHotSpotX =
			pgc2v_mode->RD.CursorHotSpotY = 0;

		pgc2v_mode->RD.CheckControl = 0;

		/*	hardware pointer control
		*/
		pgc2v_mode->ptr_ctrl.meta	= pgc2v_mode;
		pgc2v_mode->ptr_ctrl.meta_io	= pgc2v_io;
		pgc2v_mode->ptr_ctrl.type	= KGI_RT_POINTER_CONTROL;
		pgc2v_mode->ptr_ctrl.prot	= KGI_PF_DRV_RWS;
		pgc2v_mode->ptr_ctrl.name	= "Permedia2v pointer control";

		pgc2v_mode->ptr_ctrl.modes	=
			KGI_MM_WINDOWS | KGI_MM_X11 | KGI_MM_3COLOR;
		pgc2v_mode->ptr_ctrl.shapes	= 1;
		pgc2v_mode->ptr_ctrl.size.x	= 64;
		pgc2v_mode->ptr_ctrl.size.y	= 64;
		pgc2v_mode->ptr_ctrl.SetMode	= pgc2v_ramdac_ptr_set_mode;
		pgc2v_mode->ptr_ctrl.Select	= NULL;
		pgc2v_mode->ptr_ctrl.SetShape	= pgc2v_ramdac_ptr_set_shape;
		
		pgc2v_mode->ptr_ctrl.Show	= pgc2v_ramdac_ptr_show;
		pgc2v_mode->ptr_ctrl.Hide	= pgc2v_ramdac_ptr_hide;
		pgc2v_mode->ptr_ctrl.Undo	= NULL;

		/*	color/attribute look up table control
		*/
		pgc2v_mode->clut_ctrl.meta	= pgc2v_mode;
		pgc2v_mode->clut_ctrl.meta_io	= pgc2v_io;
		pgc2v_mode->clut_ctrl.prot	= KGI_PF_DRV_RWS;
		if (dpm->flags & KGI_DPF_CH_ALUT) {

			pgc2v_mode->clut_ctrl.type	= KGI_RT_ALUT_CONTROL;
			pgc2v_mode->clut_ctrl.name	= "alut control";
		}
		if (dpm->flags & KGI_DPF_CH_ILUT) {

			pgc2v_mode->clut_ctrl.type	= KGI_RT_ILUT_CONTROL;
			pgc2v_mode->clut_ctrl.name	= "ilut control";
		}
		pgc2v_mode->clut_ctrl.Set	= pgc2v_ramdac_clut_set;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}
}

kgi_resource_t *pgc2v_ramdac_image_resource(pgc2v_ramdac_t *pgc2v,
	pgc2v_ramdac_mode_t *pgc2v_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));

	switch (index) {

	case 0:	return (kgi_resource_t *) &(pgc2v_mode->clut_ctrl);
	case 1:	return (kgi_resource_t *) &(pgc2v_mode->ptr_ctrl);

	}
	return NULL;
}

void pgc2v_ramdac_mode_enter(pgc2v_ramdac_t *pgc2v, pgc2v_ramdac_io_t *pgc2v_io,
	pgc2v_ramdac_mode_t *pgc2v_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t x, y;

	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_DAC07_AutoIncrement, 0x07);

	PGC2v_DAC_OUT8(pgc2v_io, 0x00, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, 0x00, PGC2v_DAC_IndexHigh);

#define	PGC2v_SETUP(register)	\
	PGC2v_DAC_OUT8(pgc2v_io, pgc2v_mode->RD.register, PGC2v_DAC_IndexData);

	PGC2v_SETUP(MiscControl);
	PGC2v_SETUP(SyncControl);
	PGC2v_SETUP(DACControl);
	PGC2v_SETUP(PixelSize);
	PGC2v_SETUP(ColorFormat);
	PGC2v_SETUP(CursorMode);
	PGC2v_SETUP(CursorControl);
	PGC2v_SETUP(CursorXLow);
	PGC2v_SETUP(CursorXHigh);
	PGC2v_SETUP(CursorYLow);
	PGC2v_SETUP(CursorYHigh);
	PGC2v_SETUP(CursorHotSpotX);
	PGC2v_SETUP(CursorHotSpotY);
	PGC2v_SETUP(OverlayKey);
	PGC2v_SETUP(Pan);

#undef	PGC2v_SETUP
 
	PGC2v_DAC_OUT8(pgc2v_io, PGC2v_EDAC_CheckControl, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, 0, PGC2v_DAC_IndexData);

/*	kgi_u8_t cursor_clut[PGC2v_EDAC_CursorPaletteSize];
**	kgi_u8_t cursor_data[PGC2v_EDAC_CursorPatternSize];
**	kgi_u8_t clut[3*256];
*/
}
