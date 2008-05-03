/* ----------------------------------------------------------------------------
**	KGI system layer defintion
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
**	$Log: system.h,v $
**	Revision 1.7  2005/01/23 21:03:06  nsouch
**	Prevent include of kernel headers in user apps.
**	
**	Revision 1.6  2004/03/06 14:18:39  nsouch
**	
**	Enable compilation of MACH64 chipset driver.
**	
**	Include alloc.h in system.h to later remove alloc.h. One should now
**	specify KGI_SYS_NEED_MALLOC to get alloc stuff included.
**	
**	Revision 1.5  2002/10/15 00:02:43  aldot
**	- include cleanup
**	
**	Revision 1.4  2002/09/21 14:31:18  aldot
**	- add complaint about lack of high-res timer support
**	
**	Revision 1.3  2002/07/25 19:26:41  foske
**	Fixed a compile error.
**	
**	Revision 1.2  2002/07/22 17:53:31  foske
**	Implemented kgi_udelay
**	
**	Revision 1.1.1.1  2001/10/01 14:16:11  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/09/06 03:34:20  skids
**	
**	typo in comments
**	
**	Revision 1.1.1.1  2000/04/18 08:50:43  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_kgi_system_h
#define	_kgi_system_h

#include <kgi/compiler.h>

/*	Operating systems
*/
#define	HOST_OS_IRIX	1
#define	HOST_OS_Linux	2
#define	HOST_OS_BeOS	3
#define HOST_OS_FreeBSD 4
#define HOST_OS_GNU	5

/*	Intel CPUs
*/
#define	HOST_CPU_i286	0x0001
#define	HOST_CPU_i386	0x0002
#define	HOST_CPU_i486	0x0003
#define	HOST_CPU_i586	0x0004
#define	HOST_CPU_i686	0x0005
#define	HOST_CPU_BePC	HOST_CPU_i586

/*	MIPS CPUs
*/
#define	HOST_CPU_IP32	0x0101

#if !(defined(HOST_OS) || defined(HOST_CPU))
#	error HOST_OS or HOST_CPU not defined
#endif

#ifdef KGI_SYS_NEED_ATOMIC

#if (HOST_OS == HOST_OS_Linux)
#define kgi_atomic_t atomic_t
#define kgi_atomic_set atomic_set
#define kgi_atomic_read atomic_read
#else
#error kgi_atomic_t not implemented
#endif

#endif /* KGI_SYS_NEED_ATOMIC */

#if ((HOST_OS == HOST_OS_IRIX) && (HOST_CPU == HOST_CPU_IP32))
#	include <kgi/mips-types.h>
#	ifdef	KGI_SYS_NEED_IO
#		define	__KGI_SYS_IO_HAS_MEM
#	endif
#endif

/*
**	i386 CPUs
*/
#if ((HOST_OS == HOST_OS_Linux) || (HOST_OS == HOST_OS_BeOS) || (HOST_OS == HOST_OS_GNU))
#	if ((HOST_CPU == HOST_CPU_i386) || (HOST_CPU == HOST_CPU_i486) || \
		(HOST_CPU == HOST_CPU_i586) || (HOST_CPU == HOST_CPU_i686))
#		include <kgi/i386-types.h>
#		ifdef KGI_SYS_NEED_IO
#			define	__KGI_SYS_IO_HAS_PCICFG
#			define	__KGI_SYS_IO_HAS_BUS
#			define	__KGI_SYS_IO_HAS_IO
#			define	__KGI_SYS_IO_HAS_MEM
#			define	__KGI_SYS_IO_HAS_IRQ
#			include <kgi/io.h>
#			if (HOST_OS == HOST_OS_Linux)
#				include <linux/pci.h>
#			else
#				include <kgi/pci/pci.h>
#			endif
#		endif
#		include <kgi/i386-compiler.h>
#	endif
#endif

/*	this is common to all archs ------------------------------------------
*/
#if !defined(HOST_BE) || !defined(HOST_LE)
#	error check this, no type definitions included?
#endif

#if (BITS_PER_LONG == 64)
#	define KGI_SYS_NEED_i64
#endif

#if (HOST_OS == HOST_OS_GNU)
	static inline void _nanosleep(__kgi_u32_t ns)
	{
		#include <errno.h>
		#include <time.h>
		struct timespec ts={0, ns};
		while(nanosleep(&ts, &ts) && errno==EINTR)
			;
	}
#endif

static inline void kgi_udelay (__kgi_u32_t d) {
#if (HOST_OS == HOST_OS_Linux)
	#include <asm/delay.h>
	udelay ((unsigned long)d);
#elif (HOST_OS == HOST_OS_GNU)
	/* XXX is nanosleep() precise enough for this?... */
	_nanosleep(1000*d);
#else
#error kgi_udelay not implemented
#endif
}

#if (HOST_OS == HOST_OS_Linux)
/*FIXME: inaccurate */
#define kgi_nanosleep(x) kgi_udelay(2)
#elif (HOST_OS == HOST_OS_GNU)
#define kgi_nanosleep(x) _nanosleep(x)
#else
#error kgi_nanosleep not implemented
#endif

#ifdef __KERNEL__
#if (HOST_OS == HOST_OS_Linux)
#include <linux/sched.h>
#define kgi_wakeup wake_up
#elif (HOST_OS == HOST_OS_GNU)
#include <error.h>
#define kgi_wakeup(x) error_at_line(1, 0, __FILE__, __LINE__, "kgi_wakeup() not implemented")
#else
#error kgi_wakeup not implemented
#endif
#endif

#ifndef	KGI_SYS_NEED_i64

	typedef struct 
	{
		__kgi_u32_t low; __kgi_u32_t high; 

	} 			__kgi_i64_t;
	typedef __kgi_i64_t	__kgi_s64_t;
	typedef __kgi_i64_t	__kgi_u64_t;
#endif

typedef union
{
	__kgi_s8_t		priv_s8;
	__kgi_u8_t		priv_u8;

	__kgi_s16_t		priv_s16;
	__kgi_u16_t		priv_u16;

	__kgi_s32_t		priv_s32;
	__kgi_u32_t		priv_u32;

	__kgi_s64_t		priv_s64;
	__kgi_u64_t		priv_u64;

	__kgi_s_t		priv_s;
	__kgi_u_t		priv_u;

	__kgi_ascii_t		priv_ascii;
	__kgi_unicode_t		priv_unicode;
	__kgi_isochar_t		priv_isochar;

	__kgi_virt_addr_t	priv_virt_addr;
	__kgi_phys_addr_t	priv_phys_addr;
	__kgi_bus_addr_t	priv_bus_addr;

	__kgi_size_t		priv_size;

	void			*priv_ptr;

} __kgi_private_t;

#define	KGI_SYS_DECLARE_INTEGRAL_TYPES(prefix)	\
	typedef __kgi_u8_t		prefix ## _u8_t;	\
	typedef	__kgi_s8_t		prefix ## _s8_t;	\
	typedef	__kgi_u16_t		prefix ## _u16_t;	\
	typedef	__kgi_s16_t		prefix ## _s16_t;	\
	typedef	__kgi_u32_t		prefix ## _u32_t;	\
	typedef	__kgi_s32_t		prefix ## _s32_t;	\
	typedef	__kgi_u64_t		prefix ## _u64_t;	\
	typedef	__kgi_s64_t		prefix ## _s64_t;	\
	typedef	__kgi_u_t		prefix ## _u_t;		\
	typedef	__kgi_s_t		prefix ## _s_t;		\
	typedef	__kgi_error_t		prefix ## _error_t;	\
	typedef __kgi_ascii_t		prefix ## _ascii_t;	\
	typedef __kgi_unicode_t		prefix ## _unicode_t;	\
	typedef __kgi_isochar_t		prefix ## _isochar_t;	\
	typedef	__kgi_private_t		prefix ## _private_t;	\
	typedef __kgi_virt_addr_t	prefix ## _virt_addr_t;	\
	typedef __kgi_phys_addr_t	prefix ## _phys_addr_t;	\
	typedef __kgi_bus_addr_t	prefix ## _bus_addr_t;	\
	typedef __kgi_size_t		prefix ## _size_t;

/*
**	byte order conversions
*/
#if HOST_BE

/*	host -> big endian, no conversion required for big endian host
*/
#	define	KGI_BE_isochar(x)	((__kgi_isochar_t) x)
#	define	KGI_BE_unicode(x)	((__kgi_unicode_t) x)
#	define	KGI_BE_s16(x)		((__kgi_s16_t) x)
#	define	KGI_BE_u16(x)		((__kgi_u16_t) x)
#	define	KGI_BE_s32(x)		((__kgi_s32_t) x)
#	define	KGI_BE_u32(x)		((__kgi_u32_t) x)
#	ifdef KGI_SYS_NEED_i64
#		define	KGI_BE_s64(x)	((__kgi_s64_t) x)
#		define	KGI_BE_u64(x)	((__kgi_u64_t) x)
#	endif

/*	host -> little endian, conversion required for big endian host
*/
#	define	KGI_LE_isochar(x)	__kgi_isochar_swap(x)
#	define	KGI_LE_unicode(x)	__kgi_unicode_swap(x)
#	define	KGI_LE_s16(x)		__kgi_s16_swap(x)
#	define	KGI_LE_u16(x)		__kgi_u16_swap(x)
#	define	KGI_LE_s32(x)		__kgi_s32_swap(x)
#	define	KGI_LE_u32(x)		__kgi_u32_swap(x)
#	ifdef KGI_SYS_NEED_i64
#		define	KGI_LE_s64(x)	__kgi_s64_swap(x)
#		define	KGI_LE_u64(x)	__kgi_u64_swap(x)
#	endif

#endif


#if HOST_LE

/*	host <-> big endian, conversion required for little endian host
*/
#	define	KGI_BE_isochar(x)	__kgi_isochar_swap(x)
#	define	KGI_BE_unicode(x)	__kgi_unicode_swap(x)
#	define	KGI_BE_s16(x)		__kgi_s16_swap(x)
#	define	KGI_BE_u16(x)		__kgi_u16_swap(x)
#	define	KGI_BE_s32(x)		__kgi_s32_swap(x)
#	define	KGI_BE_u32(x)		__kgi_u32_swap(x)
#	ifdef KGI_SYS_NEED_i64
#		define	KGI_BE_s64(x)	__kgi_s64_swap(x)
#		define	KGI_BE_u64(x)	__kgi_u64_swap(x)
#	endif

/*	host <-> little endian, no conversion required for little endian host
*/
#	define	KGI_LE_isochar(x)	((__kgi_isochar_t) x)
#	define	KGI_LE_unicode(x)	((__kgi_unicode_t) x)
#	define	KGI_LE_s16(x)		((__kgi_s16_t) x)
#	define	KGI_LE_u16(x)		((__kgi_u16_t) x)
#	define	KGI_LE_s32(x)		((__kgi_s32_t) x)
#	define	KGI_LE_u32(x)		((__kgi_u32_t) x)
#	ifdef KGI_SYS_NEED_i64
#		define	KGI_LE_s64(x)	((__kgi_s64_t) x)
#		define	KGI_LE_u64(x)	((__kgi_u64_t) x)
#	endif

#endif

#define	KB	*1024
#define	MB	*(1024*1024)
#define	GB	*(1024*1024*1024)


/*	ISO 10646 defines symbol codes as 32 bit unsigned int organized into
**	groups, planes, rows and cells. By now only the first plane (group=0,
**	plane=0) is defined, which maps directly to UNICODE. Use these macros
**	to extract group, plane, row or cell.
*/
#define KGI_ISOCHAR_GROUP(isochar)	(((isochar) >> 24) & 0xFF)
#define	KGI_ISOCHAR_PLANE(isochar)	(((isochar) >> 16) & 0xFF)
#define	KGI_ISOCHAR_ROW(isochar)	(((isochar) >>  8) & 0xFF)
#define	KGI_ISOCHAR_CELL(isochar)	((isochar) & 0xFF)

#ifdef KGI_SYS_NEED_MALLOC

#define __KGI_NEED_MALLOC
#include <kgi/alloc.h>

#endif /* KGI_SYS_NEED_MALLOC */

#endif	/* #ifdef _kgi_sys_system_h */
