/* ----------------------------------------------------------------------------
**	VGA meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: VGA-bind.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:20  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:20  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_IBM_VGA_bind_h
#define	_chipset_IBM_VGA_bind_h

#include "chipset/IBM/VGA-meta.h"

KGIM_META(vga_chipset)
KGIM_META_INIT_MODULE_FN(vga_chipset)
KGIM_META_DONE_MODULE_FN(vga_chipset)

#endif	/* #ifdef _chipset_IBM_VGA_bind_h */
