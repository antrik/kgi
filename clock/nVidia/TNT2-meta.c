/* -----------------------------------------------------------------------------
**	nVidia TNT2 PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright	1997-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: TNT2-meta.c,v $
**	Revision 1.1.1.1  2000/04/18 08:51:14  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Jon_Taylor
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#include "ramdac/nVidia/TNT2.h"
#include "clock/nVidia/TNT2-meta.h"

kgi_error_t tnt2_clock_init(tnt2_clock_t *pgc, tnt2_clock_io_t *tnt2_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "tnt2_clock_init()");

#warning save initial state here! -- seeger_s

	return KGI_EOK;
}

extern void tnt2_clock_done(tnt2_clock_t *pgc, tnt2_clock_io_t *tnt2_io,
	const kgim_options_t *options)
{
	KRN_DEBUG(2, "tnt2_clock_done()");

#warning restore initial state here! -- seeger_s
}

void tnt2_clock_mode_enter(tnt2_clock_t *pgc, tnt2_clock_io_t *tnt2_io,
	tnt2_clock_mode_t *tnt2_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "tnt2_clock_mode_enter()");
	
	KRN_ASSERT(tnt2_mode->pll.mul < 256);
	KRN_ASSERT(tnt2_mode->pll.div < 256);
	KRN_ASSERT(tnt2_mode->pll.p   < 8);
	
	KRN_DEBUG(2, "mul = %d, div = %d, p = %d", tnt2_mode->pll.mul,
		  tnt2_mode->pll.div, tnt2_mode->pll.p);

#warning driver does not set clock yet!
#if 0
	/* We use clock C, as this one is 'not valid' at boot up */
	TNT2_EDAC_OUT8(tnt2_io, (tnt2_mode->pll.mul) & 0xFF, TNT2_EDAC_PClkC_M);
	TNT2_EDAC_OUT8(tnt2_io, (tnt2_mode->pll.div) & 0xFF, TNT2_EDAC_PClkC_N); 
	TNT2_EDAC_OUT8(tnt2_io, ((tnt2_mode->pll.p) & 0x07) | TNT2_EDAC_PClkP_Enable, TNT2_EDAC_PClkC_P);

	TNT2_CLK_OUT8(tnt2_io, 3, 0);

	TNT2_DAC_OUT8(tnt2_io, 0x29, TNT2_DAC_EXT_ADDR);
	cnt = 10000;
	while (cnt-- && (TNT2_DAC_IN8(tnt2_io, TNT2_DAC_EXT_DATA) & TNT2_EDAC29_PLLlocked));

	KRN_ASSERT(cnt);
#endif
}
