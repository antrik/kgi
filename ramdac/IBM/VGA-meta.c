/* ----------------------------------------------------------------------------
**	VGA DAC meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-meta.c,v $
**	Revision 1.3  2003/03/25 03:42:43  fspacek
**	- palette support
**	
**	Revision 1.2  2003/01/25 02:27:57  aldot
**	- sync up to current API, (first part -- 20021001)
**	
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		The_KGI_Project
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.3 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	3
#endif

#include <kgi/module.h>

#include "ramdac/IBM/VGA.h"
#include "ramdac/IBM/VGA-meta.h"


#define VGA_MAGIC	0x0101
#define VGA_SHIFT	10

struct vga_ramdac_mode_record_s
{
	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_ratio_t		best_lclk;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
};

static const vga_ramdac_mode_record_t vga_ramdac_mode[] =
{
	{ 1, 0, {1, 1},	KGI_AM_TEXT, KGI_AS_448,	1 },

	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_1,		1 },
	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_2,		1 },
	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_4,		1 },
	{ 1, 0, {1, 1},	KGI_AM_I,    KGI_AS_8,		1 }	
};
#define	VGA_RAMDAC_NR_MODES (sizeof(vga_ramdac_mode) / sizeof(vga_ramdac_mode[0]))

static inline const vga_ramdac_mode_record_t *vga_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	KRN_DEBUG(2, "vga_ramdac_mode_record()");


	for (i = 0; i < VGA_RAMDAC_NR_MODES; i++) {

		if (vga_ramdac_mode[i].dam != fam) {

			continue;
		}

		if (kgim_strcmp(vga_ramdac_mode[i].bpda, bpfa) != 0) {

			continue;
		}

		break;
	}

	if (i == VGA_RAMDAC_NR_MODES) {

		KRN_ERROR("Couldn't find a ramdac mode");

	} else {

		KRN_DEBUG(2, "Found ramdac mode %i", i);
	}

	return (i == VGA_RAMDAC_NR_MODES) ? NULL : vga_ramdac_mode + i;
}

/* ----------------------------------------------------------------------------
**	image resources
*/

static void vga_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	vga_ramdac_mode_t *vga_mode = clut->meta;
	vga_ramdac_io_t   *vga_io   = clut->meta_io;
	kgi_u_t	cnt, src, dst;

	KRN_DEBUG(2, "vga_ramdac_set_clut()");
KRN_DEBUG(1,"table=%.8x index=%d count=%d am=%.8x", table,index,count,am);
	KRN_ASSERT(table == 0);
	KRN_ASSERT(count > 0);

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {

		if (am & KGI_AM_COLOR1) {

			vga_mode->clut[dst+0] = data[src++] >> VGA_SHIFT;
		}
		
		if (am & KGI_AM_COLOR2) {

			vga_mode->clut[dst+1] = data[src++] >> VGA_SHIFT;
		}
		
		if (am & KGI_AM_COLOR3) {

			vga_mode->clut[dst+2] = data[src++] >> VGA_SHIFT;
		}
		
		dst += 3;
	}

	VGA_DAC_OUT8(vga_io, index, VGA_DAC_PW_INDEX);
	VGA_DAC_OUTS8(vga_io, VGA_DAC_P_DATA,
		vga_mode->clut + 3*index, 3*count);
}



/* ----------------------------------------------------------------------------
**	global ramdac operations
*/

kgi_error_t vga_ramdac_init(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);
#if 1
	VGA_DAC_OUT8(vga_io, 0, VGA_DAC_PR_INDEX);
	VGA_DAC_INS8(vga_io, VGA_DAC_P_DATA, vga->clut, sizeof(vga->clut));

/*	kgi_u8_t cursor_palette[3*4];
**	kgi_u8_t cursor_data[];
*/
#endif
	return KGI_EOK;
}

void vga_ramdac_done(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "vga_ramdac_done()");
	
#if 1
	VGA_DAC_OUT8(vga_io, 0, VGA_DAC_PW_INDEX);
	VGA_DAC_OUTS8(vga_io, VGA_DAC_P_DATA, vga->clut, sizeof(vga->clut));

/*
	cursor_palette[3*4];
	cursor_data[];
*/
#endif
}

kgi_error_t vga_ramdac_mode_check(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	vga_ramdac_mode_t *vga_mode,kgi_timing_command_t cmd, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t lclk;
	kgi_dot_port_mode_t *dpm;
	const vga_ramdac_mode_record_t *rec;

	KRN_DEBUG(2, "vga_ramdac_mode_check()");

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = vga_ramdac_mode_record(img[0].fam, img[0].bpfa);

	if (rec == NULL) {

		KRN_DEBUG(2, "Couldn't handle dot attributes (dam %.8x)",
			dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE:");

		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;

		dpm->lclk.mul = dpm->lclk.mul ? dpm->lclk.mul : 1;
		dpm->lclk.div = dpm->lclk.div ? dpm->lclk.div : 1;
		dpm->rclk.mul = dpm->rclk.mul ? dpm->rclk.mul : 1;
		dpm->rclk.div = dpm->rclk.div ? dpm->rclk.div : 1;

		dpm->dclk = vga->ramdac.lclk.max * dpm->lclk.mul /
			dpm->lclk.div;

		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames) {

			KRN_DEBUG(2, "frames == 0, setting to %i", rec->frames);
			img[0].frames = rec->frames;
		}

		KRN_DEBUG(3, "dclk %d, lclk %i:%i, rclk %i:%i, dam 0x%.8x",
			dpm->dclk,
			dpm->lclk.mul, dpm->lclk.div,
			dpm->rclk.mul, dpm->rclk.div,
			dpm->dam);

 		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(3, "KGI_TC_LOWER:");

		if ((dpm->lclk.mul > 0) && (dpm->lclk.div > 0)) {

			lclk = vga->ramdac.lclk.max * dpm->lclk.mul /
				dpm->lclk.div;
		} else {

			KRN_DEBUG(1, "Invalid LCLK mul or div (%i,%i)",
				dpm->lclk.mul, dpm->lclk.div);
			return -KGI_ERRNO(RAMDAC, UNKNOWN);
		}


		if ((lclk < vga->ramdac.lclk.min) ||
			(dpm->dclk < vga->ramdac.dclk.min)) {

			KRN_DEBUG(2, "%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

		if ((lclk > vga->ramdac.lclk.max) ||
			(dpm->dclk > vga->ramdac.dclk.max)) {

			register kgi_u_t dclk = vga->ramdac.lclk.max *
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk < vga->ramdac.dclk.max)
				? dclk : vga->ramdac.dclk.max;

			KRN_DEBUG(2, "dpm->dclk = %i", dpm->dclk);
		}

		return KGI_EOK;

	case KGI_TC_RAISE:
		KRN_DEBUG(3, "KGI_TC_RAISE:");

		if ((dpm->lclk.mul > 0) && (dpm->lclk.div > 0)) {

			lclk = vga->ramdac.lclk.max * dpm->lclk.mul /
				dpm->lclk.div;
		} else {

			KRN_DEBUG(1, "Invalid LCLK mul or div (%i,%i)", 
				dpm->lclk.mul, dpm->lclk.div);
			return -KGI_ERRNO(RAMDAC, UNKNOWN);
		}


		if ((lclk > vga->ramdac.lclk.max) ||
			(dpm->dclk > vga->ramdac.dclk.max)) {

			KRN_DEBUG(2, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

		if ((lclk < vga->ramdac.lclk.min) ||
			(dpm->dclk < vga->ramdac.dclk.min)) {

			register kgi_u_t dclk = vga->ramdac.dclk.min * 
				dpm->lclk.div / dpm->lclk.mul;

			dpm->dclk = (dclk > vga->ramdac.dclk.min) 
				? dclk : vga->ramdac.dclk.min;

			KRN_DEBUG(2, "dpm->dclk = %i", dpm->dclk);
		}

		return KGI_EOK;

     case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK:");

		if (! ((dpm->lclk.mul == rec->best_lclk.mul) &&
			(dpm->lclk.div == rec->best_lclk.div))) {

			KRN_ERROR("Unsupported lclk mul (%i) or div (%i)",
				dpm->lclk.mul, dpm->lclk.div);
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		lclk = dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

		if ((dpm->dclk < vga->ramdac.dclk.min) || 
			(vga->ramdac.dclk.max < dpm->dclk) ||
			(lclk < vga->ramdac.lclk.min) || 
			(vga->ramdac.lclk.max < lclk)) {

			KRN_ERROR("%i Hz DCLK (%i Hz LCLK) is out of bounds",
				dpm->dclk, lclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (vga_mode->kgim.crt->sync & 
			(KGIM_ST_SYNC_ON_RED | KGIM_ST_SYNC_ON_BLUE)) {

			KRN_ERROR("Can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

		/*	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		vga_mode->rec = rec;

#if 1
		vga_mode->ptr.pos.x = vga_mode->ptr.pos.y = 0x3FF;
		vga_mode->ptr.hot.x = vga_mode->ptr.hot.y = 0;
		vga_mode->ptr.shift.x = 64 + vga_mode->kgim.crt->x.total - 
			vga_mode->kgim.crt->x.blankend;
		vga_mode->ptr.shift.y = 64 + vga_mode->kgim.crt->y.total - 
			vga_mode->kgim.crt->y.blankend;
#else

		vga_mode->ptr_shift.x	= 64 +
			vga_mode->kgim.crt->x.total -
			vga_mode->kgim.crt->x.blankend;
		vga_mode->ptr_shift.y	= 64 +
			vga_mode->kgim.crt->y.total -
			vga_mode->kgim.crt->y.blankend;


		vga_mode->ptr_ctrl.meta		= vga_mode;
		vga_mode->ptr_ctrl.meta_io	= vga_io;
		vga_mode->ptr_ctrl.type		= KGI_RT_POINTER_CONTROL;
		vga_mode->ptr_ctrl.prot		= KGI_PF_DRV_RWS;
		vga_mode->ptr_ctrl.name		= "VGA pointer control";
		vga_mode->ptr_ctrl.modes	=
			KGI_MM_WINDOWS | KGI_MM_X11 | KGI_MM_3COLOR;
		vga_mode->ptr_ctrl.shapes	= 1;
		vga_mode->ptr_ctrl.size.x	=
		vga_mode->ptr_ctrl.size.y	= 64;

		vga_mode->ptr_ctrl.SetMode	= vga_ramdac_ptr_set_mode;
		vga_mode->ptr_ctrl.Select	= NULL;
		vga_mode->ptr_ctrl.SetShape	= vga_ramdac_ptr_set_shape;
		vga_mode->ptr_ctrl.Show		= vga_ramdac_ptr_show;
		vga_mode->ptr_ctrl.Hide		= vga_ramdac_ptr_hide;
		vga_mode->ptr_ctrl.Undo		= vga_ramdac_ptr_undo;
#endif

		vga_mode->clut_ctrl.meta	= vga_mode;
		vga_mode->clut_ctrl.meta_io	= vga_io;
		vga_mode->clut_ctrl.prot	= KGI_PF_DRV_RWS;
		if (dpm->flags & KGI_DPF_CH_ALUT) {
			vga_mode->clut_ctrl.type = KGI_RT_ALUT_CONTROL;
			vga_mode->clut_ctrl.name = "alut control";
		}
		if (dpm->flags & KGI_DPF_CH_ILUT) {
			vga_mode->clut_ctrl.type = KGI_RT_ILUT_CONTROL;
			vga_mode->clut_ctrl.name = "ilut control";
		}
		vga_mode->clut_ctrl.Select	= NULL;
		vga_mode->clut_ctrl.Set		= vga_ramdac_set_clut;
		vga_mode->clut_ctrl.tables	= 1;
		vga_mode->clut_ctrl.entries	= 256;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}
}

void vga_ramdac_mode_enter(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io, 
	vga_ramdac_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t x, y;

	KRN_DEBUG(2, "vga_ramdac_mode_enter()");

	VGA_DAC_OUT8(vga_io, 0xff, VGA_DAC_PIXEL_MASK);

	x = vga_mode->ptr.pos.x + vga_mode->ptr.shift.x - vga_mode->ptr.hot.x;
	y = vga_mode->ptr.pos.y + vga_mode->ptr.shift.y - vga_mode->ptr.hot.y;	
}

kgi_resource_t *vga_ramdac_image_resource(
	vga_ramdac_t *vga, vga_ramdac_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));
	
	switch (index) {
	case 0:	return (kgi_resource_t *) &(vga_mode->clut_ctrl);
	/*case 1: return (kgi_resource_t *) &(vga_mode->ptr_ctrl);*/
	default:
		KRN_DEBUG(2, "Unknown resource %i requested", index);
		return NULL;
	}
	return NULL;
}
