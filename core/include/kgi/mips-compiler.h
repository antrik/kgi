/* ----------------------------------------------------------------------------
**	MIPS compiler dependent stuff
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
**	$Log: mips-compiler.h,v $
**	Revision 1.1.1.1  2000/04/18 08:50:43  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:43  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_kgi_mips_compiler_h
#define	_kgi_mips_compiler_h

#ifdef __GNUC__
#ifdef	KGI_SYS_NEED_IO
/*
**	memory I/O space
*/

#define	__DECLARE_MEM_OPS(w)						\
	extern inline __kgi_u##w##_t mem_in##w (mem_vaddr_t addr) 	\
	{								\
		return *((__volatile__ __kgi_u##w##_t *) addr);		\
	}								\
	extern inline void mem_out##w (const __kgi_u##w##_t val,	\
		const mem_vaddr_t addr)					\
	{								\
		*((__volatile__ __kgi_u##w##_t *) addr) = val;		\
	}								\
	extern inline void mem_ins##w (const mem_vaddr_t addr, void *buf, \
		__kgi_size_t cnt)					\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			((__volatile__ __kgi_u##w##_t *) buf)[i++] =	\
				*((__volatile__ __kgi_u##w##_t *) addr);\
		}							\
	}								\
	extern inline void mem_outs##w (const mem_vaddr_t addr,		\
		const void *buf, __kgi_size_t cnt)			\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			*((__volatile__ __kgi_u##w##_t *) addr) =	\
			     ((__volatile__ __kgi_u##w##_t *) buf)[i++];\
		}							\
	}

__DECLARE_MEM_OPS(8)
__DECLARE_MEM_OPS(16)
__DECLARE_MEM_OPS(32)

#endif	/* #ifdef KGI_SYS_NEED_IO */
#endif	/* #ifdef __GNUC__	*/

#endif	/* #ifdef _kgi_mips_compiler_h */
