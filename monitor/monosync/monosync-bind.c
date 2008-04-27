/* ----------------------------------------------------------------------------
**	monosync monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: monosync-bind.c,v $
**	Revision 1.2  2002/06/25 02:47:44  aldot
**	- temporary workaround for monitor driver
**	    build in currently configured monitor; merely ment for ease of Users.
**	    FIXME: allow -out module to be replaced
**	
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:59:00  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 2
#endif

#include <kgi/module.h>
#include "monitor/monosync/monosync-meta.h"

extern const monosync_timing_t monosync_monitor_timing;
static kgi_error_t monosync_monitor_init_module(monosync_monitor_t *monosync,
						monosync_monitor_io_t *monosync_io,
						const kgim_options_t *options)
{
	KRN_DEBUG(2, "monosync_monitor_init_module()");

	monosync->timing = &monosync_monitor_timing;

	return (options->monitor->init_module(&monosync->monitor, options->monitor));
}

const kgim_meta_t monosync_monitor_meta =
{
	(kgim_meta_init_module_fn *)	monosync_monitor_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		monosync_monitor_init,
	(kgim_meta_done_fn *)		NULL,
	(kgim_meta_mode_check_fn *)	monosync_monitor_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	NULL,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(monosync_monitor_t),
	0,
	sizeof(monosync_monitor_mode_t)
};
