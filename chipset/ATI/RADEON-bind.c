/* ----------------------------------------------------------------------------
**	ATI RADEON chipset binding
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: RADEON-bind.c,v $
**	Revision 1.7  2003/08/27 21:46:44  redmondp
**	- try to read clock programming constants from the ROM
**	
**	Revision 1.6  2003/03/09 20:59:26  skids
**	
**	Add DDC2_io functions for VGA DAC DDC. (DVI and second VGA DAC not implemented)
**	Zero out chipset_io, perhaps redundant.
**	
**	Revision 1.5  2003/03/08 14:07:36  skids
**	
**	Progress to date on GC-related register save/restore.
**	
**	Revision 1.4  2002/12/09 18:31:47  fspacek
**	- radeon DDC support
**	
**	Revision 1.3  2002/09/26 04:42:51  fspacek
**	RSOD fixups, text mode fix
**	
**	Revision 1.2  2002/09/26 02:32:04  redmondp
**	Added palette support to ramdac
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#include <kgi/maintainers.h>
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.7 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#include "chipset/IBM/VGA.h"
#include "chipset/ATI/RADEON.h"
#include "chipset/ATI/RADEON-bind.h"

/*
**	radeon specific I/O functions
*/
#define	RADEON_CONTROL_BASE	radeon_io->control.base_virt

static inline void radeon_chipset_ctrl_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "Ctrl%.3x <= %.8x", reg, val);
	mem_out8(val, RADEON_CONTROL_BASE + reg);
}

static inline kgi_u8_t radeon_chipset_ctrl_in8(radeon_chipset_io_t *radeon_io,
	kgi_u32_t reg)
{
	return mem_in8(RADEON_CONTROL_BASE + reg);
}

static inline void radeon_chipset_ctrl_out32(radeon_chipset_io_t *radeon_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "Ctrl%.3x <= %.8x", reg, val);
	mem_out32(val, RADEON_CONTROL_BASE + reg);
}

static inline kgi_u32_t radeon_chipset_ctrl_in32(radeon_chipset_io_t *radeon_io,
	kgi_u32_t reg)
{
	return mem_in32(RADEON_CONTROL_BASE + reg);
}

/*
**	VGA subsystem I/O functions
*/

#define RADEON_VGA_BASE 0x3C0

static inline void radeon_chipset_vga_seq_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <= %.2x", reg, val);
	
	io_out8(reg, RADEON_VGA_BASE + VGA_SEQ_INDEX);
	io_out8(val, RADEON_VGA_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t radeon_chipset_vga_seq_in8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, RADEON_VGA_BASE + VGA_SEQ_INDEX);
	val = io_in8(RADEON_VGA_BASE + VGA_SEQ_DATA);
	
	KRN_DEBUG(3, "VGA_SEQ%.2x => %.2x", reg, val);
	
	return val;
}

static inline void radeon_chipset_vga_crt_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <= %.2x", reg, val);
	
	io_out8(reg, RADEON_VGA_BASE + VGA_CRT_INDEX);
	io_out8(val, RADEON_VGA_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t radeon_chipset_vga_crt_in8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, RADEON_VGA_BASE + VGA_CRT_INDEX);
	val = io_in8(RADEON_VGA_BASE + VGA_CRT_DATA);
	
	KRN_DEBUG(3, "VGA_CRT%.2x => %.2x", reg, val);
	
	return val;
}

static inline void radeon_chipset_vga_grc_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <= %.2x", reg, val);
	
	io_out8(reg, RADEON_VGA_BASE + VGA_GRC_INDEX);
	io_out8(val, RADEON_VGA_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t radeon_chipset_vga_grc_in8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg)
{
	kgi_u8_t val;

	io_out8(reg, RADEON_VGA_BASE + VGA_GRC_INDEX);
	val = io_in8(RADEON_VGA_BASE + VGA_GRC_DATA);

	KRN_DEBUG(3, "VGA_CRT%.2x => %.2x", reg, val);
	
	return val;
}

static inline void radeon_chipset_vga_atc_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <= %.2x", reg, val);

	io_in8(RADEON_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, RADEON_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	io_out8(val, RADEON_VGA_BASE + VGA_ATC_DATAw);
	kgi_nanosleep(250);
}

static inline kgi_u8_t radeon_chipset_vga_atc_in8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_in8(RADEON_VGA_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, RADEON_VGA_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	val = io_in8(RADEON_VGA_BASE + VGA_ATC_DATAr);
	kgi_nanosleep(250);

	KRN_DEBUG(3, "VGA_ATC%.2x => %.2x", reg, val);

	return val;
}

static inline void radeon_chipset_vga_misc_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <= %.2x", val);
	
	io_out8(val, RADEON_VGA_BASE + VGA_MISCw);
}

static inline kgi_u8_t radeon_chipset_vga_misc_in8(radeon_chipset_io_t *radeon_io)
{
	kgi_u8_t val;
	
	val = io_in8(RADEON_VGA_BASE + VGA_MISCr);
	
	KRN_DEBUG(3, "VGA_MISC => %.2x", val);
	
	return val;
}

static inline void radeon_chipset_vga_fctrl_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <= %.2x", val);
	
	io_out8(val, RADEON_VGA_BASE + VGA_FCTRLw);
}

static inline kgi_u8_t radeon_chipset_vga_fctrl_in8(radeon_chipset_io_t *radeon_io)
{
	kgi_u8_t val;
	
	val = io_in8(RADEON_VGA_BASE + VGA_FCTRLr);
	
	KRN_DEBUG(3, "VGA_FCTL => %.2x", val);
	
	return val;
}

/*
**	DAC subsystem I/O
*/
#warning	provide DAC I/O functions here.

static inline void radeon_chipset_dac_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_ASSERT(reg < RADEON_MAX_DacRegisters);

	KRN_DEBUG(3, "DAC%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t radeon_chipset_dac_in8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;
	KRN_ASSERT(reg < RADEON_MAX_DacRegisters);

	val = 0xab;
	KRN_DEBUG(3, "DAC%.2x => %.2x", reg, val);
	return val;
}

static inline void radeon_chipset_dac_outs8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < RADEON_MAX_DacRegisters);

	KRN_DEBUG(3, "DAC%.2x <= (%i bytes)", reg, cnt);
}

static inline void radeon_chipset_dac_ins8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_ASSERT(reg < RADEON_MAX_DacRegisters);

	KRN_DEBUG(3, "DAC%.2x => (%i bytes)", reg, cnt);
}

/*
**	Clock Control
*/
#warning provide clock I/O functions here.
static inline void radeon_chipset_clk_out8(radeon_chipset_io_t *radeon_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "CLK%.2x <= %.2x", reg, val);
}

static inline kgi_u8_t radeon_chipset_clk_in8(radeon_chipset_io_t *radeon_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;

	val = 0xab;
	KRN_DEBUG(3, "CLK%.2x => %.2x", reg, val);
	return val;
}

static inline void radeon_chipset_clock_out32(radeon_chipset_io_t *radeon_io,
	kgi_u32_t val, kgi_u32_t reg)
{
	KRN_DEBUG(3, "RadeonClockOut32 Idx 0x%.2x <= 0x%.8x", reg, val);
	radeon_chipset_ctrl_out8(radeon_io, (reg & RADEON_PLL_ADDRMask) |
		RADEON_PLL_WR_EN, RADEON_CLOCK_CNTL_INDEX);
	radeon_chipset_ctrl_out32(radeon_io, val, RADEON_CLOCK_CNTL_DATA);
}

static inline kgi_u32_t radeon_chipset_clock_in32(radeon_chipset_io_t *radeon_io,
	kgi_u32_t reg)
{
	kgi_u32_t val;
	
	radeon_chipset_ctrl_out8(radeon_io, reg & RADEON_PLL_ADDRMask,
		RADEON_CLOCK_CNTL_INDEX);
	val = radeon_chipset_ctrl_in32(radeon_io, RADEON_CLOCK_CNTL_DATA);
	
	KRN_DEBUG(3, "RadeonClockIn32 Idx 0x%.2x = 0x%.8x", reg, val);
	
	return val;
}

/*
**	DDC io functions
*/
static inline void radeon_chipset_ddc_init(radeon_chipset_io_t *radeon_io)
{
	kgi_u32_t val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_GEN_CNTL);
	val |= RADEON_CRTC_EXT_DISP_EN;
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_GEN_CNTL);

	val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_GEN_CNTL);
	val &= ~RADEON_CRTC_EN;
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_GEN_CNTL);
}

static inline void radeon_chipset_ddc_done(radeon_chipset_io_t *radeon_io)
{
	kgi_u32_t val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_GEN_CNTL);
	val |= RADEON_CRTC_EN;
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_GEN_CNTL);
}

static inline void radeon_chipset_ddc_set_scl(radeon_chipset_io_t *radeon_io,
	kgi_u8_t value)
{
	kgi_u32_t val=RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_V_SYNC_STRT_WID);
	val &= ~RADEON_CRTC_V_SYNC_POL;
	val |= value ? RADEON_CRTC_V_SYNC_POL : 0;
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_V_SYNC_STRT_WID);
}

static inline kgi_u8_t radeon_chipset_ddc_get_sda(radeon_chipset_io_t *radeon_io)
{
	kgi_u32_t val = RADEON_CTRL_IN32(radeon_io, RADEON_GPIO_VGA_DDC);
	return (val & RADEON_VGA_DDC_DATA_INPUT) ? 1 : 0;
}

static kgi_error_t radeon_i2c_crt1_putbits (radeon_chipset_io_t *radeon_io, 
					    kgi_u_t scl, kgi_u_t sda) { 
	/* Important: the register descriptions in the docs are deceptive
	 * to anyone who doesn't already know DDC/I2C or who doesn't
	 * have an EE degree.  Fortunately we have at least two EEs 
	 * working here at the KGI project :-)
	 *
	 * Radeon provides two ways to use I2C busses.
	 *
	 * The standard way, that is, the way that's pretty much guaranteed 
	 * never to fry anything, is to allow the pull-up resistors to
	 * bring the signal up when you want a 1.  However for high-rate
	 * data this makes the bus more prone to error.
	 *
	 * The other way is to drive the bus high for 1's.  This kills the
	 * noise/static that may occur but in the off chance that someone
	 * was clumsy enough to manufacture a device that somehow doesn't
	 * like it when you do this, you could be in for trouble.
	 *
	 * Our data rate is hardly "high rate", so we stick with the 
	 * tried and true way - we drive 0s and let the transistor's
	 * collector float up to 1s.  So we keep the drive values 
	 * (VGA_DDC_DATA_OUT and VGA_DDC_CLK_OUT) set to 0 at all times. 
	 * We enable the drive when we want a zero, and disable 
	 * it off when we want a 1.  Clear as mud?
	 */
	RADEON_CTRL_OUT32(radeon_io,
			  (scl ? 0 : RADEON_VGA_DDC_CLK_OUT_EN) |
			  (sda ? 0 : RADEON_VGA_DDC_DATA_OUT_EN), 
			  RADEON_GPIO_VGA_DDC);
	return -KGI_EOK;
}

static kgi_error_t radeon_i2c_crt1_getbits (radeon_chipset_io_t *radeon_io, 
					    kgi_u_t *scl, kgi_u_t *sda) {
	kgi_u32_t rd;

	rd = RADEON_CTRL_IN32(radeon_io, RADEON_GPIO_VGA_DDC);
	*scl = (rd & RADEON_VGA_DDC_CLK_INPUT) ? 1 : 0;
	*sda = (rd & RADEON_VGA_DDC_DATA_INPUT) ? 1 : 0;
	return -KGI_EOK;
}


/*
**	chipset_init_module()
*/
kgi_error_t radeon_chipset_init_module(radeon_chipset_t *radeon, 
	radeon_chipset_io_t *radeon_io, const kgim_options_t *options)
{
	static const kgi_u32_t radeon_chipset_pcicfg[] =
	{
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100a),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100b),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100c),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100d),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV100a),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV100b),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M6a),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M6b),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV200a),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV200b),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M7a),
		PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M7b),
		
		PCICFG_SIGNATURE(0,0)
	};

	pcicfg_vaddr_t pcidev = options->pci->dev;
	kgi_u16_t subvendor, subdevice;

	KRN_ASSERT(radeon);
	KRN_ASSERT(radeon_io);
	KRN_ASSERT(options);


	/*	auto-detect/verify the chipset
	*/
	if (pcidev == PCICFG_NULL) {

		if (pcicfg_find_device(&pcidev, radeon_chipset_pcicfg)) {

			KRN_ERROR("No supported device found!");
			return -KGI_ERRNO(CHIPSET,INVAL);
		}
	}

	/* 	radeon->chipset.memory is initialized after we have access 
	** 	to the chipset.
	*/
	kgim_memset(radeon, 0, sizeof(*radeon));
	radeon->chipset.revision	= KGIM_CHIPSET_REVISION;
	radeon->chipset.mode_size	= sizeof(radeon_chipset_mode_t);
	radeon->chipset.dclk.max	= 200000000 /* Hz */;
	radeon->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	radeon->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	switch (PCICFG_SIGNATURE(radeon->chipset.vendor_id,
		radeon->chipset.device_id)) {

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100b):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100c):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_R100d):
		kgim_strcpy(radeon->chipset.vendor, "ATI");
		kgim_strcpy(radeon->chipset.model,  "Radeon R100");
		radeon->chipset.maxdots.x = 2048;
		radeon->chipset.maxdots.y = 2048;
		radeon->chip = RADEON_CHIP_R100;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV100a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV100b):
		kgim_strcpy(radeon->chipset.vendor, "ATI");
		kgim_strcpy(radeon->chipset.model,  "Radeon RV100");
		radeon->chipset.maxdots.x = 2048;
		radeon->chipset.maxdots.y = 2048;
		radeon->chip = RADEON_CHIP_RV100;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M6a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M6b):
		kgim_strcpy(radeon->chipset.vendor, "ATI");
		kgim_strcpy(radeon->chipset.model,  "Radeon M6");
		radeon->chipset.maxdots.x = 2048;
		radeon->chipset.maxdots.y = 2048;
		radeon->chip = RADEON_CHIP_M6;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV200a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RV200b):
		kgim_strcpy(radeon->chipset.vendor, "ATI");
		kgim_strcpy(radeon->chipset.model,  "Radeon RV200");
		radeon->chipset.maxdots.x = 2048;
		radeon->chipset.maxdots.y = 2048;
		radeon->chip = RADEON_CHIP_RV200;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M7a):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_M7b):
		kgim_strcpy(radeon->chipset.vendor, "ATI");
		kgim_strcpy(radeon->chipset.model,  "Radeon M7");
		radeon->chipset.maxdots.x = 2048;
		radeon->chipset.maxdots.y = 2048;
		radeon->chip = RADEON_CHIP_M7;
		break;

	default:
		KRN_ERROR("Device not yet supported (vendor %.4x device %.4x).",
			radeon->chipset.vendor_id, radeon->chipset.device_id);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	subvendor = pcicfg_in16(pcidev + PCI_SUBSYSTEM_VENDOR_ID);
	subdevice = pcicfg_in16(pcidev + PCI_SUBSYSTEM_ID);
	KRN_DEBUG(1, "  subvendor %.4x, subdevice %.4x", subvendor, subdevice);

	/*	save initial PCICFG state
	*/
	radeon->pci.Command	= pcicfg_in32(pcidev + PCI_COMMAND);
	radeon->pci.LatTimer	= pcicfg_in32(pcidev + PCI_LATENCY_TIMER);
	radeon->pci.IntLine	= pcicfg_in32(pcidev + PCI_INTERRUPT_LINE);
	radeon->pci.BaseAddr0	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_0);
	radeon->pci.BaseAddr1	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_1);
	radeon->pci.BaseAddr2	= pcicfg_in32(pcidev + PCI_BASE_ADDRESS_2);

	/*	Initialize driver claimed regions and I/O binding.
	*/
	kgim_memset(radeon_io, 0, sizeof(*radeon_io));
	radeon_io->vga.kgim.pcidev = pcidev;

	radeon_io->fb.name      = "Radeon frame buffer";
	radeon_io->fb.device    = pcidev;
	radeon_io->fb.base_virt = MEM_NULL;
	radeon_io->fb.base_io   = radeon->pci.BaseAddr0 & ~(RADEON_Base0_Size - 1);
	radeon_io->fb.size	= RADEON_Base0_Size;
	radeon_io->fb.decode	= MEM_DECODE_ALL;
	radeon_io->fb.rid	= PCI_BASE_ADDRESS_0;

	radeon_io->control.name      = "Radeon control";
	radeon_io->control.device    = pcidev;
	radeon_io->control.base_virt = MEM_NULL;
	radeon_io->control.base_io   = radeon->pci.BaseAddr2 & ~(RADEON_Base2_Size - 1);
	radeon_io->control.size	     = RADEON_Base2_Size;
	radeon_io->control.decode    = MEM_DECODE_ALL;
	radeon_io->control.rid	     = PCI_BASE_ADDRESS_2;

	radeon_io->irq.flags	= IF_SHARED_IRQ;
	radeon_io->irq.name	= "Radeon irq line";
	radeon_io->irq.line	= radeon->pci.IntLine & 0xFF;
	radeon_io->irq.meta	= radeon;
	radeon_io->irq.meta_io	= radeon_io;
	radeon_io->irq.High	= (irq_handler_fn *)radeon_chipset_irq_handler;
	radeon_io->irq.rid	= 0;

	radeon_io->vga.ports.name 	= "Radeon VGA";
	radeon_io->vga.ports.device 	= pcidev;
	radeon_io->vga.ports.base_virt	= IO_NULL;
	radeon_io->vga.ports.base_io 	= VGA_IO_BASE;
	radeon_io->vga.ports.size 	= VGA_IO_SIZE;

	/*	make sure no other driver is serving the chipset
	*/
#if 0 /* XXX Disable VGA region check */
	if (radeon->pci.Command & PCI_COMMAND_IO) {

		if (io_check_region(&radeon_io->vga.ports)) {

			KRN_ERROR("%s region served (maybe another driver?).",
				radeon_io->vga.ports.name);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}
#endif

	/*	If root specified new base addresses, he knows the
	**	consequences. If not, it's not our fault...
	*/
#	define	SET_BASE(region, addr, size)			\
		if (addr) {					\
			region.base_io = addr & ~(size - 1);	\
		}

	SET_BASE(radeon_io->fb, options->pci->base0, RADEON_Base0_Size);
	SET_BASE(radeon_io->control, options->pci->base2, RADEON_Base2_Size);

#	undef SET_BASE

	/*	make sure the memory regions are free.
	*/
	if (mem_check_region(&radeon_io->fb) ||
		mem_check_region(&radeon_io->control)) {

		KRN_ERROR("check of radeon io or memory regions failed!");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* claim the regions & the IRQ line */
	mem_claim_region(&radeon_io->fb);
	mem_claim_region(&radeon_io->control);

	if (KGI_EOK == irq_claim_line(&radeon_io->irq)) {

		KRN_DEBUG(1, "interrupt line claimed successfully");
		radeon->flags |= RADEON_CF_IRQ_CLAIMED;
	}
	else
	{
	    KRN_ERROR("Unable to claim Radeon interrupt line!");
	}

	radeon_io->vga.aperture.name      = "VGA mmio aperture";
	radeon_io->vga.aperture.device    = pcidev;
	radeon_io->vga.aperture.base_virt = MEM_NULL;
	radeon_io->vga.aperture.base_io   = 0xB8000;
	radeon_io->vga.aperture.size      = 4096;
	radeon_io->vga.aperture.decode    = MEM_DECODE_ALL;

#if 0 /* XXX Disable VGA region claim */
	mem_claim_region(&radeon_io->vga.aperture);
#endif

	radeon_io->vga.kgim.DacOut8  = (void *) radeon_chipset_dac_out8;
	radeon_io->vga.kgim.DacIn8   = (void *) radeon_chipset_dac_in8;
	radeon_io->vga.kgim.DacOuts8 = (void *) radeon_chipset_dac_outs8;
	radeon_io->vga.kgim.DacIns8  = (void *) radeon_chipset_dac_ins8;

	radeon_io->vga.kgim.ClkOut8  = (void *) radeon_chipset_clk_out8;
	radeon_io->vga.kgim.ClkIn8   = (void *) radeon_chipset_clk_in8;

	radeon_io->vga.kgim.DDCInit	= (void *) radeon_chipset_ddc_init;
	radeon_io->vga.kgim.DDCDone	= (void *) radeon_chipset_ddc_done;
	radeon_io->vga.kgim.DDCSetSCL	= (void *) radeon_chipset_ddc_set_scl;
	radeon_io->vga.kgim.DDCGetSDA	= (void *) radeon_chipset_ddc_get_sda;
	
	radeon_io->vga.SeqOut8   = (void *) radeon_chipset_vga_seq_out8;
	radeon_io->vga.SeqIn8    = (void *) radeon_chipset_vga_seq_in8;
	radeon_io->vga.CrtOut8   = (void *) radeon_chipset_vga_crt_out8;
	radeon_io->vga.CrtIn8    = (void *) radeon_chipset_vga_crt_in8;
	radeon_io->vga.GrcOut8   = (void *) radeon_chipset_vga_grc_out8;
	radeon_io->vga.GrcIn8    = (void *) radeon_chipset_vga_grc_in8;
	radeon_io->vga.AtcOut8   = (void *) radeon_chipset_vga_atc_out8;
	radeon_io->vga.AtcIn8    = (void *) radeon_chipset_vga_atc_in8;
	radeon_io->vga.MiscOut8  = (void *) radeon_chipset_vga_misc_out8;
	radeon_io->vga.MiscIn8   = (void *) radeon_chipset_vga_misc_in8;
	radeon_io->vga.FctrlOut8 = (void *) radeon_chipset_vga_fctrl_out8;
	radeon_io->vga.FctrlIn8  = (void *) radeon_chipset_vga_fctrl_in8;

	radeon_io->CtrlOut32	= (void *) radeon_chipset_ctrl_out32;
	radeon_io->CtrlIn32	= (void *) radeon_chipset_ctrl_in32;
	radeon_io->CtrlOut8	= (void *) radeon_chipset_ctrl_out8;
	radeon_io->CtrlIn8	= (void *) radeon_chipset_ctrl_in8;
	
	radeon_io->ClockOut32	= (void *) radeon_chipset_clock_out32;
	radeon_io->ClockIn32	= (void *) radeon_chipset_clock_in32;

	radeon_io->vga.kgim.DDC2_io.PutBits = 
		(void *) radeon_i2c_crt1_putbits;
	radeon_io->vga.kgim.DDC2_io.GetBits = 
		(void *) radeon_i2c_crt1_getbits;
	/* i2c driver provides default timing paremeters. */
	radeon_io->vga.kgim.DDC2_io.chipset_io = radeon_io;

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		radeon->chipset.vendor, radeon->chipset.model);
	return KGI_EOK;
}

void radeon_chipset_done_module(radeon_chipset_t *radeon, 
	radeon_chipset_io_t *radeon_io, const kgim_options_t *options)
{
	if (radeon->flags & RADEON_CF_IRQ_CLAIMED) {

		irq_free_line(&radeon_io->irq);
	}
		
	mem_free_region(&radeon_io->control);
	mem_free_region(&radeon_io->fb);
	mem_free_region(&radeon_io->vga.aperture);
}

const kgim_meta_t radeon_chipset_meta =
{
	(kgim_meta_init_module_fn *)	radeon_chipset_init_module,
	(kgim_meta_done_module_fn *)	radeon_chipset_done_module,
	(kgim_meta_init_fn *)		radeon_chipset_init,
	(kgim_meta_done_fn *)		radeon_chipset_done,
	(kgim_meta_mode_check_fn *)	radeon_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	radeon_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	radeon_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	radeon_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	radeon_chipset_mode_leave,
	(kgim_meta_image_resource_fn *)	radeon_chipset_image_resource,

	sizeof(radeon_chipset_t),
	sizeof(radeon_chipset_io_t),
	sizeof(radeon_chipset_mode_t)
};
