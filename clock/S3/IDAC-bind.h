/* -----------------------------------------------------------------------------
**	S3 IDAC PLL meta language binding
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: IDAC-bind.h,v $
**	Revision 1.1.1.1  2000/08/04 11:51:37  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/08/04 11:51:37  seeger_s
**	- merged driver posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- driver status 30% reported by Jos Hulzink
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _clock_S3_IDAC_bind_h
#define	_clock_S3_IDAC_bind_h

#include "clock/S3/IDAC-meta.h"

KGIM_META_INIT_MODULE_FN(idac_clock)
KGIM_META(idac_clock)

#endif	/* #ifndef _clock_S3_IDAC_bind_h	*/
