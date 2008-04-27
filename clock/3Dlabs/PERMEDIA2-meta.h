/* -----------------------------------------------------------------------------
**	PERMEDIA2 PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: PERMEDIA2-meta.h,v $
**	Revision 1.1.1.1  2000/04/26 13:59:35  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/04/26 13:59:35  seeger_s
**	- wrong arguments in I/O macros
**	
**	Revision 1.1.1.1  2000/04/18 08:51:13  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _clock_3Dlabs_PERMEDIA2_meta_h
#define	_clock_3Dlabs_PERMEDIA2_meta_h

#include "clock/pll-meta.h"

typedef pll_clock_io_t pgc_clock_io_t;

#define	PGC_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define	PGC_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define	PGC_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define	PGC_CLK_IN8(ctx, reg)		PLL_CLK_IN8(ctx, reg)

typedef struct
{
	pll_clock_mode_t	pll;

} pgc_clock_mode_t;

typedef struct
{
	pll_clock_t	pll;

	kgi_u8_t	PixelClockC1;
	kgi_u8_t	PixelClockC2;
	kgi_u8_t	PixelClockC3;

} pgc_clock_t;

KGIM_META_INIT_FN(pgc_clock)
KGIM_META_DONE_FN(pgc_clock)
KGIM_META_MODE_ENTER_FN(pgc_clock)

#endif	/* #ifndef _clock_3Dlabs_PERMEDIA2_meta_h	*/
