/* -----------------------------------------------------------------------------
**	S3 ViRGE PLL meta language binding
** -----------------------------------------------------------------------------
**	Copyright (C) 	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: ViRGE-bind.c,v $
**	Revision 1.2  2003/01/30 13:55:42  foske
**	Added resources.
**	
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Jon_Taylor
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"
#define	DEBUG_LEVEL	2

#include <kgi/module.h>
#include "clock/S3/ViRGE-meta.h"

kgi_error_t virge_clock_init_module(virge_clock_t *virge, 
	virge_clock_io_t *virge_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_clock_init_module()");
	
	kgim_strcpy(virge->pll.clock.vendor, "S3");
	kgim_strcpy(virge->pll.clock.model, "ViRGE integrated clock");
	
	virge->pll.clock.revision	= KGIM_CLOCK_REVISION;
/*	virge->pll.clock.flags	*/
	virge->pll.clock.mode_size	= sizeof(virge_clock_mode_t);
/*	virge->pll.clock.Command	*/

	virge->pll.clock.type	= KGIM_CT_PROG;
	virge->pll.clock.dclk.range[0].min =    250000;
	virge->pll.clock.dclk.range[0].max = 110000000;

	virge->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);
	virge->pll.a.mul = virge->pll.a.div = 1;
	virge->pll.fvco.min = KGIM_DEFAULT(options->clock->fvco_min, 135000000);
	virge->pll.fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 270000000);
	virge->pll.p.min = 0;
	virge->pll.p.max = 3;		/* post-VCO divider range	*/
	virge->pll.mul.min = 3;
	virge->pll.mul.max = 129;	/* dividend range		*/
	virge->pll.div.min = 3;
	virge->pll.div.max = 33;	/* divisor range		*/

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER,
		virge->pll.clock.vendor, virge->pll.clock.model);

	KRN_DEBUG(2, "PLL parameter %i, %i-%i; options %i, %i-%i", 
		  virge->pll.fref, virge->pll.fvco.min, virge->pll.fvco.max,
		  options->clock->fref, options->clock->fvco_min,
		  options->clock->fvco_max);

	return KGI_EOK;
}

void virge_clock_done_module(virge_clock_t *virge, virge_clock_io_t *virge_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_clock_done_module()");	
}

static const kgim_meta_t virge_clock_meta =
{
	(kgim_meta_init_module_fn *)	virge_clock_init_module,
	(kgim_meta_done_module_fn *)	virge_clock_done_module,
	(kgim_meta_init_fn *)		virge_clock_init,
	(kgim_meta_done_fn *)		virge_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	virge_clock_mode_prepare,
	(kgim_meta_mode_enter_fn *)	virge_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(virge_clock_t),
	0,
	sizeof(virge_clock_mode_t)
};
