/* ----------------------------------------------------------------------------
**	ATI Rage 128 chipset implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128-meta.c,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Paul_Redmond
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/R128.h"
#include "clock/ATI/R128.h"
#include "chipset/ATI/R128-meta.h"
#include "chipset/ATI/R128-microcode.h"

static void r128_flush_pixel_cache(r128_chipset_io_t *r128_io)
{
	kgi_u_t i;
	
	R128_CTRL_OUT32(r128_io, 
		R128_184_PC_FLUSH_GUIMask | R128_184_PC_RI_GUIMask |
		R128_184_PC_FLUSH_NONGUIMask | R128_184_PC_RI_NONGUIMask,
		R128_PC_NGUI_CTLSTAT);
	
	for (i = 0; i < 16384; i++) {
	
		if (R128_CTRL_IN32(r128_io, R128_PC_NGUI_CTLSTAT) & 
			R128_1748_PC_BUSY)
			break;
	}
}

static kgi_error_t r128_wait_for_fifo(r128_chipset_io_t *r128_io, kgi_u_t entries)
{
	kgi_u_t i;
	
	for (i = 0; i < 5000; i++) {
	
		if (R128_CTRL_IN32(r128_io, R128_PM4_STAT) & 0xFFF < entries)
			continue;
		
		return -KGI_EOK;
	}
	
	return -KGI_ERRNO(CHIPSET, UNKNOWN);
}

static kgi_error_t r128_wait_for_idle(r128_chipset_io_t *r128_io)
{
	kgi_u_t i;
	
	if (r128_wait_for_fifo(r128_io, 192) != KGI_EOK) {
	
		KRN_DEBUG(1, "wait_for_fifo failed");
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}
	
	for (i = 0; i < 5000; i++) {
	
		if (R128_CTRL_IN32(r128_io, R128_PM4_STAT) &
			(R128_7B4_PM4_BUSY | R128_7B4_GUI_ACTIVE))
			continue;
			
		r128_flush_pixel_cache(r128_io);
		return -KGI_EOK;
	}

	return -KGI_ERRNO(CHIPSET, UNKNOWN);
}

static void r128_load_microcode(r128_chipset_io_t *r128_io)
{
	kgi_u_t i, size = 
		sizeof(r128_cce_microcode) / sizeof(r128_cce_microcode[0]) / 2;
	kgi_u32_t *microcode = r128_cce_microcode;
	
	r128_wait_for_idle(r128_io);

	R128_CTRL_OUT32(r128_io, 0x0, R128_PM4_MICROCODE_ADDR);	
	for (i = 0; i < size; i++) {
	
		R128_CTRL_OUT32(r128_io, *microcode, R128_PM4_MICROCODE_DATAH);
		R128_CTRL_OUT32(r128_io, *microcode, R128_PM4_MICROCODE_DATAL);
	}
}

static void r128_reset_engine(r128_chipset_io_t *r128_io)
{
	kgi_u32_t clock_cntl_index, mclk_cntl, gen_reset_cntl;
	
	r128_flush_pixel_cache(r128_io);
	
	clock_cntl_index = R128_CTRL_IN32(r128_io, R128_CLOCK_CNTL_INDEX);
	mclk_cntl = R128_PLL_IN32(r128_io, R128_MCLK_CNTL);

	R128_PLL_OUT32(r128_io, mclk_cntl | R128_0F_FORCE_GCP | 
		R128_0F_FORCE_PIPE3D_CP, R128_MCLK_CNTL);

	gen_reset_cntl = R128_CTRL_IN32(r128_io, R128_GEN_RESET_CNTL);
	
	R128_CTRL_OUT32(r128_io, mclk_cntl | R128_F0_SOFT_RESET_GUI, R128_MCLK_CNTL);
	R128_CTRL_IN32(r128_io, R128_MCLK_CNTL);
	R128_CTRL_OUT32(r128_io, mclk_cntl & ~R128_F0_SOFT_RESET_GUI, R128_MCLK_CNTL);
	R128_CTRL_IN32(r128_io, R128_MCLK_CNTL);

	R128_PLL_OUT32(r128_io, mclk_cntl, R128_MCLK_CNTL);
	R128_CTRL_OUT32(r128_io, clock_cntl_index, R128_CLOCK_CNTL_INDEX);
	R128_CTRL_OUT32(r128_io, gen_reset_cntl, R128_GEN_RESET_CNTL);
}

static void r128_init_engine(r128_chipset_t *r128, r128_chipset_io_t *r128_io, 
	r128_chipset_mode_t *r128_mode)
{
	r128_load_microcode(r128_io);
	r128_reset_engine(r128_io);
	/* PIO 192 deep fifo */
	R128_CTRL_OUT32(r128_io, 0x0, R128_PM4_BUFFER_CNTL);
	R128_CTRL_OUT32(r128_io, 0x40000000, R128_PM4_MICRO_CNTL);
	
	R128_CTRL_OUT32(r128_io, 0x0, R128_SCALE_3D_CNTL);
	r128_reset_engine(r128_io);
	
	r128_wait_for_fifo(r128_io, 1);
	R128_CTRL_OUT32(r128_io, 0x0, R128_DEFAULT_OFFSET);
	
	R128_CTRL_OUT32(r128_io, r128_mode->r128.default_pitch, R128_DEFAULT_PITCH);
	
	r128_wait_for_fifo(r128_io, 1);
	R128_CTRL_OUT32(r128_io, (0x1FFF << 16) | 0x1FFF, R128_DEFAULT_SC_BOTTOM_RIGHT);
	
	r128_wait_for_fifo(r128_io, 1);
	R128_CTRL_OUT32(r128_io, 
		0x0 | 0xD0 | (r128_mode->r128.dst_bpp << 8) | 0x3000 | 0xF00000 | 0x02000000 | 
		0x10000000 | 0x20000000 | 0x40000000, 
		R128_DP_GUI_MASTER_CNTL);
		
	r128_wait_for_fifo(r128_io, 8);
	
	R128_CTRL_OUT32(r128_io, 0x0, R128_DST_BRES_ERR);
	R128_CTRL_OUT32(r128_io, 0x0, R128_DST_BRES_INC);
	R128_CTRL_OUT32(r128_io, 0x0, R128_DST_BRES_DEC);
	
	R128_CTRL_OUT32(r128_io, 0xFFFFFFFF, R128_DP_SRC_FRGD_CLR);
	R128_CTRL_OUT32(r128_io, 0x00000000, R128_DP_SRC_BKGD_CLR);
	
	R128_CTRL_OUT32(r128_io, 0xFFFFFFFF, R128_DP_WRITE_MASK);
	
	r128_wait_for_idle(r128_io);
}

static void r128_chipset_accel_init(kgi_accel_t *accel, void *context)
{
	KRN_DEBUG(1, "accel_init");
}

static void r128_chipset_accel_done(kgi_accel_t *accel, void *context)
{
	KRN_DEBUG(1, "accel_done");
}

static void r128_chipset_accel_exec(kgi_accel_t *accel,
	kgi_accel_buffer_t *buffer)
{
	r128_chipset_io_t *r128_io = accel->meta_io;
	kgi_u_t i;
	kgi_u32_t *data = (kgi_u32_t*)buffer->aperture.virt;
	
	KRN_ASSERT(accel);
	KRN_ASSERT(buffer);
	KRN_ASSERT(data);
	
	KRN_DEBUG(1, "executing %d bytes of buffer data", buffer->execution.size);
	buffer->execution.state = KGI_AS_WAIT;
	
	for (i = 0; i < buffer->execution.size / 8 ; i++) {
	
		r128_wait_for_fifo(r128_io, 2);
		R128_CTRL_OUT32(r128_io, *data++, R128_PM4_FIFO_DATA_EVEN);
		R128_CTRL_OUT32(r128_io, *data++, R128_PM4_FIFO_DATA_ODD);
	}
	if ((buffer->execution.size % 8) == 4) {
	
		r128_wait_for_fifo(r128_io, 1);
		R128_CTRL_OUT32(r128_io, *data++, R128_PM4_FIFO_DATA_EVEN);
	}
	
	buffer->execution.state = KGI_AS_IDLE;
}

/*
**	Helper functions
*/

static inline kgi_u_t r128_next_supported_width(kgi_u_t width)
{
	return (width + 7) & ~7;
}

static inline void r128_chipset_save_mode(r128_chipset_t *r128, 
	r128_chipset_io_t *r128_io)
{
	KRN_DEBUG(2, "saving initial chipset mode state");
	r128->saved.crtc_gen_cntl = 
		R128_CTRL_IN32(r128_io, R128_CRTC_GEN_CNTL);
	r128->saved.crtc_ext_cntl = 
		R128_CTRL_IN32(r128_io, R128_CRTC_EXT_CNTL);
	r128->saved.dac_cntl = 
		R128_CTRL_IN32(r128_io, R128_DAC_CNTL);
	r128->saved.crtc_h_total_disp = 
		R128_CTRL_IN32(r128_io, R128_CRTC_H_TOTAL_DISP);
	r128->saved.crtc_h_sync_strt_wid = 
		R128_CTRL_IN32(r128_io, R128_CRTC_H_SYNC_STRT_WID);
	r128->saved.crtc_v_total_disp = 
		R128_CTRL_IN32(r128_io, R128_CRTC_V_TOTAL_DISP);
	r128->saved.crtc_v_sync_strt_wid = 
		R128_CTRL_IN32(r128_io, R128_CRTC_V_SYNC_STRT_WID);
	r128->saved.crtc_pitch = 
		R128_CTRL_IN32(r128_io, R128_CRTC_PITCH);
	r128->saved.crtc_offset = 
		R128_CTRL_IN32(r128_io, R128_CRTC_OFFSET);
	r128->saved.crtc_offset_cntl = 
		R128_CTRL_IN32(r128_io, R128_CRTC_OFFSET_CNTL);
	r128->saved.ovr_clr = 
		R128_CTRL_IN32(r128_io, R128_OVR_CLR);
	r128->saved.ovr_wid_left_right = 
		R128_CTRL_IN32(r128_io, R128_OVR_WID_LEFT_RIGHT);
	r128->saved.ovr_wid_top_bottom = 
		R128_CTRL_IN32(r128_io, R128_OVR_WID_TOP_BOTTOM);
	r128->saved.ov0_scale_cntl = 
		R128_CTRL_IN32(r128_io, R128_OV0_SCALE_CNTL);
	r128->saved.mpp_tb_config = 
		R128_CTRL_IN32(r128_io, R128_MPP_TB_CONFIG);
	r128->saved.mpp_gp_config = 
		R128_CTRL_IN32(r128_io, R128_MPP_GP_CONFIG);
	r128->saved.subpic_cntl = 
		R128_CTRL_IN32(r128_io, R128_SUBPIC_CNTL);
	r128->saved.viph_control = 
		R128_CTRL_IN32(r128_io, R128_VIPH_CONTROL);
	r128->saved.i2c_cntl_1 = 
		R128_CTRL_IN32(r128_io, R128_I2C_CNTL_1);
	r128->saved.gen_int_cntl = 
		R128_CTRL_IN32(r128_io, R128_GEN_INT_CNTL);
	r128->saved.cap0_trig_cntl = 
		R128_CTRL_IN32(r128_io, R128_CAP0_TRIG_CNTL);
	r128->saved.cap1_trig_cntl = 
		R128_CTRL_IN32(r128_io, R128_CAP1_TRIG_CNTL);
}

static inline void r128_chipset_restore_mode(r128_chipset_t *r128,
	r128_chipset_io_t *r128_io)
{
	KRN_DEBUG(1, "restoring initial chipset mode state");
	
	R128_CTRL_OUT32(r128_io,
		(R128_CTRL_IN32(r128_io, R128_CRTC_EXT_CNTL) &
		0xFFFF03FF) | R128_54_CRTC_DISPLAY_DIS, R128_CRTC_EXT_CNTL);
	
	R128_CTRL_OUT32(r128_io, r128->saved.ovr_clr, R128_OVR_CLR);
	R128_CTRL_OUT32(r128_io, r128->saved.ovr_wid_left_right, R128_OVR_WID_LEFT_RIGHT);
	R128_CTRL_OUT32(r128_io, r128->saved.ovr_wid_top_bottom, R128_OVR_WID_TOP_BOTTOM);
	R128_CTRL_OUT32(r128_io, r128->saved.ov0_scale_cntl, R128_OV0_SCALE_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.mpp_tb_config, R128_MPP_TB_CONFIG);
	R128_CTRL_OUT32(r128_io, r128->saved.mpp_gp_config, R128_MPP_GP_CONFIG);
	R128_CTRL_OUT32(r128_io, r128->saved.subpic_cntl, R128_SUBPIC_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.viph_control, R128_VIPH_CONTROL);
	R128_CTRL_OUT32(r128_io, r128->saved.i2c_cntl_1, R128_I2C_CNTL_1);
	R128_CTRL_OUT32(r128_io, r128->saved.gen_int_cntl, R128_GEN_INT_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.cap0_trig_cntl, R128_CAP0_TRIG_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.cap1_trig_cntl, R128_CAP1_TRIG_CNTL);
	
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_gen_cntl, R128_CRTC_GEN_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_ext_cntl, R128_CRTC_EXT_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.dac_cntl, R128_DAC_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_h_total_disp, R128_CRTC_H_TOTAL_DISP);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_h_sync_strt_wid, R128_CRTC_H_SYNC_STRT_WID);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_v_total_disp, R128_CRTC_V_TOTAL_DISP);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_v_sync_strt_wid, R128_CRTC_V_SYNC_STRT_WID);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_offset, R128_CRTC_OFFSET);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_offset_cntl, R128_CRTC_OFFSET_CNTL);
	R128_CTRL_OUT32(r128_io, r128->saved.crtc_pitch, R128_CRTC_PITCH);
	
	R128_CTRL_OUT32(r128_io,
		R128_CTRL_IN32(r128_io, R128_CRTC_EXT_CNTL) & 0xFFFF03FF,
		R128_CRTC_EXT_CNTL);
}

kgi_error_t r128_chipset_irq_handler(r128_chipset_t *r128, 
	r128_chipset_io_t *r128_io, irq_system_t *system)
{
	kgi_u32_t handled = 0;
#warning read IRQ status flags here.
	kgi_u32_t flags = 0;

	KRN_ASSERT(r128);
	KRN_ASSERT(r128_io);

	if (flags & ~handled) {

		KRN_ERROR("unhandled interrupt flag(s) %.8x (pcidev %.8x)",
			flags & ~handled, R128_PCIDEV(r128_io));
	}

	if (! flags) {

		KRN_ERROR("interrupt but no reason indicated.");
	}

#warning clean handled interrupts here.

	return KGI_EOK;
}

kgi_error_t r128_chipset_init(r128_chipset_t *r128,
	r128_chipset_io_t *r128_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = R128_PCIDEV(r128_io);

	KRN_ASSERT(r128);
	KRN_ASSERT(r128_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "initializing %s %s",
		r128->chipset.vendor, r128->chipset.model);

	KRN_DEBUG(2, "PCI (re-)configuration done");

	KRN_DEBUG(2, "chipset enabled");

	KRN_DEBUG(1, "detecting chipset memory size");
	r128->chipset.memory = options->chipset->memory
		? options->chipset->memory
		: (R128_CTRL_IN32(r128_io, R128_CONFIG_MEMSIZE) &
		   R128_F8_CONFIG_MEMSIZEMask);

	KRN_DEBUG(2, "detected %d MB framebuffer", 
		r128->chipset.memory / (1024*1024));

	r128_chipset_save_mode(r128, r128_io);

	vga_text_chipset_init(&r128->vga, &r128_io->vga, options);

	if (r128->flags & R128_CF_IRQ_CLAIMED) {
		
		KRN_DEBUG(1, "enabling interrupts on device");
	}

	return KGI_EOK;
}

void r128_chipset_done(r128_chipset_t *r128, 
	r128_chipset_io_t *r128_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = R128_PCIDEV(r128_io);

	if (r128->flags & R128_CF_IRQ_CLAIMED) {

#warning	disable interrupts on device here.
	}

	r128_chipset_restore_mode(r128, r128_io);
}

kgi_error_t r128_chipset_mode_check(r128_chipset_t *r128,
	r128_chipset_io_t *r128_io,
	r128_chipset_mode_t *r128_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = r128_mode->r128.kgim.crt;
	kgi_u_t bpf, bpc, bpp, width;
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t bpd;

	dpm->lclk.mul = dpm->lclk.div = 1;
	dpm->rclk.mul = dpm->rclk.div = 1;

	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	if (img[0].frames == 0) {
	
		img[0].frames = 1;
	}
	
	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
		(img[0].fam & KGI_AM_TEXTURE_INDEX)) {

		KRN_DEBUG(1, "falling back on vga text mode check");
		return vga_text_chipset_mode_check(&r128->vga, &r128_io->vga,
			&r128_mode->vga, cmd, img, images);
	}
	
	KRN_DEBUG(1, "checking image flags");
	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | 
		KGI_IF_STEREO | KGI_IF_VIRTUAL)) {

		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	check if common attributes are supported.
	*/
	KRN_DEBUG(1, "checking image common attributes");
	if (img[0].cam) {

		KRN_DEBUG(1, "common attributes %.8x not supported",
			  img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	total bits per dot
	*/
	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO)
		? (bpc + bpf*img[0].frames*2)
		: (bpc + bpf*img[0].frames);

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_DEBUG(1, "KGI_TC_PROPOSE");

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		/*	if size.x or size.y are zero, default to 640x400
		*/
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/*	if virt.x and virt.y are zero, default to size;
		**	if either virt.x xor virt.y is zero, maximize the other
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x) {

			img[0].virt.x = (8 * r128->chipset.memory) / 
				(img[0].virt.y * bpp);

			if (img[0].virt.x > r128->chipset.maxdots.x) {

				img[0].virt.x = r128->chipset.maxdots.x;
			}
		}

		if (0 == img[0].virt.y) {

			img[0].virt.y = (8 * r128->chipset.memory) /
				(img[0].virt.x * bpp);
		}

		/*	Are we beyond the limits of the H/W?
		*/
		if ((img[0].size.x >= r128->chipset.maxdots.x) || 
			(img[0].virt.x >= r128->chipset.maxdots.x)) {

			KRN_DEBUG(1, "%i (%i) horizontal pixels are too many",
				img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= r128->chipset.maxdots.y) ||
			(img[0].virt.y >= r128->chipset.maxdots.y)) {

			KRN_DEBUG(1, "%i (%i) vertical pixels are too many",
				img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 
			(8 * r128->chipset.memory)) {

			KRN_DEBUG(1, "not enough memory for (%ipf*%if + %ipc)@"
				"%ix%i", bpf, img[0].frames, bpc,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

		/*	set CRT visible fields
		*/
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		if (img[0].size.y < 400) {

			dpm->dots.y += img[0].size.y;
		}
		
		return KGI_EOK;

	case KGI_TC_LOWER:

		KRN_DEBUG(1, "KGI_TC_LOWER");

		if (dpm->dclk < r128->chipset.dclk.min) {

			KRN_DEBUG(1, "DCLK = %i Hz is too low", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		return KGI_EOK;

	case KGI_TC_RAISE:

		KRN_DEBUG(1, "KGI_TC_RAISE");

		if (dpm->dclk > r128->chipset.dclk.max) {

			KRN_DEBUG(1, "DCLK = %i Hz is too high",
				  dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_DEBUG(1, "KGI_TC_CHECK");
		
		width = r128_next_supported_width(img[0].virt.x);

		if (width != img[0].virt.x) {

			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
		if ((img[0].size.x >= r128->chipset.maxdots.x) ||
			(img[0].size.y >= r128->chipset.maxdots.y) || 
			(img[0].virt.x >= r128->chipset.maxdots.x) ||
			((img[0].virt.y * img[0].virt.x * bpp) >
				(8 * r128->chipset.memory))) {

			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
				img[0].size.x, img[0].size.y,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		break;
	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}
	
	r128_mode->r128.default_pitch = img[0].virt.x;
	switch (bpp) {
	case 8: r128_mode->r128.dst_bpp = 0x2; break;
	case 15: r128_mode->r128.dst_bpp = 0x3; break;
	case 16: r128_mode->r128.dst_bpp = 0x4; break;
	case 24: r128_mode->r128.dst_bpp = 0x5; break;
	case 32: r128_mode->r128.dst_bpp = 0x6; break;
	}
	
	KRN_DEBUG(0, "setting up mode");
	r128_mode->r128.crtc_gen_cntl = R128_50_CRTC_EXT_DISP_EN | 
		R128_50_CRTC_EN;
	switch (bpp) {
	case 4:  r128_mode->r128.crtc_gen_cntl |= 
		1 << R128_50_CRTC_PIX_WIDTHShift; break;
	case 8:  r128_mode->r128.crtc_gen_cntl |= 
		2 << R128_50_CRTC_PIX_WIDTHShift; break;
	case 15: r128_mode->r128.crtc_gen_cntl |= 
		3 << R128_50_CRTC_PIX_WIDTHShift; break;
	case 16: r128_mode->r128.crtc_gen_cntl |= 
		4 << R128_50_CRTC_PIX_WIDTHShift; break;
	case 24: r128_mode->r128.crtc_gen_cntl |= 
		5 << R128_50_CRTC_PIX_WIDTHShift; break;
	case 32: r128_mode->r128.crtc_gen_cntl |=
		6 << R128_50_CRTC_PIX_WIDTHShift; break;
	}
	/* dblscan 
	r128_mode->r128.crtc_gen_cntl |= R128_50_CRTC_DBL_SCAN_EN;
	*/
	/* interlaced 
	r128_mode->r128.crtc_gen_cntl |= R128_50_CRTC_INTERLACE_EN;
	*/
	KRN_DEBUG(1, "calculating crtc register");
	r128_mode->r128.dac_cntl = R128_CTRL_IN32(r128_io, R128_DAC_CNTL) &
		(R128_58_DAC_RANGE_CNTLMask | R128_58_DAC_BLANKING) | 
		R128_58_DAC_VGA_ADR_EN | R128_58_DAC_MASKMask;
	r128_mode->r128.dac_cntl &= ~R128_58_DAC_8BIT_EN;
	if (bpp != 8) r128_mode->r128.dac_cntl |= R128_58_DAC_8BIT_EN;
	
	r128_mode->r128.crtc_h_total_disp = 
		(((crt_mode->x.width/8)-1) << R128_200_CRTC_H_DISPShift) |
		(((crt_mode->x.total/8)-1) << R128_200_CRTC_H_TOTALShift);
		
	r128_mode->r128.crtc_h_sync_strt_wid =
		((crt_mode->x.syncstart/8) << 
		R128_204_CRTC_H_SYNC_STRT_CHARShift) |
		(((crt_mode->x.syncend - crt_mode->x.syncstart)/8) << 
		R128_204_CRTC_H_SYNC_WIDShift) |
		(crt_mode->x.polarity ? 0 : R128_204_CRTC_H_SYNC_POL);
		
	r128_mode->r128.crtc_v_total_disp =
		(crt_mode->y.width - 1) << R128_208_CRTC_V_DISPShift |
		(crt_mode->y.total - 1) << R128_208_CRTC_V_TOTALShift;

	r128_mode->r128.crtc_v_sync_strt_wid = 
		crt_mode->y.syncstart << R128_20C_CRTC_V_SYNC_STRTShift |
		(crt_mode->y.syncend - crt_mode->y.syncstart) << 
		R128_20C_CRTC_V_SYNC_WIDShift |
		(crt_mode->y.polarity ? 0 : R128_20C_CRTC_V_SYNC_POL);
		
	r128_mode->r128.crtc_pitch = 
		(img[0].size.x / 8) << R128_22C_CRTC_PITCHShift;

	/*	initialize exported resources
	*/
	KRN_DEBUG(1, "setting up framebuffer resource");
	r = &r128_mode->r128.framebuffer;
	r->meta     = r128;
	r->meta_io  = r128_io;
	r->type     = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot     = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name     = "Rage 128 video memory aperture";
	r->access   = 64 + 32 + 16 + 8;
	r->align    = 64 + 32 + 16 + 8;
	r->size     = r->win.size = r128->chipset.memory;
	r->win.bus  = r128_io->fb.base_bus;
	r->win.phys = r128_io->fb.base_phys;
	r->win.virt = r128_io->fb.base_virt;

	KRN_DEBUG(1, "setting up accelerator resource");
	a = &r128_mode->r128.accelerator;
	a->meta = r128;
	a->meta_io = r128_io;
	a->type = KGI_RT_ACCELERATOR;
	a->prot = KGI_PF_LIB | KGI_PF_DRV; 
	a->name = "rage128 accelerator";
	a->buffers = 3;
	a->buffer_size = 4096;
	a->context_size = sizeof(r128_chipset_accel_context_t);
	a->execution.context = NULL;
	a->execution.queue = NULL;	
	a->Init = r128_chipset_accel_init;
	a->Done = r128_chipset_accel_done;
	a->Exec = r128_chipset_accel_exec;

	return KGI_EOK;
}

kgi_resource_t *r128_chipset_mode_resource(r128_chipset_t *r128, 
	r128_chipset_mode_t *r128_mode, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		KRN_DEBUG(1, "falling back on vga text mode resource");
		return vga_text_chipset_mode_resource(&r128->vga, 
			&r128_mode->vga, img, images, index);
	}

	switch (index) {

	case 0:	return (kgi_resource_t *) &r128_mode->r128.framebuffer;
	case 1:	return (kgi_resource_t *) &r128_mode->r128.accelerator;
	}
	return NULL;
}

void r128_chipset_mode_prepare(r128_chipset_t *r128, r128_chipset_io_t *r128_io,
	r128_chipset_mode_t *r128_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		KRN_DEBUG(1, "disabling extended display for vga text");
		kgi_u32_t val  = R128_CTRL_IN32(r128_io, R128_CRTC_GEN_CNTL);
		val &= ~R128_50_CRTC_EXT_DISP_EN;
		R128_CTRL_OUT32(r128_io, val, R128_CRTC_GEN_CNTL);

		KRN_DEBUG(1, "falling back on vga text mode prepare");
		vga_text_chipset_mode_prepare(&r128->vga, &r128_io->vga,
			&r128_mode->vga, img, images);

		KRN_DEBUG(2, "prepared for VGA-mode");
		return;
	}

	/* disable the display */
	KRN_DEBUG(1, "disabling display");
	R128_CTRL_OUT32(r128_io,
		(R128_CTRL_IN32(r128_io, R128_CRTC_EXT_CNTL) &
		0xFFFF03FF) | R128_54_CRTC_DISPLAY_DIS, R128_CRTC_EXT_CNTL);
	
	/* set common registers */
	KRN_DEBUG(1, "setting common registers");
	R128_CTRL_OUT32(r128_io, 0x0, R128_OVR_CLR);
	R128_CTRL_OUT32(r128_io, 0x0, R128_OVR_WID_LEFT_RIGHT);
	R128_CTRL_OUT32(r128_io, 0x0, R128_OVR_WID_TOP_BOTTOM);
	R128_CTRL_OUT32(r128_io, 0x0, R128_OV0_SCALE_CNTL);
	R128_CTRL_OUT32(r128_io, 0x0, R128_MPP_TB_CONFIG);
	R128_CTRL_OUT32(r128_io, 0x0, R128_MPP_GP_CONFIG);
	R128_CTRL_OUT32(r128_io, 0x0, R128_SUBPIC_CNTL);
	R128_CTRL_OUT32(r128_io, 0x0, R128_VIPH_CONTROL);
	R128_CTRL_OUT32(r128_io, 0x0, R128_I2C_CNTL_1);
	R128_CTRL_OUT32(r128_io, 0x0, R128_GEN_INT_CNTL);
	R128_CTRL_OUT32(r128_io, 0x0, R128_CAP0_TRIG_CNTL);
	R128_CTRL_OUT32(r128_io, 0x0, R128_CAP1_TRIG_CNTL);
	
	KRN_DEBUG(2, "prepared for Rage 128 mode");
}

void r128_chipset_mode_enter(r128_chipset_t *r128, r128_chipset_io_t *r128_io,
	r128_chipset_mode_t *r128_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		KRN_DEBUG(1, "falling back on vga text mode enter");
		vga_text_chipset_mode_enter(&r128->vga, &r128_io->vga,
			&r128_mode->vga, img, images);
		return;
	}

	KRN_DEBUG(1, "programming crtc registers");
	R128_CTRL_OUT32(r128_io, r128_mode->r128.crtc_gen_cntl,
		R128_CRTC_GEN_CNTL);
	
	R128_CTRL_OUT32(r128_io, R128_CTRL_IN32(r128_io,
		R128_CRTC_EXT_CNTL) & (R128_54_CRTC_VSYNC_DIS |
		R128_54_CRTC_HSYNC_DIS | R128_54_CRTC_DISPLAY_DIS) |
		R128_54_VGA_XCRT_CNT_EN | R128_54_VGA_ATI_LINEAR,
	 	R128_CRTC_EXT_CNTL);
		
	/* TODO: is it worth moving this to the DAC driver ?? */
	R128_CTRL_OUT32(r128_io, r128_mode->r128.dac_cntl,
		R128_DAC_CNTL);
		
	R128_CTRL_OUT32(r128_io, r128_mode->r128.crtc_h_total_disp,
		R128_CRTC_H_TOTAL_DISP);

	R128_CTRL_OUT32(r128_io, r128_mode->r128.crtc_h_sync_strt_wid,
		R128_CRTC_H_SYNC_STRT_WID);

	R128_CTRL_OUT32(r128_io, r128_mode->r128.crtc_v_total_disp,
		R128_CRTC_V_TOTAL_DISP);

	R128_CTRL_OUT32(r128_io, r128_mode->r128.crtc_v_sync_strt_wid,
		R128_CRTC_V_SYNC_STRT_WID);

	R128_CTRL_OUT32(r128_io, 0x0, R128_CRTC_OFFSET);

	R128_CTRL_OUT32(r128_io, 0x0, R128_CRTC_OFFSET_CNTL);

	R128_CTRL_OUT32(r128_io, r128_mode->r128.crtc_pitch,
		R128_CRTC_PITCH);
	
	r128_init_engine(r128, r128_io, r128_mode);
	
	/* enable the display */
	KRN_DEBUG(1, "enabling display");
	R128_CTRL_OUT32(r128_io,
		R128_CTRL_IN32(r128_io, R128_CRTC_EXT_CNTL) & 0xFFFF03FF,
		R128_CRTC_EXT_CNTL);

	r128->mode = r128_mode;
}

void r128_chipset_mode_leave(r128_chipset_t *r128, r128_chipset_io_t *r128_io,
	r128_chipset_mode_t *r128_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	r128->mode = NULL;
}
