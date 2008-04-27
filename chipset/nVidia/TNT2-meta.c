/* ----------------------------------------------------------------------------
**	nVidia Riva TNT chipset meta-language implementation
** ----------------------------------------------------------------------------
**	Copyright	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	$Log: TNT2-meta.c,v 4
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1.1.1 $"

#define	DEBUG_LEVEL	2

#include <kgi/module.h>

#define	__nVidia_TNT2
#include "chipset/nVidia/TNT2.h"
#include "chipset/nVidia/TNT2-meta.h"
#include "chipset/nVidia/TNT2-bind.h"

#define	TNT2_ControlBase		((kgi_u8 *) tnt2_mem_control.base_virt)
#define	MEM_VGA			(TNT2_ControlBase + TNT2_VGABase)

#if 0
static int nv4Busy(RIVA_HW_INST *chip)
{
	DEBUG2("nv3busy()");
	return ((!(chip->PFIFO[0x00001214 / 4] & 0x10)) | (chip->PGRAPH[0x00000700 / 4] & 0x01));
}

static int ShowHideCursor(RIVA_HW_INST *chip, int ShowHide)
{
	int current;
	
	DEBUG2("ShowHideCursor()");

	current = chip->CurrentState->cursor1;
	chip->CurrentState->cursor1 = (chip->CurrentState->cursor1 & 0xFE) | (ShowHide & 0x01);
	
	io_out8(0x31, 0x3D4); io_out8(chip->CurrentState->cursor1, 0x3D5);
	
	return (current & 0x01);
}
#endif

#ifdef DEBUG_LEVEL
/* Print verbose chipset configuration for debugging purposes */
static inline void tnt2_chipset_examine(tnt2_chipset_t *tnt2, kgi_u32_t flags)
{
	KRN_DEBUG(1, "tnt2_chipset_examine()");
	
	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "PCI Registers:");
	KRN_DEBUG(1, "--------------");
	KRN_DEBUG(1, "PCIBASE0 = %.8x", tnt2->pci.baseaddr0);
	KRN_DEBUG(1, "PCIBASE1 = %.8x", tnt2->pci.baseaddr1);

	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "CRT Registers:");
	KRN_DEBUG(1, "--------------");
	KRN_DEBUG(1, "HTOTAL = %.2x", tnt2->crt.htotal);
	KRN_DEBUG(1, "HBLANKEND = %.2x", tnt2->crt.hblankend);
	KRN_DEBUG(1, "HRETRACESTART = %.2x", tnt2->crt.hretracestart);
	KRN_DEBUG(1, "HRETRACEEND = %.2x", tnt2->crt.hretraceend);
	KRN_DEBUG(1, "VTOTAL = %.2x", tnt2->crt.vtotal);
	KRN_DEBUG(1, "OVERFLOW = %.2x", tnt2->crt.overflow);
	KRN_DEBUG(1, "PRESETROWSCAN = %.2x", tnt2->crt.presetrowscan);
	KRN_DEBUG(1, "MAXSCANLINE = %.2x", tnt2->crt.maxscanline);
	KRN_DEBUG(1, "CURSORSTART = %.2x", tnt2->crt.cursorstart);
	KRN_DEBUG(1, "CURSOREND = %.2x", tnt2->crt.cursorend);
	KRN_DEBUG(1, "STARTADDRHIGH = %.2x", tnt2->crt.startaddrhigh);
	KRN_DEBUG(1, "STARTADDRLOW = %.2x", tnt2->crt.startaddrlow);
	KRN_DEBUG(1, "CURSORLOCATIONHIGH = %.2x", tnt2->crt.cursorlocationhigh);
	KRN_DEBUG(1, "CURSORLOCATIONLOW = %.2x", tnt2->crt.cursorlocationlow);
	KRN_DEBUG(1, "VRETRACESTART = %.2x", tnt2->crt.vretracestart);
	KRN_DEBUG(1, "VRETRACEEND = %.2x", tnt2->crt.vretraceend);
	KRN_DEBUG(1, "DISPLAYEND = %.2x", tnt2->crt.displayend);
	KRN_DEBUG(1, "OFFSET = %.2x", tnt2->crt.offset);
	KRN_DEBUG(1, "UNDERLINELOCATION = %.2x", tnt2->crt.underlinelocation);
	KRN_DEBUG(1, "VBLANKSTART = %.2x", tnt2->crt.vblankstart);
	KRN_DEBUG(1, "VBLANKEND = %.2x", tnt2->crt.vblankend);
	KRN_DEBUG(1, "MODECONTROL = %.2x", tnt2->crt.modecontrol);
	KRN_DEBUG(1, "LINECOMPARE = %.2x", tnt2->crt.linecompare);
	KRN_DEBUG(1, "REPAINT0 = %.2x", tnt2->crt.repaint0);
	KRN_DEBUG(1, "REPAINT1 = %.2x", tnt2->crt.repaint1);
	KRN_DEBUG(1, "FIFOCONTROL = %.2x", tnt2->crt.fifocontrol);
	KRN_DEBUG(1, "FIFO = %.2x", tnt2->crt.fifo);
	KRN_DEBUG(1, "EXTRA = %.2x", tnt2->crt.extra);
	KRN_DEBUG(1, "PIXEL = %.2x", tnt2->crt.pixel);
	KRN_DEBUG(1, "HEXTRA = %.2x", tnt2->crt.hextra);
	KRN_DEBUG(1, "GRCURSOR0 = %.2x", tnt2->crt.grcursor0);
	KRN_DEBUG(1, "GRCURSOR1 = %.2x", tnt2->crt.grcursor1);

	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "PRAMDAC Registers:");
	KRN_DEBUG(1, "--------------");
	KRN_DEBUG(1, "GRCURSOR_START_POS = %.8x", tnt2->dac.grcursorstartpos);
	KRN_DEBUG(1, "NVPLL_COEFF = %.8x", tnt2->dac.nvpllcoeff);
	KRN_DEBUG(1, "MPLL_COEFF = %.8x", tnt2->dac.mpllcoeff);
	KRN_DEBUG(1, "VPLL_COEFF = %.8x", tnt2->dac.vpllcoeff);
	KRN_DEBUG(1, "PLL_COEFF_SELECT = %.8x", tnt2->dac.pllcoeffselect);
	KRN_DEBUG(1, "GENERAL_CONTROL = %.8x", tnt2->dac.generalcontrol);
}
#endif	/* #if (DEBUG_LEVEL > 1) */

#if (DEBUG_LEVEL > 0)
/* Symbolic subsystem names for post-reset initalization debug messages */
static const kgi_ascii_t *tnt2_chipset_subsystem[16] =
{
	"CS",		"MC",		"GPFIFO",	"VC",
	"DAC",		"VS",		"VGA",		"R7",
	"GP",		"R9",		"RA",		"RB",
	"RC",		"RD",		"RE",		"RF"
};
#endif	/* #if (DEBUG_LEVEL > 0)	*/

/* Calculate partial product codes from width */
static kgi_u_t tnt2_chipset_calc_partials(kgi_u_t width, kgi_u_t *pp)
{
	kgi_u_t current_width = 0;
	register kgi_u_t p0, p1, p2;

	KRN_DEBUG(2, "tnt2_chipset_calc_partials()");
	
	for (p0 = 0; p0 <= 7; p0++) 
	{
		for (p1 = 0; p1 <= p0; p1++) 
		{
			for (p2 = 0; p2 <= p1; p2++) 
			{
				current_width = ((16 << p0) & ~0x1F) + ((16 << p1) & ~0x1F) + ((16 << p2) & ~0x1F);

				if (current_width >= width) 
				{
					KRN_DEBUG(2, "current_width = %i, pp[] = { %i, %i, %i }", current_width, p0, p1, p2); 
					pp[0] = p0;
					pp[1] = p1;
					pp[2] = p2;
					
					return current_width;
				}
			}
		}
	}
	
	KRN_DEBUG(0, "invalid width, current_width = 0, pp[] = { 0, 0, 0 }");
	pp[0] = pp[1] = pp[2] = 0;
	
	return 0;
}

static inline void tnt2_chipset_sync(tnt2_chipset_io_t *tnt2_io)
{
	KRN_DEBUG(2, "tnt2_chipset_sync()");
	
	return;

#if 0
	do 
	{
		while (count--);
		count = tnt2_CS_IN32(tnt2_io, tnt2_CS_DMACount);

	} while (count);

	count = 1;
	
	do 
	{
		while (count++ < 255);
		count = tnt2_CS_IN32(tnt2_io, tnt2_CS_InFIFOSpace);

	} while (count);

	while (tnt2_CS_IN32(tnt2_io, 0x068) & tnt2_CS068_GraphicsProcessorActive);
#endif
}

/* Accelerator implementation */

typedef struct
{
	kgi_u32_t

#define	TNT2_CORE_CONTEXT_GROUP00	0x007F8000
	Group00,
		StartXDom, dXDom, StartXSub, dXSub, StartY, dY, Count,

#define	TNT2_CORE_CONTEXT_GROUP01	0x00238010
	Group01,
		RasterizerMode, YLimits, XLimits,

#define	TNT2_CORE_CONTEXT_GROUP02	0x04008020
	Group02,
		PackedDataLimits,

#define	TNT2_CORE_CONTEXT_GROUP03	0x021F8030
	Group03,
		ScissorMode, ScissorMinXY, ScissorMaxXY, ScreenSize,
		AreaStippleMode, WindowOrigin,

#define	TNT2_CORE_CONTEXT_GROUP04	0x00FF8040
	Group04,
		AreaStipplePattern0, AreaStipplePattern1,
		AreaStipplePattern2, AreaStipplePattern3,
		AreaStipplePattern4, AreaStipplePattern5,
		AreaStipplePattern6, AreaStipplePattern7,

#define	TNT2_CORE_CONTEXT_GROUP07	0x03FF8070
	Group07,
		TextureAddressMode, SStart, dSdx, dSdyDom, TStart, dTdx,
		dTdyDom, QStart, dQdx, dQdyDom,

#define	TNT2_CORE_CONTEXT_GROUP0B	0x000780B0
	Group0B,
		TextureBaseAddress, TextureMapFormat, TextureDataFormat,

#define	TNT2_CORE_CONTEXT_GROUP0C	0xC00180C0
	Group0C,
		Texel0, TextureReadMode, TexelLUTMode,

#define	TNT2_CORE_CONTEXT_GROUP0D	0x7E7D80D0
	Group0D,
		TextureColorMode, FogMode, FogColor, FStart, dFdx, dFdyDom,
		KsStart, dKsdx, dKsdyDom, KdStart, Kddx, dKddyDom,

#define	TNT2_CORE_CONTEXT_GROUP0F	0x33FF80F0
	Group0F,
		RStart, dRdx, dRdyDom, GStart, dGdx, dGdyDom, BStart, dBdx,
		dBdyDom, AStart, ColorDDAMode, ConstantColor,

#define	TNT2_CORE_CONTEXT_GROUP10	0x007C8100
	Group10,
		AlphaBlendMode, DitherMode, FBSoftwareWriteMask, LogicalOpMode,
		FBWriteData,

#define	TNT2_CORE_CONTEXT_GROUP11	0x43878110
	Group11,
		LBReadMode, LBReadFormat, LBSourceOffset, LBWindowBase,
		LBWriteMode, LBWriteFormat,

#define	TNT2_CORE_CONTEXT_GROUP13	0x0FFF8130
	Group13,
		Window, StencilMode, StencilData, Stencil, DepthMode, Depth,
		ZStartU, ZStartL, dZdxU, dZdxL, dZdyDomU, dZdyDomL,

#define	TNT2_CORE_CONTEXT_GROUP15	0x07C78150
	Group15,
		FBReadMode, FBSourceOffset, FBPixelOffset, FBWindowBase,
		FBWriteMode, FBHardwareWriteMask, FBBlockColor, FBReadPixel,

#define	TNT2_CORE_CONTEXT_GROUP18	0x000F8180
	Group18,
		FilterMode, StatisticMode, MinRegion, MaxRegion,

#define	TNT2_CORE_CONTEXT_GROUP1D	0xFFFF81D0
	Group1D,
		TexelLUT0, TexelLUT1, TexelLUT2, TexelLUT3,
		TexelLUT4, TexelLUT5, TexelLUT6, TexelLUT7,
		TexelLUT8, TexelLUT9, TexelLUTA, TexelLUTB,
		TexelLUTC, TexelLUTD, TexelLUTE, TexelLUTF,

#define	TNT2_CORE_CONTEXT_GROUP1E	0x000781E0
	Group1E,
		YUVMode, ChromaUpperBound, ChromaLowerBound;

} tnt2_chipset_core_context_t;

typedef struct
{
	kgi_u32_t

#define	TNT2_DELTA_CONTEXT_GROUP23	0x1FFF8230
	Group23,
		V0Float_s, V0Float_t, V0Float_q, V0Float_Ks, V0Float_Kd,
		V0Float_red, V0Float_green, V0Float_blue, V0Float_alpha,
		V0Float_fog, V0Float_x, V0Float_y, V0Float_z,

#define	TNT2_DELTA_CONTEXT_GROUP24	0x1FFF8240
	Group24,
		V1Float_s, V1Float_t, V1Float_q, V1Float_Ks, V1Float_Kd,
		V1Float_red, V1Float_green, V1Float_blue, V1Float_alpha,
		V1Float_fog, V1Float_x, V1Float_y, V1Float_z,

#define	TNT2_DELTA_CONTEXT_GROUP25	0x1FFF8250
	Group25,
		V2Float_s, V2Float_t, V2Float_q, V2Float_Ks, V2Float_Kd,
		V2Float_red, V2Float_green, V2Float_blue, V2Float_alpha,
		V2Float_fog, V2Float_x, V2Float_y, V2Float_z,

#define	TNT2_DELTA_CONTEXT_GROUP26	0x80018260
	Group26,
		DeltaMode, BroadcastMask;

} tnt2_chipset_delta_context_t;

typedef struct
{
	kgi_u32_t

#define	TNT2_PERMEDIA2_CONTEXT_GROUP18	0x60008180
	Group18,
		FBBlockColorU, FBBlockColorL,

#define	TNT2_PERMEDIA2_CONTEXT_GROUP1B	0x000781B0
	Group1B,
		FBSourceBase, FBSourceDelta, Config,

#define	TNT2_PERMEDIA2_CONTEXT_GROUP1E	0x001881E0
	Group1E,
		AlphaMapUpperBound, AlphaMapLowerBound,

#define	TNT2_PERMEDIA2_CONTEXT_GROUP23	0x5FFF8230
	Group23,
		V0Float_s, V0Float_t, V0Float_q, V0Float_Ks, V0Float_Kd,
		V0Float_red, V0Float_green, V0Float_blue, V0Float_alpha,
		V0Float_fog, V0Float_x, V0Float_y, V0Float_z, V0Float_color,

#define	TNT2_PERMEDIA2_CONTEXT_GROUP24	0x5FFF8240
	Group24,
		V1Float_s, V1Float_t, V1Float_q, V1Float_Ks, V1Float_Kd,
		V1Float_red, V1Float_green, V1Float_blue, V1Float_alpha,
		V1Float_fog, V1Float_x, V1Float_y, V1Float_z, V1Float_color,

#define	TNT2_PERMEDIA2_CONTEXT_GROUP25	0x5FFF8250
	Group25,
		V2Float_s, V2Float_t, V2Float_q, V2Float_Ks, V2Float_Kd,
		V2Float_red, V2Float_green, V2Float_blue, V2Float_alpha,
		V2Float_fog, V2Float_x, V2Float_y, V2Float_z, V2Float_color,

#define	TNT2_PERMEDIA2_CONTEXT_GROUP26	0x00018260
	Group26,
		DeltaMode;

#warning handle texture LUT!

} tnt2_chipset_permedia2_context_t;


typedef struct
{
	kgi_accel_context_t kgi;
	kgi_aperture_t aperture;

	struct 
	{
		tnt2_chipset_core_context_t core;

		union 
		{
			tnt2_chipset_delta_context_t delta;
			tnt2_chipset_permedia2_context_t	p2;
		} ext;

	} state;

} tnt2_chipset_accel_context_t;

static void tnt2_chipset_accel_init(kgi_accel_t *accel, void *ctx)
{
	KRN_DEBUG(2, "tnt2_chipset_accel_init()");
	
	return;
	
#if 0
	tnt2_chipset_t *tnt = accel->meta;
	tnt2_chipset_io_t *tnt2_io = accel->meta_io;
	tnt2_chipset_accel_context_t *tnt2_ctx = ctx;
	kgi_size_t offset;

	/*	To be able to load ctx->state via DMA we precalculate the
	**	aperture info needed to have it at hand when needed.
	*/
	tnt2_ctx->aperture.size = sizeof(tnt2_ctx->state.core);
	if (tnt2->flags & tnt2_CF_DELTA) {

		KRN_ASSERT(! (tnt2->flags & tnt2_CF_PERMEDIA2));
		tnt2_ctx->aperture.size += sizeof(tnt2_ctx->state.ext.delta);
	}
	if (tnt2->flags & tnt2_CF_PERMEDIA2) {

		KRN_ASSERT(! (tnt2->flags & tnt2_CF_DELTA));
		tnt2_ctx->aperture.size += sizeof(tnt2_ctx->state.ext.p2);
	}
	offset = (mem_vaddr_t) &tnt2_ctx->state - (mem_vaddr_t) tnt2_ctx;
	tnt2_ctx->aperture.bus  = tnt2_ctx->kgi.aperture.bus  + offset;
	tnt2_ctx->aperture.phys = tnt2_ctx->kgi.aperture.phys + offset;
	tnt2_ctx->aperture.virt = tnt2_ctx->kgi.aperture.virt + offset;

	tnt2_ctx->state.core.Group00 = tnt2_CORE_CONTEXT_GROUP00;
	tnt2_ctx->state.core.Group01 = tnt2_CORE_CONTEXT_GROUP01;
	tnt2_ctx->state.core.Group02 = tnt2_CORE_CONTEXT_GROUP02;
	tnt2_ctx->state.core.Group03 = tnt2_CORE_CONTEXT_GROUP03;
	tnt2_ctx->state.core.Group04 = tnt2_CORE_CONTEXT_GROUP04;
	tnt2_ctx->state.core.Group07 = tnt2_CORE_CONTEXT_GROUP07;
	tnt2_ctx->state.core.Group0B = tnt2_CORE_CONTEXT_GROUP0B;
	tnt2_ctx->state.core.Group0C = tnt2_CORE_CONTEXT_GROUP0C;
	tnt2_ctx->state.core.Group0D = tnt2_CORE_CONTEXT_GROUP0D;
	tnt2_ctx->state.core.Group0F = tnt2_CORE_CONTEXT_GROUP0F;
	tnt2_ctx->state.core.Group10 = tnt2_CORE_CONTEXT_GROUP10;
	tnt2_ctx->state.core.Group11 = tnt2_CORE_CONTEXT_GROUP11,
	tnt2_ctx->state.core.Group13 = tnt2_CORE_CONTEXT_GROUP13;
	tnt2_ctx->state.core.Group15 = tnt2_CORE_CONTEXT_GROUP15;
	tnt2_ctx->state.core.Group18 = tnt2_CORE_CONTEXT_GROUP18;
	tnt2_ctx->state.core.Group1D = tnt2_CORE_CONTEXT_GROUP1D;
	tnt2_ctx->state.core.Group1E = tnt2_CORE_CONTEXT_GROUP1E;

	if (tnt2->flags & tnt2_CF_DELTA) {

		tnt2_ctx->state.ext.delta.Group23 = tnt2_DELTA_CONTEXT_GROUP23;
		tnt2_ctx->state.ext.delta.Group24 = tnt2_DELTA_CONTEXT_GROUP24;
		tnt2_ctx->state.ext.delta.Group25 = tnt2_DELTA_CONTEXT_GROUP25;
		tnt2_ctx->state.ext.delta.Group26 = tnt2_DELTA_CONTEXT_GROUP26;
	}

	if (tnt2->flags & tnt2_CF_PERMEDIA2) {

		tnt2_ctx->state.ext.p2.Group18 = tnt2_PERMEDIA2_CONTEXT_GROUP18;
		tnt2_ctx->state.ext.p2.Group1B = tnt2_PERMEDIA2_CONTEXT_GROUP1B;
		tnt2_ctx->state.ext.p2.Group1E = tnt2_PERMEDIA2_CONTEXT_GROUP1E;
		tnt2_ctx->state.ext.p2.Group23 = tnt2_PERMEDIA2_CONTEXT_GROUP23;
		tnt2_ctx->state.ext.p2.Group24 = tnt2_PERMEDIA2_CONTEXT_GROUP24;
		tnt2_ctx->state.ext.p2.Group25 = tnt2_PERMEDIA2_CONTEXT_GROUP25;
		tnt2_ctx->state.ext.p2.Group26 = tnt2_PERMEDIA2_CONTEXT_GROUP26;
	}
#endif
}

static void tnt2_chipset_accel_done(kgi_accel_t *accel, void *ctx)
{
	KRN_DEBUG(2, "tnt2_chipset_accel_done()");
	
	if (ctx == accel->context) 
	{
		accel->context = NULL;
	}
}

static inline void tnt2_chipset_accel_save(kgi_accel_t *accel)
{
	KRN_DEBUG(2, "tnt2_chipset_accel_save()");
	
	return;
	
#if 0
	
	tnt2_chipset_t *tnt = accel->meta;
	tnt2_chipset_io_t *tnt2_io = accel->meta_io;
	tnt2_chipset_accel_context_t *tnt2_ctx = accel->ctx;
	mem_vaddr_t gpr = tnt2_io->control.base_virt + tnt2_GPRegisterBase;

	KRN_ASSERT(tnt);
	KRN_ASSERT(tnt2_io);
	KRN_ASSERT(tnt2_ctx);
	KRN_ASSERT(tnt2_io->control.base_virt);

	KRN_ASSERT(0 == tnt2_CS_IN32(tnt2_io, tnt2_CS_DMACount));

#define	TNT2_SAVE(reg)	\
	tnt2_ctx->state.core.reg = mem_in32(gpr + (TNT2_GP_##reg << 3))

	TNT2_SAVE(StartXDom);
	TNT2_SAVE(dXDom);
	TNT2_SAVE(StartXSub);
	TNT2_SAVE(dXSub);
	TNT2_SAVE(StartY);
	TNT2_SAVE(dY);
	TNT2_SAVE(Count);

	TNT2_SAVE(RasterizerMode);
	TNT2_SAVE(YLimits);
	TNT2_SAVE(XLimits);

	TNT2_SAVE(PackedDataLimits);

	TNT2_SAVE(ScissorMode);
	TNT2_SAVE(ScissorMinXY);
	TNT2_SAVE(ScissorMaxXY);
	TNT2_SAVE(ScreenSize);
	TNT2_SAVE(AreaStippleMode);
	TNT2_SAVE(WindowOrigin);

	TNT2_SAVE(AreaStipplePattern0);
	TNT2_SAVE(AreaStipplePattern1);
	TNT2_SAVE(AreaStipplePattern2);
	TNT2_SAVE(AreaStipplePattern3);
	TNT2_SAVE(AreaStipplePattern4);
	TNT2_SAVE(AreaStipplePattern5);
	TNT2_SAVE(AreaStipplePattern6);
	TNT2_SAVE(AreaStipplePattern7);

	TNT2_SAVE(TextureAddressMode);
	TNT2_SAVE(SStart);
	TNT2_SAVE(dSdx);
	TNT2_SAVE(dSdyDom);
	TNT2_SAVE(TStart);
	TNT2_SAVE(dTdx);
	TNT2_SAVE(dTdyDom);
	TNT2_SAVE(QStart);
	TNT2_SAVE(dQdx);
	TNT2_SAVE(dQdyDom);

	TNT2_SAVE(TextureBaseAddress);
	TNT2_SAVE(TextureMapFormat);
	TNT2_SAVE(TextureDataFormat);

	TNT2_SAVE(Texel0);
	TNT2_SAVE(TextureReadMode);
	TNT2_SAVE(TexelLUTMode);

	TNT2_SAVE(TextureColorMode);
	TNT2_SAVE(FogMode);
	TNT2_SAVE(FogColor);
	TNT2_SAVE(FStart);
	TNT2_SAVE(dFdx);
	TNT2_SAVE(dFdyDom);
	TNT2_SAVE(KsStart);
	TNT2_SAVE(dKsdx);
	TNT2_SAVE(dKsdyDom);
	TNT2_SAVE(KdStart);
	TNT2_SAVE(Kddx);
	TNT2_SAVE(dKddyDom);

	TNT2_SAVE(RStart);
	TNT2_SAVE(dRdx);
	TNT2_SAVE(dRdyDom);
	TNT2_SAVE(GStart);
	TNT2_SAVE(dGdx);
	TNT2_SAVE(dGdyDom);
	TNT2_SAVE(BStart);
	TNT2_SAVE(dBdx);
	TNT2_SAVE(dBdyDom);
	TNT2_SAVE(AStart);
	TNT2_SAVE(ColorDDAMode);
	TNT2_SAVE(ConstantColor);

	TNT2_SAVE(AlphaBlendMode);
	TNT2_SAVE(DitherMode);
	TNT2_SAVE(FBSoftwareWriteMask);
	TNT2_SAVE(LogicalOpMode);

	TNT2_SAVE(LBReadMode);
	TNT2_SAVE(LBReadFormat);
	TNT2_SAVE(LBSourceOffset);
	TNT2_SAVE(LBWindowBase);
	TNT2_SAVE(LBWriteMode);
	TNT2_SAVE(LBWriteFormat);
	TNT2_SAVE(FBWriteData);

	TNT2_SAVE(Window);
	TNT2_SAVE(StencilMode);
	TNT2_SAVE(StencilData);
	TNT2_SAVE(Stencil);
	TNT2_SAVE(DepthMode);
	TNT2_SAVE(Depth);
	TNT2_SAVE(ZStartU);
	TNT2_SAVE(ZStartL);
	TNT2_SAVE(dZdxU);
	TNT2_SAVE(dZdxL);
	TNT2_SAVE(dZdyDomU);
	TNT2_SAVE(dZdyDomL);

	TNT2_SAVE(FBReadMode);
	TNT2_SAVE(FBSourceOffset);
	TNT2_SAVE(FBPixelOffset);
	TNT2_SAVE(FBWindowBase);
	TNT2_SAVE(FBWriteMode);
	TNT2_SAVE(FBHardwareWriteMask);
	TNT2_SAVE(FBBlockColor);
	TNT2_SAVE(FBReadPixel);

	TNT2_SAVE(FilterMode);
	TNT2_SAVE(StatisticMode);
	TNT2_SAVE(MinRegion);
	TNT2_SAVE(MaxRegion);

	TNT2_SAVE(TexelLUT0);
	TNT2_SAVE(TexelLUT1);
	TNT2_SAVE(TexelLUT2);
	TNT2_SAVE(TexelLUT3);
	TNT2_SAVE(TexelLUT4);
	TNT2_SAVE(TexelLUT5);
	TNT2_SAVE(TexelLUT6);
	TNT2_SAVE(TexelLUT7);
	TNT2_SAVE(TexelLUT8);
	TNT2_SAVE(TexelLUT9);
	TNT2_SAVE(TexelLUTA);
	TNT2_SAVE(TexelLUTB);
	TNT2_SAVE(TexelLUTC);
	TNT2_SAVE(TexelLUTD);
	TNT2_SAVE(TexelLUTE);
	TNT2_SAVE(TexelLUTF);

	TNT2_SAVE(YUVMode);
	TNT2_SAVE(ChromaUpperBound);
	TNT2_SAVE(ChromaLowerBound);

#undef	TNT2_SAVE
	

#define	TNT2_SAVE(reg)	\
	tnt2_ctx->state.ext.delta.reg = mem_in32(gpr + (DELTA_GC_##reg << 3))

	if (tnt2->flags & tnt2_CF_DELTA) {

		tnt2_SAVE(V0Float_s);
		tnt2_SAVE(V0Float_t);
		tnt2_SAVE(V0Float_q);
		tnt2_SAVE(V0Float_Ks);
		tnt2_SAVE(V0Float_Kd);
		tnt2_SAVE(V0Float_red);
		tnt2_SAVE(V0Float_green);
		tnt2_SAVE(V0Float_blue);
		tnt2_SAVE(V0Float_alpha);
		tnt2_SAVE(V0Float_fog);
		tnt2_SAVE(V0Float_x);
		tnt2_SAVE(V0Float_y);
		tnt2_SAVE(V0Float_z);

		tnt2_SAVE(V1Float_s);
		tnt2_SAVE(V1Float_t);
		tnt2_SAVE(V1Float_q);
		tnt2_SAVE(V1Float_Ks);
		tnt2_SAVE(V1Float_Kd);
		tnt2_SAVE(V1Float_red);
		tnt2_SAVE(V1Float_green);
		tnt2_SAVE(V1Float_blue);
		tnt2_SAVE(V1Float_alpha);
		tnt2_SAVE(V1Float_fog);
		tnt2_SAVE(V1Float_x);
		tnt2_SAVE(V1Float_y);
		tnt2_SAVE(V1Float_z);

		tnt2_SAVE(V2Float_s);
		tnt2_SAVE(V2Float_t);
		tnt2_SAVE(V2Float_q);
		tnt2_SAVE(V2Float_Ks);
		tnt2_SAVE(V2Float_Kd);
		tnt2_SAVE(V2Float_red);
		tnt2_SAVE(V2Float_green);
		tnt2_SAVE(V2Float_blue);
		tnt2_SAVE(V2Float_alpha);
		tnt2_SAVE(V2Float_fog);
		tnt2_SAVE(V2Float_x);
		tnt2_SAVE(V2Float_y);
		tnt2_SAVE(V2Float_z);

		tnt2_SAVE(DeltaMode);
		tnt2_SAVE(BroadcastMask);
	}

#undef	tnt2_SAVE
#define	tnt2_SAVE(reg)	\
	tnt2_ctx->state.ext.p2.reg = mem_in32(gpr + (tnt2_GP_##reg << 3))

	if (tnt2->flags & tnt2_CF_PERMEDIA2) {

		tnt2_SAVE(FBBlockColorU);
		tnt2_SAVE(FBBlockColorL);

		tnt2_SAVE(FBSourceBase);
		tnt2_SAVE(FBSourceDelta);
		tnt2_SAVE(Config);

		tnt2_SAVE(AlphaMapUpperBound);
		tnt2_SAVE(AlphaMapLowerBound);

		tnt2_SAVE(V0Float_s);
		tnt2_SAVE(V0Float_t);
		tnt2_SAVE(V0Float_q);
		tnt2_SAVE(V0Float_Ks);
		tnt2_SAVE(V0Float_Kd);
		tnt2_SAVE(V0Float_red);
		tnt2_SAVE(V0Float_green);
		tnt2_SAVE(V0Float_blue);
		tnt2_SAVE(V0Float_alpha);
		tnt2_SAVE(V0Float_fog);
		tnt2_SAVE(V0Float_x);
		tnt2_SAVE(V0Float_y);
		tnt2_SAVE(V0Float_z);
		tnt2_SAVE(V0Float_color);

		tnt2_SAVE(V1Float_s);
		tnt2_SAVE(V1Float_t);
		tnt2_SAVE(V1Float_q);
		tnt2_SAVE(V1Float_Ks);
		tnt2_SAVE(V1Float_Kd);
		tnt2_SAVE(V1Float_red);
		tnt2_SAVE(V1Float_green);
		tnt2_SAVE(V1Float_blue);
		tnt2_SAVE(V1Float_alpha);
		tnt2_SAVE(V1Float_fog);
		tnt2_SAVE(V1Float_x);
		tnt2_SAVE(V1Float_y);
		tnt2_SAVE(V1Float_z);
		tnt2_SAVE(V1Float_color);

		tnt2_SAVE(V2Float_s);
		tnt2_SAVE(V2Float_t);
		tnt2_SAVE(V2Float_q);
		tnt2_SAVE(V2Float_Ks);
		tnt2_SAVE(V2Float_Kd);
		tnt2_SAVE(V2Float_red);
		tnt2_SAVE(V2Float_green);
		tnt2_SAVE(V2Float_blue);
		tnt2_SAVE(V2Float_alpha);
		tnt2_SAVE(V2Float_fog);
		tnt2_SAVE(V2Float_x);
		tnt2_SAVE(V2Float_y);
		tnt2_SAVE(V2Float_z);
		tnt2_SAVE(V2Float_color);

		tnt2_SAVE(DeltaMode);
	}
#undef	tnt2_SAVE
	
#endif
}

static inline void tnt2_chipset_accel_restore(kgi_accel_t *accel)
{
//	tnt2_chipset_io_t *tnt2_io = accel->meta_io;
//	tnt2_chipset_accel_context_t *tnt2_ctx = accel->ctx;

	KRN_DEBUG(2, "tnt2_chipset_accel_restore()");
	
//	KRN_ASSERT(0 == TNT2_CS_IN32(tnt2_io, TNT2_CS_DMACount));

//#warning flush cache-lines of the context buffer!

//	TNT2_CS_OUT32(tnt2_io, tnt2_ctx->aperture.bus, TNT2_CS_DMAAddress);
//	TNT2_CS_OUT32(tnt2_io, tnt2_ctx->aperture.size >> 2, TNT2_CS_DMACount);
}

static void tnt2_chipset_accel_schedule(kgi_accel_t *accel)
{
//#warning this must not be interrupted!
//	tnt2_chipset_io_t *tnt2_io = accel->meta_io;
	kgi_accel_buffer_t *buffer = accel->execution.queue;

	KRN_DEBUG(2, "tnt2_chipset_accel_schedule()");
	
	KRN_ASSERT(buffer);

	switch (buffer->execution.state) 
	{
	case KGI_AS_EXEC:
		/*	Execution of the current buffer finished, so we 
		**	mark it KGI_AS_IDLE and remove it from the queue.
		*/
		accel->execution.queue = buffer->next;
		buffer->next = NULL;
		buffer->execution.state = KGI_AS_IDLE;
//#warning wakeup buffer->executed !

		if (NULL == accel->execution.queue) 
		{
			/*	no further buffers queued, thus we are done */
#warning wakeup tnt2_accel->idle
			return;
		}

		buffer = accel->execution.queue;
		KRN_ASSERT(KGI_AS_WAIT == buffer->execution.state);
		/* Fall through */
	case KGI_AS_WAIT:
		/*	If necessary we initiate a GP context switch and
		**	re-enter here when the new context is loaded.
		**	Otherwise we just initiate the buffer transfer.
		*/
#warning is buffer->context right here?
		if (accel->context != buffer->context) 
		{
			if (accel->context) 
			{
				tnt2_chipset_accel_save(accel);
			}
			
			accel->context = buffer->context;
			tnt2_chipset_accel_restore(accel);
			
			return;
		}

//		KRN_ASSERT(0 == tnt2_CS_IN32(tnt2_io, tnt2_CS_DMACount));

		buffer->execution.state = KGI_AS_EXEC;
		
//		tnt2_CS_OUT32(tnt2_io, buffer->aperture.bus, tnt2_CS_DMAAddress);
//		tnt2_CS_OUT32(tnt2_io, buffer->exec_size >> 2, tnt2_CS_DMACount);

		return;

	default:
		KRN_ERROR("PERMEDIA: invalid state %i for queued buffer", buffer->execution.state);
		KRN_INTERNAL_ERROR;
		return;
	}
}

static void tnt2_chipset_accel_exec(kgi_accel_t *accel, 
	kgi_accel_buffer_t *buffer)
{
#warning check/validate validate data stream!!!
#warning this must not be interrupted!

	KRN_DEBUG(2, "tnt2_chipset_accel_exec()");
	
	KRN_ASSERT(KGI_AS_FILL == buffer->execution.state);

	buffer->execution.state = KGI_AS_WAIT;

	if (accel->execution.queue) 
	{
		kgi_accel_buffer_t *queued = accel->execution.queue;

#warning is this right?
		while (queued->next && (queued->next->priority >= buffer->priority)) 
		{
			queued = queued->next;
		}

		buffer->next = queued->next;
		queued->next = buffer;
		
		return;
	}

	buffer->next = NULL;
	accel->execution.queue = buffer;

	tnt2_chipset_accel_schedule(accel);
}

/* IRQ and error handlers */

static inline void tnt2_chipset_error_handler(tnt2_chipset_t *tnt2, 
	tnt2_chipset_io_t *tnt2_io, irq_system_t *system)
{
	KRN_DEBUG(2, "tnt2_chipset_error_handler()");
	
#if 0
	kgi_u32_t handled = 0;
	kgi_u32_t flags = tnt2_CS_IN32(tnt2_io, TNT2_CS_ErrorFlags);

#define	TNT2_ERROR(err, msg)						\
	if (flags & tnt2_CS038_##err##Error) {				\
									\
		handled |= tnt2_CS038_##err##Error;			\
		KRN_TRACE(0, tnt2->error.err++);				\
		KRN_ERROR(msg " (pcidev %.8x)", TNT2_PCIDEV(tnt2_io));	\
	}

	TNT2_ERROR(InFIFO, "write to full input FIFO");
	TNT2_ERROR(OutFIFO, "read from empty output FIFO");
	TNT2_ERROR(Message, "incorrect FIFO/GC access");
	TNT2_ERROR(DMA, "input FIFO write during DMA transfer");
	TNT2_ERROR(VideoFifoUnderflow, "video FIFO underflow");
	TNT2_ERROR(VSBUnderflow, "Stream B: FIFO underflow");
	TNT2_ERROR(VSAUnderflow, "Stream A: FIFO underflow");
	TNT2_ERROR(Master, "PCI bus-mastering error");
	TNT2_ERROR(OutDMA, "output FIFO read during DMA transfer");
	TNT2_ERROR(InDMAOverwrite, "InDMACount overwritten");
	TNT2_ERROR(OutDMAOverwrite, "OutDMACount overwritten");
	TNT2_ERROR(VSAInvalidInterlace, "Stream A: invalid interlace");
	TNT2_ERROR(VSBInvalidInterlace, "Stream B: invalid interlace");

#undef	TNT2_ERROR

	if (flags & ~handled) 
	{
		KRN_TRACE(0, tnt2->error.unknown++);
		KRN_ERROR("unknown TNT error(s): %.8x", flags & ~handled);
	}

	if (! handled) {

		KRN_TRACE(0, tnt2->error.no_reason++);
		KRN_ERROR("TNT error interrupt, but no errors indicated.");
	}

	TNT2_CS_OUT32(tnt2_io, handled, tnt2_CS_ErrorFlags);
#endif
}

kgi_error_t tnt2_chipset_irq_handler(tnt2_chipset_t *tnt2, 
	tnt2_chipset_io_t *tnt2_io, irq_system_t *system)
{
#if 0
	kgi_u32_t handled = 0;
	kgi_u32_t flags = tnt2_CS_IN32(tnt2_io, TNT2_CS_IntFlags);

	KRN_ASSERT(tnt);
	KRN_ASSERT(tnt2_io);

	if (flags & tnt2_CS010_DMAFlag) 
	{
		handled |= TNT2_CS010_DMAFlag;
		KRN_TRACE(0, tnt2->interrupt.DMA++);
		KRN_DEBUG(1, "DMA interrupt");
		
		if (tnt2->mode) 
		{
			tnt2_chipset_accel_schedule(&(tnt2->mode->tnt.gp_accel));
		}
	}

	if (flags & tnt2_CS010_SyncFlag) 
	{
		handled |= tnt2_CS010_SyncFlag;
		KRN_TRACE(0, tnt2->interrupt.Sync++);
		KRN_DEBUG(1, "Sync interrupt (pcidev %.8x)", TNT2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_ErrorFlag) 
	{
		handled |= TNT2_CS010_ErrorFlag;
		KRN_TRACE(0, tnt2->interrupt.Error++);
		tnt2_chipset_error_handler(tnt2, tnt2_io, system);
	}

	if (flags & tnt2_CS010_VRetraceFlag) {

		handled |= tnt2_CS010_VRetraceFlag;
		KRN_TRACE(0, tnt2->interrupt.VRetrace++);
	}

	if (flags & tnt2_CS010_ScanlineFlag) {

		handled |= tnt2_CS010_ScanlineFlag;
		KRN_TRACE(0, tnt2->interrupt.Scanline++);
	}

	if (flags & tnt2_CS010_TextureInvalidFlag) {

		handled |= tnt2_CS010_TextureInvalidFlag;
		KRN_TRACE(0, tnt2->interrupt.TextureInvalid++);
		KRN_DEBUG(1, "texture invalid interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_BypassDMAIntFlag) {

		handled |= tnt2_CS010_BypassDMAIntFlag;
		KRN_TRACE(0, tnt2->interrupt.BypassDMA++);
		KRN_DEBUG(1, "bypass DMA interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_VSBFlag) {

		handled |= tnt2_CS010_VSBFlag;
		KRN_TRACE(0, tnt2->interrupt.VSB++);
		KRN_DEBUG(1, "video stream B interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_VSAFlag) {

		handled |= tnt2_CS010_VSAFlag;
		KRN_TRACE(0, tnt2->interrupt.VSA++);
		KRN_DEBUG(1, "video stream A interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_VideoStreamSerialFlag) {

		handled |= tnt2_CS010_VideoStreamSerialFlag;
		KRN_TRACE(0, tnt2->interrupt.VideoStreamSerial++);
		KRN_DEBUG(1, "video stream serial interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_VideoDDCFlag) {

		handled |= tnt2_CS010_VideoDDCFlag;
		KRN_TRACE(0, tnt2->interrupt.VideoDDC++);
		KRN_DEBUG(1, "video DDC interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & tnt2_CS010_VideoStreamExternalFlag) {

		handled |= tnt2_CS010_VideoStreamExternalFlag;
		KRN_TRACE(0, tnt2->interrupt.VideoStreamExternal++);
		KRN_DEBUG(1, "video stream external interrupt (pcidev %.8x)",
			tnt2_PCIDEV(tnt2_io));
	}

	if (flags & ~handled) 
	{
		KRN_TRACE(0, tnt2->interrupt.not_handled++);
		KRN_ERROR("TNT: unhandled interrupt flag(s) %.8x (pcidev %.8x)", flags & ~handled, TNT2_PCIDEV(tnt2_io));
	}

	if (!flags) 
	{
		KRN_TRACE(0, tnt2->interrupt.no_reason++);
		KRN_ERROR("TNT: interrupt but no reason indicated.");
	}

	TNT2_CS_OUT32(tnt2_io, handled, TNT2_CS_IntFlags);

#endif
	return KGI_EOK;
}

#warning extern declarations outside  headers are not allowed!
extern kgi_u8_t tnt2_chipset_vga_crt_in8(tnt2_chipset_io_t *io, kgi_u_t reg);
extern kgi_u32_t tnt2_chipset_ctl_in32(tnt2_chipset_io_t *mem, kgi_u_t reg);


kgi_error_t tnt2_chipset_init(tnt2_chipset_t *tnt2, tnt2_chipset_io_t *tnt2_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = TNT2_PCIDEV(tnt2_io);

	KRN_DEBUG(2, "tnt2_chipset_init()");
	
	KRN_ASSERT(tnt2);
	KRN_ASSERT(tnt2_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "Initializing %s %s", tnt2->chipset.vendor, tnt2->chipset.model);

#define	PCICFG_SET_BASE(value, reg)		\
	pcicfg_out32(0xFFFFFFFF, reg);	\
	pcicfg_in32(reg);		\
	pcicfg_out32((value), reg)

	PCICFG_SET_BASE(tnt2_io->control.base_io,   pcidev + PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE(tnt2_io->framebuffer.base_io, pcidev + PCI_BASE_ADDRESS_1);
	
#undef	PCICFG_SET_BASE

	KRN_DEBUG(1, "PCI (re-)configuration done");

	if (tnt2->pci.command & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) 
	{
		KRN_DEBUG(2, "Chipset initialized, reading configuration");
		
		tnt2->flags |= TNT2_CF_RESTORE_INITIAL;
		
#if 1

		            tnt2->crt.htotal = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_TOTAL);
		         tnt2->crt.hblankend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_BLANK_END);
		     tnt2->crt.hretracestart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_RETRACE_START);
		       tnt2->crt.hretraceend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_RETRACE_END);
		            tnt2->crt.vtotal = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_TOTAL);
		          tnt2->crt.overflow = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_OVERFLOW);
		     tnt2->crt.presetrowscan = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_PRESET_ROW_SCAN);
		       tnt2->crt.maxscanline = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_MAX_SCAN_LINE);
		       tnt2->crt.cursorstart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_START);
		         tnt2->crt.cursorend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_END);
		     tnt2->crt.startaddrhigh = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_START_ADDR_HIGH);
		      tnt2->crt.startaddrlow = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_START_ADDR_LOW);
		tnt2->crt.cursorlocationhigh = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_LOCATION_HIGH);
		 tnt2->crt.cursorlocationlow = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_LOCATION_LOW);
		     tnt2->crt.vretracestart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_RETRACE_START);
		       tnt2->crt.vretraceend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_RETRACE_END);
		        tnt2->crt.displayend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_DISPLAY_END);
		            tnt2->crt.offset = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_OFFSET);
		 tnt2->crt.underlinelocation = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_UNDERLINE_LOCATION);
		       tnt2->crt.vblankstart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_BLANK_START);
		         tnt2->crt.vblankend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_BLANK_END);
		       tnt2->crt.modecontrol = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_MODE_CONTROL);
		       tnt2->crt.linecompare = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_LINE_COMPARE);
		          tnt2->crt.repaint0 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_REPAINT0);
		          tnt2->crt.repaint1 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_REPAINT1);
		       tnt2->crt.fifocontrol = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_FIFO_CONTROL);
		              tnt2->crt.fifo = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_FIFO);
		             tnt2->crt.extra = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_EXTRA);
		             tnt2->crt.pixel = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_PIXEL);
		            tnt2->crt.hextra = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_EXTRA);
		         tnt2->crt.grcursor0 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_GRCURSOR0);
		         tnt2->crt.grcursor1 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_GRCURSOR1);

		  tnt2->dac.grcursorstartpos = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_GRCURSOR_START_POS);
		        tnt2->dac.nvpllcoeff = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_NVPLL_COEFF);
		         tnt2->dac.mpllcoeff = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_MPLL_COEFF);
		         tnt2->dac.vpllcoeff = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_VPLL_COEFF);
		    tnt2->dac.pllcoeffselect = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_PLL_COEFF_SELECT);
		    tnt2->dac.generalcontrol = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_GENERAL_CONTROL);
#endif
		vga_text_chipset_init(&(tnt2->vga), &(tnt2_io->vga), options);

	} else 
	{

//		kgi_u_t cnt = 0;

		KRN_DEBUG(2, "Chipset not initialized, resetting");


#if 1
		            tnt2->crt.htotal = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_TOTAL);
		         tnt2->crt.hblankend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_BLANK_END);
		     tnt2->crt.hretracestart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_RETRACE_START);
		       tnt2->crt.hretraceend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_RETRACE_END);
		            tnt2->crt.vtotal = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_TOTAL);
		          tnt2->crt.overflow = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_OVERFLOW);
		     tnt2->crt.presetrowscan = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_PRESET_ROW_SCAN);
		       tnt2->crt.maxscanline = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_MAX_SCAN_LINE);
		       tnt2->crt.cursorstart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_START);
		         tnt2->crt.cursorend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_END);
		     tnt2->crt.startaddrhigh = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_START_ADDR_HIGH);
		      tnt2->crt.startaddrlow = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_START_ADDR_LOW);
		tnt2->crt.cursorlocationhigh = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_LOCATION_HIGH);
		 tnt2->crt.cursorlocationlow = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_CURSOR_LOCATION_LOW);
		     tnt2->crt.vretracestart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_RETRACE_START);
		       tnt2->crt.vretraceend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_RETRACE_END);
		        tnt2->crt.displayend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_DISPLAY_END);
		            tnt2->crt.offset = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_OFFSET);
		 tnt2->crt.underlinelocation = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_UNDERLINE_LOCATION);
		       tnt2->crt.vblankstart = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_BLANK_START);
		         tnt2->crt.vblankend = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_VERT_BLANK_END);
		       tnt2->crt.modecontrol = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_MODE_CONTROL);
		       tnt2->crt.linecompare = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_LINE_COMPARE);
		          tnt2->crt.repaint0 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_REPAINT0);
		          tnt2->crt.repaint1 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_REPAINT1);
		       tnt2->crt.fifocontrol = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_FIFO_CONTROL);
		              tnt2->crt.fifo = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_FIFO);
		             tnt2->crt.extra = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_EXTRA);
		             tnt2->crt.pixel = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_PIXEL);
		            tnt2->crt.hextra = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_HORIZ_EXTRA);
		         tnt2->crt.grcursor0 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_GRCURSOR0);
		         tnt2->crt.grcursor1 = tnt2_chipset_vga_crt_in8(tnt2_io, NV_PCRTC_GRCURSOR1);

		  tnt2->dac.grcursorstartpos = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_GRCURSOR_START_POS);
		        tnt2->dac.nvpllcoeff = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_NVPLL_COEFF);
		         tnt2->dac.mpllcoeff = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_MPLL_COEFF);
		         tnt2->dac.vpllcoeff = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_VPLL_COEFF);
		    tnt2->dac.pllcoeffselect = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_PLL_COEFF_SELECT);
		    tnt2->dac.generalcontrol = tnt2_chipset_ctl_in32(tnt2_io, NV_PRAMDAC_GENERAL_CONTROL);
#endif
		
		vga_text_chipset_init(&(tnt2->vga), &(tnt2_io->vga), options);
	}

	KRN_DEBUG(2, "Chipset enabled");

	/* Detect RAM size */
	
	tnt2->fb.boot0 = tnt2_chipset_ctl_in32(tnt2_io, NV_PFB_BOOT_0);
	
	switch(tnt2->fb.boot0 & 0x00000003)
	{
	case NV_PFB_BOOT_0_RAM_AMOUNT_4MB:
		KRN_DEBUG(1, "4MB video memory detected");
		tnt2->chipset.memory = 4 MB;
		break;
	
	case NV_PFB_BOOT_0_RAM_AMOUNT_8MB:
		KRN_DEBUG(1, "8MB video memory detected");
		tnt2->chipset.memory = 8 MB;
		break;
	
	case NV_PFB_BOOT_0_RAM_AMOUNT_16MB:
		KRN_DEBUG(1, "4MB video memory detected");
		tnt2->chipset.memory = 16 MB;
		break;
	
	case NV_PFB_BOOT_0_RAM_AMOUNT_32MB:
		KRN_DEBUG(1, "32MB video memory detected");
		tnt2->chipset.memory = 32 MB;
		break;
	}
	
	switch(tnt2->fb.boot0 & 0x00000030)
	{
	case NV_PMC_BOOT_0_MAJOR_REVISION_A:
		KRN_DEBUG(1, "Major revision A detected");
		/* video bandwidth = 800000 K/sec */
		break;
	
	default:
		/* video bandwidth = 1000000 K/sec */
		break;
	}
	
#if 0
	chip->CrystalFreqKHz   = (chip->PEXTDEV[0x00000000 / 4] & 0x00000040) ? 14318 : 13500;
	chip->CURSOR           = &(chip->PRAMIN[0x00010000 / 4 - 0x0800 / 4]);
	chip->CURSORPOS        = &(chip->PRAMDAC[0x0300 / 4]);
	chip->VBLANKENABLE     = &(chip->PCRTC[0x0140 / 4]);
	chip->VBLANK           = &(chip->PCRTC[0x0100 / 4]);
	chip->VBlankBit        = 0x00000001;
	chip->MaxVClockFreqKHz = 250000;
	chip->LockUnlockIO     = 0x3D4;
	chip->LockUnlockIndex  = 0x1F;
#endif
	
	KRN_TRACE(2, tnt2_chipset_examine(tnt2, 1));

	if (tnt2->flags & TNT2_CF_IRQ_CLAIMED) 
	{
//		tnt2_CS_OUT32(tnt2_io, TNT2_CS010_SyncFlag | TNT2_CS010_ErrorFlag | TNT2_CS010_VRetraceFlag, 0x010);
//		tnt2_CS_OUT32(tnt2_io, TNT2_CS008_SyncIntEnable | TNT2_CS008_ErrorIntEnable | TNT2_CS008_VRetraceIntEnable, 0x008);
	}

	return KGI_EOK;
}

void tnt2_chipset_done(tnt2_chipset_t *tnt2, tnt2_chipset_io_t *tnt2_io,
	const kgim_options_t *options)
{
//	pcicfg_vaddr_t pcidev = TNT2_PCIDEV(tnt2_io);

	KRN_DEBUG(2, "tnt2_chipset_done()");
	
	if (tnt2->flags & TNT2_CF_IRQ_CLAIMED) 
	{
//		TNT2_CS_OUT32(tnt2_io, TNT2_CS008_DisableInterrupts, 0x008);
	}

	if (tnt2->flags & TNT2_CF_RESTORE_INITIAL) 
	{

		KRN_DEBUG(2, "Restoring initial chipset state");
		vga_text_chipset_done((&tnt2->vga), &(tnt2_io->vga), options);
	}
}

kgi_error_t tnt2_chipset_mode_check(tnt2_chipset_t *tnt2, 
	tnt2_chipset_io_t *tnt2_io, tnt2_chipset_mode_t *tnt2_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = tnt2_mode->tnt.kgim.crt;
	kgi_u_t shift, bpf, bpc, bpp, pgm, width, lclk, pp[3];
	kgi_mmio_region_t *r;
	kgi_u_t mul, div, bpd;

	KRN_DEBUG(2, "tnt2_chipset_mode_check()");
	
	if (images != 1) 
	{
		KRN_DEBUG(2, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || (img[0].fam & KGI_AM_TEXTURE_INDEX)) 
	{
		return vga_text_chipset_mode_check(&tnt2->vga, &tnt2_io->vga, &tnt2_mode->vga, cmd, img, images);
	}
	
	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL)) 
	{
		KRN_DEBUG(2, "Image flags %.8x not supported", img[0].flags);
		
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* Check if common attributes are supported */
	switch (img[0].cam) 
	{
	case 0:
		KRN_DEBUG(2, "img[0].cam = 0");
		break;

	case KGI_AM_STENCIL | KGI_AM_Z:
		
		if ((1 != img[0].bpca[0]) || (15 != img[0].bpca[1])) 
		{
			KRN_DEBUG(2, "S%iZ%i local buffer not supported", img[0].bpca[0], img[0].bpca[1]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		break;

	case KGI_AM_Z:
		if (16 != img[0].bpca[1]) 
		{
			KRN_DEBUG(2, "Z%i local buffer not supported", img[0].bpca[0]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
	default:
		KRN_DEBUG(2, "Common attributes %.8x not supported", img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* total bits per dot */
	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO) ? (bpc + bpf * img[0].frames * 2) : (bpc + bpf*img[0].frames);

	shift = 0;
	
	switch (bpd) 
	{
	case  1:	shift++;	/* fall through	*/
	case  2:	shift++;	/* fall through */
	case  4:	shift++;	/* fall through	*/
	case  8:	shift++;	/* fall through	*/
	case 16:	shift++;	/* fall through	*/
	case 32:
		pgm = 1;
		pgm = (pgm << shift) - 1;
		break;

	default:	
		KRN_DEBUG(0, "%i bpd not supported", bpd);
		return -KGI_ERRNO(CHIPSET, FAILED);
	}

	lclk = (cmd == KGI_TC_PROPOSE) ? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) 
	{
	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE");
		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		/* If size.x or size.y are zero, default to 640x400 */
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) 
		{
			KRN_DEBUG(2, "Defaulting to 640x480");
			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/*	if virt.x and virt.y are zero, default to size;
		**	if either virt.x xor virt.y is zero, maximize the other
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) 
		{
			KRN_DEBUG(2, "Defaulting to size,x, size.y");
			
			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x) 
		{
			KRN_DEBUG(2, "virt.x = 0");
			
			img[0].virt.x = (8 * tnt2->chipset.memory) / (img[0].virt.y * bpp);

			if (img[0].virt.x > tnt2->chipset.maxdots.x) 
			{
				KRN_DEBUG(2, "virt.x > maxdots.x");
				img[0].virt.x = tnt2->chipset.maxdots.x;
			}
		}
		
		img[0].virt.x =  tnt2_chipset_calc_partials(img[0].virt.x, pp);

		if (0 == img[0].virt.y) 
		{
			KRN_DEBUG(2, "virt.y = 0");
			img[0].virt.y = (8 * tnt2->chipset.memory) / (img[0].virt.x * bpp);
		}

		/* Are we beyond the limits of the H/W? */
		if ((img[0].size.x >= tnt2->chipset.maxdots.x) || (img[0].virt.x >= tnt2->chipset.maxdots.x)) 
		{
			KRN_DEBUG(2, "%i (%i) horizontal pixels are too many", img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= tnt2->chipset.maxdots.y) || (img[0].virt.y >= tnt2->chipset.maxdots.y)) 
		{

			KRN_DEBUG(2, "%i (%i) vertical pixels are too many", img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > (8 * tnt2->chipset.memory)) 
		{
			KRN_DEBUG(2, "not enough memory for (%ipf*%if + %ipc)@%ix%i", bpf, img[0].frames, bpc, img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

		/* Take screen visible width up to next 32/64-bit word */
//		if (img[0].size.x & pgm) 
//		{
//			img[0].size.x &= ~pgm;
//			img[0].size.x += pgm + 1;
//		}

		/* Set CRT visible fields */
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		if (img[0].size.y < 400) 
		{
			dpm->dots.y += img[0].size.y;
		}
		
		KRN_DEBUG(2, "dots.x = %d, dots.y = %d", dpm->dots.x, dpm->dots.y);
		
		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(3, "KGI_TC_LOWER");
	case KGI_TC_RAISE:
		if (cmd != KGI_TC_LOWER)
		  KRN_DEBUG(3, "KGI_TC_RAISE");

		/* Adjust lclk and rclk. Use 64 bit bus on P2, 32 on P1 */
		dpm->lclk.mul = 1;
		dpm->lclk.div = 1 + pgm;

		dpm->rclk.mul = 1;
//		dpm->rclk.div = (tnt2->flags & TNT2_CF_PERMEDIA2) ? 1 : dpm->lclk.div;
		dpm->rclk.div = dpm->lclk.div;

		if (cmd == KGI_TC_LOWER) 
		{
			if (dpm->dclk < tnt2->chipset.dclk.min) 
			{
				KRN_DEBUG(1, "DCLK = %i Hz is too low", dpm->dclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}

			if (lclk > 50000000) 
			{
				dpm->dclk = 50000000 * dpm->lclk.div / dpm->lclk.mul;
			}

		} else 
		{
			if (lclk > 50000000) 
			{
				KRN_DEBUG(1, "LCLK = %i Hz is too high", lclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
		}
		
		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK");

#warning DO PROPER CHECKING!!!

		width = tnt2_chipset_calc_partials(img[0].virt.x, pp);
		KRN_ASSERT(pp[0] < 8);
		KRN_ASSERT(pp[1] < 8);
		KRN_ASSERT(pp[2] < 8);

		if (width != img[0].virt.x) 
		{
			KRN_DEBUG(2, "Invalid width!");
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
		if ((img[0].size.x >= tnt2->chipset.maxdots.x) ||
		    (img[0].size.y >= tnt2->chipset.maxdots.y) || 
		    (img[0].virt.x >= tnt2->chipset.maxdots.x) ||
		    ((img[0].virt.y * img[0].virt.x * bpp) >
		     (8 * tnt2->chipset.memory))) 
		{
			KRN_DEBUG(1, "Resolution too high: %ix%i (%ix%i)", img[0].size.x, img[0].size.y, img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (((dpm->lclk.mul != 1) && (dpm->lclk.div != 1 + pgm)) || ((dpm->rclk.mul != dpm->lclk.mul) && (dpm->rclk.div != dpm->lclk.div))) {

			KRN_DEBUG(1, "invalid LCLK (%i:%i) or CLK (%i:%i)", dpm->lclk.mul, dpm->lclk.div, dpm->rclk.mul, dpm->rclk.div);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (lclk > 50000000) 
		{
			KRN_DEBUG(1, "LCLK = %i Hz is too high\n", lclk);
			return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		}

		if (img[0].flags & KGI_IF_STEREO) 
		{
			KRN_DEBUG(1, "stereo modes not supported on TNT2");
			return -KGI_ERRNO(CHIPSET, NOSUP);
		}
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}


	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
//	tnt2_mode->tnt.VideoControl = tnt2_VC058_EnableGPVideo | tnt2_VC058_VSyncActive | tnt2_VC058_HSyncActive | ((crt_mode->x.polarity > 0) ? tnt2_VC058_HSyncHigh : tnt2_VC058_HSyncLow) | ((crt_mode->y.polarity > 0) ? tnt2_VC058_VSyncHigh : tnt2_VC058_VSyncLow) | (((tnt2->flags & tnt2_CF_PERMEDIA2) && (kgim_attr_bits(img[0].bpfa) == 8)) ? 0 : tnt2_VC058_Data64Enable);

//	tnt2_mode->tnt.ScreenBase = 0;
	
//	if ((img[0].flags & KGI_IF_STEREO) && (tnt2->flags & TNT2_CF_PERMEDIA2)) 
//	{
//		kgi_u_t 
//		Bpf = ((img[0].virt.x * img[0].virt.y * bpd) / 8 + 7) & ~7;

//		tnt2_mode->tnt.VideoControl |= tnt2_VC058_StereoEnable;
//		tnt2_mode->tnt.ScreenBaseRight = ((1 == img[0].frames) && (2*bpf == bpd)) ? tnt2_mode->tnt.ScreenBase : tnt2_mode->tnt.ScreenBase + Bpf;
//	}

	mul = dpm->lclk.mul;
	div = dpm->lclk.div;
	bpd = kgim_attr_bits(dpm->bpda);

	/* Based on 64bit units */
//	tnt2_mode->tnt.ScreenStride = img[0].virt.x * mul / div;
	
//	if ((tnt2->flags & TNT2_CF_PERMEDIA) || ((tnt2->flags & TNT2_CF_PERMEDIA2) && (bpd == 8))) 
//	{
//		tnt2_mode->tnt.ScreenStride /= 2;
//	}

	/* Based on LCLKs (32bit or 64bit units) */
	tnt2_mode->tnt.HTotal = (crt_mode->x.total * mul / div) - 2;
	tnt2_mode->tnt.HgEnd = ((crt_mode->x.total - crt_mode->x.width) * mul / div) - 1;
	tnt2_mode->tnt.HbEnd = ((crt_mode->x.total - crt_mode->x.width) * mul / div) - 1;
	tnt2_mode->tnt.HsStart = (crt_mode->x.syncstart - crt_mode->x.width) * mul / div;
	tnt2_mode->tnt.HsEnd = (crt_mode->x.syncend - crt_mode->x.width) * mul / div;

	/* For some reason the video unit seems to delay the hsync pulse */
	shift = (((dpm->dclk / 10000) * 100) / 10000) * mul / div;
	if (tnt2_mode->tnt.HsStart < shift) 
	{

		shift = tnt2_mode->tnt.HsStart;
	}
	
	tnt2_mode->tnt.HsStart -= shift;
	tnt2_mode->tnt.HsEnd -= shift;

	/* Based on lines */
	tnt2_mode->tnt.VTotal	= crt_mode->y.total - 2;
	tnt2_mode->tnt.VsStart	= crt_mode->y.syncstart - crt_mode->y.width;
	tnt2_mode->tnt.VsEnd	= crt_mode->y.syncend - crt_mode->y.width - 1;
	tnt2_mode->tnt.VbEnd	= crt_mode->y.total - crt_mode->y.width - 1;

	tnt2_chipset_calc_partials(img[0].virt.x, tnt2_mode->tnt.pp);

	/* Initialize exported resources */
	r = &tnt2_mode->tnt.aperture1;
	r->meta = tnt2;
	r->meta_io = tnt2_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name = "TNT framebuffer";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = tnt2->chipset.memory;
	r->win.bus  = tnt2_io->framebuffer.base_bus;
	r->win.phys = tnt2_io->framebuffer.base_phys;
	r->win.virt = tnt2_io->framebuffer.base_virt;

	return KGI_EOK;
}

kgi_resource_t *tnt2_chipset_mode_resource(tnt2_chipset_t *tnt2, tnt2_chipset_mode_t *tnt2_mode, kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	KRN_DEBUG(2, "tnt2_chipset_mode_resource()");
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) 
	{
		return vga_text_chipset_mode_resource(&tnt2->vga, &tnt2_mode->vga, img, images, index);
	}

	switch (index) 
	{
	case 0:	
		KRN_DEBUG(2, "Returning aperture1 as a resource");
		return	(kgi_resource_t *) &tnt2_mode->tnt.aperture1;
	case 1:	
		KRN_DEBUG(2, "Returning aperture2 as a resource");
		return	(kgi_resource_t *) &tnt2_mode->tnt.aperture2;
	}
	
	KRN_DEBUG(2, "Returning NULL, index = %d", index);
	
	return NULL;
}

void tnt2_chipset_mode_prepare(tnt2_chipset_t *tnt2, tnt2_chipset_io_t *tnt2_io, tnt2_chipset_mode_t *tnt2_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "tnt2_chipset_mode_prepare()");

	tnt2_chipset_sync(tnt2_io);
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) 
	{
		tnt2_io->flags |= TNT2_IF_VGA_DAC;

//		tnt2_MC_OUT32(tnt2_io, -1, tnt2_MC_BypassWriteMask);
//		tnt2_CS_OUT32(tnt2_io, tnt2_CS050_ControllerVGA, 0x050);
//		tnt2_CS_OUT32(tnt2_io, (tnt2_CS_IN32(tnt2_io, 0x070) |
//			tnt2_CS070_VGAEnable) & ~tnt2_CS070_VGAFixed, 0x070);
//		kgi_chipset_vga_seq_out8(&(tnt2_io->vga), SR05_VGAEnableDisplay |
//					 SR05_EnableHostMemoryAccess |
//					 SR05_EnableHostDACAccess, 0x05);
		vga_text_chipset_mode_prepare(&tnt2->vga, &tnt2_io->vga,
					 &tnt2_mode->vga, img, images);

		KRN_DEBUG(2, "Prepared for VGA-mode");
		return;
	}

	tnt2_io->flags &= ~TNT2_IF_VGA_DAC;

//	tnt2_MC_OUT32(tnt2_io, -1, tnt2_MC_BypassWriteMask);
//	tnt2_CS_OUT32(tnt2_io, tnt2_CS050_ControllerMem, 0x050);
//	tnt2_CS_OUT32(tnt2_io, tnt2_CS_IN32(tnt2_io, 0x070) &
//		~tnt2_CS070_VGAEnable, 0x070);
//	VGA_SEQ_OUT8(&(tnt2_io->vga), 0, 0x05);

	KRN_DEBUG(2, "prepared for TNT mode");
}

void tnt2_chipset_mode_enter(tnt2_chipset_t *tnt2, tnt2_chipset_io_t *tnt2_io,
	tnt2_chipset_mode_t *tnt2_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "tnt2_chipset_mode_enter()");

	vga_text_chipset_mode_enter(&tnt2->vga, &tnt2_io->vga, 
			       &tnt2_mode->vga,
			       img, images);
	KRN_DEBUG(2, "After vga_text_chipset_mode_enter()");
#if 0	
	int DCLKdiv = 8;
	int xtotal, xwidth, xblankstart, xblankend, xsyncstart, xsyncend,
	    ytotal, ywidth, yblankstart, yblankend, ysyncstart, ysyncend,
	    offset, bpp;

//	memcpy(&newmode, &reg_template, sizeof(struct riva_regs));

//	priv->newmode = &newmode;

//	KRN_DEBUG(2, "Setting mode... (%dx%d %d)", mode->request.virt.x, mode->request.virt.y, mode->request.graphtype);

	xtotal = (mode->x.total / DCLKdiv) - 5;
	xwidth = (mode->x.width / DCLKdiv) - 1;
	xblankstart = mode->x.blankstart / DCLKdiv;
	xblankend = mode->x.blankend / DCLKdiv;
	xsyncstart = mode->x.syncstart / DCLKdiv;
	xsyncend = mode->x.syncend / DCLKdiv;

	ytotal = mode->y.total - 2;
	ywidth = mode->y.width - 1;
	yblankstart = mode->y.blankstart - 1;
	yblankend = mode->y.blankend - 1;
	ysyncstart = mode->y.syncstart - 1;
	ysyncend = mode->y.syncend - 1;

	switch(mode->request.graphtype) 
	{
	case KGIGT_8BIT: bpp = 8; break;
	case KGIGT_16BIT: bpp = 16; break;
	case KGIGT_32BIT: bpp = 32; break;
	default:
		INTERNAL_ERROR;
		return;
	}

	offset = (mode->request.virt.x/8) * (bpp/8);

	tnt2->crt.htotal = xtotal & 0xFF;
	tnt2->crt.hdisplayend = xwidth & 0xFF;
	tnt2->crt.hblankstart = xblankstart & 0xFF;
	tnt2->crt.hblankend |= xblankend & 0x1F;
	tnt2->crt.hretracestart = xsyncstart & 0xFF;
	tnt2->crt.hretraceend = (xsyncend & 0x1F) | SetBitField(xblankend, 5:5, NV_PCRTC_HORIZ_RETRACE_END_HORIZ_BLANK_END_5);

	/* Vertical */
	tnt2->crt.vdisplayend = ywidth & 0xFF;
	tnt2->crt.vblankstart = yblankstart & 0xFF;
	tnt2->crt.vretracestart = ysyncstart & 0xFF;
	tnt2->crt.vretraceend = ysyncend & 0xFF;
	tnt2->crt.vblankend = yblankend & 0xFF;
	tnt2->crt.vtotal = ytotal & 0xFF;

	/* Other */

	tnt2->crt.overflow =
		SetBitField( ysyncstart,  9:9, NV_PCRTC_OVERFLOW_VERT_RETRACE_START_9) |
		SetBitField( ywidth,	  9:9, NV_PCRTC_OVERFLOW_VERT_DISPLAY_END_9) |
		SetBitField( ytotal,      9:9, NV_PCRTC_OVERFLOW_VERT_TOTAL_9) |
		SetBitField( 1,		  0:0, NV_PCRTC_OVERFLOW_LINE_COMPARE_8) |
		SetBitField( yblankstart, 8:8, NV_PCRTC_OVERFLOW_VERT_BLANK_START_8) |
		SetBitField( ysyncstart,  8:8, NV_PCRTC_OVERFLOW_VERT_RETRACE_START_8) |
		SetBitField( ywidth,      8:8, NV_PCRTC_OVERFLOW_VERT_DISPLAY_END_8) |
		SetBitField( ytotal,      8:8, NV_PCRTC_OVERFLOW_VERT_TOTAL_8);

	tnt2->crt.offset = offset & 0xFF;

	tnt2->crt.maxscanline =
		SetBitField( (mode->magnify.y == 2), 0:0, NV_PCRTC_MAX_SCAN_LINE_DOUBLE_SCAN) |
		SetBitField(1,             	     0:0, NV_PCRTC_MAX_SCAN_LINE_LINE_COMPARE_9) |
		SetBitField(yblankstart,	     9:9, NV_PCRTC_MAX_SCAN_LINE_VERT_BLANK_START_9) |
		SetBitField(0,			     4:0, NV_PCRTC_MAX_SCAN_LINE_MAX_SCAN_LINE);

//        newmode.misc_output |=
//		((mode->x.polarity > 0) ? 0 : MISC_NEG_HSYNC) |
//		((mode->y.polarity > 0) ? 0 : MISC_NEG_VSYNC);

	/* Ok.  We'll do the actual hardware programming now. */

//        kgim_clock_set_mode(dpy, mode);

	riva.CalcStateExt(&riva, &newmode.ext, bpp, mode->request.virt.x,
			   mode->x.width, xwidth, xsyncstart, xsyncend,
			   xtotal, mode->request.virt.y, ywidth, ysyncstart,
			   ysyncend, ytotal,
			   priv->clock / 1000); /* the clock is calculated in kgim_clock_set_mode */

//	ENTER_CRITICAL;
	
//	load_state(dpy, &newmode);
//	kgim_monitor_set_mode(dpy, mode);
//	kgim_ramdac_set_mode(dpy, mode);
//	kgim_accel_set_mode(dpy, mode);
	
//	LEAVE_CRITICAL;

//	NOTICE2("setting up display...");

//	chipset_setup_dpy(dpy, mode);

//	NOTICE1("Mode set complete");
	
#if DEBUG_LEVEL > 1
	KRN_DEBUG(1, "Newly programmed state:");
	dump_state(dpy, &newmode);
#endif
#endif	
	KRN_DEBUG(2, "Before...");
	tnt2->mode = tnt2_mode;
	KRN_DEBUG(2, "...After.");
}

void tnt2_chipset_mode_leave(tnt2_chipset_t *tnt2, tnt2_chipset_io_t *tnt2_io, 
	tnt2_chipset_mode_t *tnt2_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "tnt2_chipset_mode_leave()");
	
	tnt2_chipset_sync(tnt2_io);

	tnt2->mode = NULL;
}
