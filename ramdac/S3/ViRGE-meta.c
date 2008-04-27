/* ----------------------------------------------------------------------------
**	S3 ViRGE ramdac meta implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**			2000		Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ViRGE-meta.c,v $
**	Revision 1.2  2003/01/30 13:51:05  foske
**	Update, cleanup, it compiles again.
**	
**	Revision 1.1.1.1  2000/09/21 12:32:27  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2000/09/21 12:32:27  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.2  2000/08/04 11:57:50  seeger_s
**	- merged code posted by Jos Hulzink to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- driver status 30% is reported by Jos Hulzink
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#define	DEBUG_LEVEL	255

#include <kgi/module.h>
#include "ramdac/S3/ViRGE.h"
#include "ramdac/S3/ViRGE-meta.h"

#if 0
/* I/O helper functions */
static inline void VIRGE_EDAC_OUT8(virge_ramdac_io_t *virge_io, kgi_u8_t val, kgi_u8_t reg)
{
	VIRGE_DAC_OUT8(virge_io, reg, VIRGE_DAC_EXT_ADDR);
	VIRGE_DAC_OUT8(virge_io, val, VIRGE_DAC_EXT_DATA);
}

static inline kgi_u8_t VIRGE_EDAC_IN8(virge_ramdac_io_t *virge_io, kgi_u8_t reg)
{
	VIRGE_DAC_OUT8(virge_io, reg, VIRGE_DAC_EXT_ADDR);
	return VIRGE_DAC_IN8(virge_io, VIRGE_DAC_EXT_DATA);
}
#endif

#define	VIRGE_MAGIC	0x0101
#define	VIRGE_SHIFT	8

struct virge_ramdac_mode_record_s
{
	kgi_u8_t		crt67;
	kgi_u8_t		reserved;

	kgi_u8_t		reserved2;
	kgi_u8_t		streams;

	kgi_ratio_t		best_lclk;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

static const virge_ramdac_mode_record_t virge_ramdac_mode[] =
{
	{ 0x00, 0x00, 1, 0, {1,1},	KGI_AM_TEXT, KGI_AS_448,	1 },

	/* 8 Bit CLUT mode. ViRGE doesn't support 8 bit RGB */
	{ 0x00, 0x10, 0, 1, {1,4},	KGI_AM_I,    KGI_AS_8,		1 },
	
	/* 15 Bit RGB modes */
	{ 0x10, 0xB4, 0, 1, {1,4},	KGI_AM_RGBA, KGI_AS_5551,	1 },
	{ 0x10, 0xB4, 0, 1, {1,4},	KGI_AM_RGBX, KGI_AS_5551,	1 },

	/* 16 Bit RGB modes. Attribute bit is not possible here */
	{ 0x10, 0xB6, 0, 1, {1,4},	KGI_AM_RGB,  KGI_AS_565,	1 },
	
	/* 24 Bit RGB mode, no attribute bits */
	{ 0x10, 0xB9, 0, 1, {3,8},	KGI_AM_RGB,  KGI_AS_888,	1 },
	
	/* 32 Bit RGB, plus 8 bit attribute */
	{ 0x10, 0xB8, 0, 1, {1,2},	KGI_AM_RGBA, KGI_AS_8888,	1 },
	{ 0x10, 0xB8, 0, 1, {1,2},	KGI_AM_RGBX, KGI_AS_8888,	1 }
};

#define	VIRGE_RAMDAC_NR_MODES (sizeof(virge_ramdac_mode) / sizeof(virge_ramdac_mode[0]))

static inline const virge_ramdac_mode_record_t *virge_ramdac_mode_record(kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	KRN_DEBUG(2, "virge_ramdac_mode_record()");
	
	/* !!!	handle BGR modes */
	for (i = 0; i < VIRGE_RAMDAC_NR_MODES; i++) 
	{
		if (virge_ramdac_mode[i].dam != fam) 
		{
			continue;
		}
		
		if (kgim_strcmp(virge_ramdac_mode[i].bpda, bpfa) != 0) 
		{
			continue;
		}
		
		break;
	}
	
	if (i == VIRGE_RAMDAC_NR_MODES)
	{
		KRN_DEBUG(1, "Couldn't find a ramdac mode");
	}
	
	return (i == VIRGE_RAMDAC_NR_MODES) ? NULL : virge_ramdac_mode + i;
}

#define	LCLK(m,d)	((dpm->lclk.mul == m) && (dpm->lclk.div == d))

/* ----	text16 context functions --------------------------------------------*/
#define	VIRGE_RAMDAC_IO(ctx)	\
	KGIM_SUBSYSTEM_IO((kgim_display_t *) ctx->meta_object, ramdac)

#define	VIRGE_RAMDAC_MODE(ctx)	\
	KGIM_SUBSYSTEM_MODE((kgim_display_mode_t *) ctx->meta_mode, ramdac)

#if 0
static void virge_text16_set_ilut(kgic_mode_text16context_t *ctx, kgic_ilut_entries_t *ilut)
{
	virge_ramdac_io_t   *virge_io   = virge_RAMDAC_IO(ctx);
	virge_ramdac_mode_t *virge_mode = VIRGE_RAMDAC_MODE(ctx);
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(ilut->img == 0);
	KRN_ASSERT(ilut->lut == 0);
	KRN_ASSERT(ilut->cnt > 0);

	cnt = ilut->cnt;
	dst = ilut->cnt * 3;
	src = 0;

	while (cnt--) 
	{
		if (ilut->am & KGI_AM_COLOR1) 
		{
			virge_mode->clut[dst+0] = ilut->data[src++] >> VIRGE_SHIFT;
		}
		
		if (ilut->am & KGI_AM_COLOR2) 
		{
			virge_mode->clut[dst+1] = ilut->data[src++] >> VIRGE_SHIFT;
		}
		
		if (ilut->am & KGI_AM_COLOR3) 
		{
			virge_mode->clut[dst+2] = ilut->data[src++] >> VIRGE_SHIFT;
		}
		
		dst += 3;
	}

	src = 3 * ilut->idx;
	cnt = 3 * ilut->cnt;
	VIRGE_DAC_OUT8(virge_io, ilut->idx, VIRGE_DAC_PW_INDEX);
	VIRGE_DAC_OUTS8(virge_io, VIRGE_DAC_P_DATA, virge_mode->clut + src, cnt);
}


void virge_text16_hp_set_shape(kgic_mode_text16context_t *ctx, kgi_pointer64x64 *ptr)
{
	virge_ramdac_io_t   *virge_io   = VIRGE_RAMDAC_IO(ctx);
	virge_ramdac_mode_t *VIRGE_mode = VIRGE_RAMDAC_MODE(ctx);
	kgi_u_t i;

	virge_mode->ptr.hot.x = ptr->hotx;
	virge_mode->ptr.hot.y = ptr->hoty;

	/* Update private state */
	virge_mode->ext06 = VIRGE_EDAC06_Cursor64x64;
	
	switch (ptr->mode) 
	{
	case KGI_PM_WINDOWS:
		virge_mode->ext06 |= VIRGE_EDAC06_CursorXGA;
		break;

	case KGI_PM_X11:
		virge_mode->ext06 |= VIRGE_EDAC06_CursorX;
		break;

	case KGI_PM_THREE_COLOR:
		virge_mode->ext06 |= VIRGE_EDAC06_CursorThreeColor;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	
	VIRGE_EDAC_OUT8(virge_io, virge_mode->ext06, 0x06);
        VIRGE_DAC_OUT8(virge_io, 0x00, 0x00); /* Reset A0 - A7 */

	/* Set pattern */
        for (i = 0; i < 512; i++) 
	{
        	VIRGE_DAC_OUT8(virge_io, ptr->xor_mask[i], VIRGE_DAC_CURSOR_RAM_DATA);
        }
	
        for (i = 0; i < 512; i++) 
	{
        	VIRGE_DAC_OUT8(virge_io, ptr->and_mask[i], VIRGE_DAC_CURSOR_RAM_DATA);
        }

	/* Set colors */
	VIRGE_DAC_OUT8(virge_io, 1, VIRGE_DAC_CW_INDEX);
	
	for (i = 0; i < 3; i++) 
	{
		VIRGE_DAC_OUT8(virge_io, ptr->col[i].r >> SHIFT, VIRGE_DAC_C_DATA);
		VIRGE_DAC_OUT8(virge_io, ptr->col[i].g >> SHIFT, VIRGE_DAC_C_DATA);
		VIRGE_DAC_OUT8(virge_io, ptr->col[i].b >> SHIFT, VIRGE_DAC_C_DATA);
	}
}


static void virge_text16_hp_show(kgic_mode_text16context_t *ctx, kgi_u_t x, kgi_u_t y)
{
	virge_ramdac_io_t   *virge_io   = VIRGE_RAMDAC_IO(ctx);
	virge_ramdac_mode_t *virge_mode = VIRGE_RAMDAC_MODE(ctx);

	KRN_DEBUG(1, "virge_text16_hp_show()");
	
	if ((virge_mode->ptr.pos.x != x) || (virge_mode->ptr.pos.y != y)) 
	{
		virge_mode->ptr.pos.x = x;
		virge_mode->ptr.pos.y = y;

		x += virge_mode->ptr.shift.x - virge_mode->ptr.hot.x;
		y += virge_mode->ptr.shift.y - virge_mode->ptr.hot.y;

//		VIRGE_DAC_OUT8(virge_io, x,      VIRGE_DAC_CURSOR_XL);
//		VIRGE_DAC_OUT8(virge_io, x >> 8, VIRGE_DAC_CURSOR_XH);
//		VIRGE_DAC_OUT8(virge_io, y,      VIRGE_DAC_CURSOR_YL);
//		VIRGE_DAC_OUT8(virge_io, y >> 8, VIRGE_DAC_CURSOR_YH);
	}
}

static void virge_text16_hp_hide(kgic_mode_text16context_t *ctx)
{
	virge_ramdac_io_t   *virge_io   = VIRGE_RAMDAC_IO(ctx);
	virge_ramdac_mode_t *virge_mode = VIRGE_RAMDAC_MODE(ctx);

	KRN_DEBUG(1, "virge_text16_hp_hide()");
	
	virge_mode->ptr.pos.x = virge_mode->ptr.pos.y = 0x7FF;

//	VIRGE_DAC_OUT8(virge_io, 0xFF, VIRGE_DAC_CURSOR_XL);
//	VIRGE_DAC_OUT8(virge_io, 0x07, VIRGE_DAC_CURSOR_XH);
//	VIRGE_DAC_OUT8(virge_io, 0xFF, VIRGE_DAC_CURSOR_YL);
//	VIRGE_DAC_OUT8(virge_io, 0x07, VIRGE_DAC_CURSOR_YH);
}
#endif

#undef	VIRGE_RAMDAC_MODE
#undef	VIRGE_RAMDAC_IO
/*---- end of text16 context functions -----------------------------------*/

kgi_error_t virge_ramdac_init(virge_ramdac_t *virge, virge_ramdac_io_t *virge_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_ramdac_init()");

#if 0
	virge->EXT.CursorControl = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_CursorControl);
	virge->EXT.ColorMode     = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_ColorMode);
	virge->EXT.ModeControl   = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_ModeControl);
	virge->EXT.PalettePage   = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_PalettePage);
	virge->EXT.MiscControl   = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_MiscControl);
	virge->EXT.ColorKeyControl=VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_ColorKeyControl);
	virge->EXT.OverlayKey    = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_OverlayKey);
	virge->EXT.RedKey        = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_RedKey);
	virge->EXT.GreenKey      = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_GreenKey);
	virge->EXT.BlueKey       = VIRGE_EDAC_IN8(virge_io, VIRGE_EDAC_BlueKey);

	virge_DAC_OUT8(virge_io, 0, VIRGE_DAC_PR_INDEX);
	VIRGE_DAC_INS8(virge_io, VIRGE_DAC_P_DATA, virge->clut, sizeof(virge->clut));

/*	kgi_u8_t cursor_palette[3*4];
**	kgi_u8_t cursor_data[];
*/
	virge->CursorXLow	 = VIRGE_DAC_IN8(virge_io, VIRGE_DAC_CURSOR_XL);
	virge->CursorXHigh = VIRGE_DAC_IN8(virge_io, VIRGE_DAC_CURSOR_XH);
	virge->CursorYLow	 = VIRGE_DAC_IN8(virge_io, VIRGE_DAC_CURSOR_YL);
	virge->CursorYHigh = VIRGE_DAC_IN8(virge_io, VIRGE_DAC_CURSOR_YH);
#endif

	return KGI_EOK;
}

void virge_ramdac_done(virge_ramdac_t *virge, virge_ramdac_io_t *virge_io, const kgim_options_t *options)
{
	KRN_DEBUG(1, "virge_ramdac_done()");
	
}

kgi_error_t virge_ramdac_mode_check(virge_ramdac_t *virge, virge_ramdac_io_t *virge_io, virge_ramdac_mode_t *virge_mode, kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const virge_ramdac_mode_record_t *rec;
	const kgim_monitor_mode_t *crt_mode = virge_mode->kgim.crt;

	KRN_DEBUG(1, "virge_ramdac_mode_check()");
#define KRN_DEBUG_TIMING(t, name) \
	KRN_DEBUG(4, "%s.width = %d", name, t.width); \
	KRN_DEBUG(4, "%s.blankstart = %d", name, t.blankstart); \
	KRN_DEBUG(4, "%s.syncstart = %d", name, t.syncstart); \
	KRN_DEBUG(4, "%s.syncend = %d", name, t.syncend); \
	KRN_DEBUG(4, "%s.blankend = %d", name, t.blankend); \
	KRN_DEBUG(4, "%s.total = %d", name, t.total); \
	KRN_DEBUG(4, "%s.polarity = %d", name, t.polarity);

	KRN_DEBUG_TIMING(crt_mode->x, "adjust x");
	KRN_DEBUG_TIMING(crt_mode->y, "adjust y");
	
	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = virge_ramdac_mode_record(img[0].fam, img[0].bpfa);
	if (rec == NULL) 
	{
		KRN_DEBUG(2, "could not handle dot attributes (dam %.8x)", dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	switch (cmd) 
	{
	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE:");

		dpm->lclk = rec->best_lclk;
		dpm->rclk.mul = dpm->rclk.div = 1;
		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames) 
		{
			img[0].frames = rec->frames;
		}

		KRN_DEBUG(3, "lclk %i:%i, dclk %i:%i", dpm->lclk.mul, dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);

 		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(3, "KGI_TC_LOWER:");

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk < virge->ramdac.lclk.min) || (dpm->dclk < virge->ramdac.dclk.min)) 
		{
			KRN_DEBUG(2, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		
		if ((lclk > virge->ramdac.lclk.max) || (dpm->dclk > virge->ramdac.dclk.max)) 
		{
			register kgi_u_t dclk = virge->ramdac.lclk.max * dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < virge->ramdac.dclk.max) ? dclk : virge->ramdac.dclk.max;
		}
		
		return KGI_EOK;

	case KGI_TC_RAISE:
		KRN_DEBUG(3, "KGI_TC_RAISE:");

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((lclk > virge->ramdac.lclk.max) || (dpm->dclk > virge->ramdac.dclk.max)) 
		{
			KRN_DEBUG(2, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		
		if ((lclk < virge->ramdac.lclk.min) || (dpm->dclk < virge->ramdac.dclk.min)) 
		{
			register kgi_u_t dclk = virge->ramdac.dclk.min * dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > virge->ramdac.dclk.min) ? dclk : virge->ramdac.dclk.min;
		}
		
		return KGI_EOK;

     case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK:");

		if (! ((dpm->lclk.mul == rec->best_lclk.mul) && (dpm->lclk.div == rec->best_lclk.div))) 
		{
			KRN_DEBUG(2, "Unsupported lclk mul or div");
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		
		if ((dpm->dclk < virge->ramdac.dclk.min) || (virge->ramdac.dclk.max < dpm->dclk) || (lclk < virge->ramdac.lclk.min) || (virge->ramdac.lclk.max < lclk)) {

			KRN_DEBUG(2, "%i Hz DCLK (%i Hz LCLK) is out of bounds", dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (virge_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED | KGIM_ST_SYNC_ON_BLUE)) 
		{
			KRN_DEBUG(2, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		virge_mode->rec = rec;

//		virge_mode->ext1E = VIRGE_EDAC1E_8BitEnable;
		
		if (virge_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN) 
		{
//			virge_mode->ext1E |= VIRGE_EDAC1E_SyncOnGreen;
		}
		
		if (virge_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) 
		{
//			virge_mode->ext1E |= VIRGE_EDAC1E_BlankPedestral;
		}

//		virge_mode->ext06 = VIRGE_EDAC06_Cursor64x64 | VIRGE_EDAC06_CursorOff;
		virge_mode->ptr.pos.x = virge_mode->ptr.pos.y = 0x7FF;
		virge_mode->ptr.hot.x = virge_mode->ptr.hot.y = 0;
		virge_mode->ptr.shift.x = 64 + virge_mode->kgim.crt->x.total - virge_mode->kgim.crt->x.blankend;
		virge_mode->ptr.shift.y = 64 + virge_mode->kgim.crt->y.total - virge_mode->kgim.crt->y.blankend;
	
	KRN_DEBUG_TIMING(crt_mode->x, "adjust x");
	KRN_DEBUG_TIMING(crt_mode->y, "adjust y");
	
		return KGI_EOK;
	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}
}

void virge_ramdac_mode_enter(virge_ramdac_t *virge, virge_ramdac_io_t *virge_io, virge_ramdac_mode_t *virge_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t x, y;

	KRN_DEBUG(1, "virge_ramdac_mode_enter()");
	
	x = virge_mode->ptr.pos.x + virge_mode->ptr.shift.x - virge_mode->ptr.hot.x;
	y = virge_mode->ptr.pos.y + virge_mode->ptr.shift.y - virge_mode->ptr.hot.y;

}
