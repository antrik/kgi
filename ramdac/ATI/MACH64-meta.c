/* -----------------------------------------------------------------------------
**	ATI MACH64 ramdac implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: MACH64-meta.c,v $
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
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Filip_Spacek
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/MACH64.h"
#include "ramdac/ATI/MACH64-meta.h"

/***	MACH64_MAGIC * device_value == 16bit_palette_value
****	16bit_value >> MACH64_SHIFT == device_value
****
****			MAGIC		SHIFT
****	8bit device:	0x0101		8
****	6bit device:	0x0410		10
***/
#define	MACH64_MAGIC	0x0101
#define	MACH64_SHIFT	8

struct mach64_ramdac_mode_record_s
{
	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

/*
 * Modes directly available by the chipset. Integrated RAMDAC does not
 * add any functionality to the one present in the chipset
 */
static const mach64_ramdac_mode_record_t mach64_ramdac_mode[] =
{
	{ 1, 0, KGI_AM_TEXT, KGI_AS_448,	1 },

	{ 1, 0, KGI_AM_I,    KGI_AS_8,		1 },
	{ 0, 1, KGI_AM_RGB,  KGI_AS_332,	1 },
	{ 0, 1, KGI_AM_RGB,  KGI_AS_565,	1 },
	{ 0, 1, KGI_AM_RGBX, KGI_AS_5551,	1 },
	{ 0, 1, KGI_AM_RGB,  KGI_AS_888,	1 },
	{ 0, 1, KGI_AM_RGBX, KGI_AS_8888,	1 }

};
#define	MACH64_RAMDAC_NR_MODES \
	(sizeof(mach64_ramdac_mode)/sizeof(mach64_ramdac_mode[0]))

static inline const mach64_ramdac_mode_record_t *mach64_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	for (i = 0; i < MACH64_RAMDAC_NR_MODES; i++) {
	  
		if (mach64_ramdac_mode[i].dam != fam)
			continue;
		if (kgim_strcmp(mach64_ramdac_mode[i].bpda, bpfa) != 0)
			continue;

		break;
	}
	return (i == MACH64_RAMDAC_NR_MODES) ? NULL : mach64_ramdac_mode + i;
}


#define	LCLK(m,d) ((dpm->lclk.mul == m) && (dpm->lclk.div == d))
#define	RCLK(m,d) ((dpm->rclk.mul == m) && (dpm->rclk.div == d))

static void mach64_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	mach64_ramdac_mode_t *mach64_mode = clut->meta;
	mach64_ramdac_io_t   *mach64_io   = clut->meta_io;
	kgi_u_t	cnt, src, dst;

	KRN_DEBUG(1, "Setting the clut");

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {

		if (am & KGI_AM_COLOR1) {

			mach64_mode->clut[dst+0] = data[src++] >> MACH64_SHIFT;
		}
		if (am & KGI_AM_COLOR2) {

			mach64_mode->clut[dst+1] = data[src++] >> MACH64_SHIFT;
		}
		if (am & KGI_AM_COLOR3) {

			mach64_mode->clut[dst+2] = data[src++] >> MACH64_SHIFT;
		}
		dst += 3;
		
	}

	MACH64_DAC_OUT8(mach64_io, 0xff, MACH64_DAC_MASK);
	MACH64_DAC_OUT8(mach64_io, index, MACH64_DAC_W_INDEX);
	MACH64_DAC_OUTS8(mach64_io, MACH64_DAC_DATA,
			 mach64_mode->clut + 3*index, 3*count);
	
	KRN_DEBUG(1, "clut set");
}

#if 0
void mach64_text16_hp_set_shape(kgic_mode_text16context_t *ctx,
	kgi_pointer64x64 *ptr)
{
	mach64_ramdac_io_t   *mach64_io   = KGIM_TEXT16_IO(ctx, ramdac);
	mach64_ramdac_mode_t *mach64_mode = KGIM_TEXT16_MODE(ctx, ramdac);
	kgi_u_t i;

	mach64_mode->ptr.hot.x = ptr->hotx;
	mach64_mode->ptr.hot.y = ptr->hoty;

	/*	update private cursor mode state
	*/
	switch (ptr->mode) {

	case KGI_PM_WINDOWS:
		break;

	case KGI_PM_X11:
		break;

	case KGI_PM_THREE_COLOR:
		break;

	default:
		KRN_INTERNAL_ERROR;
	}

#warning set cursor pattern in device.

	/*	set colors
	*/
#warning set cursor colors.
	for (i = 0; i < 3; i++) {

		/* set ptr->col[i].r	*/
		/* set ptr->col[i].g	*/
		/* set ptr->col[i].b	*/
	}
}
#endif
/*
static void mach64_text16_hp_show(kgic_mode_text16context_t *ctx, 
	kgi_u_t x, kgi_u_t y)
{
	mach64_ramdac_io_t   *mach64_io   = KGIM_TEXT16_IO(ctx, ramdac);
	mach64_ramdac_mode_t *mach64_mode = KGIM_TEXT16_MODE(ctx, ramdac);

	if ((mach64_mode->ptr.pos.x != x) || (mach64_mode->ptr.pos.y != y)) {

		mach64_mode->ptr.pos.x = x;
		mach64_mode->ptr.pos.y = y;

		x += mach64_mode->ptr.shift.x - mach64_mode->ptr.hot.x;
		y += mach64_mode->ptr.shift.y - mach64_mode->ptr.hot.y;

#warning set hardware cursor position.
	}
}

static void mach64_text16_hp_hide(kgic_mode_text16context_t *ctx)
{
	mach64_ramdac_io_t   *mach64_io   = KGIM_TEXT16_IO(ctx, ramdac);
	mach64_ramdac_mode_t *mach64_mode = KGIM_TEXT16_MODE(ctx, ramdac);

	mach64_mode->ptr.pos.x = mach64_mode->ptr.pos.y = 0x7FF;

#warning hide hardware cursor (e.g. set postion outside visible range)
}
*/
/*
** ----	end of text16 context functions ---------------------------------------
*/

kgi_error_t mach64_ramdac_init(mach64_ramdac_t *mach64,
	mach64_ramdac_io_t *mach64_io, const kgim_options_t *options)
{
	return KGI_EOK;
}

void mach64_ramdac_done(mach64_ramdac_t *mach64,
	mach64_ramdac_io_t *mach64_io, const kgim_options_t *options)
{
  
}

kgi_error_t mach64_ramdac_mode_check(mach64_ramdac_t *mach64,
	mach64_ramdac_io_t *mach64_io,
	mach64_ramdac_mode_t *mach64_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const mach64_ramdac_mode_record_t *rec;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = mach64_ramdac_mode_record(img[0].fam, img[0].bpfa);
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

		/* The integrated RAMDAC has no concept of Load Clock */
		dpm->lclk.mul = dpm->lclk.div = 1;

		/* Chipsets with integrated RAMDACs do not scale the clock
		   internally before feeding it to the RAMDAC */
		dpm->rclk.mul = dpm->rclk.div = 1;
		
		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames)
			img[0].frames = rec->frames;

 		return KGI_EOK;

	case KGI_TC_LOWER:

		if (dpm->dclk < mach64->ramdac.dclk.min) {

			KRN_DEBUG(1, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if (dpm->dclk > mach64->ramdac.dclk.max) {

			dpm->dclk = mach64->ramdac.dclk.max;
			KRN_DEBUG(2, "Clamping DCLK to %d", dpm->dclk);
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		if (dpm->dclk > mach64->ramdac.dclk.max) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
		if (dpm->dclk < mach64->ramdac.dclk.min) {

			dpm->dclk = mach64->ramdac.dclk.min;
		}
		return KGI_EOK;

	case KGI_TC_CHECK:
       
		if (dpm->lclk.mul != 1 || dpm->lclk.div != 1 ||
		    dpm->rclk.mul != 1 || dpm->rclk.div != 1) {
			
			KRN_DEBUG(0,"Mach64 Internal clock with rclk/lclk!!!");
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
		if ((dpm->dclk < mach64->ramdac.dclk.min) ||
		    (mach64->ramdac.dclk.max < dpm->dclk)) {


			KRN_DEBUG(1,"%i Hz DCLK (%i Hz LCLK) is out of bounds",
				  dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}
                if (mach64_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
						   KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	/* Note that there is nothing here for us to set. Everything (by that
	   I mean the color depth) is set by the chipset driver as it is
	   part of the chipset CRTC.
	*/
	
	mach64_mode->clut_ctrl.meta	= mach64_mode;
	mach64_mode->clut_ctrl.meta_io	= mach64_io;
	mach64_mode->clut_ctrl.prot	= KGI_PF_DRV_RWS;
	if (dpm->flags & KGI_DPF_CH_ALUT) {
	
		mach64_mode->clut_ctrl.type = KGI_RT_ALUT_CONTROL;
		mach64_mode->clut_ctrl.name = "alut control";
	}
	if (dpm->flags & KGI_DPF_CH_ILUT) {
	
		mach64_mode->clut_ctrl.type = KGI_RT_ILUT_CONTROL;
		mach64_mode->clut_ctrl.name = "ilut control";
	}
	mach64_mode->clut_ctrl.Set = mach64_ramdac_set_clut;
	
	mach64_mode->rec = rec;
	
	return KGI_EOK;
}

void mach64_ramdac_mode_enter(mach64_ramdac_t *mach64,
	mach64_ramdac_io_t *mach64_io,
	mach64_ramdac_mode_t *mach64_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	int i;
	
	MACH64_DAC_OUT8(mach64_io, 0xff, MACH64_DAC_MASK);
	MACH64_DAC_OUT8(mach64_io, 0, MACH64_DAC_W_INDEX);
	for(i = 0; i < 256; ++i){
		MACH64_DAC_OUT8(mach64_io, i, MACH64_DAC_DATA);
		MACH64_DAC_OUT8(mach64_io, i, MACH64_DAC_DATA);
		MACH64_DAC_OUT8(mach64_io, i, MACH64_DAC_DATA);
        }

}

kgi_resource_t *mach64_ramdac_image_resource(mach64_ramdac_t *mach64,
	mach64_ramdac_mode_t *mach64_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));
	
	switch (index) {
	
	case 0: return (kgi_resource_t *) &(mach64_mode->clut_ctrl);
	}
	return NULL;
}
