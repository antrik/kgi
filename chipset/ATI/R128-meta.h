/* ----------------------------------------------------------------------------
**	ATI Rage 128 chipset definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128-meta.h,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#ifndef	_chipset_ATI_R128_meta_h
#define	_chipset_ATI_R128_meta_h

/**	If you need VGA text mode support, you must use the VGA-text-meta
***	language. Please delete this comment then or both this comment and
***	the #include directive below if you don't need VGA text mode support.
**/
#include "chipset/IBM/VGA-text-meta.h"

typedef enum
{
	R128_IF_VGA_DAC	= 0x00000001	/* use VGA or extended DAC */

} r128_chipset_io_flags_t;

/**	The structs below are intended as a sample framework and should
***	guide you what to put where. Please add the appropriate items
***	and delete the guiding comments around.
**/
typedef	struct
{
	vga_text_chipset_io_t	vga;

	r128_chipset_io_flags_t	flags;

	mem_region_t		control;	/* control MMIO aperture */
	mem_region_t		fb;		/* framebuffer MMIO aperture */
	irq_line_t		irq;		/* IRQ line */

	kgim_io_out32_fn	*CtrlOut32;
	kgim_io_in32_fn		*CtrlIn32;
	kgim_io_out8_fn		*CtrlOut8;
	kgim_io_in8_fn		*CtrlIn8;
	kgim_io_out32_fn	*PLLOut32;
	kgim_io_in32_fn		*PLLIn32;

} r128_chipset_io_t;


#define	R128_CTRL_OUT32(ctx, val, reg)	(ctx)->CtrlOut32((ctx),(val),(reg))
#define	R128_CTRL_IN32(ctx, reg)	(ctx)->CtrlIn32((ctx),(reg))
#define	R128_CTRL_OUT8(ctx, val, reg)	(ctx)->CtrlOut8((ctx),(val),(reg))
#define	R128_CTRL_IN8(ctx, reg)		(ctx)->CtrlIn8((ctx),(reg))
#define R128_PLL_OUT32(ctx, val, reg)	(ctx)->PLLOut32((ctx),(val),(reg))
#define R128_PLL_IN32(ctx, reg)		(ctx)->PLLIn32((ctx),(reg)) 
#define	R128_PCIDEV(ctx)		((ctx)->vga.kgim.pcidev)

typedef struct
{
	kgi_accel_context_t kgi;

} r128_chipset_accel_context_t;

typedef union
{
	kgim_chipset_mode_t	kgim;
	vga_text_chipset_mode_t	vga;

	struct {

		kgim_chipset_mode_t	kgim;

		kgi_u32_t		crtc_gen_cntl,
					dac_cntl,
					crtc_h_total_disp,
					crtc_h_sync_strt_wid,
					crtc_v_total_disp,
					crtc_v_sync_strt_wid,
					crtc_pitch,
					default_pitch,
					dst_bpp;
					
		kgi_mmio_region_t	framebuffer;
		kgi_accel_t		accelerator;

	} r128;

} r128_chipset_mode_t;


/*	driver global state
*/
typedef enum
{
	R128_CF_RESTORE_INITIAL	= 0x00000001,	/* restore init. state	*/
	R128_CF_IRQ_CLAIMED	= 0x00000002	/* IRQ line claimed	*/

} r128_chipset_flags_t;

typedef enum {

	R128_CHIP_RE,
	R128_CHIP_RF,
	R128_CHIP_RK,
	R128_CHIP_RL
	
} r128_chipset_chip_t;

typedef struct
{
	kgim_chipset_t		chipset;

	vga_text_chipset_t	vga;

	r128_chipset_chip_t	chip;
	r128_chipset_flags_t	flags;	/* driver global flags	*/
	r128_chipset_mode_t	*mode;	/* current mode (if set) */

	struct { kgi_u32_t

		Command, LatTimer, IntLine, BaseAddr0, BaseAddr2;
	} pci;
	
	struct { kgi_u32_t
	 
		crtc_gen_cntl,
		crtc_ext_cntl,
		dac_cntl,
		crtc_h_total_disp,
		crtc_h_sync_strt_wid,
		crtc_v_total_disp,
		crtc_v_sync_strt_wid,
		crtc_pitch,
		crtc_offset,
		crtc_offset_cntl,
		ovr_clr,
		ovr_wid_left_right,
		ovr_wid_top_bottom,
		ov0_scale_cntl,
		mpp_tb_config,
		mpp_gp_config,
		subpic_cntl,
		viph_control,
		i2c_cntl_1,
		gen_int_cntl,
		cap0_trig_cntl,
		cap1_trig_cntl;
	} saved;
	
} r128_chipset_t;

KGIM_META_IRQ_HANDLER_FN(r128_chipset)
KGIM_META_INIT_FN(r128_chipset)
KGIM_META_DONE_FN(r128_chipset)
KGIM_META_MODE_CHECK_FN(r128_chipset)
KGIM_META_MODE_RESOURCE_FN(r128_chipset)
KGIM_META_MODE_PREPARE_FN(r128_chipset)
KGIM_META_MODE_ENTER_FN(r128_chipset)
KGIM_META_MODE_LEAVE_FN(r128_chipset)

#endif	/* #ifdef _chipset_ATI_r128_meta_h */
