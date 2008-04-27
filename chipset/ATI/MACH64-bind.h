/* ----------------------------------------------------------------------------
**	ATI MACH64 chipset binding definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64-bind.h,v $
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:35  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:16:33  phil
**	Automatically generated ATI chipset code
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_ATI_MACH64_bind_h
#define	_chipset_ATI_MACH64_bind_h

#include "chipset/ATI/MACH64-meta.h"

KGIM_META(mach64_chipset)
KGIM_META_INIT_MODULE_FN(mach64_chipset)
KGIM_META_DONE_MODULE_FN(mach64_chipset)

#endif	/* #ifdef _chipset_ATI_MACH64_bind_h */
