/* -----------------------------------------------------------------------------
**	PERMEDIA2 PLL meta language binding
** -----------------------------------------------------------------------------
**	Copyright	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:58:59  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.1.1.1  2000/04/18 08:51:13  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"
#define	DEBUG_LEVEL	1

#include <kgi/module.h>
#include "clock/3Dlabs/PERMEDIA2-bind.h"

kgi_error_t pgc_clock_init_module(pgc_clock_t *pgc, pgc_clock_io_t *pgc_io,
	const kgim_options_t *options)
{
	kgim_strcpy(pgc->pll.clock.vendor, "3Dlabs Incorporated");
	kgim_strcpy(pgc->pll.clock.model, "PERMEDIA2 integrated clock");

	pgc->pll.clock.revision	= KGIM_CLOCK_REVISION;
/*	pgc->pll.clock.flags	*/
	pgc->pll.clock.mode_size	= sizeof(pgc_clock_mode_t);
/*	pgc->pll.clock.Command	*/

	pgc->pll.clock.type	= KGIM_CT_PROG;
	pgc->pll.clock.dclk.range[0].min =    400000;
	pgc->pll.clock.dclk.range[0].max = 120000000;

	pgc->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);
	pgc->pll.a.mul = pgc->pll.a.div = 1;
	pgc->pll.fvco.min = KGIM_DEFAULT(options->clock->fvco_min,  50000000);
	pgc->pll.fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 120000000);
	pgc->pll.p.min = 0;
	pgc->pll.p.max = 7;		/* post-VCO divider range	*/
	pgc->pll.mul.min = 1;
	pgc->pll.mul.max = 255;		/* dividend range		*/
	pgc->pll.div.min = 1;
	pgc->pll.div.max = 255;		/* divisor range		*/

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		pgc->pll.clock.vendor, pgc->pll.clock.model);

	KRN_DEBUG(2, "pll parameter %i, %i-%i; options %i, %i-%i", 
		pgc->pll.fref, pgc->pll.fvco.min, pgc->pll.fvco.max,
		options->clock->fref, options->clock->fvco_min,
		options->clock->fvco_max);

	return KGI_EOK;
}

const kgim_meta_t pgc_clock_meta =
{
	(kgim_meta_init_module_fn *)	pgc_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		pgc_clock_init,
	(kgim_meta_done_fn *)		pgc_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	pgc_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(pgc_clock_t),
	0,
	sizeof(pgc_clock_mode_t)
};
