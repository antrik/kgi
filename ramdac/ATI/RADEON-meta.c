/* -----------------------------------------------------------------------------
**	ATI RADEON ramdac implementation
** -----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: RADEON-meta.c,v $
**	Revision 1.4  2004/02/15 22:11:59  aldot
**	- add missing parenthesis
**	
**	Revision 1.3  2002/10/10 21:47:27  redmondp
**	- fix up clut_set to work with changes in graphic
**	- fill in tables and entries in clut_ctrl resource
**	
**	Revision 1.2  2002/09/26 02:32:04  redmondp
**	Added palette support to ramdac
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#include <kgi/maintainers.h>
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.4 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/ATI/RADEON.h"
#include "ramdac/ATI/RADEON-meta.h"
#include "chipset/ATI/RADEON-meta.h"

/***	RADEON_MAGIC * device_value == 16bit_palette_value
****	16bit_value >> RADEON_SHIFT == device_value
****
****			MAGIC		SHIFT
****	8bit device:	0x0101		8
****	6bit device:	0x0410		10
***/
#define	RADEON_MAGIC	0x0101
#define	RADEON_SHIFT	6 /* using 10bit per component palette */

static void radeon_ramdac_set_clut(kgi_clut_t *clut,
	kgi_u_t table, kgi_u_t index, kgi_u_t count,
	kgi_attribute_mask_t am, const kgi_u16_t *data)
{
	radeon_ramdac_mode_t *radeon_mode = clut->meta;
	radeon_chipset_io_t *radeon_io = clut->meta_io;
	kgi_u_t dst = index * 3;
	/* graphic will only pass sane values so no need to check index, count,
	   etc. */
	KRN_ASSERT((am & ~KGI_AM_COLORS) == 0);

	RADEON_CTRL_OUT8(radeon_io, index, RADEON_PALETTE_INDEX);
	for (; count; count--) {
	
		kgi_u32_t rgb = 0;
		
		if (am & KGI_AM_COLOR1)
			radeon_mode->clut[dst+0] = *(data++) >> RADEON_SHIFT;
		if (am & KGI_AM_COLOR2)
			radeon_mode->clut[dst+1] = *(data++) >> RADEON_SHIFT;
		if (am & KGI_AM_COLOR3)
			radeon_mode->clut[dst+2] = *(data++) >> RADEON_SHIFT;
			
		/* form the palette entry and write it */
    		rgb |= (radeon_mode->clut[dst+0] << RADEON_PALETTE_30_DATA_RShift) &
			RADEON_PALETTE_30_DATA_RMask;
		rgb |= (radeon_mode->clut[dst+1] << RADEON_PALETTE_30_DATA_GShift) &
			RADEON_PALETTE_30_DATA_GMask;
		rgb |= (radeon_mode->clut[dst+2] << RADEON_PALETTE_30_DATA_BShift) &
			RADEON_PALETTE_30_DATA_BMask;
		
		RADEON_CTRL_OUT32(radeon_io, rgb, RADEON_PALETTE_30_DATA);
		dst += 3;
	}		
}


kgi_error_t radeon_ramdac_init(radeon_ramdac_t *radeon,
	radeon_ramdac_io_t *radeon_io, const kgim_options_t *options)
{
	kgi_u_t i;

	/* Save the initial palette */
	/* XXX The value writtent was actually 256 and truncated */
	RADEON_CTRL_OUT8(radeon_io, 0, RADEON_PALETTE_INDEX + 2);
	for (i = 0; i < 256; i++) {
	
		kgi_u32_t rgb = RADEON_CTRL_IN32(radeon_io, 
						 RADEON_PALETTE_30_DATA);
		
		radeon->clut[i*3+0] = (rgb & RADEON_PALETTE_30_DATA_RMask) >>
			RADEON_PALETTE_30_DATA_RShift;
		radeon->clut[i*3+0] = (rgb & RADEON_PALETTE_30_DATA_RMask) >>
			RADEON_PALETTE_30_DATA_RShift;
		radeon->clut[i*3+0] = (rgb & RADEON_PALETTE_30_DATA_RMask) >>
			RADEON_PALETTE_30_DATA_RShift;
	}		
	
	return KGI_EOK;
}

void radeon_ramdac_done(radeon_ramdac_t *radeon,
	radeon_ramdac_io_t *radeon_io, const kgim_options_t *options)
{
	//FIXME: restore the palette
}

kgi_error_t radeon_ramdac_mode_check(radeon_ramdac_t *radeon,
	radeon_ramdac_io_t *radeon_io,
	radeon_ramdac_mode_t *radeon_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;

	KRN_ASSERT((1 == images) && (NULL != img));
	KRN_ASSERT(NULL != img->out);

	if (cmd == KGI_TC_CHECK) {

		// FIXME: set default palette
	}

	radeon_mode->clut_ctrl.meta = radeon_mode;
	radeon_mode->clut_ctrl.meta_io = radeon_io;
	radeon_mode->clut_ctrl.prot = KGI_PF_DRV_RWS;
	if (dpm->flags & KGI_DPF_CH_ALUT) {
	
		radeon_mode->clut_ctrl.type = KGI_RT_ALUT_CONTROL;
		radeon_mode->clut_ctrl.name = "alut control";
	}
	if (dpm->flags & KGI_DPF_CH_ILUT) {
	
		radeon_mode->clut_ctrl.type = KGI_RT_ILUT_CONTROL;
		radeon_mode->clut_ctrl.name = "ilut control";
	}
	radeon_mode->clut_ctrl.Set = radeon_ramdac_set_clut;
	radeon_mode->clut_ctrl.tables = 1;
	radeon_mode->clut_ctrl.entries = 256;
	
	return KGI_EOK;
}

void radeon_ramdac_mode_enter(radeon_ramdac_t *radeon,
	radeon_ramdac_io_t *radeon_io,
	radeon_ramdac_mode_t *radeon_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t i;

	/* XXX The value writtent was actually 256 and truncated */
	RADEON_CTRL_OUT8(radeon_io, 0, RADEON_PALETTE_INDEX);
	for (i = 0; i < 256; i++) {
	
		kgi_u32_t rgb = 0;
		
    		rgb |= (i << 2) << RADEON_PALETTE_30_DATA_RShift;
		rgb |= (i << 2) << RADEON_PALETTE_30_DATA_GShift;
		rgb |= (i << 2) << RADEON_PALETTE_30_DATA_BShift;
		
		RADEON_CTRL_OUT32(radeon_io, rgb, RADEON_PALETTE_30_DATA);
	}		
}

kgi_resource_t *radeon_ramdac_image_resource(radeon_ramdac_t *radeon,
	radeon_ramdac_mode_t *radeon_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));
	
	switch (index) {
	
	case 0: return (kgi_resource_t *) &(radeon_mode->clut_ctrl);
	}
	return NULL;
}
