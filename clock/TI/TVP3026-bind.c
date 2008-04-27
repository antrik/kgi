/* ----------------------------------------------------------------------------
**	TI TVP3026 PLL binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: TVP3026-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 10:06:39  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1  2000/04/26 14:12:18  seeger_s
**	- added TI TVP3026 driver
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/TI/TVP3026.h"
#include "clock/TI/TVP3026-bind.h"

kgi_error_t tvp3026_clock_init_module(tvp3026_clock_t *tvp3026,
	tvp3026_clock_io_t *tvp3026_io,
	const kgim_options_t *options)
{
	kgi_u8_t id;

	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_ID, TVP3026_EDAC_ADDR); 
	if (TVP3026_EDAC3F_TVP3026_ID !=
		TVP3026_DAC_IN8(tvp3026_io,TVP3026_EDAC_DATA)) {

		KRN_ERROR("no Texas Instruments TVP3026 detected.");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}

	kgim_strcpy(tvp3026->pll.clock.vendor, "TI");
	kgim_strcpy(tvp3026->pll.clock.model, "tvp3026");

	tvp3026->pll.clock.revision	= KGIM_CLOCK_REVISION;
/*	tvp3026->pll.clock.flags	*/
	tvp3026->pll.clock.mode_size	= sizeof(tvp3026_clock_mode_t);
/*	tvp3026->pll.clock.Command	*/

	tvp3026->pll.clock.type	= KGIM_CT_PROG;
	tvp3026->pll.clock.dclk.range[0].min =  13800000;
	tvp3026->pll.clock.dclk.range[0].max = 250000000;

	tvp3026->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);

	tvp3026->pll.a.mul = 8;
	tvp3026->pll.a.div = 1;			/* factor		*/

	tvp3026->pll.fvco.min= KGIM_DEFAULT(options->clock->fvco_min,110000000);
	tvp3026->pll.fvco.max= KGIM_DEFAULT(options->clock->fvco_max,250000000);

	tvp3026->pll.p.min = 0;
	tvp3026->pll.p.max = 3;			/* post-divider range	*/

	tvp3026->pll.mul.min = 3;
	tvp3026->pll.mul.max = 64;		/* dividend range	*/

	tvp3026->pll.div.min = 3;
	tvp3026->pll.div.max = 25;		/* divisor range	*/

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		tvp3026->pll.clock.vendor, tvp3026->pll.clock.model);

	KRN_DEBUG(2, "pll parameter %i, %i-%i; options %i, %i-%i", 
		tvp3026->pll.fref, 
		tvp3026->pll.fvco.min, tvp3026->pll.fvco.max,
		options->clock->fref, options->clock->fvco_min,
		options->clock->fvco_max);

	return KGI_EOK;
}

const kgim_meta_t tvp3026_clock_meta =
{
	(kgim_meta_init_module_fn *)	tvp3026_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		tvp3026_clock_init,
	(kgim_meta_done_fn *)		tvp3026_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	tvp3026_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(tvp3026_clock_t),
	0,
	sizeof(tvp3026_clock_mode_t)
};
