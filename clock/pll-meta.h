/* ---------------------------------------------------------------------------
**	PLL clock driver meta language definition
** ---------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**
**	$Log: pll-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:13  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:13  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#ifndef	_clock_pll_pll_meta_h
#define	_clock_pll_pll_meta_h

typedef kgim_chipset_io_t pll_clock_io_t;

#define	PLL_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	PLL_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)
#define	PLL_CLK_OUT8(ctx, val, reg)	KGIM_CLK_OUT8(ctx, val, reg)
#define	PLL_CLK_IN8(ctx, reg)		KGIM_CLK_IN8(ctx, reg)

typedef struct
{
	kgim_clock_mode_t	kgim;

	kgi_u_t	f;		/* produced frequency	*/
	kgi_u_t	fvco;		/* VCO frequency	*/
	kgi_s_t	p;		/* post-VCO divider	*/
	kgi_u_t	mul;		/* multiplier value	*/
	kgi_u_t	div;		/* divider value	*/

} pll_clock_mode_t;

#ifndef	PLL_CLOCK_T
#define	PLL_CLOCK_T
#endif

typedef struct
{
	kgim_clock_t	clock;

	kgi_u_t		fref;	/* reference frequency		*/
	kgi_ratio_t	a;	/* generator dependent factor	*/
	kgi_urange_t	fvco;	/* VCO frequency limits		*/
	kgi_srange_t	p;	/* post-VCO divider range	*/
	kgi_urange_t	mul;	/* multiplier value range	*/
	kgi_urange_t	div;	/* divider value range		*/

	PLL_CLOCK_T		/* driver dependent initial state */

} pll_clock_t;

KGIM_META_MODE_CHECK_FN(pll_clock)

#endif	/* #ifndef _clock_pll_pll_meta_h	*/
