/* ----------------------------------------------------------------------------
**	generic monosync monitor meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: monosync-meta.c,v $
**	Revision 1.4  2002/08/23 20:06:39  aldot
**	- emit ERRORs instead of DEBUGs on failure paths
**	
**	Revision 1.3  2002/07/05 01:17:11  aldot
**	- remove mistakenly checked in debug cruft
**	
**	Revision 1.2  2002/06/25 02:47:44  aldot
**	- temporary workaround for monitor driver
**	    build in currently configured monitor; merely ment for ease of Users.
**	    FIXME: allow -out module to be replaced
**	
**	Revision 1.1.1.1  2000/09/21 10:06:40  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Steffen_Seeger
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.4 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "monitor/monosync/monosync-meta.h"

kgi_error_t monosync_monitor_init(monosync_monitor_t *monosync, 
	monosync_monitor_io_t *monosync_io, const kgim_options_t *options)
{
	KRN_NOTICE("%s %s [monosync %s] driver " KGIM_MONITOR_DRIVER,
		monosync->monitor.vendor, monosync->monitor.model,
		monosync->timing->name);

	return KGI_EOK;
}

static kgi_s_t monosync_monitor_err(kgi_s_t pixels, kgi_s_t ns, kgi_u_t dclk)
{
	kgi_s_t width = (ns * (dclk / 10000)) / 100000;
	kgi_s_t delta = (100*(pixels - width)) / width;
	kgi_s_t ret = (delta < -2) || (2 < delta);

	if (ret) {

		KRN_DEBUG(0, "Timing difference %i > 2%% (%i pixel %i ns %i dclk)",
			delta, pixels, ns, dclk);
	}

	return ret;
}

static inline kgi_error_t monosync_monitor_fcheck(monosync_monitor_t *monosync, 
	kgi_u_t dclk, kgi_u_t xtotal, kgi_u_t ytotal)
{
	kgi_u_t flags = 0, i;
	kgi_u_t hfreq = dclk / xtotal;
	kgi_u_t vfreq = dclk / (xtotal * ytotal);

	i = KGIM_MONITOR_MAX_HFREQ;
	while (i--) {

		if ((monosync->monitor.hfreq[i].min <= hfreq) &&
			(hfreq <= monosync->monitor.hfreq[i].max)) {

			flags |= 1;
		}
	}

	i = KGIM_MONITOR_MAX_VFREQ;
	while (i--) {

		if ((monosync->monitor.vfreq[i].min <= vfreq) &&
			(vfreq <= monosync->monitor.vfreq[i].max)) {

			flags |= 2;
		}
	}
	return (flags == 3) ? KGI_EOK : -KGI_ERRNO(MONITOR,INVAL);
}

kgi_error_t monosync_monitor_mode_check(monosync_monitor_t *monosync,
	monosync_monitor_io_t *monosync_io,
	monosync_monitor_mode_t *monosync_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	const monosync_timing_t *t = monosync->timing;
	kgi_u_t v = 0, h = 0;
	kgi_u_t dclk, ddclk;
	kgi_u_t dotsx = monosync_mode->kgim.in.dots.x;
	kgi_u_t dotsy = monosync_mode->kgim.in.dots.y;

	if ((dotsx > monosync->monitor.maxdots.x) || 
		(dotsy > monosync->monitor.maxdots.y)) {

		KRN_ERROR("Resolution %ix%i higher than limit %ix%i",
			dotsx, dotsy, monosync->monitor.maxdots.x, 
			monosync->monitor.maxdots.y);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	while ((v < t->vtimings) && (t->vtiming[v].width != dotsy)) {

		v++;
	}
	if ((v == t->vtimings) || (t->vtiming[v].width != dotsy)) {

		KRN_ERROR("No vtiming for %i lines in timing set %s.",
			dotsy, t->name);
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	h = t->vtiming[v].polarity & ~MONOSYNC_POLARITY_MASK;
	dclk = ((dotsx * 100000) / t->htiming[h].width) * 10000;
	ddclk = (monosync_mode->kgim.in.dclk > dclk)
		? (monosync_mode->kgim.in.dclk - dclk)
		: (dclk - monosync_mode->kgim.in.dclk);

	switch (cmd) {

	case KGI_TC_PROPOSE:
		monosync_mode->kgim.in.dclk	= dclk;
		monosync_mode->kgim.y		= t->vtiming[v];
		monosync_mode->kgim.y.polarity	= 
			(t->vtiming[v].polarity & MONOSYNC_VPOS) ? 1 : 0;

#		define POS(X)  ((dotsx * t->htiming[h].X) / t->htiming[h].width)
		monosync_mode->kgim.x.width      = POS(width);
		monosync_mode->kgim.x.blankstart = POS(blankstart);
		monosync_mode->kgim.x.syncstart  = POS(syncstart);
		monosync_mode->kgim.x.syncend    = POS(syncend);
		monosync_mode->kgim.x.blankend   = POS(blankend);
		monosync_mode->kgim.x.total      = POS(total);
		monosync_mode->kgim.x.polarity   = 
			(t->vtiming[v].polarity & MONOSYNC_HPOS) ? 1 : 0;
#		undef POS

		monosync_mode->kgim.sync = monosync->monitor.sync;
		  
		if (monosync->monitor.flags & KGIM_MF_PROPSIZE) {

			monosync_mode->kgim.size.x = dotsx *
				monosync->monitor.size.x /
				monosync->monitor.maxdots.x;
			monosync_mode->kgim.size.y = dotsy *
				monosync->monitor.size.y /
				monosync->monitor.maxdots.y;
		} else {

			monosync_mode->kgim.size = monosync->monitor.size;
		}

		KRN_DEBUG(2,
			"propose dclk = %i Hz, hfreq = %i Hz, vfreq = %i Hz",
			monosync_mode->kgim.in.dclk,
			monosync_mode->kgim.in.dclk /
				monosync_mode->kgim.x.total,
			monosync_mode->kgim.in.dclk /
				(monosync_mode->kgim.x.total * 
					monosync_mode->kgim.y.total));

		return KGI_TC_LOWER;

	case KGI_TC_LOWER:
		if (monosync_mode->kgim.in.dclk < monosync->monitor.dclk.min) {

			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_ERROR("dclk is %i (should be %i) Hz",
				 monosync_mode->kgim.in.dclk, dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_RAISE:
		if (monosync_mode->kgim.in.dclk > monosync->monitor.dclk.max) {

			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((100 * ddclk)  >  (2 * dclk)) {

			KRN_ERROR("dclk is %i (should be %i) Hz",
				monosync_mode->kgim.in.dclk, dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_CHECK:
		if ((monosync_mode->kgim.in.dclk < monosync->monitor.dclk.min) || 
			(monosync_mode->kgim.in.dclk > monosync->monitor.dclk.max)) {

			KRN_ERROR("DCLK of %i Hz is out of bounds.",
				monosync_mode->kgim.in.dclk);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if ((monosync->monitor.maxdots.x < dotsx) ||
			(monosync->monitor.maxdots.y < dotsy)) {

			KRN_ERROR("resolution too high (%ix%i).",
				dotsx, dotsy);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if (((100 * ddclk) > (2 * dclk)) || 
			monosync_monitor_fcheck(monosync,
				monosync_mode->kgim.in.dclk,
				monosync_mode->kgim.x.total,
				monosync_mode->kgim.y.total)) {

			KRN_ERROR("frequency limits violated.");
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		KRN_DEBUG(2, "%i\t%i %i %i %i %i %i\t%i %i %i %i %i %i",
			monosync_mode->kgim.in.dclk,
			monosync_mode->kgim.x.width,
			monosync_mode->kgim.x.blankstart,
			monosync_mode->kgim.x.syncstart,
			monosync_mode->kgim.x.syncend,
			monosync_mode->kgim.x.blankend,
			monosync_mode->kgim.x.total,
			monosync_mode->kgim.y.width,
			monosync_mode->kgim.y.blankstart,
			monosync_mode->kgim.y.syncstart,
			monosync_mode->kgim.y.syncend,
			monosync_mode->kgim.y.blankend,
			monosync_mode->kgim.y.total);

#		define ERR(X) ((monosync_mode->kgim.y.X != t->vtiming[v].X) || \
			monosync_monitor_err(monosync_mode->kgim.x.X,	\
			t->htiming[h].X, monosync_mode->kgim.in.dclk))

			if (ERR(width) || ERR(blankstart) || ERR(syncstart) ||
				ERR(syncend) || ERR(blankend) || ERR(total)) {

				KRN_ERROR("Error in timings.");
				return -KGI_ERRNO(MONITOR, UNKNOWN);
			}

#		undef	ERR

		/*	now that everything is checked, initialize the
		**	driver dependent mode.
		*/
		monosync_mode->timing = t;

		return KGI_TC_READY;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
}
