/* -----------------------------------------------------------------------------
**	PERMEDIA2v PLL meta language implementation
** -----------------------------------------------------------------------------
**	Copyright	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: PERMEDIA2v-meta.c,v $
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

#include "ramdac/3Dlabs/PERMEDIA2v.h"
#include "clock/3Dlabs/PERMEDIA2v-meta.h"

static inline void PGC2v_EDAC_OUT8(pgc2v_clock_io_t *pgc2v_io, 
	kgi_u8_t val, kgi_u16_t reg)
{
	PGC2v_DAC_OUT8(pgc2v_io, reg & 0xFF, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, reg >> 8, PGC2v_DAC_IndexHigh);
	PGC2v_DAC_OUT8(pgc2v_io, val, PGC2v_DAC_IndexData);
}

static inline kgi_u8_t PGC2v_EDAC_IN8(pgc2v_clock_io_t *pgc2v_io, kgi_u16_t reg)
{
	PGC2v_DAC_OUT8(pgc2v_io, reg & 0xFF, PGC2v_DAC_IndexLow);
	PGC2v_DAC_OUT8(pgc2v_io, reg >> 8, PGC2v_DAC_IndexData);
	return PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData);
}

kgi_error_t pgc2v_clock_init(pgc2v_clock_t *pgc2v, pgc2v_clock_io_t *pgc2v_io,
	const kgim_options_t *options)
{
	pgc2v->DClk3PreScale =
		PGC2v_EDAC_IN8(pgc2v_io, PGC2v_EDAC_DClk3PreScale);
	pgc2v->DClk3FeedbackScale =
		PGC2v_EDAC_IN8(pgc2v_io, PGC2v_EDAC_DClk3FeedbackScale);
	pgc2v->DClk3PostScale =
		PGC2v_EDAC_IN8(pgc2v_io, PGC2v_EDAC_DClk3PostScale);

	return KGI_EOK;
}

void pgc2v_clock_done(pgc2v_clock_t *pgc2v, pgc2v_clock_io_t *pgc2v_io,
	const kgim_options_t *options)
{
	PGC2v_EDAC_OUT8(pgc2v_io,
		pgc2v->DClk3PreScale, PGC2v_EDAC_DClk3PreScale);
	PGC2v_EDAC_OUT8(pgc2v_io,
		pgc2v->DClk3FeedbackScale, PGC2v_EDAC_DClk3FeedbackScale);
	PGC2v_EDAC_OUT8(pgc2v_io, 
		pgc2v->DClk3PostScale, PGC2v_EDAC_DClk3PostScale);
}

void pgc2v_clock_mode_enter(pgc2v_clock_t *pgc2v, pgc2v_clock_io_t *pgc2v_io,
	pgc2v_clock_mode_t *pgc2v_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t cnt;

	KRN_ASSERT(pgc2v_mode->pll.mul < 256);
	KRN_ASSERT(pgc2v_mode->pll.div < 256);
	KRN_ASSERT(pgc2v_mode->pll.p   < 5);

	/*	we use clock 3, as this one is 'not valid' at boot up.
	*/
	PGC2v_EDAC_OUT8(pgc2v_io, PGC2v_E200_DClkDisable, 0x200);

	PGC2v_EDAC_OUT8(pgc2v_io,
		pgc2v_mode->pll.mul, PGC2v_EDAC_DClk3FeedbackScale);
	PGC2v_EDAC_OUT8(pgc2v_io,
		pgc2v_mode->pll.div, PGC2v_EDAC_DClk3PreScale); 
	PGC2v_EDAC_OUT8(pgc2v_io,
		pgc2v_mode->pll.p, PGC2v_EDAC_DClk3PostScale);

	PGC2v_CLK_OUT8(pgc2v_io, 3, 0);

	PGC2v_EDAC_OUT8(pgc2v_io, PGC2v_E200_DClkEnable, 0x200);
	cnt = 10000;
	while (cnt-- && (PGC2v_DAC_IN8(pgc2v_io, PGC2v_DAC_IndexData) &
		PGC2v_E200_DClkLocked));

	KRN_ASSERT(cnt);
}
