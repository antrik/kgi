/* ----------------------------------------------------------------------------
**	nVidia TNT2 ramdac meta implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: TNT2-meta.c,v $
**	Revision 1.1.1.1  2000/09/21 10:06:40  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:06  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/nVidia/TNT2.h"
#include "ramdac/nVidia/TNT2-meta.h"

#if 0
/* I/O helper functions */
static inline void TNT2_EDAC_OUT8(tnt2_ramdac_io_t *tnt2_io,
	kgi_u8_t val, kgi_u8_t reg)
{
	TNT2_DAC_OUT8(tnt2_io, reg, TNT2_DAC_EXT_ADDR);
	TNT2_DAC_OUT8(tnt2_io, val, TNT2_DAC_EXT_DATA);
}

static inline kgi_u8_t TNT2_EDAC_IN8(tnt2_ramdac_io_t *tnt2_io, kgi_u8_t reg)
{
	TNT2_DAC_OUT8(tnt2_io, reg, TNT2_DAC_EXT_ADDR);
	return TNT2_DAC_IN8(tnt2_io, TNT2_DAC_EXT_DATA);
}
#endif

#define	TNT2_MAGIC	0x0101
#define	TNT2_SHIFT	8

struct tnt2_ramdac_mode_record_s
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

static const tnt2_ramdac_mode_record_t tnt2_ramdac_mode[] =
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

#define	TNT2_RAMDAC_NR_MODES (sizeof(tnt2_ramdac_mode) / sizeof(tnt2_ramdac_mode[0]))

static const tnt2_ramdac_mode_record_t tnt2_ramdac_mode_RGBP5551 =
{
	0x10, 0xB4, 0, 1, {1,2}, KGI_AM_RGBP, KGI_AS_AAA2, 2
};

static inline const tnt2_ramdac_mode_record_t *tnt2_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	KRN_DEBUG(2, "tnt2_ramdac_mode_record()");
	
	if (((KGI_AM_PRIVATE | KGI_AM_RGB) == fam) &&
		(kgim_strcmp(KGI_AS_5551, bpfa) == 0)) {

		return &tnt2_ramdac_mode_RGBP5551;
	}

	/* !!!	handle BGR modes
	*/
	for (i = 0; i < TNT2_RAMDAC_NR_MODES; i++) {

		if (tnt2_ramdac_mode[i].dam != fam) {

			continue;
		}
		
		if (kgim_strcmp(tnt2_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}
		
		break;
	}
	
	if (i == TNT2_RAMDAC_NR_MODES) {

		KRN_DEBUG(1, "Couldn't find a ramdac mode");
	}
	
	return (i == TNT2_RAMDAC_NR_MODES) ? NULL : tnt2_ramdac_mode + i;
}

#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d)	((dpm->rclk.mul == m) && (dpm->rclk.div == d))

/* ----	text16 context functions -------------------------------------------- */
#define	TNT2_RAMDAC_IO(ctx)	\
	KGIM_SUBSYSTEM_IO((kgim_display_t *) ctx->meta_object, ramdac)

#define	TNT2_RAMDAC_MODE(ctx)	\
	KGIM_SUBSYSTEM_MODE((kgim_display_mode_t *) ctx->meta_mode, ramdac)

#if 0
static void tnt2_text16_set_ilut(kgic_mode_text16context_t *ctx, kgic_ilut_entries_t *ilut)
{
	tnt2_ramdac_io_t   *tnt2_io   = TNT2_RAMDAC_IO(ctx);
	tnt2_ramdac_mode_t *tnt2_mode = TNT2_RAMDAC_MODE(ctx);
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(ilut->img == 0);
	KRN_ASSERT(ilut->lut == 0);
	KRN_ASSERT(ilut->cnt > 0);

	cnt = ilut->cnt;
	dst = ilut->cnt * 3;
	src = 0;

	while (cnt--) {

		if (ilut->am & KGI_AM_COLOR1) {

			tnt2_mode->clut[dst+0] =
				ilut->data[src++] >> TNT2_SHIFT;
		}
		
		if (ilut->am & KGI_AM_COLOR2) {

			tnt2_mode->clut[dst+1] = 
				ilut->data[src++] >> TNT2_SHIFT;
		}
		
		if (ilut->am & KGI_AM_COLOR3) {

			tnt2_mode->clut[dst+2] = 
				ilut->data[src++] >> TNT2_SHIFT;
		}
		
		dst += 3;
	}

	src = 3 * ilut->idx;
	cnt = 3 * ilut->cnt;
	TNT2_DAC_OUT8(tnt2_io, ilut->idx, TNT2_DAC_PW_INDEX);
	TNT2_DAC_OUTS8(tnt2_io, TNT2_DAC_P_DATA, tnt2_mode->clut + src, cnt);
}


void tnt2_text16_hp_set_shape(kgic_mode_text16context_t *ctx,
	kgi_pointer64x64 *ptr)
{
	tnt2_ramdac_io_t   *tnt2_io   = tnt2_RAMDAC_IO(ctx);
	tnt2_ramdac_mode_t *tnt2_mode = tnt2_RAMDAC_MODE(ctx);
	kgi_u_t i;

	tnt2_mode->ptr.hot.x = ptr->hotx;
	tnt2_mode->ptr.hot.y = ptr->hoty;

	/*	Update private state
	*/
	tnt2_mode->ext06 = TNT2_EDAC06_Cursor64x64;
	
	switch (ptr->mode) {

	case KGI_PM_WINDOWS:
		tnt2_mode->ext06 |= TNT2_EDAC06_CursorXGA;
		break;

	case KGI_PM_X11:
		tnt2_mode->ext06 |= TNT2_EDAC06_CursorX;
		break;

	case KGI_PM_THREE_COLOR:
		tnt2_mode->ext06 |= TNT2_EDAC06_CursorThreeColor;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	
	TNT2_EDAC_OUT8(tnt2_io, tnt2_mode->ext06, 0x06);
        TNT2_DAC_OUT8(tnt2_io, 0x00, 0x00); /* Reset A0 - A7 */

	/*	Set pattern
	*/
        for (i = 0; i < 512; i++) {

        	TNT2_DAC_OUT8(tnt2_io, ptr->xor_mask[i], 
			TNT2_DAC_CURSOR_RAM_DATA);
        }
	
        for (i = 0; i < 512; i++) {

        	TNT2_DAC_OUT8(tnt2_io, ptr->and_mask[i],
			TNT2_DAC_CURSOR_RAM_DATA);
        }

	/*	Set colors
	*/
	TNT2_DAC_OUT8(tnt2_io, 1, TNT2_DAC_CW_INDEX);
	
	for (i = 0; i < 3; i++) {

		TNT2_DAC_OUT8(tnt2_io, ptr->col[i].r >> SHIFT, TNT2_DAC_C_DATA);
		TNT2_DAC_OUT8(tnt2_io, ptr->col[i].g >> SHIFT, TNT2_DAC_C_DATA);
		TNT2_DAC_OUT8(tnt2_io, ptr->col[i].b >> SHIFT, TNT2_DAC_C_DATA);
	}
}

static void tnt2_text16_hp_show(kgic_mode_text16context_t *ctx,
	kgi_u_t x, kgi_u_t y)
{
	tnt2_ramdac_io_t   *tnt2_io   = TNT2_RAMDAC_IO(ctx);
	tnt2_ramdac_mode_t *tnt2_mode = TNT2_RAMDAC_MODE(ctx);

	KRN_DEBUG(1, "tnt2_text16_hp_show()");
	
	if ((tnt2_mode->ptr.pos.x != x) || (tnt2_mode->ptr.pos.y != y)) {

		tnt2_mode->ptr.pos.x = x;
		tnt2_mode->ptr.pos.y = y;

		x += tnt2_mode->ptr.shift.x - tnt2_mode->ptr.hot.x;
		y += tnt2_mode->ptr.shift.y - tnt2_mode->ptr.hot.y;
#if 0
		TNT2_DAC_OUT8(tnt2_io, x,      TNT2_DAC_CURSOR_XL);
		TNT2_DAC_OUT8(tnt2_io, x >> 8, TNT2_DAC_CURSOR_XH);
		TNT2_DAC_OUT8(tnt2_io, y,      TNT2_DAC_CURSOR_YL);
		TNT2_DAC_OUT8(tnt2_io, y >> 8, TNT2_DAC_CURSOR_YH);
#endif
	}
}

static void tnt2_text16_hp_hide(kgic_mode_text16context_t *ctx)
{
	tnt2_ramdac_io_t   *tnt2_io   = TNT2_RAMDAC_IO(ctx);
	tnt2_ramdac_mode_t *tnt2_mode = TNT2_RAMDAC_MODE(ctx);

	KRN_DEBUG(1, "tnt2_text16_hp_hide()");
	
	tnt2_mode->ptr.pos.x = tnt2_mode->ptr.pos.y = 0x7FF;
#if 0
	TNT2_DAC_OUT8(tnt2_io, 0xFF, TNT2_DAC_CURSOR_XL);
	TNT2_DAC_OUT8(tnt2_io, 0x07, TNT2_DAC_CURSOR_XH);
	TNT2_DAC_OUT8(tnt2_io, 0xFF, TNT2_DAC_CURSOR_YL);
	TNT2_DAC_OUT8(tnt2_io, 0x07, TNT2_DAC_CURSOR_YH);
#endif
}
#endif

#undef	TNT2_RAMDAC_MODE
#undef	TNT2_RAMDAC_IO
/* ---- end of text16 context functions ----------------------------------- */

kgi_error_t tnt2_ramdac_init(tnt2_ramdac_t *tnt2, tnt2_ramdac_io_t *tnt2_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(1, "tnt2_ramdac_init()");

#warning save initial state here! -- seeger_s 
#if 0
	tnt2->EXT.CursorControl = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_CursorControl);
	tnt2->EXT.ColorMode     = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_ColorMode);
	tnt2->EXT.ModeControl   = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_ModeControl);
	tnt2->EXT.PalettePage   = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_PalettePage);
	tnt2->EXT.MiscControl   = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_MiscControl);
	tnt2->EXT.ColorKeyControl=TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_ColorKeyControl);
	tnt2->EXT.OverlayKey    = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_OverlayKey);
	tnt2->EXT.RedKey        = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_RedKey);
	tnt2->EXT.GreenKey      = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_GreenKey);
	tnt2->EXT.BlueKey       = TNT2_EDAC_IN8(tnt2_io, TNT2_EDAC_BlueKey);

	TNT2_DAC_OUT8(tnt2_io, 0, TNT2_DAC_PR_INDEX);
	TNT2_DAC_INS8(tnt2_io, TNT2_DAC_P_DATA, tnt2->clut, sizeof(tnt2->clut));

/*	kgi_u8_t cursor_palette[3*4];
**	kgi_u8_t cursor_data[];
*/
	tnt2->CursorXLow	 = TNT2_DAC_IN8(tnt2_io, TNT2_DAC_CURSOR_XL);
	tnt2->CursorXHigh = TNT2_DAC_IN8(tnt2_io, TNT2_DAC_CURSOR_XH);
	tnt2->CursorYLow	 = TNT2_DAC_IN8(tnt2_io, TNT2_DAC_CURSOR_YL);
	tnt2->CursorYHigh = TNT2_DAC_IN8(tnt2_io, TNT2_DAC_CURSOR_YH);
#endif

	return KGI_EOK;
}

void tnt2_ramdac_done(tnt2_ramdac_t *tnt, tnt2_ramdac_io_t *tnt2_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(1, "tnt2_ramdac_done()");
	
#if 0
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.CursorControl,  TNT2_EDAC_CursorControl);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.ColorMode,      TNT2_EDAC_ColorMode);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.ModeControl,    TNT2_EDAC_ModeControl);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.PalettePage,    TNT2_EDAC_PalettePage);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.MiscControl,    TNT2_EDAC_MiscControl);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.OverlayKey,     TNT2_EDAC_OverlayKey);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.RedKey,         TNT2_EDAC_RedKey);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.GreenKey,       TNT2_EDAC_GreenKey);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.BlueKey,        TNT2_EDAC_BlueKey);
	TNT2_EDAC_OUT8(tnt2_io, tnt2->EXT.ColorKeyControl,TNT2_EDAC_ColorKeyControl);

	TNT2_DAC_OUT8(tnt2_io, 0, TNT2_DAC_PW_INDEX);
	TNT2_DAC_OUTS8(tnt2_io, TNT2_DAC_P_DATA, tnt2->clut, sizeof(tnt2->clut));

/*
	cursor_palette[3*4];
	cursor_data[];
*/
	TNT2_DAC_OUT8(tnt2_io, tnt2->CursorXLow,	TNT2_DAC_CURSOR_XL);
	TNT2_DAC_OUT8(tnt2_io, tnt2->CursorXHigh,	TNT2_DAC_CURSOR_XH);
	TNT2_DAC_OUT8(tnt2_io, tnt2->CursorYLow,	TNT2_DAC_CURSOR_YL);
	TNT2_DAC_OUT8(tnt2_io, tnt2->CursorYHigh,	TNT2_DAC_CURSOR_YH);
#endif
}

kgi_error_t tnt2_ramdac_mode_check(tnt2_ramdac_t *tnt2,
	tnt2_ramdac_io_t *tnt2_io, tnt2_ramdac_mode_t *tnt2_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const tnt2_ramdac_mode_record_t *rec;

	KRN_DEBUG(1, "tnt2_ramdac_mode_check()");
	
	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = tnt2_ramdac_mode_record(img[0].fam, img[0].bpfa);
	
	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE:");
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

		KRN_DEBUG(3, "lclk %i:%i, dclk %i:%i", dpm->lclk.mul, 
			dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(3, "KGI_TC_LOWER:");

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk < tnt2->ramdac.lclk.min) || 
			(dpm->dclk < tnt2->ramdac.dclk.min)) {

			KRN_DEBUG(1, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		
		if ((lclk > tnt2->ramdac.lclk.max) ||
			(dpm->dclk > tnt2->ramdac.dclk.max)) {

			register kgi_u_t 
			dclk = tnt2->ramdac.lclk.max * 
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < tnt2->ramdac.dclk.max) 
				? dclk : tnt2->ramdac.dclk.max;
		}
		
		return KGI_EOK;

	case KGI_TC_RAISE:
		KRN_DEBUG(3, "KGI_TC_RAISE:");

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > tnt2->ramdac.lclk.max) ||
			(dpm->dclk > tnt2->ramdac.dclk.max)) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		
		if ((lclk < tnt2->ramdac.lclk.min) ||
			(dpm->dclk < tnt2->ramdac.dclk.min)) {

			register kgi_u_t 
			dclk = tnt2->ramdac.dclk.min * 
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > tnt2->ramdac.dclk.min) 
				? dclk : tnt2->ramdac.dclk.min;
		}
		
		return KGI_EOK;

     case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK:");

		if (!  ((dpm->lclk.mul == rec->best_lclk.mul) &&
			(dpm->lclk.div == rec->best_lclk.div))) {

			KRN_DEBUG(1, "Unsupported lclk mul or div");
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		
		if ((dpm->dclk < tnt2->ramdac.dclk.min) ||
			(tnt2->ramdac.dclk.max < dpm->dclk) ||
			(lclk < tnt2->ramdac.lclk.min) || 
			(tnt2->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(1, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (tnt2_mode->kgim.crt->sync &
			(KGIM_ST_SYNC_ON_RED | KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		tnt2_mode->rec = rec;

/*		tnt2_mode->ext1E = tnt2_EDAC1E_8BitEnable;
*/		
		if (tnt2_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN) {

/*			tnt2_mode->ext1E |= TNT2_EDAC1E_SyncOnGreen;
*/		}
		
		if (tnt2_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

/*			tnt2_mode->ext1E |= TNT2_EDAC1E_BlankPedestral;
*/		}

/*		tnt2_mode->ext06 = TNT2_EDAC06_Cursor64x64 | 
**			TNT2_EDAC06_CursorOff;
*/		tnt2_mode->ptr.pos.x = tnt2_mode->ptr.pos.y = 0x7FF;
		tnt2_mode->ptr.hot.x = tnt2_mode->ptr.hot.y = 0;
		tnt2_mode->ptr.shift.x = 64 + 
			tnt2_mode->kgim.crt->x.total -
			tnt2_mode->kgim.crt->x.blankend;
		tnt2_mode->ptr.shift.y = 64 + 
			tnt2_mode->kgim.crt->y.total -
			tnt2_mode->kgim.crt->y.blankend;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}
}

void tnt2_ramdac_mode_enter(tnt2_ramdac_t *tnt2, tnt2_ramdac_io_t *tnt2_io,
	tnt2_ramdac_mode_t *tnt2_mode, kgi_image_mode_t *img, kgi_u_t images)
{
/*	kgi_u_t x, y;
*/
	KRN_DEBUG(1, "tnt2_ramdac_mode_enter()");
	
#if 0	
	TNT2_EDAC_OUT8(tnt2_io, tnt2_mode->rec->ext40, 0x40);
	TNT2_EDAC_OUT8(tnt2_io, 0, TNT2_EDAC_OverlayKey);
	TNT2_EDAC_OUT8(tnt2_io, 0, TNT2_EDAC_RedKey);
	TNT2_EDAC_OUT8(tnt2_io, 0, TNT2_EDAC_GreenKey);
	TNT2_EDAC_OUT8(tnt2_io, 0, TNT2_EDAC_BlueKey);
	TNT2_EDAC_OUT8(tnt2_io, 0, TNT2_EDAC_PalettePage);

	x = tnt2_mode->ptr.pos.x + tnt2_mode->ptr.shift.x - 
		tnt2_mode->ptr.hot.x;
	y = tnt2_mode->ptr.pos.y + tnt2_mode->ptr.shift.y - 
		tnt2_mode->ptr.hot.y;

/*
	clut
	cursor clut
	cursor shape
*/
	TNT2_EDAC_OUT8(tnt2_io, tnt2_mode->ext06, 0x06);
	TNT2_DAC_OUT8(tnt2_io, x,      TNT2_DAC_CURSOR_XL);
	TNT2_DAC_OUT8(tnt2_io, x >> 8, TNT2_DAC_CURSOR_XH);
	TNT2_DAC_OUT8(tnt2_io, y,      TNT2_DAC_CURSOR_YL);
	TNT2_DAC_OUT8(tnt2_io, y >> 8, TNT2_DAC_CURSOR_YH);

	TNT2_EDAC_OUT8(tnt2_io, tnt2_mode->rec->ext18, 0x18);
	TNT2_EDAC_OUT8(tnt2_io, TNT2_EDAC19_FrontBuffer, 0x19);
	TNT2_EDAC_OUT8(tnt2_io, tnt2_mode->ext1E, 0x1E);
#endif
}
