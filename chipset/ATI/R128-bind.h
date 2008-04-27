/* ----------------------------------------------------------------------------
**	ATI Rage 128 chipset binding definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128-bind.h,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#ifndef	_chipset_ATI_R128_bind_h
#define	_chipset_ATI_R128_bind_h

#include "chipset/ATI/R128-meta.h"

KGIM_META(r128_chipset)
KGIM_META_INIT_MODULE_FN(r128_chipset)
KGIM_META_DONE_MODULE_FN(r128_chipset)

#endif	/* #ifdef _chipset_ATI_R128_bind_h */
