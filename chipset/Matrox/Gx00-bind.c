/* ----------------------------------------------------------------------------
**	Matrox Gx00 chipset binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx00-bind.c,v 1.4 2003/01/09 12:14:51 foske Exp $
**	
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.4 $"

/*
** Debug levels
**
** 0: No debug (only notices)
** 1: Noticeable information
** 2: Functions and steps
** 3: Individual register accesses (esp. VGA)
** 4: Elementary DAC accesses (non VGA)
** 5: All register accesses (including all control aperture memory accesses)
*/

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#define	__Matrox_Gx00
#include "chipset/Matrox/Gx00.h"
#include "chipset/Matrox/Gx00-meta.h"
#include "chipset/IBM/VGA.h"

/*
**  Graphic control aperture access functions
*/
/*
** These are low level chipset-native IO helpers.
*/
static inline void mgag_gc_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
				kgi_u32_t reg) 
{
  KRN_DEBUG(5, "GC[%.4x] <- %.2x", reg, val); 
  mem_out8(val, mgag_io->control.base_virt + reg);
}

static inline kgi_u8_t mgag_gc_in8(mgag_chipset_io_t *mgag_io, kgi_u32_t reg) 
{
  kgi_u8_t val = mem_in8(mgag_io->control.base_virt + reg);
  KRN_DEBUG(5, "GC[%.4x] -> %.2x", reg, val);
  return val;
}

static inline void mgag_gc_out32(mgag_chipset_io_t *mgag_io, kgi_u32_t val,
				 kgi_u32_t reg)
{
  KRN_DEBUG(5, "GC[%.4x] <- %.8x", reg, val);
  mem_out32(val, mgag_io->control.base_virt + reg);
}

static inline kgi_u32_t mgag_gc_in32(mgag_chipset_io_t *mgag_io, kgi_u32_t reg)
{
  kgi_u32_t val = mem_in32(mgag_io->control.base_virt + reg);
  KRN_DEBUG(5, "GC[%.4x] -> %.8x", reg, val);
  return val;
}

/*
** eXtended DAC registers access functions
*/
static inline void mgag_xdac_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
				  kgi_u8_t reg)
{
  KRN_DEBUG(3, "XDAC[%.2x] <= %.2x", reg, val);
  mem_out8(reg, mgag_io->control.base_virt + MGAG_RAMDAC + PALWTADD);
  mem_out8(val, mgag_io->control.base_virt + MGAG_RAMDAC + X_DATAREG);
}

static inline kgi_u8_t mgag_xdac_in8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg)
{
  kgi_u8_t val;
  mem_out8(reg, mgag_io->control.base_virt + MGAG_RAMDAC + PALWTADD);
  val = mem_in8(mgag_io->control.base_virt + MGAG_RAMDAC + X_DATAREG);
  KRN_DEBUG(3, "XDAC[%.2x] => %.2x", reg, val);
  return val;
}

/*
** Extended CRT registers access functions
*/
static inline void mgag_ecrt_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
				  kgi_u8_t reg)
{
  KRN_DEBUG(3, "ECRT[%.2x] <= %.2x", reg, val);
  mem_out8(reg, mgag_io->control.base_virt + MMIO_CRTCEXTi);
  mem_out8(val, mgag_io->control.base_virt + MMIO_CRTCEXTd);
}

static inline kgi_u8_t mgag_ecrt_in8(mgag_chipset_io_t *mgag_io, kgi_u8_t reg)
{
  kgi_u8_t val;
  mem_out8(reg, mgag_io->control.base_virt + MMIO_CRTCEXTi);
  val = mem_in8(mgag_io->control.base_virt + MMIO_CRTCEXTd);
  KRN_DEBUG(3, "ECRT[%.2x] => %.2x", reg, val);
  return val;
}

/*
** ROM / Video BIOS access functions
*/
static inline kgi_u8_t mgag_rom_in8(mgag_chipset_io_t *mgag_io, kgi_u16_t reg)
{
  kgi_u8_t val = mem_in8(mgag_io->rom.base_virt + reg);
  KRN_DEBUG(5, "ROM[%.4x] -> %.2x", reg, val);
  return val;
}
static inline kgi_u16_t mgag_rom_in16(mgag_chipset_io_t *mgag_io, kgi_u16_t reg)
{
  kgi_u16_t val = mem_in16(mgag_io->rom.base_virt + reg);
  KRN_DEBUG(5, "ROM[%.4x] -> %.4x", reg, val);
  return val;
}
static inline kgi_u32_t mgag_rom_in32(mgag_chipset_io_t *mgag_io, kgi_u16_t reg)
{
  kgi_u32_t val = mem_in32(mgag_io->rom.base_virt + reg);
  KRN_DEBUG(5, "ROM[%.4x] -> %.8x", reg, val);
  return val;
}


/*
**     VGA subsystem I/O functions
*/
/* There is a VGA MMIO aperture at MGABASE1(PCI)+0x1F00 - 0x1FFF.
 * Apparently, this area is fully compatible with the original VGA I/O
 * adressing, which MGABASE1+0x1FC0 as a basis.
 * Note we do *not* use the VGA_IO_BASE offset but a hardcoded value.
 */
#define MGA_VGA_BASE (mgag_io->control.base_virt + MGAG_VGAREG + (VGA_IO_BASE & 0xFF))
/* Should be: + 0x1FC0 */

/* VGA Sequencer */
static inline void mgag_chipset_vga_seq_out8(mgag_chipset_io_t *mgag_io,
					     kgi_u8_t val, kgi_u_t reg)
{
  KRN_DEBUG(3, "VGA-SEQ%.2x <= %.2x", reg, val);
  mem_out8(reg, MGA_VGA_BASE + VGA_SEQ_INDEX);
  mem_out8(val, MGA_VGA_BASE + VGA_SEQ_DATA);
}
static inline kgi_u8_t mgag_chipset_vga_seq_in8(mgag_chipset_io_t *mgag_io,
						kgi_u_t reg)
{
  kgi_u8_t val;
  mem_out8(reg, MGA_VGA_BASE + VGA_SEQ_INDEX);
  val = mem_in8(MGA_VGA_BASE + VGA_SEQ_DATA);
  KRN_DEBUG(3, "VGA-SEQ%.2x => %.2x", reg, val);
  return val;
}

/* VGA CRTC registers */
static inline void mgag_chipset_vga_crt_out8(mgag_chipset_io_t *mgag_io,
					     kgi_u8_t val, kgi_u_t reg)
{
  KRN_DEBUG(3, "VGA-CRT%.2x <= %.2x", reg, val);
  mem_out8(reg, MGA_VGA_BASE + VGA_CRT_INDEX);
  mem_out8(val, MGA_VGA_BASE + VGA_CRT_DATA);
}
static inline kgi_u8_t mgag_chipset_vga_crt_in8(mgag_chipset_io_t *mgag_io,
						kgi_u_t reg)
{
  kgi_u8_t val;
  mem_out8(reg, MGA_VGA_BASE + VGA_CRT_INDEX);
  val = mem_in8(MGA_VGA_BASE + VGA_CRT_DATA);
  KRN_DEBUG(3, "VGA-CRT%.2x => %.2x", reg, val);
  return val;
}

/* VGA Graphics controller */
static inline void mgag_chipset_vga_grc_out8(mgag_chipset_io_t *mgag_io,
						 kgi_u8_t val, kgi_u_t reg)
{
  KRN_DEBUG(3, "VGA-GRC%.2x <= %.2x", reg, val);
  mem_out8(reg, MGA_VGA_BASE + VGA_GRC_INDEX);
  mem_out8(val, MGA_VGA_BASE + VGA_GRC_DATA);
}
static inline kgi_u8_t mgag_chipset_vga_grc_in8(mgag_chipset_io_t *mgag_io,
						kgi_u_t reg)
{
  kgi_u8_t val;
  mem_out8(reg, MGA_VGA_BASE + VGA_GRC_INDEX);
  val = mem_in8(MGA_VGA_BASE + VGA_GRC_DATA);
  KRN_DEBUG(3, "VGA-GRC%.2x => %.2x", reg, val);
  return val;
}

/* VGA Attribute controller */
static inline void mgag_chipset_vga_atc_out8(mgag_chipset_io_t *mgag_io,
					     kgi_u8_t val, kgi_u_t reg)
{
  KRN_DEBUG(3, "VGA-ATC%.2x <= %.2x", reg, val);
  /* First, we do a read in the feature control registers (CGA location) */
  mem_in8(MGA_VGA_BASE + VGA_ATC_AFF);
  kgi_nanosleep(250);
  /* Then we access the ATC regs */
  mem_out8(reg, MGA_VGA_BASE + VGA_ATC_INDEX); /* also enable the palette */
  kgi_nanosleep(250);
  mem_out8(val, MGA_VGA_BASE + VGA_ATC_DATAw);
  kgi_nanosleep(250);
#if 0 /* TODO: Remove this one as soon as it is validated -- ortalo */
  /* Then enable the palette */
  mem_out8(VGA_ATCI_ENABLE_DISPLAY, MGA_VGA_BASE + VGA_ATC_INDEX);
#endif
}
static inline kgi_u8_t mgag_chipset_vga_atc_in8(mgag_chipset_io_t *mgag_io,
						kgi_u_t reg)
{
  register kgi_u8_t val;
  /* First, we do a read in the feature control registers (CGA location) */
  mem_in8(MGA_VGA_BASE + VGA_ATC_AFF);
  kgi_nanosleep(250);
  /* Then we access the ATC regs */
  mem_out8(reg, MGA_VGA_BASE + VGA_ATC_INDEX); /* also enable the palette */
  kgi_nanosleep(250);
  val = mem_in8(MGA_VGA_BASE + VGA_ATC_DATAr);
  kgi_nanosleep(250);
#if 0 /* TODO: Remove this too -- ortalo */
  /* Then enable the palette */
  mem_out8(VGA_ATCI_ENABLE_DISPLAY, MGA_VGA_BASE + VGA_ATC_INDEX);
#endif
  KRN_DEBUG(3, "VGA-ATC%.2x => %.2x", reg, val);
  return val;
}

/* VGA Miscellaneous Output */
static inline void mgag_chipset_vga_misc_out8(mgag_chipset_io_t *mgag_io,
					      kgi_u8_t val)
{
  KRN_DEBUG(3, "VGA-MISC <- %.2x", val);
  mem_out8(val, MGA_VGA_BASE + VGA_MISCw);
}
static inline kgi_u8_t mgag_chipset_vga_misc_in8(mgag_chipset_io_t *mgag_io)
{
  kgi_u8_t val;
  val = mem_in8(MGA_VGA_BASE + VGA_MISCr);
  KRN_DEBUG(3, "VGA-MISC -> %.2x", val);
  return val;
}

/* VGA Feature Control */
/* Note that, for these to work, CGA emulation should be use (MISC<0>=1) */
static inline void mgag_chipset_vga_fctrl_out8(mgag_chipset_io_t *mgag_io,
					       kgi_u8_t val)
{
  KRN_DEBUG(3, "VGA-FCTRL <- %.2x", val);
  mem_out8(val, MGA_VGA_BASE + VGA_FCTRLw);
}
static inline kgi_u8_t mgag_chipset_vga_fctrl_in8(mgag_chipset_io_t *mgag_io)
{
  kgi_u8_t val;
  val = mem_in8(MGA_VGA_BASE + VGA_FCTRLr);
  KRN_DEBUG(3, "VGA-FCTRL -> %.2x", val);
  return val;
}

#undef MGA_VGA_BASE

/* The idea: DDC1 is a contiuous bitstream. We get it in as 128 x 9 bits
 * and the DDC driver will find the sync bit, order and CRC the data.
 */
/* Manually clocking VSYNC is something depending on your chipset.
 * For the Gx00 series, I force VSYNC inactive, and
 * by changing the inactive level (positive or negative sync)
 * the level of the VSYNC line changes.
 * This is likely to work on many chipsets.
 */	

static void mgag_chipset_ddc1_init (mgag_chipset_io_t * mgag_io) {
	/* First, force the VSYNC & HSYNC lines inactive
	 * Sync pulses disabled, but vsync must not be tristated !
	 */
	MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io,1) | 0x30, 1);
	/* Let the DDC1 SDA line act as an input. */
	MGAG_EDAC_OUT8(mgag_io,0,XGENIODATA);
      	MGAG_EDAC_OUT8(mgag_io,0,XGENIOCTRL);
}

static void mgag_chipset_ddc1_done (mgag_chipset_io_t * mgag_io) {
	MGAG_ECRT_OUT8(mgag_io,MGAG_ECRT_IN8(mgag_io,1) & 0xCF, 1);
}

static kgi_u8_t mgag_chipset_ddc1_get_sda (mgag_chipset_io_t * mgag_io) {
	return (MGAG_EDAC_IN8(mgag_io,XGENIODATA)&2)>>1;
}

static void mgag_chipset_ddc1_set_vsync (mgag_chipset_io_t * mgag_io, 
					 kgi_u8_t value) {
	MGAG_MISC_OUT8(mgag_io, (MGAG_MISC_IN8(mgag_io)&0x7f) | (value?0x80:0));
}
static kgi_u8_t mgag_chipset_ddc1_get_vsync (mgag_chipset_io_t * mgag_io) {
	return ((MGAG_GC_IN32(mgag_io,STATUS) && STATUS_VSYNCSTS) != 0);
}

/*
**      DAC subsystem I/O
*/
/* DAC registers 0,1,2,3 (commonly VGA 0x3C8, 0x3C9, 0x3C6 and 0x3C7)
 * are conveniently mapped to MMIO at addresses 0x3C00-0x3C03 (in the
 * right order).
 * Therefore, we can use a simple MMIO access to usual DAC regs (VGA).
 * Cool.
 */
static inline void mgag_dac_out8(mgag_chipset_io_t *mgag_io, kgi_u8_t val,
	kgi_u32_t reg)
{
  if (reg & PX_POINTER1) {
    if (!(mgag_io->flags & MGAG_POINTER_FB_READY)) {
      KRN_DEBUG(1, "Access to pointer shape while fboffset not known!!");
      /* Does nothing! */
    } else {
      /* This is not a real DAC register: we access the pointer
      ** shape in fb memory.
      */
      KRN_DEBUG(5, "Cursor1[%.3x] <- %.2x (@%.8x)",  reg & 0x3FF, val,
		mgag_io->fb.base_virt + mgag_io->ptr_fboffset + (reg & 0x3FF));
      mem_out8(val, mgag_io->fb.base_virt
	       + mgag_io->ptr_fboffset + (reg & 0x3FF));
    }
  } else if (mgag_io->flags & MGAG_IF_VGA_MODE) {
    /* Chipset in VGA mode: use VGA DAC ports - only 4 of them */
    /* TODO: KRN_ASSERT(reg < 4); ? */
    KRN_DEBUG(3, "VGA-DAC[%.2x] <- %.2x", reg, val);
    mem_out8(val, mgag_io->control.base_virt + MGAG_RAMDAC + reg);
  } else {
    /* Normal DAC accesses */
    /* TODO: KRN_ASSERT(reg <= 0x0A); ? */
    KRN_DEBUG(4, "DAC[%.4x] <- %.2x", reg, val);
    mem_out8(val, mgag_io->control.base_virt + MGAG_RAMDAC + reg);
  }
}

static inline kgi_u8_t mgag_dac_in8(mgag_chipset_io_t *mgag_io, kgi_u32_t reg)
{
  if (mgag_io->flags & MGAG_IF_VGA_MODE) {
    register kgi_u8_t val;
    /* Chipset in VGA mode: use VGA DAC ports - only 4 of them */
    /*    KRN_ASSERT(reg < 4); */
    val = mem_in8(mgag_io->control.base_virt + MGAG_RAMDAC + reg);
    KRN_DEBUG(3, "VGA-DAC[%.2x] -> %.2x", reg, val);
    return val;
  } else {
    register kgi_u8_t val;
    /* Normal DAC accesses */
    /* KRN_ASSERT(reg <= 0x0A); */
    val = mem_in8(mgag_io->control.base_virt + MGAG_RAMDAC + reg);
    KRN_DEBUG(4, "DAC[%.4x] -> %.2x", reg, val);
    return val;
  }
}

static inline void mgag_dac_outs8(mgag_chipset_io_t *mgag_io, kgi_u_t reg,
				  void *buf, kgi_u_t cnt) 
{
  if (mgag_io->flags & MGAG_IF_VGA_MODE) {
    /* Chipset in VGA mode: use VGA DAC ports - only 4 of them */
    /* KRN_ASSERT(reg < 4); */
    KRN_DEBUG(3, "VGA-DAC[%.2x] <- (%i bytes at %p) ...", reg, cnt, buf);
    mem_outs8(mgag_io->control.base_virt + MGAG_RAMDAC + reg, buf, cnt);
  } else {
    /* Normal DAC accesses */
    /* KRN_ASSERT(reg <= 0x0A); */
    KRN_DEBUG(4, "DAC[%.4x] <- (%i bytes at %p) ...", reg, cnt, buf);
    mem_outs8(mgag_io->control.base_virt + MGAG_RAMDAC + reg, buf, cnt);
  }
}

static inline void mgag_dac_ins8(mgag_chipset_io_t *mgag_io, kgi_u_t reg,
	void *buf, kgi_u_t cnt) 
{
  if (mgag_io->flags & MGAG_IF_VGA_MODE) {
    /* Chipset in VGA mode: use VGA DAC ports - only 4 of them */
    /* KRN_ASSERT(reg < 4); */
    KRN_DEBUG(3, "VGA-DAC[%.2x] -> (%i bytes at %p) ...", reg, cnt, buf);
    mem_ins8(mgag_io->control.base_virt + MGAG_RAMDAC + reg, buf, cnt);
  } else {
    /* Normal DAC accesses */
    /* KRN_ASSERT(reg <= 0x0A); */
    KRN_DEBUG(4, "DAC[%.4x] -> (%i bytes at %p)", reg, cnt, buf);
    mem_ins8(mgag_io->control.base_virt + MGAG_RAMDAC + reg, buf, cnt);
  }
}

/*
**       Clock Control
*/
static inline void mgag_clk_out8(mgag_chipset_io_t *mgag_io,
				 kgi_u8_t val, kgi_u_t reg)
{
  KRN_ASSERT(reg == 0);
#ifdef DEBUG_LEVEL
  if (mgag_io->flags & MGAG_IF_VGA_MODE) {
    KRN_DEBUG(3,"VGA-CLK%.2x <= %.2x", reg, val);
  } else {
    KRN_DEBUG(3,"CLK%.2x <= %.2x", reg, val);
  }
#endif
  mem_out8((mem_in8(mgag_io->control.base_virt + MMIO_MISCr) & 
	    ~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2),
	   mgag_io->control.base_virt + MMIO_MISCw);
}

static inline kgi_u8_t mgag_clk_in8(mgag_chipset_io_t *mgag_io, kgi_u_t reg) 
{
  register kgi_u8_t val;
  KRN_ASSERT(reg == 0);
  val = ((mem_in8(mgag_io->control.base_virt + MMIO_MISCr) & 
	  VGA_MISC_CLOCK_MASK) >> 2);
#ifdef DEBUG_LEVEL
  if (mgag_io->flags & MGAG_IF_VGA_MODE) {
    KRN_DEBUG(3,"VGA-CLK%.2x => %.2x", reg, val);
  } else {
    KRN_DEBUG(3,"CLK%.2x => %.2x", reg, val);
  }
#endif
  return val;
}

/*	KGI interface
*/

static kgi_error_t mgag_chipset_init_module(mgag_chipset_t *mgag, 
	mgag_chipset_io_t *mgag_io, const kgim_options_t *options)
{
	kgi_u_t		mgabase2_size = 0;

	kgi_u32_t	pci_mgabase1 = 0,
			pci_mgabase2 = 0,
			pci_rid1 = 0, pci_rid2 = 0;

	static const kgi_u32_t mgag_chipset_pcicfg[] = {

		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_PCI),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G200_AGP),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G550),
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;

	KRN_DEBUG(2, "entered");

	KRN_ASSERT(mgag);
	KRN_ASSERT(mgag_io);
	KRN_ASSERT(options);

	/* auto-detect/verify the chipset */

	if (pcidev == PCICFG_NULL) {

		if (pcicfg_find_device(&pcidev, mgag_chipset_pcicfg)) {

			KRN_ERROR("No supported device found.");

			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	kgim_memset(mgag, 0, sizeof(*mgag));

	kgim_strcpy(mgag->chipset.vendor, "Matrox Graphics Inc.");
	kgim_strcpy(mgag->chipset.model,  "1x64/Gx00 Chipset");

	mgag->chipset.revision  = KGIM_CHIPSET_REVISION;
	mgag->chipset.mode_size = sizeof(mgag_chipset_mode_t);

	mgag->chipset.vendor_id = pcicfg_in16(pcidev + PCI_VENDOR_ID);
	mgag->chipset.device_id = pcicfg_in16(pcidev + PCI_DEVICE_ID);

	mgag->pci.Command	= pcicfg_in16(pcidev + PCI_COMMAND);
	mgag->pci.LatTimer	= pcicfg_in8(pcidev + PCI_LATENCY_TIMER);
	mgag->pci.IntLine	= pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
	mgag->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	mgag->pci.BaseAddr1	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
	mgag->pci.BaseAddr2	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_2);

	switch PCICFG_SIGNATURE(mgag->chipset.vendor_id,
		mgag->chipset.device_id) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_MYS):

		mgag->chipset.maxdots.x = 2048;
		mgag->chipset.maxdots.y = 2048;

		mgabase2_size = M1x64_MGABASE2_SIZE;

		mgag->flags |= MGAG_CF_1x64 | MGAG_CF_SGRAM;

		KRN_NOTICE("Matrox Mystique revision %i",
			   pcicfg_in8(pcidev + PCI_REVISION_ID));

		switch(pcicfg_in8(pcidev + PCI_REVISION_ID)) {

		case 1:
			mgag->chipset.dclk.max = 135 MHZ;

			mgag->flags |= MGAG_CF_OLD; /* Ancient setup */
			pci_mgabase1 = mgag->pci.BaseAddr0;
			pci_mgabase2 = mgag->pci.BaseAddr1;

			break;

			/* TODO: check dclk limits for Mystique chipset
			 * TODO: with revision >= 2 ! -- ortalo
			 */
		case 2:
			mgag->chipset.dclk.max = 175 MHZ;

			mgag->flags |= MGAG_CF_OLD; /* Ancient setup */
			pci_mgabase1 = mgag->pci.BaseAddr0;
			pci_mgabase2 = mgag->pci.BaseAddr1;

			break;
		case 3:
			mgag->chipset.dclk.max = 220 MHZ;

			/* TODO: check that Mystique rev.3 really
			 * TODO: has MGABASE2 at PCIBASEADDR0! -- ortalo
			 */
			pci_mgabase1 = mgag->pci.BaseAddr1;
			pci_mgabase2 = mgag->pci.BaseAddr0;

			break;
		default:
			KRN_INTERNAL_ERROR;
			return -KGI_ERRNO(CHIPSET, NOSUP);
		}

		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, 
		PCI_DEVICE_ID_MATROX_G200_PCI):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX,
		PCI_DEVICE_ID_MATROX_G200_AGP):

		KRN_NOTICE("Matrox G200 revision %i",
			   pcicfg_in8(pcidev + PCI_REVISION_ID));

		mgag->chipset.maxdots.x = 4096;
		mgag->chipset.maxdots.y = 4096;

		mgag->flags |= MGAG_CF_G200;

		mgabase2_size = G200_MGABASE2_SIZE;

		pci_mgabase1 = mgag->pci.BaseAddr1;
		pci_mgabase2 = mgag->pci.BaseAddr0;

		pci_rid1 = PCI_BASE_ADDRESS_1;
		pci_rid2 = PCI_BASE_ADDRESS_0;

		mgag->chipset.dclk.max = (mgag->flags & MGAG_CF_SGRAM) 
			? 250 MHZ : 230 MHZ;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G400):

		KRN_NOTICE("Matrox G400 revision %i",
			   pcicfg_in8(pcidev + PCI_REVISION_ID));

		mgag->chipset.maxdots.x = 4096;
		mgag->chipset.maxdots.y = 4096;

		mgag->flags |= MGAG_CF_G400;
		if (pcicfg_in8(pcidev + PCI_REVISION_ID) >= 128)
		  {
		    KRN_NOTICE("Matrox G450 variant");
		    mgag->flags |= MGAG_CF_G450;
		  }

		mgabase2_size = G400_MGABASE2_SIZE;

		pci_mgabase1 = mgag->pci.BaseAddr1;
		pci_mgabase2 = mgag->pci.BaseAddr0;

		pci_rid1 = PCI_BASE_ADDRESS_1;
		pci_rid2 = PCI_BASE_ADDRESS_0;

		mgag->chipset.dclk.max = 300 MHZ; /* TODO: Check/Updt (G450) */

		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_MATROX, PCI_DEVICE_ID_MATROX_G550):

		KRN_NOTICE("Matrox G550 revision %i",
			   pcicfg_in8(pcidev + PCI_REVISION_ID));

		mgag->chipset.maxdots.x = 4096;
		mgag->chipset.maxdots.y = 4096;

		mgag->flags |= MGAG_CF_G550;

		mgabase2_size = G550_MGABASE2_SIZE;

		pci_mgabase1 = mgag->pci.BaseAddr1;
		pci_mgabase2 = mgag->pci.BaseAddr0;

		pci_rid1 = PCI_BASE_ADDRESS_1;
		pci_rid2 = PCI_BASE_ADDRESS_0;

		mgag->chipset.dclk.max = 300 MHZ; /* TODO: Check/Update. */

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	if MEMORY set, the board was initialized by the BIOS and 
	**	we needn't do memory init 
	*/
	if (mgag->pci.Command & PCI_COMMAND_IO) {	  
	  KRN_DEBUG(1, "board is a primary card - already initialized by the BIOS");
	  mgag->flags |= MGAG_CF_PRIMARY;
	} else {
	  KRN_DEBUG(1, "board is NOT a primary card - NOT initialized by BIOS");
	  mgag->flags &= ~MGAG_CF_PRIMARY;	  
	}

	/* Initialize driver claimed regions */

	mgag_io->vga.kgim.pcidev       	= pcidev;

	mgag_io->control.name		= "1x64/Gx00 Control Aperture";
	mgag_io->control.device		= pcidev;
	mgag_io->control.base_virt	= MEM_NULL;
	mgag_io->control.base_io	= pci_mgabase1 & ~(MGAG_MGABASE1_SIZE - 1);
	mgag_io->control.size		= MGAG_MGABASE1_SIZE;
	mgag_io->control.decode		= MEM_DECODE_ALL;
	mgag_io->control.rid		= pci_rid1;

	mgag_io->fb.name		= "1x64/Gx00 Framebuffer Aperture";
	mgag_io->fb.device		= pcidev;
	mgag_io->fb.base_virt		= MEM_NULL;
	mgag_io->fb.base_io		= pci_mgabase2 & ~(mgabase2_size - 1);
	mgag_io->fb.size		= mgabase2_size;
	mgag_io->fb.decode		= MEM_DECODE_ALL;
	mgag_io->fb.rid			= pci_rid2;

	mgag_io->iload.name		= "1x64/Gx00 ILOAD Aperture";
	mgag_io->iload.device		= pcidev;
	mgag_io->iload.base_virt	= MEM_NULL;
	mgag_io->iload.base_io		= mgag->pci.BaseAddr2 & ~(MGAG_MGABASE3_SIZE - 1);
	mgag_io->iload.size		= MGAG_MGABASE3_SIZE;
	mgag_io->iload.decode		= MEM_DECODE_ALL;
	mgag_io->iload.rid		= PCI_BASE_ADDRESS_2;

	mgag->pci.RomAddr      = mgag_io->control.base_io + 0x10000;

	mgag_io->rom.name      = "Matrox BIOS";
	mgag_io->rom.device    = pcidev;
	mgag_io->rom.base_virt = MEM_NULL;
	mgag_io->rom.base_io   = mgag->pci.RomAddr & ~(MGAG_ROM_SIZE -1);
	mgag_io->rom.size      = MGAG_ROM_SIZE;
	mgag_io->rom.decode    = MEM_DECODE_ALL;
	mgag_io->rom.rid       = PCI_ROM_ADDRESS;

	pcicfg_out32(mgag_io->rom.base_io | PCI_ROM_ADDRESS_ENABLE,
		     pcidev + PCI_ROM_ADDRESS);

	mgag_io->irq.flags	= IF_SHARED_IRQ;
	mgag_io->irq.device     = pcidev;
	mgag_io->irq.name	= "1x64/Gx00 interrupt line";
	mgag_io->irq.line	= mgag->pci.IntLine & 0xFF;
	mgag_io->irq.meta	= mgag;
	mgag_io->irq.meta_io	= mgag_io;
	mgag_io->irq.High	= (irq_handler_fn *) mgag_chipset_irq_handler;
	mgag_io->irq.rid        = 0;

	/* VGA text16 aperture */
	mgag_io->text16fb.name      = "Matrox VGA text16 aperture";
	mgag_io->text16fb.device    = pcidev;
	mgag_io->text16fb.base_virt = MEM_NULL;
	mgag_io->text16fb.base_io   = VGA_TEXT_MEM_BASE;
	mgag_io->text16fb.size      = VGA_TEXT_MEM_SIZE / 2;
	mgag_io->text16fb.decode    = MEM_DECODE_ALL;
	mgag_io->text16fb.rid       = 1;

	/*
	** We dissallow option override of PCI bases, since they are 
	** chipset/revision dependant. (They should be done here.)
	*/

#if 0
	/*
	** Make sure the memory regions are free.
	*/
	if (mem_check_region(&mgag_io->text16fb)) {
	  KRN_ERROR("%s region already served!", mgag_io->text16fb.name);
	  return -KGI_ERRNO(CHIPSET, INVAL);
	}
#endif

	/* TODO: Provide better error messages */
	if (	mem_check_region(&mgag_io->control)	||
		mem_check_region(&mgag_io->fb)		||
		mem_check_region(&mgag_io->iload)) {

		KRN_ERROR("check of 1x64/Gx00 memory regions failed.");

		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*
	** Checks the ROM (video BIOS) region independently
	*/
	if (mem_check_region(&mgag_io->rom)) {
	  KRN_ERROR("%s region already served! (base_io: %.8x)",
		    mgag_io->rom.name, mgag_io->rom.base_io);
	  return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* claim the regions */
	
#if 0
	mem_claim_region(&mgag_io->text16fb); /* VGA text fb */
#endif
	mem_claim_region(&mgag_io->control);
	mem_claim_region(&mgag_io->fb);
	mem_claim_region(&mgag_io->iload);
	mem_claim_region(&mgag_io->rom);

	if (KGI_EOK == irq_claim_line(&mgag_io->irq))
	  {
	    KRN_DEBUG(1, "interrupt line claimed successfully");
	    mgag->flags |= MGAG_CF_IRQ_CLAIMED;
	  }
	else
	  {
	    KRN_ERROR("Unable to claim Gx00 interrupt line!");
	  }

	/* Initialize the VGA aperture */
	mgag_io->vga.aperture = mgag_io->text16fb;

	/* initializing the io helper struct. */
	mgag_io->vga.kgim.DacOut8	= (void *) mgag_dac_out8;
	mgag_io->vga.kgim.DacIn8	= (void *) mgag_dac_in8;
	mgag_io->vga.kgim.DacOuts8	= (void *) mgag_dac_outs8;
	mgag_io->vga.kgim.DacIns8	= (void *) mgag_dac_ins8;

	mgag_io->vga.kgim.ClkOut8	= (void *) mgag_clk_out8;
	mgag_io->vga.kgim.ClkIn8	= (void *) mgag_clk_in8;

	mgag_io->vga.kgim.DDCInit	= (void *) mgag_chipset_ddc1_init;
	mgag_io->vga.kgim.DDCDone	= (void *) mgag_chipset_ddc1_done;
	mgag_io->vga.kgim.DDCSetSCL	= (void *) NULL;
	mgag_io->vga.kgim.DDCGetSDA	= (void *) mgag_chipset_ddc1_get_sda;
	mgag_io->vga.kgim.DDCSetVSYNC	= (void *) mgag_chipset_ddc1_set_vsync;
	mgag_io->vga.kgim.DDCGetVSYNC	= (void *) mgag_chipset_ddc1_get_vsync;
	
	mgag_io->vga.SeqOut8   = (void *) mgag_chipset_vga_seq_out8;
	mgag_io->vga.SeqIn8    = (void *) mgag_chipset_vga_seq_in8;
	mgag_io->vga.CrtOut8   = (void *) mgag_chipset_vga_crt_out8;
	mgag_io->vga.CrtIn8    = (void *) mgag_chipset_vga_crt_in8;
	mgag_io->vga.GrcOut8   = (void *) mgag_chipset_vga_grc_out8;
	mgag_io->vga.GrcIn8    = (void *) mgag_chipset_vga_grc_in8;
	mgag_io->vga.AtcOut8   = (void *) mgag_chipset_vga_atc_out8;
	mgag_io->vga.AtcIn8    = (void *) mgag_chipset_vga_atc_in8;
	mgag_io->vga.MiscOut8  = (void *) mgag_chipset_vga_misc_out8;
	mgag_io->vga.MiscIn8   = (void *) mgag_chipset_vga_misc_in8;
	mgag_io->vga.FctrlOut8 = (void *) mgag_chipset_vga_fctrl_out8;
	mgag_io->vga.FctrlIn8  = (void *) mgag_chipset_vga_fctrl_in8;

	mgag_io->GCIn8		= (void *) mgag_gc_in8;
	mgag_io->GCOut8		= (void *) mgag_gc_out8;
	mgag_io->GCIn32		= (void *) mgag_gc_in32;
	mgag_io->GCOut32	= (void *) mgag_gc_out32;

	mgag_io->ECRTIn8        = (void *) mgag_ecrt_in8;
	mgag_io->ECRTOut8       = (void *) mgag_ecrt_out8;

	mgag_io->EDACIn8        = (void *) mgag_xdac_in8;
	mgag_io->EDACOut8       = (void *) mgag_xdac_out8;

	mgag_io->ROMIn8         = (void *) mgag_rom_in8;
	mgag_io->ROMIn16        = (void *) mgag_rom_in16;
	mgag_io->ROMIn32        = (void *) mgag_rom_in32;

	/* Initializes io-flags to all 0 */
	mgag_io->flags = 0; 

 	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		   mgag->chipset.vendor, mgag->chipset.model);

	KRN_DEBUG(2, "completed");

	return KGI_EOK;
}

static void mgag_chipset_done_module(mgag_chipset_t *mgag,
				     mgag_chipset_io_t *mgag_io,
				     const kgim_options_t *options) 
{
	KRN_DEBUG(2, "entered");

	if (mgag->flags & MGAG_CF_IRQ_CLAIMED) {

		irq_free_line(&mgag_io->irq);
	}

	mem_free_region(&mgag_io->rom);
	mem_free_region(&mgag_io->iload);
	mem_free_region(&mgag_io->fb);
	mem_free_region(&mgag_io->control);
#if 0
	mem_free_region(&mgag_io->text16fb);
#endif

	KRN_NOTICE("%s %s driver removed.", 
		mgag->chipset.vendor, mgag->chipset.model);

	KRN_DEBUG(2, "completed");
}

const kgim_meta_t mgag_chipset_meta = {

	(kgim_meta_init_module_fn *)	mgag_chipset_init_module,
	(kgim_meta_done_module_fn *)	mgag_chipset_done_module,
	(kgim_meta_init_fn *)		mgag_chipset_init,
	(kgim_meta_done_fn *)		mgag_chipset_done,
	(kgim_meta_mode_check_fn *)	mgag_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	mgag_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	mgag_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	mgag_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	mgag_chipset_mode_leave,
	(kgim_meta_image_resource_fn *)	mgag_chipset_image_resource,

	sizeof(mgag_chipset_t),
	sizeof(mgag_chipset_io_t),
	sizeof(mgag_chipset_mode_t)
};
