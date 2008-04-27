/* ----------------------------------------------------------------------------
**	VGA integrated DAC meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-bind.c,v $
**	Revision 1.2  2003/01/25 02:27:57  aldot
**	- sync up to current API, (first part -- 20021001)
**	
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.1.1.1  2000/04/18 08:51:07  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.2 $"

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include <kgi/module.h>

#include "ramdac/IBM/VGA.h"
#include "ramdac/IBM/VGA-bind.h"

kgi_error_t vga_ramdac_init_module(vga_ramdac_t *vga, vga_ramdac_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "vga_ramdac_init_module()");

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	kgim_memset(vga, 0, sizeof(*vga));

	kgim_strcpy(vga->ramdac.vendor, "IBM");
	kgim_strcpy(vga->ramdac.model, "VGA RAMDAC");

	vga->ramdac.revision	= KGIM_RAMDAC_REVISION;
/*	vga->ramdac.flags	*/
	vga->ramdac.mode_size	= sizeof(vga_ramdac_mode_t);
/*	vga->ramdac.Command	*/

	vga->ramdac.maxdots.x	= 0x03FF;
	vga->ramdac.maxdots.y	= 0x03FF;
#warning RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRANGE checks
	vga->ramdac.lclk.min = options->ramdac->lclk_min
		? options->ramdac->lclk_min : 25000000;
	vga->ramdac.lclk.max = options->ramdac->lclk_max > 0
		? options->ramdac->lclk_max : 32000000;
	vga->ramdac.dclk.min = options->ramdac->dclk_min
		? options->ramdac->dclk_min : 25000000;
	vga->ramdac.dclk.max = options->ramdac->dclk_max > 0
		? options->ramdac->dclk_max : 32000000;


	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		vga->ramdac.vendor, vga->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t vga_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	vga_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		vga_ramdac_init,
	(kgim_meta_done_fn *)		vga_ramdac_done,
	(kgim_meta_mode_check_fn *)	vga_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	vga_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	vga_ramdac_image_resource,

	sizeof(vga_ramdac_t),
	0,
	sizeof(vga_ramdac_mode_t)
};
