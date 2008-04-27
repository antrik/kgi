/* ---------------------------------------------------------------------------
**	Fixed clock driver meta language definition
** ---------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**	Copyright (C)	2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: fixed-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:12  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_clock_fixed_meta_h
#define	_clock_fixed_meta_h

typedef kgim_chipset_io_t fixed_clock_io_t;

#define	FIXED_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define	FIXED_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)
#define	FIXED_CLK_OUT8(ctx, val, reg)	KGIM_CLK_OUT8(ctx, val, reg)
#define	FIXED_CLK_IN8(ctx, reg)		KGIM_CLK_IN8(ctx, reg)

typedef struct
{
	kgim_clock_mode_t	kgim;
	kgi_u_t			reg0;
	
} fixed_clock_mode_t;

typedef struct
{
	kgim_clock_t	clock;
	
	kgi_u_t		clk_index;
	kgi_u_t		reg0[__KGIM_MAX_NR_FIXED_CLOCKS];

} fixed_clock_t;

KGIM_META_INIT_FN(fixed_clock)
KGIM_META_DONE_FN(fixed_clock)
KGIM_META_MODE_CHECK_FN(fixed_clock)
KGIM_META_MODE_ENTER_FN(fixed_clock)

#endif	/* #ifndef _clock_fixed_meta_h	*/
