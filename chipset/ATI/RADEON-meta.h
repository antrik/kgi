/* ----------------------------------------------------------------------------
**	ATI RADEON chipset definitions
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: RADEON-meta.h,v $
**	Revision 1.6  2003/08/27 21:46:44  redmondp
**	- try to read clock programming constants from the ROM
**	
**	Revision 1.5  2003/03/13 03:03:58  fspacek
**	- merge clock into chipset, remove atomic update, fix sync polarity,
**	  clear high bits on CRTC registers upon VGA mode enter
**	
**	Revision 1.4  2003/03/08 14:07:36  skids
**	
**	Progress to date on GC-related register save/restore.
**	
**	Revision 1.3  2002/10/25 03:10:29  fspacek
**	PIO accelerator.
**	
**	Revision 1.2  2002/09/26 02:32:04  redmondp
**	Added palette support to ramdac
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#ifndef	_chipset_ATI_RADEON_meta_h
#define	_chipset_ATI_RADEON_meta_h

#include "chipset/IBM/VGA-text-meta.h"

typedef enum
{
	RADEON_IF_VGA_DAC	= 0x00000001,	/* use VGA or extended DAC */
		
} radeon_chipset_io_flags_t;

typedef	struct
{
	vga_text_chipset_io_t	vga;

	radeon_chipset_io_flags_t	flags;

	mem_region_t		control;	/* control MMIO aperture*/
	mem_region_t		fb;		/* frame buffer aperture*/
	irq_line_t		irq;		/* IRQ line		*/

	kgim_io_out32_fn	*CtrlOut32;
	kgim_io_in32_fn		*CtrlIn32;
	kgim_io_out8_fn		*CtrlOut8;
	kgim_io_in8_fn		*CtrlIn8;
	kgim_io_out32_fn	*ClockOut32;
	kgim_io_in32_fn		*ClockIn32;
  
} radeon_chipset_io_t;



#define	RADEON_CTRL_OUT32(ctx,val,reg)  ((ctx)->CtrlOut32((ctx), (val), (reg)))
#define RADEON_CTRL_OUT8(ctx,val,reg)	((ctx)->CtrlOut8((ctx), (val), (reg)))
#define	RADEON_CTRL_IN32(ctx,reg)       ((ctx)->CtrlIn32((ctx), (reg)))
#define RADEON_CLOCK_OUT32(ctx,val,reg) ((ctx)->ClockOut32((ctx),(val),(reg)))
#define	RADEON_CLOCK_IN32(ctx,reg)      ((ctx)->ClockIn32((ctx), (reg)))
#define	RADEON_PCIDEV(ctx)              ((ctx)->vga.kgim.pcidev)

#define RADEON_ALL_NUM_SAVEREGI_GC 1
#define RADEON_R200_NUM_SAVEREGI_GC 213
#define RADEON_LARGEST_NUM_SAVEREGI_GC	 RADEON_R200_NUM_SAVEREGI_GC
#define RADEON_ALL_NUM_SAVEREGI_MGC 1
#define RADEON_R200_NUM_SAVEREGI_MGC 130
#define RADEON_LARGEST_NUM_SAVEREGI_MGC	 RADEON_R200_NUM_SAVEREGI_MGC
#define RADEON_NUM_OTHER_REGI_GC 8
#define RADEON_NUM_SPECIAL_REGI 130
#define RADEON_NUM_SCALARS 61
#define RADEON_NUM_VECTORS 124

typedef struct
{

	/* Data stored in texture source CLUT RAM area. */
	kgi_u32_t sclut[256];

	/* TCL vectors */
	kgi_u32_t vectors[RADEON_NUM_VECTORS * 4];

	/* TCL scalars */
	kgi_u32_t scalars[RADEON_NUM_SCALARS];

	/* Model-independent registers in normal GCs */
	int gc_nregi;
	kgi_u16_t *gc_regi;
	kgi_u32_t gc_data[RADEON_ALL_NUM_SAVEREGI_GC];

	/* Model-dependent registers in normal GCs */
	int gc_chip_nregi;  
	kgi_u16_t *gc_chip_regi;
	kgi_u32_t gc_chip_data[RADEON_LARGEST_NUM_SAVEREGI_GC];

	/* Model-independent registers in MGC */
	int mgc_nregi;
	kgi_u16_t *mgc_regi;
	kgi_u32_t *mgc_data;

	/* Model-dependent registers in MGC */
	int mgc_chip_nregi;  
	kgi_u16_t *mgc_chip_regi;
	kgi_u32_t *mgc_chip_data;

} radeon_chipset_context_t;

typedef struct
{
	kgi_accel_context_t kgi;

	radeon_chipset_context_t state;

} radeon_chipset_accel_context_t;

typedef union
{
	kgim_chipset_mode_t	kgim;
	vga_text_chipset_mode_t	vga;

	struct {
		vga_text_chipset_mode_t	vga_pad;

		kgi_u32_t		DacCntl, CrtcGenCntl,
					CrtcHTotalDisp, CrtcHTotal,
					CrtcHSyncStrtWid,
					CrtcVTotalDisp,CrtcVSyncStrtWid,
					CrtcPitch, DefaultPitchOffset;

		kgi_u32_t		CSQPrimarySize;

		kgi_u_t			dclk;
		kgi_u32_t		feedback_divider, post_divider;

		kgi_mmio_region_t	video_memory_aperture;
		kgi_accel_t		accelerator;

	  	kgi_u32_t mgc_data[RADEON_ALL_NUM_SAVEREGI_MGC];
		kgi_u32_t mgc_chip_data[RADEON_LARGEST_NUM_SAVEREGI_MGC];
		radeon_chipset_context_t mgc;

	} radeon;

} radeon_chipset_mode_t;


/*	driver global state
*/
typedef enum
{
	RADEON_CF_RESTORE_INITIAL = 0x00000001,	/* restore init. state	*/
	RADEON_CF_IRQ_CLAIMED	  = 0x00000002	/* IRQ line claimed	*/

} radeon_chipset_flags_t;

typedef enum
{
	RADEON_CHIP_R100 = 0x0,
	RADEON_CHIP_RV100,
	RADEON_CHIP_M6,
	RADEON_CHIP_RV200,
	RADEON_CHIP_M7
	
} radeon_chipset_chip_t;

typedef struct
{
	kgim_chipset_t		chipset;

	vga_text_chipset_t	vga;

	radeon_chipset_flags_t	flags;	/* driver global flags	*/
	radeon_chipset_mode_t	*mode;	/* current mode (if set)	*/
	radeon_chipset_chip_t	chip;   /* type of chipset */

	kgi_u_t			fref;
	kgi_urange_t		fvco;
	kgi_u8_t		ref_div;

	/**
	***	non-VGA initial state
	**/
	struct { kgi_u32_t DacCntl, CrtcGenCntl, CrtcExtCntl,
			   CrtcHTotalDisp, CrtcHTotal,
			   CrtcHSyncStrtWid,
			   CrtcVTotalDisp,CrtcVSyncStrtWid,
			   CrtcOffset, CrtcOffsetCntl, CrtcPitch,
			   InterruptLine;

	} Ctrl;

	struct { kgi_u32_t

		Command, LatTimer, IntLine, 
		BaseAddr0, BaseAddr1, BaseAddr2;
	} pci;

	radeon_chipset_context_t init_gc;
	kgi_u32_t init_data_mgc[RADEON_ALL_NUM_SAVEREGI_MGC];
	kgi_u32_t init_data_chip_mgc[RADEON_LARGEST_NUM_SAVEREGI_MGC];

} radeon_chipset_t;

KGIM_META_IRQ_HANDLER_FN(radeon_chipset)
KGIM_META_INIT_FN(radeon_chipset)
KGIM_META_DONE_FN(radeon_chipset)
KGIM_META_MODE_CHECK_FN(radeon_chipset)
KGIM_META_MODE_RESOURCE_FN(radeon_chipset)
KGIM_META_MODE_PREPARE_FN(radeon_chipset)
KGIM_META_MODE_ENTER_FN(radeon_chipset)
KGIM_META_MODE_LEAVE_FN(radeon_chipset)
KGIM_META_IMAGE_RESOURCE_FN(radeon_chipset)

#endif	/* #ifdef _chipset_ATI_radeon_meta_h */
