/* ----------------------------------------------------------------------------
**	TI TVP3026 PLL meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: TVP3026-meta.h,v $
**	Revision 1.1.1.1  2000/04/26 14:12:18  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1  2000/04/26 14:12:18  seeger_s
**	- added TI TVP3026 driver
**	
*/
#ifndef _clock_TI_TVP3026_meta_h
#define	_clock_TI_TVP3026_meta_h

#include "clock/pll-meta.h"

typedef pll_clock_io_t tvp3026_clock_io_t;

#define	TVP3026_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define	TVP3026_DAC_IN8(ctx, reg)	PLL_DAC_IN8(ctx, reg)
#define	TVP3026_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define	TVP3026_CLK_IN8(ctx, reg)	PLL_CLK_IN8(ctx, reg)

typedef struct
{
	pll_clock_mode_t	pll;

} tvp3026_clock_mode_t;

typedef struct
{
	pll_clock_t	pll;

	kgi_u8_t	ClockSelect, PLLAddress, PClkM, PClkN, PClkP;

} tvp3026_clock_t;

KGIM_META_INIT_FN(tvp3026_clock)
KGIM_META_DONE_FN(tvp3026_clock)
KGIM_META_MODE_ENTER_FN(tvp3026_clock)

#endif	/* #ifndef _clock_TI_TVP3026_meta_h	*/
