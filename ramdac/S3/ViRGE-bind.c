/* ----------------------------------------------------------------------------
**	S3 ViRGE ramdac meta binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**			2000		Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ViRGE-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/08/04 11:57:50  seeger_s
**	- merged code posted by Jos Hulzink to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- driver status 30% is reported by Jos Hulzink
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	255
#endif

#include <kgi/module.h>
#define	__S3_ViRGE
#include "chipset/S3/ViRGE.h"
#include "ramdac/S3/ViRGE.h"
#include "ramdac/S3/ViRGE-meta.h"

kgi_error_t virge_ramdac_init_module(virge_ramdac_t *virge, virge_ramdac_io_t *virge_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_DEBUG(2, "virge_ramdac_init_module()");
	
	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);
	KRN_ASSERT(options);

	vendor_id = pcicfg_in16(VIRGE_PCIDEV(virge_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(VIRGE_PCIDEV(virge_io) + PCI_DEVICE_ID);

	virge->ramdac.revision = KGIM_RAMDAC_REVISION;
	
	kgim_strcpy(virge->ramdac.vendor, "S3");
	kgim_strcpy(virge->ramdac.model, "ViRGE DAC");
	
/*	virge->ramdac.flags	*/
	virge->ramdac.mode_size	= sizeof(virge_ramdac_mode_t);
/*	virge->ramdac.Command	*/

	virge->ramdac.maxdots.x	= 0x07FF;
	virge->ramdac.maxdots.y	= 0x07FF;
	virge->ramdac.lclk.min = options->ramdac->lclk_min ? options->ramdac->lclk_min : 0;
	virge->ramdac.lclk.max = options->ramdac->lclk_max ? options->ramdac->lclk_max : 135000000;
	virge->ramdac.dclk.min = options->ramdac->dclk_min ? options->ramdac->dclk_min : 0;
	virge->ramdac.dclk.max = options->ramdac->dclk_max ? options->ramdac->dclk_max : 135000000;

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER, virge->ramdac.vendor, virge->ramdac.model);

	return KGI_EOK;
}

static const kgim_meta_t virge_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	virge_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		virge_ramdac_init,
	(kgim_meta_done_fn *)		virge_ramdac_done,
	(kgim_meta_mode_check_fn *)	virge_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	virge_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(virge_ramdac_t),
	0,
	sizeof(virge_ramdac_mode_t)

};
