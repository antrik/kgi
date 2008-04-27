/* ----------------------------------------------------------------------------
**	ATI MACH64 chipset binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64-bind.c,v $
**	Revision 1.6  2003/07/26 18:45:12  cegger
**	merge improvements from the FreeBSD guys:
**	- improves OS independency
**	- add support for new MACH64 chip variants (GX and LM)
**	- bug fixes
**	
**	Revision 1.5  2002/09/21 14:35:10  aldot
**	- fix ATC border color reads
**	
**	Revision 1.4  2002/06/12 14:21:09  fspacek
**	PCI ids for Rage Pro, beginings of a cursor and lots of register definitions
**	
**	Revision 1.3  2002/06/07 01:24:35  fspacek
**	Export VGA-text resources, fixup of some register definition, implement
**	DAC io, and beginings of accelerator context handling
**	
**	Revision 1.2  2002/06/01 23:02:06  fspacek
**	Some reformating and PCI ids for Rage XL/XC
**	
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:34  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:16:33  phil
**	Automatically generated ATI chipset code
**	
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Filip_Spacek
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.6 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/IBM/VGA.h"
#include "chipset/ATI/MACH64.h"
#include "chipset/ATI/MACH64-bind.h"

/*
**	mach64 specific I/O functions
*/
#define	MACH64_REG_BASE                                           \
(mach64_io->mem.base_virt +                                       \
 ((mach64_io->flags & MACH64_IF_8M_APP) ? 0x3ffc00 : 0x7ffc00))

static inline void mach64_chipset_ctrl_out8(mach64_chipset_io_t *mach64_io,
					    kgi_u8_t val, kgi_s_t reg)
{
	mem_out8(val, MACH64_REG_BASE + reg);
}

static inline kgi_u32_t mach64_chipset_ctrl_in8(mach64_chipset_io_t* mach64_io,
						kgi_s_t reg)
{
	return mem_in8(MACH64_REG_BASE + reg);
}

static inline void mach64_chipset_ctrl_out32(mach64_chipset_io_t *mach64_io,
					     kgi_u32_t val, kgi_s_t reg)
{
	mem_out32(val, MACH64_REG_BASE + reg);
	KRN_DEBUG(4,"Ctrl MM_%.2X (%d) (at %.8x or %.8x) <= %.8x",
		  reg>>2, reg, MACH64_REG_BASE + reg, 0x7ffc00 + reg, val);
}

static inline kgi_u32_t mach64_chipset_ctrl_in32(mach64_chipset_io_t*mach64_io,
						 kgi_s_t reg)
{
	kgi_u32_t val;
	val = mem_in32(MACH64_REG_BASE + reg);
	KRN_DEBUG(4, "CtrlIn32 MM_%.2X (at 0x%.8x or 0x%.8x) = 0x%.8x",
		  reg>>2, MACH64_REG_BASE + reg,  0x7ffc00 + reg, val);
	return val;
}

static inline void mach64_chipset_clock_out8(mach64_chipset_io_t *mach64_io,
					     kgi_u8_t val, kgi_u_t reg)
{
	// Write a clock register (4.2.5 in MACH64 register reference)
	KRN_DEBUG(4, "ClockOut 0x%.2x <= 0x%.2x", reg, val);
	mem_out8((reg << 2) | 2, MACH64_REG_BASE + MACH64_CLOCK_CNTL+1);
	mem_out8(val, MACH64_REG_BASE + MACH64_CLOCK_CNTL+2);
}

static inline kgi_u8_t mach64_chipset_clock_in8(mach64_chipset_io_t *mach64_io,
						kgi_u_t reg)
{
	kgi_u8_t val;
    
	mem_out8((reg << 2), MACH64_REG_BASE + MACH64_CLOCK_CNTL + 1);
	val = mem_in8(MACH64_REG_BASE + MACH64_CLOCK_CNTL + 2);
	KRN_DEBUG(4, "ClockIn 0x%.2x = 0x%.2x", reg, val);
	return val;
}

/*
**	VGA subsystem I/O functions
*/
#define	MACH64_VGA_DAC		(mach64_io->flags & MACH64_IF_VGA_DAC)
#define MACH64_VGA_BASE         0x3c0

static inline void mach64_chipset_vga_seq_out8(mach64_chipset_io_t *mach64_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(4, "VGA_SEQ%.2x <= %.2x", reg, val);
	io_out8(reg, MACH64_VGA_BASE + VGA_SEQ_INDEX);
	io_out8(val, MACH64_VGA_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t mach64_chipset_vga_seq_in8(mach64_chipset_io_t *mach64_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_out8(reg, MACH64_VGA_BASE + VGA_SEQ_INDEX);
	val = io_in8(MACH64_VGA_BASE + VGA_SEQ_DATA);
	KRN_DEBUG(4, "VGA_SEQ%.2x => %.2x", reg, val);
	return val;
}

static inline void mach64_chipset_vga_crt_out8(mach64_chipset_io_t *mach64_io,
					       kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(4, "VGA_CRT%.2x <= %.2x", reg, val);
	io_out8(reg, MACH64_VGA_BASE + VGA_CRT_INDEX);
	io_out8(val, MACH64_VGA_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t mach64_chipset_vga_crt_in8(mach64_chipset_io_t *mach64_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_out8(reg, MACH64_VGA_BASE + VGA_CRT_INDEX);
	val = io_in8(MACH64_VGA_BASE + VGA_CRT_DATA);
	KRN_DEBUG(4, "VGA_CRT%.2x => %.2x", reg, val);
	return val;
}

static inline void mach64_chipset_vga_grc_out8(mach64_chipset_io_t *mach64_io,
					       kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(4, "VGA_GRC%.2x <= %.2x", reg, val);
	io_out8(reg, MACH64_VGA_BASE + VGA_GRC_INDEX);
	io_out8(val, MACH64_VGA_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t mach64_chipset_vga_grc_in8(mach64_chipset_io_t *mach64_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_out8(reg, MACH64_VGA_BASE + VGA_GRC_INDEX);
	val = io_in8(MACH64_VGA_BASE + VGA_GRC_DATA);
	KRN_DEBUG(4, "VGA_CRT%.2x => %.2x", reg, val);
	return val;
}

static inline void mach64_chipset_vga_atc_out8(mach64_chipset_io_t *mach64_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(4, "VGA_ATC%.2x <= %.2x", reg, val);
	io_in8(MACH64_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, MACH64_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	io_out8(val, MACH64_VGA_BASE + VGA_ATC_DATAw);
	kgi_nanosleep(250);
}

static inline kgi_u8_t mach64_chipset_vga_atc_in8(mach64_chipset_io_t *mach64_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_in8(MACH64_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, MACH64_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	val = io_in8(MACH64_VGA_BASE + VGA_ATC_DATAr);
	kgi_nanosleep(250);
	KRN_DEBUG(4, "VGA_ATC%.2x => %.2x", reg, val);
	return val;
}

static inline void mach64_chipset_vga_misc_out8(mach64_chipset_io_t *mach64_io,
	kgi_u8_t val)
{
	KRN_DEBUG(4, "VGA_MISC <= %.2x", val);
	io_out8(val, MACH64_VGA_BASE + VGA_MISCw);
}

static inline kgi_u8_t mach64_chipset_vga_misc_in8(mach64_chipset_io_t *mach64_io)
{
	register kgi_u8_t val;
	val = io_in8(MACH64_VGA_BASE + VGA_MISCr);
	KRN_DEBUG(4, "VGA_MISC => %.2x", val);
	return val;
}

static inline void mach64_chipset_vga_fctrl_out8(mach64_chipset_io_t *mach64_io,
						 kgi_u8_t val)
{
	KRN_DEBUG(4, "VGA_FCTRL <= %.2x", val);
	io_out8(val, MACH64_VGA_BASE + VGA_FCTRLw );
}

static inline kgi_u8_t mach64_chipset_vga_fctrl_in8(mach64_chipset_io_t *mach64_io)
{
	register kgi_u8_t val;
	val = io_in8(MACH64_VGA_BASE + VGA_FCTRLr);
	KRN_DEBUG(4, "VGA_FCTL => %.2x", val);
	return val;
}

/*
**	DAC subsystem I/O
*/
static const kgi_u_t mach64_vga_dac_reg[4] = { 0x08, 0x09, 0x06, 0x07 };

static inline void mach64_chipset_dac_out8(mach64_chipset_io_t *mach64_io,
	kgi_u8_t val, kgi_u_t reg)
{
	if (MACH64_VGA_DAC) {
	
		KRN_DEBUG(4, "DAC_VGA_OUT8 0x%.2x <= 0x%.2x", reg, val);
		io_out8(val, MACH64_VGA_BASE + mach64_vga_dac_reg[reg]);
	}
	else {
	
		KRN_DEBUG(4, "DAC_MACH_OUT8 0x%.2x <= 0x%.2x", reg, val);
		mem_out8(val, MACH64_REG_BASE + MACH64_DAC_REGS + reg);
	}
}

static inline kgi_u8_t mach64_chipset_dac_in8(mach64_chipset_io_t *mach64_io,
	kgi_u_t reg)
{
	kgi_u8_t val;

	if (MACH64_VGA_DAC) {
	
		val = io_in8(MACH64_VGA_BASE + mach64_vga_dac_reg[reg]);
	}
	else {
	
		val = mem_in8(MACH64_REG_BASE + MACH64_DAC_REGS + reg);
	}

	KRN_DEBUG(4, "DAC%.2x => %.2x", reg, val);
	return val;
}

static inline void mach64_chipset_dac_outs8(mach64_chipset_io_t *mach64_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	if (MACH64_VGA_DAC) {
	
		KRN_DEBUG(4, "DAC_VGA_OUTS8 0x%.2x <= (%d bytes)", reg, cnt);
		io_outs8(MACH64_VGA_BASE + mach64_vga_dac_reg[reg], buf, cnt);
	}
	else {
	
		KRN_DEBUG(4, "DAC_MACH_OUTS8, 0x%.2x <= (%d bytes)", reg, cnt);
		mem_outs8(MACH64_REG_BASE + MACH64_DAC_REGS + reg, buf, cnt);
	}
}

static inline void mach64_chipset_dac_ins8(mach64_chipset_io_t *mach64_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	if (MACH64_VGA_DAC) {
	
		KRN_DEBUG(4, "DAC_VGA_INS8 0x%.2x <= (%d bytes)", reg, cnt);
		io_ins8(MACH64_VGA_BASE + mach64_vga_dac_reg[reg], buf, cnt);
	}
	else {
	
		KRN_DEBUG(4, "DAC_MACH_INS8, 0x%.2x <= (%d bytes)", reg, cnt);
		mem_ins8(MACH64_REG_BASE + MACH64_DAC_REGS + reg, buf, cnt);
	}
}

/*
** Clock Control
*/
static inline void mach64_chipset_clk_out8(mach64_chipset_io_t *mach64_io,
	kgi_u8_t val, kgi_u_t reg)
{
	if(MACH64_VGA_DAC){
		KRN_DEBUG(4, "VGA_CLK%.2x <= %.2x", reg, val);
		io_out8((io_in8(MACH64_VGA_BASE + VGA_MISCr) &
			 ~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2),
			MACH64_VGA_BASE + VGA_MISCw);
	}
	else {
		kgi_u8_t new_val = mem_in8(MACH64_REG_BASE+MACH64_CLOCK_CNTL);
		new_val &= ~MACH64_24_CLOCK_SELMask;
		new_val |= (val & MACH64_24_CLOCK_SELMask) <<
			MACH64_24_CLOCK_SELShift;
		mem_out8(new_val, MACH64_REG_BASE + MACH64_CLOCK_CNTL);
	}
}

static inline kgi_u8_t mach64_chipset_clk_in8(mach64_chipset_io_t *mach64_io,
	kgi_u_t reg)
{
	if(MACH64_VGA_DAC){
		return (io_in8(MACH64_VGA_BASE + VGA_MISCr) &
			VGA_MISC_CLOCK_MASK) >> 2;
	}
	else {
		return (mem_in8(MACH64_REG_BASE + MACH64_CLOCK_CNTL) &
			MACH64_24_CLOCK_SELMask) >> MACH64_24_CLOCK_SELShift;
	}
		
}

/*
**	chipset_init_module()
*/

kgi_error_t mach64_chipset_init_module(mach64_chipset_t *mach64, 
	mach64_chipset_io_t *mach64_io, const kgim_options_t *options)
{
	static const kgi_u32_t mach64_chipset_pcicfg[] = {
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GX),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64CT),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT2C),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GB),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GD),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GI),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GP),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GQ),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GR),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GS),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GO),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GL),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GM),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GN),
	PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64LM),
	PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;
	
	KRN_ASSERT(mach64);
	KRN_ASSERT(mach64_io);
	KRN_ASSERT(options);
	

	/*	auto-detect/verify the chipset
	 */
	if (pcidev == PCICFG_NULL) {
		if (pcicfg_find_device(&pcidev, mach64_chipset_pcicfg)) {
			KRN_ERROR("No supported device found!");
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}
  
	/* 	mach64->chipset.memory is initialized after we have access 
	** 	to the chipset.
	*/
	kgim_memset(mach64, 0, sizeof(*mach64));

	kgim_strcpy(mach64->chipset.vendor, "ATI");
	mach64->chipset.revision	= KGIM_CHIPSET_REVISION;
	mach64->chipset.mode_size	= sizeof(mach64_chipset_mode_t);
	mach64->chipset.dclk.max	= 200000000 /* Hz */;
	mach64->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	mach64->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	/* Dunno if this is a good policy, but as long as there's enough memory
	   we allow any mode (that is as long as it fits into registers) */
	mach64->chipset.maxdots.x = 2048;
	mach64->chipset.maxdots.y = 2048;
  
	switch (PCICFG_SIGNATURE(mach64->chipset.vendor_id,
				 mach64->chipset.device_id)) {
    
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GX):
		kgim_strcpy(mach64->chipset.model, "MACH64GX");
		mach64->chip = MACH64_CHIP_88800GX;
		/* FIXME: how much? */
		mach64->chipset.dclk.max = 100000000;
		break;
		
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64CT):
		kgim_strcpy(mach64->chipset.model, "MACH64CT");
		mach64->chip = MACH64_CHIP_264CT;
		mach64->chipset.dclk.max = 135000000;
		break;
		
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT):
		kgim_strcpy(mach64->chipset.model, "MACH64GT");
		mach64->chip = MACH64_CHIP_264GT;
		mach64->chipset.dclk.max = 135000000;
		break;
		
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GT2C):
		kgim_strcpy(mach64->chipset.model, "MACH64GT2C");
		mach64->chip = MACH64_CHIP_264GT2C;
		mach64->chipset.dclk.max = 135000000;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GB):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GD):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GP):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GQ):
		kgim_strcpy(mach64->chipset.model, "MACH64GTPRO");
		mach64->chip = MACH64_CHIP_264GTPRO;
		mach64->chipset.dclk.max = 135000000;
		break;
		
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GR):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GS):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GO):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GL):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GM):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64GN):
		kgim_strcpy(mach64->chipset.model, "MACH64XL");
		mach64->chip = MACH64_CHIP_264XL;
		mach64->chipset.dclk.max = 135000000;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_MACH64LM):
		kgim_strcpy(mach64->chipset.model, "MACH64LM");
		mach64->chip = MACH64_CHIP_MOBILITY;
		mach64->chipset.dclk.max = 135000000;
		break;

	default:
		KRN_ERROR("Device not yet supported (vendor %.4x device %.4x)",
			  mach64->chipset.vendor_id,mach64->chipset.device_id);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(1, "  vendor %4x, device %4x",
		  mach64->chipset.vendor_id, mach64->chipset.device_id); 
	KRN_DEBUG(1, "  subvendor %.4x, subdevice %4x", subvendor, subdevice);
  
	/*	save initial PCICFG state
	 */
	mach64->pci.Command	= pcicfg_in32(pcidev + PCI_COMMAND);
	mach64->pci.LatTimer	= pcicfg_in32(pcidev + PCI_LATENCY_TIMER);
	mach64->pci.IntLine	= pcicfg_in32(pcidev + PCI_INTERRUPT_LINE);
	mach64->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	mach64->pci.BaseAddr1	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);

	/*	Initialize driver claimed regions and I/O binding.
	 */
	mach64_io->vga.kgim.pcidev = pcidev;
	
	mach64_io->mem.name      = "mach64 mmio aperture";
	mach64_io->mem.device    = pcidev;
	mach64_io->mem.base_virt = MEM_NULL;
	mach64_io->mem.base_io   = mach64->pci.BaseAddr0 &
		~(MACH64_Base0_Size - 1);
	mach64_io->mem.size	 = MACH64_Base0_Size;
	mach64_io->mem.decode	 = MEM_DECODE_ALL;
	mach64_io->mem.rid	 = PCI_BASE_ADDRESS_0;
  
	mach64_io->irq.flags   = IF_SHARED_IRQ;
	mach64_io->irq.name    = "mach64 irq line";
	mach64_io->irq.line    = mach64->pci.IntLine & 0xFF;
	mach64_io->irq.meta    = mach64;
	mach64_io->irq.meta_io = mach64_io;
	mach64_io->irq.High    = (irq_handler_fn *)mach64_chipset_irq_handler;
	mach64_io->irq.rid     = 0;
  
	mach64_io->vga.ports.name      = "MACH64 VGA";
	mach64_io->vga.ports.device    = pcidev;
	mach64_io->vga.ports.base_virt = IO_NULL;
	mach64_io->vga.ports.base_io   = VGA_IO_BASE;
	mach64_io->vga.ports.size      = VGA_IO_SIZE;
	mach64_io->vga.ports.rid       = 1;

	/* Note that most mach64 cards also have 1K of IO mapped registers
	   at base address 1. These are all available on top of the memory
	   mapped aperture so there is no need to use these. Trying to
	   allocate them would only complicate things for the (high-end)
	   cards that do not expose their registers in IO space.

	   mach64_io->io.name      = "mach64 io";
	   mach64_io->io.device    = pcidev;
	   mach64_io->io.base_virt = IO_NULL;
	   mach64_io->io.base_io   = mach64->pci.BaseAddr1 &
	                               ~(MACH64_Base1_Size - 1);
	   mach64_io->io.size	   = MACH64_Base1_Size;
	   mach64_io->io.decode	   = IO_DECODE_ALL;
	   mach64_io->io.rid	   = PCI_BASE_ADDRESS_1;
	*/

	/*	make sure no other driver is serving the chipset
	 */
	if (mach64->pci.Command & PCI_COMMAND_IO) {
		if (io_check_region(&mach64_io->vga.ports)) {
			KRN_ERROR("%s region served (maybe another driver?).",
				  mach64_io->vga.ports.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}
	
	if (mach64->pci.Command & PCI_COMMAND_MEMORY) {
		if (mem_check_region(&mach64_io->mem)) {
			KRN_ERROR("%s region already served!",
				  mach64_io->mem.name);
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

	SET_BASE(mach64_io->mem, options->pci->base0, MACH64_Base1_Size);

#	undef SET_BASE

	/*	make sure the memory regions are free.
	 */
	if (mem_check_region(&mach64_io->mem)){
		KRN_ERROR("check of mach64 memory region failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	claim the regions & the IRQ line
	 */
	mem_claim_region(&mach64_io->mem);
  
	if (KGI_EOK == irq_claim_line(&mach64_io->irq)) {
		KRN_DEBUG(1, "interrupt line claimed successfully");
		mach64->flags |= MACH64_CF_IRQ_CLAIMED;
	}


	/*   We use the standard VGA text memory location. Obviously this will
	     not work if this is a second adaptor. FIXME
	*/
	mach64_io->vga.aperture.name      = "mach64 mmio aperture";
	mach64_io->vga.aperture.device    = pcidev;
	mach64_io->vga.aperture.base_virt = MEM_NULL;
	mach64_io->vga.aperture.base_io   = 0xb8000;
	mach64_io->vga.aperture.size	  = 4096;
	mach64_io->vga.aperture.decode	  = MEM_DECODE_ALL;
	mach64_io->vga.aperture.rid	  = 2;
	/* mach64_io->vga.aperture = mach64_io->mem; */

	/*	make sure the memory regions are free. */
	if (mem_check_region(&mach64_io->vga.aperture)){
		KRN_ERROR("check of mach64 vga aperture failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	mem_claim_region(&mach64_io->vga.aperture);

	mach64_io->vga.kgim.DacOut8  = (void *) mach64_chipset_dac_out8;
	mach64_io->vga.kgim.DacIn8   = (void *) mach64_chipset_dac_in8;
	mach64_io->vga.kgim.DacOuts8 = (void *) mach64_chipset_dac_outs8;
	mach64_io->vga.kgim.DacIns8  = (void *) mach64_chipset_dac_ins8;
	
	mach64_io->vga.kgim.ClkOut8  = (void *) mach64_chipset_clk_out8;
	mach64_io->vga.kgim.ClkIn8   = (void *) mach64_chipset_clk_in8;
	
	mach64_io->vga.SeqOut8   = (void *) mach64_chipset_vga_seq_out8;
	mach64_io->vga.SeqIn8    = (void *) mach64_chipset_vga_seq_in8;
	mach64_io->vga.CrtOut8   = (void *) mach64_chipset_vga_crt_out8;
	mach64_io->vga.CrtIn8    = (void *) mach64_chipset_vga_crt_in8;
	mach64_io->vga.GrcOut8   = (void *) mach64_chipset_vga_grc_out8;
	mach64_io->vga.GrcIn8    = (void *) mach64_chipset_vga_grc_in8;
	mach64_io->vga.AtcOut8   = (void *) mach64_chipset_vga_atc_out8;
	mach64_io->vga.AtcIn8    = (void *) mach64_chipset_vga_atc_in8;
	mach64_io->vga.MiscOut8  = (void *) mach64_chipset_vga_misc_out8;
	mach64_io->vga.MiscIn8   = (void *) mach64_chipset_vga_misc_in8;
	mach64_io->vga.FctrlOut8 = (void *) mach64_chipset_vga_fctrl_out8;
	mach64_io->vga.FctrlIn8  = (void *) mach64_chipset_vga_fctrl_in8;
  
	mach64_io->CtrlOut32	= (void *) mach64_chipset_ctrl_out32;
	mach64_io->CtrlIn32	= (void *) mach64_chipset_ctrl_in32;
	mach64_io->CtrlOut8	= (void *) mach64_chipset_ctrl_out8;
	mach64_io->CtrlIn8	= (void *) mach64_chipset_ctrl_in8;
	mach64_io->ClockOut8    = (void *) mach64_chipset_clock_out8;
	mach64_io->ClockIn8     = (void *) mach64_chipset_clock_in8;
	
	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		   mach64->chipset.vendor, mach64->chipset.model);

	/* Enabling of the memory mapped registers
	{
		int i;
		KRN_DEBUG(1, "CONFIG_CNTL at 0x6AEC 0x%.8x", io_in32(0x6aec));
		io_out32(io_in32(0x6AEC) | 0x00000002, 0x6AEC);
		KRN_DEBUG(1, "CONFIG_CNTL at 0x6AEC 0x%.8x", io_in32(0x6aec));
		for(i = 0; i < 10000; ++i)
			mach64_io->mem.base_virt[i] = 100;
	}
	*/
	
	return KGI_EOK;
}

void mach64_chipset_done_module(mach64_chipset_t *mach64, 
	mach64_chipset_io_t *mach64_io, const kgim_options_t *options)
{
	if (mach64->flags & MACH64_CF_IRQ_CLAIMED)
		irq_free_line(&mach64_io->irq);

	mem_free_region(&mach64_io->mem);
	/* mem_free_region(&mach64_io->vga.aperture); */
}

const kgim_meta_t mach64_chipset_meta =
{
	(kgim_meta_init_module_fn *)    mach64_chipset_init_module,
	(kgim_meta_done_module_fn *)    mach64_chipset_done_module,
	(kgim_meta_init_fn *)           mach64_chipset_init,
	(kgim_meta_done_fn *)           mach64_chipset_done,
	(kgim_meta_mode_check_fn *)     mach64_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)  mach64_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)   mach64_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)     mach64_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)     mach64_chipset_mode_leave,
	(kgim_meta_image_resource_fn *) mach64_chipset_image_resource,
	
	sizeof(mach64_chipset_t),
	sizeof(mach64_chipset_io_t),
	sizeof(mach64_chipset_mode_t)
};
