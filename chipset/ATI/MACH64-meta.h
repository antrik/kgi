/* ----------------------------------------------------------------------------
**	ATI MACH64 chipset definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64-meta.h,v $
**	Revision 1.4  2002/06/12 14:21:09  fspacek
**	PCI ids for Rage Pro, beginings of a cursor and lots of register definitions
**	
**	Revision 1.3  2002/06/07 01:24:35  fspacek
**	Export VGA-text resources, fixup of some register definition, implement
**	DAC io, and beginings of accelerator context handling
**	
**	Revision 1.2  2002/06/01 23:02:07  fspacek
**	Some reformating and PCI ids for Rage XL/XC
**	
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:35  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:16:34  phil
**	Automatically generated ATI chipset code
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_ATI_MACH64_meta_h
#define	_chipset_ATI_MACH64_meta_h

#include "chipset/IBM/VGA-text-meta.h"

typedef enum
{
	/* use VGA or extended DAC */
	MACH64_IF_VGA_DAC = 0x00000001,
	/* 8MB (as opposed to 16MB) memory apperture */
	MACH64_IF_8M_APP  = 0x00000002
	
} mach64_chipset_io_flags_t; 


typedef void mach64_io_out32_fn(void *ctx, kgi_u32_t val, kgi_s_t reg);
typedef void mach64_io_out8_fn(void *ctx, kgi_u8_t val, kgi_s_t reg);
typedef kgi_u32_t mach64_io_in32_fn(void *ctx, kgi_s_t reg);
typedef kgi_u8_t mach64_io_in8_fn(void *ctx, kgi_s_t reg);


typedef	struct
{
	vga_text_chipset_io_t vga;

	mach64_chipset_io_flags_t flags;
	
	mem_region_t mem;
	irq_line_t   irq;
	
	mach64_io_out32_fn *CtrlOut32;
	mach64_io_in32_fn  *CtrlIn32;
	mach64_io_out8_fn  *CtrlOut8;
	mach64_io_in8_fn   *CtrlIn8;
	kgim_io_out8_fn  *ClockOut8;
	kgim_io_in8_fn   *ClockIn8;

} mach64_chipset_io_t;

#define MACH64_CLK_OUT8(ctx, val, reg) (ctx)->vga.kgim.ClkOut8((ctx),(val),(reg))
#define MACH64_CLK_IN8(ctx, val, reg) (ctx)->vga.kgim.ClkIn8((ctx), (val), (reg))
#define	MACH64_CLOCK_OUT8(ctx, val, reg) (ctx)->ClockOut8((ctx), (val), (reg))
#define	MACH64_CLOCK_IN8(ctx, reg)       (ctx)->ClockIn8((ctx), (reg))
#define	MACH64_CTRL_OUT32(ctx, val, reg) (ctx)->CtrlOut32((ctx), (val), (reg))
#define	MACH64_CTRL_IN32(ctx, reg)       (ctx)->CtrlIn32((ctx), (reg))
#define	MACH64_CTRL_OUT8(ctx, val, reg)  (ctx)->CtrlOut8((ctx), (val), (reg))
#define	MACH64_CTRL_IN8(ctx, reg)        (ctx)->CtrlIn8((ctx), (reg))
#define	MACH64_PCIDEV(ctx)               ((ctx)->vga.kgim.pcidev)


typedef union
{
	kgim_chipset_mode_t     kgim;
	vga_text_chipset_mode_t vga;

	struct {

		kgim_chipset_mode_t	kgim;
		
		/**
		***	All chipset state info needed to setup a mode.
		**/
		kgi_u32_t h_total_disp, h_sync_strt_wid;
		kgi_u32_t v_total_disp, v_sync_strt_wid;
		kgi_u32_t off_pitch, gen_cntl, dp_set_gui_engine;
		
		/**
		***	resources exported in any mode (not all need to be
		***	available in every mode)
		**/
		kgi_mmio_region_t	video_memory_aperture;
		kgi_accel_t		accelerator;
		kgi_marker_t		pointer_ctrl;
    
	} mach64;
	
} mach64_chipset_mode_t;


/*	driver global state
*/
typedef enum
{
	MACH64_CF_RESTORE_INITIAL = 0x00000001,	/* restore init. state	*/
	MACH64_CF_IRQ_CLAIMED	  = 0x00000002	/* IRQ line claimed	*/

} mach64_chipset_flags_t;

/*
 * Chipset type. Ripped out of XFree86 source of ATI driver by
 * Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
 */
typedef enum
{
	MACH64_CHIP_88800GXC,
	MACH64_CHIP_88800GXD,
	MACH64_CHIP_88800GXE,
	MACH64_CHIP_88800GXF,
	MACH64_CHIP_88800GX,
	MACH64_CHIP_88800CX,
	MACH64_CHIP_264CT,
	MACH64_CHIP_264ET,
	MACH64_CHIP_264VT,
	MACH64_CHIP_264GT,
	MACH64_CHIP_264VTB,
	MACH64_CHIP_264GTB,
	MACH64_CHIP_264VT3,
	MACH64_CHIP_264GTDVD,
	MACH64_CHIP_264LT,
	MACH64_CHIP_264VT4,
	MACH64_CHIP_264GT2C,
	MACH64_CHIP_264GTPRO,
	MACH64_CHIP_264LTPRO,
	MACH64_CHIP_264XL,
	MACH64_CHIP_MOBILITY
	
} mach64_chipset_chip_t;

typedef struct
{
	kgim_chipset_t chipset;
	
	vga_text_chipset_t vga;
	
	/*
	** inital state of VGA-extensions
	*/
	kgi_u16_t vga_SEQ_ControlReg;
	kgi_u16_t vga_GRC_Mode640Reg;
	
	mach64_chipset_flags_t flags; /* driver global flags	 */
	mach64_chipset_mode_t *mode;  /* current mode (if set) */
	mach64_chipset_chip_t chip;
	
#if (DEBUG_LEVEL >= 0)
	struct {
		kgi_u_t vblank, vline, dma, gp;
	}interrupt;
#endif
	
	/*
	** non-VGA initial state
	*/
	struct {
		kgi_u32_t ScreenBase, ScreenStride, 
			HTotal, HgEnd, HbEnd, HsStart, HsEnd,
			VTotal, VbEnd, VsStart, VsEnd, VideoControl,
			InterruptLine;
		
	} Ctrl;
	
	struct {
		kgi_u32_t Command, LatTimer, IntLine, BaseAddr0, BaseAddr1;
	} pci;
	
} mach64_chipset_t;

KGIM_META_IRQ_HANDLER_FN(mach64_chipset)
KGIM_META_INIT_FN(mach64_chipset)
KGIM_META_DONE_FN(mach64_chipset)
KGIM_META_MODE_CHECK_FN(mach64_chipset)
KGIM_META_MODE_RESOURCE_FN(mach64_chipset)
KGIM_META_MODE_PREPARE_FN(mach64_chipset)
KGIM_META_MODE_ENTER_FN(mach64_chipset)
KGIM_META_MODE_LEAVE_FN(mach64_chipset)
KGIM_META_IMAGE_RESOURCE_FN(mach64_chipset)

#endif	/* #ifdef _chipset_ATI_mach64_meta_h */
