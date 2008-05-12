/* ----------------------------------------------------------------------------
**	KGI kernel module library definitions
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
**	$Log: modulelib.h,v $
**	Revision 1.4  2002/09/23 13:38:03  aldot
**	- add kgim_strncpy
**	
**	Revision 1.3  2002/07/27 00:58:53  aldot
**	- typo in comment
**	
**	Revision 1.1.1.1  2000/04/18 08:51:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:10  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _kgi_modulelib_h
#define _kgi_modulelib_h

/*
**	debugging framework overrides
*/
#if defined(__KERNEL__) && (defined(__MODULE__) || defined(MODULE))

#undef	__krn_debug
#undef	__krn_notice
#undef	__krn_error
#ifdef	KRN_DEBUG_ANSI_CPP
#	define	__krn_debug	kgim_ansi_debug
#	define	__krn_error	kgim_ansi_error
#endif
#ifdef	KRN_DEBUG_GNU_CPP
#	define	__krn_debug	kgim_gnu_debug
#	define	__krn_error	kgim_gnu_error
#endif
#	define	__krn_notice	kgim_notice
#endif	/* #if defined(__KERNEL__) && defined(__MODULE__) */

extern void kgim_notice(const char *fmt, ...);

extern void kgim_ansi_debug(int level, const char *fmt, ...);
extern void kgim_ansi_error(const char *fmt, ...);

extern void kgim_gnu_debug(const char *file, int line, const char *func, int level, const char *fmt, ...);
extern void kgim_gnu_error(const char *file, int line, const char *func, const char *fmt, ...);

extern kgi_u_t kgim_attr_bits(const kgi_u8_t *bpa);

#define kgim_memset memset
#define kgim_memcpy memcpy
#define kgim_memcmp memcmp

#define kgim_strcmp strcmp
#define kgim_strcpy strcpy
#define kgim_strncpy strncpy

extern kgi_error_t kgim_display_init(kgim_display_t *dpy);
extern void kgim_display_done(kgim_display_t *dpy);

#define	KGIM_ABS(x)	(((x) < 0) ? -(x) : (x))

#endif	/* #ifdef _kgi_modulelib_h */
