/* -----------------------------------------------------------------------------
**	ATI Rage 128 ramdac binding
** -----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: R128-bind.c,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Paul_Redmond
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1 $"

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include <kgi/module.h>

#include "chipset/ATI/R128.h"
#include "ramdac/ATI/R128-bind.h"

kgi_error_t r128_ramdac_init_module(r128_ramdac_t *r128,
	r128_ramdac_io_t *r128_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_ASSERT(r128);
	KRN_ASSERT(r128_io);
	KRN_ASSERT(options);

	vendor_id = pcicfg_in16(R128_PCIDEV(r128_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(R128_PCIDEV(r128_io) + PCI_DEVICE_ID);

	KRN_DEBUG(1, "setting up ramdac");
	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RAGE128RE):
		break;

	default:
		KRN_NOTICE("no ATI Rage 128 DAC detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}

	r128->ramdac.revision = KGIM_RAMDAC_REVISION;
	kgim_strcpy(r128->ramdac.vendor, "ATI");
	kgim_strcpy(r128->ramdac.model, "Rage 128 DAC");
	r128->ramdac.mode_size = sizeof(r128_ramdac_mode_t);

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		r128->ramdac.vendor, r128->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t r128_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	r128_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		r128_ramdac_init,
	(kgim_meta_done_fn *)		r128_ramdac_done,
	(kgim_meta_mode_check_fn *)	r128_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	r128_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	r128_ramdac_image_resource,

	sizeof(r128_ramdac_t),
	0,
	sizeof(r128_ramdac_mode_t)
};
