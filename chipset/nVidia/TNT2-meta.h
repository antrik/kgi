/* ----------------------------------------------------------------------------
**	TNT2 meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: TNT2-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:29  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:29  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_nVidia_TNT2_meta_h
#define	_chipset_nVidia_TNT2_meta_h

#include "chipset/IBM/VGA-text-meta.h"

typedef enum
{
	TNT2_IF_VGA_DAC	= 0x00000001

} tnt2_chipset_io_flags_t;

typedef	struct
{
	vga_text_chipset_io_t	vga;

	tnt2_chipset_io_flags_t	flags;

	mem_region_t		control;
	mem_region_t		framebuffer;
	
	irq_line_t		irq;

	kgim_io_out32_fn	*CTLOut32;
	kgim_io_in32_fn		*CTLIn32;
	
} tnt2_chipset_io_t;

#define	TNT2_CTL_OUT32(ctx, val, reg)	(ctx)->CTLOut32((ctx), (val), (reg))
#define	TNT2_CTL_IN32(ctx, reg)		(ctx)->CTLIn32((ctx), (reg))
#define	TNT2_ACCEL_SAVE(ctx, buf)	(ctx)->AccelSave((ctx), (buf))
#define	TNT2_PCIDEV(ctx)			((ctx)->vga.kgim.pcidev)

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

	} tnt;

} tnt2_chipset_mode_t;


/* Driver global state */
typedef enum
{
	TNT2_CF_RESTORE_INITIAL	= 0x00000002,	/* restore init. state	*/
	TNT2_CF_TNT2		= 0x00000100,
	TNT2_CF_IRQ_CLAIMED	= 0x00001000

} tnt2_chipset_flags_t;

typedef struct
{
	kgim_chipset_t		chipset;

	vga_text_chipset_t	vga;
//	kgi_u16_t		vga_SEQ_ControlReg;
//	kgi_u16_t		vga_GRC_Mode640Reg;

	tnt2_chipset_flags_t	flags;
	tnt2_chipset_mode_t	*mode;

	/* Saved initial state */
	struct 
	{ 
		kgi_u32_t command, baseaddr0, baseaddr1;
	} pci;

	struct 
	{ 
		kgi_u8_t 
		htotal, hblankend, hretracestart, hretraceend,
		vtotal, overflow, presetrowscan, maxscanline,
		cursorstart, cursorend, startaddrhigh, startaddrlow,
		cursorlocationhigh, cursorlocationlow,
		vretracestart, vretraceend, displayend,
		offset, underlinelocation,
		vblankstart, vblankend, modecontrol, linecompare,
		repaint0, repaint1, fifocontrol, fifo, extra, 
		pixel, hextra, 
		grcursor0, grcursor1;
	} crt;
	
	struct
	{
		kgi_u32_t
		grcursorstartpos, nvpllcoeff, mpllcoeff, vpllcoeff, 
		pllcoeffselect, generalcontrol;
	} dac;
	
	struct 
	{
		kgi_u32_t
		boot0, intr0, intren0, intrread0, enable;
	} pmc;
	
	struct
	{
		kgi_u32_t
		intr0, intren0, numerator, denominator, time0, time1, alarm0;
	} timer;
	
	struct
	{
		kgi_u32_t
		delay0, dmatimeslice, piotimeslice, timeslice, nextchannel,
		debug0, intr0, intren0, ramht, ramfc, ramro, caches, mode,
		dma, size, cache0push0, cache1push0, cache0push1, cache1push1,
		cache1dmapush, cache1dmafetch, cache1dmaput, cache1dmaget, cache1dmastate,
		cache1dmainstance, cache1dmactl, cache1dmalimit, cache1dmatlbtag,
		cache1dmatlbpte, cache0pull0, cache1pull0, cache0pull1, cache1pull1,
		cache0hash, cache1hash, cache0status, cache1status, cache1status1,
		cache0put, cache0get, cache1put, cache1get, cache0engine, cache1engine,
		runoutstatus, runoutput, runoutget;
	} fifo;
	
	struct 
	{
		kgi_u32_t
		debug0, debug1, debug2, debug3, intr, nstatus, nsource,
		intren, ctxswitch1, ctxswitch2, ctxswitch3, ctxswitch4,
		ctxcontrol, ctxuser, fifo, ffintfcfifoptr, ffintfcst2,
		ffintfcst2d, status, trappedaddr, trappeddata, surface, notify,
		boffset0, boffset1, boffset2, boffset3, boffset4, boffset5,
		bbase0, bbase1, bbase2, bbase3, bbase4, bbase5,
		bpitch0, bpitch1, bpitch2, bpitch3, bpitch4,
		blimit0, blimit1, blimit2, blimit3, blimit4, blimit5, 
		bswizzle2, bswizzle5, pixel, limitviolpix, state,
		cacheindex, cacheram, dmapitch, dmacolorfmt, scaledformat,
		pattcolor0, pattcolor1, patternshape, monocolor0, rop3, chroma,
		betaand, betapremult, control0, control1, control2, blend,
		dpramindex, dpramdata, dpramadrs0, dpramadrs1, dpramdata0, dpramdata1,
		dpramwe0, dpramwe1, dpramalpha0, dpramalpha1, storedfmt, formats,
		xylogicmisc0, xylogicmisc1, xylogicmisc2, xylogicmisc3,
		xmisc, ymisc, 
		absuclipxmin, absuclipxmax, absuclipymin, absuclipymax, 
		absuclipaxmin, absuclipaxmax, absuclipaymin, absuclipaymax, 
		sourcecolor, valid1, valid2, absiclipxmax, absiclipymax, 
		clipx0, clipx1, clipy0, clipy1,
		misc240, misc241, misc242, passthru0, passthru1, passthru2,
		dmastart0, dmastart1, dmalength, dmamisc, dmadata0, dmadata1,
		dmarm, 
		dmaaxlateinst, dmaacontrol, dmaalimit, dmaatlbpte,
		dmaaadjoffset, dmaaoffset, dmaasize, dmaaysize, 
		dmabxlateinst, dmabcontrol, dmablimit, dmabtlbpte,
		dmabadjoffset, dmaboffset, dmabsize, dmabysize;
	} graph;
	
	struct
	{
		kgi_u32_t
		boot0, config0, config1, debug0, rtl;
	} fb;
	
	struct
	{
		kgi_u32_t
		boot0;
	} EXTDEV;
	
} tnt2_chipset_t;

KGIM_META_IRQ_HANDLER_FN(tnt2_chipset)
KGIM_META_INIT_FN(tnt2_chipset)
KGIM_META_DONE_FN(tnt2_chipset)
KGIM_META_MODE_CHECK_FN(tnt2_chipset)
KGIM_META_MODE_RESOURCE_FN(tnt2_chipset)
KGIM_META_MODE_PREPARE_FN(tnt2_chipset)
KGIM_META_MODE_ENTER_FN(tnt2_chipset)
KGIM_META_MODE_LEAVE_FN(tnt2_chipset)

#endif	/* #ifdef _chipset_nVidia_TNT2_meta_h */
