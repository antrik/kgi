/* ----------------------------------------------------------------------------
**	ATI Rage 128 PLL binding
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
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
#define	MAINTAINER		Paul_Redmond
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#include "clock/ATI/R128-bind.h"
#include "chipset/ATI/R128.h"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

typedef struct {
		
	kgi_u8_t	clock_chip_type;
	kgi_u8_t	struct_size;
	kgi_u8_t	accel_entry;
	kgi_u8_t	vga_entry;
	kgi_u16_t	vga_table_offset;
	kgi_u16_t	post_table_offset;
	kgi_u16_t	xclk;
	kgi_u16_t	mclk;
	kgi_u8_t	num_pll_blocks;
	kgi_u8_t	size_pll_blocks;
	kgi_u16_t	pclk_ref_freq;
	kgi_u16_t	pclk_ref_divider;
	kgi_u32_t	pclk_min_freq;
	kgi_u32_t	pclk_max_freq;
	kgi_u16_t	mclk_ref_freq;
	kgi_u16_t	mclk_ref_divider;
	kgi_u32_t	mclk_min_freq;
	kgi_u32_t	mclk_max_freq;
	kgi_u16_t	xclk_ref_freq;
	kgi_u16_t	xclk_ref_divider;
	kgi_u32_t	xclk_min_freq;
	kgi_u32_t	xclk_max_freq;
	
} __attribute__ ((packed)) r128_pll_info_t;

kgi_error_t r128_clock_read_rom(r128_clock_t *r128)
{
	const kgi_u8_t *ati_sig = "761295520";
	const kgi_u_t ati_sig_len = 9;
	kgi_u32_t seg_start;
	mem_region_t rom;
	const struct {
		kgi_u8_t *sig;
		kgi_u_t len;
	} r128_sigs[] = {
		{ "R128", 4 },
		{ "RAGE128", 7 }
	};
	
	rom.name      = "Rage 128 rom";
	rom.size      = 0x1000;
	rom.decode    = MEM_DECODE_ALL;

	for (seg_start = 0x0C0000; seg_start < 0x0F0000; seg_start += 0x01000) {

		kgi_u8_t *p;
		kgi_u_t i;

		rom.base_virt = MEM_NULL;
		rom.base_io = seg_start;
		mem_claim_region(&rom);
		p = rom.base_virt;
		
		if ((*p == 0x55) || (((*(p + 1)) & 0xFF) == 0xAA))
			goto find_ati_sig;
			
		goto not_found;
		
	find_ati_sig:
		KRN_DEBUG(1, "Found 0x55AA at 0x%.8x", p);
		p = rom.base_virt;
		
		for (i = 0; i < 118; i++) {
		
			if (*p == ati_sig[0])
				if (kgim_memcmp(ati_sig, p, ati_sig_len) == 0)
					goto find_r128_sig;
			p++;
		}
		goto not_found;
		
	find_r128_sig:
		KRN_DEBUG(1, "Found ATI signature at 0x%.8x", p);
		p = rom.base_virt;
		
		for (i = 0; i < 512; i++) {
		
			kgi_u_t n;
			for (n = 0; n < (sizeof(r128_sigs) / 
				sizeof(r128_sigs[0])); n++) {
			
				if (*p == r128_sigs[n].sig[0]) {
				
					if (kgim_memcmp(r128_sigs[n].sig, p, 
						r128_sigs[n].len) == 0) {
						
						goto found;
					}
				}
			}
			p++;
		}
		goto not_found;	
		
	found:
		KRN_DEBUG(1, "Found Rage 128 signature at 0x%.8x", p);
		{
		kgi_u32_t addr;
		void *ptr;
		kgi_u16_t bios_header_offset, pll_info_offset;
		r128_pll_info_t *pll_info;
		
		addr = (kgi_u32_t)rom.base_virt + 0x48;
		ptr = (void*)addr;
		bios_header_offset = mem_in16(ptr);
		
		addr = (kgi_u32_t)rom.base_virt + (kgi_u32_t)bios_header_offset + 0x30;
		ptr = (void*)addr;
		pll_info_offset = mem_in16(ptr);
		
		addr = (kgi_u32_t)rom.base_virt + (kgi_u32_t)pll_info_offset;
		ptr = (void*)addr;
		pll_info = ptr;

		r128->xclk 		= pll_info->xclk * 10 KHZ;
		r128->pclk_ref_freq     = pll_info->pclk_ref_freq * 10 KHZ;
		r128->pclk_min_freq     = pll_info->pclk_min_freq * 10 KHZ;
		r128->pclk_max_freq     = pll_info->pclk_max_freq * 10 KHZ;
		r128->pclk_ref_divider  = pll_info->pclk_ref_divider;
#if 1
		KRN_DEBUG(1, "PLL Information");
		KRN_DEBUG(1, "clock_chip_type   =%d", pll_info->clock_chip_type);
		KRN_DEBUG(1, "struct_size       =%d", pll_info->struct_size);
		KRN_DEBUG(1, "accel_entry       =%d", pll_info->accel_entry);
		KRN_DEBUG(1, "vga_entry         =%d", pll_info->vga_entry);
		KRN_DEBUG(1, "vga_table_offset  =%d", pll_info->vga_table_offset);
		KRN_DEBUG(1, "post_table_offset =%d", pll_info->post_table_offset);
		KRN_DEBUG(1, "xclk              =%d", pll_info->xclk);
		KRN_DEBUG(1, "mclk              =%d", pll_info->mclk);
		KRN_DEBUG(1, "num_pll_blocks    =%d", pll_info->num_pll_blocks);
		KRN_DEBUG(1, "size_pll_blocks   =%d", pll_info->size_pll_blocks);
		KRN_DEBUG(1, "pclk_ref_freq     =%d", pll_info->pclk_ref_freq);
		KRN_DEBUG(1, "pclk_ref_divider  =%d", pll_info->pclk_ref_divider);
		KRN_DEBUG(1, "pclk_min_freq     =%d", pll_info->pclk_min_freq);
		KRN_DEBUG(1, "pclk_max_freq     =%d", pll_info->pclk_max_freq);
		KRN_DEBUG(1, "mclk_ref_freq     =%d", pll_info->mclk_ref_freq);
		KRN_DEBUG(1, "mclk_ref_divider  =%d", pll_info->mclk_ref_divider);
		KRN_DEBUG(1, "mclk_min_freq     =%d", pll_info->mclk_min_freq);
		KRN_DEBUG(1, "mclk_max_freq     =%d", pll_info->mclk_max_freq);
		KRN_DEBUG(1, "xclk_ref_freq     =%d", pll_info->xclk_ref_freq);
		KRN_DEBUG(1, "xclk_ref_divider  =%d", pll_info->xclk_ref_divider);
		KRN_DEBUG(1, "xclk_min_freq     =%d", pll_info->xclk_min_freq);
		KRN_DEBUG(1, "xclk_max_freq     =%d", pll_info->xclk_max_freq);
#endif
		}
		mem_free_region(&rom);
		return -KGI_EOK;
		
	not_found:
		mem_free_region(&rom);
	}
	
	KRN_DEBUG(1, "No ATI Rage 128 ROM found");
	return -KGI_ERRNO(CHIPSET, INVAL);
}

kgi_error_t r128_clock_init_module(r128_clock_t *r128,
				     r128_clock_io_t *r128_io,
				     const kgim_options_t *options)
{
	kgi_u16_t vendor_id, device_id;
  
	vendor_id = pcicfg_in16(R128_PCIDEV(r128_io) + PCI_VENDOR_ID);
	device_id = pcicfg_in16(R128_PCIDEV(r128_io) + PCI_DEVICE_ID);

	KRN_DEBUG(1, "setting up clock");
	switch (PCICFG_SIGNATURE(vendor_id, device_id)) {
    
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RAGE128RE):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RAGE128RF):
	
		r128->mem_type = R128_MEM_128BIT_SGRAM;
		break;

	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RAGE128RK):
	case PCICFG_SIGNATURE(PCI_VENDOR_ID_ATI, PCI_DEVICE_ID_ATI_RAGE128RL):

		r128->mem_type = R128_MEM_64BIT_SGRAM;
		break;

	default:
		KRN_NOTICE("No ATI Rage 128 Integrated Clock detected");
		return -KGI_ERRNO(RAMDAC, NODEV);
	}
  
	kgim_strcpy(r128->clock.vendor, "ATI");
	kgim_strcpy(r128->clock.model, "Rage 128");

	r128->clock.revision = KGIM_CLOCK_REVISION;
	r128->clock.mode_size = sizeof(r128_clock_mode_t);
  	r128->clock.type = KGIM_CT_PROG;
  	r128->clock.dclk.range[0].min =   5 MHZ;
	r128->clock.dclk.range[0].max = 135 MHZ;

	if (r128_clock_read_rom(r128) != KGI_EOK)
	{
		/* default values for when the rom can't be read */
		r128->xclk           	=  75 MHZ;
		r128->pclk_ref_freq	=  27 MHZ;
		r128->pclk_min_freq	= 125 MHZ;
		r128->pclk_max_freq	= 250 MHZ;
		r128->pclk_ref_divider	= 60;
	}
	
	KRN_NOTICE("%s %s driver " KGIM_CLOCK_DRIVER, 
		   r128->clock.vendor, r128->clock.model);
  
	return KGI_EOK;
}

const kgim_meta_t r128_clock_meta =
{
	(kgim_meta_init_module_fn *)    r128_clock_init_module,
	(kgim_meta_done_module_fn *)    NULL,
	(kgim_meta_init_fn *)           r128_clock_init,
	(kgim_meta_done_fn *)           r128_clock_done,
	(kgim_meta_mode_check_fn *)     r128_clock_mode_check,
	(kgim_meta_mode_resource_fn *)  NULL,
	(kgim_meta_mode_prepare_fn *)   NULL,
	(kgim_meta_mode_enter_fn *)     r128_clock_mode_enter,
	(kgim_meta_mode_leave_fn *)     NULL,
	(kgim_meta_image_resource_fn *) NULL,
  
	sizeof(r128_clock_t),
	0,
	sizeof(r128_clock_mode_t)
};
