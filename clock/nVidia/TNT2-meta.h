/* -----------------------------------------------------------------------------
**	nVidia TNT2 PLL meta language definition
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: TNT2-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:14  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _clock_nVidia_TNT2_meta_h
#define	_clock_nVidia_TNT2_meta_h

#include "clock/pll-meta.h"

typedef pll_clock_io_t tnt2_clock_io_t;

#define	TNT2_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define	TNT2_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define	TNT2_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define	TNT2_CLK_IN8(ctx, val, reg)	PLL_CLK_IN8(ctx, reg)

typedef struct
{
	pll_clock_mode_t	pll;

} tnt2_clock_mode_t;

typedef struct
{
	pll_clock_t	pll;

} tnt2_clock_t;

KGIM_META_INIT_FN(tnt2_clock)
KGIM_META_DONE_FN(tnt2_clock)
KGIM_META_MODE_ENTER_FN(tnt2_clock)

#endif	/* #ifndef _clock_nVidia_TNT2_meta_h */
