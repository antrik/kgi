/* ----------------------------------------------------------------------------
**	KGI alloc
** ----------------------------------------------------------------------------
**	Copyright (C)	2001	Steffen Seeger
**	Copyright (C)	2001	Bernhard Fischer
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	
**	$Id: alloc.h,v 1.2 2004/03/06 14:18:39 nsouch Exp $
**	
**	Revision 1.1.2.1  2001/11/25 22:59:28  aldot
**	kgi alloc wrapper added
**	
**	
*/
#ifndef	_kgi_alloc_h
#define	_kgi_alloc_h

#if !defined(HOST_OS)
#	error HOST_OS not defined
#endif

/* -------------------------------------------------------------------------- */
#if defined(__KGI_NEED_MALLOC)

#if defined(__KERNEL__) || defined(_KERNEL)


#if (HOST_OS == HOST_OS_Linux)
#	include <linux/slab.h>
#endif
static inline void * kgi_kmalloc(__kgi_size_t size,
				__kgi_u_t flags)
{
#if (HOST_OS == HOST_OS_Linux)
	return kmalloc(size, flags);
#else
#error implement kgi_kmalloc for unknown HOST_OS
#endif
}

static inline void kgi_kfree(const void *ptr)
{
#if (HOST_OS == HOST_OS_Linux)
	kfree(ptr);
#else
#error implement kgi_kfree for unknown HOST_OS
#endif
}

#if (HOST_OS == HOST_OS_Linux)
#	define kmalloc kgi_kmalloc
#	define kfree kgi_kfree
#endif

static inline void * kgi_cmalloc(__kgi_size_t size)
{
#if (HOST_OS == HOST_OS_Linux)
	return (void *)__get_free_pages(GFP_KERNEL, size/PAGE_SIZE);
#else
#error implement kgi_cmalloc for unknown HOST_OS
#endif
}

static inline void kgi_cfree(const void * ptr, __kgi_size_t size)
{
#if (HOST_OS == HOST_OS_Linux)
	free_pages((unsigned long)ptr, size/PAGE_SIZE);
#else
#error implement kgi_cfree for unknown HOST_OS
#endif
}

#endif	/* __KERNEL__ */

#if defined(__MODULE__) || defined(_MODULE)
#	define kgim_alloc kgi_kmalloc
#	define kgim_free kgi_kfree
#endif

#undef __KGI_NEED_MALLOC
#endif /* __KGI_NEED_MALLOC */


#endif	/* _kgi_alloc_h */
