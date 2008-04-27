/* -----------------------------------------------------------------------------
**	PERMEDIA2 PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:13  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:13  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#include "ramdac/3Dlabs/PERMEDIA2.h"
#include "clock/3Dlabs/PERMEDIA2-meta.h"

static inline void PGC_EDAC_OUT8(pgc_clock_io_t *pgc_io, kgi_u8_t val, kgi_u8_t reg)
{
	PGC_DAC_OUT8(pgc_io, reg, PGC_DAC_EXT_ADDR);
	PGC_DAC_OUT8(pgc_io, val, PGC_DAC_EXT_DATA);
}

static inline kgi_u8_t PGC_EDAC_IN8(pgc_clock_io_t *pgc_io, kgi_u8_t reg)
{
	PGC_DAC_OUT8(pgc_io, reg, PGC_DAC_EXT_ADDR);
	return PGC_DAC_IN8(pgc_io, PGC_DAC_EXT_DATA);
}

kgi_error_t pgc_clock_init(pgc_clock_t *pgc, pgc_clock_io_t *pgc_io,
	const kgim_options_t *options)
{
	pgc->PixelClockC1 = PGC_EDAC_IN8(pgc_io, PGC_EDAC_PClkC_M);
	pgc->PixelClockC2 = PGC_EDAC_IN8(pgc_io, PGC_EDAC_PClkC_N);
	pgc->PixelClockC3 = PGC_EDAC_IN8(pgc_io, PGC_EDAC_PClkC_P);

	return KGI_EOK;
}

void pgc_clock_done(pgc_clock_t *pgc, pgc_clock_io_t *pgc_io,
	const kgim_options_t *options)
{
	PGC_EDAC_OUT8(pgc_io, pgc->PixelClockC1, PGC_EDAC_PClkC_M);
	PGC_EDAC_OUT8(pgc_io, pgc->PixelClockC2, PGC_EDAC_PClkC_N);
	PGC_EDAC_OUT8(pgc_io, pgc->PixelClockC3, PGC_EDAC_PClkC_P);
}

void pgc_clock_mode_enter(pgc_clock_t *pgc, pgc_clock_io_t *pgc_io,
	pgc_clock_mode_t *pgc_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t cnt;

	KRN_ASSERT(pgc_mode->pll.mul < 256);
	KRN_ASSERT(pgc_mode->pll.div < 256);
	KRN_ASSERT(pgc_mode->pll.p   < 8);

	/*	we use clock C, as this one is 'not valid' at boot up.
	*/
	PGC_EDAC_OUT8(pgc_io, (pgc_mode->pll.mul) & 0xFF, PGC_EDAC_PClkC_M);
	PGC_EDAC_OUT8(pgc_io, (pgc_mode->pll.div) & 0xFF, PGC_EDAC_PClkC_N); 
	PGC_EDAC_OUT8(pgc_io, ((pgc_mode->pll.p) & 0x07) | 
		PGC_EDAC_PClkP_Enable, PGC_EDAC_PClkC_P);

	PGC_CLK_OUT8(pgc_io, 3, 0);

	PGC_DAC_OUT8(pgc_io, 0x29, PGC_DAC_EXT_ADDR);
	cnt = 10000;
	while (cnt-- && (PGC_DAC_IN8(pgc_io, PGC_DAC_EXT_DATA) &
		PGC_EDAC29_PLLlocked));

	KRN_ASSERT(cnt);
}
