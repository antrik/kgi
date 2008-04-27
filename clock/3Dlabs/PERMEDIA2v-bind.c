/* -----------------------------------------------------------------------------
**	PERMEDIA2v PLL meta language binding
** -----------------------------------------------------------------------------
**	Copyright	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2v-bind.c,v $
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
#include "clock/3Dlabs/PERMEDIA2v-bind.h"

kgi_error_t pgc2v_clock_init_module(pgc2v_clock_t *pgc2v,
	pgc2v_clock_io_t *pgc2v_io, const kgim_options_t *options)
{
	kgim_strcpy(pgc2v->pll.clock.vendor, "3Dlabs Incorporated");
	kgim_strcpy(pgc2v->pll.clock.model, "PERMEDIA2v integrated clock");

	pgc2v->pll.clock.revision	= KGIM_CLOCK_REVISION;
/*	pgc2v->pll.clock.flags	*/
	pgc2v->pll.clock.mode_size	= sizeof(pgc2v_clock_mode_t);
/*	pgc2v->pll.clock.Command	*/

	pgc2v->pll.clock.type	= KGIM_CT_PROG;
	pgc2v->pll.clock.dclk.range[0].min =    400000;
	pgc2v->pll.clock.dclk.range[0].max = 120000000;

	pgc2v->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);
	pgc2v->pll.a.mul = pgc2v->pll.a.div = 1;
	pgc2v->pll.fvco.min = KGIM_DEFAULT(options->clock->fvco_min,  50000000);
	pgc2v->pll.fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 120000000);
	pgc2v->pll.p.min = 0;
	pgc2v->pll.p.max = 4;		/* post-VCO divider range	*/
	pgc2v->pll.mul.min = 1;
	pgc2v->pll.mul.max = 255;	/* dividend range		*/
	pgc2v->pll.div.min = 1;
	pgc2v->pll.div.max = 255;	/* divisor range		*/

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		pgc2v->pll.clock.vendor, pgc2v->pll.clock.model);

	return KGI_EOK;
}

const kgim_meta_t pgc2v_clock_meta =
{
	(kgim_meta_init_module_fn *)	pgc2v_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		pgc2v_clock_init,
	(kgim_meta_done_fn *)		pgc2v_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	pgc2v_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(pgc2v_clock_t),
	0,
	sizeof(pgc2v_clock_mode_t)
};
