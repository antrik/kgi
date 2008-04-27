/* ----------------------------------------------------------------------------
**	Matrox Gx50 clock driver binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2002		Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx50-bind.c,v 1.1 2002/11/30 18:23:14 ortalo Exp $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

/* We do *not* use the pll meta-language. */
#include "clock/Matrox/Gx50-meta.h"
#include "chipset/Matrox/Gx00.h"

static kgi_error_t Gx50_clock_init_module(Gx50_clock_t *Gx50, Gx50_clock_io_t *Gx50_io,
	const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_DEBUG(2, "Gx50_clock_init_module()");

	KRN_ASSERT(Gx50);
	KRN_ASSERT(Gx50_io);
	KRN_ASSERT(options);

	kgim_memset(Gx50, 0, sizeof(*Gx50));

	kgim_strcpy(Gx50->clock.vendor, "Matrox Graphics Inc");
	kgim_strcpy(Gx50->clock.model,  "G450/G550 Integrated Clock");

	Gx50->clock.revision  = KGIM_CLOCK_REVISION;
	Gx50->clock.mode_size = sizeof(Gx50_clock_mode_t);
	Gx50->clock.type      = KGIM_CT_PROG;

	/*	where does this dclk.min come from? specs hint at 50Mhz, 
	**	but thats awfully high.
	*/
	Gx50->clock.dclk.range[0].min = 5648969;

	Gx50->clock.dclk.range[0].max = /* 300 MHZ ? */
	Gx50->pll.fvco.min =
	  KGIM_DEFAULT(options->clock->fvco_min, 256 MHZ);

	Gx50->pll.fref = KGIM_DEFAULT(options->clock->fref, 27 MHZ);

	Gx50->pll.mul.min = 9;
	Gx50->pll.mul.max = 125;
	Gx50->pll.div.max = 10;
	Gx50->pll.div.min = 1;

	Gx50->pll.p.min = 0;
	Gx50->pll.p.max = 4;

	Gx50->pll.a.mul = 2;
	Gx50->pll.a.div = 1;

	vendor_id = pcicfg_in16(Gx50_PCIDEV(Gx50_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(Gx50_PCIDEV(Gx50_io) + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(vendor_id,device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		if (pcicfg_in8(Gx50_PCIDEV(Gx50_io) + PCI_REVISION_ID) >= 128)
		  {
		    KRN_NOTICE("G450 clock parameters");
		    Gx50->flags |= Gx50_CF_G450;

		    Gx50->pll.fvco.max = 
		      KGIM_DEFAULT(options->clock->fvco_max, 600 MHZ);

		  }
		else
		  {
		    KRN_INTERNAL_ERROR;
		    return -KGI_ERRNO(CLOCK, UNKNOWN);
		  }

		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G550):

		KRN_NOTICE("G550 clock parameters");

		Gx50->flags |= Gx50_CF_G550; /* TODO: Update */

		Gx50->pll.fvco.max = 
		  KGIM_DEFAULT(options->clock->fvco_max, 1200 MHZ);

		break;
	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		Gx50->clock.vendor, Gx50->clock.model);

	KRN_DEBUG(2, "Gx50 pll parameter %i, %i-%i; options %i, %i-%i", 
		Gx50->pll.fref, Gx50->pll.fvco.min, Gx50->pll.fvco.max, 
		options->clock->fref, options->clock->fvco_min, 
		options->clock->fvco_max);

	return KGI_EOK;
}

const kgim_meta_t Gx50_clock_meta =
{
	(kgim_meta_init_module_fn *)	Gx50_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		Gx50_clock_init,
	(kgim_meta_done_fn *)		Gx50_clock_done,
	(kgim_meta_mode_check_fn *)	Gx50_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	Gx50_clock_mode_prepare,
	(kgim_meta_mode_enter_fn *)	Gx50_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(Gx50_clock_t),
	0,
	sizeof(Gx50_clock_mode_t)
};
