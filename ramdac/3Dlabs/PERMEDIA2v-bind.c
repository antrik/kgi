/* -----------------------------------------------------------------------------
**	PERMEDIA2v integrated DAC meta language binding
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2v-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:56:15  seeger_s
**	- implemented CLUT control and hardware cursor support
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:04  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	0	/* do silent checking */
#endif

#include <kgi/module.h>
#define	__3Dlabs_Permedia
#define	__3Dlabs_Permedia2
#define	__3Dlabs_Permedia2v
#include "chipset/3Dlabs/PERMEDIA.h"
#include "ramdac/3Dlabs/PERMEDIA2v.h"
#include "ramdac/3Dlabs/PERMEDIA2v-meta.h"

kgi_error_t pgc2v_ramdac_init_module(pgc2v_ramdac_t *pgc2v,
	pgc2v_ramdac_io_t *pgc2v_io, const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;

	KRN_ASSERT(pgc2v);
	KRN_ASSERT(pgc2v_io);
	KRN_ASSERT(options);

	vendor_id = pcicfg_in16(PGC2v_PCIDEV(pgc2v_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(PGC2v_PCIDEV(pgc2v_io) + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA2v):
		kgim_strcpy(pgc2v->ramdac.vendor, "3Dlabs Incorporated");
		kgim_strcpy(pgc2v->ramdac.model, "PERMEDIA2 DAC");
		break;

	default:
		KRN_NOTICE("no 3Dlabs PERMEDIA2 or TI TVP4020 DAC detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}

	pgc2v->ramdac.revision	= KGIM_RAMDAC_REVISION;
/*	pgc2v->ramdac.flags	*/
	pgc2v->ramdac.mode_size	= sizeof(pgc2v_ramdac_mode_t);
/*	pgc2v->ramdac.Command	*/

	pgc2v->ramdac.maxdots.x	= 0x07FF;
	pgc2v->ramdac.maxdots.y	= 0x07FF;
	pgc2v->ramdac.lclk.min = 
			KGIM_DEFAULT(options->ramdac->lclk_min, 0);
	pgc2v->ramdac.lclk.max =
			KGIM_DEFAULT(options->ramdac->lclk_max, 135000000);
	pgc2v->ramdac.dclk.min = 
			KGIM_DEFAULT(options->ramdac->dclk_min, 0);
	pgc2v->ramdac.dclk.max =
			KGIM_DEFAULT( options->ramdac->dclk_max, 220000000);

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		pgc2v->ramdac.vendor, pgc2v->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t pgc2v_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	pgc2v_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		pgc2v_ramdac_init,
	(kgim_meta_done_fn *)		pgc2v_ramdac_done,
	(kgim_meta_mode_check_fn *)	pgc2v_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	pgc2v_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	pgc2v_ramdac_image_resource,

	sizeof(pgc2v_ramdac_t),
	0,
	sizeof(pgc2v_ramdac_mode_t)
};
