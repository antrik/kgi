/* ----------------------------------------------------------------------------
**	S3 ViRGE chipset meta language binding
** ----------------------------------------------------------------------------
**	Copyright (C)	1999		Jon Taylor
**	Copyright (C)	1999-2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ViRGE-bind.c,v $
**	Revision 1.3  2003/01/30 13:44:46  foske
**	Mayor cleanup, fix textmode, drop secondary card support, it wasn't going to work anyway.
**	
**	Revision 1.2  2002/09/21 14:35:10  aldot
**	- fix ATC border color reads
**	
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.4  2001/07/03 08:58:59  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.3  2000/09/21 12:32:21  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.2  2000/08/04 11:39:08  seeger_s
**	- merged version posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- status is reported to be 30% by Jos.
**	
**	Revision 1.1.1.1  2000/04/18 08:51:25  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#include <kgi/maintainers.h>
#define	DEBUG_LEVEL	2
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.3 $"

#include <kgi/module.h>

#define	__S3_ViRGE
#include "chipset/S3/ViRGE.h"
#include "chipset/S3/ViRGE-meta.h"

#define VIRGE_MMIO_BASE		(virge_io->aperture.base_virt)+0x1000000
#define VIRGE_VGA_IO_BASE      	VGA_IO_BASE
#define VIRGE_VGA_MEM_BASE	(VIRGE_MMIO_BASE + 0x83C0)
#define	VIRGE_VGA_DAC		(virge_io->flags & VIRGE_IF_VGA_DAC)

/*	ViRGE specific I/O functions
*/

static inline void virge_chipset_mmio_out16(virge_chipset_io_t *virge_io, kgi_u16_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "MMIO%.4x <- %.4x", reg, val);
	
	mem_out16(val, (VIRGE_MMIO_BASE + reg));
}

static inline kgi_u16_t virge_chipset_mmio_in16(virge_chipset_io_t *virge_io, kgi_u32_t reg)
{
	kgi_u16_t val;
	
	val = mem_in16(VIRGE_MMIO_BASE + reg);
	
	KRN_DEBUG(3, "MMIO%.4x -> %.4x", reg, val);
	
	return val;
}

static inline void virge_chipset_mmio_out32(virge_chipset_io_t *virge_io, kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "MMIO%.4x <- %.8x", reg, val);
	
	mem_out32(val, (VIRGE_MMIO_BASE + reg));
}

static inline kgi_u32_t virge_chipset_mmio_in32(virge_chipset_io_t *virge_io, kgi_u32_t reg)
{
	kgi_u32_t val;
	
	val = mem_in32(VIRGE_MMIO_BASE + reg);
	
	KRN_DEBUG(3, "MMIO%.4x -> %.8x", reg, val);
	
	return val;
}

/* VGA subsystem I/O functions */

static inline void virge_chipset_vga_seq_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  KRN_DEBUG(3, "VGA_SEQ mm %.2x w %.2x", reg, val);
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_SEQ_INDEX);
	  mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_SEQ_DATA);
	} else {
	  KRN_DEBUG(3, "VGA_SEQ io %.2x w %.2x", reg, val);
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_SEQ_INDEX);
	  io_out8(val, VIRGE_VGA_IO_BASE + VGA_SEQ_DATA);
	}
#ifdef CHECK_WRITES
	SEQ_IN8(virge_io, reg);
#endif
}

static inline kgi_u8_t virge_chipset_vga_seq_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
	kgi_u8_t val;
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_SEQ_INDEX);	
	  val = mem_in8(VIRGE_VGA_MEM_BASE + VGA_SEQ_DATA);
	  KRN_DEBUG(3, "VGA_SEQ mm %.2x r %.2x", reg, val);
	} else {
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_SEQ_INDEX);
	  val = io_in8(VIRGE_VGA_IO_BASE + VGA_SEQ_DATA);
	  KRN_DEBUG(3, "VGA_SEQ io %.2x r %.2x", reg, val);
	}
	return val;
}

static inline void virge_chipset_vga_crt_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_CRT_INDEX);
	  mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_CRT_DATA);
	  KRN_DEBUG(3, "VGA_CRT mm %.2x w %.2x", reg, val);
	} else {
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_CRT_INDEX);
	  io_out8(val, VIRGE_VGA_IO_BASE + VGA_CRT_DATA);
	  KRN_DEBUG(3, "VGA_CRT io %.2x w %.2x", reg, val);
	}
#ifdef CHECK_WRITES
	CRT_IN8(virge_io, reg);
#endif
}

static inline kgi_u8_t virge_chipset_vga_crt_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
	kgi_u8_t val;
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_CRT_INDEX);
	  val = mem_in8(VIRGE_VGA_MEM_BASE + VGA_CRT_DATA);
	  KRN_DEBUG(3, "VGA_CRT mm %.2x r %.2x", reg, val);
	} else {
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_CRT_INDEX);
	  val = io_in8(VIRGE_VGA_IO_BASE + VGA_CRT_DATA);
	  KRN_DEBUG(3, "VGA_CRT io %.2x r %.2x", reg, val);
	}
	return val;
}

static inline void virge_chipset_vga_grc_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_GRC_INDEX);
	  mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_GRC_DATA);
	  KRN_DEBUG(3, "VGA_GRC mm %.2x w %.2x", reg, val);
	} else {
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_GRC_INDEX);
	  io_out8(val, VIRGE_VGA_IO_BASE + VGA_GRC_DATA);
	  KRN_DEBUG(3, "VGA_GRC io %.2x w %.2x", reg, val);
	}
#ifdef CHECK_WRITES
	GRC_IN8(virge_io, reg);
#endif
}

static inline kgi_u8_t virge_chipset_vga_grc_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
	kgi_u8_t val;
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_GRC_INDEX);	
	  val = mem_in8(VIRGE_VGA_MEM_BASE + VGA_GRC_DATA);
	  KRN_DEBUG(3, "VGA_GRC mm %.2x r %.2x", reg, val);
	} else {
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_GRC_INDEX);
	  val = io_in8(VIRGE_VGA_IO_BASE + VGA_GRC_DATA);
	  KRN_DEBUG(3, "VGA_GRC io %.2x r %.2x", reg, val);
	}	
	return val;
}

static inline void virge_chipset_vga_atc_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_in8(VIRGE_VGA_MEM_BASE + VGA_ATC_AFF);
	  kgi_nanosleep(250);
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_ATC_INDEX);
	  kgi_nanosleep(250);
	  mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_ATC_DATAw);
	  kgi_nanosleep(250);
	  KRN_DEBUG(3, "VGA_ATC mm %.2x w %.2x", reg, val);
	} else {
	  io_in8(VIRGE_VGA_IO_BASE + VGA_ATC_AFF);
	  kgi_nanosleep(250);
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);
	  kgi_nanosleep(250);
	  io_out8(val, VIRGE_VGA_IO_BASE + VGA_ATC_DATAw);
	  kgi_nanosleep(250);
	  KRN_DEBUG(3, "VGA_ATC io %.2x w %.2x", reg, val);
	}
#ifdef CHECK_WRITES
	ATC_IN8(virge_io, reg);
#endif
}

static inline kgi_u8_t virge_chipset_vga_atc_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
	register kgi_u8_t val;
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_in8(VIRGE_VGA_MEM_BASE + VGA_ATC_AFF);
	  kgi_nanosleep(250);
	  mem_out8(reg, VIRGE_VGA_MEM_BASE + VGA_ATC_INDEX);
	  kgi_nanosleep(250);
	  val = mem_in8(VIRGE_VGA_MEM_BASE + VGA_ATC_DATAr);
	  kgi_nanosleep(250);
	  KRN_DEBUG(3, "VGA_ATC mm %.2x r %.2x", reg, val);
	} else {
	  io_in8(VIRGE_VGA_IO_BASE + VGA_ATC_AFF);
	  kgi_nanosleep(250);
	  io_out8(reg, VIRGE_VGA_IO_BASE + VGA_ATC_INDEX);
	  kgi_nanosleep(250);
	  val = io_in8(VIRGE_VGA_IO_BASE + VGA_ATC_DATAr);
	  kgi_nanosleep(250);
	  KRN_DEBUG(3, "VGA_ATC io %.2x r %.2x", reg, val);
	}
	return val;
}

static inline void virge_chipset_vga_misc_out8(virge_chipset_io_t *virge_io, kgi_u8_t val)
{
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_MISCw);
	  KRN_DEBUG(3, "VGA_MISC mm w %.2x", val);
	} else {
	  io_out8(val, VIRGE_VGA_IO_BASE + VGA_MISCw);
	  KRN_DEBUG(3, "VGA_MISC io w %.2x", val);
	}
}

static inline kgi_u8_t virge_chipset_vga_misc_in8(virge_chipset_io_t *virge_io)
{
	kgi_u8_t val;
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  val = mem_in8(VIRGE_VGA_MEM_BASE + VGA_MISCr);
	  KRN_DEBUG(3, "VGA_MISC mm r %.2x", val);
	} else {
	  val = io_in8(VIRGE_VGA_IO_BASE + VGA_MISCr);
	  KRN_DEBUG(3, "VGA_MISC io r %.2x", val);
	}
	return val;
}

static inline void virge_chipset_vga_fctrl_out8(virge_chipset_io_t *virge_io, kgi_u8_t val)
{
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  mem_out8(val, VIRGE_VGA_MEM_BASE + VGA_FCTRLw);
	  KRN_DEBUG(3, "VGA_FCTRL mm w %.2x", val);
	} else {
	  io_out8(val, VIRGE_VGA_IO_BASE + VGA_FCTRLw);
	  KRN_DEBUG(3, "VGA_FCTRL io w %.2x", val);
	}
}

static inline kgi_u8_t virge_chipset_vga_fctrl_in8(virge_chipset_io_t *virge_io)
{
	kgi_u8_t val;
	
	if (virge_io->flags & VIRGE_IF_MMIO_ENABLED) {
	  val = mem_in8(VIRGE_VGA_MEM_BASE + VGA_FCTRLr);
	  KRN_DEBUG(3, "VGA_FCTRL mm r %.2x", val);
	} else {
	  val = io_in8(VIRGE_VGA_IO_BASE + VGA_FCTRLr);
	  KRN_DEBUG(3, "VGA_FCTRL io r%.2x", val);
	}
	return val;
}

virge_chipset_save_state (virge_chipset_state_t * virge_state, virge_chipset_io_t * virge_io) {
	int i;
	
	KRN_DEBUG(2, "ViRGE: Saving state");
	for (i=0; i < ViRGE_NR_CRT_REGS; i++) 
		virge_state->crt[i] = CRT_IN8(virge_io, i);
	for (i=0; i < ViRGE_NR_SEQ_REGS; i++)
		virge_state->seq[i] = SEQ_IN8(virge_io, i);
	for (i=0; i < ViRGE_NR_ATC_REGS; i++)
		virge_state->atc[i] = ATC_IN8(virge_io, i);
	for (i=0; i < ViRGE_NR_GRC_REGS; i++)
		virge_state->grc[i] = GRC_IN8(virge_io, i);
}

virge_chipset_restore_state (virge_chipset_state_t * virge_state, virge_chipset_io_t * virge_io) {
	int i;

	KRN_DEBUG(2, "ViRGE: Restoring state");
	for (i=0; i < ViRGE_NR_GRC_REGS; i++)
		GRC_OUT8 (virge_io, virge_state->grc[i], i);
	for (i=0; i < ViRGE_NR_ATC_REGS; i++)
		ATC_OUT8 (virge_io, virge_state->atc[i], i);
	for (i=0; i < ViRGE_NR_SEQ_REGS; i++)
		SEQ_OUT8 (virge_io, virge_state->seq[i], i);
	for (i=0; i < ViRGE_NR_CRT_REGS; i++)
		CRT_OUT8 (virge_io, virge_state->crt[i], i);
}


/* Manually clocking VSYNC is something depending on your chipset.
 * For the ViRGE series, I can take over control of the VSYNC line.
 */	

static void virge_chipset_ddc1_init (virge_chipset_io_t * virge_io) {
	/* Set the sync lines manually */
//	KRN_DEBUG(1,"ddc1_init");
	SEQ_OUT8(virge_io, (SEQ_IN8(virge_io, 0x0D) & 0x0C) | 0x52, 0x0D);
	/* Let the DDC1 SDA line act as an input. */
	//SEQ_OUT8(virge_io, 0x01, 0x1C);
	//MMIO_OUT32(0x00000000, LPB_GENIOPORT);
	//CRT_OUT8(virge_io, 0xff, 0x5C);
	MMIO_OUT32(0x00000012, LPB_SERIALPORT);
}

static void virge_chipset_ddc1_done (virge_chipset_io_t * virge_io) {
//	KRN_DEBUG(1,"ddc1_done");
	SEQ_OUT8(virge_io, (SEQ_IN8(virge_io, 0x0D) & 0x0C) | 0x00, 0x0D);
	MMIO_OUT32 (0x00000000,LPB_SERIALPORT);
}

static void virge_chipset_ddc1_set_scl (virge_chipset_io_t * virge_io, 
				       kgi_u8_t value) {
	SEQ_OUT8(virge_io, (SEQ_IN8(virge_io, 0x0D) & 0x3F) | 
		 (value ? 0x80 : 0x40), 0x0D);
}

static kgi_u8_t virge_chipset_ddc1_get_sda (virge_chipset_io_t * virge_io) {
	return	(MMIO_IN32(LPB_SERIALPORT) & 0x8 ) >> 3;
}


/* DAC subsystem I/O */
static const kgi_u_t virge_vga_dac_register[4] = { 0x08, 0x09, 0x06, 0x07 };

#define	virge_SET_DAC_ADDR23						  	\
	mem_out8(0x05, VIRGE_VGA_BASE + VGA_SEQ_INDEX);		  	  	\
	mem_out8((mem_in8(VIRGE_VGA_BASE + VGA_SEQ_DATA) &		  	\
		~VIRGE_SR05_DACAddrMask) | ((reg << 2) & VIRGE_SR05_DACAddrMask),	\
		VIRGE_VGA_BASE + VGA_SEQ_DATA)

static inline void virge_chipset_dac_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
/*	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);
*/
	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "DAC_IO %.2x <- %.2x", reg, val);

/*		virge_SET_DAC_ADDR23;
**		mem_out8(val, VIRGE_VGA_BASE + VIRGE_vga_dac_register[reg & 3]);
*/		io_out8(val, VIRGE_VGA_IO_BASE + virge_vga_dac_register[reg & 3]);
	} else {

		KRN_DEBUG(3, "DAC_MEM %.2x <- %.2x", reg, val);
		
/*		mem_out8(val, VIRGE_VGA_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3));
*/
	}
}

static inline kgi_u8_t virge_chipset_dac_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
/*	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);
*/
	if (VIRGE_VGA_DAC) {

		register kgi_u8_t val;

/*		VIRGE_SET_DAC_ADDR23;
**		
**		val = mem_in8(VIRGE_VGA_BASE + virge_vga_dac_register[reg & 3]);
*/		val = io_in8(VIRGE_VGA_IO_BASE + virge_vga_dac_register[reg & 3]);
		
		KRN_DEBUG(3, "DAC_IO %.2x -> %.2x", reg, val);
		
		return val;

	} else {

		register kgi_u8_t val = 0xff;

/*		val = mem_in8(VIRGE_VGA_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3));
*/
		KRN_DEBUG(3, "DAC_MEM %.2x -> %.2x", reg, val);
		
		return val;
	}
}

static inline void virge_chipset_dac_outs8(virge_chipset_io_t *virge_io, kgi_u_t reg, void *buf, kgi_u_t cnt)
{
#if 0 /* FIXME */
	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);

	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "DAC_VGA %.2x <- (%i bytes)", reg, cnt);
		
		VIRGE_SET_DAC_ADDR23;
		
		mem_outs8(VIRGE_VGA_BASE + virge_vga_dac_register[reg & 3], buf, cnt);

	} else {

		KRN_DEBUG(3, "dac_mem %.2x <- (%i bytes)", reg, cnt);
		
		mem_outs8(VIRGE_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3), buf, cnt);
	}
#endif
}

static inline void virge_chipset_dac_ins8(virge_chipset_io_t *virge_io, kgi_u_t reg, void *buf, kgi_u_t cnt)
{
/*	KRN_ASSERT(reg < VIRGE_MAX_DAC_REGISTERS);
*/
	if (VIRGE_VGA_DAC) {

		KRN_DEBUG(3, "DAC_VGA %.2x -> (%i bytes)", reg, cnt);
		
/*		VIRGE_SET_DAC_ADDR23;
**
**		mem_ins8(VIRGE_VGA_BASE + VIRGE_vga_dac_register[reg & 3], buf, cnt);
*/		io_ins8(VIRGE_VGA_IO_BASE + virge_vga_dac_register[reg & 3], buf, cnt);

	} else {

		KRN_DEBUG(3, "DAC_MEM %.2x -> (%i bytes)", reg, cnt);
		
/*		mem_ins8(VIRGE_MMIO_BASE + VIRGE_DAC_BASE + (reg << 3), buf, cnt);
*/
	}
}

/* Clock control */
static inline void virge_chipset_clk_out8(virge_chipset_io_t *virge_io, kgi_u8_t val, kgi_u_t reg)
{
	if (reg < 7) {

		KRN_DEBUG(3, "VGA_CLK%.2x <= %.2x", reg, val);
		switch (reg) {

		case 0:
			val = (val & 3) << 2;
			MISC_OUT8(virge_io, (MISC_IN8(virge_io) & ~VGA_MISC_CLOCK_MASK) |
				val);
			return;

		case 6:
		  	SEQ_OUT8(virge_io, 
				 (SEQ_IN8(virge_io, 0x15) & ~0x23) |
				 (val & 0x23), 0x15);
			return;

		default:
			SEQ_OUT8(virge_io, val, reg + 0x0F);
		}

	} else {

		KRN_DEBUG(0,"invalid CLK register index (%i)", reg);
	}
}

static inline kgi_u8_t virge_chipset_clk_in8(virge_chipset_io_t *virge_io, kgi_u_t reg)
{
	if (reg < 7) {

		switch (reg) {

		case 0:
			return (MISC_IN8(virge_io) >> 2) & 3;

		case 6:
			return SEQ_IN8(virge_io, 0x15) & ~0x23;

		default:
			return SEQ_IN8(virge_io, reg + 0x0F);
		}

	} else {

		KRN_DEBUG(0,"invalid CLK register index (%i)", reg);
		return 0xFF;
	}
}


kgi_error_t virge_chipset_init_module(virge_chipset_t *virge, virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	static const kgi_u32_t virge_chipset_pcicfg[] =
        {
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_VX),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_DXGX),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_GX2),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MX),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXP),
                PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXPMV),
                PCICFG_SIGNATURE(0,0)
        };
	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;
	
	KRN_DEBUG(2, "virge_chipset_init_module()");

	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);
	KRN_ASSERT(options);

	/*	Auto-detect/verify the chipset
	*/
	if (pcidev == PCICFG_NULL) {		

		if (pcicfg_find_device(&pcidev, virge_chipset_pcicfg)) {

			KRN_DEBUG(2, "No supported device found!");
			
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}
	
	kgim_memset(virge, 0, sizeof(*virge));
	
	virge->chipset.revision		= KGIM_CHIPSET_REVISION;
	virge->chipset.mode_size	= sizeof(virge_chipset_mode_t);
	virge->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	virge->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(virge->chipset.vendor_id,
		virge->chipset.device_id)) {

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE):
		kgim_strcpy(virge->chipset.vendor, "S3");
		kgim_strcpy(virge->chipset.model,  "ViRGE");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000; */
		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_VX):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE VX");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.mclk.max = 50000000; */
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_DXGX):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE GX / DX");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 170000000;
/*		virge->chipset.mclk.max = 66000000; */
		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_GX2):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE GX2");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.lclk.max = 66000000; */
		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MX):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE MX");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.lclk.max = 50000000; */
		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXP):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE MXP");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.lclk.max = 50000000;	*/
		break;

        case PCICFG_SIGNATURE(PCI_VENDOR_ID_S3, PCI_DEVICE_ID_S3_ViRGE_MXPMV):
                kgim_strcpy(virge->chipset.vendor, "S3");
                kgim_strcpy(virge->chipset.model,  "ViRGE MXPMV");
                virge->chipset.maxdots.x = 2048;
                virge->chipset.maxdots.y = 2048;
                virge->chipset.dclk.max = 135000000;
/*		virge->chipset.lclk.max = 50000000;	*/
                break;

        default:
                KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).",
                        virge->chipset.vendor_id, virge->chipset.device_id);
                return -KGI_ERRNO(CHIPSET, NOSUP);
        }

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(2, "subvendor %.4x, subdevice %.4x", subvendor, subdevice);

        /*	save initial PCICFG state
	*/
        virge->pci.Command      = pcicfg_in16(pcidev + PCI_COMMAND);
        virge->pci.LatTimer     = pcicfg_in8(pcidev + PCI_LATENCY_TIMER);
        virge->pci.IntLine      = pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
        virge->pci.BaseAddr0    = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
        virge->pci.RomAddr      = pcicfg_in32(pcidev + PCI_ROM_ADDRESS);

	/*	Initialize driver claimed regions and I/O binding
	*/
	virge_io->vga.kgim.pcidev = pcidev;

	virge_io->aperture.name		= "ViRGE aperture";
	virge_io->aperture.device	= pcidev;
	virge_io->aperture.base_virt	= MEM_NULL;
	virge_io->aperture.base_io	= virge->pci.BaseAddr0 & ~(VIRGE_Base0_Size - 1);
	virge_io->aperture.size		= VIRGE_Base0_Size;
	virge_io->aperture.decode	= MEM_DECODE_ALL;

	virge_io->irq.flags = IF_SHARED_IRQ;
	virge_io->irq.name = "ViRGE interrupt line";
	virge_io->irq.line = pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
	virge_io->irq.meta = virge;
	virge_io->irq.meta_io = virge_io;
	virge_io->irq.High = (irq_handler_fn *) virge_chipset_irq_handler;

	virge_io->vga.ports.name       	= "ViRGE VGA IO";
	virge_io->vga.ports.device	= pcidev;
	virge_io->vga.ports.base_virt	= IO_NULL;
	virge_io->vga.ports.base_io	= VGA_IO_BASE;
	virge_io->vga.ports.size       	= VGA_IO_SIZE;
	
	virge_io->vga.aperture.name      = "ViRGE VGA text aperture";
	virge_io->vga.aperture.device    = pcidev;
	virge_io->vga.aperture.base_virt = MEM_NULL;
	virge_io->vga.aperture.base_io   = VGA_TEXT_MEM_BASE;
	virge_io->vga.aperture.size      = VGA_TEXT_MEM_SIZE;
	virge_io->vga.aperture.decode    = MEM_DECODE_ALL;
	
	/*	Make sure no other driver is serving the chipset
	*/
	if (virge->pci.Command & PCI_COMMAND_IO) {

		if (io_check_region(&virge_io->vga.ports)) {

			KRN_ERROR("%s region served (maybe another driver?).", 
				virge_io->vga.ports.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	if (mem_check_region(&virge_io->vga.aperture)) {

		KRN_ERROR("%s region already served!", 
			virge_io->vga.aperture.name);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	if (virge->pci.Command & PCI_COMMAND_MEMORY) {

		if (mem_check_region(&virge_io->aperture)) {

			KRN_ERROR("%s region already served!",
				virge_io->aperture.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
	}

	/*	If root specified new base addresses, he knows the
	**	consequences. If not, it's not our fault...
	*/
#define	SET_BASE(region, addr, size)					\
	if (addr) {							\
		region.base_io = addr & ~(size - 1);			\
		KRN_DEBUG(1, "region.base_io = %.8x", region.base_io);	\
	}

	SET_BASE(virge_io->aperture, options->pci->base0, VIRGE_Base0_Size);

#undef SET_BASE
	
	/*	Make sure the memory regions are free
	*/
	if (mem_check_region(&virge_io->aperture)) {

		KRN_ERROR("Check of ViRGE PCI LAW region failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	if (mem_check_region(&virge_io->vga.aperture)) {

		KRN_ERROR("Check of ViRGE VGA memory region failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	/*	Claim the regions 
	*/
	io_claim_region(&virge_io->vga.ports);
	mem_claim_region(&virge_io->vga.aperture);
	mem_claim_region(&virge_io->aperture);

	if (KGI_EOK == irq_claim_line(&virge_io->irq)) {

		KRN_DEBUG(2, "Interrupt line claimed successfully");
		virge->flags |= VIRGE_CF_IRQ_CLAIMED;
	}
	
	virge->vga.chipset.maxdots.x = virge->chipset.maxdots.x;
	virge->vga.chipset.maxdots.y = virge->chipset.maxdots.y;
	virge->vga.chipset.memory    = virge->chipset.memory;
	virge->vga.chipset.dclk.max  = virge->chipset.dclk.max;
	virge->vga.chipset.dclk.min  = virge->chipset.dclk.min;

	virge_io->vga.kgim.DacOut8  = (void *) virge_chipset_dac_out8;
	virge_io->vga.kgim.DacIn8   = (void *) virge_chipset_dac_in8;
	virge_io->vga.kgim.DacOuts8 = (void *) virge_chipset_dac_outs8;
	virge_io->vga.kgim.DacIns8  = (void *) virge_chipset_dac_ins8;

	virge_io->vga.kgim.ClkOut8  = (void *) virge_chipset_clk_out8;
	virge_io->vga.kgim.ClkIn8   = (void *) virge_chipset_clk_in8;
	
	virge_io->vga.kgim.DDCInit	= (void *) virge_chipset_ddc1_init;
	virge_io->vga.kgim.DDCDone	= (void *) virge_chipset_ddc1_done;
	virge_io->vga.kgim.DDCSetSCL	= (void *) virge_chipset_ddc1_set_scl;
	virge_io->vga.kgim.DDCGetSDA	= (void *) virge_chipset_ddc1_get_sda;
	
	virge_io->vga.SeqOut8   = (void *) virge_chipset_vga_seq_out8;
	virge_io->vga.SeqIn8    = (void *) virge_chipset_vga_seq_in8;
	virge_io->vga.CrtOut8   = (void *) virge_chipset_vga_crt_out8;
	virge_io->vga.CrtIn8    = (void *) virge_chipset_vga_crt_in8;
	virge_io->vga.GrcOut8   = (void *) virge_chipset_vga_grc_out8;
	virge_io->vga.GrcIn8    = (void *) virge_chipset_vga_grc_in8;
	virge_io->vga.AtcOut8   = (void *) virge_chipset_vga_atc_out8;
	virge_io->vga.AtcIn8    = (void *) virge_chipset_vga_atc_in8;
	virge_io->vga.MiscOut8  = (void *) virge_chipset_vga_misc_out8;
	virge_io->vga.MiscIn8   = (void *) virge_chipset_vga_misc_in8;
	virge_io->vga.FctrlOut8 = (void *) virge_chipset_vga_fctrl_out8;
	virge_io->vga.FctrlIn8  = (void *) virge_chipset_vga_fctrl_in8;

	virge_io->mmio_out16	= (void *) virge_chipset_mmio_out16;
	virge_io->mmio_in16	= (void *) virge_chipset_mmio_in16;
	
	virge_io->mmio_out32	= (void *) virge_chipset_mmio_out32;
	virge_io->mmio_in32	= (void *) virge_chipset_mmio_in32;
	
	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER, 
		virge->chipset.vendor, virge->chipset.model);
	
	return KGI_EOK;
}

void virge_chipset_done_module(virge_chipset_t *virge, 
	virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	KRN_DEBUG(2, "virge_chipset_done_module()");
	
	if (virge->flags & VIRGE_CF_IRQ_CLAIMED) {

		irq_free_line(&virge_io->irq);
	}

	mem_free_region(&virge_io->aperture);
	mem_free_region(&virge_io->vga.aperture);
	if (io_check_region(&virge_io->vga.ports)) 
		io_free_region(&virge_io->vga.ports);
}

const kgim_meta_t virge_chipset_meta =
{
	(kgim_meta_init_module_fn *)	virge_chipset_init_module,
	(kgim_meta_done_module_fn *)	virge_chipset_done_module,
	(kgim_meta_init_fn *)		virge_chipset_init,
	(kgim_meta_done_fn *)		virge_chipset_done,
	(kgim_meta_mode_check_fn *)	virge_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	virge_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	virge_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	virge_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	virge_chipset_mode_leave,
	(kgim_meta_image_resource_fn *)	virge_chipset_image_resource,

	sizeof(virge_chipset_t),
	sizeof(virge_chipset_io_t),
	sizeof(virge_chipset_mode_t)
};
