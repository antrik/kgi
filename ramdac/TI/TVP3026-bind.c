/* -----------------------------------------------------------------------------
**	TI TVP3026 ramdac binding
** -----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: TVP3026-bind.c,v $
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:05  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_RAMDAC_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/TI/TVP3026.h"
#include "ramdac/TI/TVP3026-bind.h"

kgi_error_t tvp3026_ramdac_init_module(tvp3026_ramdac_t *tvp3026,
	tvp3026_ramdac_io_t *tvp3026_io, const kgim_options_t *options)
{
	kgi_u8_t e3F;

	KRN_ASSERT(tvp3026);
	KRN_ASSERT(tvp3026_io);
	KRN_ASSERT(options);

	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_ID, TVP3026_EDAC_ADDR);
	e3F = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);
	if (e3F != TVP3026_EDAC3F_TVP3026_ID) {

		KRN_ERROR("unknown chip ID %.2x.", e3F);
		return -KGI_ERRNO(RAMDAC, NODEV);
	}
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_Revision, TVP3026_EDAC_ADDR);
	tvp3026->rev = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);

	tvp3026->ramdac.revision = KGIM_RAMDAC_REVISION;
	kgim_strcpy(tvp3026->ramdac.vendor, "TI");
	kgim_strcpy(tvp3026->ramdac.model, (tvp3026->rev <= 0x20)
			? "TVP 3026 rev A" : "TVP 3026 rev B");
/*	tvp3026->ramdac.flags	*/
	tvp3026->ramdac.mode_size = sizeof(tvp3026_ramdac_mode_t);
/*	tvp3026->ramdac.Command	*/

	tvp3026->ramdac.maxdots.x = 0x0FFF;
	tvp3026->ramdac.maxdots.y = 0x0FFF;
	tvp3026->ramdac.lclk.min =
			KGIM_DEFAULT(options->ramdac->lclk_min, 0);
	tvp3026->ramdac.lclk.max =
			KGIM_DEFAULT(options->ramdac->lclk_max, 85000000);
	tvp3026->ramdac.dclk.min =
			KGIM_DEFAULT(options->ramdac->dclk_min, 0);
	tvp3026->ramdac.dclk.max =
			KGIM_DEFAULT(options->ramdac->dclk_max, 135000000);

	KRN_NOTICE("%s %s driver " KGIM_RAMDAC_DRIVER,
		tvp3026->ramdac.vendor, tvp3026->ramdac.model);

	return KGI_EOK;
}

const kgim_meta_t tvp3026_ramdac_meta =
{
	(kgim_meta_init_module_fn *)	tvp3026_ramdac_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		tvp3026_ramdac_init,
	(kgim_meta_done_fn *)		tvp3026_ramdac_done,
	(kgim_meta_mode_check_fn *)	tvp3026_ramdac_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	tvp3026_ramdac_mode_enter,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	tvp3026_ramdac_image_resource,

	sizeof(tvp3026_ramdac_t),
	0,
	sizeof(tvp3026_ramdac_mode_t)
};
