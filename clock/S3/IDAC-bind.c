/* -----------------------------------------------------------------------------
**	S3 IDAC PLL meta language binding
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: IDAC-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/08/04 11:51:37  seeger_s
**	- merged driver posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- driver status 30% reported by Jos Hulzink
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"
#define	DEBUG_LEVEL	255

#include <kgi/module.h>
#include "clock/S3/IDAC-bind.h"

kgi_error_t idac_clock_init_module(idac_clock_t *idac, idac_clock_io_t *idac_io,
	const kgim_options_t *options)
{
	kgim_strcpy(idac->pll.clock.vendor, "S3 Incorporated");
	kgim_strcpy(idac->pll.clock.model, "TRIO / ViRGE integrated clock");

	idac->pll.clock.revision	= KGIM_CLOCK_REVISION;
/*	idac->pll.clock.flags	*/
	idac->pll.clock.mode_size	= sizeof(idac_clock_mode_t);
/*	idac->pll.clock.Command	*/

	idac->pll.clock.type	= KGIM_CT_PROG;
	idac->pll.clock.dclk.range[0].min =   400000;
	idac->pll.clock.dclk.range[0].max = 94500000;

	idac->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);
	idac->pll.a.mul = idac->pll.a.div = 1;
	idac->pll.fvco.min = KGIM_DEFAULT(options->clock->fvco_min, 135000000);
	idac->pll.fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 270000000);
	idac->pll.p.min = 0;
	idac->pll.p.max = 3;		/* post-VCO divider range	*/
	idac->pll.mul.min = 3;
	idac->pll.mul.max = 129;	/* dividend range		*/
	idac->pll.div.min = 3;
	idac->pll.div.max = 33;		/* divisor range		*/

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		idac->pll.clock.vendor, idac->pll.clock.model);

	KRN_DEBUG(2, "pll parameter %i, %i-%i; options %i, %i-%i", 
		idac->pll.fref, idac->pll.fvco.min, idac->pll.fvco.max,
		options->clock->fref, options->clock->fvco_min,
		options->clock->fvco_max);

	return KGI_EOK;
}

const kgim_meta_t idac_clock_meta =
{
	(kgim_meta_init_module_fn *)	idac_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		idac_clock_init,
	(kgim_meta_done_fn *)		idac_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	idac_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	idac_clock_mode_leave,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(idac_clock_t),
	0,
	sizeof(idac_clock_mode_t)
};
