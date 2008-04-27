/* -----------------------------------------------------------------------------
**	nVidia TNT2 ramdac meta binding
** -----------------------------------------------------------------------------
**	Copyright	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: TNT2-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.1.1.1  2000/04/18 08:51:05  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Jon_Taylor
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif


#define	__nVidia_TNT2
#include "chipset/nVidia/TNT2.h"
#include "ramdac/nVidia/TNT2.h"
#include "ramdac/nVidia/TNT2-meta.h"

kgi_error_t tnt2_ramdac_init_module(tnt2_ramdac_t *tnt2, 
	tnt2_ramdac_io_t *tnt2_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_DEBUG(2, "tnt2_ramdac_init_module()");
	
	KRN_ASSERT(tnt2);
	KRN_ASSERT(tnt2_io);
	KRN_ASSERT(options);

	vendor_id = pcicfg_in16(TNT2_PCIDEV(tnt2_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(TNT2_PCIDEV(tnt2_io) + PCI_DEVICE_ID);

	tnt2->ramdac.revision = KGIM_RAMDAC_REVISION;
	
	kgim_strcpy(tnt2->ramdac.vendor, "nVidia");
	kgim_strcpy(tnt2->ramdac.model, "TNT2 DAC");
	
/*	tnt2->ramdac.flags	*/
	tnt2->ramdac.mode_size	= sizeof(tnt2_ramdac_mode_t);
/*	tnt2->ramdac.Command	*/

	tnt2->ramdac.maxdots.x	= 0x07FF;
	tnt2->ramdac.maxdots.y	= 0x07FF;
	tnt2->ramdac.lclk.min = options->ramdac->lclk_min
		? options->ramdac->lclk_min : 0;
	tnt2->ramdac.lclk.max = options->ramdac->lclk_max 
		? options->ramdac->lclk_max : 135000000;
	tnt2->ramdac.dclk.min = options->ramdac->dclk_min 
		? options->ramdac->dclk_min : 0;
	tnt2->ramdac.dclk.max = options->ramdac->dclk_max 
		? options->ramdac->dclk_max : 220000000;

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER, 
		tnt2->ramdac.vendor, tnt2->ramdac.model);

	return KGI_EOK;
}

static const kgim_meta_t tnt2_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	tnt2_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		tnt2_ramdac_init,
	(kgim_meta_done_fn *)		tnt2_ramdac_done,
	(kgim_meta_mode_check_fn *)	tnt2_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	tnt2_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(tnt2_ramdac_t),
	0,
	sizeof(tnt2_ramdac_mode_t)

};
