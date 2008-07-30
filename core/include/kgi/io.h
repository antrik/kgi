/* ----------------------------------------------------------------------------
**	hardware I/O layer definiton
** ----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: io.h,v $
**	Revision 1.7  2004/03/03 20:51:30  nsouch
**	
**	Add rid field. This may not yet solve all the MACH64 compilation: still
**	in progress.
**	
**	Revision 1.6  2003/01/25 02:57:45  aldot
**	- add IF sample random
**	
**	Revision 1.5  2002/10/15 00:02:43  aldot
**	- include cleanup
**	
**	Revision 1.4  2002/09/19 22:39:30  aldot
**	- silence gcc
**	
**	Revision 1.3  2002/07/27 00:00:26  aldot
**	- for 'crappy' 32bit virtual addresses, emit PCICFG_NULL
**	
**	Revision 1.2  2002/06/12 00:52:15  aldot
**	- add pcicfg_find_class needed for VGA driver
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_kgi_io_h
#define	_kgi_io_h

/*	Explanations on I/O regions can be found in the system layer 
**	documentation.
*/

/* ----------------------------------------------------------------------------
**	PCI configuration space
** ----------------------------------------------------------------------------
**	These functions should be used to access the PCI configuration space.
**	As the address is given by the hardware wiring, a registration
**	scheme doesn't make sense. But I/O is pretty useful :) For the same
**	reason, and as it is used for configuration (bootstrap) purposes,
**	a physical->virtual mapping is not possible. All addresses are
**	virtual and no mapping takes place.
*/
#ifdef __KGI_SYS_IO_HAS_PCICFG


#if KGI_SYS_NEED_i64
typedef __kgi_u64_t	pcicfg_vaddr_t;	/* the virtual address type	*/
#define PCICFG_64_NULL ((pcicfg_vaddr_t) 0xFFFFFFFFFFFFFFFF)
#define PCICFG_NULL PCICFG_64_NULL
#else
typedef __kgi_u32_t	pcicfg_vaddr_t;	/* the virtual address type	*/
#define	PCICFG_32_NULL ((pcicfg_vaddr_t) 0xFFFF0000)	/* an invalid virtual address	*/
#define PCICFG_NULL PCICFG_32_NULL
#endif

#define	PCICFG_VADDR(bus, slot, fn)	\
	!((bus+1 > 0) || (slot+1 > 0) || (fn+1 > 0))			\
	? PCICFG_NULL							\
	: ( (pcicfg_vaddr_t)						\
		((((bus) & 0xFF) << 24) | (PCI_DEVFN((slot),(fn)) << 16)) )

#define	PCICFG_BUS(vaddr)	(((vaddr) >> 24) & 0xFF)
#define	PCICFG_DEV(vaddr)	PCI_SLOT(((vaddr) >> 16) & 0xFF)
#define	PCICFG_FN(vaddr)	PCI_FUNC(((vaddr) >> 16) & 0xFF)
#define	PCICFG_REG(vaddr)	((vaddr) & 0xFFFF)
#define	PCICFG_SIGNATURE(vendor, device) ((vendor << 16) | device)

extern int pcicfg_find_class(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures);
extern int pcicfg_find_device(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures);
extern int pcicfg_find_subsystem(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures);

extern __kgi_u8_t  pcicfg_in8 (const pcicfg_vaddr_t vaddr);
extern __kgi_u16_t pcicfg_in16(const pcicfg_vaddr_t vaddr);
extern __kgi_u32_t pcicfg_in32(const pcicfg_vaddr_t vaddr);

extern void pcicfg_out8 (const __kgi_u8_t  val, const pcicfg_vaddr_t vaddr);
extern void pcicfg_out16(const __kgi_u16_t val, const pcicfg_vaddr_t vaddr);
extern void pcicfg_out32(const __kgi_u32_t val, const pcicfg_vaddr_t vaddr);

#define	PCICFG_SET_BASE32(value, reg)		\
		pcicfg_out32(0xFFFFFFFF, reg);	\
		pcicfg_in32(reg);		\
		pcicfg_out32((value), reg)

#endif	/* #ifdef __KGI_SYS_IO_HAS_PCICFG	*/


#define __KGI_SYS_IO(x)	\
typedef struct								\
{									\
	pcicfg_vaddr_t	device;		/* (PCI) device this belongs to	*/\
	x##_vaddr_t	base_virt;	/* virtual base address		*/\
	x##_paddr_t	base_io;	/* I/O base address		*/\
	x##_baddr_t	base_bus;	/* bus address			*/\
	x##_paddr_t	base_phys;	/* physical address		*/\
	__kgi_size_t	size;		/* size of region		*/\
	x##_paddr_t	decode;		/* decoded I/O address lines	*/\
	__kgi_u32_t	rid;		/* region unique id for device  */\
	const __kgi_ascii_t *name;	/* name of the region		*/\
									\
} x##_region_t;								\
extern int	   x##_check_region (x##_region_t *r);			\
extern x##_vaddr_t x##_claim_region (x##_region_t *r);			\
extern x##_vaddr_t x##_free_region  (x##_region_t *r);			\
extern x##_vaddr_t x##_alloc_region (x##_region_t *r);			\
									\
extern __kgi_u8_t  x##_in8  (const x##_vaddr_t vaddr);			\
extern __kgi_u16_t x##_in16 (const x##_vaddr_t vaddr);			\
extern __kgi_u32_t x##_in32 (const x##_vaddr_t vaddr);			\
									\
extern void x##_out8  (const __kgi_u8_t  val, const x##_vaddr_t vaddr);	\
extern void x##_out16 (const __kgi_u16_t val, const x##_vaddr_t vaddr);	\
extern void x##_out32 (const __kgi_u32_t val, const x##_vaddr_t vaddr);	\
									\
extern void x##_ins8  (const x##_vaddr_t addr, void *buf, __kgi_size_t cnt);\
extern void x##_ins16 (const x##_vaddr_t addr, void *buf, __kgi_size_t cnt);\
extern void x##_ins32 (const x##_vaddr_t addr, void *buf, __kgi_size_t cnt);\
									\
extern void x##_outs8  (const x##_vaddr_t addr, const void *buf, __kgi_size_t cnt);\
extern void x##_outs16 (const x##_vaddr_t addr, const void *buf, __kgi_size_t cnt);\
extern void x##_outs32 (const x##_vaddr_t addr, const void *buf, __kgi_size_t cnt);\
  


/* ----------------------------------------------------------------------------
**	Intel style in/out I/O space
** ----------------------------------------------------------------------------
**	The io_in.. / io_out.. functions generate the necessary hardware
**	actions to do a read/write cycle in a cards I/O space. (Registers
**	accessed via the in.. / out.. instructions on i386 architecture.)
*/
#ifdef __KGI_SYS_IO_HAS_IO


#define	IO_NULL	((io_vaddr_t) 0)	/* an invalid virtual address	*/
#define	IO_DECODE_ALL ((io_paddr_t) -1UL) /* all lines being decoded	*/

typedef __kgi_u_t	io_paddr_t;	/* the physical address type	*/
typedef __kgi_u_t	io_vaddr_t;	/* the virtual address type	*/
typedef __kgi_u_t	io_baddr_t;	/* the bus address type		*/

__KGI_SYS_IO(io)

#endif	/* #ifdef __KGI_SYS_IO_HAS_IO	*/

/* ----------------------------------------------------------------------------
**	memory I/O space
** ----------------------------------------------------------------------------
**	This is the 'normal' shared memory I/O space accessed using the
**	mov instructions on i386 architecture. The difference between
**	*vaddr = val and mem_out32(val, vaddr) is that the latter will not
**	be optimized away when compiling with maximum optimization.
*/
#ifdef __KGI_SYS_IO_HAS_MEM


#define	MEM_NULL	((mem_vaddr_t) 0) /* an invalid virtual address	*/
#define	MEM_DECODE_ALL	((mem_paddr_t) -1UL) /* all lines being decoded	*/

typedef __kgi_phys_addr_t mem_paddr_t;	/* the physical address type	*/
typedef __kgi_virt_addr_t mem_vaddr_t;	/* the virtual address type	*/
typedef	__kgi_bus_addr_t  mem_baddr_t;	/* the bus address type		*/

__KGI_SYS_IO(mem)


extern void mem_put8 (const mem_vaddr_t dst, const void *buf, __kgi_size_t cnt);
extern void mem_put16(const mem_vaddr_t dst, const void *buf, __kgi_size_t cnt);
extern void mem_put32(const mem_vaddr_t dst, const void *buf, __kgi_size_t cnt);

extern void mem_get8 (void *buf, const mem_vaddr_t src, __kgi_size_t cnt);
extern void mem_get16(void *buf, const mem_vaddr_t src, __kgi_size_t cnt);
extern void mem_get32(void *buf, const mem_vaddr_t src, __kgi_size_t cnt);

#endif	/* #ifdef __KGI_SYS_IO_HAS_MEM	*/

/* ----------------------------------------------------------------------------
**	irq handling
** ----------------------------------------------------------------------------
**	use similar to io regions.
*/
#ifdef __KGI_SYS_IO_HAS_IRQ

#define	IRQ_NULL	((unsigned int) -1)	/* an invalid irq line	*/
#define	IRQ_DECODE_ALL	((irq_mask_t) -1)	/* all lines decoded	*/

typedef __kgi_u_t	irq_mask_t;

typedef enum
{
	IF_SHARED_IRQ	= 0x00000001,	/* line is shared */
	IF_RANDOM_IRQ	= 0x00000010	/* line used as random source */
	
} irq_flags_t;

typedef struct irq_system_s irq_system_t; /* system dependent data */
typedef int irq_handler_fn(void *meta, void *meta_io, irq_system_t *system);

typedef struct
{
	pcicfg_vaddr_t		device;	/* (PCI) device this belongs to	*/
	irq_flags_t		flags;	/* properties			*/
	const __kgi_ascii_t	*name;	/* name of the line		*/
	__kgi_u_t		line;	/* requested IRQ line		*/
	__kgi_u32_t		rid;	/* region unique id for device  */

	void	*meta;			/* handler meta data		*/
	void	*meta_io;		/* handler meta I/O data	*/
	irq_handler_fn	*High, *Low;	/* high/low priority handlers	*/

} irq_line_t;

/*	These may be OS dependent, and have to be supplied by the OS/kernel
**	layer. As they aren't performance critical either, a call doesn't hurt.
*/
extern __kgi_error_t  irq_claim_line(irq_line_t *irq);
extern void irq_free_line(irq_line_t *irq);

#endif	/* #ifdef __KGI_SYS_IO_HAS_IRQ	*/

#undef	__KGI_SYS_IO_HAS_PCICFG
#undef	__KGI_SYS_IO_HAS_IO
#undef	__KGI_SYS_IO_HAS_MEM
#undef	__KGI_SYS_IO_HAS_IRQ

#endif	/* #ifdef _kgi_io_h */
