/* -----------------------------------------------------------------------------
**	TI tvp3026 ramdac implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: TVP3026-meta.c,v $
**	Revision 1.2  2003/02/05 19:09:30  foske
**	Update the driver to match current kgi core.
**	
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.4  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.3  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.2  2000/04/26 14:07:08  seeger_s
**	- minor cleanups
**	
**	Revision 1.1.1.1  2000/04/18 08:51:05  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/TI/TVP3026.h"
#include "ramdac/TI/TVP3026-meta.h"

/*
**	I/O helper functions
*/
static inline void TVP3026_EDAC_OUT8(tvp3026_ramdac_io_t *tvp3026_io, 
	kgi_u8_t val, kgi_u8_t reg)
{
	KRN_DEBUG(3, "%.2x -> EDAC%.2X", val, reg);
	TVP3026_DAC_OUT8(tvp3026_io, reg, TVP3026_EDAC_ADDR);
        TVP3026_DAC_OUT8(tvp3026_io, val, TVP3026_EDAC_DATA);   
}

static inline kgi_u8_t TVP3026_EDAC_IN8(tvp3026_ramdac_io_t *tvp3026_io,
	kgi_u8_t reg)
{
	register kgi_u8_t val;

	TVP3026_DAC_OUT8(tvp3026_io, reg, TVP3026_EDAC_ADDR);
        val = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);     
	KRN_DEBUG(3, "EDAC%.2X -> %.2x", reg, val);
	return val;
}

#define	TVP3026_MAGIC	0x0101
#define	TVP3026_SHIFT	8

struct tvp3026_ramdac_mode_record_s
{
	kgi_u8_t		TrueColorControl;

	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_ratio_t		best_lclk;
	kgi_u16_t		lclk_divs;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;
};

static const tvp3026_ramdac_mode_record_t tvp3026_ramdac_mode[] =
{
	{ 0x80, 1, 0, {1, 1}, 0x0001,	KGI_AM_TEXT,  KGI_AS_448  },

	{ 0x80, 1, 0, {1,16}, 0x8142,	KGI_AM_I,     KGI_AS_4    },
	{ 0x80, 1, 0, {1, 8}, 0x0143,	KGI_AM_I,     KGI_AS_8    },

	{ 0x43, 0, 1, {1, 4}, 0x0043,	KGI_AM_RGB,   KGI_AS_664  },
	{ 0x45, 0, 1, {1, 4}, 0x0043,	KGI_AM_RGB,   KGI_AS_565  },
	{ 0x56, 0, 1, {3, 8}, 0x0003,	KGI_AM_RGB,   KGI_AS_888  },

	{ 0x05, 0, 0, {1, 4}, 0x0043,	KGI_AM_RGB,   KGI_AS_565  },
	{ 0x03, 0, 0, {1, 4}, 0x0043,	KGI_AM_RGB,   KGI_AS_664  },
	{ 0x16, 0, 0, {3, 8}, 0x0140,	KGI_AM_RGB,   KGI_AS_888  },

	{ 0x01, 1, 0, {1, 4}, 0x0043,	KGI_AM_RGBA,  KGI_AS_4444 },
	{ 0x04, 1, 0, {1, 4}, 0x0043,	KGI_AM_RGBA,  KGI_AS_5551 },
	{ 0x06, 1, 0, {1, 2}, 0x0003,	KGI_AM_RGBA,  KGI_AS_8888 },

	{ 0x41, 0, 1, {1, 4}, 0x0043,	KGI_AM_RGBX,  KGI_AS_4444 },
	{ 0x44, 0, 1, {1, 4}, 0x0043,	KGI_AM_RGBX,  KGI_AS_5551 },
	{ 0x46, 0, 1, {1, 2}, 0x0003,	KGI_AM_RGBX,  KGI_AS_8888 }
};
#define	TVP3026_RAMDAC_NR_MODES (sizeof(tvp3026_ramdac_mode)/sizeof(tvp3026_ramdac_mode[0]))

static inline const tvp3026_ramdac_mode_record_t *tvp3026_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	for (i = 0; i < TVP3026_RAMDAC_NR_MODES; i++) {

		if (tvp3026_ramdac_mode[i].dam != fam) {

			continue;
		}
		if (kgim_strcmp(tvp3026_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}
		break;
	}
	return (i == TVP3026_RAMDAC_NR_MODES) 
		? NULL : tvp3026_ramdac_mode + i;
}




#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d)	((dpm->rclk.mul == m) && (dpm->rclk.div == d))

/* ----------------------------------------------------------------------------
**	image resources
** ----------------------------------------------------------------------------
*/

static void tvp3026_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	tvp3026_ramdac_mode_t *tvp3026_mode = clut->meta;
	tvp3026_ramdac_io_t   *tvp3026_io   = clut->meta_io;
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(table == 0);
	KRN_ASSERT(count > 0);

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {

		if (am & KGI_AM_COLOR1) {

			tvp3026_mode->clut[dst+0] = 
				data[src++] >> TVP3026_SHIFT;
		}
		if (am & KGI_AM_COLOR2) {

			tvp3026_mode->clut[dst+1] =
				data[src++] >> TVP3026_SHIFT;
		}
		if (am & KGI_AM_COLOR3) {

			tvp3026_mode->clut[dst+2] =
				data[src++] >> TVP3026_SHIFT;
		}
		dst += 3;
	}

	TVP3026_DAC_OUT8(tvp3026_io, index, TVP3026_DAC_PW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_P_DATA,
		tvp3026_mode->clut + 3*index, 3*count);
}

#define	tvp3026_ramdac_select_clut	NULL

#if 0
void tvp3026_text16_hp_set_shape(kgic_mode_text16context_t *ctx,
	kgi_pointer64x64 *ptr)
{
	tvp3026_ramdac_io_t   *tvp3026_io   = KGIM_TEXT16_IO(ctx, ramdac);
	tvp3026_ramdac_mode_t *tvp3026_mode = KGIM_TEXT16_MODE(ctx, ramdac);
	kgi_u_t i;

	tvp3026_mode->ptr.hot.x = ptr->hotx;
	tvp3026_mode->ptr.hot.y = ptr->hoty;

	/*	update private cursor mode state
	*/
	switch (ptr->mode) {

	case KGI_PM_WINDOWS:
		tvp3026_mode->CursorCtrl = TVP3026_DAC09_CursorXGA;
		break;

	case KGI_PM_X11:
		tvp3026_mode->CursorCtrl = TVP3026_DAC09_CursorX;
		break;

	case KGI_PM_THREE_COLOR:
		tvp3026_mode->CursorCtrl = TVP3026_DAC09_CursorThreeColor;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026_mode->CursorCtrl, 
		TVP3026_CURSOR_CTRL);

	/*	set cursor pattern
	*/
	KRN_ASSERT(sizeof(tvp3026_mode->cursor_data) == 1024);
	kgim_memcpy(tvp3026_mode->cursor_data, ptr->xor_mask, 512);
	kgim_memcpy(tvp3026_mode->cursor_data + 512, ptr->and_mask, 512);

	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026->CursorControl & 
		~(TVP3026_EDAC06_CursorAddr9 | TVP3026_EDAC06_CursorAddr9),
		0x06);
	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_CURSOR_RAM_DATA,
		tvp3026->cursor_data, 1024);

	/*	set colors
	*/
	for (i = 0; i < 3; i++) {

		register kgi_u8_t *clut = tvp3026_mode->cursor_clut + 3*i;

		clut[0] = ptr->col[i].r >> TVP3026_SHIFT;
		clut[1] = ptr->col[i].g >> TVP3026_SHIFT;
		clut[2] = ptr->col[i].b >> TVP3026_SHIFT;
	}
	TVP3026_DAC_OUT8(tvp3026_io, 0x01, TVP3026_DAC_CW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_CW_INDEX,
		tvp3026_mode->cursor_clut + 3, 9);
}
#endif

static void tvp3026_ramdac_cursor_show(kgi_marker_t *cursor, 
	kgi_u_t x, kgi_u_t y)
{
	tvp3026_ramdac_mode_t *tvp3026_mode = cursor->meta;
	tvp3026_ramdac_io_t   *tvp3026_io   = cursor->meta_io;

	x += tvp3026_mode->ptr_shift.x - tvp3026_mode->ptr_hot.x;
	y += tvp3026_mode->ptr_shift.y - tvp3026_mode->ptr_hot.y;

	TVP3026_DAC_OUT8(tvp3026_io, x & 0xFF, 
		TVP3026_DAC_CURSOR_XL);
	TVP3026_DAC_OUT8(tvp3026_io, (x >> 8) & 0xFF, 
		TVP3026_DAC_CURSOR_XH);
	TVP3026_DAC_OUT8(tvp3026_io, y & 0xFF,
		TVP3026_DAC_CURSOR_YL);
	TVP3026_DAC_OUT8(tvp3026_io, (y >> 8) & 0xFF,
		TVP3026_DAC_CURSOR_YH);
}

static void tvp3026_ramdac_cursor_hide(kgi_marker_t *cursor)
{
	tvp3026_ramdac_mode_t *tvp3026_mode = cursor->meta;
	tvp3026_ramdac_io_t   *tvp3026_io   = cursor->meta_io;

	TVP3026_DAC_OUT8(tvp3026_io, 0xFF, TVP3026_DAC_CURSOR_XL);
	TVP3026_DAC_OUT8(tvp3026_io, 0x0F, TVP3026_DAC_CURSOR_XH);
	TVP3026_DAC_OUT8(tvp3026_io, 0xFF, TVP3026_DAC_CURSOR_YL);
	TVP3026_DAC_OUT8(tvp3026_io, 0x0F, TVP3026_DAC_CURSOR_YH);
}

#define	tvp3026_ramdac_cursor_undo	NULL


kgi_error_t tvp3026_ramdac_init(tvp3026_ramdac_t *tvp3026,
	tvp3026_ramdac_io_t *tvp3026_io, const kgim_options_t *options)
{
	tvp3026->CursorCtrl = TVP3026_DAC_IN8(tvp3026_io, 
		TVP3026_DAC_CURSOR_CTRL);

#define	TVP3026_SAVE(reg)	\
	tvp3026->reg = TVP3026_EDAC_IN8(tvp3026_io, TVP3026_EDAC_##reg)

	TVP3026_SAVE(CursorControl);
	TVP3026_SAVE(LatchControl);
	TVP3026_SAVE(TrueColorControl);
	TVP3026_SAVE(MultiplexControl);
	TVP3026_SAVE(ClockSelect);
	TVP3026_SAVE(PalettePage);
	TVP3026_SAVE(GeneralControl);
	TVP3026_SAVE(MiscControl);
	TVP3026_SAVE(GIOControl);
	TVP3026_SAVE(GIOData);
	TVP3026_SAVE(ColKeyOverlayL);
	TVP3026_SAVE(ColKeyOverlayH);
	TVP3026_SAVE(ColKeyRedL);
	TVP3026_SAVE(ColKeyRedH);
	TVP3026_SAVE(ColKeyGreenL);
	TVP3026_SAVE(ColKeyGreenH);
	TVP3026_SAVE(ColKeyBlueL);
	TVP3026_SAVE(ColKeyBlueH);
	TVP3026_SAVE(ColKeyControl);

#undef	TVP3026_SAVE

#warning save Loop PLL registers

	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PR_INDEX);
	TVP3026_DAC_INS8(tvp3026_io, TVP3026_DAC_P_DATA,
		tvp3026->clut, sizeof(tvp3026->clut));

	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_CR_INDEX);
	TVP3026_DAC_INS8(tvp3026_io, TVP3026_DAC_C_DATA,
		tvp3026->cursor_clut, sizeof(tvp3026->cursor_clut));

	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026->CursorControl & 
		~(TVP3026_EDAC06_CursorAddr9 | TVP3026_EDAC06_CursorAddr9),
		TVP3026_EDAC_CursorControl);
	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PR_INDEX);
	TVP3026_DAC_INS8(tvp3026_io, TVP3026_DAC_CURSOR_RAM_DATA,
		tvp3026->cursor_data, sizeof(tvp3026->cursor_data));

	return KGI_EOK;
}

void tvp3026_ramdac_done(tvp3026_ramdac_t *tvp3026,
	tvp3026_ramdac_io_t *tvp3026_io, const kgim_options_t *options)
{
	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_P_DATA,
		tvp3026->clut, sizeof(tvp3026->clut));

	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_CW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_C_DATA,
		tvp3026->cursor_clut, sizeof(tvp3026->cursor_clut));

	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026->CursorControl & 
		~(TVP3026_EDAC06_CursorAddr9 | TVP3026_EDAC06_CursorAddr9),
		0x06);
	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_CURSOR_RAM_DATA,
		tvp3026->cursor_data, sizeof(tvp3026->cursor_data));

#warning restore loop PLL registers

#define	TVP3026_RESTORE(reg)	\
	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026->reg, TVP3026_EDAC_##reg)

	TVP3026_RESTORE(CursorControl);
	TVP3026_RESTORE(LatchControl);
	TVP3026_RESTORE(TrueColorControl);
	TVP3026_RESTORE(MultiplexControl);
	TVP3026_RESTORE(ClockSelect);
	TVP3026_RESTORE(PalettePage);
	TVP3026_RESTORE(GeneralControl);
	TVP3026_RESTORE(MiscControl);
	TVP3026_RESTORE(GIOControl);
	TVP3026_RESTORE(GIOData);
	TVP3026_RESTORE(ColKeyOverlayL);
	TVP3026_RESTORE(ColKeyOverlayH);
	TVP3026_RESTORE(ColKeyRedL);
	TVP3026_RESTORE(ColKeyRedH);
	TVP3026_RESTORE(ColKeyGreenL);
	TVP3026_RESTORE(ColKeyGreenH);
	TVP3026_RESTORE(ColKeyBlueL);
	TVP3026_RESTORE(ColKeyBlueH);
	TVP3026_RESTORE(ColKeyControl);

#undef	TVP3026_RESTORE

	TVP3026_DAC_OUT8(tvp3026_io, tvp3026->CursorCtrl,
		TVP3026_DAC_CURSOR_CTRL);
}

kgi_error_t tvp3026_ramdac_mode_check(tvp3026_ramdac_t *tvp3026,
	tvp3026_ramdac_io_t *tvp3026_io,
	tvp3026_ramdac_mode_t *tvp3026_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const tvp3026_ramdac_mode_record_t *rec;
	kgi_u_t	lm, ln, lp, lq, z;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = tvp3026_ramdac_mode_record(img[0].fam, img[0].bpfa);
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

			img[0].frames = 1;
		}

		KRN_DEBUG(2, " lclk %i:%i, dclk %i:%i", dpm->lclk.mul,
			dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk < tvp3026->ramdac.lclk.min) ||
			(dpm->dclk < tvp3026->ramdac.dclk.min)) {

			KRN_DEBUG(1, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk > tvp3026->ramdac.lclk.max) || 
			(dpm->dclk > tvp3026->ramdac.dclk.max)) {

			register kgi_u_t dclk = tvp3026->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < tvp3026->ramdac.dclk.max)
				? dclk : tvp3026->ramdac.dclk.max;
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > tvp3026->ramdac.lclk.max) || 
			(dpm->dclk > tvp3026->ramdac.dclk.max)) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if ((lclk < tvp3026->ramdac.lclk.min) ||
			(dpm->dclk < tvp3026->ramdac.dclk.min)) {

			register kgi_u_t dclk = tvp3026->ramdac.dclk.min *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > tvp3026->ramdac.dclk.min)
				? dclk : tvp3026->ramdac.dclk.min;
		}
		return KGI_EOK;

     case KGI_TC_CHECK:

		if (((dpm->lclk.mul != 1) && (dpm->lclk.mul != 3)) ||
			(((1 << (dpm->lclk.mul - 1)) & rec->lclk_divs) == 0)) {

			KRN_DEBUG(1, "invalid dpm->lclk ration (%i:%i)",
				dpm->lclk.mul, dpm->lclk.div);
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		if ((dpm->dclk < tvp3026->ramdac.dclk.min) ||
			(tvp3026->ramdac.dclk.max < dpm->dclk) ||
			(lclk < tvp3026->ramdac.lclk.min) ||
			(tvp3026->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(1, "%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
                if (tvp3026_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
			KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		if (dpm->flags & KGI_DPF_TP_2XCLOCK) {

			KRN_DEBUG(1, "can't handle internal clock doubling.");
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}
		if (((dpm->flags & KGI_DPF_CH_ALUT) && !rec->alut) ||
			((dpm->flags & KGI_DPF_CH_ILUT) && !rec->ilut)) {

			KRN_DEBUG(1, "LUT configuration not supported");
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	/*	Now everything except the LCLK ration is checked and should 
	**	be sane. Thus we proceed to setup of the device dependent mode.
	*/
	tvp3026_mode->rec = rec;
	
	/*	palette setup
	*/
	tvp3026_mode->PixelMask = 0xFF;
	tvp3026_mode->PalettePage = 0;
		
	tvp3026_mode->ptr_shift.x	 = 64 +
		tvp3026_mode->kgim.crt->x.total -
		tvp3026_mode->kgim.crt->x.blankend;
	tvp3026_mode->ptr_shift.y	 = 64 +
		tvp3026_mode->kgim.crt->y.total -
		tvp3026_mode->kgim.crt->y.blankend;


	tvp3026_mode->clut_ctrl.meta	= tvp3026_mode;
	tvp3026_mode->clut_ctrl.meta_io	= tvp3026_io;
	tvp3026_mode->clut_ctrl.prot	= KGI_PF_DRV_RWS;
	if (dpm->flags & KGI_DPF_CH_ALUT) {

		tvp3026_mode->clut_ctrl.type = KGI_RT_ALUT_CONTROL;
		tvp3026_mode->clut_ctrl.name = "alut control";
	}
	if (dpm->flags & KGI_DPF_CH_ILUT) {

		tvp3026_mode->clut_ctrl.type = KGI_RT_ILUT_CONTROL;
		tvp3026_mode->clut_ctrl.name = "ilut control";
	}
	tvp3026_mode->clut_ctrl.Set	= tvp3026_ramdac_set_clut;
	tvp3026_mode->clut_ctrl.Select	= tvp3026_ramdac_select_clut;

	/*	cursor setup
	*/
	tvp3026_mode->CursorCtrl = TVP3026_DAC09_CursorOff;
	tvp3026_mode->CursorControl = 
		((dpm->flags & KGI_DPF_MASK) == KGI_DPF_TP_LRTB_I1)
			? TVP3026_EDAC06_InterlacedCursor : 0;

	tvp3026_mode->ptr_ctrl.meta	= tvp3026_mode;
	tvp3026_mode->ptr_ctrl.meta_io= tvp3026_io;
	tvp3026_mode->ptr_ctrl.type	= KGI_RT_CURSOR_CONTROL;
	tvp3026_mode->ptr_ctrl.prot	= KGI_PF_DRV_RWS;
	tvp3026_mode->ptr_ctrl.name	= "pointer";
	tvp3026_mode->ptr_ctrl.Show	= tvp3026_ramdac_cursor_show;
	tvp3026_mode->ptr_ctrl.Hide	= tvp3026_ramdac_cursor_hide;
	tvp3026_mode->ptr_ctrl.Undo	= tvp3026_ramdac_cursor_undo;

#warning init tvp3026_mode->cursor_clut
#warning init tvp3026_mode->cursor_data

	/*	mode dependent registers
	*/
	tvp3026_mode->TrueColorControl	= rec->TrueColorControl;
	tvp3026_mode->MiscControl = TVP3026_EDAC1E_6n8Disable |
		TVP3026_EDAC1E_8BitEnable;
	tvp3026_mode->MultiplexControl	= 0;
	tvp3026_mode->ClockSelect	= 0;
	tvp3026_mode->GIOData		= 0;

	switch (tvp3026_mode->TrueColorControl) {

	case 0x80:

		if (dpm->dam == KGI_AM_TEXT) {

			tvp3026_mode->ClockSelect = 0x07;
			tvp3026_mode->GIOData = 0x14; /* ??? really needed ? */
			tvp3026_mode->MultiplexControl = 0x98;	
			break;
		}

		KRN_ASSERT(dpm->dam == KGI_AM_I);

		if (dpm->bpda[0] == 4) {

 			tvp3026_mode->ClockSelect = 0x00;
			tvp3026_mode->GIOData = 0x14; /* ??? really needed ? */

			switch (dpm->lclk.div) {

			case  2: tvp3026_mode->MultiplexControl = 0x41; break;
			case  4: tvp3026_mode->MultiplexControl = 0x42; break;
			case  8: tvp3026_mode->MultiplexControl = 0x43; break;
			case 16: tvp3026_mode->MultiplexControl = 0x44; break;

			}
			break;
		}

		if (dpm->bpda[0] == 8) {

			tvp3026_mode->ClockSelect = 0x25;
			tvp3026_mode->GIOData = 0x10;

			switch (dpm->lclk.div) {

			case 1:	tvp3026_mode->MultiplexControl = 0x49;	break; 
			case 2:	tvp3026_mode->MultiplexControl = 0x4A;	break; 
			case 4:	tvp3026_mode->MultiplexControl = 0x4B;	break; 
			case 8:	tvp3026_mode->MultiplexControl = 0x4C;	break;

			}
			break;
		}

		KRN_INTERNAL_ERROR;
		break;

	case 0x01:
	case 0x04:
	case 0x41:
	case 0x43:
	case 0x44:
	case 0x45:
		tvp3026_mode->ClockSelect = (1 << TVP3026_EDAC1A_VCLKShift) |
			TVP3026_EDAC1A_ClkPClkPLL;
		tvp3026_mode->MiscControl |= TVP3026_EDAC1E_PortSwitchInvert;
		tvp3026_mode->GIOData = 0x1E; /* ??? really needed? */

		switch (dpm->lclk.div) {

		case  1: tvp3026_mode->MultiplexControl = 0x52; break; 
		case  2: tvp3026_mode->MultiplexControl = 0x53; break; 
		case  4: tvp3026_mode->MultiplexControl = 0x54; break;

		}
		break;

	case 0x56:
                tvp3026_mode->ClockSelect = (2 << TVP3026_EDAC1A_VCLKShift) |
			TVP3026_EDAC1A_ClkPClkPLL;
                tvp3026_mode->MiscControl |= TVP3026_EDAC1E_PortSwitchInvert;
                tvp3026_mode->GIOData = 0x1E; /* ??? really needed ? */

		switch (dpm->lclk.div) {

		case 4: tvp3026_mode->MultiplexControl = 0x5B; break;
		case 8: tvp3026_mode->MultiplexControl = 0x5C; break;

		}
		break;

	case 0x06:
	case 0x46:
                tvp3026_mode->ClockSelect = (0 << TVP3026_EDAC1A_VCLKShift) |
			TVP3026_EDAC1A_ClkPClkPLL;
                tvp3026_mode->MiscControl |= TVP3026_EDAC1E_PortSwitchInvert;
                tvp3026_mode->GIOData = 0x1E; /* ??? really needed ? */

		switch (dpm->lclk.div) {

		case 1: tvp3026_mode->MultiplexControl = 0x5B; break;
		case 2: tvp3026_mode->MultiplexControl = 0x5C; break;

		}
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, NOSUP);
	}	

	/*	Loop PLL & LatchControl setup
	*/
	switch (dpm->lclk.mul) {

	case 1:	tvp3026_mode->LatchControl =
			(dpm->lclk.div == 2) ? 0x07 : 0x06;

		lm = 65 - 4 * dpm->lclk.mul;
		ln = 65 - 4 * dpm->lclk.div;

		/*	now calculate z. This is really z * 100
		*/
		z = ((dpm->rclk.mul == 1) && (dpm->rclk.div == 1))
			? (11000000 / (dpm->dclk / 1000))
			: (11000000 / (dpm->dclk / dpm->lclk.div / 1000)); 
		break;

	case 3:	tvp3026_mode->LatchControl = 
			((tvp3026->rev > 0x20) && (dpm->lclk.div == 4))
				? 0x08 : 0x07;

		lm = 65 - dpm->lclk.mul;
                ln = 65 - dpm->lclk.div;

                /*	now calculate z. This is really z * 100
		*/
                z = (11000000 * (65 - ln)) / ((dpm->dclk / 1000) * (65 - lm));
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, NOSUP);
	}

	if (z > 1600) {

		/*	smallest q greater (z - 16) / 16
		*/
		lp = 3;
		lq = ((z - 1600) / 1600) + 1;

	} else {

		/*	largest p less then log2(z)
		*/
		for (lp = 0; z > (200U << lp); lp++) {
		}
		lq = 0;
        }
	KRN_DEBUG(1, "Loop PLL { m, n, q } = { %i, %i, %i }", lm, ln, lq);

	switch (dpm->lclk.mul) {

	case 1:	tvp3026_mode->ln = (ln & 0x3f) | TVP3026_LClkN_Bias;
		tvp3026_mode->lm = (lm & 0x3f) | TVP3026_LClkM_Bias;
		tvp3026_mode->lp = (lp & 0x03) | 
			(TVP3026_LClkP_Bias | TVP3026_LClkP_PLLEnable);
		break;

	case 3:	tvp3026_mode->ln = (ln & 0x3f) | TVP3026_LClkN_Bias;
		tvp3026_mode->lm = (lm & 0x3f) | ((tvp3026->rev > 0x20)
			? ((tvp3026_mode->TrueColorControl & 0x20) ? 0x00:0x80)
			: (((dpm->lclk.div == 8) ? 0x80 : 0x00) ^
				((tvp3026_mode->TrueColorControl & 0x20)
					? 0x80 : 0x00)));
		tvp3026_mode->lp = (lp & 0x03) | 
			(TVP3026_LClkP_Bias | TVP3026_LClkP_PLLEnable |
			TVP3026_LClkP_LForce);
		break;
	}
	KRN_DEBUG(2, "LCLK registers { m, n, p } = { %.2x, %.2x, %.2x }",
		tvp3026_mode->lm, tvp3026_mode->ln, tvp3026_mode->lp);

	tvp3026_mode->MClkLoopControl = 
		TVP3026_EDAC39_MClkOutStrobe | TVP3026_EDAC39_MClkOutMClk |
                ((dpm->dam == KGI_AM_TEXT)
			? TVP3026_EDAC39_PClkOutPClk 
			: (lq | TVP3026_EDAC39_PClkOutLClk));

	/*	general control setup
	*/
	tvp3026_mode->GeneralControl = 0;
	if (tvp3026_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN) {

		tvp3026_mode->GeneralControl |= TVP3026_EDAC1D_SyncEnable;
	}
	if (tvp3026_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

		tvp3026_mode->GeneralControl |= TVP3026_EDAC1D_BlankPedestral;
	}

	/*	overlay setup
	*/
	tvp3026_mode->ColKeyOverlayL	= 0x00;
	tvp3026_mode->ColKeyOverlayH	= 0xFF;
	tvp3026_mode->ColKeyRedL	= 0x00;
	tvp3026_mode->ColKeyRedH	= 0xFF;
	tvp3026_mode->ColKeyGreenL	= 0x00;
	tvp3026_mode->ColKeyGreenH	= 0xFF;
	tvp3026_mode->ColKeyBlueL	= 0x00;
	tvp3026_mode->ColKeyBlueH	= 0xFF;
	tvp3026_mode->ColKeyControl	= 0x00;

#warning handle zooming properly!

	return KGI_EOK;
}

kgi_resource_t *tvp3026_ramdac_image_resource(tvp3026_ramdac_t *tvp3026,
	tvp3026_ramdac_mode_t *tvp3026_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));

	switch (index) {

	case 0:	return (kgi_resource_t *) &(tvp3026_mode->ptr_ctrl);
	case 1:	return (kgi_resource_t *) &(tvp3026_mode->clut_ctrl);

	}
	return NULL;
}

void tvp3026_ramdac_mode_enter(tvp3026_ramdac_t *tvp3026,
	tvp3026_ramdac_io_t *tvp3026_io,
	tvp3026_ramdac_mode_t *tvp3026_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026_mode->PixelMask,
		TVP3026_DAC_PIXEL_MASK);
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026_mode->CursorCtrl,
		TVP3026_DAC_CURSOR_CTRL);

	TVP3026_EDAC_OUT8(tvp3026_io, 0x00, TVP3026_EDAC_PLLAddress);
	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026_mode->ln, TVP3026_EDAC_LClkData);
	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026_mode->lm, TVP3026_EDAC_LClkData);
	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026_mode->lp, TVP3026_EDAC_LClkData);

#define	TVP3026_EDAC_SET(reg)	\
	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026_mode->reg, TVP3026_EDAC_##reg)

	TVP3026_EDAC_SET(MClkLoopControl);

	TVP3026_EDAC_SET(CursorControl);
	TVP3026_EDAC_SET(LatchControl);
	TVP3026_EDAC_SET(TrueColorControl);
	TVP3026_EDAC_SET(MultiplexControl);
	TVP3026_EDAC_SET(PalettePage);
	TVP3026_EDAC_SET(GeneralControl);
        TVP3026_EDAC_SET(MiscControl);
        TVP3026_EDAC_SET(ClockSelect);
        TVP3026_EDAC_SET(GIOData);

	TVP3026_EDAC_SET(ColKeyOverlayL);
	TVP3026_EDAC_SET(ColKeyOverlayH);
	TVP3026_EDAC_SET(ColKeyRedL);
	TVP3026_EDAC_SET(ColKeyRedH);
	TVP3026_EDAC_SET(ColKeyGreenL);
	TVP3026_EDAC_SET(ColKeyGreenH);
	TVP3026_EDAC_SET(ColKeyBlueL);
	TVP3026_EDAC_SET(ColKeyBlueH);
	TVP3026_EDAC_SET(ColKeyControl);

#undef	TVP3026_EDAC_SET

	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_P_DATA,
		tvp3026_mode->clut, sizeof(tvp3026_mode->clut));

	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_CW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_C_DATA,
		tvp3026_mode->cursor_clut, sizeof(tvp3026_mode->cursor_clut));

	TVP3026_EDAC_OUT8(tvp3026_io, tvp3026_mode->CursorControl & 
		~(TVP3026_EDAC06_CursorAddr9 | TVP3026_EDAC06_CursorAddr9),
		0x06);
	TVP3026_DAC_OUT8(tvp3026_io, 0x00, TVP3026_DAC_PW_INDEX);
	TVP3026_DAC_OUTS8(tvp3026_io, TVP3026_DAC_CURSOR_RAM_DATA,
		tvp3026_mode->cursor_data, sizeof(tvp3026_mode->cursor_data));
}
