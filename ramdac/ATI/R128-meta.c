/* -----------------------------------------------------------------------------
**	ATI Rage 128 ramdac implementation
** -----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: R128-meta.c,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Paul_Redmond
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include <kgi/module.h>

#include "ramdac/ATI/R128.h"
#include "ramdac/ATI/R128-meta.h"

/***	R128_MAGIC * device_value == 16bit_palette_value
****	16bit_value >> R128_SHIFT == device_value
****
****			MAGIC		SHIFT
****	8bit device:	0x0101		8
****	6bit device:	0x0410		10
***/
#define	R128_MAGIC	0x0101
#define	R128_SHIFT	8

typedef struct 
{
	kgi_u8_t		ilut;
	kgi_u8_t		alut;

	kgi_attribute_mask_t	dam;
	const kgi_u8_t		*bpda;

	kgi_u8_t		frames;
} r128_ramdac_mode_record_t;

static const r128_ramdac_mode_record_t r128_ramdac_mode[] =
{
	{ 1, 0, KGI_AM_TEXT, KGI_AS_448,	1 },

	{ 1, 0, KGI_AM_I,    KGI_AS_8,		1 },
	{ 0, 1, KGI_AM_RGB,  KGI_AS_332,	1 },
	{ 0, 1, KGI_AM_RGB,  KGI_AS_565,	1 },
	{ 0, 1, KGI_AM_RGBX, KGI_AS_5551,	1 },
	{ 0, 1, KGI_AM_RGB,  KGI_AS_888,	1 },
	{ 0, 1, KGI_AM_RGBX, KGI_AS_8888,	1 }

};

#define	R128_RAMDAC_NR_MODES \
	(sizeof(r128_ramdac_mode)/sizeof(r128_ramdac_mode[0]))

static inline const r128_ramdac_mode_record_t *r128_ramdac_mode_record(
	kgi_attribute_mask_t fam, const kgi_u8_t *bpfa)
{
	kgi_u_t i;

	for (i = 0; i < R128_RAMDAC_NR_MODES; i++) {
	  
		if (r128_ramdac_mode[i].dam != fam)
			continue;
		if (kgim_strcmp(r128_ramdac_mode[i].bpda, bpfa) != 0)
			continue;

		break;
	}
	return (i == R128_RAMDAC_NR_MODES) ? NULL : r128_ramdac_mode + i;
}

/*
** ----------------------------------------------------------------------------
**	image resources
*/

static void r128_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	r128_ramdac_mode_t	*r128_mode	= clut->meta;
	r128_ramdac_io_t	*r128_io	= clut->meta_io;
	kgi_u_t	cnt, src, dst;

	KRN_ASSERT(table == 0);
	KRN_ASSERT(count > 0);

	cnt = count;
	dst = index * 3;
	src = 0;

	while (cnt--) {

		if (am & KGI_AM_COLOR1) {

			r128_mode->clut[dst+0] = 
				data[src++] >> R128_SHIFT;
		}
		if (am & KGI_AM_COLOR2) {

			r128_mode->clut[dst+1] =
				data[src++] >> R128_SHIFT;
		}
		if (am & KGI_AM_COLOR3) {

			r128_mode->clut[dst+2] =
				data[src++] >> R128_SHIFT;
		}
		dst += 3;
	}

}

void r128_ramdac_ptr_set_mode(kgi_marker_t *ptr, kgi_marker_mode_t mode)
{
	r128_ramdac_mode_t	*r128_mode	= ptr->meta;
	r128_ramdac_io_t	*r128_io	= ptr->meta_io;
	
	/*	update private cursor mode state
	*/
/*	switch (ptr->mode) {

	case KGI_PM_WINDOWS:
		break;

	case KGI_PM_X11:
		break;

	case KGI_PM_THREE_COLOR:
		break;

	default:
		KRN_INTERNAL_ERROR;
	}
*/
}

static void r128_ramdac_ptr_set_shape(kgi_marker_t *ptr, kgi_u_t shape,
	kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
	const kgi_rgb_color_t *color)
{
	r128_ramdac_mode_t	*r128_mode	= ptr->meta;
	r128_ramdac_io_t	*r128_io	= ptr->meta_io;
	kgi_u_t i;

	r128_mode->ptr_hot.x = hot_x;
	r128_mode->ptr_hot.y = hot_y;

	/*	set pattern
	*/

	/*	set colors
	*/

}

static void r128_ramdac_ptr_show(kgi_marker_t *ptr, kgi_u_t x, kgi_u_t y)
{
	r128_ramdac_mode_t	*r128_mode	= ptr->meta;
	r128_ramdac_io_t	*r128_io	= ptr->meta_io;

	x += r128_mode->ptr_shift.x - r128_mode->ptr_hot.x;
	y += r128_mode->ptr_shift.y - r128_mode->ptr_hot.y;

}

static void r128_ramdac_ptr_hide(kgi_marker_t *ptr)
{
	r128_ramdac_mode_t *r128_mode = ptr->meta;
	r128_ramdac_io_t *r128_io = ptr->meta_io;

}

#define r128_ramdac_ptr_undo	NULL


/*
** ----------------------------------------------------------------------------
**	global ramdac operations
*/

kgi_error_t r128_ramdac_init(r128_ramdac_t *r128,
	r128_ramdac_io_t *r128_io, const kgim_options_t *options)
{
	kgi_u32_t *clut;
	kgi_u32_t i;
	
	/* save the clut */
	KRN_DEBUG(1, "saving initial palette");
	r128->palette_index = R128_CTRL_IN32(r128_io, R128_PALETTE_INDEX);
	R128_CTRL_OUT32(r128_io, 0x0, R128_PALETTE_INDEX + 2);
	for (i = 0, clut = r128->clut; i < 256; i++, clut++) {

		*clut = R128_CTRL_IN32(r128_io, R128_PALETTE_DATA);
	}
	
	/* save the cursor */
	KRN_DEBUG(1, "saving initial cursor");
	r128->cur_offset = R128_CTRL_IN32(r128_io, R128_CUR_OFFSET);
	r128->cur_horz_vert_posn = 
		R128_CTRL_IN32(r128_io, R128_CUR_HORZ_VERT_POSN);
	r128->cur_horz_vert_off = 
		R128_CTRL_IN32(r128_io, R128_CUR_HORZ_VERT_OFF);
	r128->cur_clr0 = R128_CTRL_IN32(r128_io, R128_CUR_CLR0);
	r128->cur_clr1 = R128_CTRL_IN32(r128_io, R128_CUR_CLR1);
	
	return KGI_EOK;
}

void r128_ramdac_done(r128_ramdac_t *r128,
	r128_ramdac_io_t *r128_io, const kgim_options_t *options)
{
	kgi_u32_t *clut;
	kgi_u32_t i;
	
	/* restore the clut */
	KRN_DEBUG(1, "restoring initial palette");
	R128_CTRL_OUT8(r128_io, 0x0, R128_PALETTE_INDEX);
	for (i = 0, clut = r128->clut; i < 256; i++, clut++) {

		R128_CTRL_OUT32(r128_io, *clut, R128_PALETTE_DATA);
	}
	R128_CTRL_OUT32(r128_io, r128->palette_index, R128_PALETTE_INDEX);

	/* restore cursor */
	KRN_DEBUG(1, "restoring initial cursor");
	R128_CTRL_OUT32(r128_io, r128->cur_offset, R128_CUR_OFFSET);
	R128_CTRL_OUT32(r128_io, r128->cur_horz_vert_posn,
		R128_CUR_HORZ_VERT_POSN);
	R128_CTRL_OUT32(r128_io, r128->cur_horz_vert_off,
		R128_CUR_HORZ_VERT_OFF);
	R128_CTRL_OUT32(r128_io, r128->cur_clr0, R128_CUR_CLR0);
	R128_CTRL_OUT32(r128_io, r128->cur_clr1, R128_CUR_CLR1);
}

kgi_error_t r128_ramdac_mode_check(r128_ramdac_t *r128,
	r128_ramdac_io_t *r128_io,
	r128_ramdac_mode_t *r128_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm;
	const r128_ramdac_mode_record_t *rec;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	dpm = img->out;
	rec = r128_ramdac_mode_record(img[0].fam, img[0].bpfa);
	if (rec == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes (dam %.8x)",
			dpm->dam);
		return -KGI_ERRNO(RAMDAC, UNKNOWN);
	}

	if (cmd == KGI_TC_PROPOSE) {
	
		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= rec->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= rec->ilut ? KGI_DPF_CH_ILUT : 0;
		
		dpm->lclk.mul = dpm->lclk.div = 1;
		dpm->rclk.mul = dpm->rclk.div = 1;
		
		dpm->dam  = rec->dam;
		dpm->bpda = rec->bpda;

		if (0 == img[0].frames)
			img[0].frames = rec->frames;

 		return KGI_EOK;
	}

	if (cmd ==  KGI_TC_CHECK) {
	
		KRN_DEBUG(3, "KGI_TC_CHECK:");
	}
	
	/*	Now everything is checked and should be sane.
	**	Proceed to setup device dependent mode.
	*/

	return KGI_EOK;
}

void r128_ramdac_mode_enter(r128_ramdac_t *r128,
	r128_ramdac_io_t *r128_io,
	r128_ramdac_mode_t *r128_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u32_t i;
	
	KRN_DEBUG(1, "setting palette for mode");
	R128_CTRL_OUT8(r128_io, 0x0, R128_PALETTE_INDEX);

	for (i = 0; i < 256; i++) {

		kgi_u32_t data =  
			(i << R128_B4_PALETTE_DATA_RShift) |
			(i << R128_B4_PALETTE_DATA_GShift) |
			(i << R128_B4_PALETTE_DATA_BShift);
		R128_CTRL_OUT32(r128_io, data, R128_PALETTE_DATA);
	}

}

kgi_resource_t *r128_ramdac_image_resource(r128_ramdac_t *r128,
	r128_ramdac_mode_t *r128_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT(NULL != img);
/*
	switch (index) {

	case 0:	return (kgi_resource_t *) &(r128_mode->clut_ctrl);
	case 1:	return (kgi_resource_t *) &(r128_mode->ptr_ctrl);

	}*/
	return NULL;
}
