/* ----------------------------------------------------------------------------
**	3Dlabs PERMEDIA chipset meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: PERMEDIA-bind.c,v $
**	Revision 1.2  2002/09/21 14:35:10  aldot
**	- fix ATC border color reads
**	
**	Revision 1.1.1.1  2001/07/18 14:57:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:53:51  seeger_s
**	- implemented image resources and export
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:15  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#define	__3Dlabs_Permedia
#define	__3Dlabs_Permedia2
#define	__3Dlabs_Permedia2v
#include "chipset/3Dlabs/PERMEDIA.h"
#include "chipset/3Dlabs/PERMEDIA-bind.h"

#define	PGC_CONTROL_BASE	(pgc_io->control.base_virt)
#define	PGC_VGA_BASE		(PGC_CONTROL_BASE + PGC_VGABase + VGA_IO_BASE)
#define	PGC_VGA_DAC		(pgc_io->flags & PGC_IF_VGA_DAC)

/*
**	PERMEDIA specific I/O functions
*/
static inline void pgc_chipset_cs_out32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "CS%.3x <= %.8x", reg, val);
	mem_out32(val, PGC_CONTROL_BASE + PGC_ControlStatusBase + reg);
}

static inline kgi_u32_t pgc_chipset_cs_in32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t reg)
{
	return mem_in32(PGC_CONTROL_BASE + PGC_ControlStatusBase + reg);
}

static inline void pgc_chipset_mc_out32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "MC%.3x <= %.8x", reg, val);
	mem_out32(val, PGC_CONTROL_BASE + PGC_MemoryControlBase + reg);
}

static inline kgi_u32_t pgc_chipset_mc_in32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t reg)
{
	return mem_in32(PGC_CONTROL_BASE + PGC_MemoryControlBase + reg);
}

#if 0
static inline void pgc_chipset_gp_out32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	mem_out32(val, PGC_CONTROL_BASE + PGC_GPFIFOAccessBase + reg);
}

static inline kgi_u32_t pgc_chipset_gp_in32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t reg)
{
	return mem_in32(PGC_CONTROL_BASE + PGC_GPFIFOAccessBase + reg);
}

static inline void pgc_chipset_gpr_out32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t val, kgi_u32_t tag)
{
	mem_out32(val, PGC_CONTROL_BASE + PGC_GPRegisterBase + (tag << 3));
}

static inline kgi_u32_t pgc_chipset_gpr_in32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t tag)
{
	return mem_in32(PGC_CONTROL_BASE + PGC_GPRegisterBase + (tag << 3));
}
#endif

static inline void pgc_chipset_vc_out32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "VC%.3x <= %.8x", reg, val);
	mem_out32(val, PGC_CONTROL_BASE + PGC_VideoControlBase + reg);
}

static inline kgi_u32_t pgc_chipset_vc_in32(pgc_chipset_io_t *pgc_io,
	kgi_u32_t reg)
{
	return mem_in32(PGC_CONTROL_BASE + PGC_VideoControlBase + reg);
}

/*
**	VGA subsystem I/O functions
*/
static inline void pgc_chipset_vga_seq_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <= %.2x", reg, val);
	mem_out8(reg, PGC_VGA_BASE + VGA_SEQ_INDEX);
	mem_out8(val, PGC_VGA_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t pgc_chipset_vga_seq_in8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg)
{
	mem_out8(reg, PGC_VGA_BASE + VGA_SEQ_INDEX);
	return mem_in8(PGC_VGA_BASE + VGA_SEQ_DATA);
}

static inline void pgc_chipset_vga_crt_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <= %.2x", reg, val);
	mem_out8(reg, PGC_VGA_BASE + VGA_CRT_INDEX);
	mem_out8(val, PGC_VGA_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t pgc_chipset_vga_crt_in8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg)
{
	mem_out8(reg, PGC_VGA_BASE + VGA_CRT_INDEX);
	return mem_in8(PGC_VGA_BASE + VGA_CRT_DATA);
}

static inline void pgc_chipset_vga_grc_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <= %.2x", reg, val);
	mem_out8(reg, PGC_VGA_BASE + VGA_GRC_INDEX);
	mem_out8(val, PGC_VGA_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t pgc_chipset_vga_grc_in8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg)
{
	mem_out8(reg, PGC_VGA_BASE + VGA_GRC_INDEX);
	return mem_in8(PGC_VGA_BASE + VGA_GRC_DATA);
}

static inline void pgc_chipset_vga_atc_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <= %.2x", reg, val);
	mem_in8(PGC_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	mem_out8(reg, PGC_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	mem_out8(val, PGC_VGA_BASE + VGA_ATC_DATAw);
	kgi_nanosleep(250);
}

static inline kgi_u8_t pgc_chipset_vga_atc_in8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	mem_in8(PGC_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	mem_out8(reg, PGC_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	val = mem_in8(PGC_VGA_BASE + VGA_ATC_DATAr);
	kgi_nanosleep(250);
	return val;
}

static inline void pgc_chipset_vga_misc_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <= %.2x", val);
	mem_out8(val, PGC_VGA_BASE + VGA_MISCw);
}

static inline kgi_u8_t pgc_chipset_vga_misc_in8(pgc_chipset_io_t *pgc_io)
{
	return mem_in8(PGC_VGA_BASE + VGA_MISCr);
}

static inline void pgc_chipset_vga_fctrl_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <= %.2x", val);
	mem_out8(val, PGC_VGA_BASE + VGA_FCTRLw);
}

static inline kgi_u8_t pgc_chipset_vga_fctrl_in8(pgc_chipset_io_t *pgc_io)
{
	return mem_in8(PGC_VGA_BASE + VGA_FCTRLr);
}

/*
**	DAC subsystem I/O
*/
static const kgi_u_t pgc_vga_dac_register[4] = { 0x08, 0x09, 0x06, 0x07 };

#define	PGC_SET_DAC_ADDR23						  \
	mem_out8(0x05, PGC_VGA_BASE + VGA_SEQ_INDEX);		  \
	mem_out8((mem_in8(PGC_VGA_BASE + VGA_SEQ_DATA) &		  \
		~PGC_SR05_DACAddrMask) | ((reg << 2) & PGC_SR05_DACAddrMask), \
		PGC_VGA_BASE + VGA_SEQ_DATA)

static inline void pgc_chipset_dac_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg < PGC_MAX_DacRegisters);

	if (PGC_VGA_DAC) {

		KRN_DEBUG(3, "dac_vga %.2x <- %.2x", reg, val);
		PGC_SET_DAC_ADDR23;
		mem_out8(val, PGC_VGA_BASE + pgc_vga_dac_register[reg & 3]);

	} else {

		KRN_DEBUG(3, "dac_mem %.2x <- %.2x", reg, val);
		mem_out8(val, PGC_CONTROL_BASE + PGC_DacBase + (reg << 3));
	}
}

static inline kgi_u8_t pgc_chipset_dac_in8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg)
{
	KRN_ASSERT(reg < PGC_MAX_DacRegisters);

	if (PGC_VGA_DAC) {

		register kgi_u8_t val;

		PGC_SET_DAC_ADDR23;
		val = mem_in8(PGC_VGA_BASE + pgc_vga_dac_register[reg & 3]);
		KRN_DEBUG(3, "dac_vga %.2x -> %.2x", reg, val);
		return val;

	} else {

		register kgi_u8_t val;

		val = mem_in8(PGC_CONTROL_BASE + PGC_DacBase + (reg << 3));
		KRN_DEBUG(3, "dac_mem %.2x -> %.2x", reg, val);
		return val;
	}
}

static inline void pgc_chipset_dac_outs8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < PGC_MAX_DacRegisters);

	if (PGC_VGA_DAC) {

		KRN_DEBUG(3, "dac_vga %.2x <- (%i bytes)", reg, cnt);
		PGC_SET_DAC_ADDR23;
		mem_outs8(PGC_VGA_BASE + pgc_vga_dac_register[reg & 3],
			buf, cnt);

	} else {

		KRN_DEBUG(3, "dac_mem %.2x <- (%i bytes)", reg, cnt);
		mem_outs8(PGC_CONTROL_BASE + PGC_DacBase + (reg << 3),
			buf, cnt);
	}
}

static inline void pgc_chipset_dac_ins8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < PGC_MAX_DacRegisters);

	if (PGC_VGA_DAC) {

		KRN_DEBUG(3, "dac_vga %.2x -> (%i bytes)", reg, cnt);
		PGC_SET_DAC_ADDR23;
		mem_ins8(PGC_VGA_BASE + pgc_vga_dac_register[reg & 3],
			buf, cnt);

	} else {

		KRN_DEBUG(3, "dac_mem %.2x -> (%i bytes)", reg, cnt);
		mem_ins8(PGC_CONTROL_BASE + PGC_DacBase + (reg << 3),
			buf, cnt);
	}
}

/*
**	Clock Control
*/
static inline void pgc_chipset_clk_out8(pgc_chipset_io_t *pgc_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg == 0);

	if (PGC_VGA_DAC) {

		KRN_DEBUG(3, "VGA_CLK%.2x <= %.2x", reg, val);
		mem_out8((mem_in8(PGC_VGA_BASE + VGA_MISCr) &
			~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2),
			PGC_VGA_BASE + VGA_MISCw);

	} else {

		register kgi_u32_t CS040 = mem_in32(
			PGC_CONTROL_BASE + PGC_ControlStatusBase + 0x040);

		mem_out32((CS040 & ~PGC_CS040_VClkMask) |
			(val & PGC_CS040_VClkMask),
			PGC_CONTROL_BASE + PGC_ControlStatusBase + 0x040);
	}
}

static inline kgi_u8_t pgc_chipset_clk_in8(pgc_chipset_io_t *pgc_io,
	kgi_u_t reg)
{
	KRN_ASSERT(reg == 0);

	if (PGC_VGA_DAC) {

		return (mem_in8(PGC_VGA_BASE + VGA_MISCr) &
			VGA_MISC_CLOCK_MASK) >> 2;

	} else {

		return mem_in32(PGC_CONTROL_BASE + PGC_ControlStatusBase +
			0x040) & PGC_CS040_VClkMask;
	}
}

/*
**	chipset_init_module()
*/

kgi_error_t pgc_chipset_init_module(pgc_chipset_t *pgc, 
	pgc_chipset_io_t *pgc_io, const kgim_options_t *options)
{
	static const kgi_u32_t pgc_chipset_pcicfg[] =
	{
		PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA2),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA2v),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TVP4010),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TVP4020),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TVP4020A),
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;

	KRN_ASSERT(pgc);
	KRN_ASSERT(pgc_io);
	KRN_ASSERT(options);


	/*	auto-detect/verify the chipset
	*/
	if (pcidev == PCICFG_NULL) {

		if (pcicfg_find_device(&pcidev, pgc_chipset_pcicfg)) {

			KRN_ERROR("No supported device found!");
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}

	/* NOTE	pgc->chipset.memory is initialized after we have gained access 
	** NOTE	to the chipset (because we can't read MC_MemConfig before that).
	*/
	kgim_memset(pgc, 0, sizeof(*pgc));
	pgc->chipset.revision	= KGIM_CHIPSET_REVISION;
	pgc->chipset.mode_size	= sizeof(pgc_chipset_mode_t);
	pgc->chipset.dclk.max	= 200000000 /* Hz */;
	pgc->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	pgc->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(pgc->chipset.vendor_id,
		pgc->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA):
		pgc->flags |= PGC_CF_PERMEDIA;
		kgim_strcpy(pgc->chipset.vendor, "3Dlabs Incorporated");
		kgim_strcpy(pgc->chipset.model,  "PERMEDIA");
		pgc->chipset.maxdots.x = 1536;
		pgc->chipset.maxdots.y = 2048;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TVP4010):
		pgc->flags |= PGC_CF_PERMEDIA;
		kgim_strcpy(pgc->chipset.vendor, "Texas Instruments");
		kgim_strcpy(pgc->chipset.model,  "TVP 4010");
		pgc->chipset.maxdots.x = 1536;
		pgc->chipset.maxdots.y = 2048;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA2):
		pgc->flags |= PGC_CF_PERMEDIA2;
		kgim_strcpy(pgc->chipset.vendor, "3Dlabs Incorporated");
		kgim_strcpy(pgc->chipset.model,	 "PERMEDIA2");
		pgc->chipset.maxdots.x = 2048;
		pgc->chipset.maxdots.y = 2048;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TVP4020):
		pgc->flags |= PGC_CF_PERMEDIA2;
		kgim_strcpy(pgc->chipset.vendor, "Texas Instruments");
		kgim_strcpy(pgc->chipset.model,  "TVP 4020");
		pgc->chipset.maxdots.x = 2048;
		pgc->chipset.maxdots.y = 2048;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_3Dlabs, PCI_DEVICE_ID_PERMEDIA2v):
		pgc->flags |= PGC_CF_PERMEDIA2;
		kgim_strcpy(pgc->chipset.vendor, "3Dlabs Incorporated");
		kgim_strcpy(pgc->chipset.model,	 "PERMEDIA2v");
		pgc->chipset.maxdots.x = 2048;
		pgc->chipset.maxdots.y = 2048;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_TI, PCI_DEVICE_ID_TVP4020A):
		pgc->flags |= PGC_CF_PERMEDIA2;
		kgim_strcpy(pgc->chipset.vendor, "Texas Instruments");
		kgim_strcpy(pgc->chipset.model,  "TVP 4020A");
		pgc->chipset.maxdots.x = 2048;
		pgc->chipset.maxdots.y = 2048;
		break;

	default:
		KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).",
			pgc->chipset.vendor_id, pgc->chipset.device_id);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(1, "  subvendor %.4x, subdevice %4x", subvendor, subdevice);

	/*	save initial PCICFG state
	*/
	pgc->pci.Command	= pcicfg_in32(pcidev + PCI_COMMAND);
	pgc->pci.LatTimer	= pcicfg_in32(pcidev + PCI_LATENCY_TIMER);
	pgc->pci.IntLine	= pcicfg_in32(pcidev + PCI_INTERRUPT_LINE);
	pgc->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	pgc->pci.BaseAddr1	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
	pgc->pci.BaseAddr2	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_2);
	pgc->pci.BaseAddr3	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_3);
	pgc->pci.BaseAddr4	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_4);
	pgc->pci.RomAddr	= pcicfg_in32(pcidev + PCI_ROM_ADDRESS);

	/*	Initialize driver claimed regions and I/O binding.
	*/
	pgc_io->vga.kgim.pcidev = pcidev;

	pgc_io->control.name      = "PERMEDIA control";
	pgc_io->control.device    = pcidev;
	pgc_io->control.base_virt = MEM_NULL;
	pgc_io->control.base_io	  = pgc->pci.BaseAddr0 & ~(PGC_Base0_Size - 1);
	pgc_io->control.size	  = PGC_Base0_Size;
	pgc_io->control.decode	  = MEM_DECODE_ALL;

	pgc_io->aperture1.name    = "PERMEDIA aperture1";
	pgc_io->aperture1.device  = pcidev;
	pgc_io->aperture1.base_virt = MEM_NULL;
	pgc_io->aperture1.base_io = pgc->pci.BaseAddr1 & ~(PGC_Base1_Size - 1);
	pgc_io->aperture1.size	  = PGC_Base1_Size;
	pgc_io->aperture1.decode  = MEM_DECODE_ALL;

	pgc_io->aperture2.name     = "PERMEDIA aperture2";
	pgc_io->aperture2.device   = pcidev;
	pgc_io->aperture2.base_virt= MEM_NULL;
	pgc_io->aperture2.base_io  = pgc->pci.BaseAddr2 & ~(PGC_Base2_Size - 1);
	pgc_io->aperture2.size     = PGC_Base2_Size;
	pgc_io->aperture2.decode   = MEM_DECODE_ALL;

	pgc_io->auxiliary.name     = "PERMEDIA auxiliary";
	pgc_io->auxiliary.device   = pcidev;
	pgc_io->auxiliary.base_virt= IO_NULL;
	pgc_io->auxiliary.base_io  = pgc->pci.BaseAddr3 & ~(PGC_Base3_Size - 1);
	pgc_io->auxiliary.size	   = PGC_Base3_Size;
	pgc_io->auxiliary.decode   = IO_DECODE_ALL;

	pgc_io->delta.name	= "PERMEDIA delta";
	pgc_io->delta.device	= pcidev;
	pgc_io->delta.base_virt	= MEM_NULL;
	pgc_io->delta.base_io	= pgc->pci.BaseAddr4 & ~(PGC_Base4_Size - 1);
	pgc_io->delta.size	= PGC_Base4_Size;
	pgc_io->delta.decode	= MEM_DECODE_ALL;

	pgc_io->rom.name	= "PERMEDIA BIOS";
	pgc_io->rom.device	= pcidev;
	pgc_io->rom.base_virt	= MEM_NULL;
	pgc_io->rom.base_io	= pgc->pci.RomAddr & ~(PGC_ROM_Size - 1);
	pgc_io->rom.size	= PGC_ROM_Size;
	pgc_io->rom.decode	= MEM_DECODE_ALL;

	pgc_io->irq.flags = IF_SHARED_IRQ;
	pgc_io->irq.name = "PERMEDIA interrupt line";
	pgc_io->irq.line = pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
	pgc_io->irq.meta = pgc;
	pgc_io->irq.meta_io = pgc_io;
	pgc_io->irq.High = (irq_handler_fn *) pgc_chipset_irq_handler;

	pgc_io->vga.ports.name		= "PERMEDIA VGA";
	pgc_io->vga.ports.device	= pcidev;
	pgc_io->vga.ports.base_virt	= IO_NULL;
	pgc_io->vga.ports.base_io	= VGA_IO_BASE;
	pgc_io->vga.ports.size		= VGA_IO_SIZE;

	/*	make sure no other driver is serving the chipset
	*/
	if (pgc->pci.Command & PCI_COMMAND_IO) {

		if (io_check_region(&pgc_io->vga.ports)) {

			KRN_ERROR("%s region served (maybe another driver?).",
				pgc_io->vga.ports.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}
	if (pgc->pci.Command & PCI_COMMAND_MEMORY) {

		if (mem_check_region(&pgc_io->control)) {

			KRN_ERROR("%s region already served!",
				pgc_io->control.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	/*	If root specified new base addresses, he knows the
	**	consequences. If not, it's not our fault...
	*/
#	define	SET_BASE(region, addr, size)			\
		if (addr) {					\
			region.base_io = addr & ~(size - 1);	\
		}

		SET_BASE(pgc_io->control,   options->pci->base0,PGC_Base0_Size);
		SET_BASE(pgc_io->aperture1, options->pci->base1,PGC_Base1_Size);
		SET_BASE(pgc_io->aperture2, options->pci->base2,PGC_Base2_Size);
		SET_BASE(pgc_io->auxiliary, options->pci->base3,PGC_Base3_Size);
		SET_BASE(pgc_io->delta,     options->pci->base4,PGC_Base4_Size);
		SET_BASE(pgc_io->rom,       options->pci->baseR,PGC_ROM_Size);

#	undef SET_BASE

	/*	make sure the memory regions are free.
	*/
	if (mem_check_region(&pgc_io->control) ||
		mem_check_region(&pgc_io->aperture1) ||
		mem_check_region(&pgc_io->aperture2) ||
		mem_check_region(&pgc_io->rom) ||

		((pgc->flags & PGC_CF_PERMEDIA) && (
		(pgc_io->auxiliary.base_io && 
			io_check_region(&pgc_io->auxiliary)) ||
		(pgc_io->delta.base_io && mem_check_region(&pgc_io->delta))
		))) {

		KRN_ERROR("check of PERMEDIA io and memory regions failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	claim the regions
	*/
	mem_claim_region(&pgc_io->control);
	mem_claim_region(&pgc_io->aperture1);
	mem_claim_region(&pgc_io->aperture2);
	mem_claim_region(&pgc_io->rom);

	if (pgc->flags & PGC_CF_PERMEDIA) {

		io_claim_region(&pgc_io->auxiliary);
		mem_claim_region(&pgc_io->delta);
	}

	if (KGI_EOK == irq_claim_line(&pgc_io->irq)) {

		KRN_DEBUG(1, "interrupt line claimed successfully");
		pgc->flags |= PGC_CF_IRQ_CLAIMED;
	}

	/*	aperture1 serves as vga aperture in MMIO-only mode
	*/
	pgc_io->vga.aperture = pgc_io->aperture1;

	pgc_io->vga.kgim.DacOut8  = (void *) pgc_chipset_dac_out8;
	pgc_io->vga.kgim.DacIn8   = (void *) pgc_chipset_dac_in8;
	pgc_io->vga.kgim.DacOuts8 = (void *) pgc_chipset_dac_outs8;
	pgc_io->vga.kgim.DacIns8  = (void *) pgc_chipset_dac_ins8;

	pgc_io->vga.kgim.ClkOut8  = (void *) pgc_chipset_clk_out8;
	pgc_io->vga.kgim.ClkIn8   = (void *) pgc_chipset_clk_in8;
	
	pgc_io->vga.SeqOut8   = (void *) pgc_chipset_vga_seq_out8;
	pgc_io->vga.SeqIn8    = (void *) pgc_chipset_vga_seq_in8;
	pgc_io->vga.CrtOut8   = (void *) pgc_chipset_vga_crt_out8;
	pgc_io->vga.CrtIn8    = (void *) pgc_chipset_vga_crt_in8;
	pgc_io->vga.GrcOut8   = (void *) pgc_chipset_vga_grc_out8;
	pgc_io->vga.GrcIn8    = (void *) pgc_chipset_vga_grc_in8;
	pgc_io->vga.AtcOut8   = (void *) pgc_chipset_vga_atc_out8;
	pgc_io->vga.AtcIn8    = (void *) pgc_chipset_vga_atc_in8;
	pgc_io->vga.MiscOut8  = (void *) pgc_chipset_vga_misc_out8;
	pgc_io->vga.MiscIn8   = (void *) pgc_chipset_vga_misc_in8;
	pgc_io->vga.FctrlOut8 = (void *) pgc_chipset_vga_fctrl_out8;
	pgc_io->vga.FctrlIn8  = (void *) pgc_chipset_vga_fctrl_in8;

	pgc_io->CSOut32	= (void *) pgc_chipset_cs_out32;
	pgc_io->CSIn32	= (void *) pgc_chipset_cs_in32;
	pgc_io->MCOut32	= (void *) pgc_chipset_mc_out32;
	pgc_io->MCIn32	= (void *) pgc_chipset_mc_in32;
	pgc_io->VCOut32	= (void *) pgc_chipset_vc_out32;
	pgc_io->VCIn32	= (void *) pgc_chipset_vc_in32;

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		pgc->chipset.vendor, pgc->chipset.model);
	return KGI_EOK;
}

void pgc_chipset_done_module(pgc_chipset_t *pgc, 
	pgc_chipset_io_t *pgc_io, const kgim_options_t *options)
{
	if (pgc->flags & PGC_CF_IRQ_CLAIMED) {

		irq_free_line(&pgc_io->irq);
	}

	if (pgc->flags & PGC_CF_PERMEDIA) {

		mem_free_region(&pgc_io->delta);
		io_free_region(&pgc_io->auxiliary);
	}
	mem_free_region(&pgc_io->rom);
	mem_free_region(&pgc_io->aperture2);
	mem_free_region(&pgc_io->aperture1);
	mem_free_region(&pgc_io->control);
}

const kgim_meta_t pgc_chipset_meta =
{
	(kgim_meta_init_module_fn *)	pgc_chipset_init_module,
	(kgim_meta_done_module_fn *)	pgc_chipset_done_module,
	(kgim_meta_init_fn *)		pgc_chipset_init,
	(kgim_meta_done_fn *)		pgc_chipset_done,
	(kgim_meta_mode_check_fn *)	pgc_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	pgc_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	pgc_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	pgc_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	pgc_chipset_mode_leave,
        (kgim_meta_image_resource_fn *)	pgc_chipset_image_resource,

	sizeof(pgc_chipset_t),
	sizeof(pgc_chipset_io_t),
	sizeof(pgc_chipset_mode_t)
};
