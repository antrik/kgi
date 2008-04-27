/* ----------------------------------------------------------------------------
**	ViRGE meta language binding definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: ViRGE-bind.h,v $
**	Revision 1.2  2003/01/30 13:44:46  foske
**	Mayor cleanup, fix textmode, drop secondary card support, it wasn't going to work anyway.
**	
**	Revision 1.1.1.1  2000/08/04 11:39:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/08/04 11:39:08  seeger_s
**	- merged version posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- status is reported to be 30% by Jos.
**	
**	Revision 1.1.1.1  2000/04/18 08:51:25  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_S3_ViRGE_bind_h
#define	_chipset_S3_ViRGE_bind_h

#include "chipset/S3/ViRGE-meta.h"

extern virge_chipset_save_state (virge_chipset_state_t *, virge_chipset_io_t *);
extern virge_chipset_restore_state (virge_chipset_state_t *, virge_chipset_io_t *);


KGIM_META(virge_chipset)
KGIM_META_INIT_MODULE_FN(virge_chipset)
KGIM_META_DONE_MODULE_FN(virge_chipset)

#endif	/* #ifdef _chipset_S3_ViRGE_bind_h */
