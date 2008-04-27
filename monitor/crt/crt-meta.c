/* ----------------------------------------------------------------------------
**	generic crt monitor meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2002		Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: crt-meta.c,v $
**	Revision 1.5  2003/02/06 09:49:44  foske
**	No comment... sigh...
**	
**	Revision 1.4  2003/02/06 09:37:09  foske
**	Dropped VESA standard for faster DDC1 detection. New module option added
**	monitor_ddc: 0 = no ddc, 1 = ddc2 + fast ddc1, 2 = ddc2 + slow ddc1
**	
**	Revision 1.3  2003/02/02 22:49:34  cegger
**	fix warnings about implicit function calls
**	
**	Revision 1.2  2003/01/09 12:41:36  foske
**	Debugging didn't work.
**	
**	Revision 1.1  2002/12/09 18:20:28  fspacek
**	- initial version of the unified monitor driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Filip_Spacek
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.5 $"


#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>
#include "monitor/crt/crt-meta.h"
#include "monitor/crt/DDC.h"

kgi_error_t crt_monitor_init(crt_monitor_t *crt, 
	crt_monitor_io_t *crt_io, const kgim_options_t *options)
{
	kgi_error_t err;
	
	if (options->monitor->ddc > 0) {
		err = -KGI_ERRNO(MONITOR, UNKNOWN);
		if (0)
			err = DDC2_Read_Configuration(crt, crt_io, options);
		
		if ((crt_io->DDCInit) && (err != -KGI_EOK)) 
			err = DDC1_Read_Configuration(crt, crt_io, options);
	}
	crt_monitor_fixed_configuration(crt, crt_io, options);

	if (!crt->num_fixed && (!crt->num_vfreq || !crt->num_hfreq)) {
	
		KRN_ERROR("No useable monitor configuration found");
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
	
	return -KGI_EOK;
}

kgi_error_t crt_monitor_mode_check(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	switch (cmd) {

	case KGI_TC_PROPOSE:

		if ((crt_mode->kgim.in.dots.x > crt->kgim.maxdots.x) || 
		    (crt_mode->kgim.in.dots.y > crt->kgim.maxdots.y)) {
			
			KRN_ERROR("Resolution %ix%i higher than limit %ix%i",
				  crt_mode->kgim.in.dots.x, crt_mode->kgim.in.dots.y,
				  crt->kgim.maxdots.x, crt->kgim.maxdots.y);
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		if (crt_monitor_fixed_propose(crt,
			crt_io, crt_mode) == KGI_EOK) {

			KRN_DEBUG(2, "Found exact timing");
		}
		else if (crt_monitor_gtf_propose(crt,
			crt_io, crt_mode) != KGI_EOK) {
		
			KRN_DEBUG(2, "Unable to calculate timings");
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}
		
		crt_mode->kgim.sync = crt->kgim.sync;
		  
		if (crt->kgim.flags & KGIM_MF_PROPSIZE &&
			crt->kgim.maxdots.x != 0 && crt->kgim.maxdots.y != 0){

			crt_mode->kgim.size.x = 
				crt_mode->kgim.in.dots.x *
				crt->kgim.size.x / crt->kgim.maxdots.x;
			crt_mode->kgim.size.y = 
				crt_mode->kgim.in.dots.y *
				crt->kgim.size.y / crt->kgim.maxdots.y;
		}
		else {

			crt_mode->kgim.size = crt->kgim.size;
		}

		KRN_DEBUG(2,
			"propose dclk = %i Hz, hfreq = %i Hz, vfreq = %i Hz",
			crt_mode->kgim.in.dclk,
			crt_mode->kgim.in.dclk /
				crt_mode->kgim.x.total,
			crt_mode->kgim.in.dclk /
				(crt_mode->kgim.x.total * 
					crt_mode->kgim.y.total));

		return KGI_TC_LOWER;

	case KGI_TC_LOWER:

		if (crt_monitor_fixed_adjust(crt,
			crt_io, crt_mode) == KGI_EOK) {

			KRN_DEBUG(2, "KGI_TC_LOWER: Fixed timing matches");
		}
		else if (crt_monitor_gtf_adjust(crt,
			crt_io, crt_mode) != KGI_EOK) {
			
			KRN_DEBUG(2, "Unable to calculate timings");
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}
		
		return KGI_TC_CHECK;

	case KGI_TC_RAISE:
		
		if (crt_monitor_fixed_adjust(crt,
			crt_io, crt_mode) == KGI_EOK) {

			KRN_DEBUG(2, "KGI_TC_LOWER: Fixed timing matches");
		}
		else if (crt_monitor_gtf_adjust(crt,
			crt_io, crt_mode) != KGI_EOK) {
			
			KRN_DEBUG(2, "Unable to calculate timings");
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}

		return KGI_TC_CHECK;

	case KGI_TC_CHECK:

		KRN_DEBUG(2, "%i\t%i %i %i %i %i %i\t%i %i %i %i %i %i",
			crt_mode->kgim.in.dclk,
			crt_mode->kgim.x.width,
			crt_mode->kgim.x.blankstart,
			crt_mode->kgim.x.syncstart,
			crt_mode->kgim.x.syncend,
			crt_mode->kgim.x.blankend,
			crt_mode->kgim.x.total,
			crt_mode->kgim.y.width,
			crt_mode->kgim.y.blankstart,
			crt_mode->kgim.y.syncstart,
			crt_mode->kgim.y.syncend,
			crt_mode->kgim.y.blankend,
			crt_mode->kgim.y.total);

		return KGI_TC_READY;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
}
