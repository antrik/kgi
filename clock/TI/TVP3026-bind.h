/* ----------------------------------------------------------------------------
**	TI TVP3026 PLL binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: TVP3026-bind.h,v $
**	Revision 1.1.1.1  2000/04/26 14:12:18  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1  2000/04/26 14:12:18  seeger_s
**	- added TI TVP3026 driver
**	
*/
#ifndef _clock_TI_TVP3026_bind_h
#define	_clock_TI_TVP3026_bind_h

#include "clock/TI/TVP3026-meta.h"

KGIM_META_INIT_MODULE_FN(tvp3026_clock)
KGIM_META(tvp3026_clock)

#endif	/* #ifndef _clock_TI_TVP3026_bind_h	*/
