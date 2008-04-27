/* ----------------------------------------------------------------------------
**	Matrox Gx00 chipset binding definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Rodolphe_Ortalo
**
**	$Id: Gx00-bind.h,v 1.2 2002/11/30 18:23:13 ortalo Exp $
*/
#ifndef	_chipset_Matrox_Gx00_bind_h
#define	_chipset_Matrox_Gx00_bind_h

#include "chipset/Matrox/Gx00-meta.h"

KGIM_META(mga_chipset)
KGIM_META_INIT_MODULE_FN(mga_chipset)
KGIM_META_DONE_MODULE_FN(mga_chipset)

#endif	/* #ifdef _chipset_Matrox_Gx00_bind_h */
