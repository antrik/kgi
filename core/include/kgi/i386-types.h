/* ----------------------------------------------------------------------------
**	i386 data type definitions
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
**	$Log: i386-types.h,v $
**	Revision 1.1.1.1  2000/04/18 08:50:42  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_kgi_i386_types_h
#define	_kgi_i386_types_h

#ifdef	__STRICT_ANSI__
#	define	signed
#	define	const
#endif

typedef	signed char		__kgi_s8_t;
typedef	unsigned char		__kgi_u8_t;

typedef	signed short		__kgi_s16_t;
typedef	unsigned short		__kgi_u16_t;

typedef	signed int		__kgi_s32_t;
typedef	unsigned int		__kgi_u32_t;

#ifdef	KGI_SYS_NEED_i64
typedef	signed long long	__kgi_s64_t;
typedef	unsigned long long	__kgi_u64_t;
#endif

typedef	signed int		__kgi_s_t;
typedef	unsigned int		__kgi_u_t;

typedef	char			__kgi_ascii_t;
typedef	unsigned short		__kgi_unicode_t;
typedef	unsigned int		__kgi_isochar_t;

typedef int			__kgi_error_t;

typedef	void	 		*__kgi_virt_addr_t;
typedef unsigned long		__kgi_phys_addr_t;
typedef unsigned long		__kgi_bus_addr_t;

typedef unsigned long		__kgi_size_t;


#ifndef	NULL
#	define	NULL	((void *) 0)
#endif

/*	byte order conversions
**
**	For 8bit (byte) types, byteorder conversion doesn't make sense,
**	so only for 16,32 and 64 bit types conversion is defined.
*/

#define	HOST_BE		0
#define	HOST_LE		1

/* !!!	as this is i386/types.h, i386 optimized (inline) functions should
** !!!	go here.
*/
extern __kgi_u16_t	__kgi_u16_swap(const __kgi_u16_t x);
extern __kgi_s16_t	__kgi_s16_swap(const __kgi_s16_t x);

extern __kgi_u32_t	__kgi_u32_swap(const __kgi_u32_t x);
extern __kgi_s32_t	__kgi_s32_swap(const __kgi_s32_t x);

#ifdef	KGI_SYS_NEED_i64
extern __kgi_u64_t	__kgi_u64_swap(const __kgi_u64_t x);
extern __kgi_s64_t	__kgi_s64_swap(const __kgi_s64_t x);
#endif

extern __kgi_unicode_t __kgi_unicode_swap(const __kgi_unicode_t x);
extern __kgi_isochar_t __kgi_isochar_swap(const __kgi_isochar_t x);

#endif	/* #ifdef _kgi_i386_types_h */
