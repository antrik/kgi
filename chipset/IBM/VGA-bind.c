/* ----------------------------------------------------------------------------
**	IBM VGA chipset meta language binding
** ----------------------------------------------------------------------------
**	Copyright	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-bind.c,v $
**	Revision 1.5  2004/02/15 15:19:37  aldot
**	- save revision
**	
**	Revision 1.4  2003/11/16 21:38:32  aldot
**	- remove .resource_id added by FreeBSD without adding the field to the headers
**	
**	Revision 1.3  2003/07/26 18:59:20  cegger
**	merge fixes from FreeBSD folk
**	
**	Revision 1.2  2003/01/25 02:27:57  aldot
**	- sync up to current API, (first part -- 20021001)
**	
**	Revision 1.1.1.1  2001/07/18 14:57:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:58:59  seeger_s
**	- updated to changes in kgi/module.h
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:20  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		The_KGI_Project
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.5 $"

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include <kgi/module.h>

#include "chipset/IBM/VGA.h"
#include "chipset/IBM/VGA-bind.h"

/*	VGA subsystem I/O functions
*/
static inline void vga_chipset_seq_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_SEQ%.2x <- %.2x", reg, val);
	
	io_out8(reg, VGA_IO_BASE + VGA_SEQ_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_SEQ_DATA);
}

static inline kgi_u8_t vga_chipset_seq_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, VGA_IO_BASE + VGA_SEQ_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_SEQ_DATA);
	
	KRN_DEBUG(3, "VGA_SEQ%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_crt_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CRT%.2x <- %.2x", reg, val);
	
	io_out8(reg, VGA_IO_BASE + VGA_CRT_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_CRT_DATA);
}

static inline kgi_u8_t vga_chipset_crt_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, VGA_IO_BASE + VGA_CRT_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_CRT_DATA);
	
	KRN_DEBUG(3, "VGA_CRT%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_grc_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_GRC%.2x <- %.2x", reg, val);
	
	io_out8(reg, VGA_IO_BASE + VGA_GRC_INDEX);
	io_out8(val, VGA_IO_BASE + VGA_GRC_DATA);
}

static inline kgi_u8_t vga_chipset_grc_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	io_out8(reg, VGA_IO_BASE + VGA_GRC_INDEX);
	val = io_in8(VGA_IO_BASE + VGA_GRC_DATA);
	
	KRN_DEBUG(3, "VGA_GRC%.2x -> %.2x", reg, val);
	
	return val;
}

static inline void vga_chipset_atc_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_ATC%.2x <- %.2x", reg, val);

	/* reset to address mode */
	io_in8(VGA_IO_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, VGA_IO_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	io_out8(val, VGA_IO_BASE + VGA_ATC_DATAw);
	kgi_nanosleep(250);
}

static inline kgi_u8_t vga_chipset_atc_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	register kgi_u8_t val;

	/* reset to address mode */
	io_in8(VGA_IO_BASE + VGA_ATC_AFF);
	kgi_nanosleep(250);
	io_out8(reg, VGA_IO_BASE + VGA_ATC_INDEX);
	kgi_nanosleep(250);
	val = io_in8(VGA_IO_BASE + VGA_ATC_DATAr);
	kgi_nanosleep(250);

	KRN_DEBUG(3, "VGA_ATC%.2x -> %.2x", reg, val);
	return val;
}

static inline void vga_chipset_misc_out8(vga_chipset_io_t *vga_io, kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_MISC <- %.2x", val);
	
	io_out8(val, VGA_IO_BASE + VGA_MISCw);
}

static inline kgi_u8_t vga_chipset_misc_in8(vga_chipset_io_t *vga_io)
{
	kgi_u8_t val;
	
	val = io_in8(VGA_IO_BASE + VGA_MISCr);
	
	KRN_DEBUG(3, "VGA_MISC -> %.2x", val);
	
	return val;
}

static inline void vga_chipset_fctrl_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val)
{
	KRN_DEBUG(3, "VGA_FCTRL <- %.2x", val);
	
	io_out8(val, VGA_IO_BASE + VGA_FCTRLw);
}

static inline kgi_u8_t vga_chipset_fctrl_in8(vga_chipset_io_t *vga_io)
{
	kgi_u8_t val;
	
	val = io_in8(VGA_IO_BASE + VGA_FCTRLr);
	
	KRN_DEBUG(3, "VGA_FCTRL -> %.2x", val);
	
	return val;
}

/*	DAC subsystem I/O
*/
static const kgi_u_t vga_dac_register[4] = { 0x08, 0x09, 0x06, 0x07 };

static inline void vga_chipset_dac_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_DAC %.2x <- %.2x", reg, val);
		
	io_out8(val, VGA_IO_BASE + vga_dac_register[reg & 3]);
}

static inline kgi_u8_t vga_chipset_dac_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;

	val = io_in8(VGA_IO_BASE + vga_dac_register[reg & 3]);
		
	KRN_DEBUG(3, "VGA_DAC %.2x -> %.2x", reg, val);
		
	return val;
}

static inline void vga_chipset_dac_outs8(vga_chipset_io_t *vga_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_DEBUG(3, "VGA_DAC %.2x <- (%i bytes)", reg, cnt);
	
	io_outs8(VGA_IO_BASE + vga_dac_register[reg & 3], buf, cnt);
}

static inline void vga_chipset_dac_ins8(vga_chipset_io_t *vga_io,
	kgi_u_t reg, void *buf, kgi_u_t cnt)
{
	KRN_DEBUG(3, "VGA_DAC %.2x -> (%i bytes)", reg, cnt);
	
	io_ins8(VGA_IO_BASE + vga_dac_register[reg & 3], buf, cnt);
}

/*	Clock Control
*/

static inline void vga_chipset_clk_out8(vga_chipset_io_t *vga_io,
	kgi_u8_t val, kgi_u_t reg)
{
	KRN_DEBUG(3, "VGA_CLK%.2x <- %.2x", reg, val);
	
	io_out8((io_in8(VGA_IO_BASE + VGA_MISCr) & ~VGA_MISC_CLOCK_MASK) |
		((val & 3) << 2), VGA_IO_BASE + VGA_MISCw);
}

static inline kgi_u8_t vga_chipset_clk_in8(vga_chipset_io_t *vga_io,
	kgi_u_t reg)
{
	kgi_u8_t val;
	
	val = (io_in8(VGA_IO_BASE + VGA_MISCr) & VGA_MISC_CLOCK_MASK) >> 2;
	
	KRN_DEBUG(3, "VGA_CLK%.2x -> %.2x", reg, val);
	
	return val;
}


kgi_error_t vga_chipset_init_module(vga_chipset_t *vga,
	vga_chipset_io_t *vga_io, const kgim_options_t *options)
{
	static const kgi_u32_t vga_chipset_pcicfg[] = {
		PCI_CLASS_NOT_DEFINED_VGA,
		PCI_CLASS_DISPLAY_VGA,
		0
	};
	pcicfg_vaddr_t pcidev = options->pci->dev;

	KRN_DEBUG(2, "ENTER");

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	if (PCICFG_NULL == pcidev) {

		if (pcicfg_find_class(&pcidev, vga_chipset_pcicfg)) {

			KRN_ERROR("No supported device found");
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	}

	kgim_memset(vga, 0, sizeof(*vga));

	kgim_strcpy(vga->chipset.vendor, "IBM");
	kgim_strcpy(vga->chipset.model, "VGA");

	vga->chipset.revision	= KGIM_CHIPSET_REVISION;
	vga->chipset.flags	|= KGIM_CF_NORMAL;
	vga->chipset.mode_size	= sizeof(vga_chipset_mode_t);

	vga->chipset.vendor_id	= pcicfg_in16(pcidev + PCI_VENDOR_ID);
	vga->chipset.device_id	= pcicfg_in16(pcidev + PCI_DEVICE_ID);

	vga->chipset.maxdots.x	= 920;
	vga->chipset.maxdots.y	= 550;
	vga->chipset.memory	= 256 KB;

	vga->chipset.dclk.min	= 0;
	vga->chipset.dclk.max	= 32000000;


	vga_io->ports.name	= "VGA IO";
	vga_io->ports.device	= pcidev;
	vga_io->ports.base_virt	= IO_NULL;
	vga_io->ports.base_io	= VGA_IO_BASE;
	vga_io->ports.size	= VGA_IO_SIZE;
	vga_io->ports.decode	= IO_DECODE_ALL;
	vga_io->ports.rid	= 0;

	vga_io->text16fb.name		= "VGA text16 aperture";
	vga_io->text16fb.device		= pcidev;
	vga_io->text16fb.base_virt	= MEM_NULL;
	vga_io->text16fb.base_io	= VGA_TEXT_MEM_BASE;
	vga_io->text16fb.size		= VGA_TEXT_MEM_SIZE;
	vga_io->text16fb.decode		= MEM_DECODE_ALL;
	vga_io->text16fb.rid		= 0;

	vga_io->pixelfb.name		= "VGA pixelfb aperture";
	vga_io->pixelfb.device		= pcidev;
	vga_io->pixelfb.base_virt	= MEM_NULL;
	vga_io->pixelfb.base_io		= VGA_MEM_BASE;
	vga_io->pixelfb.size		= VGA_MEM_SIZE;
	vga_io->pixelfb.decode		= MEM_DECODE_ALL;
	vga_io->pixelfb.rid		= 1;

	vga_io->vgatxt.kgim.pcidev	= pcidev;

	vga_io->vgatxt.ports.name	= "VGA-text IO";
	vga_io->vgatxt.ports.device	= pcidev;
	vga_io->vgatxt.ports.base_virt	= IO_NULL;
	vga_io->vgatxt.ports.base_io	= VGA_IO_BASE;
	vga_io->vgatxt.ports.size	= VGA_IO_SIZE;
	vga_io->vgatxt.ports.rid	= 1;


	/*	Make sure no other driver is serving the chipset 
	*/
	if (io_check_region(&vga_io->ports)) {

		KRN_ERROR("%s region served (maybe another driver?).",
			vga_io->ports.name);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	if (mem_check_region(&vga_io->text16fb)) {

		KRN_ERROR("%s region already served!",
			vga_io->text16fb.name);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	if (mem_check_region(&vga_io->pixelfb)) {

		KRN_ERROR("%s region already served!", vga_io->pixelfb.name);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	Claim the regions
	*/
	io_claim_region(&vga_io->ports);
	mem_claim_region(&vga_io->text16fb);
	mem_claim_region(&vga_io->pixelfb);

	/*	I/O */
	vga_io->vgatxt.aperture	= vga_io->text16fb;

	vga_io->SeqOut8   = (void *) vga_chipset_seq_out8;
	vga_io->SeqIn8    = (void *) vga_chipset_seq_in8;
	vga_io->CrtOut8   = (void *) vga_chipset_crt_out8;
	vga_io->CrtIn8    = (void *) vga_chipset_crt_in8;
	vga_io->GrcOut8   = (void *) vga_chipset_grc_out8;
	vga_io->GrcIn8    = (void *) vga_chipset_grc_in8;
	vga_io->AtcOut8   = (void *) vga_chipset_atc_out8;
	vga_io->AtcIn8    = (void *) vga_chipset_atc_in8;
	vga_io->MiscOut8  = (void *) vga_chipset_misc_out8;
	vga_io->MiscIn8   = (void *) vga_chipset_misc_in8;
	vga_io->FctrlOut8 = (void *) vga_chipset_fctrl_out8;
	vga_io->FctrlIn8  = (void *) vga_chipset_fctrl_in8;

	vga_io->vgatxt.SeqOut8	= (void *) vga_chipset_seq_out8;
	vga_io->vgatxt.SeqIn8	= (void *) vga_chipset_seq_in8;
	vga_io->vgatxt.CrtOut8	= (void *) vga_chipset_crt_out8;
	vga_io->vgatxt.CrtIn8	= (void *) vga_chipset_crt_in8;
	vga_io->vgatxt.GrcOut8	= (void *) vga_chipset_grc_out8;
	vga_io->vgatxt.GrcIn8	= (void *) vga_chipset_grc_in8;
	vga_io->vgatxt.AtcOut8	= (void *) vga_chipset_atc_out8;
	vga_io->vgatxt.AtcIn8	= (void *) vga_chipset_atc_in8;
	vga_io->vgatxt.MiscOut8	= (void *) vga_chipset_misc_out8;
	vga_io->vgatxt.MiscIn8	= (void *) vga_chipset_misc_in8;
	vga_io->vgatxt.FctrlOut8= (void *) vga_chipset_fctrl_out8;
	vga_io->vgatxt.FctrlIn8	= (void *) vga_chipset_fctrl_in8;

	vga_io->vgatxt.kgim.DacOut8  = (void *) vga_chipset_dac_out8;
	vga_io->vgatxt.kgim.DacIn8   = (void *) vga_chipset_dac_in8;
	vga_io->vgatxt.kgim.DacOuts8 = (void *) vga_chipset_dac_outs8;
	vga_io->vgatxt.kgim.DacIns8  = (void *) vga_chipset_dac_ins8;
	vga_io->vgatxt.kgim.ClkOut8  = (void *) vga_chipset_clk_out8;
	vga_io->vgatxt.kgim.ClkIn8   = (void *) vga_chipset_clk_in8;

	vga_io->flags	= 0;

	KRN_DEBUG(2, "%s %s driver " KGIM_CHIPSET_DRIVER " (on %.4x %.4x)",
	vga->chipset.vendor, vga->chipset.model,
	vga->chipset.vendor_id, vga->chipset.device_id);

	return KGI_EOK;
}

void vga_chipset_done_module(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "vga_chipset_done_module()");

/*	mem_free_region(&vga_io->text16fb);
*/
/*	#warning shutting down all of vga*/
	mem_free_region(&vga_io->pixelfb);
	mem_free_region(&vga_io->text16fb);
	io_free_region(&vga_io->ports);
}

const kgim_meta_t vga_chipset_meta =
{
	(kgim_meta_init_module_fn *)	vga_chipset_init_module,
	(kgim_meta_done_module_fn *)	vga_chipset_done_module,
	(kgim_meta_init_fn *)		vga_chipset_init,
	(kgim_meta_done_fn *)		vga_chipset_done,
	(kgim_meta_mode_check_fn *)	vga_chipset_mode_check,
	(kgim_meta_mode_resource_fn *)	vga_chipset_mode_resource,
	(kgim_meta_mode_prepare_fn *)	vga_chipset_mode_prepare,
	(kgim_meta_mode_enter_fn *)	vga_chipset_mode_enter,
	(kgim_meta_mode_leave_fn *)	vga_chipset_mode_leave,
	(kgim_meta_image_resource_fn *)	vga_chipset_image_resource,

	sizeof(vga_chipset_t),
	sizeof(vga_chipset_io_t),
	sizeof(vga_chipset_mode_t)
};
