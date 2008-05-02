/* ----------------------------------------------------------------------------
**	KII error codes
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
**	$Log: errno.h,v $
**	Revision 1.1.1.1  2000/09/21 09:19:30  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1  2000/09/21 09:19:30  seeger_s
**	- added /dev/event mapper definitions
**	
*/
#ifndef	_kii_errno_h
#define	_kii_errno_h

enum __kii_error
{
	KII_EOK	= 0,
	KII_NOMEM,
	KII_INVAL,
	KII_NOSUP,
	KII_NODEV,
	KII_FAILED,
	KII_DLL_ERROR,
	KII_NOT_IMPLEMENTED,
	KII_UNKNOWN,

	KII_NO_FOCUS,

	KII_LIB		= 0x01000000,
	KII_KII		= 0x02000000,
	KII_DRIVER	= 0x03000000,
	KII_DEVICE	= 0x03000000,

	KII_ERROR_SUBSYSTEM = 0x7F000000,

	__KII_LAST_ERROR

};

#define	KII_ERRNO(system, code)	((KII_##system) | (KII_##code))

#endif	/* #ifndef _kii_errno_h	*/
