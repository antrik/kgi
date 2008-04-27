/* ----------------------------------------------------------------------------
**	fixed clock meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: fixed-bind.c,v $
**	Revision 1.3  2002/10/14 23:55:02  aldot
**	- fixed clock usable again for other drivers
**	
**	Revision 1.2  2002/06/25 02:38:11  aldot
**	- update of fixed clock driver
**	
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Jon_Taylor
#define KGIM_CLOCK_DRIVER	"$Revision: 1.3 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#include "clock/fixed-meta.h"



/*
**	clock spec conversion macros - may be considered "advanced Voodoo"
*/
#define Data 1
#define Clock(vendor,model,meta)					\
									\
kgi_error_t fixed_clock_init_module_##meta(fixed_clock_t *fixed,	\
	fixed_clock_io_t *fixed_io,					\
	const kgim_options_t *options);					\
									\
kgim_meta_t fixed_clock_meta =						\
{									\
	(kgim_meta_init_module_fn *)	fixed_clock_init_module_##meta,	\
	(kgim_meta_done_module_fn *)	NULL,				\
	(kgim_meta_init_fn *)		fixed_clock_init,		\
	(kgim_meta_done_fn *)		fixed_clock_done,		\
	(kgim_meta_mode_check_fn *)	fixed_clock_mode_check,		\
	(kgim_meta_mode_resource_fn *)	NULL,				\
	(kgim_meta_mode_prepare_fn *)	NULL,				\
	(kgim_meta_mode_enter_fn *)	fixed_clock_mode_enter,		\
	(kgim_meta_mode_leave_fn *)	NULL,				\
	(kgim_meta_image_resource_fn *)	NULL,				\
									\
	sizeof(fixed_clock_t),						\
	0,								\
	sizeof(fixed_clock_mode_t)					\
};									\
									\
kgi_error_t fixed_clock_init_module_##meta(fixed_clock_t *fixed,	\
	fixed_clock_io_t *fixed_io,					\
	const kgim_options_t *options)					\
{									\
	kgi_u_t clocks = 0;						\
	KRN_DEBUG(2, "ENTER");						\
									\
	fixed->clock.type = KGIM_CT_FIXED;				\
	fixed->clock.mode_size = sizeof(fixed_clock_mode_t);

#define Begin								\
	fixed->clock.revision = KGIM_CLOCK_REVISION;
#define Contributor(c)
#define Vendor(v)							\
	kgim_strcpy(fixed->clock.vendor, #v);
#define Model(m)							\
	kgim_strcpy(fixed->clock.model, #m);

#define	Rclk(idx, f)	KRN_ASSERT(clocks < __KGIM_MAX_NR_FIXED_CLOCKS);\
			fixed->clock.dclk.freq[idx] = f;		\
			fixed->reg0[idx] = idx;				\
			clocks++;

#define End								\
	KRN_DEBUG(1, "%s %s initialized, %i clock values",		\
		fixed->clock.vendor, fixed->clock.model, clocks);	\
									\
	KRN_DEBUG(2, "EXIT");						\
	return KGI_EOK;							\
}

#ifndef	FIXED_CLOCK_SPEC
#	define	FIXED_CLOCK_SPEC	"fixed-bind.spec"
#endif

#include FIXED_CLOCK_SPEC
