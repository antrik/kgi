/* ----------------------------------------------------------------------------
**	i386 compiler dependent stuff
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
**	$Log: i386-compiler.h,v $
**	Revision 1.1.1.1  2000/04/18 08:50:42  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_kgi_i386_compiler_h
#define	_kgi_i386_compiler_h

#ifdef __STRICT_ANSI__
#	undef	inline
#	define	inline
#endif

#ifdef __GNUC__
#ifdef	KGI_SYS_NEED_IO
/*
**	Intel style in/out I/O space
*/
extern __inline__ __kgi_u8_t io_in8 (const io_vaddr_t addr) 
{
	__kgi_u8_t  val;
	__asm__ __volatile__ ("inb %w1,%0" : "=a" (val) : "Nd" (addr));
	return val;
}

extern __inline__ __kgi_u16_t io_in16(const io_vaddr_t addr)
{
	__kgi_u16_t val;
	__asm__ __volatile__ ("inw %w1,%0" : "=a" (val) : "Nd" (addr));
	return val;
}

extern __inline__ __kgi_u32_t io_in32(const io_vaddr_t addr)
{
	__kgi_u32_t val;
	__asm__ __volatile__ ("inl %w1,%0" : "=a" (val) : "Nd" (addr));
	return val;
}


extern __inline__ void io_out8(const __kgi_u8_t val, const io_vaddr_t addr)
{
	__asm__ __volatile__ ("outb %b0,%w1" : : "a" (val), "Nd" (addr));
}

extern __inline__ void io_out16(const __kgi_u16_t val, const io_vaddr_t addr)
{
	__asm__ __volatile__ ("outw %w0,%w1" : : "a" (val), "Nd" (addr));
}

extern __inline__ void io_out32(const __kgi_u32_t val, io_vaddr_t addr)
{
	__asm__ __volatile__ ("outl %0,%w1" : : "a" (val), "Nd" (addr));
}


extern __inline__ void io_ins8(const io_vaddr_t addr, void *buf, 
	__kgi_size_t cnt)
{
	__asm__ __volatile__ ("cld ; rep ; insb" 
		: "=D" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

extern __inline__ void io_ins16(const io_vaddr_t addr, void * buf,
	__kgi_size_t cnt)
{
	__asm__ __volatile__ ("cld ; rep ; insw" 
		: "=D" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

extern __inline__ void io_ins32(const io_vaddr_t addr, void *buf,
	__kgi_size_t cnt)
{
	__asm__ __volatile__ ("cld ; rep ; insl"
		: "=D" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

extern __inline__ void io_outs8(const io_vaddr_t addr, const void *buf,
	__kgi_size_t cnt)
{
	__asm__ __volatile__ ("cld ; rep ; outsb"
		: "=S" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

extern __inline__ void io_outs16(const io_vaddr_t addr, const void *buf,
	__kgi_size_t cnt)
{
	__asm__ __volatile__ ("cld ; rep ; outsw"
		: "=S" (buf), "=c" (cnt) : "d" (addr), "0" (buf), "1" (cnt));
}

extern __inline__ void io_outs32(const io_vaddr_t addr, const void *buf,
	__kgi_size_t cnt)
{
	__asm__ __volatile__ ("cld ; rep ; outsl"
		: "=S" (buf), "=c" (cnt) : "d" (addr),"0" (buf), "1" (cnt));
}

/*
**	memory I/O space
*/

#define	__DECLARE_MEM_OPS(w)						\
	extern __inline__ __kgi_u##w##_t mem_in##w (mem_vaddr_t addr) 	\
	{								\
		return *((__volatile__ __kgi_u##w##_t *) addr);		\
	}								\
	extern __inline__ void mem_out##w (const __kgi_u##w##_t val,	\
		const mem_vaddr_t addr)					\
	{								\
		*((__volatile__ __kgi_u##w##_t *) addr) = val;		\
	}								\
	extern __inline__ void mem_ins##w (const mem_vaddr_t addr,	\
		void *buf, __kgi_size_t cnt)				\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			((__volatile__ __kgi_u##w##_t *) buf)[i++] =	\
				*((__volatile__ __kgi_u##w##_t *) addr);\
		}							\
	}								\
	extern __inline__ void mem_outs##w (const mem_vaddr_t addr, 	\
		const void *buf, __kgi_size_t cnt)			\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			*((__volatile__ __kgi_u##w##_t *) addr) =	\
			     ((__volatile__ __kgi_u##w##_t *) buf)[i++];\
		}							\
	}								\
	extern __inline__ void mem_put##w (const mem_vaddr_t addr,	\
		const void *buf, __kgi_size_t cnt)			\
	{								\
		register __kgi_size_t i = 0;				\
		while (i < cnt) {					\
			((__volatile__ __kgi_u##w##_t *) addr)[i] =	\
			     ((__volatile__ __kgi_u##w##_t *) buf)[i];	\
			i++;						\
		}							\
	}

__DECLARE_MEM_OPS(8)
__DECLARE_MEM_OPS(16)
__DECLARE_MEM_OPS(32)

#undef	inline
#define	inline	__inline__

#endif	/* #ifdef KGI_SYS_NEED_IO */
#endif	/* #ifdef __GNUC__	*/

#endif	/* #ifdef _kgi_i386_compiler_h */
