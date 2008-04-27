/* ----------------------------------------------------------------------------
**	nVidia Riva TNT2 chipset meta language binding
** ----------------------------------------------------------------------------
**	Copyright	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: TNT2-bind.c,v $
**	Revision 1.2  2002/09/21 14:35:10  aldot
**	- fix ATC border color reads
**	
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:58:59  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:28  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.2 $"

#define	DEBUG_LEVEL	2

#include <kgi/module.h>

#define	__nVidia_TNT2
#include "chipset/nVidia/TNT2.h"
#include "chipset/nVidia/TNT2-meta.h"

#define	TNT2_CONTROL_BASE	(tnt2_io->control.base_virt)
#define TNT2_VGA_IO_BASE		VGA_IO_BASE
#define TNT2_VGA_MEM_BASE	(TNT2_CONTROL_BASE + TNT2_VGABase + VGA_IO_BASE)
#define	TNT2_VGA_DAC		(tnt2_io->flags & TNT2_IF_VGA_DAC)

/* TNT2 specific I/O functions */

static inline void tnt2_chipset_ctl_out32(tnt2_chipset_io_t *tnt2_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "CTL%.4x <- %.8x", reg, val);
	
	mem_out32(val, (TNT2_CONTROL_BASE + reg));
}

static inline kgi_u32_t tnt2_chipset_ctl_in32(tnt2_chipset_io_t *tnt2_io,
	kgi_u32_t reg)
{
	kgi_u32_t val;
	
	val = mem_in32(TNT2_CONTROL_BASE + reg);
	
	KRN_DEBUG(3, "CTL%.4x -> %.8x", reg, val);
	
	return val;
}

/* VGA subsystem I/O functions */

static inline void tnt2_chipset_vga_seq_out8(tnt2_chipset_io_t *tnt2_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <- %.2x", reg, val);
	
//	mem_out8(reg, TNT2_VGA_MEM_BASE + VGA_SEQ_INDEX);
//	mem_out8(val, TNT2_VGA_MEM_BASE + VGA_SEQ_DATA);

	io_out8(reg, TNT2_VGA_IO_BASE + VGA_SEQ_INDEX);
	io_out8(val, TNT2_VGA_IO_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t tnt2_chipset_vga_seq_in8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u_t reg)
{
	kgi_u8_t val;
	
//	mem_out8(reg, TNT2_VGA_MEM_BASE + VGA_SEQ_INDEX);	
//	return mem_in8(TNT2_VGA_MEM_BASE + VGA_SEQ_DATA);

	io_out8(reg, TNT2_VGA_IO_BASE + VGA_SEQ_INDEX);
	val = io_in8(TNT2_VGA_IO_BASE + VGA_SEQ_DATA);
	
	KRN_DEBUG(3, "VGA_SEQ%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void tnt2_chipset_vga_crt_out8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <- %.2x", reg, val);
	
//	mem_out8(reg, TNT2_VGA_MEM_BASE + VGA_CRT_INDEX);
//	mem_out8(val, TNT2_VGA_MEM_BASE + VGA_CRT_DATA);

	io_out8(reg, TNT2_VGA_IO_BASE + VGA_CRT_INDEX);
	io_out8(val, TNT2_VGA_IO_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t tnt2_chipset_vga_crt_in8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u_t reg)
{
	kgi_u8_t val;
	
//	mem_out8(reg, TNT2_VGA_BASE + VGA_CRT_INDEX);
//	return mem_in8(TNT2_VGA_BASE + VGA_CRT_DATA);

	io_out8(reg, TNT2_VGA_IO_BASE + VGA_CRT_INDEX);
	val = io_in8(TNT2_VGA_IO_BASE + VGA_CRT_DATA);
	
	KRN_DEBUG(3, "VGA_CRT%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void tnt2_chipset_vga_grc_out8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <- %.2x", reg, val);
	
//	mem_out8(reg, TNT2_VGA_BASE + VGA_GRC_INDEX);
//	mem_out8(val, TNT2_VGA_BASE + VGA_GRC_DATA);

	io_out8(reg, TNT2_VGA_IO_BASE + VGA_GRC_INDEX);
	io_out8(val, TNT2_VGA_IO_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t tnt2_chipset_vga_grc_in8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u_t reg)
{
	kgi_u8_t val;
	
//	mem_out8(reg, TNT2_VGA_BASE + VGA_GRC_INDEX);	
//	return mem_in8(TNT2_VGA_BASE + VGA_GRC_DATA);

	io_out8(reg, TNT2_VGA_IO_BASE + VGA_GRC_INDEX);
	val = io_in8(TNT2_VGA_IO_BASE + VGA_GRC_DATA);
	
	KRN_DEBUG(3, "VGA_GRC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void tnt2_chipset_vga_atc_out8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <- %.2x", reg, val);
	
//	mem_in8(TNT2_VGA_BASE + VGA_ATC_AFF);
//	mem_out8(reg, TNT2_VGA_BASE + VGA_ATC_INDEX);
//	mem_out8(val, TNT2_VGA_BASE + VGA_ATC_DATAw);
//	mem_out8(VGA_ATCI_ENABLE_DISPLAY, TNT2_VGA_BASE + VGA_ATC_INDEX);

//	io_out8(reg, TNT2_VGA_IO_BASE + VGA_ATC_INDEX);
//	io_out8(val, TNT2_VGA_IO_BASE + VGA_ATC_DATAw);
	
	io_in8(TNT2_VGA_IO_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, TNT2_VGA_IO_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	io_out8(val, TNT2_VGA_IO_BASE + VGA_ATC_DATAw);
	kgi_nanosleep(250);
}

static inline kgi_u8_t tnt2_chipset_vga_atc_in8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u_t reg)
{
	register kgi_u8_t val;
	
//	mem_in8(TNT2_VGA_BASE + VGA_ATC_AFF);
//	mem_out8(reg, TNT2_VGA_BASE + VGA_ATC_INDEX);
//	val = mem_in8(TNT2_VGA_BASE + VGA_ATC_DATAr);
//	mem_out8(VGA_ATCI_ENABLE_DISPLAY, TNT2_VGA_BASE + VGA_ATC_INDEX);

	io_in8(TNT2_VGA_IO_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, TNT2_VGA_IO_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	val = io_in8(TNT2_VGA_IO_BASE + VGA_ATC_DATAr);
	kgi_nanosleep(250);

	KRN_DEBUG(3, "VGA_ATC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void tnt2_chipset_vga_misc_out8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <- %.2x", val);
	
//	mem_out8(val, TNT2_VGA_BASE + VGA_MISCw);

	io_out8(val, TNT2_VGA_IO_BASE + VGA_MISCw);
}

static inline kgi_u8_t tnt2_chipset_vga_misc_in8(tnt2_chipset_io_t *tnt2_io)
{
	kgi_u8_t val;
	
	KRN_DEBUG(2, "tnt2_chipset_vga_misc()");
	
//	return mem_in8(TNT2_VGA_BASE + VGA_MISCr);

	val = io_in8(TNT2_VGA_IO_BASE + VGA_MISCr);
	
	KRN_DEBUG(3, "VGA_MISC -> %.2x", val);
	
	return val;
}

static inline void tnt2_chipset_vga_fctrl_out8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <- %.2x", val);
	
//	mem_out8(val, TNT2_VGA_BASE + VGA_FCTRLw);

	io_out8(val, TNT2_VGA_IO_BASE + VGA_FCTRLw);
}

static inline kgi_u8_t tnt2_chipset_vga_fctrl_in8(tnt2_chipset_io_t *tnt2_io)
{
	kgi_u8_t val;
	
	KRN_DEBUG(2, "tnt2_chipset_vga_fctrl_in8()");
	
//	return mem_in8(TNT2_VGA_BASE + VGA_FCTRLr);

	val = io_in8(TNT2_VGA_IO_BASE + VGA_FCTRLr);
	
	KRN_DEBUG(3, "VGA_FCTRL -> %.2x", val);
	
	return val;
}

#if 1
/* DAC subsystem I/O */
static const kgi_u_t tnt2_vga_dac_register[4] = { 0x08, 0x09, 0x06, 0x07 };

#define	tnt2_SET_DAC_ADDR23						  	\
	mem_out8(0x05, TNT2_VGA_BASE + VGA_SEQ_INDEX);		  	  	\
	mem_out8((mem_in8(TNT2_VGA_BASE + VGA_SEQ_DATA) &		  	\
		~TNT2_SR05_DACAddrMask) | ((reg << 2) & TNT2_SR05_DACAddrMask),	\
		TNT2_VGA_BASE + VGA_SEQ_DATA)

static inline void tnt2_chipset_dac_out8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg < TNT2_MAX_DACRegisters);

	if (TNT2_VGA_DAC) {

		KRN_DEBUG(3, "dac_vga %.2x <- %.2x", reg, val);
		
//		TNT2_SET_DAC_ADDR23;
//		mem_out8(val, TNT2_VGA_BASE + tnt2_vga_dac_register[reg & 3]);
		io_out8(val, TNT2_VGA_IO_BASE + tnt2_vga_dac_register[reg & 3]);

	} else {

//		KRN_DEBUG(3, "dac_mem %.2x <- %.2x", reg, val);
		
//		mem_out8(val, TNT2_CONTROL_BASE + TNT2_DACBase + (reg << 3));
	}
}

static inline kgi_u8_t tnt2_chipset_dac_in8(tnt2_chipset_io_t *tnt2_io,
	kgi_u_t reg)
{
	KRN_ASSERT(reg < TNT2_MAX_DACRegisters);

	if (TNT2_VGA_DAC) 
	{
		register kgi_u8_t val;

//		TNT2_SET_DAC_ADDR23;
		
//		val = mem_in8(TNT2_VGA_BASE + tnt2_vga_dac_register[reg & 3]);
		val = io_in8(TNT2_VGA_IO_BASE + tnt2_vga_dac_register[reg & 3]);
		
		KRN_DEBUG(3, "dac_vga %.2x -> %.2x", reg, val);
		
		return val;

	} else {

		register kgi_u8_t val = 0xff;

//		val = mem_in8(TNT2_CONTROL_BASE + TNT2_DACBase + (reg << 3));
		
//		KRN_DEBUG(3, "dac_mem %.2x -> %.2x", reg, val);
		
		return val;
	}
}

static inline void tnt2_chipset_dac_outs8(tnt2_chipset_io_t *tnt2_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < TNT2_MAX_DACRegisters);

	if (TNT2_VGA_DAC) {

		KRN_DEBUG(3, "dac_vga %.2x <- (%i bytes)", reg, cnt);
		
//		TNT2_SET_DAC_ADDR23;
		
//		mem_outs8(TNT2_VGA_BASE + tnt2_vga_dac_register[reg & 3], buf, cnt);

	} else {

		KRN_DEBUG(3, "dac_mem %.2x <- (%i bytes)", reg, cnt);
		
//		mem_outs8(TNT2_CONTROL_BASE + TNT2_DACBase + (reg << 3), buf, cnt);
	}
}

static inline void tnt2_chipset_dac_ins8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < TNT2_MAX_DACRegisters);

	if (TNT2_VGA_DAC) 
	{
		KRN_DEBUG(3, "dac_vga %.2x -> (%i bytes)", reg, cnt);
		
//		TNT2_SET_DAC_ADDR23;
		
//		mem_ins8(TNT2_VGA_BASE + tnt2_vga_dac_register[reg & 3], buf, cnt);
		io_ins8(TNT2_VGA_IO_BASE + tnt2_vga_dac_register[reg & 3], buf, cnt);
	} else {

//		KRN_DEBUG(3, "dac_mem %.2x -> (%i bytes)", reg, cnt);
		
//		mem_ins8(TNT2_CONTROL_BASE + TNT2_DACBase + (reg << 3), buf, cnt);
	}
}

/* Clock control */
static inline void tnt2_chipset_clk_out8(tnt2_chipset_io_t *tnt2_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg == 0);

	if (TNT2_VGA_DAC) {

		KRN_DEBUG(3, "VGA_CLK%.2x <= %.2x", reg, val);
		
//		mem_out8((mem_in8(TNT2_VGA_BASE + VGA_MISCr) & ~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2), TNT2_VGA_BASE + VGA_MISCw);
		io_out8((io_in8(TNT2_VGA_IO_BASE + VGA_MISCr) & ~VGA_MISC_CLOCK_MASK) | ((val & 3) << 2), TNT2_VGA_IO_BASE + VGA_MISCw);
	} else 
	{
//		register kgi_u32_t CS040 = mem_in32(TNT2_CONTROL_BASE + TNT2_ControlStatusBase + 0x040);

//		mem_out32((CS040 & ~tnt2_CS040_VClkMask) | (val & tnt2_CS040_VClkMask), TNT2_CONTROL_BASE + TNT2_ControlStatusBase + 0x040);
	}
}

static inline kgi_u8_t tnt2_chipset_clk_in8(tnt2_chipset_io_t *tnt2_io, 
	kgi_u_t reg)
{
	KRN_ASSERT(reg == 0);

	if (TNT2_VGA_DAC) {

//		return (mem_in8(TNT2_VGA_BASE + VGA_MISCr) & VGA_MISC_CLOCK_MASK) >> 2;
		return (io_in8(TNT2_VGA_IO_BASE + VGA_MISCr) & VGA_MISC_CLOCK_MASK) >> 2;
	} else 
	{
//		return mem_in32(TNT2_CONTROL_BASE + tnt2_ControlStatusBase + 0x040) & tnt2_CS040_VClkMask;
	}
	
return 0;
}
#endif

kgi_error_t tnt2_chipset_init_module(tnt2_chipset_t *tnt2, 
	tnt2_chipset_io_t *tnt2_io, const kgim_options_t *options)
{
	static const kgi_u32_t tnt2_chipset_pcicfg[] =
	{
		PCICFG_SIGNATURE(PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_TNT2),
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;
	
	KRN_DEBUG(2, "tnt2_chipset_init_module()");

	KRN_ASSERT(tnt2);
	KRN_ASSERT(tnt2_io);
	KRN_ASSERT(options);

	/* Auto-detect/verify the chipset */
	if (pcidev == PCICFG_NULL) {		

		if (pcicfg_find_device(&pcidev, tnt2_chipset_pcicfg)) {

			KRN_DEBUG(2, "No supported device found!");
			
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}
	
	kgim_memset(tnt2, 0, sizeof(*tnt2));
	
	tnt2->pci.baseaddr0 = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	tnt2->pci.baseaddr1 = pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
	
	tnt2->chipset.revision	= KGIM_CHIPSET_REVISION;
	tnt2->chipset.mode_size	= sizeof(tnt2_chipset_mode_t);
	tnt2->chipset.dclk.max	= 200000000 /* Hz */;
	tnt2->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	tnt2->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(tnt2->chipset.vendor_id, 
		tnt2->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_TNT2):
		tnt2->flags |= TNT2_CF_TNT2;
		kgim_strcpy(tnt2->chipset.vendor, "nVidia");
		kgim_strcpy(tnt2->chipset.model,  "Riva TNT2 Ultra");
		tnt2->chipset.maxdots.x = 2048;
		tnt2->chipset.maxdots.y = 2048;
		break;

	default:
		KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).", tnt2->chipset.vendor_id, tnt2->chipset.device_id);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(2, "subvendor %.4x, subdevice %.4x", subvendor, subdevice);

	/* Initialize driver claimed regions and I/O binding */
	
	tnt2_io->vga.kgim.pcidev = pcidev;

	tnt2_io->control.name      = "TNT2 control";
	tnt2_io->control.device    = pcidev;
	tnt2_io->control.base_virt = MEM_NULL;
	tnt2_io->control.base_io	  = tnt2->pci.baseaddr0 & ~(TNT2_Base0_Size - 1);
	tnt2_io->control.size	  = TNT2_Base0_Size;
	tnt2_io->control.decode	  = MEM_DECODE_ALL;
	
	tnt2_io->framebuffer.name    = "TNT2 framebuffer";
	tnt2_io->framebuffer.device  = pcidev;
	tnt2_io->framebuffer.base_virt = MEM_NULL;
	tnt2_io->framebuffer.base_io = tnt2->pci.baseaddr1 & 0xf0000000;
//	tnt2_io->framebuffer.size    = tnt2_Base1_Size;
	tnt2_io->framebuffer.size    = 32 * 1024 * 1024; /* FIXME: don't hardwire this */
	tnt2_io->framebuffer.decode  = MEM_DECODE_ALL;

	tnt2_io->irq.flags = IF_SHARED_IRQ;
	tnt2_io->irq.name = "TNT2 interrupt line";
	tnt2_io->irq.line = pcicfg_in8(pcidev + PCI_INTERRUPT_LINE);
	tnt2_io->irq.meta = (void *) tnt2;
	tnt2_io->irq.meta_io = tnt2_io;
	tnt2_io->irq.High = (irq_handler_fn *) tnt2_chipset_irq_handler;

	tnt2_io->vga.ports.name		= "TNT2 VGA IO";
	tnt2_io->vga.ports.device	= pcidev;
	tnt2_io->vga.ports.base_virt	= IO_NULL;
	tnt2_io->vga.ports.base_io	= VGA_IO_BASE;
	tnt2_io->vga.ports.size		= VGA_IO_SIZE;
	
	tnt2_io->vga.aperture.name      = "TNT2 VGA text aperture";
	tnt2_io->vga.aperture.device    = PCICFG_NULL;
	tnt2_io->vga.aperture.base_virt = MEM_NULL;
	tnt2_io->vga.aperture.base_io   = VGA_TEXT_MEM_BASE;
	tnt2_io->vga.aperture.size      = VGA_TEXT_MEM_SIZE;
	tnt2_io->vga.aperture.decode    = MEM_DECODE_ALL;
	
	/* Make sure no other driver is serving the chipset */
	if (tnt2->pci.command & PCI_COMMAND_IO) {

		if (io_check_region(&tnt2_io->vga.ports)) {

			KRN_ERROR("%s region served (maybe another driver?).",
				tnt2_io->vga.ports.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	if (mem_check_region(&tnt2_io->vga.aperture)) {

		KRN_ERROR("%s region already served!", tnt2_io->control.name);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	if (tnt2->pci.command & PCI_COMMAND_MEMORY) {

		if (mem_check_region(&tnt2_io->control)) {

			KRN_ERROR("%s region already served!",
				tnt2_io->control.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
		if (mem_check_region(&tnt2_io->framebuffer)) {

			KRN_ERROR("%s region already served!",
				tnt2_io->framebuffer.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	/*	If root specified new base addresses, he knows the
	**	consequences. If not, it's not our fault...
	*/
#	define	SET_BASE(region, addr, size)				\
	if (addr) {							\
		region.base_io = addr & ~(size - 1);			\
		KRN_DEBUG(1, "region.base_io = %.8x", region.base_io);	\
	}

	SET_BASE(tnt2_io->control, options->pci->base0, TNT2_Base0_Size);
	SET_BASE(tnt2_io->framebuffer, options->pci->base1, 32 * 1024 * 1024); /* FIXME */

#	undef SET_BASE
	
	/* Make sure the memory regions are free */
	if (mem_check_region(&tnt2_io->control)) {

		KRN_ERROR("check of TNT2 control region failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	if (mem_check_region(&tnt2_io->framebuffer)) {

		KRN_ERROR("check of TNT2 framebuffer region failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	if (mem_check_region(&tnt2_io->vga.aperture)) {

		KRN_ERROR("check of TNT2 I/O and memory regions failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	/* Claim the regions */
	io_claim_region(&tnt2_io->vga.ports);
	mem_claim_region(&tnt2_io->vga.aperture);
	mem_claim_region(&tnt2_io->control);
	mem_claim_region(&tnt2_io->framebuffer);

	if (KGI_EOK == irq_claim_line(&tnt2_io->irq)) {

		KRN_DEBUG(2, "Interrupt line claimed successfully");
		tnt2->flags |= TNT2_CF_IRQ_CLAIMED;
	}

	tnt2_io->vga.kgim.DacOut8  = (void *) tnt2_chipset_dac_out8;
	tnt2_io->vga.kgim.DacIn8   = (void *) tnt2_chipset_dac_in8;
	tnt2_io->vga.kgim.DacOuts8 = (void *) tnt2_chipset_dac_outs8;
	tnt2_io->vga.kgim.DacIns8  = (void *) tnt2_chipset_dac_ins8;

	tnt2_io->vga.kgim.ClkOut8  = (void *) tnt2_chipset_clk_out8;
	tnt2_io->vga.kgim.ClkIn8   = (void *) tnt2_chipset_clk_in8;
	
	tnt2_io->vga.SeqOut8   = (void *) tnt2_chipset_vga_seq_out8;
	tnt2_io->vga.SeqIn8    = (void *) tnt2_chipset_vga_seq_in8;
	tnt2_io->vga.CrtOut8   = (void *) tnt2_chipset_vga_crt_out8;
	tnt2_io->vga.CrtIn8    = (void *) tnt2_chipset_vga_crt_in8;
	tnt2_io->vga.GrcOut8   = (void *) tnt2_chipset_vga_grc_out8;
	tnt2_io->vga.GrcIn8    = (void *) tnt2_chipset_vga_grc_in8;
	tnt2_io->vga.AtcOut8   = (void *) tnt2_chipset_vga_atc_out8;
	tnt2_io->vga.AtcIn8    = (void *) tnt2_chipset_vga_atc_in8;
	tnt2_io->vga.MiscOut8  = (void *) tnt2_chipset_vga_misc_out8;
	tnt2_io->vga.MiscIn8   = (void *) tnt2_chipset_vga_misc_in8;
	tnt2_io->vga.FctrlOut8 = (void *) tnt2_chipset_vga_fctrl_out8;
	tnt2_io->vga.FctrlIn8  = (void *) tnt2_chipset_vga_fctrl_in8;

	tnt2_io->CTLOut32 = (void *) tnt2_chipset_ctl_out32;
	tnt2_io->CTLIn32	= (void *) tnt2_chipset_ctl_in32;
	
	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER, 
		tnt2->chipset.vendor, tnt2->chipset.model);
	
	return KGI_EOK;
}

void tnt2_chipset_done_module(tnt2_chipset_t *tnt2, tnt2_chipset_io_t *tnt2_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "tnt2_chipset_done_module()");
	
	if (tnt2->flags & TNT2_CF_IRQ_CLAIMED) {

		irq_free_line(&tnt2_io->irq);
	}

	mem_free_region(&tnt2_io->control);
	mem_free_region(&tnt2_io->framebuffer);
}

const kgim_meta_t tnt2_chipset_meta =
{
	(kgim_meta_init_module_fn *)	tnt2_chipset_init_module,
	(kgim_meta_done_module_fn *)	tnt2_chipset_done_module,
	(kgim_meta_init_fn *)		tnt2_chipset_init,
	(kgim_meta_done_fn *)		tnt2_chipset_done,
	(kgim_meta_mode_check_fn *)	tnt2_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	tnt2_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	tnt2_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	tnt2_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	tnt2_chipset_mode_leave,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(tnt2_chipset_t),
	sizeof(tnt2_chipset_io_t),
	sizeof(tnt2_chipset_mode_t)
};
