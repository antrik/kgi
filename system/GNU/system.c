/* ---------------------------------------------------------------------------
**	KGI system layer
** ---------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger 
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**	Alternatively you may distribute this file under the terms and
**	conditions of the GNU General Public License. Please see the file 
**	COPYRIGHT.GPL included with this software for details of these terms
**	and conditions.
** ---------------------------------------------------------------------------
**
**	$Log: system.c,v $
**	Revision 1.5  2002/10/15 00:39:22  aldot
**	- remove pre-2.4
**	- add linux-2.4.19 support
**	- include cleanup
**	
**	Revision 1.4  2002/09/21 14:28:40  aldot
**	- debugging fixes
**	
**	Revision 1.3  2002/07/27 01:11:52  aldot
**	- whitespace cleanup
**	
**	Revision 1.2  2002/06/12 00:52:15  aldot
**	- add pcicfg_find_class needed for VGA driver
**	
**	Revision 1.1.1.1  2001/11/18 00:59:36  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/11/17 21:48:29  seeger_s
**	- typo fixed
**	
**	Revision 1.2  2001/09/09 23:33:27  skids
**	
**	Updated PCI bus traversal when compiling on 2.4.x series kernels
**	
**	Revision 1.1.1.1  2000/04/18 08:50:53  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define DRIVER_REV	"$Revision: 1.5 $"

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#define __KGIM_HAS_STDARGS
#define PRINT(...) fprintf(stderr, __VA_ARGS__)
#include <stdarg.h>
#include <stdio.h>
#define KERN_ERR "KGI error at "
#define KERN_NOTICE "KGI notice: "

#include <pci/pci.h>

#include "kgi/config.h"

#define	KGI_SYS_NEED_IO
#include <kgi/kgii.h>
#include <kgi/system.h>

#define	EOK 0
#define	__KRN_BUF_SIZE	1024

#ifdef	KRN_DEBUG_ANSI_CPP
void __krn_ansi_debug(int level, const char *fmt, ...)
{
	char buf[__KRN_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT("<%i>%s\n", level, buf);
}

void __krn_ansi_error(const char *fmt, ...)
{
	char buf[__KRN_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT("%s\n", buf);
}
#endif	/* #ifdef KRN_DEBUG_ANSI_CPP	*/

#ifdef KRN_DEBUG_GNU_CPP
void __krn_gnu_debug(const char *file, int line, const char *func, int level,
	const char *fmt, ...)
{
	char buf[__KRN_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsnprintf(buf, __KRN_BUF_SIZE, fmt, args);
	va_end(args);
	PRINT("<%i>%s:%s:%i:D:%s\n", level, file, func, line, buf);
}

void __krn_gnu_error(const char *file, int line, const char *func, 
	const char *fmt, ...)
{
	char buf[__KRN_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsnprintf(buf, __KRN_BUF_SIZE, fmt, args);
	va_end(args);
	PRINT(KERN_ERR "%s:%s:%i:E:%s\n", file, func, line, buf);
}
#endif	/* #ifdef KRN_DEBUG_GNU_CPP	*/

void __krn_notice(const char *fmt, ...)
{
	char buf[__KRN_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsnprintf(buf, __KRN_BUF_SIZE, fmt, args);
	va_end(args);
	PRINT(KERN_NOTICE "%s\n", buf);
}


#if 0

/*
**	I/O stuff
*/

#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

/*
**	io I/O space
*/
__kgi_error_t io_check_region(io_region_t *r)
{
	register int foo = check_region(r->base_io, r->size);

	KRN_DEBUG(2, "io_check_region('%s', base %i, size %i): %s",
		r->name, r->base_io, r->size, foo ? "not free" : "free");
	return foo;
}

io_vaddr_t io_claim_region(io_region_t *r)
{
	KRN_ASSERT(r->base_virt == IO_NULL); /* claim of claimed region? */

	request_region(r->base_io, r->size, r->name);
	r->base_virt = r->base_phys = r->base_bus = r->base_io;

	KRN_DEBUG(2, "io_claim_region('%s', base_io %.4x, base_virt %.4x, "
		"base_phys %.4x, base_bus %.4x)", r->name,
		r->base_io, r->base_virt, r->base_phys, r->base_bus);

	return r->base_virt;
}

io_vaddr_t io_free_region(io_region_t *r)
{
	KRN_DEBUG(2, "io_free_region('%s', base_io %.4x, base_virt %.4x, "
		"base_phys %.4x, base_bus %.4x)", r->name,
		r->base_io, r->base_virt, r->base_phys, r->base_bus);
	r->base_virt = IO_NULL;
	r->base_phys = r->base_bus = 0;
	release_region(r->base_io, r->size);
	return IO_NULL;
}

io_vaddr_t io_alloc_region(io_region_t *r)
{
	/*	AFAIK nothing appropriate under Linux yet.
	*/
	return IO_NULL;
}

#endif

/*
**	memory I/O space
*/

#include "hurd_video.h"

__kgi_error_t mem_check_region(mem_region_t *r)
{
	/*	AFAIK Linux has no resource management for memory regions yet.
	**	We just rely on the BIOS doing a good job.
	*/
	KRN_DEBUG(2, "mem_check_region('%s', base %p, size %i) always OK", 
		r->name, (void *) r->base_virt, r->size);
	return EOK;
}

mem_vaddr_t mem_claim_region(mem_region_t *r)
{
	KRN_ASSERT(r->base_virt == NULL); /* claim of claimed region? */

	r->base_virt = mmio_map((void *)r->base_io, r->size);
#ifndef	__i386__
#	warning	Are we i386 specific here? CHECK THIS CODE!
#endif
	r->base_phys = (mem_paddr_t) r->base_io;
	r->base_bus  = (mem_baddr_t) r->base_io;
	KRN_DEBUG(2, "mem_region('%s', base_io %p, base_virt %p, "
		"base_phys %p, base_bus %p)", r->name, 
		(void *) r->base_io, (void *) r->base_virt, 
		(void *) r->base_phys, (void *) r->base_bus);

	return r->base_virt;
}

mem_vaddr_t mem_free_region(mem_region_t *r)
{
	KRN_ASSERT(r->base_virt != NULL);

	mmio_unmap(r->base_virt, r->size);
	KRN_DEBUG(2, "mem_free_region %s, base_io %p, base_virt %p, "
		"base_phys %p, base_bus %p", r->name, (void *) r->base_io,
		r->base_virt, (void *) r->base_phys, (void *) r->base_bus);

	r->base_bus = (mem_baddr_t) 0L;
	r->base_phys = (mem_paddr_t) 0L;
	return (r->base_virt = NULL);
}

mem_vaddr_t mem_alloc_region(mem_region_t *r)
{
	/*	AFAIK there is no resource management for memory regions
	**	in Linux yet. We just report it failed.
	*/
	return NULL;
}


/*
**	PCI configuration space
*/

static struct pci_access *pacc=NULL;

void init_libpci(void)
{
	if(pacc)
		return;

	pacc = pci_alloc();		/* Get the pci_access structure */
	/* Set all options you want -- here we stick with the defaults */
	pci_init(pacc);		/* Initialize the PCI library */
	pci_scan_bus(pacc);		/* We want to get the list of devices */
}

int pcicfg_find_device(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures)
{
	struct pci_dev *dev = NULL;

	KRN_DEBUG(2, "scanning pcicfg space:");

	if (PCICFG_NULL == *addr) {
		KRN_DEBUG(2,"scanning all devices");
	} else {
KRN_DEBUG(2,"starting at %.8x", *addr);

	}

	init_libpci();

	for(dev=pacc->devices; dev; dev=dev->next) {	/* Iterate over all devices */

		const __kgi_u32_t *check = signatures;
		__kgi_u32_t signature = 
			PCICFG_SIGNATURE(dev->vendor_id, dev->device_id);

		KRN_DEBUG(2, "scanning device %x %x", 
			dev->vendor_id, dev->device_id);

		while (*check && (*check != signature)) {

			check++;
		}

		if (*check && (*check == signature)) {

			*addr = PCICFG_VADDR(dev->bus, dev->dev, dev->func);
			KRN_DEBUG(1, "found device %.8x at %.8x", 
				signature, *addr);
			return 0;
		}
	}
	*addr = PCICFG_NULL;
	return 1;
}

int pcicfg_find_subsystem(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures)
{
	struct pci_dev *dev = NULL;

	KRN_DEBUG(2, "scanning pcicfg space:");

	if (PCICFG_NULL == *addr) {
		KRN_DEBUG(2,"scanning all devices");
	} else {
KRN_DEBUG(2,"starting at %.8x", *addr);

	}

	init_libpci();

	for(dev=pacc->devices; dev; dev=dev->next) {	/* Iterate over all devices */

		const __kgi_u32_t *check = signatures;
		__kgi_u16_t subvendor = pci_read_word(dev, PCI_SUBSYSTEM_VENDOR_ID);
		__kgi_u16_t subdevice = pci_read_word(dev, PCI_SUBSYSTEM_ID);
		__kgi_u32_t signature = PCICFG_SIGNATURE(subvendor, subdevice);

		KRN_DEBUG(1, "scanning device %x %x, subsystem %x %x", 
			dev->vendor_id, dev->device_id, subvendor, subdevice);

		while (*check && (*check != signature)) {

			check++;
		}

		if (*check && (*check == signature)) {

			*addr = PCICFG_VADDR(dev->bus, dev->dev, dev->func);
			KRN_DEBUG(1, "found device %.8x at %.8x", 
				signature, *addr);
			return 0;
		}
	}
	*addr = PCICFG_NULL;
	return 1;
}

int pcicfg_find_class(pcicfg_vaddr_t *addr, const __kgi_u32_t *signatures)
{
	struct pci_dev *dev = NULL;

	KRN_DEBUG(2, "scanning pcicfg space:");

	if (PCICFG_NULL == *addr) {
		KRN_DEBUG(2,"scanning all devices");
	} else {
KRN_DEBUG(2,"starting at %.8lx", *addr);

	}

	init_libpci();

	for(dev=pacc->devices; dev; dev=dev->next) {	/* Iterate over all devices */

		const __kgi_u32_t *check = signatures;
		__kgi_u32_t signature = dev->device_class;

		KRN_DEBUG(2, "scanning device with class %.8x", signature);

		while (*check && (*check != signature)) {

			check++;
		}

		if (*check && (*check == signature)) {
			*addr = PCICFG_VADDR(dev->bus, dev->dev, dev->func);
			KRN_DEBUG(1, "found device %02x:%02x.%d of class %.8x "
					"at %.8lx",
				dev->bus, dev->dev, dev->func,
				signature, *addr);
			return 0;
		}
	}
	*addr = PCICFG_NULL;
	return 1;
}


#define PCIDEV(vaddr) pci_get_dev(pacc, 0 /*domain*/, PCICFG_BUS(vaddr), PCICFG_DEV(vaddr), PCICFG_FN(vaddr))

#define PCICFG_IN(bits, size) \
	__kgi_u##bits##_t pcicfg_in##bits(const pcicfg_vaddr_t vaddr) \
	{ \
		struct pci_dev *dev = PCIDEV(vaddr); \
		int pos = vaddr & 0xFF; \
		__kgi_u##bits##_t retval = pci_read_##size(dev, pos); \
		pci_free_dev(dev); \
		return retval; \
	}

#define PCICFG_OUT(bits, size) \
	void pcicfg_out##bits(const __kgi_u##bits##_t val, const pcicfg_vaddr_t vaddr) \
	{ \
		struct pci_dev *dev = PCIDEV(vaddr); \
		int pos = vaddr & 0xFF; \
		pci_write_##size(dev, pos, val); \
		pci_free_dev(dev); \
	}

PCICFG_IN(8, byte);
PCICFG_IN(16, word);
PCICFG_IN(32, long);

PCICFG_OUT(8, byte);
PCICFG_OUT(16, word);
PCICFG_OUT(32, long);

#if 0

/*
**	irq handling
*/
static void irq_handler(int irqno, void *priv, struct pt_regs *regs)
{
	register irq_line_t *irq = (irq_line_t *) priv;

	if (irq->High) {

		irq->High(irq->meta, irq->meta_io, (void *) regs);
	}
}

__kgi_error_t irq_claim_line(irq_line_t *irq)
{
	unsigned int flags = SA_INTERRUPT;
	int result;

	if (irq->flags & IF_SHARED_IRQ) {

		flags |= SA_SHIRQ;
	}
	result = request_irq(irq->line, irq_handler, flags, irq->name,
			(void *) irq);
	KRN_DEBUG(2, "irq_claim_line('%s', line %i) %s", irq->name,
		irq->line, result ? "failed" : "success");
	return result;
}

void irq_free_line(irq_line_t *irq)
{
	free_irq(irq->line, irq);
	KRN_DEBUG(2, "irq_free_line('%s', line %i)", irq->name, irq->line);
}

#endif

#include "kgi/errno.h"

__kgi_error_t irq_claim_line(irq_line_t *irq)
{
	return KGI_NOT_IMPLEMENTED;
}

void irq_free_line(irq_line_t *irq)
{
}
