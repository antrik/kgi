/* ----------------------------------------------------------------------------
**	ViRGE meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**	Copyright (C)	2000		Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: ViRGE-meta.h,v $
**	Revision 1.2  2003/01/30 13:44:45  foske
**	Mayor cleanup, fix textmode, drop secondary card support, it wasn't going to work anyway.
**	
**	Revision 1.1.1.1  2000/08/04 11:39:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/08/04 11:39:08  seeger_s
**	- merged version posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- status is reported to be 30% by Jos.
**	
**	Revision 1.1.1.1  2000/04/18 08:51:27  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_S3_ViRGE_meta_h
#define	_chipset_S3_ViRGE_meta_h

#define MMIO_OUT32(d,r)		mem_out32(d,r+VIRGE_MMIO_BASE)		
#define MMIO_IN32(r)		mem_in32(r+VIRGE_MMIO_BASE)

#define MMIO_OUT16(d,r)		mem_out16(d,r+VIRGE_MMIO_BASE)		
#define MMIO_IN16(r)		mem_in16(r+VIRGE_MMIO_BASE)

#define MMIO_OUT8(d,r)		mem_out8(d,r+VIRGE_MMIO_BASE)		
#define MMIO_IN8(r)		mem_in8(r+VIRGE_MMIO_BASE)

#define MISC_OUT8(v,d)		v->vga.MiscOut8 (v,d)
#define MISC_IN8(v)		v->vga.MiscIn8 (v)

#define CRT_OUT8(v,d,r)		v->vga.CrtOut8 (v, d, r)
#define CRT_IN8(v,r)		v->vga.CrtIn8  (v, r)

#define SEQ_OUT8(v,d,r)		v->vga.SeqOut8 (v, d, r)
#define SEQ_IN8(v,r)		v->vga.SeqIn8  (v, r)

#define ATC_OUT8(v,d,r)		v->vga.AtcOut8 (v, d, r)
#define ATC_IN8(v,r)		v->vga.AtcIn8  (v, r)

#define GRC_OUT8(v,d,r)		v->vga.GrcOut8 (v, d, r)
#define GRC_IN8(v,r)		v->vga.GrcIn8  (v, r)

#define SPR_OUT32(v,d,r)	
	//v->vga.SprOut32 (v, d, r)
#define SPR_IN32(v,r)		
	//v->vga.SprIn32  (v, r)

#define IS_VIRGE(v)	 (PCI_DEVICE_ID_S3_ViRGE==v->chipset.device_id)
#define IS_VIRGE_DXGX(v) (PCI_DEVICE_ID_S3_ViRGE_DXGX==v->chipset.device_id)
#define IS_VIRGE_VX(v)	 (PCI_DEVICE_ID_S3_ViRGE_VX==v->chipset.device_id)
#define IS_VIRGE_GX2(v)	 (PCI_DEVICE_ID_S3_ViRGE_GX2==v->chipset.device_id)
#define IS_VIRGE_MX(v)	 (PCI_DEVICE_ID_S3_ViRGE_MX==v->chipset.device_id)
#define IS_VIRGE_MXP(v)	 (PCI_DEVICE_ID_S3_ViRGE_MXP==v->chipset.device_id)
#define IS_VIRGE_MXPMV(v) (PCI_DEVICE_ID_ViRGE_MXPMV==v->chipset.device_id)

typedef enum
{
	VIRGE_IF_VGA_DAC	= 0x00000001,
	VIRGE_IF_MMIO_ENABLED   = 0x80000000
} virge_chipset_io_flags_t;

#include "chipset/IBM/VGA-text-meta.h"

typedef	struct
{	
	vga_text_chipset_io_t  		vga;
	virge_chipset_io_flags_t	flags;
	mem_region_t			aperture;
	irq_line_t			irq;

	kgim_io_out16_fn	*mmio_out16;
	kgim_io_in16_fn		*mmio_in16;
	kgim_io_out32_fn	*mmio_out32;
	kgim_io_in32_fn		*mmio_in32;
	
} virge_chipset_io_t;

#define	VIRGE_MMIO_OUT8(ctx, val, reg)	(ctx)->mmio_out8((ctx), (val), (reg))
#define	VIRGE_MMIO_IN8(ctx, reg)      	(ctx)->mmio_in8((ctx), (reg))
#define	VIRGE_MMIO_OUT16(ctx, val, reg)	(ctx)->mmio_out16((ctx), (val), (reg))
#define	VIRGE_MMIO_IN16(ctx, reg)      	(ctx)->mmio_in16((ctx), (reg))
#define	VIRGE_MMIO_OUT32(ctx, val, reg)	(ctx)->mmio_out32((ctx), (val), (reg))
#define	VIRGE_MMIO_IN32(ctx, reg)      	(ctx)->mmio_in32((ctx), (reg))

#define	VIRGE_ACCEL_SAVE(ctx, buf)	(ctx)->AccelSave((ctx), (buf))
#define	VIRGE_PCIDEV(ctx)	       	((ctx)->vga.kgim.pcidev)

typedef struct {
	kgi_u8_t crt[ViRGE_NR_CRT_REGS];
	kgi_u8_t seq[ViRGE_NR_SEQ_REGS];
	kgi_u8_t atc[ViRGE_NR_ATC_REGS];
	kgi_u8_t grc[ViRGE_NR_GRC_REGS];
} virge_chipset_state_t;

typedef union
{
	kgim_chipset_mode_t	kgim;
	vga_text_chipset_mode_t	vga;

	struct {
		kgim_chipset_mode_t	kgim;
		
		kgi_u8_t		Misc;

		kgi_u32_t		ModeControl, ExtendedMode, Offset, 
					ExtendedRAMDACControl;
		kgi_u32_t		HTotal, HdEnd, HbStart, HsStart, HsEnd, HbEnd;
		kgi_u32_t		VTotal, VdEnd, VbStart, VsStart, VsEnd, VbEnd;
		kgi_u32_t		ScreenBase, ScreenBaseRight;
		kgi_u8_t		InterlaceRetraceStart, UnderlineLocation;
		kgi_u8_t	BackwardCompatability1, BackwardCompatability2,
				BackwardCompatability3;
		kgi_u8_t		ExtendedMiscellaneousControl1,
					ExtendedMiscellaneousControl2,
					Miscellaneous1;
		kgi_u32_t		LineCompare, StartFifoFetch,
					MaximumScanLine;	
		kgi_u8_t		CRTCModeControl, MemoryConfiguration,
					PresetRowScan;
					
		/* Sequencer registers */
		
		kgi_u8_t		ExtendedSequencerA, ExtendedSequencerB;
		kgi_u8_t		ExtendedSequencerC, ExtendedSequencerD;
		kgi_u8_t		DacClkSynControl, ClkSynControl1, ClkSynControl2;
		kgi_u8_t		ClockingMode, EnableWritePlane,
					CharacterFontSelect, MemoryModeControl;

		/* ATC registers */
		kgi_u8_t		HorizontalPixelPanning;

		/* Streams registers */
		kgi_u32_t		PrimaryStreamControl, ColorChromaKeyControl,
					SecondaryStreamControl,ChromaKeyUpperBound,
					SecondaryStreamStretch, BlendControl,
					PrimaryStreamAddress0, PrimaryStreamAddress1,
					PrimaryStreamStride, DoubleBufferLpbSupport,
					SecondaryStreamAddress0, SecondaryStreamAddress1,
					SecondaryStreamStride, OpaqueOverlayControl,
					K1VerticalScaleFactor, K2VerticalScaleFactor,
					DDAVerticalAccumulator, StreamsFifoControl,
					PrimaryStreamStartCoordinates, PrimaryStreamWindowSize,
					SecondaryStreamStartCoordinates, SecondaryStreamWindowSize,
					FifoControl, MiuControl, StreamsTimeout, MiscTimeout;
		
		kgi_mmio_region_t	aperture1, aperture2;
		kgi_mmio_region_t	gp_fifo, gp_regs;
		kgi_accel_t		accel;

	} virge;

} virge_chipset_mode_t;


/* Driver global state */
typedef enum
{
	VIRGE_CF_RESTORE_INITIAL	= 0x00000002,	/* restore init. state	*/
	VIRGE_CF_VIRGE			= 0x00000100,
	VIRGE_CF_VIRGE_VX		= 0x00000200,
	VIRGE_CF_VIRGE_DX		= 0x00000300,
	VIRGE_CF_VIRGE_GX		= 0x00000400,
	VIRGE_CF_VIRGE_GX2		= 0x00000500,
	VIRGE_CF_VIRGE_MX		= 0x00000600,
	VIRGE_CF_IRQ_CLAIMED		= 0x00001000

} virge_chipset_flags_t;

typedef struct
{
	kgim_chipset_t		chipset;
	vga_text_chipset_t	vga;

	virge_chipset_flags_t	flags;
	virge_chipset_mode_t	*mode;

	/* Saved initial state */
	virge_chipset_state_t initial_state;
	struct { 
    		kgi_u32_t BaseAddr0, BaseAddr1, BaseAddr2, BaseAddr3, BaseAddr4, BaseAddr5, RomAddr;
    		kgi_u32_t Command, LatTimer, IntLine;
    		pcicfg_vaddr_t *dev;
	} pci;
	
	struct 
	{
		kgi_u32_t
		VSY, S3D_DON, FIFO_OVF, FIFO_EMP, HD_DON, CD_DON, S3DF_FIF,
		LPB_INT, SPS,
		not_handled, no_reason;
	} interrupt;
	/* Virtual clock chip registers
	 * used with clk_[in,out]() functions
	 */
	struct
	{
		kgi_u8_t
		clockselect, mclk_low, mclk_high, dclk_low, dclk_high,
		clksyn_ctrl1, clksyn_crtl2;
	} clk;
	
	struct
	{
		kgi_u16_t
		ecr_subsys_stat,
		ecr_subsys_cntl,
		ecr_advfunc_cntl,
		ecr_cur_x,
		ecr_cur_y,
		ecr_dest_x,
		ecr_dest_y,
		ecr_axial_step,
		ecr_diag_setp,
		ecr_error_term,
		ecr_majaxis_cnt,
		ecr_gp_status,
		ecr_command,
		ecr_short_stroke,
		ecr_background,
		ecr_foreground,
		ecr_write_mask,
		ecr_read_mask,
		ecr_cmp_color,
		ecr_backgr_mix,
		ecr_foregr_mix,
		ecr_multifunc,
		ecr_pixel_trans;
	} ecr;

	struct 
	{
		kgi_u32_t 
		primary_streams_control,
		colorchromakey_control,
		secondary_streams_control,
		chroma_key_upper_bound,
		secondary_stretch_filter_constants,
		blend_control,
		primary_stream_fb_address0,
		primary_stream_fb_address1,
		primary_stream_stride,
		double_buffer_lpb_support,
		secondary_stream_fb_address0,
		secondary_stream_fb_address1,
		secondary_stream_stride,
		opaque_overlay_control,
		k1_vertical_scale_factor,
		k2_vertical_scale_factor,
		dda_initial_value,
		fifo_control,
		primary_stream_window_start_coord,
		primary_stream_window_size,
		secondary_stream_window_start_coord,
		secondary_stream_window_size;
	} streams;
	
	struct
	{
		kgi_u32_t
		fifo_control,
		mui_control,
		timeout1, timeout2,
		dmaread_baseaddr, dmaread_stridewidth;
	} mpc;
	
	struct
	{
		kgi_u32_t
		subsys_ctl,adv_func_ctl;
	} misc;

	struct
	{
		kgi_u32_t
		sysmem_addr, ctl_dir_xfer_len, phys_addr,
		writeptr, readptr, enable;
	} dma;
	
	struct
	{
		kgi_u32_t
		zbuffer_base, dest_base, clip_lr, clip_tb,
		src_dest_stride, z_stride, tex_base, tex_bdr_color,
		dc_fade_color, tex_bg_color, tex_fg_color;
	} virge_3dconfig;
	
	struct
	{
		kgi_u32_t
		dgdy_dbdy, dady_drdy, gs_bs, as_rs, dz, zstart,
		fill1, fill2, fill3, xend0, dx, xstart, ystart, ycount;
	} virge_3dline;
	
	struct
	{
		kgi_u32_t
		bv, bu, dwdx, dwdy, ws,
		dddx, dvdx, dudx, dddy, dvdy, dudy,
		ds, vs, us,
		dgdx_dbdx, dadx_drdx, dgdy_dbdy, dady_drdy,
		gs_bs, as_rs, dzdx, dzdy, zs02,
		dxdy12, xend12, dxdy01, xend01, dxdy02,
		xstart02, ystart, y01_y12;
	} virge_3dtri;
	
	struct
	{
		kgi_u32_t
		mode, outfifo_status, inter_flags, fb_addr0, fb_addr1,
		dir_readwrite_addr, dir_readwrite_data,
		genio_port, serial_port,
		videoin_winsize, videodata_off,
		h_decimation_ctrl, v_decimation_ctrl,
		line_off, outfifo_data;
	} lpb;
	
} virge_chipset_t;

KGIM_META_IRQ_HANDLER_FN(virge_chipset)
KGIM_META_INIT_FN(virge_chipset)
KGIM_META_DONE_FN(virge_chipset)
KGIM_META_MODE_CHECK_FN(virge_chipset)
KGIM_META_MODE_RESOURCE_FN(virge_chipset)
KGIM_META_MODE_PREPARE_FN(virge_chipset)
KGIM_META_MODE_ENTER_FN(virge_chipset)
KGIM_META_MODE_LEAVE_FN(virge_chipset)
KGIM_META_IMAGE_RESOURCE_FN(virge_chipset)
#endif	/* #ifdef _chipset_S3_ViRGE_meta_h */
