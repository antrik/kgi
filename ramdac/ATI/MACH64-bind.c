/* -----------------------------------------------------------------------------
**	ATI MACH64 ramdac binding
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: MACH64-bind.c,v $
**	Revision 1.5  2003/07/26 18:47:22  cegger
**	merge improvements from the FreeBSD guys:
**	- improves OS independency
**	- add support for new MACH64 chip variants (GX and LM)
**	- bug fixes
**	
**	Revision 1.4  2002/06/12 14:21:09  fspacek
**	PCI ids for Rage Pro, beginings of a cursor and lots of register definitions
**	
**	Revision 1.3  2002/06/07 01:22:18  fspacek
**	Export a clut image resource since at least the console layer can use it
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
**	Revision 1.1  2001/08/17 01:17:59  phil
**	Template generated code for ATI ramdac
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Filip_Spacek
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.5 $"

#include <kgi/module.h>

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/MACH64.h"
#include "ramdac/ATI/MACH64-bind.h"

kgi_error_t mach64_ramdac_init_module(mach64_ramdac_t *mach64,
	mach64_ramdac_io_t *mach64_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_ASSERT(mach64);
	KRN_ASSERT(mach64_io);
	KRN_ASSERT(options);

	vendor_id = pcicfg_in16(MACH64_PCIDEV(mach64_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(MACH64_PCIDEV(mach64_io) + PCI_DEVICE_ID);
	KRN_DEBUG(1, "vendor %d, device %d", vendor_id, device_id);

	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GX):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64CT):
	
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
		break;

	default:
		KRN_NOTICE("No ATI MACH64 Integrated RAMDAC detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}

	mach64->ramdac.revision	= KGIM_RAMDAC_REVISION;
	kgim_strcpy(mach64->ramdac.vendor, "ATI");
	kgim_strcpy(mach64->ramdac.model, "MACH64 Integrated RAMDAC");
/*	mach64->ramdac.flags	*/
	mach64->ramdac.mode_size = sizeof(mach64_ramdac_mode_t);
/*	mach64->ramdac.Command	*/

	mach64->ramdac.maxdots.x = 0x07FF;
	mach64->ramdac.maxdots.y = 0x07FF;
	mach64->ramdac.dclk.min = KGIM_DEFAULT(options->ramdac->dclk_min, 0);
	mach64->ramdac.dclk.max =
	  KGIM_DEFAULT(options->ramdac->dclk_max, 135000000);

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		   mach64->ramdac.vendor, mach64->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t mach64_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	mach64_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		mach64_ramdac_init,
	(kgim_meta_done_fn *)		mach64_ramdac_done,
	(kgim_meta_mode_check_fn *)	mach64_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	mach64_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *) mach64_ramdac_image_resource,

	sizeof(mach64_ramdac_t),
	0,
	sizeof(mach64_ramdac_mode_t)
};

