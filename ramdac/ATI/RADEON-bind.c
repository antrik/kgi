/* -----------------------------------------------------------------------------
**	ATI RADEON ramdac binding
** -----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: RADEON-bind.c,v $
**	Revision 1.2  2002/09/26 02:32:04  redmondp
**	Added palette support to ramdac
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#include <kgi/maintainers.h>
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/RADEON.h"
#include "ramdac/ATI/RADEON.h"
#include "ramdac/ATI/RADEON-bind.h"

kgi_error_t radeon_ramdac_init_module(radeon_ramdac_t *radeon,
	radeon_ramdac_io_t *radeon_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;
	
	KRN_ASSERT(radeon);
	KRN_ASSERT(radeon_io);
	KRN_ASSERT(options);

	vendor_id = pcicfg_in16(RADEON_PCIDEV(radeon_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(RADEON_PCIDEV(radeon_io) + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_R100a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_R100b):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_R100c):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_R100d):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_RV100a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_RV100b):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_M6a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_M6b):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_RV200a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_RV200b):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_M7a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_M7b):
		break;
	default:
		KRN_NOTICE("no ATI Radeon DAC detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}

	kgim_strcpy(radeon->ramdac.vendor, "ATI");
	kgim_strcpy(radeon->ramdac.model, "Radeon DAC");
	radeon->ramdac.mode_size = sizeof(radeon_ramdac_mode_t);

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		radeon->ramdac.vendor, radeon->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t radeon_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	radeon_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		radeon_ramdac_init,
	(kgim_meta_done_fn *)		radeon_ramdac_done,
	(kgim_meta_mode_check_fn *)	radeon_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	radeon_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	radeon_ramdac_image_resource,

	sizeof(radeon_ramdac_t),
	0,
	sizeof(radeon_ramdac_mode_t)
};
