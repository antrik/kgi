/* ----------------------------------------------------------------------------
**	KGI error codes
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: errno.h,v $
**	Revision 1.1.1.1  2000/09/21 09:09:43  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/09/21 09:09:43  seeger_s
**	- namespace cleanup: E() macro -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_kgi_errno_h
#define	_kgi_errno_h

enum __kgi_error
{
	KGI_EOK	= 0,
	KGI_NOMEM,
	KGI_INVAL,
	KGI_NOSUP,
	KGI_NODEV,
	KGI_FAILED,
	KGI_DLL_ERROR,
	KGI_NOT_IMPLEMENTED,
	KGI_UNKNOWN,
	KGI_CLK_LIMIT,

	KGI_NO_DISPLAY,

	KGI_LIB		= 0x01000000,
	KGI_KGI		= 0x02000000,
	KGI_DRIVER	= 0x02000000,
	KGI_MONITOR	= 0x02100000,
	KGI_RAMDAC	= 0x02200000,
	KGI_CHIPSET	= 0x02300000,
	KGI_CLOCK	= 0x02400000,
	
	KGI_DRV		= 0x03000000,

	KGI_ERROR_SUBSYSTEM = 0x7F000000,

	__KGI_LAST_ERROR

};

#define	KGI_ERRNO(system, code)	((KGI_##system) | (KGI_##code))

#endif	/* #ifndef _kgi_errno_h	*/
