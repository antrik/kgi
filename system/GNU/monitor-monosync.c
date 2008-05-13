/* ----------------------------------------------------------------------------
**	monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: monitor-bind.c,v $
**	Revision 1.2  2002/06/25 02:47:44  aldot
**	- temporary workaround for monitor driver
**	    build in currently configured monitor; merely ment for ease of Users.
**	    FIXME: allow -out module to be replaced
**	
**	Revision 1.1.1.1  2000/04/18 08:51:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#include <kgi/module.h>

/*
**	monitor spec conversion macros - may be considered "advanced Voodoo"
*/

#define	Data	1
#define	Monitor(vendor,model)						\
kgi_error_t monitor_init_module_##vendor##_##model(kgim_monitor_t *monitor, \
	kgim_options_monitor_t *options)				\
{

#define	Begin		if (monitor) monitor->revision = KGIM_MONITOR_REVISION;
#define	Contributor(c)
#define	Vendor(v)	if (monitor) kgim_strncpy(monitor->vendor, v, sizeof(monitor->vendor));
#define	Model(m)	if (monitor) kgim_strncpy(monitor->model, m, sizeof(monitor->model));
#define	Flags(f)	if (monitor) monitor->flags = f;
#define	MaxRes(x,y)	if (monitor) monitor->maxdots = (kgi_ucoord_t) { x, y };
#define	Size(x,y)	if (monitor) monitor->size = (kgi_ucoord_t) { x, y };
#define	Type(t)		if (monitor) monitor->type = t;
#define	Sync(s)		if (monitor) monitor->sync = s;
#define	Bandwidth(l,h)	if (monitor) monitor->dclk = (kgi_urange_t) { l, h };
#define	hFreq(n,l,h)	if (monitor) monitor->hfreq[n] = (kgi_urange_t) { l, h };
#define	vFreq(n,l,h)	if (monitor) monitor->vfreq[n] = (kgi_urange_t) { l, h };
#define	End		return KGI_EOK; \
}

#ifndef MONITOR_SPEC
#	define	MONITOR_SPEC	"monitor-bind.spec"
#endif
#include MONITOR_SPEC
