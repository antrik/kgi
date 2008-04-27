/* ----------------------------------------------------------------------------
**	ATI RADEON PLL binding
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: RADEON-bind.c,v $
**	Revision 1.3  2003/08/27 21:46:45  redmondp
**	- try to read clock programming constants from the ROM
**	
**	Revision 1.2  2003/03/13 03:05:13  fspacek
**	- remove radeon clock (merged into chipset)
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#include <kgi/maintainers.h>
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.3 $"

#include <kgi/module.h>

const kgim_meta_t radeon_clock_meta =
{
	(kgim_meta_init_module_fn *)	NULL,
	(kgim_meta_done_module_fn *)	NULL,
	(kgim_meta_init_fn *)		NULL,
	(kgim_meta_done_fn *)		NULL,
	(kgim_meta_mode_check_fn *)	NULL,
	(kgim_meta_mode_resource_fn *)	NULL,
	(kgim_meta_mode_prepare_fn *)	NULL,
	(kgim_meta_mode_enter_fn *)	NULL,
	(kgim_meta_mode_leave_fn *)	NULL,
	(kgim_meta_image_resource_fn *)	NULL,

	0,
	0,
	0
};
