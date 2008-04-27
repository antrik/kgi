/* ----------------------------------------------------------------------------
**	PERMEDIA meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: PERMEDIA-meta.h,v $
**	Revision 1.1.1.1  2001/10/01 14:16:07  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/09/17 20:36:30  seeger_s
**	- fixed namespace clash with linux kernel #defines
**	- fixed accelerator implementation
**	
**	Revision 1.2  2001/07/03 08:53:51  seeger_s
**	- implemented image resources and export
**	
**	Revision 1.1.1.1  2000/04/18 08:51:17  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#ifndef	_chipset_3Dlabs_PERMEDIA_meta_h
#define	_chipset_3Dlabs_PERMEDIA_meta_h

#include "chipset/IBM/VGA-text-meta.h"

typedef enum
{
	PGC_IF_VGA_DAC	= 0x00000001

} pgc_chipset_io_flags_t;

typedef	struct
{
	vga_text_chipset_io_t	vga;

	pgc_chipset_io_flags_t	flags;

	io_region_t		auxiliary;	/* Permedia1 only	*/
	mem_region_t		delta;		/* Permedia1 only	*/
	mem_region_t		control;
	mem_region_t		aperture1;
	mem_region_t		aperture2;
	mem_region_t		rom;
	irq_line_t		irq;	

	kgim_io_out32_fn	*CSOut32;
	kgim_io_in32_fn		*CSIn32;

	kgim_io_out32_fn	*MCOut32;
	kgim_io_in32_fn		*MCIn32;

	kgim_io_out32_fn	*VCOut32;
	kgim_io_in32_fn		*VCIn32;

} pgc_chipset_io_t;

#define	PGC_CS_OUT32(ctx, val, reg)	(ctx)->CSOut32((ctx), (val), (reg))
#define	PGC_CS_IN32(ctx, reg)		(ctx)->CSIn32((ctx), (reg))
#define	PGC_MC_OUT32(ctx, val, reg)	(ctx)->MCOut32((ctx), (val), (reg))
#define	PGC_MC_IN32(ctx, reg)		(ctx)->MCIn32((ctx), (reg))
#define	PGC_VC_OUT32(ctx, val, reg)	(ctx)->VCOut32((ctx), (val), (reg))
#define	PGC_VC_IN32(ctx, reg)		(ctx)->VCIn32((ctx), (reg))
#define	PGC_ACCEL_SAVE(ctx, buf)	(ctx)->AccelSave((ctx), (buf))
#define	PGC_PCIDEV(ctx)			((ctx)->vga.kgim.pcidev)

typedef union
{
	kgim_chipset_mode_t	kgim;
	vga_text_chipset_mode_t	vga;

	struct {

		kgim_chipset_mode_t	kgim;

		kgi_u32_t		VideoControl, ScreenStride;
		kgi_u32_t		HTotal, HgEnd, HsStart, HsEnd, HbEnd;
		kgi_u32_t		VTotal, VsStart, VsEnd, VbEnd;
		kgi_u32_t		ScreenBase, ScreenBaseRight;

		kgi_u_t		pp[3];

		kgi_mmio_region_t	aperture1, aperture2;
		kgi_mmio_region_t	gp_fifo, gp_regs;
		kgi_accel_t		gp_accel;

	} pgc;

} pgc_chipset_mode_t;


/*	driver global state
*/
typedef enum
{
	PGC_CF_RESTORE_INITIAL	= 0x00000002,	/* restore init. state	*/
	PGC_CF_PERMEDIA		= 0x00000100,
	PGC_CF_DELTA		= 0x00000200,
	PGC_CF_PERMEDIA2	= 0x00000400,
	PGC_CF_IRQ_CLAIMED	= 0x00000800

} pgc_chipset_flags_t;

typedef struct
{
	kgim_chipset_t		chipset;

	vga_text_chipset_t	vga;
	kgi_u16_t		vga_SEQ_ControlReg;
	kgi_u16_t		vga_GRC_Mode640Reg;

	pgc_chipset_flags_t	flags;
	pgc_chipset_mode_t	*mode;

#if (DEBUG_LEVEL >= 0)
	struct { 
		kgi_u_t	DMA, Sync, Error, VRetrace, Scanline, 
			TextureInvalid, BypassDMA, VSB, VSA, VideoStreamSerial,
			VideoDDC, VideoStreamExternal, not_handled, no_reason;
	} interrupt;

	struct {
		kgi_u_t	InFIFO, OutFIFO, Message, DMA, VideoFifoUnderflow,
			VSBUnderflow, VSAUnderflow, Master, OutDMA, 
			InDMAOverwrite, OutDMAOverwrite, VSAInvalidInterlace,
			VSBInvalidInterlace, unknown, no_reason;
	} error;
#endif

	/*	saved initial state
	*/
	struct { kgi_u32_t

		Command, LatTimer, IntLine, 
		BaseAddr0, BaseAddr1, BaseAddr2, BaseAddr3,
		BaseAddr4, RomAddr;

	} pci;

	struct { kgi_u32_t

		IntEnable, VClkCtl, ApertureOne, ApertureTwo,
		FIFODiscon, ChipConfig;
	
	} cs;
	
	struct { kgi_u32_t

		RomControl, BootAddress, MemConfig, BypassWriteMask,
		FramebufferWriteMask;

	} mc;

	struct { kgi_u32_t

		ScreenBase, ScreenStride, 
		HTotal, HgEnd, HbEnd, HsStart, HsEnd,
		VTotal, VbEnd, VsStart, VsEnd, VideoControl,
		InterruptLine;

	} vc;

} pgc_chipset_t;

KGIM_META_IRQ_HANDLER_FN(pgc_chipset)
KGIM_META_INIT_FN(pgc_chipset)
KGIM_META_DONE_FN(pgc_chipset)
KGIM_META_MODE_CHECK_FN(pgc_chipset)
KGIM_META_MODE_RESOURCE_FN(pgc_chipset)
KGIM_META_MODE_PREPARE_FN(pgc_chipset)
KGIM_META_MODE_ENTER_FN(pgc_chipset)
KGIM_META_MODE_LEAVE_FN(pgc_chipset)
KGIM_META_IMAGE_RESOURCE_FN(pgc_chipset)

#endif	/* #ifdef _chipset_3Dlabs_PERMEDIA_meta_h */
