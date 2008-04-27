/* ----------------------------------------------------------------------------
**	ATI MACH64 PLL binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64-bind.c,v $
**	Revision 1.4  2003/07/26 18:46:11  cegger
**	merge improvements from the FreeBSD guys:
**	- improves OS independency
**	- add support for new MACH64 chip variants (GX and LM)
**	- bug fixes
**	
**	Revision 1.3  2002/06/12 14:21:09  fspacek
**	PCI ids for Rage Pro, beginings of a cursor and lots of register definitions
**	
**	Revision 1.2  2002/06/01 23:02:07  fspacek
**	Some reformating and PCI ids for Rage XL/XC
**	
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:35  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:17:14  phil
**	Autmatically generated ATI clock code
**	
**	Revision 1.2  2000/04/26 14:01:35  seeger_s
**	- more resonable PLL limits
**	- minor enhancements of the templates
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Filip_Spacek
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.4 $"

#include <kgi/module.h>
#include "clock/ATI/MACH64-bind.h"
#include "chipset/ATI/MACH64.h"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

kgi_error_t mach64_clock_init_module(mach64_clock_t *mach64,
				     mach64_clock_io_t *mach64_io,
				     const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;
  
	vendor_id = pcicfg_in16(MACH64_PCIDEV(mach64_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(MACH64_PCIDEV(mach64_io) + PCI_DEVICE_ID);
	KRN_DEBUG(1, "vendor %d, device %d", vendor_id, device_id);

	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {
    
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64CT):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GX):
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT):
		mach64->flags = MACH64_DSP;
		break;
    
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT2C):

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GB):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GD):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GP):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GQ):

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GR):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GS):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GO):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GL):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GM):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GN):

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64LM):
		mach64->flags =  MACH64_ODD_POST_DIVIDERS | MACH64_DSP;
		break;

	default:
		KRN_NOTICE("No ATI MACH64 Integrated Clock detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}
  
	kgim_strcpy(mach64->clock.vendor, "ATI");
	kgim_strcpy(mach64->clock.model, "mach64");

	mach64->clock.revision = KGIM_CLOCK_REVISION;
	mach64->clock.mode_size = sizeof(mach64_clock_mode_t);
  
	mach64->clock.type = KGIM_CT_PROG;
  
	mach64->clock.dclk.range[0].min =   5 MHZ;
	mach64->clock.dclk.range[0].max = 135 MHZ;
  
	mach64->fref = KGIM_DEFAULT(options->clock->fref, 14318180 /*Hz*/);
	
	mach64->fvco.min = KGIM_DEFAULT(options->clock->fvco_min,  50 MHZ);
	mach64->fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 135 MHZ);
  
	/* dividend range */
	mach64->feedback.min = 0x80;
	mach64->feedback.max = 0xff;

	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		   mach64->clock.vendor, mach64->clock.model);
  
	KRN_DEBUG(2, "pll parameter %i, %i-%i; options %i, %i-%i", 
		  mach64->fref, mach64->fvco.min, mach64->fvco.max,
		  options->clock->fref, options->clock->fvco_min,
		  options->clock->fvco_max);
  
	return KGI_EOK;
}

const kgim_meta_t mach64_clock_meta =
{
	(kgim_meta_init_module_fn *)    mach64_clock_init_module,
	(kgim_meta_done_module_fn *)    NULL,
	(kgim_meta_init_fn *)           mach64_clock_init,
	(kgim_meta_done_fn *)           mach64_clock_done,
	(kgim_meta_mode_check_fn *)     mach64_clock_mode_check,
	(kgim_meta_mode_resource_fn *)  NULL,
	(kgim_meta_mode_prepare_fn *)   NULL,
	(kgim_meta_mode_enter_fn *)     mach64_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)     NULL,
	(kgim_meta_image_resource_fn *) NULL,
  
	sizeof(mach64_clock_t),
	0,
	sizeof(mach64_clock_mode_t)
};
