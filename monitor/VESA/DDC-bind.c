/* ----------------------------------------------------------------------------
**	DDC monitor meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2001	Jos Hulzink	
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	
*/
#include <kgi/module.h>

#define MODULE
#include <linux/config.h>
#include <linux/module.h>

MODULE_LICENSE("GPL and additional rights");

#include "monitor/VESA/DDC-meta.h"

extern kgi_error_t monitor_init_module_Standard_SVGA(kgim_monitor_t *monitor,
	const kgim_options_t *options);

extern const DDC_timing_t DDC_monitor_timing;

kgi_error_t DDC_monitor_init_module(DDC_monitor_t *DDC,
	const kgim_options_t *options)
{
	KRN_DEBUG(0, "DDC_monitor_init_module()");
	
	//DDC->timing = &DDC_monitor_timing;
	DDC->monitor.revision= KGIM_MONITOR_REVISION;
	kgim_strcpy(DDC->monitor.vendor,"VESA");
	kgim_strcpy(DDC->monitor.model ,"DDC");
	DDC->monitor.flags  = 0;
	DDC->monitor.maxdots= (kgi_ucoord_t) {0,0};
	DDC->monitor.size   = (kgi_ucoord_t) {0,0};
	DDC->monitor.type   = 0;
	DDC->monitor.sync   = 0;
	DDC->monitor.dclk   = (kgi_urange_t) {0,0};
	DDC->monitor.hfreq[0] = (kgi_urange_t) {0,0};
	DDC->monitor.vfreq[0] = (kgi_urange_t) {0,0};

	return 0;
}

const kgim_meta_t DDC_monitor_meta =
{
	(kgim_meta_init_module_fn *)	DDC_monitor_init_module,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		DDC_monitor_init,
	(kgim_meta_done_fn *)		NULL,
	(kgim_meta_mode_check_fn *)	DDC_monitor_mode_check,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	NULL,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(DDC_monitor_t),
	0,
	sizeof(DDC_monitor_mode_t)
};
