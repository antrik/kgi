/* ----------------------------------------------------------------------------
**	Matrox Gx00 ramdac meta implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx00-meta.c,v 1.3 2002/12/22 22:32:46 ortalo Exp $
**	
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.3 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>
#include "chipset/Matrox/Gx00.h"
#include "ramdac/Matrox/Gx00-meta.h"

/*
**	IO helpers
*/
static inline void MGAG_EDAC_OUT8(mgag_ramdac_io_t *mgag_io,
				  kgi_u8_t val, kgi_u8_t reg)
{
  KRN_DEBUG(3, "EDAC[%.2x] <= %.2x", reg, val);
  MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
  MGAG_DAC_OUT8(mgag_io, val, X_DATAREG);
}

static inline kgi_u8_t MGAG_EDAC_IN8(mgag_ramdac_io_t *mgag_io, kgi_u8_t reg)
{
  kgi_u8_t val;
  MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
  val = MGAG_DAC_IN8(mgag_io, X_DATAREG);
  KRN_DEBUG(3, "EDAC[%.2x] => %.2x", reg, val);
  return val;
}

#define	MGAG_MAGIC	0x0101
/* Color shift */
#define	MGAG_SHIFT	8

/*
** ----------------------------------------------------------------------------
**	mode setup private functions
** ----------------------------------------------------------------------------
*/

/*
** Private struct definition
*/
struct mgag_ramdac_mode_record_s {

	kgi_u8_t			depth;

	kgi_u_t				ilut;
	
	kgi_u_t				pal_size;

	kgi_u_t				r_shift;
	kgi_u_t				g_shift;
	kgi_u_t				b_shift;

	kgi_attribute_mask_t		dam;

	const kgi_u8_t			*bpda;

};

static const mgag_ramdac_mode_record_t mgag_ramdac_mode[] =
{
	{ MUL_8_PAL,	1,	0,	0,	0,	0,	KGI_AM_TEXT,	KGI_AS_448	},
	{ MUL_8_PAL,	1,	256,	0,	0,	0,	KGI_AM_I,	KGI_AS_8	},
	{ MUL_15_PAL,	1,	32,	3,	3,	3,	KGI_AM_RGBA,	KGI_AS_5551	},
	{ MUL_16_PAL,	1,	64,	3,	2,	3,	KGI_AM_RGB,	KGI_AS_565	},
	{ MUL_24_PAL,	1,	256,	0,	0,	0,	KGI_AM_RGB,	KGI_AS_888	},
	{ MUL_32_PAL,	1,	256,	0,	0,	0,	KGI_AM_RGBX,	KGI_AS_8888	},
	{ MUL_32_DIR,	0,	0,	0,	0,	0,	KGI_AM_RGBA,	KGI_AS_8888	}
};

#define	MGAG_RAMDAC_NR_MODES (sizeof(mgag_ramdac_mode) / sizeof(mgag_ramdac_mode[0]))

static inline const mgag_ramdac_mode_record_t *mgag_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	KRN_DEBUG(2, "entered");

	for (i = 0; i < MGAG_RAMDAC_NR_MODES; i++) {

		if (mgag_ramdac_mode[i].dam != fam) {

			continue;
		}

		if (kgim_strcmp((char *)mgag_ramdac_mode[i].bpda, (char *)bpfa) != 0) {

			continue;
		}

		break;
	}

	if (i == MGAG_RAMDAC_NR_MODES) {

		KRN_DEBUG(1, "Couldn't find a ramdac mode");
	}

	KRN_DEBUG(2, "completed");

	return (i == MGAG_RAMDAC_NR_MODES) ? NULL : mgag_ramdac_mode + i;
}

/*
** ----------------------------------------------------------------------------
**	per image resources
** ----------------------------------------------------------------------------
*/

static void mgag_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	mgag_ramdac_mode_t *mgag_mode = clut->meta;
	mgag_ramdac_io_t   *mgag_io   = clut->meta_io;
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(table == 0);
	KRN_ASSERT(count > 0);

	KRN_DEBUG(1, "setting %i entries of CLUT (starting at %i)",
		  count, index);

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {
	  /* We only change the mode CLUT for specified colors */

		if (am & KGI_AM_COLOR1) {

			mgag_mode->clut[dst+0] = data[src++] >> MGAG_SHIFT;
		}
		if (am & KGI_AM_COLOR2) {

			mgag_mode->clut[dst+1] = data[src++] >> MGAG_SHIFT;
		}
		if (am & KGI_AM_COLOR3) {

			mgag_mode->clut[dst+2] = data[src++] >> MGAG_SHIFT;
		}
		dst += 3;
	}

	/* Set up the CLUT */
	MGAG_DAC_OUT8(mgag_io, index, PALWTADD);
	MGAG_DAC_OUTS8(mgag_io, PALDATA,
		       mgag_mode->clut + 3*index, 3*count);
}

static void mgag_ramdac_ptr_set_mode(kgi_marker_t *ptr, kgi_marker_mode_t mode)
{
	mgag_ramdac_mode_t *mgag_mode = ptr->meta;
	mgag_ramdac_io_t *mgag_io = ptr->meta_io;

	KRN_DEBUG(2,"entered");

	switch (mode) {

	case KGI_MM_WINDOWS:
		mgag_mode->xcurctrl = CUR_XGA;
		break;

	case KGI_MM_X11:
		mgag_mode->xcurctrl = CUR_X;
		break;

	case KGI_MM_3COLOR:
		mgag_mode->xcurctrl = CUR_3_COL;
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
	MGAG_EDAC_OUT8(mgag_io, mgag_mode->xcurctrl, XCURCTRL);

	KRN_DEBUG(2,"completed");
}

static void mgag_ramdac_ptr_set_shape(kgi_marker_t *ptr, kgi_u_t shape,
	kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
	const kgi_rgb_color_t *color)
{
	mgag_ramdac_mode_t *mgag_mode = ptr->meta;
	mgag_ramdac_io_t   *mgag_io   = ptr->meta_io;
	const kgi_u8_t *and_mask;
	const kgi_u8_t *xor_mask;
	kgi_u_t i,j,r;

	mgag_mode->ptr_hot.x = hot_x;
	mgag_mode->ptr_hot.y = hot_y;

	KRN_DEBUG(2,"entered");

	/*	set pattern
	*/
	and_mask = data;
	xor_mask = and_mask + 512;
	r = 0;
        for (i = 0; i < 512; i += 8) {
	  for (j = 8; j > 0; j--) {
	    MGAG_DAC_OUT8(mgag_io, xor_mask[i+j-1], PX_POINTER1 | r);
	    r++;
	  }
	  for (j = 8; j > 0; j--) {
	    MGAG_DAC_OUT8(mgag_io, and_mask[i+j-1], PX_POINTER1 | r);
	    r++;
	  }
	}

	/* set colors
	 */
	MGAG_EDAC_OUT8(mgag_io, color[0].r >> MGAG_SHIFT, XCURCOL0RED);
	MGAG_EDAC_OUT8(mgag_io, color[0].g >> MGAG_SHIFT, XCURCOL0GREEN);
	MGAG_EDAC_OUT8(mgag_io, color[0].b >> MGAG_SHIFT, XCURCOL0BLUE);
	MGAG_EDAC_OUT8(mgag_io, color[1].r >> MGAG_SHIFT, XCURCOL1RED);
	MGAG_EDAC_OUT8(mgag_io, color[1].g >> MGAG_SHIFT, XCURCOL1GREEN);
	MGAG_EDAC_OUT8(mgag_io, color[1].b >> MGAG_SHIFT, XCURCOL1BLUE);
	MGAG_EDAC_OUT8(mgag_io, color[2].r >> MGAG_SHIFT, XCURCOL2RED);
	MGAG_EDAC_OUT8(mgag_io, color[2].g >> MGAG_SHIFT, XCURCOL2GREEN);
	MGAG_EDAC_OUT8(mgag_io, color[2].b >> MGAG_SHIFT, XCURCOL2BLUE);

	KRN_DEBUG(2,"completed");
}

static void mgag_ramdac_ptr_show(kgi_marker_t *ptr, kgi_u_t x, kgi_u_t y)
{
	mgag_ramdac_mode_t *mgag_mode = ptr->meta;
	mgag_ramdac_io_t   *mgag_io   = ptr->meta_io;

	x += mgag_mode->ptr_shift.x - mgag_mode->ptr_hot.x;
	y += mgag_mode->ptr_shift.y - mgag_mode->ptr_hot.y;

	MGAG_DAC_OUT8(mgag_io, x,      CURPOSXL);
	MGAG_DAC_OUT8(mgag_io, x >> 8, CURPOSXH);
	MGAG_DAC_OUT8(mgag_io, y,      CURPOSYL);
	MGAG_DAC_OUT8(mgag_io, y >> 8, CURPOSYH);
}

static void mgag_ramdac_ptr_hide(kgi_marker_t *ptr)
{
 /*	mgag_ramdac_mode_t *mgag_mode = ptr->meta; */
	mgag_ramdac_io_t   *mgag_io   = ptr->meta_io;

	/* 0x000 can be used to hide the cursor according to the doc. */
	MGAG_DAC_OUT8(mgag_io, 0x00, CURPOSXL);
	MGAG_DAC_OUT8(mgag_io, 0x00, CURPOSXH);
	MGAG_DAC_OUT8(mgag_io, 0x00, CURPOSYL);
	MGAG_DAC_OUT8(mgag_io, 0x00, CURPOSYH);
}

#define	mgag_ramdac_ptr_undo	NULL

/*
** ----------------------------------------------------------------------------
**	global ramdac operations
** ----------------------------------------------------------------------------
*/

kgi_error_t mgag_ramdac_init(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "entered");

	mgag->saved.xzoom    = MGAG_EDAC_IN8(mgag_io, XZOOMCTRL);
	mgag->saved.xgenctrl = MGAG_EDAC_IN8(mgag_io, XGENCTRL);
	mgag->saved.xmulctrl = MGAG_EDAC_IN8(mgag_io, XMULCTRL);
	mgag->saved.xmiscctrl = MGAG_EDAC_IN8(mgag_io, XMISCCTRL);
	mgag->saved.pixrdmsk = MGAG_DAC_IN8(mgag_io, PIXRDMSK);

	MGAG_DAC_OUT8(mgag_io, 0, PALWTADD);
	MGAG_DAC_INS8(mgag_io, PALDATA, mgag->saved.clut, sizeof(mgag->saved.clut));

	KRN_DEBUG(1, "original ramdac state and palette (%i bytes) saved",
		  sizeof(mgag->saved.clut));

	KRN_DEBUG(2, "completed");

	return KGI_EOK;
}

void mgag_ramdac_done(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io, 
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "entered");

	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xzoom,    XZOOMCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xgenctrl, XGENCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xmulctrl, XMULCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag->saved.xmiscctrl, XMISCCTRL);
	MGAG_DAC_OUT8 (mgag_io, mgag->saved.pixrdmsk, PIXRDMSK);

	KRN_DEBUG(1, "original ramdac state restored");

	MGAG_DAC_OUT8(mgag_io, 0, PALWTADD);
	MGAG_DAC_OUTS8(mgag_io, PALDATA, mgag->saved.clut, sizeof(mgag->saved.clut));
	
	KRN_DEBUG(1, "original ramdac palette restored (%i bytes)",
		  sizeof(mgag->saved.clut));

	KRN_DEBUG(2, "completed");
}

kgi_error_t mgag_ramdac_mode_check( mgag_ramdac_t *mgag, 
	mgag_ramdac_io_t *mgag_io, mgag_ramdac_mode_t *mgag_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm;
	const mgag_ramdac_mode_record_t *rec;

	KRN_DEBUG(2, "entered");

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = mgag_ramdac_mode_record(img[0].fam, img[0].bpfa);

	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_DEBUG(3, "KGI_TC_PROPOSE:");

		if (0 == img[0].frames) {

		  /* TODO: check that it is the ramdac which should do that! */
			img[0].frames = 1;
		}

		/*	setting dot port mode. dclk and dots, untouched
		*/
		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;
 
		dpm->lclk.mul = dpm->lclk.div =
		dpm->rclk.mul = dpm->rclk.div = 1;

		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		KRN_DEBUG(2, "lclk %i:%i, dclk %i:%i", 
			dpm->lclk.mul, dpm->lclk.div, 
			dpm->rclk.mul, dpm->rclk.div);
 
 		return KGI_EOK;

	case KGI_TC_LOWER:

		KRN_DEBUG(3, "KGI_TC_LOWER:");

		if (dpm->dclk < mgag->ramdac.dclk.min) {

			KRN_DEBUG(1, "%i Hz DCLK is too low",
				dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

		if (dpm->dclk > mgag->ramdac.dclk.max) {

			dpm->dclk = mgag->ramdac.dclk.max;
		}

		return KGI_EOK;
	case KGI_TC_RAISE:

		KRN_DEBUG(3, "KGI_TC_RAISE:");

		if (dpm->dclk > mgag->ramdac.dclk.max) {

			KRN_DEBUG(1, "%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

		if (dpm->dclk < mgag->ramdac.dclk.min) {

			dpm->dclk = mgag->ramdac.dclk.min;
		}

		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_DEBUG(3, "KGI_TC_CHECK:");

		if (! ((dpm->lclk.mul == 1) && (dpm->lclk.div == 1) &&
		       (dpm->rclk.mul == 1) && (dpm->rclk.div == 1)) ) {

			KRN_DEBUG(1, "lclk/rclk div/mul not supported");
			return -KGI_ERRNO(RAMDAC, NOSUP);
		}

		if ((dpm->dclk < mgag->ramdac.dclk.min) || 
			(mgag->ramdac.dclk.max < dpm->dclk)) {

			KRN_DEBUG(1, "%i Hz DCLK is out of bounds", 
				dpm->dclk);
			return -KGI_ERRNO(RAMDAC, INVAL);
		}

                if (mgag_mode->kgim.crt->sync & (KGIM_ST_SYNC_ON_RED |
			KGIM_ST_SYNC_ON_BLUE)) {

			KRN_DEBUG(1, "can't do red or blue composite sync");
			return -KGI_ERRNO(RAMDAC, NOSUP);
                }

       		/*
		**	Now everything is checked and should be sane.
		**	Proceed to setup device dependent mode.
		*/
		mgag_mode->rec = rec;

		if (mgag_mode->kgim.crt->sync & KGIM_ST_SYNC_PEDESTRAL) {

			mgag_mode->xgenctrl |= XGENCTRL_PEDON;
		}

		if (!(mgag_mode->kgim.crt->sync & KGIM_ST_SYNC_ON_GREEN)) {

			mgag_mode->xgenctrl |= XGENCTRL_IOGSYNCDIS;
		}

		/*
		**  hardware pointer setup
		*/
		mgag_mode->xcurctrl = CUR_DIS; /* Cursor disabled */

		mgag_mode->ptr_shift.x	 = 64 +
			mgag_mode->kgim.crt->x.total -
			mgag_mode->kgim.crt->x.blankend;
		mgag_mode->ptr_shift.y	 = 64 +
			mgag_mode->kgim.crt->y.total -
			mgag_mode->kgim.crt->y.blankend;

		mgag_mode->ptr_ctrl.meta	= mgag_mode;
		mgag_mode->ptr_ctrl.meta_io	= mgag_io;
		mgag_mode->ptr_ctrl.type	= KGI_RT_POINTER_CONTROL;
		mgag_mode->ptr_ctrl.prot	= KGI_PF_DRV_RWS;
		mgag_mode->ptr_ctrl.name	= "pointer";
		mgag_mode->ptr_ctrl.modes	=
			KGI_MM_WINDOWS | KGI_MM_X11 | KGI_MM_3COLOR;
		mgag_mode->ptr_ctrl.shapes	= 1;
		mgag_mode->ptr_ctrl.size.x	= 64;
		mgag_mode->ptr_ctrl.size.y	= 64;
		mgag_mode->ptr_ctrl.SetMode	= mgag_ramdac_ptr_set_mode;
		mgag_mode->ptr_ctrl.Select	= NULL;
		mgag_mode->ptr_ctrl.SetShape	= mgag_ramdac_ptr_set_shape;
		mgag_mode->ptr_ctrl.Show	= mgag_ramdac_ptr_show;
		mgag_mode->ptr_ctrl.Hide	= mgag_ramdac_ptr_hide;
		mgag_mode->ptr_ctrl.Undo	= mgag_ramdac_ptr_undo;
#warning init mgag_mode->cursor_clut and mgag_mode->cursor_data

		/*
		**	clut setup
		*/
		mgag_mode->clut_ctrl.meta	= mgag_mode;
		mgag_mode->clut_ctrl.meta_io	= mgag_io;
		mgag_mode->clut_ctrl.prot	= KGI_PF_DRV_RWS;
		if (dpm->flags & KGI_DPF_CH_ALUT) {

			mgag_mode->clut_ctrl.type = KGI_RT_ALUT_CONTROL;
			mgag_mode->clut_ctrl.name = "alut control";
		}
		if (dpm->flags & KGI_DPF_CH_ILUT) {

			mgag_mode->clut_ctrl.type = KGI_RT_ILUT_CONTROL;
			mgag_mode->clut_ctrl.name = "ilut control";
		}
		mgag_mode->clut_ctrl.Set = mgag_ramdac_set_clut;
		mgag_mode->clut_ctrl.tables = 1;
		mgag_mode->clut_ctrl.entries = 256;

		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	KRN_DEBUG(2, "completed");
}

kgi_resource_t *mgag_ramdac_image_resource(mgag_ramdac_t *mgag,
	mgag_ramdac_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t image,
	kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));

	switch (index) {

	case 0:		return (kgi_resource_t *) &(mgag_mode->clut_ctrl);
	case 1:		return (kgi_resource_t *) &(mgag_mode->ptr_ctrl);

	}
	return NULL;
}

void mgag_ramdac_mode_enter(mgag_ramdac_t *mgag, mgag_ramdac_io_t *mgag_io,
	mgag_ramdac_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t	i = 0;

	KRN_DEBUG(2, "entered");

	MGAG_EDAC_OUT8(mgag_io, 0x00,	XZOOMCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag_mode->xgenctrl,	XGENCTRL);
	MGAG_EDAC_OUT8(mgag_io, mgag_mode->rec->depth,	XMULCTRL);

	MGAG_EDAC_OUT8(mgag_io, (XMISCCTRL_VGA8DAC | XMISCCTRL_MCFSEL_MAFC | 
			       XMISCCTRL_RAMCS | XMISCCTRL_DACPDN), XMISCCTRL);

	MGAG_DAC_OUT8(mgag_io, 0xFF, PIXRDMSK);

	if (mgag_mode->rec->ilut) {

		MGAG_DAC_OUT8(mgag_io, 0, PALWTADD);

		KRN_DEBUG(1, "setting %i palette entries to default (arbitrary) LUT",
			  mgag_mode->rec->pal_size);

		for (i=0;i<mgag_mode->rec->pal_size;i++)
		  {
#if 0
		    /* Setting a random palette w/ psychedelic colors */
		    kgi_u8_t r = (((79 * i) % 256) | 0x40)
		      << mgag_mode->rec->r_shift;
		    kgi_u8_t g = (((53 * i) % 256) | 0x40)
		      << mgag_mode->rec->g_shift;
		    kgi_u8_t b = (((71 * i) % 256) | 0x40)
		      << mgag_mode->rec->b_shift;
#else
		    kgi_u8_t r = i << mgag_mode->rec->r_shift;
		    kgi_u8_t g = i << mgag_mode->rec->g_shift;
		    kgi_u8_t b = i << mgag_mode->rec->b_shift;
#endif
		    
		    MGAG_DAC_OUT8(mgag_io, r, PALDATA);
		    MGAG_DAC_OUT8(mgag_io, g, PALDATA);
		    MGAG_DAC_OUT8(mgag_io, b, PALDATA);
		    KRN_DEBUG(5, "DAC PAL[%i] <= %.2x,%.2x,%.2x",i,r,g,b);
		  }
	}


	KRN_DEBUG(2, "completed");
}
