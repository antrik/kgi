/* ----------------------------------------------------------------------------
**	crt monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2002		Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: crt-bind.c,v $
**	Revision 1.2  2003/03/15 20:40:03  redmondp
**	-define MAINTAINER and KGIM_MONITOR_DRIVER for all c files
**	
**	Revision 1.1  2002/12/09 18:20:27  fspacek
**	- initial version of the unified monitor driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Filip_Spacek

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 2
#endif

#include <kgi/module.h>

#include "monitor/crt/crt-meta.h"

static crt_monitor_timing_t user_timings[KGIM_OPTIONS_MAX_TIMINGS];

static kgi_error_t crt_monitor_init_module(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, const kgim_options_t *options)
{
	kgi_u_t i;
	kgi_error_t error;	

	crt->num_hfreq = 0;
	crt->num_vfreq = 0;
	crt->num_dclk = 0;
	crt->num_fixed = 0;

	for (i = 0; i < options->monitor->num_hfreq; ++i) {
	
		KRN_ASSERT(crt->num_hfreq < CRT_MONITOR_MAX_FREQ_RANGES);
		
		crt->hfreq[i].min = options->monitor->hfreq[i].min;
		crt->hfreq[i].max = options->monitor->hfreq[i].max;
		crt->num_hfreq++;
		
		KRN_DEBUG(2, "hfreq: %d-%d", 
			crt->hfreq[i].min, crt->hfreq[i].max);
	}
	
	for (i = 0; i < options->monitor->num_vfreq; ++i) {
	
		KRN_ASSERT(crt->num_vfreq < CRT_MONITOR_MAX_FREQ_RANGES);
		
		crt->vfreq[i].min = options->monitor->vfreq[i].min;
		crt->vfreq[i].max = options->monitor->vfreq[i].max;
		crt->num_vfreq++;

		KRN_DEBUG(2, "vfreq: %d-%d",
			crt->vfreq[i].min, crt->vfreq[i].max);
	}

	for (i = 0; i < options->monitor->num_dclk; ++i) {
	
		KRN_ASSERT(crt->num_dclk < CRT_MONITOR_MAX_FREQ_RANGES);
		
		crt->dclk[i].min = options->monitor->dclk[i].min;
		crt->dclk[i].max = options->monitor->dclk[i].max;
		crt->num_dclk++;

		KRN_DEBUG(2, "dclk: %d-%d", crt->dclk[i].min,crt->dclk[i].max);
	}

	for (i = 0; i < options->monitor->timings; ++i) {
	
		KRN_ASSERT(crt->num_fixed < CRT_MONITOR_MAX_FIXED_TIMINGS);
		
		user_timings[i].x = options->monitor->x[i];
		user_timings[i].y = options->monitor->y[i];
		
		crt->fixed[i] = &user_timings[i];
		crt->num_fixed++;
		
		KRN_DEBUG(2, "fixed timing. ");
	}

	/* Initialize the kgim monitor part with default values */
	if ((error = options->monitor->init_module(&crt->kgim, NULL)))
		return error;

	/* Override with optional values if any */
	if (options->monitor->sync) crt->kgim.sync = options->monitor->sync;
	if (options->monitor->type) crt->kgim.type = options->monitor->type;
	if (options->monitor->size.x && options->monitor->size.y)
		crt->kgim.size = options->monitor->size;
	
	return KGI_EOK;
}

const kgim_meta_t crt_monitor_meta =
{
	(kgim_meta_init_module_fn *)	crt_monitor_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		crt_monitor_init,
	(kgim_meta_done_fn *)		NULL,
	(kgim_meta_mode_check_fn *)	crt_monitor_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	NULL,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(crt_monitor_t),
	0,
	sizeof(crt_monitor_mode_t)
};
