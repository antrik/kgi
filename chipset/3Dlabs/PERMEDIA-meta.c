/* ----------------------------------------------------------------------------
**	3Dlabs PERMEDIA chipset meta-language implementation
** ----------------------------------------------------------------------------
**	Copyright	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: PERMEDIA-meta.c,v $
**	Revision 1.1.1.1  2001/10/01 14:16:07  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.4  2001/09/17 20:36:30  seeger_s
**	- fixed namespace clash with linux kernel #defines
**	- fixed accelerator implementation
**	
**	Revision 1.3  2001/07/03 08:53:51  seeger_s
**	- implemented image resources and export
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:17  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.1.1.1 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#define	__3Dlabs_Permedia
#define	__3Dlabs_Permedia2
#define	__3Dlabd_GLINT_Delta
#include "chipset/3Dlabs/PERMEDIA.h"
#include "chipset/3Dlabs/PERMEDIA-meta.h"
#include "chipset/3Dlabs/GLINT-Delta.h"


#define	PGC_ControlBase		((kgi_u8 *) pgc_mem_control.base_virt)
#define	MEM_VGA			(PGC_ControlBase + PGC_VGABase)

#ifdef DEBUG_LEVEL
/*	print verbose chipset configuration for debugging purpose
*/
static inline void pgc_chipset_examine(pgc_chipset_t *pgc)
{
	kgi_u32_t mclk, memsize;
	kgi_u32_t foo;

	/*
	**	ChipConfig register
	*/
	foo = pgc->cs.ChipConfig;
	KRN_DEBUG(1, "PGC_CS_ChipConfig = %.8x", foo);

	mclk = 10 + ((foo & PGC_CS070_MClkPeriodMask) >>
		PGC_CS070_MClkPeriodShift);
	KRN_DEBUG(1, "MCLK period = %i ns, (freq = %i MHz)", mclk, 1000 / mclk);

	if (foo & PGC_CS070_ShortReset) {

		KRN_DEBUG(1, "short resets enabled");
	}
	if (foo & PGC_CS070_PCIRetryDisable) {

		KRN_DEBUG(1, "PCI retry disconnect-without-data disabled");
	}
	if (pgc->flags & PGC_CF_PERMEDIA) {

		if (foo & PGC_CS070_DeltaEnable) {

			KRN_DEBUG(1, "GLINT Delta mounted");
		}
		if (foo & PGC_CS070_AuxIOEnable) {

			KRN_DEBUG(1, "Auxiliary I/O base register enabled");
		}
		if (foo & PGC_CS070_AuxEnable) {

			KRN_DEBUG(1, "Auxiliary device present");
		}
	}
	if (pgc->flags & PGC_CF_PERMEDIA2) {

		if (foo & PGC_CS070_SubsystemROM) {

			KRN_DEBUG(1, "Initialize Subsystem from ROM");
		}
		if (foo & PGC_CS070_SBACapable) {

			KRN_DEBUG(1, "SBA Capable");
		}
		if (foo & PGC_CS070_AGPCapable) {

			KRN_DEBUG(1, "AGP Capable");
		}

		switch (foo & PGC_CS070_SClkSelectMask) {

		case PGC_CS070_SClkPClk:
			KRN_DEBUG(1, "SClk is PClk");
			break;

		case PGC_CS070_SClkPClk_2:
			KRN_DEBUG(1, "SClk is 1/2 PClk");
			break;

		case PGC_CS070_SClkMClk:
			KRN_DEBUG(1, "SClk is MClk");
			break;

		case PGC_CS070_SClkMClk_2:
			KRN_DEBUG(1, "SClk is 1/2 MClk");
			break;

		default:
			KRN_INTERNAL_ERROR;
		}
	}
	if (foo & PGC_CS070_VGAFixed) {

		KRN_DEBUG(1, "VGA fixed addressing enabled");
	}
	if (foo & PGC_CS070_VGAEnable) {

		KRN_DEBUG(1, "VGA subsystem enabled");
	}
	if (foo & PGC_CS070_BaseClassZero) {

		KRN_DEBUG(1, "PCI Base Class forced to zero");
	}

	/*
	**	ROM Control
	*/
	foo = pgc->mc.RomControl;
	KRN_DEBUG(1, "PGC_MC_RomControl = %.8x", foo);
	KRN_DEBUG(1, "%s fitted", (foo & PGC_MC040_SDRAM) ? "SDRAM" : "SGRAM");
	/*
	** ???	ROM pulse width only on Permedia1
	*/
	KRN_DEBUG(1, "ROM Pulse Width = %i", foo & PGC_MC040_ROMPulseWidthMask);

	/*
	**	Memory Parameters
	*/
	KRN_DEBUG(1, "PGC_MC_BootAddress = %.8x", pgc->mc.BootAddress);

	foo = pgc->mc.MemConfig;
	KRN_DEBUG(1, "PGC_MC_MemConfig = %.8x", foo);

	KRN_DEBUG(1, "RowCharge=%i, TimeRCD=%i, TimeRC=%i TimeRP=%i, "
		"CAS3Latency=%i, TimeRASMIN=%i, DeadCycleEnable=%i, "
		"BankDelay=%i, RefreshCount=%i, BurstCycle=%i",
		(foo & PGC_MC0C0_RowCharge) ? 1 : 0,
		(foo & PGC_MC0C0_tRCDMask) >> PGC_MC0C0_tRCDShift,
		(foo & PGC_MC0C0_tRCMask) >> PGC_MC0C0_tRCShift,
		(foo & PGC_MC0C0_tRPMask) >> PGC_MC0C0_tRPShift,
		(foo & PGC_MC0C0_CASLatency3) ? 1 : 0,
		(foo & PGC_MC0C0_tRMINMask) >> PGC_MC0C0_tRMINShift,
		(foo & PGC_MC0C0_DeadCycleEnable) ? 1 : 0,
		(foo & PGC_MC0C0_BankDelayMask) >> PGC_MC0C0_BankDelayShift,
		(pgc->flags & PGC_CF_PERMEDIA)
			? ((foo & PGC_MC0C0_RefreshCountMask) >>
				  PGC_MC0C0_RefreshCountShift)
			: ((foo & PGC2_MC0C0_RefreshCountMask) >>
				  PGC2_MC0C0_RefreshCountShift),
		(foo & PGC_MC0C0_BurstCycle) ? 1 : 0);

	if (pgc->flags & PGC_CF_PERMEDIA) {

		KRN_DEBUG(1, "BoardConfig=%i",
			(foo & PGC_MC0C0_BoardConfigMask) >>
			PGC_MC0C0_BoardConfigShift);
	}
	if (pgc->flags & PGC_CF_PERMEDIA2) {

		KRN_DEBUG(1, "BlockWrite1=%i",
			(foo & PGC2_MC0C0_BlockWrite1) ? 1 : 0);
	}
	memsize = (((foo & PGC_MC0C0_NumberBanksMask) >>
		PGC_MC0C0_NumberBanksShift) + 1) * 2;
	KRN_DEBUG(1, "onboard memory: %i KB auto-detected, %i KB specified",
		memsize * 1024, pgc->chipset.memory / 1024);
}
#endif	/* #if (DEBUG_LEVEL > 1)	*/

#if (DEBUG_LEVEL > 0)
/*	symbolic subsystem names for post-reset initalization debug messages.
*/
static const kgi_ascii_t *pgc_chipset_subsystem[16] =
{
	"CS",		"MC",		"GPFIFO",	"VC",
	"DAC",		"VS",		"VGA",		"R7",
	"GP",		"R9",		"RA",		"RB",
	"RC",		"RD",		"RE",		"RF"
};
#endif	/* #if (DEBUG_LEVEL > 0)	*/

/*	calculate partial product codes from width
*/
static kgi_u_t pgc_chipset_calc_partials(kgi_u_t width, kgi_u_t *pp)
{
	kgi_u_t current_width = 0;
	register kgi_u_t p0, p1, p2;

	for (p0 = 0; p0 <= 7; p0++) {

		for (p1 = 0; p1 <= p0; p1++) {

			for (p2 = 0; p2 <= p1; p2++) {

				current_width = ((16 << p0) & ~0x1F) +
					((16 << p1) & ~0x1F) +
					((16 << p2) & ~0x1F);

				if (current_width >= width) {

					KRN_DEBUG(1, "current_width = %i, "
						"pp[] = { %i, %i, %i }",
						current_width, p0, p1, p2); 
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

static inline void pgc_chipset_sync(pgc_chipset_io_t *pgc_io)
{
	kgi_u32_t count = 1, timeout = 10;

	do {
		while (count--);
		count = PGC_CS_IN32(pgc_io, PGC_CS_DMACount);
		KRN_DEBUG(1, "DMACount is %i", count);

	} while (count && timeout--);

	count = 1; timeout = 10;
	do {
		while (count++ < 255);
		count = PGC_CS_IN32(pgc_io, PGC_CS_InFIFOSpace);
		KRN_DEBUG(1, "DMACount is %i", count);

	} while (count && timeout--);

	count = 1; timeout = 10;
	while (count && timeout--) {

		count = PGC_CS_IN32(pgc_io, 0x068) &
				PGC_CS068_GraphicsProcessorActive;
	}
}

/*
**	Accelerator implementation
*/
typedef struct
{
	kgi_u32_t

#define	PGC_CORE_CONTEXT_GROUP00	0x007F8000
	Group00,
		StartXDom, dXDom, StartXSub, dXSub, StartY, dY, Count,

#define	PGC_CORE_CONTEXT_GROUP01	0x00238010
	Group01,
		RasterizerMode, YLimits, XLimits,

#define	PGC_CORE_CONTEXT_GROUP02	0x04008020
	Group02,
		PackedDataLimits,

#define	PGC_CORE_CONTEXT_GROUP03	0x021F8030
	Group03,
		ScissorMode, ScissorMinXY, ScissorMaxXY, ScreenSize,
		AreaStippleMode, WindowOrigin,

#define	PGC_CORE_CONTEXT_GROUP04	0x00FF8040
	Group04,
		AreaStipplePattern0, AreaStipplePattern1,
		AreaStipplePattern2, AreaStipplePattern3,
		AreaStipplePattern4, AreaStipplePattern5,
		AreaStipplePattern6, AreaStipplePattern7,

#define	PGC_CORE_CONTEXT_GROUP07	0x03FF8070
	Group07,
		TextureAddressMode, SStart, dSdx, dSdyDom, TStart, dTdx,
		dTdyDom, QStart, dQdx, dQdyDom,

#define	PGC_CORE_CONTEXT_GROUP0B	0x000780B0
	Group0B,
		TextureBaseAddress, TextureMapFormat, TextureDataFormat,

#define	PGC_CORE_CONTEXT_GROUP0C	0xC00180C0
	Group0C,
		Texel0, TextureReadMode, TexelLUTMode,

#define	PGC_CORE_CONTEXT_GROUP0D	0x7E7D80D0
	Group0D,
		TextureColorMode, FogMode, FogColor, FStart, dFdx, dFdyDom,
		KsStart, dKsdx, dKsdyDom, KdStart, Kddx, dKddyDom,

#define	PGC_CORE_CONTEXT_GROUP0F	0x33FF80F0
	Group0F,
		RStart, dRdx, dRdyDom, GStart, dGdx, dGdyDom, BStart, dBdx,
		dBdyDom, AStart, ColorDDAMode, ConstantColor,

#define	PGC_CORE_CONTEXT_GROUP10	0x007C8100
	Group10,
		AlphaBlendMode, DitherMode, FBSoftwareWriteMask, LogicalOpMode,
		FBWriteData,

#define	PGC_CORE_CONTEXT_GROUP11	0x43878110
	Group11,
		LBReadMode, LBReadFormat, LBSourceOffset, LBWindowBase,
		LBWriteMode, LBWriteFormat,

#define	PGC_CORE_CONTEXT_GROUP13	0x0FFF8130
	Group13,
		Window, StencilMode, StencilData, Stencil, DepthMode, Depth,
		ZStartU, ZStartL, dZdxU, dZdxL, dZdyDomU, dZdyDomL,

#define	PGC_CORE_CONTEXT_GROUP15	0x07C78150
	Group15,
		FBReadMode, FBSourceOffset, FBPixelOffset, FBWindowBase,
		FBWriteMode, FBHardwareWriteMask, FBBlockColor, FBReadPixel,

#define	PGC_CORE_CONTEXT_GROUP18	0x000F8180
	Group18,
		FilterMode, StatisticMode, MinRegion, MaxRegion,

#define	PGC_CORE_CONTEXT_GROUP1D	0xFFFF81D0
	Group1D,
		TexelLUT0, TexelLUT1, TexelLUT2, TexelLUT3,
		TexelLUT4, TexelLUT5, TexelLUT6, TexelLUT7,
		TexelLUT8, TexelLUT9, TexelLUTA, TexelLUTB,
		TexelLUTC, TexelLUTD, TexelLUTE, TexelLUTF,

#define	PGC_CORE_CONTEXT_GROUP1E	0x000781E0
	Group1E,
		YUVMode, ChromaUpperBound, ChromaLowerBound;

} pgc_chipset_core_context_t;

typedef struct
{
	kgi_u32_t

#define	PGC_DELTA_CONTEXT_GROUP23	0x1FFF8230
	Group23,
		V0Float_s, V0Float_t, V0Float_q, V0Float_Ks, V0Float_Kd,
		V0Float_red, V0Float_green, V0Float_blue, V0Float_alpha,
		V0Float_fog, V0Float_x, V0Float_y, V0Float_z,

#define	PGC_DELTA_CONTEXT_GROUP24	0x1FFF8240
	Group24,
		V1Float_s, V1Float_t, V1Float_q, V1Float_Ks, V1Float_Kd,
		V1Float_red, V1Float_green, V1Float_blue, V1Float_alpha,
		V1Float_fog, V1Float_x, V1Float_y, V1Float_z,

#define	PGC_DELTA_CONTEXT_GROUP25	0x1FFF8250
	Group25,
		V2Float_s, V2Float_t, V2Float_q, V2Float_Ks, V2Float_Kd,
		V2Float_red, V2Float_green, V2Float_blue, V2Float_alpha,
		V2Float_fog, V2Float_x, V2Float_y, V2Float_z,

#define	PGC_DELTA_CONTEXT_GROUP26	0x80018260
	Group26,
		DeltaMode, BroadcastMask;

} pgc_chipset_delta_context_t;

typedef struct
{
	kgi_u32_t

#define	PGC_PERMEDIA2_CONTEXT_GROUP18	0x60008180
	Group18,
		FBBlockColorU, FBBlockColorL,

#define	PGC_PERMEDIA2_CONTEXT_GROUP1B	0x000781B0
	Group1B,
		FBSourceBase, FBSourceDelta, Config,

#define	PGC_PERMEDIA2_CONTEXT_GROUP1E	0x001881E0
	Group1E,
		AlphaMapUpperBound, AlphaMapLowerBound,

#define	PGC_PERMEDIA2_CONTEXT_GROUP23	0x5FFF8230
	Group23,
		V0Float_s, V0Float_t, V0Float_q, V0Float_Ks, V0Float_Kd,
		V0Float_red, V0Float_green, V0Float_blue, V0Float_alpha,
		V0Float_fog, V0Float_x, V0Float_y, V0Float_z, V0Float_color,

#define	PGC_PERMEDIA2_CONTEXT_GROUP24	0x5FFF8240
	Group24,
		V1Float_s, V1Float_t, V1Float_q, V1Float_Ks, V1Float_Kd,
		V1Float_red, V1Float_green, V1Float_blue, V1Float_alpha,
		V1Float_fog, V1Float_x, V1Float_y, V1Float_z, V1Float_color,

#define	PGC_PERMEDIA2_CONTEXT_GROUP25	0x5FFF8250
	Group25,
		V2Float_s, V2Float_t, V2Float_q, V2Float_Ks, V2Float_Kd,
		V2Float_red, V2Float_green, V2Float_blue, V2Float_alpha,
		V2Float_fog, V2Float_x, V2Float_y, V2Float_z, V2Float_color,

#define	PGC_PERMEDIA2_CONTEXT_GROUP26	0x00018260
	Group26,
		DeltaMode;

#warning handle texture LUT!

} pgc_chipset_permedia2_context_t;


typedef struct
{
	kgi_accel_context_t		kgi;
	kgi_aperture_t			aperture;

	struct {

		pgc_chipset_core_context_t	core;

		union {
			pgc_chipset_delta_context_t	delta;
			pgc_chipset_permedia2_context_t	p2;
		} ext;

	} state;

} pgc_chipset_accel_context_t;

static void pgc_chipset_accel_init(kgi_accel_t *accel, void *context)
{
	pgc_chipset_t *pgc = accel->meta;
	pgc_chipset_io_t *pgc_io = accel->meta_io;
	pgc_chipset_accel_context_t *pgc_ctx = context;
	kgi_size_t offset;

	/*	To be able to load ctx->state via DMA we precalculate the
	**	aperture info needed to have it at hand when needed.
	*/
	pgc_ctx->aperture.size = sizeof(pgc_ctx->state.core);
	if (pgc->flags & PGC_CF_DELTA) {

		KRN_ASSERT(! (pgc->flags & PGC_CF_PERMEDIA2));
		pgc_ctx->aperture.size += sizeof(pgc_ctx->state.ext.delta);
	}
	if (pgc->flags & PGC_CF_PERMEDIA2) {

		KRN_ASSERT(! (pgc->flags & PGC_CF_DELTA));
		pgc_ctx->aperture.size += sizeof(pgc_ctx->state.ext.p2);
	}
	offset = (mem_vaddr_t) &pgc_ctx->state - (mem_vaddr_t) pgc_ctx;
	pgc_ctx->aperture.bus  = pgc_ctx->kgi.aperture.bus  + offset;
	pgc_ctx->aperture.phys = pgc_ctx->kgi.aperture.phys + offset;
	pgc_ctx->aperture.virt = pgc_ctx->kgi.aperture.virt + offset;

	pgc_ctx->state.core.Group00 = PGC_CORE_CONTEXT_GROUP00;
	pgc_ctx->state.core.Group01 = PGC_CORE_CONTEXT_GROUP01;
	pgc_ctx->state.core.Group02 = PGC_CORE_CONTEXT_GROUP02;
	pgc_ctx->state.core.Group03 = PGC_CORE_CONTEXT_GROUP03;
	pgc_ctx->state.core.Group04 = PGC_CORE_CONTEXT_GROUP04;
	pgc_ctx->state.core.Group07 = PGC_CORE_CONTEXT_GROUP07;
	pgc_ctx->state.core.Group0B = PGC_CORE_CONTEXT_GROUP0B;
	pgc_ctx->state.core.Group0C = PGC_CORE_CONTEXT_GROUP0C;
	pgc_ctx->state.core.Group0D = PGC_CORE_CONTEXT_GROUP0D;
	pgc_ctx->state.core.Group0F = PGC_CORE_CONTEXT_GROUP0F;
	pgc_ctx->state.core.Group10 = PGC_CORE_CONTEXT_GROUP10;
	pgc_ctx->state.core.Group11 = PGC_CORE_CONTEXT_GROUP11,
	pgc_ctx->state.core.Group13 = PGC_CORE_CONTEXT_GROUP13;
	pgc_ctx->state.core.Group15 = PGC_CORE_CONTEXT_GROUP15;
	pgc_ctx->state.core.Group18 = PGC_CORE_CONTEXT_GROUP18;
	pgc_ctx->state.core.Group1D = PGC_CORE_CONTEXT_GROUP1D;
	pgc_ctx->state.core.Group1E = PGC_CORE_CONTEXT_GROUP1E;

	if (pgc->flags & PGC_CF_DELTA) {

		pgc_ctx->state.ext.delta.Group23 = PGC_DELTA_CONTEXT_GROUP23;
		pgc_ctx->state.ext.delta.Group24 = PGC_DELTA_CONTEXT_GROUP24;
		pgc_ctx->state.ext.delta.Group25 = PGC_DELTA_CONTEXT_GROUP25;
		pgc_ctx->state.ext.delta.Group26 = PGC_DELTA_CONTEXT_GROUP26;
	}

	if (pgc->flags & PGC_CF_PERMEDIA2) {

		pgc_ctx->state.ext.p2.Group18 = PGC_PERMEDIA2_CONTEXT_GROUP18;
		pgc_ctx->state.ext.p2.Group1B = PGC_PERMEDIA2_CONTEXT_GROUP1B;
		pgc_ctx->state.ext.p2.Group1E = PGC_PERMEDIA2_CONTEXT_GROUP1E;
		pgc_ctx->state.ext.p2.Group23 = PGC_PERMEDIA2_CONTEXT_GROUP23;
		pgc_ctx->state.ext.p2.Group24 = PGC_PERMEDIA2_CONTEXT_GROUP24;
		pgc_ctx->state.ext.p2.Group25 = PGC_PERMEDIA2_CONTEXT_GROUP25;
		pgc_ctx->state.ext.p2.Group26 = PGC_PERMEDIA2_CONTEXT_GROUP26;
	}
}

static void pgc_chipset_accel_done(kgi_accel_t *accel, void *context)
{
	if (context == accel->execution.context) {

		accel->context = NULL;
	}
}

static inline void pgc_chipset_accel_save(kgi_accel_t *accel)
{
	pgc_chipset_t *pgc = accel->meta;
	pgc_chipset_io_t *pgc_io = accel->meta_io;
	pgc_chipset_accel_context_t *pgc_ctx = accel->execution.context;
	mem_vaddr_t gpr = pgc_io->control.base_virt + PGC_GPRegisterBase;

	KRN_ASSERT(pgc);
	KRN_ASSERT(pgc_io);
	KRN_ASSERT(pgc_ctx);
	KRN_ASSERT(pgc_io->control.base_virt);

	KRN_ASSERT(0 == PGC_CS_IN32(pgc_io, PGC_CS_DMACount));

#define	PGC_SAVE(reg)	\
	pgc_ctx->state.core.reg = mem_in32(gpr + (PGC_GP_##reg << 3))

	PGC_SAVE(StartXDom);
	PGC_SAVE(dXDom);
	PGC_SAVE(StartXSub);
	PGC_SAVE(dXSub);
	PGC_SAVE(StartY);
	PGC_SAVE(dY);
	PGC_SAVE(Count);

	PGC_SAVE(RasterizerMode);
	PGC_SAVE(YLimits);
	PGC_SAVE(XLimits);

	PGC_SAVE(PackedDataLimits);

	PGC_SAVE(ScissorMode);
	PGC_SAVE(ScissorMinXY);
	PGC_SAVE(ScissorMaxXY);
	PGC_SAVE(ScreenSize);
	PGC_SAVE(AreaStippleMode);
	PGC_SAVE(WindowOrigin);

	PGC_SAVE(AreaStipplePattern0);
	PGC_SAVE(AreaStipplePattern1);
	PGC_SAVE(AreaStipplePattern2);
	PGC_SAVE(AreaStipplePattern3);
	PGC_SAVE(AreaStipplePattern4);
	PGC_SAVE(AreaStipplePattern5);
	PGC_SAVE(AreaStipplePattern6);
	PGC_SAVE(AreaStipplePattern7);

	PGC_SAVE(TextureAddressMode);
	PGC_SAVE(SStart);
	PGC_SAVE(dSdx);
	PGC_SAVE(dSdyDom);
	PGC_SAVE(TStart);
	PGC_SAVE(dTdx);
	PGC_SAVE(dTdyDom);
	PGC_SAVE(QStart);
	PGC_SAVE(dQdx);
	PGC_SAVE(dQdyDom);

	PGC_SAVE(TextureBaseAddress);
	PGC_SAVE(TextureMapFormat);
	PGC_SAVE(TextureDataFormat);

	PGC_SAVE(Texel0);
	PGC_SAVE(TextureReadMode);
	PGC_SAVE(TexelLUTMode);

	PGC_SAVE(TextureColorMode);
	PGC_SAVE(FogMode);
	PGC_SAVE(FogColor);
	PGC_SAVE(FStart);
	PGC_SAVE(dFdx);
	PGC_SAVE(dFdyDom);
	PGC_SAVE(KsStart);
	PGC_SAVE(dKsdx);
	PGC_SAVE(dKsdyDom);
	PGC_SAVE(KdStart);
	PGC_SAVE(Kddx);
	PGC_SAVE(dKddyDom);

	PGC_SAVE(RStart);
	PGC_SAVE(dRdx);
	PGC_SAVE(dRdyDom);
	PGC_SAVE(GStart);
	PGC_SAVE(dGdx);
	PGC_SAVE(dGdyDom);
	PGC_SAVE(BStart);
	PGC_SAVE(dBdx);
	PGC_SAVE(dBdyDom);
	PGC_SAVE(AStart);
	PGC_SAVE(ColorDDAMode);
	PGC_SAVE(ConstantColor);

	PGC_SAVE(AlphaBlendMode);
	PGC_SAVE(DitherMode);
	PGC_SAVE(FBSoftwareWriteMask);
	PGC_SAVE(LogicalOpMode);

	PGC_SAVE(LBReadMode);
	PGC_SAVE(LBReadFormat);
	PGC_SAVE(LBSourceOffset);
	PGC_SAVE(LBWindowBase);
	PGC_SAVE(LBWriteMode);
	PGC_SAVE(LBWriteFormat);
	PGC_SAVE(FBWriteData);

	PGC_SAVE(Window);
	PGC_SAVE(StencilMode);
	PGC_SAVE(StencilData);
	PGC_SAVE(Stencil);
	PGC_SAVE(DepthMode);
	PGC_SAVE(Depth);
	PGC_SAVE(ZStartU);
	PGC_SAVE(ZStartL);
	PGC_SAVE(dZdxU);
	PGC_SAVE(dZdxL);
	PGC_SAVE(dZdyDomU);
	PGC_SAVE(dZdyDomL);

	PGC_SAVE(FBReadMode);
	PGC_SAVE(FBSourceOffset);
	PGC_SAVE(FBPixelOffset);
	PGC_SAVE(FBWindowBase);
	PGC_SAVE(FBWriteMode);
	PGC_SAVE(FBHardwareWriteMask);
	PGC_SAVE(FBBlockColor);
	PGC_SAVE(FBReadPixel);

	PGC_SAVE(FilterMode);
	PGC_SAVE(StatisticMode);
	PGC_SAVE(MinRegion);
	PGC_SAVE(MaxRegion);

	PGC_SAVE(TexelLUT0);
	PGC_SAVE(TexelLUT1);
	PGC_SAVE(TexelLUT2);
	PGC_SAVE(TexelLUT3);
	PGC_SAVE(TexelLUT4);
	PGC_SAVE(TexelLUT5);
	PGC_SAVE(TexelLUT6);
	PGC_SAVE(TexelLUT7);
	PGC_SAVE(TexelLUT8);
	PGC_SAVE(TexelLUT9);
	PGC_SAVE(TexelLUTA);
	PGC_SAVE(TexelLUTB);
	PGC_SAVE(TexelLUTC);
	PGC_SAVE(TexelLUTD);
	PGC_SAVE(TexelLUTE);
	PGC_SAVE(TexelLUTF);

	PGC_SAVE(YUVMode);
	PGC_SAVE(ChromaUpperBound);
	PGC_SAVE(ChromaLowerBound);

#undef	PGC_SAVE
#define	PGC_SAVE(reg)	\
	pgc_ctx->state.ext.delta.reg = mem_in32(gpr + (DELTA_GC_##reg << 3))

	if (pgc->flags & PGC_CF_DELTA) {

		PGC_SAVE(V0Float_s);
		PGC_SAVE(V0Float_t);
		PGC_SAVE(V0Float_q);
		PGC_SAVE(V0Float_Ks);
		PGC_SAVE(V0Float_Kd);
		PGC_SAVE(V0Float_red);
		PGC_SAVE(V0Float_green);
		PGC_SAVE(V0Float_blue);
		PGC_SAVE(V0Float_alpha);
		PGC_SAVE(V0Float_fog);
		PGC_SAVE(V0Float_x);
		PGC_SAVE(V0Float_y);
		PGC_SAVE(V0Float_z);

		PGC_SAVE(V1Float_s);
		PGC_SAVE(V1Float_t);
		PGC_SAVE(V1Float_q);
		PGC_SAVE(V1Float_Ks);
		PGC_SAVE(V1Float_Kd);
		PGC_SAVE(V1Float_red);
		PGC_SAVE(V1Float_green);
		PGC_SAVE(V1Float_blue);
		PGC_SAVE(V1Float_alpha);
		PGC_SAVE(V1Float_fog);
		PGC_SAVE(V1Float_x);
		PGC_SAVE(V1Float_y);
		PGC_SAVE(V1Float_z);

		PGC_SAVE(V2Float_s);
		PGC_SAVE(V2Float_t);
		PGC_SAVE(V2Float_q);
		PGC_SAVE(V2Float_Ks);
		PGC_SAVE(V2Float_Kd);
		PGC_SAVE(V2Float_red);
		PGC_SAVE(V2Float_green);
		PGC_SAVE(V2Float_blue);
		PGC_SAVE(V2Float_alpha);
		PGC_SAVE(V2Float_fog);
		PGC_SAVE(V2Float_x);
		PGC_SAVE(V2Float_y);
		PGC_SAVE(V2Float_z);

		PGC_SAVE(DeltaMode);
		PGC_SAVE(BroadcastMask);
	}

#undef	PGC_SAVE
#define	PGC_SAVE(reg)	\
	pgc_ctx->state.ext.p2.reg = mem_in32(gpr + (PGC_GP_##reg << 3))

	if (pgc->flags & PGC_CF_PERMEDIA2) {

		PGC_SAVE(FBBlockColorU);
		PGC_SAVE(FBBlockColorL);

		PGC_SAVE(FBSourceBase);
		PGC_SAVE(FBSourceDelta);
		PGC_SAVE(Config);

		PGC_SAVE(AlphaMapUpperBound);
		PGC_SAVE(AlphaMapLowerBound);

		PGC_SAVE(V0Float_s);
		PGC_SAVE(V0Float_t);
		PGC_SAVE(V0Float_q);
		PGC_SAVE(V0Float_Ks);
		PGC_SAVE(V0Float_Kd);
		PGC_SAVE(V0Float_red);
		PGC_SAVE(V0Float_green);
		PGC_SAVE(V0Float_blue);
		PGC_SAVE(V0Float_alpha);
		PGC_SAVE(V0Float_fog);
		PGC_SAVE(V0Float_x);
		PGC_SAVE(V0Float_y);
		PGC_SAVE(V0Float_z);
		PGC_SAVE(V0Float_color);

		PGC_SAVE(V1Float_s);
		PGC_SAVE(V1Float_t);
		PGC_SAVE(V1Float_q);
		PGC_SAVE(V1Float_Ks);
		PGC_SAVE(V1Float_Kd);
		PGC_SAVE(V1Float_red);
		PGC_SAVE(V1Float_green);
		PGC_SAVE(V1Float_blue);
		PGC_SAVE(V1Float_alpha);
		PGC_SAVE(V1Float_fog);
		PGC_SAVE(V1Float_x);
		PGC_SAVE(V1Float_y);
		PGC_SAVE(V1Float_z);
		PGC_SAVE(V1Float_color);

		PGC_SAVE(V2Float_s);
		PGC_SAVE(V2Float_t);
		PGC_SAVE(V2Float_q);
		PGC_SAVE(V2Float_Ks);
		PGC_SAVE(V2Float_Kd);
		PGC_SAVE(V2Float_red);
		PGC_SAVE(V2Float_green);
		PGC_SAVE(V2Float_blue);
		PGC_SAVE(V2Float_alpha);
		PGC_SAVE(V2Float_fog);
		PGC_SAVE(V2Float_x);
		PGC_SAVE(V2Float_y);
		PGC_SAVE(V2Float_z);
		PGC_SAVE(V2Float_color);

		PGC_SAVE(DeltaMode);
	}
#undef	PGC_SAVE
}

static inline void pgc_chipset_accel_restore(kgi_accel_t *accel)
{
	pgc_chipset_io_t *pgc_io = accel->meta_io;
	pgc_chipset_accel_context_t *pgc_ctx = accel->execution.context;

	KRN_ASSERT(0 == PGC_CS_IN32(pgc_io, PGC_CS_DMACount));

#warning flush cache-lines of the context buffer!

	PGC_CS_OUT32(pgc_io, pgc_ctx->aperture.bus, PGC_CS_DMAAddress);
	PGC_CS_OUT32(pgc_io, pgc_ctx->aperture.size >> 2, PGC_CS_DMACount);
}

static void pgc_chipset_accel_schedule(kgi_accel_t *accel)
{
#warning this must not be interrupted!
	pgc_chipset_io_t *pgc_io = accel->meta_io;
	kgi_accel_buffer_t *buffer = accel->execution.queue;

	KRN_ASSERT(buffer);

	switch (buffer->execution.state) {

	case KGI_AS_EXEC:
		/*	Execution of the current buffer finished, so we 
		**	mark it KGI_AS_IDLE and remove it from the execution
		**	queue.
		*/
		accel->execution.queue  = buffer->execution.next;
		buffer->execution.next  = NULL;
		buffer->execution.state = KGI_AS_IDLE;
#warning wakeup buffer->executed !

		if (NULL == accel->execution.queue) {

			/*	no further buffers queued, thus we are done.
			*/
#warning wakeup pgc_accel->idle
			return;
		}

		buffer = accel->execution.queue;
		KRN_ASSERT(KGI_AS_WAIT == buffer->execution.state);
		/*	fall through
		*/
	case KGI_AS_WAIT:
		/*	If neccessary we initiate a GP context switch and
		**	re-enter here when the new context is loaded.
		**	Otherwise we just initiate the buffer transfer.
		*/
		if (accel->execution.context != buffer->context) {

			if (accel->execution.context) {

				pgc_chipset_accel_save(accel);
			}
			accel->execution.context = buffer->context;
			pgc_chipset_accel_restore(accel);
			return;
		}

		KRN_ASSERT(0 == PGC_CS_IN32(pgc_io, PGC_CS_DMACount));

		buffer->execution.state = KGI_AS_EXEC;
		PGC_CS_OUT32(pgc_io, buffer->aperture.bus, PGC_CS_DMAAddress);
		PGC_CS_OUT32(pgc_io, buffer->execution.size >> 2, PGC_CS_DMACount);

		return;

	default:
		KRN_ERROR("PERMEDIA: invalid state %i for queued buffer",
			buffer->execution.state);
		KRN_INTERNAL_ERROR;
		return;
	}
}

static void pgc_chipset_accel_exec(kgi_accel_t *accel,
	kgi_accel_buffer_t *buffer)
{
#warning check/validate validate data stream!!!
#warning this must not be interrupted!
	KRN_ASSERT(KGI_AS_FILL == buffer->execution.state);

	buffer->execution.state = KGI_AS_WAIT;

	if (accel->execution.queue) {

		kgi_accel_buffer_t *queued = accel->execution.queue;
		kgi_accel_buffer_t *next   = queued->execution.next;
		
		while (next && (next->priority >= buffer->priority)) {

			queued = next;
			next   = queued->execution.next;
		}

		buffer->execution.next = next;
		queued->execution.next = buffer;

		return;
	}

	buffer->execution.next = NULL;
	accel->execution.queue = buffer;

	pgc_chipset_accel_schedule(accel);
}

/*
**	IRQ and error handlers
*/
static inline void pgc_chipset_error_handler(pgc_chipset_t *pgc, 
	pgc_chipset_io_t *pgc_io, irq_system_t *system)
{
	kgi_u32_t handled = 0;
	kgi_u32_t flags = PGC_CS_IN32(pgc_io, PGC_CS_ErrorFlags);

#define	PGC_ERROR(err, msg)						\
	if (flags & PGC_CS038_##err##Error) {				\
									\
		handled |= PGC_CS038_##err##Error;			\
		KRN_TRACE(0, pgc->error.err++);				\
		KRN_ERROR(msg " (pcidev %.8x)", PGC_PCIDEV(pgc_io));	\
	}

	PGC_ERROR(InFIFO, "write to full input FIFO");
	PGC_ERROR(OutFIFO, "read from empty output FIFO");
	PGC_ERROR(Message, "incorrect FIFO/GC access");
	PGC_ERROR(DMA, "input FIFO write during DMA transfer");
	PGC_ERROR(VideoFifoUnderflow, "video FIFO underflow");
	PGC_ERROR(VSBUnderflow, "Stream B: FIFO underflow");
	PGC_ERROR(VSAUnderflow, "Stream A: FIFO underflow");
	PGC_ERROR(Master, "PCI bus-mastering error");
	PGC_ERROR(OutDMA, "output FIFO read during DMA transfer");
	PGC_ERROR(InDMAOverwrite, "InDMACount overwritten");
	PGC_ERROR(OutDMAOverwrite, "OutDMACount overwritten");
	PGC_ERROR(VSAInvalidInterlace, "Stream A: invalid interlace");
	PGC_ERROR(VSBInvalidInterlace, "Stream B: invalid interlace");

#undef	PGC_ERROR

	if (flags & ~handled) {

		KRN_TRACE(0, pgc->error.unknown++);
		KRN_ERROR("unknown PERMEDIA error(s): %.8x", flags & ~handled);
	}

	if (! handled) {

		KRN_TRACE(0, pgc->error.no_reason++);
		KRN_ERROR("PERMEDIA error interrupt, but no errors indicated.");
	}

	PGC_CS_OUT32(pgc_io, handled, PGC_CS_ErrorFlags);
}

kgi_error_t pgc_chipset_irq_handler(pgc_chipset_t *pgc, 
	pgc_chipset_io_t *pgc_io, irq_system_t *system)
{
	kgi_u32_t handled = 0;
	kgi_u32_t flags = PGC_CS_IN32(pgc_io, PGC_CS_IntFlags);

	KRN_ASSERT(pgc);
	KRN_ASSERT(pgc_io);

	if (flags & PGC_CS010_DMAFlag) {

		handled |= PGC_CS010_DMAFlag;
		KRN_TRACE(0, pgc->interrupt.DMA++);
		KRN_DEBUG(1, "DMA interrupt");
		if (pgc->mode) {

			pgc_chipset_accel_schedule(&(pgc->mode->pgc.gp_accel));
		}
	}

	if (flags & PGC_CS010_SyncFlag) {

		handled |= PGC_CS010_SyncFlag;
		KRN_TRACE(0, pgc->interrupt.Sync++);
		KRN_DEBUG(1, "Sync interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_ErrorFlag) {

		handled |= PGC_CS010_ErrorFlag;
		KRN_TRACE(0, pgc->interrupt.Error++);
		pgc_chipset_error_handler(pgc, pgc_io, system);
	}

	if (flags & PGC_CS010_VRetraceFlag) {

		handled |= PGC_CS010_VRetraceFlag;
		KRN_TRACE(0, pgc->interrupt.VRetrace++);
	}

	if (flags & PGC_CS010_ScanlineFlag) {

		handled |= PGC_CS010_ScanlineFlag;
		KRN_TRACE(0, pgc->interrupt.Scanline++);
	}

	if (flags & PGC_CS010_TextureInvalidFlag) {

		handled |= PGC_CS010_TextureInvalidFlag;
		KRN_TRACE(0, pgc->interrupt.TextureInvalid++);
		KRN_DEBUG(1, "texture invalid interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_BypassDMAIntFlag) {

		handled |= PGC_CS010_BypassDMAIntFlag;
		KRN_TRACE(0, pgc->interrupt.BypassDMA++);
		KRN_DEBUG(1, "bypass DMA interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_VSBFlag) {

		handled |= PGC_CS010_VSBFlag;
		KRN_TRACE(0, pgc->interrupt.VSB++);
		KRN_DEBUG(1, "video stream B interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_VSAFlag) {

		handled |= PGC_CS010_VSAFlag;
		KRN_TRACE(0, pgc->interrupt.VSA++);
		KRN_DEBUG(1, "video stream A interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_VideoStreamSerialFlag) {

		handled |= PGC_CS010_VideoStreamSerialFlag;
		KRN_TRACE(0, pgc->interrupt.VideoStreamSerial++);
		KRN_DEBUG(1, "video stream serial interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_VideoDDCFlag) {

		handled |= PGC_CS010_VideoDDCFlag;
		KRN_TRACE(0, pgc->interrupt.VideoDDC++);
		KRN_DEBUG(1, "video DDC interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & PGC_CS010_VideoStreamExternalFlag) {

		handled |= PGC_CS010_VideoStreamExternalFlag;
		KRN_TRACE(0, pgc->interrupt.VideoStreamExternal++);
		KRN_DEBUG(1, "video stream external interrupt (pcidev %.8x)",
			PGC_PCIDEV(pgc_io));
	}

	if (flags & ~handled) {

		KRN_TRACE(0, pgc->interrupt.not_handled++);
		KRN_ERROR("PERMEDIA: unhandled interrupt flag(s) %.8x "
			"(pcidev %.8x)", flags & ~handled, PGC_PCIDEV(pgc_io));
	}

	if (! flags) {

		KRN_TRACE(0, pgc->interrupt.no_reason++);
		KRN_ERROR("PERMEDIA: interrupt but no reason indicated.");
	}

	PGC_CS_OUT32(pgc_io, handled, PGC_CS_IntFlags);

	return KGI_EOK;
}


kgi_error_t pgc_chipset_init(pgc_chipset_t *pgc, pgc_chipset_io_t *pgc_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = PGC_PCIDEV(pgc_io);

	KRN_ASSERT(pgc);
	KRN_ASSERT(pgc_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "initializing %s %s",
		pgc->chipset.vendor, pgc->chipset.model);

	PCICFG_SET_BASE32(pgc_io->control.base_io,   pcidev + PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE32(pgc_io->aperture1.base_io, pcidev + PCI_BASE_ADDRESS_1);
	PCICFG_SET_BASE32(pgc_io->aperture2.base_io, pcidev + PCI_BASE_ADDRESS_2);
	PCICFG_SET_BASE32(pgc_io->rom.base_io | PCI_ROM_ADDRESS_ENABLE, pcidev + PCI_ROM_ADDRESS);

	if (pgc->flags & PGC_CF_PERMEDIA) {

		PCICFG_SET_BASE32(pgc_io->auxiliary.base_io, 
			pcidev + PCI_BASE_ADDRESS_3);
		PCICFG_SET_BASE32(pgc_io->delta.base_io,
			pcidev + PCI_BASE_ADDRESS_4);
	}

	KRN_DEBUG(2, "PCI (re-)configuration done");

	pcicfg_out16(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER,
		pcidev + PCI_COMMAND);

	if (pgc->pci.Command & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) {

		KRN_DEBUG(2, "chipset initialized, reading configuration");
		pgc->flags |= PGC_CF_RESTORE_INITIAL;

		pgc->cs.IntEnable   = PGC_CS_IN32(pgc_io, PGC_CS_IntEnable);
		pgc->cs.VClkCtl     = PGC_CS_IN32(pgc_io, PGC_CS_VClkCtl);
		pgc->cs.ApertureOne = PGC_CS_IN32(pgc_io, PGC_CS_ApertureOne);
		pgc->cs.ApertureTwo = PGC_CS_IN32(pgc_io, PGC_CS_ApertureTwo);
		pgc->cs.FIFODiscon  = PGC_CS_IN32(pgc_io, PGC_CS_FIFODiscon);
		pgc->cs.ChipConfig  = PGC_CS_IN32(pgc_io, PGC_CS_ChipConfig);

		pgc->mc.RomControl  = PGC_MC_IN32(pgc_io, PGC_MC_RomControl);
		pgc->mc.BootAddress = PGC_MC_IN32(pgc_io, PGC_MC_BootAddress);
		pgc->mc.MemConfig   = PGC_MC_IN32(pgc_io, PGC_MC_MemConfig);
		pgc->mc.BypassWriteMask	=
			PGC_MC_IN32(pgc_io, PGC_MC_BypassWriteMask);
		pgc->mc.FramebufferWriteMask =
			PGC_MC_IN32(pgc_io, PGC_MC_FramebufferWriteMask);

		pgc->vc.ScreenBase   = PGC_VC_IN32(pgc_io, PGC_VC_ScreenBase);
		pgc->vc.ScreenStride = PGC_VC_IN32(pgc_io, PGC_VC_ScreenStride);
		pgc->vc.HTotal       = PGC_VC_IN32(pgc_io, PGC_VC_HTotal);
		pgc->vc.HgEnd        = PGC_VC_IN32(pgc_io, PGC_VC_HgEnd);
		pgc->vc.HbEnd        = PGC_VC_IN32(pgc_io, PGC_VC_HbEnd);
		pgc->vc.HsStart      = PGC_VC_IN32(pgc_io, PGC_VC_HsStart);
		pgc->vc.HsEnd        = PGC_VC_IN32(pgc_io, PGC_VC_HsEnd);
		pgc->vc.VTotal       = PGC_VC_IN32(pgc_io, PGC_VC_VTotal);
		pgc->vc.VbEnd        = PGC_VC_IN32(pgc_io, PGC_VC_VbEnd);
		pgc->vc.VsStart      = PGC_VC_IN32(pgc_io, PGC_VC_VsStart);
		pgc->vc.VsEnd        = PGC_VC_IN32(pgc_io, PGC_VC_VsEnd);
		pgc->vc.VideoControl = PGC_VC_IN32(pgc_io, PGC_VC_VideoControl);
		pgc->vc.InterruptLine =
			PGC_VC_IN32(pgc_io, PGC_VC_InterruptLine);

		pgc->vga_SEQ_ControlReg =
			VGA_SEQ_IN8(&(pgc_io->vga), PGC_SEQ_VGAControlReg);
		pgc->vga_GRC_Mode640Reg =
			VGA_GRC_IN8(&(pgc_io->vga), PGC_GRC_Mode640Reg);

		vga_text_chipset_init(&(pgc->vga), &(pgc_io->vga), options);

	} else {

		kgi_u_t cnt = 0;

		KRN_DEBUG(2, "chipset not initialized, resetting");

		PGC_CS_OUT32(pgc_io, PGC_CS000_SoftwareReset, 0x000);
		while ((PGC_CS_IN32(pgc_io, 0x000) & PGC_CS000_SoftwareReset) &&
			(cnt++ < 10000));
		if (cnt == 10000) {

			KRN_ERROR("PERMEDIA reset timeout!");
		}

		pgc->cs.ChipConfig  = PGC_CS_IN32(pgc_io, PGC_CS_ChipConfig);

		if ((pgc->flags & PGC_CF_PERMEDIA2) &&
			(pgc->cs.ChipConfig & PGC_CS070_SubsystemROM)) {

			const kgi_u16_t *offset;
			const kgi_u32_t *table;
			kgi_u32_t count;

			offset = (kgi_u16_t *) (pgc_io->rom.base_virt + 0x001C);
			table  = (kgi_u32_t *) (pgc_io->rom.base_virt +
					KGI_LE_u16(*offset));
			count = KGI_LE_u32(*table);

			KRN_DEBUG(2, "post-reset table @%.4x, sign. %.8x",
				KGI_LE_u16(*offset), KGI_LE_u32(*table));
	
			if (0x3D3D0000 == (count & 0xFFFF0000)) {

				kgi_u32_t val, reg;

				table++;
				count &= 0xFFFF;

				while (count--) {

					reg = KGI_LE_u32(table[0]);
					val = KGI_LE_u32(table[1]);

					KRN_DEBUG(2, "post-reset %s%.3X = %.8x",
						pgc_chipset_subsystem[reg>>12],
						reg & 0xFFF, val);

					PGC_CS_OUT32(pgc_io, val, reg);
					table += 2;
				}
				PGC_MC_OUT32(pgc_io, 0, PGC_MC_Reboot);
			}
		}

		pgc->mc.RomControl  = PGC_MC_IN32(pgc_io, PGC_MC_RomControl);
		pgc->mc.BootAddress = PGC_MC_IN32(pgc_io, PGC_MC_BootAddress);
		pgc->mc.MemConfig   = PGC_MC_IN32(pgc_io, PGC_MC_MemConfig);

		vga_text_chipset_init(&(pgc->vga), &(pgc_io->vga), options);
	}

	KRN_DEBUG(2, "chipset enabled");

	pgc->chipset.memory = options->chipset->memory
		? options->chipset->memory
		: 2 MB * (((pgc->mc.MemConfig & PGC_MC0C0_NumberBanksMask) >>
			PGC_MC0C0_NumberBanksShift) + 1);

	KRN_TRACE(2, pgc_chipset_examine(pgc));

	if (pgc->flags & PGC_CF_IRQ_CLAIMED) {

		PGC_CS_OUT32(pgc_io,
			PGC_CS010_SyncFlag |
			PGC_CS010_ErrorFlag |
			PGC_CS010_VRetraceFlag, 0x010);
		PGC_CS_OUT32(pgc_io,
			PGC_CS008_SyncIntEnable |
			PGC_CS008_ErrorIntEnable |
			PGC_CS008_VRetraceIntEnable, 0x008);
	}

	return KGI_EOK;
}

void pgc_chipset_done(pgc_chipset_t *pgc, pgc_chipset_io_t *pgc_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = PGC_PCIDEV(pgc_io);

	if (pgc->flags & PGC_CF_IRQ_CLAIMED) {

		PGC_CS_OUT32(pgc_io, PGC_CS008_DisableInterrupts, 0x008);
	}

	if (pgc->flags & PGC_CF_RESTORE_INITIAL) {

		KRN_DEBUG(2, "restoring initial chipset state");
		PGC_CS_OUT32(pgc_io, pgc->cs.IntEnable, PGC_CS_IntEnable);
		PGC_CS_OUT32(pgc_io, pgc->cs.VClkCtl, PGC_CS_VClkCtl);
		PGC_CS_OUT32(pgc_io, pgc->cs.ApertureOne, PGC_CS_ApertureOne);
		PGC_CS_OUT32(pgc_io, pgc->cs.ApertureTwo, PGC_CS_ApertureTwo);
		PGC_CS_OUT32(pgc_io, pgc->cs.FIFODiscon, PGC_CS_FIFODiscon);
		PGC_CS_OUT32(pgc_io, pgc->cs.ChipConfig, PGC_CS_ChipConfig);

		PGC_MC_OUT32(pgc_io, pgc->mc.RomControl, PGC_MC_RomControl);
		PGC_MC_OUT32(pgc_io, pgc->mc.BootAddress, PGC_MC_BootAddress);
		PGC_MC_OUT32(pgc_io, pgc->mc.MemConfig, PGC_MC_MemConfig);
		PGC_MC_OUT32(pgc_io, pgc->mc.BypassWriteMask,
			PGC_MC_BypassWriteMask);
		PGC_MC_OUT32(pgc_io, pgc->mc.FramebufferWriteMask,
			PGC_MC_FramebufferWriteMask);

		PGC_VC_OUT32(pgc_io, pgc->vc.ScreenBase, PGC_VC_ScreenBase);
		PGC_VC_OUT32(pgc_io, pgc->vc.ScreenStride, PGC_VC_ScreenStride);
		PGC_VC_OUT32(pgc_io, pgc->vc.HTotal, PGC_VC_HTotal);
		PGC_VC_OUT32(pgc_io, pgc->vc.HgEnd, PGC_VC_HgEnd);
		PGC_VC_OUT32(pgc_io, pgc->vc.HbEnd, PGC_VC_HbEnd);
		PGC_VC_OUT32(pgc_io, pgc->vc.HsStart, PGC_VC_HsStart);
		PGC_VC_OUT32(pgc_io, pgc->vc.HsEnd, PGC_VC_HsEnd);
		PGC_VC_OUT32(pgc_io, pgc->vc.VTotal, PGC_VC_VTotal);
		PGC_VC_OUT32(pgc_io, pgc->vc.VbEnd, PGC_VC_VbEnd);
		PGC_VC_OUT32(pgc_io, pgc->vc.VsStart, PGC_VC_VsStart);
		PGC_VC_OUT32(pgc_io, pgc->vc.VsEnd, PGC_VC_VsEnd);
		PGC_VC_OUT32(pgc_io, pgc->vc.VideoControl, PGC_VC_VideoControl);
		PGC_VC_OUT32(pgc_io, pgc->vc.InterruptLine, 
			PGC_VC_InterruptLine);

		VGA_GRC_OUT8(&(pgc_io->vga), pgc->vga_GRC_Mode640Reg,
			PGC_GRC_Mode640Reg);
		VGA_SEQ_OUT8(&(pgc_io->vga), pgc->vga_SEQ_ControlReg,
			PGC_SEQ_VGAControlReg);

		vga_text_chipset_done((&pgc->vga), &(pgc_io->vga), options);
	}

	pcicfg_out32(pgc->pci.Command,	pcidev + PCI_COMMAND);
	pcicfg_out32(pgc->pci.LatTimer,	pcidev + PCI_LATENCY_TIMER);
	pcicfg_out32(pgc->pci.IntLine,	pcidev + PCI_INTERRUPT_LINE);

	PCICFG_SET_BASE32(pgc->pci.BaseAddr0, pcidev + PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE32(pgc->pci.BaseAddr1, pcidev + PCI_BASE_ADDRESS_1);
	PCICFG_SET_BASE32(pgc->pci.BaseAddr2, pcidev + PCI_BASE_ADDRESS_2);
	PCICFG_SET_BASE32(pgc->pci.BaseAddr3, pcidev + PCI_BASE_ADDRESS_3);
	PCICFG_SET_BASE32(pgc->pci.BaseAddr4, pcidev + PCI_BASE_ADDRESS_4);
	PCICFG_SET_BASE32(pgc->pci.RomAddr,   pcidev + PCI_ROM_ADDRESS);
}

kgi_error_t pgc_chipset_mode_check(pgc_chipset_t *pgc, pgc_chipset_io_t *pgc_io,
	pgc_chipset_mode_t *pgc_mode, kgi_timing_command_t cmd,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = pgc_mode->pgc.kgim.crt;
	kgi_u_t shift, bpf, bpc, bpp, pgm, width, lclk, pp[3];
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t mul, div, bpd;

	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
		(img[0].fam & KGI_AM_TEXTURE_INDEX)) {

		return vga_text_chipset_mode_check(&pgc->vga, &pgc_io->vga,
			&pgc_mode->vga, cmd, img, images);
	}
	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL)) {

		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	check if common attributes are supported.
	*/
	switch (img[0].cam) {

	case 0:
		break;

	case KGI_AM_STENCIL | KGI_AM_Z:
		if ((1 != img[0].bpca[0]) || (15 != img[0].bpca[1])) {

			KRN_DEBUG(1, "S%iZ%i local buffer not supported",
				img[0].bpca[0], img[0].bpca[1]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		break;

	case KGI_AM_Z:
		if (16 != img[0].bpca[1]) {

			KRN_DEBUG(1,"Z%i local buffer not supported",
				img[0].bpca[0]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
	default:
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

	shift = 0;
	switch (bpd) {

	case  1:	shift++;	/* fall through	*/
	case  2:	shift++;	/* fall through */
	case  4:	shift++;	/* fall through	*/
	case  8:	shift++;	/* fall through	*/
	case 16:	shift++;	/* fall through	*/
	case 32:	pgm = (pgc->flags & PGC_CF_PERMEDIA2)
				 ? ((bpd == 8) ? 1 : 2) : 1;
			pgm = (pgm << shift) - 1;
			break;

	default:	KRN_DEBUG(0, "%i bpd not supported", bpd);
			return -KGI_ERRNO(CHIPSET, FAILED);
	}

	lclk = (cmd == KGI_TC_PROPOSE)
		? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) {

	case KGI_TC_PROPOSE:

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

			img[0].virt.x = (8 * pgc->chipset.memory) / 
				(img[0].virt.y * bpp);

			if (img[0].virt.x > pgc->chipset.maxdots.x) {

				img[0].virt.x = pgc->chipset.maxdots.x;
			}
		}
		img[0].virt.x =  pgc_chipset_calc_partials(img[0].virt.x, pp);

		if (0 == img[0].virt.y) {

			img[0].virt.y = (8 * pgc->chipset.memory) /
				(img[0].virt.x * bpp);
		}

		/*	Are we beyond the limits of the H/W?
		*/
		if ((img[0].size.x >= pgc->chipset.maxdots.x) || 
			(img[0].virt.x >= pgc->chipset.maxdots.x)) {

			KRN_DEBUG(1, "%i (%i) horizontal pixels are too many",
				img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= pgc->chipset.maxdots.y) ||
			(img[0].virt.y >= pgc->chipset.maxdots.y)) {

			KRN_DEBUG(1, "%i (%i) vertical pixels are too many",
				img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 
			(8 * pgc->chipset.memory)) {

			KRN_DEBUG(1, "not enough memory for (%ipf*%if + %ipc)@"
				"%ix%i", bpf, img[0].frames, bpc,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

		/*	Take screen visible width up to next 32/64-bit word
		*/
		if (img[0].size.x & pgm) {

			img[0].size.x &= ~pgm;
			img[0].size.x += pgm + 1;
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
	case KGI_TC_RAISE:

		/*	adjust lclk and rclk. Use 64 bit bus on P2, 32 on P1
		*/
		dpm->lclk.mul = 1;
		dpm->lclk.div = 1 + pgm;

		dpm->rclk.mul = 1;
		dpm->rclk.div = (pgc->flags & PGC_CF_PERMEDIA2)
					? 1 : dpm->lclk.div;

		if (cmd == KGI_TC_LOWER) {

			if (dpm->dclk < pgc->chipset.dclk.min) {

				KRN_DEBUG(1, "DCLK = %i Hz is too low",
					dpm->dclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}

			if (lclk > 50000000) {

				dpm->dclk = 50000000 * dpm->lclk.div /
					dpm->lclk.mul;
			}

		} else {

			if (lclk > 50000000) {

				KRN_DEBUG(1, "LCLK = %i Hz is too high", lclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
		}
		return KGI_EOK;


	case KGI_TC_CHECK:

#warning DO PROPER CHECKING!!!

		width = pgc_chipset_calc_partials(img[0].virt.x, pp);
		KRN_ASSERT(pp[0] < 8);
		KRN_ASSERT(pp[1] < 8);
		KRN_ASSERT(pp[2] < 8);

		if (width != img[0].virt.x) {

			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((img[0].size.x >= pgc->chipset.maxdots.x) ||
			(img[0].size.y >= pgc->chipset.maxdots.y) || 
			(img[0].virt.x >= pgc->chipset.maxdots.x) ||
			((img[0].virt.y * img[0].virt.x * bpp) >
				(8 * pgc->chipset.memory))) {

			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
				img[0].size.x, img[0].size.y,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (((dpm->lclk.mul != 1) &&
			(dpm->lclk.div != 1 + pgm)) ||
			((dpm->rclk.mul != dpm->lclk.mul) &&
			(dpm->rclk.div != dpm->lclk.div))) {

			KRN_DEBUG(1, "invalid LCLK (%i:%i) or CLK (%i:%i)", 
				dpm->lclk.mul, dpm->lclk.div,
				dpm->rclk.mul, dpm->rclk.div);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (lclk > 50000000) {

			KRN_DEBUG(1, "LCLK = %i Hz is too high\n", lclk);
			return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		}

		if ((img[0].flags & KGI_IF_STEREO) &&
			!(pgc->flags & PGC_CF_PERMEDIA2)) {

			KRN_DEBUG(1, "stereo modes not supported on PERMEDIA");
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
	pgc_mode->pgc.VideoControl = PGC_VC058_EnableGPVideo |
		PGC_VC058_VSyncActive | PGC_VC058_HSyncActive |
		((crt_mode->x.polarity > 0)
			? PGC_VC058_HSyncHigh : PGC_VC058_HSyncLow) |
		((crt_mode->y.polarity > 0)
			? PGC_VC058_VSyncHigh : PGC_VC058_VSyncLow) |
		(((pgc->flags & PGC_CF_PERMEDIA2) && 
			(kgim_attr_bits(img[0].bpfa) == 8))
			? 0 : PGC_VC058_Data64Enable);

	pgc_mode->pgc.ScreenBase = 0;
	if ((img[0].flags & KGI_IF_STEREO) && (pgc->flags & PGC_CF_PERMEDIA2)) {

		kgi_u_t 
		Bpf = ((img[0].virt.x * img[0].virt.y * bpd) / 8 + 7) & ~7;

		pgc_mode->pgc.VideoControl |= PGC_VC058_StereoEnable;
		pgc_mode->pgc.ScreenBaseRight = 
			((1 == img[0].frames) && (2*bpf == bpd))
				? pgc_mode->pgc.ScreenBase
				: pgc_mode->pgc.ScreenBase + Bpf;
	}

	mul = dpm->lclk.mul;
	div = dpm->lclk.div;
	bpd = kgim_attr_bits(dpm->bpda);

	/*	based on 64bit units
	*/
	pgc_mode->pgc.ScreenStride = img[0].virt.x * mul / div;
	if ((pgc->flags & PGC_CF_PERMEDIA) || 
		((pgc->flags & PGC_CF_PERMEDIA2) && (bpd == 8))) {

		pgc_mode->pgc.ScreenStride /= 2;
	}

	/*	based on LCLKs (32bit or 64bit units)
	*/
	pgc_mode->pgc.HTotal
		= (crt_mode->x.total * mul/div) - 2;
	pgc_mode->pgc.HgEnd
		= ((crt_mode->x.total - crt_mode->x.width) * mul/div) - 1;
	pgc_mode->pgc.HbEnd
		= ((crt_mode->x.total - crt_mode->x.width) * mul/div) - 1;
	pgc_mode->pgc.HsStart
		= (crt_mode->x.syncstart - crt_mode->x.width) * mul/div;
	pgc_mode->pgc.HsEnd
		= (crt_mode->x.syncend - crt_mode->x.width) * mul/div;

	/*	For some reason the video unit seems to delay the hsync pulse.
	*/
	shift = (((dpm->dclk / 10000) * 100) / 10000) * mul/div;
	if (pgc_mode->pgc.HsStart < shift) {

		shift = pgc_mode->pgc.HsStart;
	}
	pgc_mode->pgc.HsStart -= shift;
	pgc_mode->pgc.HsEnd -= shift;

	/*	based on lines
	*/
	pgc_mode->pgc.VTotal	= crt_mode->y.total - 2;
	pgc_mode->pgc.VsStart	= crt_mode->y.syncstart - crt_mode->y.width;
	pgc_mode->pgc.VsEnd	= crt_mode->y.syncend - crt_mode->y.width - 1;
	pgc_mode->pgc.VbEnd	= crt_mode->y.total - crt_mode->y.width - 1;

	pgc_chipset_calc_partials(img[0].virt.x, pgc_mode->pgc.pp);

	/*	initialize exported resources
	*/
	r = &pgc_mode->pgc.aperture1;
	r->meta = pgc;
	r->meta_io = pgc_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name = "PERMEDIA aperture1";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = pgc->chipset.memory;
	r->win.bus  = pgc_io->aperture1.base_bus;
	r->win.phys = pgc_io->aperture1.base_phys;
	r->win.virt = pgc_io->aperture1.base_virt;

	r = &pgc_mode->pgc.aperture2;
	r->meta = pgc;
	r->meta_io = pgc_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name	= "PERMEDIA aperture2";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = pgc->chipset.memory;
	r->win.bus  = pgc_io->aperture2.base_bus;
	r->win.phys = pgc_io->aperture2.base_phys;
	r->win.virt = pgc_io->aperture2.base_virt;

	r = &pgc_mode->pgc.gp_fifo;
	r->meta = pgc;
	r->meta_io = pgc_io;
	r->type = KGI_RT_MMIO_PRIVATE;
	r->prot = KGI_PF_LIB | KGI_PF_DRV | KGI_PF_WRITE_ORDER;
	r->name = "PERMEDIA gp fifo";
	r->access = 32;
	r->align  = 32;
	r->size   = r->win.size = PGC_GPFIFOAccessSize;
	r->win.bus  = PGC_GPFIFOAccessBase + pgc_io->control.base_bus;
	r->win.phys = PGC_GPFIFOAccessBase + pgc_io->control.base_phys;
	r->win.virt = PGC_GPFIFOAccessBase + pgc_io->control.base_virt;

	r = &pgc_mode->pgc.gp_regs;
	r->meta = pgc;
	r->meta_io = pgc_io;
	r->type = KGI_RT_MMIO_PRIVATE;
	r->prot = KGI_PF_LIB | KGI_PF_DRV | KGI_PF_WRITE_ORDER;
	r->name = "PERMEDIA gp control";
	r->access = 32;
	r->align  = 32;
	r->size   = r->win.size = PGC_GPRegisterSize;
	r->win.bus  = PGC_GPRegisterBase + pgc_io->control.base_bus;
	r->win.phys = PGC_GPRegisterBase + pgc_io->control.base_phys;
	r->win.virt = PGC_GPRegisterBase + pgc_io->control.base_virt;

	a = &pgc_mode->pgc.gp_accel;
	a->meta = pgc;
	a->meta_io = pgc_io;
	a->type = KGI_RT_ACCELERATOR;
	a->prot = KGI_PF_LIB | KGI_PF_DRV; 
	a->name = "PERMEDIA graphics processor";
	a->flags |= KGI_AF_DMA_BUFFERS;
	a->buffers = 3;
	a->buffer_size = 8 KB;
	a->context_size = sizeof(pgc_chipset_accel_context_t);
	a->execution.context = NULL;
	a->execution.queue = NULL;
#warning initialize a->idle!!!
	a->Init = pgc_chipset_accel_init;
	a->Done = pgc_chipset_accel_done;
	a->Exec = pgc_chipset_accel_exec;

	return KGI_EOK;
}

kgi_resource_t *pgc_chipset_mode_resource(pgc_chipset_t *pgc, 
	pgc_chipset_mode_t *pgc_mode, kgi_image_mode_t *img,
	kgi_u_t images, kgi_u_t index)
{
	if (img[0].fam & KGI_AM_TEXTURE_INDEX) {

		return vga_text_chipset_mode_resource(&pgc->vga, &pgc_mode->vga,
			img, images, index);
	}

	switch (index) {

	case 0:	return	(kgi_resource_t *) &pgc_mode->pgc.aperture1;
	case 1:	return	(kgi_resource_t *) &pgc_mode->pgc.aperture2;
	case 2:	return	(kgi_resource_t *) &pgc_mode->pgc.gp_fifo;
	case 3:	return	(kgi_resource_t *) &pgc_mode->pgc.gp_regs;

	}
	return NULL;
}

kgi_resource_t *pgc_chipset_image_resource(pgc_chipset_t *pgc,
	pgc_chipset_mode_t *pgc_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT(image == 0);

	if (img[0].fam & KGI_AM_TEXTURE_INDEX) {

		return vga_text_chipset_image_resource(&(pgc->vga),
			&(pgc_mode->vga), img, image, index);
	}
	return NULL;
}

void pgc_chipset_mode_prepare(pgc_chipset_t *pgc, pgc_chipset_io_t *pgc_io,
	pgc_chipset_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	pgc_chipset_sync(pgc_io);

	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		pgc_io->flags |= PGC_IF_VGA_DAC;

		PGC_MC_OUT32(pgc_io, -1, PGC_MC_BypassWriteMask);
		PGC_CS_OUT32(pgc_io, PGC_CS050_ControllerVGA, 0x050);
		PGC_CS_OUT32(pgc_io, (PGC_CS_IN32(pgc_io, 0x070) |
			PGC_CS070_VGAEnable) & ~PGC_CS070_VGAFixed, 0x070);
		VGA_SEQ_OUT8(&(pgc_io->vga), PGC_SR05_VGAEnableDisplay |
			PGC_SR05_EnableHostMemoryAccess |
			PGC_SR05_EnableHostDACAccess, 0x05);
		vga_text_chipset_mode_prepare(&pgc->vga, &pgc_io->vga,
			&pgc_mode->vga, img, images);

		KRN_DEBUG(2, "prepared for VGA-mode");
		return;
	}

	pgc_io->flags &= ~PGC_IF_VGA_DAC;

	PGC_MC_OUT32(pgc_io, -1, PGC_MC_BypassWriteMask);
	PGC_CS_OUT32(pgc_io, PGC_CS050_ControllerMem, 0x050);
	PGC_CS_OUT32(pgc_io, PGC_CS_IN32(pgc_io, 0x070) &
		~PGC_CS070_VGAEnable, 0x070);
	VGA_SEQ_OUT8(&(pgc_io->vga), 0, 0x05);

	KRN_DEBUG(2, "prepared for PGC mode");
}

void pgc_chipset_mode_enter(pgc_chipset_t *pgc, pgc_chipset_io_t *pgc_io,
	pgc_chipset_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		vga_text_chipset_mode_enter(&pgc->vga, &pgc_io->vga,
			&pgc_mode->vga, img, images);
		return;
	}

	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.ScreenStride, PGC_VC_ScreenStride);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.HTotal,  PGC_VC_HTotal);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.HgEnd,   PGC_VC_HgEnd);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.HbEnd,   PGC_VC_HbEnd);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.HsStart, PGC_VC_HsStart);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.HsEnd,   PGC_VC_HsEnd);

	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.VTotal,  PGC_VC_VTotal);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.VbEnd,   PGC_VC_VbEnd);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.VsStart, PGC_VC_VsStart);
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.VsEnd,   PGC_VC_VsEnd);
	
	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.VideoControl, PGC_VC_VideoControl);
	PGC_VC_OUT32(pgc_io, PGC_MAX_InterruptLine,  PGC_VC_InterruptLine);

	PGC_VC_OUT32(pgc_io, pgc_mode->pgc.ScreenBase, PGC_VC_ScreenBase);
	if (pgc->flags & PGC_CF_PERMEDIA2) {

		PGC_VC_OUT32(pgc_io, pgc_mode->pgc.ScreenBaseRight,
			PGC_VC_ScreenBaseRight);
	}

	pgc->mode = pgc_mode;
}

void pgc_chipset_mode_leave(pgc_chipset_t *pgc, pgc_chipset_io_t *pgc_io,
	pgc_chipset_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	pgc_chipset_sync(pgc_io);

	pgc->mode = NULL;
}
