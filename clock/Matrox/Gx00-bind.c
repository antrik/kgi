/* ----------------------------------------------------------------------------
**	Matrox Gx00 clock driver binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2002	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx00-bind.c,v 1.2 2002/11/30 18:23:14 ortalo Exp $
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#include "clock/pll-meta.h"
#include "clock/Matrox/Gx00-meta.h"
#include "chipset/Matrox/Gx00.h"

static kgi_error_t mgag_clock_init_module(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io,
	const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_DEBUG(2, "mgag_clock_init_module()");

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);
	KRN_ASSERT(options);

	kgim_memset(mgag, 0, sizeof(*mgag));

	kgim_strcpy(mgag->pll.clock.vendor, "Matrox Graphics Inc");
	kgim_strcpy(mgag->pll.clock.model,  "1x64/Gx00 Integrated Clock");

	mgag->pll.clock.revision  = KGIM_CLOCK_REVISION;
	mgag->pll.clock.mode_size = sizeof(mgag_clock_mode_t);
	mgag->pll.clock.type	  = KGIM_CT_PROG;

	/*	where does this dclk.min come from? specs hint at 50Mhz, 
	**	but thats awfully high.
	*/
	mgag->pll.clock.dclk.range[0].min = 5648969;

	mgag->pll.fvco.min = KGIM_DEFAULT(options->clock->fvco_min, 50 MHZ);
	mgag->pll.a.mul =
	mgag->pll.a.div = 1;
	mgag->pll.p.min = 0;
	mgag->pll.p.max = 3;
	mgag->pll.mul.max = 128;
	mgag->pll.div.min = 2;

	vendor_id = pcicfg_in16(MGAG_PCIDEV(mgag_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(MGAG_PCIDEV(mgag_io) + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(vendor_id,device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS):

		mgag->flags |= MGAG_CLOCK_1x64;

		mgag->pll.fref = KGIM_DEFAULT(options->clock->fref, 14318180);

		mgag->pll.mul.min = 101;
		mgag->pll.div.max = 32;

		switch (pcicfg_in8(MGAG_PCIDEV(mgag_io) + PCI_REVISION_ID)) {

		case 1:
			mgag->pll.clock.dclk.range[0].max = mgag->pll.fvco.max =
				KGIM_DEFAULT(options->clock->fvco_max, 135 MHZ);
			break;
		case 2:
			mgag->pll.clock.dclk.range[0].max = mgag->pll.fvco.max =
				KGIM_DEFAULT(options->clock->fvco_max, 175 MHZ);
			break;
		case 3:
			mgag->pll.clock.dclk.range[0].max = mgag->pll.fvco.max =
				KGIM_DEFAULT(options->clock->fvco_max, 220 MHZ);
			break;
		default:
			KRN_INTERNAL_ERROR;
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		break;
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_PCI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_AGP):

		mgag->flags |= MGAG_CLOCK_G200;

		mgag->pll.fref = KGIM_DEFAULT(options->clock->fref, 27 MHZ);

		mgag->pll.mul.min = 8;
		mgag->pll.div.max = 7;

		if (mgag->flags & MGAG_CLOCK_SGRAM) {

			mgag->pll.clock.dclk.range[0].max = 
			mgag->pll.fvco.max = 
			KGIM_DEFAULT(options->clock->fvco_max, 250 MHZ);

		} else {
			mgag->pll.clock.dclk.range[0].max = 
			mgag->pll.fvco.max = 
			KGIM_DEFAULT(options->clock->fvco_max, 230 MHZ);
		}

		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		mgag->flags |= MGAG_CLOCK_G400;

		if (pcicfg_in8(MGAG_PCIDEV(mgag_io) + PCI_REVISION_ID) >= 128)
		  {
		    KRN_NOTICE("G450 clock detected (id of G400 w/ rev.>=128)."
			       " Use the Gx50 clock subsystem.");
		    return -KGI_ERRNO(CLOCK, UNKNOWN);
		  }
		mgag->pll.fref = KGIM_DEFAULT(options->clock->fref, 27 MHZ);

		mgag->pll.mul.min = 8;
		mgag->pll.div.max = 32;

		mgag->pll.clock.dclk.range[0].max = mgag->pll.fvco.max = 
			KGIM_DEFAULT(options->clock->fvco_max, 300 MHZ);

		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G550):

		KRN_NOTICE("G550 clock detected. Use the Gx50 clock subsytem.");

		return -KGI_ERRNO(CLOCK, UNKNOWN);

		break;
	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		mgag->pll.clock.vendor, mgag->pll.clock.model);

	KRN_DEBUG(2, "pll parameter %i, %i-%i; options %i, %i-%i", 
		mgag->pll.fref, mgag->pll.fvco.min, mgag->pll.fvco.max, 
		options->clock->fref, options->clock->fvco_min, 
		options->clock->fvco_max);

	return KGI_EOK;
}

const kgim_meta_t mgag_clock_meta =
{
	(kgim_meta_init_module_fn *)	mgag_clock_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		mgag_clock_init,
	(kgim_meta_done_fn *)		mgag_clock_done,
	(kgim_meta_mode_check_fn *)	pll_clock_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	mgag_clock_mode_prepare,
	(kgim_meta_mode_enter_fn *)	mgag_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(mgag_clock_t),
	0,
	sizeof(mgag_clock_mode_t)
};
