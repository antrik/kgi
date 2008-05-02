/* --------------------------------------------------------------------------
**	A generic debugging framework.
** --------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: debug.h,v $
**	Revision 1.3  2002/09/19 22:37:26  aldot
**	- debugging fixes
**	
**	Revision 1.2  2002/07/27 01:22:01  aldot
**	- whitespace cleanup
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	__kgi_debug_h
#define	__kgi_debug_h

/* --------------------------------------------------------------------------
**	kernel debugging macros
** --------------------------------------------------------------------------
*/
#ifdef	__KERNEL__

#	ifdef __STRICT_ANSI__

#		define	KRN_DEBUG_ANSI_CPP	1

#		define	KRN_ERROR	__krn_error
#		define	KRN_DEBUG	__krn_debug
#		define	KRN_NOTICE	__krn_notice

#		define	KRN_ASSERT(x)	\
			if (x) {} else { KRN_DEBUG(0, "", #x); }

#	else

#		define	KRN_DEBUG_GNU_CPP	1

#		define	KRN_ERROR(fmt, args...)	\
			__krn_error( __FILE__ , __LINE__ , \
				__PRETTY_FUNCTION__ , fmt , ##args )

#		ifdef DEBUG_LEVEL
#			define	KRN_DEBUG(level, fmt, args... )		\
				if (level <= DEBUG_LEVEL) {		\
					__krn_debug( __FILE__ ,		\
					__LINE__ , __PRETTY_FUNCTION__	\
					, level , fmt , ##args );	\
				}
#		else
#			define	KRN_DEBUG(x...) do {} while(0)
#		endif

#		define	KRN_NOTICE(fmt, args...)		\
			__krn_notice( fmt , ##args )
#		define	KRN_ASSERT(x)					\
			if (x) {} else {				\
				KRN_ERROR("assertion %s failed.", #x);	\
			}
#	endif

#	define	KRN_INTERNAL_ERROR	\
		KRN_ERROR("internal error. Please report to %s. Thanks.", \
			MAINTAINER)

#ifdef DEBUG_LEVEL
#	define	KRN_TRACE(level, x)	\
		if (level <= DEBUG_LEVEL) { x; } else do {} while (0)
#else
#	define	KRN_TRACE(level, x)	
#endif

#ifndef PANIC
#	define PANIC(x)	\
		panic(__FILE__ ": " x);
#endif


#else	/* #ifdef __KERNEL__ */

/* ----------------------------------------------------------------------------
**	library debugging macros
** --------------------------------------------------------------------------
*/
#ifdef	__LIBRARY__

#	ifdef __STRICT_ANSI__

#		define	LIB_DEBUG_ANSI_CPP	1

#		define	LIB_ERROR	__lib_error
#		define	LIB_DEBUG	__lib_debug
#		define	LIB_NOTICE	__lib_notice

#		define	LIB_ASSERT(x)	if (x) {} else {}

#	else

#		define	LIB_DEBUG_GNU_CPP	1

#		define	LIB_ERROR(fmt, args...)	\
			__lib_error( __FILE__ , __LINE__ , \
				__PRETTY_FUNCTION__ , fmt , ##args )

#		ifdef DEBUG_LEVEL
#			define	LIB_DEBUG(level, fmt, args... )		\
				if (level <= DEBUG_LEVEL) {		\
					__lib_debug( __FILE__ ,		\
					__LINE__ , __PRETTY_FUNCTION__	\
					, level , fmt , ##args );	\
				}
#		else
#			define	LIB_DEBUG(x...) do {} while(0)
#		endif

#		define	LIB_NOTICE(fmt, args...)		\
			__lib_notice( fmt , ##args )
#		define	LIB_ASSERT(x)	\
			if (x) {} else { LIB_DEBUG(0, "assertion %s failed.", #x); }
#	endif

#	define	LIB_INTERNAL_ERROR	\
		LIB_ERROR("internal error. Please report to %s. Thanks.", MAINTAINER)

#ifdef DEBUG_LEVEL
#	define	LIB_TRACE(level, x)	\
		if (level <= DEBUG_LEVEL) { x; } else do {} while (0)
#else
#	define	LIB_TRACE(level, x)	
#endif

#endif	/* #ifdef __LIBRARY__ */

/* --------------------------------------------------------------------------
**	application debugging macros
** --------------------------------------------------------------------------
*/

#ifdef	__APPLICATION__

#	ifdef __STRICT_ANSI__

#		define	APP_DEBUG_ANSI_CPP	1

#		define	APP_ERROR	__app_error
#		define	APP_DEBUG	__app_debug
#		define	APP_NOTICE	__app_notice

#		define	APP_ASSERT(x)	if (x) {} else {}

#	else

#		define	APP_DEBUG_GNU_CPP	1

#		define	APP_ERROR(fmt, args...)	\
			__app_error( __FILE__ , __LINE__ , \
				__PRETTY_FUNCTION__ , fmt , ##args )

#		ifdef DEBUG_LEVEL
#			define	APP_DEBUG(level, fmt, args... )		\
				if (level <= DEBUG_LEVEL) {		\
					__app_debug( __FILE__ ,		\
					__LINE__ , __PRETTY_FUNCTION__	\
					, level , fmt , ##args );	\
				}
#		else
#			define	APP_DEBUG(x...) do {} while(0)
#		endif

#		define	APP_NOTICE(fmt, args...)		\
			__app_notice( fmt , ##args )
#		define	APP_ASSERT(x)	\
			if (x) {} else { APP_DEBUG(0, "assertion %s failed.", #x); }
#	endif

#	define	APP_INTERNAL_ERROR	\
		APP_ERROR("internal error. Please report to %s. Thanks.", MAINTAINER)

#ifdef DEBUG_LEVEL
#	define	APP_TRACE(level, x)	\
		if (level <= DEBUG_LEVEL) { x; } else do {} while (0)
#else
#	define	APP_TRACE(level, x)	
#endif

#endif	/* #ifdef __APPLICATION__ */
#endif	/* #ifdef __KERNEL__ */

#endif	/* #ifdef __kgi_debug_h */
