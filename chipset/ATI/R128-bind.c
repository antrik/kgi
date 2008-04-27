/* ----------------------------------------------------------------------------
**	ATI Rage 128 chipset binding
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128-bind.c,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#include "chipset/IBM/VGA.h"
#include "chipset/ATI/R128.h"
#include "chipset/ATI/R128-bind.h"

/*
**	rage 128 specific I/O functions
*/
#define	R128_CONTROL_BASE	r128_io->control.base_virt

static inline void r128_chipset_ctrl_out8(r128_chipset_io_t *r128_io,
	kgi_u8_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "Ctrl[0x%.3x] <= 0x%.2x", reg, val);
	mem_out8(val, R128_CONTROL_BASE + reg);
}

static inline kgi_u8_t r128_chipset_ctrl_in8(r128_chipset_io_t *r128_io,
	kgi_u32_t reg)
{
	return mem_in8(R128_CONTROL_BASE + reg);
}

static inline void r128_chipset_ctrl_out32(r128_chipset_io_t *r128_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "Ctrl[0x%.3x] <= 0x%.8x", reg, val);
	mem_out32(val, R128_CONTROL_BASE + reg);
}

static inline kgi_u32_t r128_chipset_ctrl_in32(r128_chipset_io_t *r128_io,
	kgi_u32_t reg)
{
	return mem_in32(R128_CONTROL_BASE + reg);
}

static inline void r128_chipset_pll_out32(r128_chipset_io_t *r128_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	R128_CTRL_OUT32(r128_io, (reg & 0x1F) | R128_08_PLL_WR_EN,
		R128_CLOCK_CNTL_INDEX);
	R128_CTRL_OUT32(r128_io, val, R128_CLOCK_CNTL_DATA);
}

static inline kgi_u32_t r128_chipset_pll_in32(r128_chipset_io_t *r128_io,
	kgi_u32_t reg)
{
	R128_CTRL_OUT32(r128_io, reg & 0x1F, R128_CLOCK_CNTL_INDEX);
	return R128_CTRL_IN32(r128_io, R128_CLOCK_CNTL_DATA);
}

/*
**	VGA subsystem I/O functions
*/
#define R128_VGA_BASE         0x3c0

static inline void r128_chipset_vga_seq_out8(r128_chipset_io_t *r128_io,
	kgi_u8_t val, kgi_u_t reg)
{
//	KRN_DEBUG(4, "VGA_SEQ%.2x <= %.2x", reg, val);
	io_out8(reg, R128_VGA_BASE + VGA_SEQ_INDEX);
	io_out8(val, R128_VGA_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t r128_chipset_vga_seq_in8(r128_chipset_io_t *r128_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_out8(reg, R128_VGA_BASE + VGA_SEQ_INDEX);
	val = io_in8(R128_VGA_BASE + VGA_SEQ_DATA);
//	KRN_DEBUG(4, "VGA_SEQ%.2x => %.2x", reg, val);
	return val;
}

static inline void r128_chipset_vga_crt_out8(r128_chipset_io_t *r128_io,
					       kgi_u8_t val, kgi_u_t reg)
{
//	KRN_DEBUG(4, "VGA_CRT%.2x <= %.2x", reg, val);
	io_out8(reg, R128_VGA_BASE + VGA_CRT_INDEX);
	io_out8(val, R128_VGA_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t r128_chipset_vga_crt_in8(r128_chipset_io_t *r128_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_out8(reg, R128_VGA_BASE + VGA_CRT_INDEX);
	val = io_in8(R128_VGA_BASE + VGA_CRT_DATA);
//	KRN_DEBUG(4, "VGA_CRT%.2x => %.2x", reg, val);
	return val;
}

static inline void r128_chipset_vga_grc_out8(r128_chipset_io_t *r128_io,
					       kgi_u8_t val, kgi_u_t reg)
{
//	KRN_DEBUG(4, "VGA_GRC%.2x <= %.2x", reg, val);
	io_out8(reg, R128_VGA_BASE + VGA_GRC_INDEX);
	io_out8(val, R128_VGA_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t r128_chipset_vga_grc_in8(r128_chipset_io_t *r128_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_out8(reg, R128_VGA_BASE + VGA_GRC_INDEX);
	val = io_in8(R128_VGA_BASE + VGA_GRC_DATA);
//	KRN_DEBUG(4, "VGA_CRT%.2x => %.2x", reg, val);
	return val;
}

static inline void r128_chipset_vga_atc_out8(r128_chipset_io_t *r128_io,
	kgi_u8_t val, kgi_u_t reg)
{
//	KRN_DEBUG(4, "VGA_ATC%.2x <= %.2x", reg, val);
	io_in8(R128_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, R128_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	io_out8(val, R128_VGA_BASE + VGA_ATC_DATAw);
	kgi_nanosleep(250);
}

static inline kgi_u8_t r128_chipset_vga_atc_in8(r128_chipset_io_t *r128_io,
						  kgi_u_t reg)
{
	register kgi_u8_t val;
	io_in8(R128_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, R128_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	val = io_in8(R128_VGA_BASE + VGA_ATC_DATAr);
	kgi_nanosleep(250);
//	KRN_DEBUG(4, "VGA_ATC%.2x => %.2x", reg, val);
	return val;
}

static inline void r128_chipset_vga_misc_out8(r128_chipset_io_t *r128_io,
	kgi_u8_t val)
{
//	KRN_DEBUG(4, "VGA_MISC <= %.2x", val);
	io_out8(val, R128_VGA_BASE + VGA_MISCw);
}

static inline kgi_u8_t r128_chipset_vga_misc_in8(r128_chipset_io_t *r128_io)
{
	register kgi_u8_t val;
	val = io_in8(R128_VGA_BASE + VGA_MISCr);
//	KRN_DEBUG(4, "VGA_MISC => %.2x", val);
	return val;
}

static inline void r128_chipset_vga_fctrl_out8(r128_chipset_io_t *r128_io,
						 kgi_u8_t val)
{
//	KRN_DEBUG(4, "VGA_FCTRL <= %.2x", val);
	io_out8(val, R128_VGA_BASE + VGA_FCTRLw );
}

static inline kgi_u8_t r128_chipset_vga_fctrl_in8(r128_chipset_io_t *r128_io)
{
	register kgi_u8_t val;
	val = io_in8(R128_VGA_BASE + VGA_FCTRLr);
//	KRN_DEBUG(4, "VGA_FCTL => %.2x", val);
	return val;
}

/*
**	DAC subsystem I/O
*/
static inline void r128_chipset_dac_out8(r128_chipset_io_t *r128_io,
	kgi_u8_t val, kgi_u_t reg)
{
//	KRN_DEBUG(3, "DAC%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t r128_chipset_dac_in8(r128_chipset_io_t *r128_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	val = 0xab;
//	KRN_DEBUG(3, "DAC%.2x => %.2x", reg, val);
	return val;
}

static inline void r128_chipset_dac_outs8(r128_chipset_io_t *r128_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
//	KRN_DEBUG(3, "DAC%.2x <= (%i bytes)", reg, cnt);
}

static inline void r128_chipset_dac_ins8(r128_chipset_io_t *r128_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
//	KRN_DEBUG(3, "DAC%.2x => (%i bytes)", reg, cnt);
}

/*
**	Clock Control
*/
static inline void r128_chipset_clk_out8(r128_chipset_io_t *r128_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "CLK%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t r128_chipset_clk_in8(r128_chipset_io_t *r128_io,
	kgi_u_t reg)
{
	return 0;
}

static inline void r128_chipset_clock_out32(r128_chipset_io_t *r128_io,
	kgi_u32_t val, kgi_u32_t reg)
{
}

static inline kgi_u32_t r128_chipset_clock_in32(r128_chipset_io_t *r128_io,
	kgi_u32_t reg)
{
	return 0;
}

/*
**	DDC io functions
*/
static inline void r128_chipset_ddc_init(r128_chipset_io_t *r128_io)
{
	kgi_u32_t val;
	
	val = R128_CTRL_IN32(r128_io, R128_CRTC_GEN_CNTL);
	val |= R128_50_CRTC_EXT_DISP_EN;
	R128_CTRL_OUT32(r128_io, val, R128_CRTC_GEN_CNTL);

	val = R128_CTRL_IN32(r128_io, R128_CRTC_GEN_CNTL);
	val &= ~R128_50_CRTC_EN;
	val &= ~R128_50_CRTC_CUR_MODEMask;
	val |= 0x3 << R128_50_CRTC_CUR_MODEShift;
	KRN_DEBUG(1, "disabling display for ddc [0x%.8x]",val);
	R128_CTRL_OUT32(r128_io, val, R128_CRTC_GEN_CNTL);
	
}

static inline void r128_chipset_ddc_done(r128_chipset_io_t *r128_io)
{
	kgi_u32_t val;

	val = R128_CTRL_IN32(r128_io, R128_CRTC_GEN_CNTL);
	val |= R128_50_CRTC_EN;
	KRN_DEBUG(1, "enabling display after ddc [0x%.8x]",val);
	R128_CTRL_OUT32(r128_io, val, R128_CRTC_GEN_CNTL);
}

static inline void r128_chipset_ddc_set_scl(r128_chipset_io_t *r128_io,
	kgi_u8_t scl)
{
	kgi_u32_t val;
	
	val = R128_CTRL_IN32(r128_io, R128_CRTC_V_SYNC_STRT_WID);
	val &= ~R128_20C_CRTC_V_SYNC_POL;
	val |= scl ? R128_20C_CRTC_V_SYNC_POL : 0x0;
	R128_CTRL_OUT32(r128_io, val, R128_CRTC_V_SYNC_STRT_WID);
}

static inline kgi_u8_t r128_chipset_ddc_get_sda(r128_chipset_io_t *r128_io)
{
	kgi_u32_t val;

	val = R128_CTRL_IN32(r128_io, R128_GPIO_MONID);
	KRN_DEBUG(1,"GET_SDA=0x%.8x", val);
	return ((val & 0x00000100) ? 1 : 0);
}

static kgi_error_t r128_i2c_crt1_putbits(r128_chipset_io_t *r128_io, 
					    kgi_u_t scl, kgi_u_t sda) 
{
	kgi_u32_t val;
	
	val = R128_CTRL_IN32(r128_io, R128_GPIO_MONID);
	val &= ~((0x8|0x1) << R128_68_MONID_ENShift);
	val |= (((scl & 0x1) << 3) | (sda & 0x1)) << R128_68_MONID_ENShift;
	R128_CTRL_OUT32(r128_io, val, R128_GPIO_MONID);
	
	return -KGI_EOK;
}

static kgi_error_t r128_i2c_crt1_getbits(r128_chipset_io_t *r128_io, 
					    kgi_u_t *scl, kgi_u_t *sda) 
{
	kgi_u32_t val;
	
	val = R128_CTRL_IN32(r128_io, R128_GPIO_MONID) >> R128_68_MONID_YShift;
	*scl = val & 0x8 ? 1 : 0;
	*sda = val & 0x1 ? 1 : 0;
	
	return -KGI_EOK;
}


/*
**	chipset_init_module()
*/
kgi_error_t r128_chipset_init_module(r128_chipset_t *r128, 
	r128_chipset_io_t *r128_io, const kgim_options_t *options)
{
	static const kgi_u32_t r128_chipset_pcicfg[] =
	{
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI,PCI_DEVICE_ID_ATI_RAGE128RE),		
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;

	KRN_ASSERT(r128);
	KRN_ASSERT(r128_io);
	KRN_ASSERT(options);

	/* auto-detect/verify the chipset */
	if (pcidev == PCICFG_NULL) {

		if (pcicfg_find_device(&pcidev, r128_chipset_pcicfg)) {

			KRN_ERROR("No supported Rage 128 device found!");
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}

	KRN_DEBUG(1, "setting up chipset");
	kgim_memset(r128, 0, sizeof(*r128));
	r128->chipset.revision	= KGIM_CHIPSET_REVISION;
	r128->chipset.mode_size	= sizeof(r128_chipset_mode_t);

	r128->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	r128->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(r128->chipset.vendor_id,
		r128->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RAGE128RE):
		kgim_strcpy(r128->chipset.vendor, "ATI");
		kgim_strcpy(r128->chipset.model,  "Rage 128 RE (PCI)");
		/* TODO: these aren't right and must be set correctly !! */
		r128->chipset.dclk.max	= 250 MHZ;
		r128->chipset.maxdots.x = 2048;
		r128->chipset.maxdots.y = 2048;
		r128->chip = R128_CHIP_RE;
		break;

	default:
		KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).",
			r128->chipset.vendor_id, r128->chipset.device_id);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(1, "  subvendor %.4x, subdevice %.4x", subvendor, subdevice);

	/* save initial PCICFG state */
	KRN_DEBUG(0, "saving pci config state");
	r128->pci.Command	= pcicfg_in32(pcidev + PCI_COMMAND);
	r128->pci.LatTimer	= pcicfg_in32(pcidev + PCI_LATENCY_TIMER);
	r128->pci.IntLine	= pcicfg_in32(pcidev + PCI_INTERRUPT_LINE);
	r128->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	r128->pci.BaseAddr2	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_2);

	/* Initialize driver claimed regions and I/O binding. */
	KRN_DEBUG(1, "setting up chipset io");
	kgim_memset(r128_io, 0, sizeof(*r128_io));
	r128_io->vga.kgim.pcidev = pcidev;

	r128_io->fb.name      = "Rage 128 frame buffer";
	r128_io->fb.device    = pcidev;
	r128_io->fb.base_virt = MEM_NULL;
	r128_io->fb.base_io   = r128->pci.BaseAddr0 & ~(R128_Base0_Size - 1);
	r128_io->fb.size	= R128_Base0_Size;
	r128_io->fb.decode	= MEM_DECODE_ALL;

	r128_io->control.name      = "Rage 128 control";
	r128_io->control.device    = pcidev;
	r128_io->control.base_virt = MEM_NULL;
	r128_io->control.base_io   = r128->pci.BaseAddr2 & ~(R128_Base2_Size - 1);
	r128_io->control.size	   = R128_Base2_Size;
	r128_io->control.decode    = MEM_DECODE_ALL;

	r128_io->irq.flags	= IF_SHARED_IRQ;
	r128_io->irq.name	= "Rage 128 irq line";
	r128_io->irq.line	= r128->pci.IntLine & 0xFF;
	r128_io->irq.meta	= r128;
	r128_io->irq.meta_io	= r128_io;
	r128_io->irq.High	= (irq_handler_fn *)r128_chipset_irq_handler;

	r128_io->vga.ports.name 	= "Rage 128 VGA";
	r128_io->vga.ports.device 	= pcidev;
	r128_io->vga.ports.base_virt	= IO_NULL;
	r128_io->vga.ports.base_io 	= VGA_IO_BASE;
	r128_io->vga.ports.size 	= VGA_IO_SIZE;

	r128_io->vga.aperture.name      = "VGA mmio aperture";
	r128_io->vga.aperture.device    = pcidev;
	r128_io->vga.aperture.base_virt = MEM_NULL;
	r128_io->vga.aperture.base_io   = 0xB8000;
	r128_io->vga.aperture.size      = 4096;
	r128_io->vga.aperture.decode    = MEM_DECODE_ALL;
	
	/* make sure no other driver is serving the chipset */
	if (r128->pci.Command & PCI_COMMAND_IO) {

		KRN_DEBUG(1, "checking io regions");
		if (io_check_region(&r128_io->vga.ports)) {

			KRN_ERROR("check of Rage 128 vga io failed");
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}
/*
	if (r128->pci.Command & PCI_COMMAND_MEMORY) {

		if (mem_check_region(&r128_io->control)) {

			KRN_ERROR("%s region already served!",
				r128_io->control.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}
*/
	/* make sure the memory regions are free */
	KRN_DEBUG(1, "checking memory regions");
	if (mem_check_region(&r128_io->fb)) {

		KRN_ERROR("check of Rage 128 fb mem failed");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	if (mem_check_region(&r128_io->control)) {

		KRN_ERROR("check of Rage 128 control mem failed");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	if (mem_check_region(&r128_io->vga.aperture)) {
	
		KRN_ERROR("check of Rage 128 vga mem failed");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* claim the regions & the IRQ line */
	KRN_DEBUG(1, "claiming memory regions");
	mem_claim_region(&r128_io->fb);
	mem_claim_region(&r128_io->control);
	mem_claim_region(&r128_io->vga.aperture);

	KRN_DEBUG(1, "claiming irq line");
	if (KGI_EOK == irq_claim_line(&r128_io->irq)) {

		KRN_DEBUG(1, "interrupt line claimed successfully");
		r128->flags |= R128_CF_IRQ_CLAIMED;
	}

	/* setup io hooks */
	r128_io->vga.kgim.DacOut8  = (void *) r128_chipset_dac_out8;
	r128_io->vga.kgim.DacIn8   = (void *) r128_chipset_dac_in8;
	r128_io->vga.kgim.DacOuts8 = (void *) r128_chipset_dac_outs8;
	r128_io->vga.kgim.DacIns8  = (void *) r128_chipset_dac_ins8;

	r128_io->vga.kgim.ClkOut8  = (void *) r128_chipset_clk_out8;
	r128_io->vga.kgim.ClkIn8   = (void *) r128_chipset_clk_in8;

	r128_io->vga.SeqOut8   = (void *) r128_chipset_vga_seq_out8;
	r128_io->vga.SeqIn8    = (void *) r128_chipset_vga_seq_in8;
	r128_io->vga.CrtOut8   = (void *) r128_chipset_vga_crt_out8;
	r128_io->vga.CrtIn8    = (void *) r128_chipset_vga_crt_in8;
	r128_io->vga.GrcOut8   = (void *) r128_chipset_vga_grc_out8;
	r128_io->vga.GrcIn8    = (void *) r128_chipset_vga_grc_in8;
	r128_io->vga.AtcOut8   = (void *) r128_chipset_vga_atc_out8;
	r128_io->vga.AtcIn8    = (void *) r128_chipset_vga_atc_in8;
	r128_io->vga.MiscOut8  = (void *) r128_chipset_vga_misc_out8;
	r128_io->vga.MiscIn8   = (void *) r128_chipset_vga_misc_in8;
	r128_io->vga.FctrlOut8 = (void *) r128_chipset_vga_fctrl_out8;
	r128_io->vga.FctrlIn8  = (void *) r128_chipset_vga_fctrl_in8;

	r128_io->CtrlOut32 = (void *) r128_chipset_ctrl_out32;
	r128_io->CtrlIn32  = (void *) r128_chipset_ctrl_in32;
	r128_io->CtrlOut8  = (void *) r128_chipset_ctrl_out8;
	r128_io->CtrlIn8   = (void *) r128_chipset_ctrl_in8;
	
	r128_io->PLLOut32 = (void *) r128_chipset_pll_out32;
	r128_io->PLLIn32  = (void *) r128_chipset_pll_in32;

#if 1
	r128_io->vga.kgim.DDCInit   = (void *) r128_chipset_ddc_init;
	r128_io->vga.kgim.DDCDone   = (void *) r128_chipset_ddc_done;
	r128_io->vga.kgim.DDCSetSCL = (void *) r128_chipset_ddc_set_scl;
	r128_io->vga.kgim.DDCGetSDA = (void *) r128_chipset_ddc_get_sda;
#endif
#if 0
	r128_io->vga.kgim.DDC2_io.PutBits = (void *) r128_i2c_crt1_putbits;
	r128_io->vga.kgim.DDC2_io.GetBits = (void *) r128_i2c_crt1_getbits;
	r128_io->vga.kgim.DDC2_io.chipset_io = r128_io;
#endif

	KRN_NOTICE("%s %s chipset driver " KGIM_CHIPSET_DRIVER,
		r128->chipset.vendor, r128->chipset.model);
	return KGI_EOK;
}

void r128_chipset_done_module(r128_chipset_t *r128, 
	r128_chipset_io_t *r128_io, const kgim_options_t *options)
{
	if (r128->flags & R128_CF_IRQ_CLAIMED) {

		KRN_DEBUG(1, "freeing irq line");
		irq_free_line(&r128_io->irq);
	}

	KRN_DEBUG(1, "freeing memory regions");
	mem_free_region(&r128_io->control);
	mem_free_region(&r128_io->fb);
	mem_free_region(&r128_io->vga.aperture);
}

const kgim_meta_t r128_chipset_meta =
{
	(kgim_meta_init_module_fn *)	r128_chipset_init_module,
	(kgim_meta_done_module_fn *)	r128_chipset_done_module,
	(kgim_meta_init_fn *)		r128_chipset_init,
	(kgim_meta_done_fn *)		r128_chipset_done,
	(kgim_meta_mode_check_fn *)	r128_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	r128_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	r128_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	r128_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	r128_chipset_mode_leave,
	(kgim_meta_image_resource_fn *)	NULL,

	sizeof(r128_chipset_t),
	sizeof(r128_chipset_io_t),
	sizeof(r128_chipset_mode_t)
};
