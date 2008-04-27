/* ----------------------------------------------------------------------------
**	Fixed timings
** ----------------------------------------------------------------------------
**	Copyright (C)	2002		Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: fixed.c,v $
**	Revision 1.7  2003/09/15 02:36:26  fspacek
**	The goto was the right thing to do.
**	
**	Revision 1.6  2003/07/26 18:49:49  cegger
**	use return rather than goto :) - fix from FreeBSD folk
**	
**	Revision 1.5  2003/03/15 20:40:03  redmondp
**	-define MAINTAINER and KGIM_MONITOR_DRIVER for all c files
**	
**	Revision 1.4  2003/03/12 06:18:46  fspacek
**	- reverse fixed search order to maximize refresh rate
**	
**	Revision 1.3  2003/02/02 22:49:34  cegger
**	fix warnings about implicit function calls
**	
**	Revision 1.2  2003/01/18 20:50:31  fspacek
**	- correct debug output
**	
**	Revision 1.1  2002/12/09 18:20:28  fspacek
**	- initial version of the unified monitor driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Filip_Spacek
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.7 $"


#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	3
#endif

#include <kgi/module.h>

#include "monitor/crt/crt-meta.h"


#define KRN_DEBUG_TIMING(t, name) \
	KRN_DEBUG(4, "%s.width = %d", name, t.width); \
	KRN_DEBUG(4, "%s.blankstart = %d", name, t.blankstart); \
	KRN_DEBUG(4, "%s.syncstart = %d", name, t.syncstart); \
	KRN_DEBUG(4, "%s.syncend = %d", name, t.syncend); \
	KRN_DEBUG(4, "%s.blankend = %d", name, t.blankend); \
	KRN_DEBUG(4, "%s.total = %d", name, t.total); \
	KRN_DEBUG(4, "%s.polarity = %d", name, t.polarity);

extern crt_monitor_timing_set_t crt_monitor_VGA_timings;
extern crt_monitor_timing_set_t crt_monitor_VESA_timings;

static void insert_timings(crt_monitor_t *crt, crt_monitor_timing_set_t *set)
{
	kgi_u_t i, j;
	const crt_monitor_timing_t **fixed = crt->fixed;
	kgi_u_t vfreq, hfreq;

	/* If there are no limits, we don't implicitly insert any timings */
	if (crt->num_hfreq == 0 || crt->num_vfreq == 0) {

		KRN_DEBUG(4, "No default timings inserted (no limits)");	
		return;
	}

	for (i = 0; i < set->num; ++i) {

		for (j = 0; j < crt->num_fixed; ++j) {

			if (&set->t[i] == fixed[j]) {

				return;
			}
		}

		hfreq = 1e9/set->t[i].x.total;

		for (j = 0; j < crt->num_hfreq; ++j) {

			if (crt->hfreq[j].min <= hfreq &&
			    crt->hfreq[j].max >= hfreq) {

				break;
			}
		}
		if (j == crt->num_hfreq) {

			KRN_DEBUG(4, "not added default timing "
				"hfreq out of range (%d kHz)", hfreq);
			continue;
		}

		vfreq = 1e9/set->t[i].x.total/set->t[i].y.total;

		for (j = 0; j < crt->num_vfreq; ++j) {

			if (crt->vfreq[j].min <= vfreq &&
			    crt->vfreq[j].max >= vfreq) {

				break;
			}
		}
		if (j == crt->num_vfreq) {
			
			KRN_DEBUG(4, "not added default timing "
				"vfreq out of range (%d Hz)", vfreq);
			continue;
		}

		crt->fixed[crt->num_fixed] = &set->t[i];
		crt->num_fixed++;
	
		/* do_not_insert: */
		;
	}
}

void crt_monitor_fixed_configuration(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "inserting default timings");
	insert_timings(crt, &crt_monitor_VGA_timings);
	insert_timings(crt, &crt_monitor_VESA_timings);
}
	
kgi_error_t crt_monitor_fixed_propose(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode)
{
	const crt_monitor_timing_t **fixed = crt->fixed;
	kgi_u_t dotsx = crt_mode->kgim.in.dots.x;
	kgi_u_t dotsy = crt_mode->kgim.in.dots.y;
	kgi_s_t t = crt->num_fixed - 1;

	while ((t >= 0) && (fixed[t]->y.width != dotsy)) {

		t--;
	}
	if (t < 0) {

		KRN_DEBUG(2, "No hard-coded vtiming for %i lines.", dotsy);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	crt_mode->kgim.in.dclk =
		((dotsx * 100000) / fixed[t]->x.width) * 10000;

	crt_mode->kgim.y = fixed[t]->y;

#	define POS(X) ((dotsx * fixed[t]->x.X) / fixed[t]->x.width)
	crt_mode->kgim.x.width		= POS(width);
	crt_mode->kgim.x.blankstart	= POS(blankstart);
	crt_mode->kgim.x.syncstart	= POS(syncstart);
	crt_mode->kgim.x.syncend	= POS(syncend);
	crt_mode->kgim.x.blankend	= POS(blankend);
	crt_mode->kgim.x.total		= POS(total);
	crt_mode->kgim.x.polarity	= fixed[t]->x.polarity;
#	undef POS

	KRN_DEBUG_TIMING(crt_mode->kgim.x, "propose x");
	KRN_DEBUG_TIMING(crt_mode->kgim.y, "propose y");
	
	return KGI_EOK;
}

kgi_error_t crt_monitor_fixed_adjust(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode)
{
	const crt_monitor_timing_t **fixed = crt->fixed;
	kgi_u_t dotsx = crt_mode->kgim.in.dots.x;
	kgi_u_t dotsy = crt_mode->kgim.in.dots.y;
	kgi_u_t dclk, ddclk;
	kgi_s_t t;

	for (t = crt->num_fixed - 1; t >= 0; --t) {

		dclk = ((dotsx * 100000) / fixed[t]->x.width) * 10000;
		ddclk = (crt_mode->kgim.in.dclk > dclk)
			? (crt_mode->kgim.in.dclk - dclk)
			: (dclk - crt_mode->kgim.in.dclk);
		
		if (fixed[t]->y.width == dotsy &&
		    (100 * ddclk) < (2 * dclk)) {

			break;
		}
	}

	if (t < 0) {

		KRN_DEBUG(2, "No hard-coded vtiming for %i lines at %i Hz.",
			  dotsy, crt_mode->kgim.in.dclk);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	crt_mode->kgim.in.dclk =
		((dotsx * 100000) / fixed[t]->x.width) * 10000;
	
	crt_mode->kgim.y = fixed[t]->y;

#	define POS(X) ((dotsx * fixed[t]->x.X) / fixed[t]->x.width)
	crt_mode->kgim.x.width		= POS(width);
	crt_mode->kgim.x.blankstart	= POS(blankstart);
	crt_mode->kgim.x.syncstart	= POS(syncstart);
	crt_mode->kgim.x.syncend	= POS(syncend);
	crt_mode->kgim.x.blankend	= POS(blankend);
	crt_mode->kgim.x.total		= POS(total);
	crt_mode->kgim.x.polarity	= fixed[t]->x.polarity;
#	undef POS

	KRN_DEBUG_TIMING(crt_mode->kgim.x, "adjust x");
	KRN_DEBUG_TIMING(crt_mode->kgim.y, "adjust y");
	
	return KGI_EOK;
}
