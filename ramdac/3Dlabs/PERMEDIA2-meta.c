/* -----------------------------------------------------------------------------
**	PERMEDIA2 integrated DAC meta language implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2-meta.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:56:15  seeger_s
**	- implemented CLUT control and hardware cursor support
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:03  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/3Dlabs/PERMEDIA2.h"
#include "ramdac/3Dlabs/PERMEDIA2-meta.h"

/*
**	I/O helper functions
*/
static inline void PGC_EDAC_OUT8(pgc_ramdac_io_t *pgc_io, 
	kgi_u8_t val, kgi_u8_t reg)
{
	PGC_DAC_OUT8(pgc_io, reg, PGC_DAC_EXT_ADDR);
	PGC_DAC_OUT8(pgc_io, val, PGC_DAC_EXT_DATA);
}

static inline kgi_u8_t PGC_EDAC_IN8(pgc_ramdac_io_t *pgc_io, kgi_u8_t reg)
{
	PGC_DAC_OUT8(pgc_io, reg, PGC_DAC_EXT_ADDR);
	return PGC_DAC_IN8(pgc_io, PGC_DAC_EXT_DATA);
}


#define	PGC_MAGIC	0x0101
#define	PGC_SHIFT	8

struct pgc_ramdac_mode_record_s
{
	kgi_u8_t		ext40;
	kgi_u8_t		ext18;

	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_ratio_t		best_lclk;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

static const pgc_ramdac_mode_record_t pgc_ramdac_mode[] =
{
	{ 0x00, 0x00, 1, 0, {1,1},	KGI_AM_TEXT, KGI_AS_448,	1 },

	{ 0x00, 0x10, 1, 0, {1,4},	KGI_AM_I,    KGI_AS_8,		1 },
	{ 0x10, 0xB1, 0, 1, {1,4},	KGI_AM_RGB,  KGI_AS_332,	1 },
	{ 0x10, 0xB3, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_2321,	1 },
	{ 0x10, 0xB3, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_2321,	1 },

	{ 0x10, 0xB4, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_5551,	1 },
	{ 0x10, 0xB4, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_5551,	1 },
	{ 0x10, 0xB5, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_4444,	1 },
	{ 0x10, 0xB5, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_4444,	1 },
	{ 0x10, 0xB6, 0, 1, {1,4},	KGI_AM_RGB,  KGI_AS_565,	1 },

	{ 0x10, 0xB8, 0, 1, {1,2},	KGI_AM_RGBA, KGI_AS_8888,	1 },
	{ 0x10, 0xB8, 0, 1, {1,2},	KGI_AM_RGBX, KGI_AS_8888,	1 },
	{ 0x10, 0xB9, 0, 1, {3,8},	KGI_AM_RGB,  KGI_AS_888,	1 }
};
#define	PGC_RAMDAC_NR_MODES (sizeof(pgc_ramdac_mode)/sizeof(pgc_ramdac_mode[0]))

static const pgc_ramdac_mode_record_t pgc_ramdac_mode_RGBP5551 =
{
	0x10, 0xB4, 0, 1, {1,2}, KGI_AM_RGBP, KGI_AS_AAA2, 2
};

static inline const pgc_ramdac_mode_record_t *pgc_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	if (((KGI_AM_PRIVATE | KGI_AM_RGB) == fam) &&
		(kgim_strcmp(KGI_AS_5551, bpfa) == 0)) {

		return &pgc_ramdac_mode_RGBP5551;
	}

	/* !!!	handle BGR modes */
	for (i = 0; i < PGC_RAMDAC_NR_MODES; i++) {

		if (pgc_ramdac_mode[i].dam != fam) {

			continue;
		}
		if (kgim_strcmp(pgc_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}
		break;
	}
	return (i == PGC_RAMDAC_NR_MODES) ? NULL : pgc_ramdac_mode + i;
}




#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d)	((dpm->rclk.mul == m) && (dpm->rclk.div == d))

/* ----------------------------------------------------------------------------
**	image resources
** ----------------------------------------------------------------------------
*/

static void pgc_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	pgc_ramdac_mode_t *pgc_mode = clut->meta;
	pgc_ramdac_io_t   *pgc_io   = clut->meta_io;
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(table == 0);
	KRN_ASSERT(count > 0);

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {

		if (am & KGI_AM_COLOR1) {

			pgc_mode->clut[dst+0] = data[src++] >> PGC_SHIFT;
		}
		if (am & KGI_AM_COLOR2) {

			pgc_mode->clut[dst+1] = data[src++] >> PGC_SHIFT;
		}
		if (am & KGI_AM_COLOR3) {

			pgc_mode->clut[dst+2] = data[src++] >> PGC_SHIFT;
		}
		dst += 3;
	}

	PGC_DAC_OUT8(pgc_io, index, PGC_DAC_PW_INDEX);
	PGC_DAC_OUTS8(pgc_io, PGC_DAC_P_DATA,
		pgc_mode->clut + 3*index, 3*count);
}


static void pgc_ramdac_ptr_set_mode(kgi_marker_t *ptr, kgi_marker_mode_t mode)
{
	pgc_ramdac_mode_t *pgc_mode = ptr->meta;
	pgc_ramdac_io_t	*pgc_io = ptr->meta_io;

	pgc_mode->ext06 = PGC_EDAC06_Cursor64x64;
	switch (mode) {

	case KGI_MM_WINDOWS:
		pgc_mode->ext06 |= PGC_EDAC06_CursorXGA;
		break;

	case KGI_MM_X11:
		pgc_mode->ext06 |= PGC_EDAC06_CursorX;
		break;

	case KGI_MM_3COLOR:
		pgc_mode->ext06 |= PGC_EDAC06_CursorThreeColor;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext06, 0x06);
}

static void pgc_ramdac_ptr_set_shape(kgi_marker_t *ptr, kgi_u_t shape,
	kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
	const kgi_rgb_color_t *color)
{
	pgc_ramdac_mode_t *pgc_mode = ptr->meta;
	pgc_ramdac_io_t   *pgc_io   = ptr->meta_io;
	const kgi_u8_t *mask = data;
	kgi_u_t i;

	pgc_mode->ptr_hot.x = hot_x;
	pgc_mode->ptr_hot.y = hot_y;

	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext06, 0x06);
        PGC_DAC_OUT8(pgc_io, 0x00, 0x00); /* Reset A0 - A7 */

	/*	set pattern
	*/
        for (i = 0; i < 512; i++) {

        	PGC_DAC_OUT8(pgc_io, mask[i], PGC_DAC_CURSOR_RAM_DATA);
        }
        for (i = 0; i < 512; i++) {

        	PGC_DAC_OUT8(pgc_io, mask[i+512], PGC_DAC_CURSOR_RAM_DATA);
        }

	/*	set colors
	*/
	PGC_DAC_OUT8(pgc_io, 1, PGC_DAC_CW_INDEX);
	for (i = 0; i < 3; i++) {

		PGC_DAC_OUT8(pgc_io, color[i].r >> PGC_SHIFT, PGC_DAC_C_DATA);
		PGC_DAC_OUT8(pgc_io, color[i].g >> PGC_SHIFT, PGC_DAC_C_DATA);
		PGC_DAC_OUT8(pgc_io, color[i].b >> PGC_SHIFT, PGC_DAC_C_DATA);
	}
}

static void pgc_ramdac_ptr_show(kgi_marker_t *ptr, kgi_u_t x, kgi_u_t y)
{
	pgc_ramdac_mode_t *pgc_mode = ptr->meta;
	pgc_ramdac_io_t   *pgc_io   = ptr->meta_io;

	x += pgc_mode->ptr_shift.x - pgc_mode->ptr_hot.x;
	y += pgc_mode->ptr_shift.y - pgc_mode->ptr_hot.y;

	PGC_DAC_OUT8(pgc_io, x,      PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, x >> 8, PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, y,      PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, y >> 8, PGC_DAC_CURSOR_YH);
}

static void pgc_ramdac_ptr_hide(kgi_marker_t *ptr)
{
	pgc_ramdac_mode_t *pgc_mode = ptr->meta;
	pgc_ramdac_io_t   *pgc_io   = ptr->meta_io;

	PGC_DAC_OUT8(pgc_io, 0xFF, PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, 0x07, PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, 0xFF, PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, 0x07, PGC_DAC_CURSOR_YH);
}

#define	pgc_ramdac_ptr_undo	NULL


kgi_error_t pgc_ramdac_init(pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io,
	const kgim_options_t *options)
{
	pgc->EXT.CursorControl = PGC_EDAC_IN8(pgc_io, PGC_EDAC_CursorControl);
	pgc->EXT.ColorMode     = PGC_EDAC_IN8(pgc_io, PGC_EDAC_ColorMode);
	pgc->EXT.ModeControl   = PGC_EDAC_IN8(pgc_io, PGC_EDAC_ModeControl);
	pgc->EXT.PalettePage   = PGC_EDAC_IN8(pgc_io, PGC_EDAC_PalettePage);
	pgc->EXT.MiscControl   = PGC_EDAC_IN8(pgc_io, PGC_EDAC_MiscControl);
	pgc->EXT.ColorKeyControl=PGC_EDAC_IN8(pgc_io, PGC_EDAC_ColorKeyControl);
	pgc->EXT.OverlayKey    = PGC_EDAC_IN8(pgc_io, PGC_EDAC_OverlayKey);
	pgc->EXT.RedKey        = PGC_EDAC_IN8(pgc_io, PGC_EDAC_RedKey);
	pgc->EXT.GreenKey      = PGC_EDAC_IN8(pgc_io, PGC_EDAC_GreenKey);
	pgc->EXT.BlueKey       = PGC_EDAC_IN8(pgc_io, PGC_EDAC_BlueKey);

	PGC_DAC_OUT8(pgc_io, 0, PGC_DAC_PR_INDEX);
	PGC_DAC_INS8(pgc_io, PGC_DAC_P_DATA, pgc->clut, sizeof(pgc->clut));

/*	kgi_u8_t cursor_palette[3*4];
**	kgi_u8_t cursor_data[];
*/
	pgc->CursorXLow	 = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_XL);
	pgc->CursorXHigh = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_XH);
	pgc->CursorYLow	 = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_YL);
	pgc->CursorYHigh = PGC_DAC_IN8(pgc_io, PGC_DAC_CURSOR_YH);

	return KGI_EOK;
}

void pgc_ramdac_done(pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io, 
	const kgim_options_t *options)
{
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.CursorControl,  PGC_EDAC_CursorControl);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.ColorMode,      PGC_EDAC_ColorMode);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.ModeControl,    PGC_EDAC_ModeControl);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.PalettePage,    PGC_EDAC_PalettePage);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.MiscControl,    PGC_EDAC_MiscControl);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.OverlayKey,     PGC_EDAC_OverlayKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.RedKey,         PGC_EDAC_RedKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.GreenKey,       PGC_EDAC_GreenKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.BlueKey,        PGC_EDAC_BlueKey);
	PGC_EDAC_OUT8(pgc_io, pgc->EXT.ColorKeyControl,
		PGC_EDAC_ColorKeyControl);

	PGC_DAC_OUT8(pgc_io, 0, PGC_DAC_PW_INDEX);
	PGC_DAC_OUTS8(pgc_io, PGC_DAC_P_DATA, pgc->clut, sizeof(pgc->clut));

/*
	cursor_palette[3*4];
	cursor_data[];
*/
	PGC_DAC_OUT8(pgc_io, pgc->CursorXLow,	PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, pgc->CursorXHigh,	PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, pgc->CursorYLow,	PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, pgc->CursorYHigh,	PGC_DAC_CURSOR_YH);
}

kgi_error_t pgc_ramdac_mode_check(
	pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io, pgc_ramdac_mode_t *pgc_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const pgc_ramdac_mode_record_t *rec;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = pgc_ramdac_mode_record(img[0].fam, img[0].bpfa);
	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:
		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;

		dpm->lclk = rec->best_lclk;
		dpm->rclk.mul = dpm->rclk.div = 1;
		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames) {

			img[0].frames = rec->frames;
		}

		KRN_DEBUG(2, " lclk %i:%i, dclk %i:%i", dpm->lclk.mul,
			dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk < pgc->ramdac.lclk.min) ||
			(dpm->dclk < pgc->ramdac.dclk.min)) {

			KRN_DEBUG(1, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk > pgc->ramdac.lclk.max) || 
			(dpm->dclk > pgc->ramdac.dclk.max)) {

			register kgi_u_t dclk = pgc->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < pgc->ramdac.dclk.max)
				? dclk : pgc->ramdac.dclk.max;
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > pgc->ramdac.lclk.max) || 
			(dpm->dclk > pgc->ramdac.dclk.max)) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk < pgc->ramdac.lclk.min) ||
			(dpm->dclk < pgc->ramdac.dclk.min)) {

			register kgi_u_t dclk = pgc->ramdac.dclk.min *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > pgc->ramdac.dclk.min)
				? dclk : pgc->ramdac.dclk.min;
		}
		return KGI_EOK;

     case KGI_TC_CHECK:

		if (! ((dpm->lclk.mul == rec->best_lclk.mul) &&
			(dpm->lclk.div == rec->best_lclk.div))) {

			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		if ((dpm->dclk < pgc->ramdac.dclk.min) ||
			(pgc->ramdac.dclk.max < dpm->dclk) ||
			(lclk < pgc->ramdac.lclk.min) ||
			(pgc->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(1, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (pgc_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
			KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		pgc_mode->rec = rec;

		pgc_mode->ext1E = PGC_EDAC1E_8BitEnable;
		if (pgc_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN) {

			pgc_mode->ext1E |= PGC_EDAC1E_SyncOnGreen;
		}
		if (pgc_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

			pgc_mode->ext1E |= PGC_EDAC1E_BlankPedestral;
		}

		/*	hardware pointer setup
		*/
		pgc_mode->ext06 = PGC_EDAC06_Cursor64x64 | PGC_EDAC06_CursorOff;

		pgc_mode->ptr_shift.x	 = 64 +
			pgc_mode->kgim.crt->x.total -
			pgc_mode->kgim.crt->x.blankend;
		pgc_mode->ptr_shift.y	 = 64 +
			pgc_mode->kgim.crt->y.total -
			pgc_mode->kgim.crt->y.blankend;

		pgc_mode->ptr_ctrl.meta		= pgc_mode;
		pgc_mode->ptr_ctrl.meta_io	= pgc_io;
		pgc_mode->ptr_ctrl.type		= KGI_RT_POINTER_CONTROL;
		pgc_mode->ptr_ctrl.prot		= KGI_PF_DRV_RWS;
		pgc_mode->ptr_ctrl.name		= "pointer";
		pgc_mode->ptr_ctrl.modes	=
			KGI_MM_WINDOWS | KGI_MM_X11 | KGI_MM_3COLOR;
		pgc_mode->ptr_ctrl.shapes	= 1;
		pgc_mode->ptr_ctrl.size.x	= 64;
		pgc_mode->ptr_ctrl.size.y	= 64;
		pgc_mode->ptr_ctrl.SetMode	= pgc_ramdac_ptr_set_mode;
		pgc_mode->ptr_ctrl.Select	= NULL;
		pgc_mode->ptr_ctrl.SetShape	= pgc_ramdac_ptr_set_shape;
		pgc_mode->ptr_ctrl.Show		= pgc_ramdac_ptr_show;
		pgc_mode->ptr_ctrl.Hide		= pgc_ramdac_ptr_hide;
		pgc_mode->ptr_ctrl.Undo		= pgc_ramdac_ptr_undo;
#warning init pgc_mode->cursor_clut
#warning init pgc_mode->cursor_data

		/*	clut setup
		*/
		pgc_mode->clut_ctrl.meta	= pgc_mode;
		pgc_mode->clut_ctrl.meta_io	= pgc_io;
		pgc_mode->clut_ctrl.prot	= KGI_PF_DRV_RWS;
		if (dpm->flags & KGI_DPF_CH_ALUT) {

			pgc_mode->clut_ctrl.type = KGI_RT_ALUT_CONTROL;
			pgc_mode->clut_ctrl.name = "alut control";
		}
		if (dpm->flags & KGI_DPF_CH_ILUT) {

			pgc_mode->clut_ctrl.type = KGI_RT_ILUT_CONTROL;
			pgc_mode->clut_ctrl.name = "ilut control";
		}
		pgc_mode->clut_ctrl.Set = pgc_ramdac_set_clut;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}
}

kgi_resource_t *pgc_ramdac_image_resource(pgc_ramdac_t *pgc,
	pgc_ramdac_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t image,
	kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));

	switch (index) {

	case 0:		return (kgi_resource_t *) &(pgc_mode->clut_ctrl);
	case 1:		return (kgi_resource_t *) &(pgc_mode->ptr_ctrl);

	}
	return NULL;
}

void pgc_ramdac_mode_enter(pgc_ramdac_t *pgc, pgc_ramdac_io_t *pgc_io,
	pgc_ramdac_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	PGC_EDAC_OUT8(pgc_io, pgc_mode->rec->ext40, 0x40);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_OverlayKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_RedKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_GreenKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_BlueKey);
	PGC_EDAC_OUT8(pgc_io, 0, PGC_EDAC_PalettePage);

/*
	clut
	cursor clut
	cursor shape
*/
	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext06, 0x06);
	PGC_DAC_OUT8(pgc_io, 0xFF, PGC_DAC_CURSOR_XL);
	PGC_DAC_OUT8(pgc_io, 0x07, PGC_DAC_CURSOR_XH);
	PGC_DAC_OUT8(pgc_io, 0xFF, PGC_DAC_CURSOR_YL);
	PGC_DAC_OUT8(pgc_io, 0x07, PGC_DAC_CURSOR_YH);

	PGC_EDAC_OUT8(pgc_io, pgc_mode->rec->ext18, 0x18);
	PGC_EDAC_OUT8(pgc_io, PGC_EDAC19_FrontBuffer, 0x19);
	PGC_EDAC_OUT8(pgc_io, pgc_mode->ext1E, 0x1E);
}
