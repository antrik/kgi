/* ----------------------------------------------------------------------------
**	ATI Rage 128 PLL meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Paul_Redmond
**
**	$Log: R128-meta.h,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#ifndef _clock_ATI_R128_meta_h
#define	_clock_ATI_R128_meta_h

#include "chipset/ATI/R128-meta.h"

typedef r128_chipset_io_t r128_clock_io_t;

typedef struct
{
	kgim_clock_mode_t	kgim;
	
	struct {
		kgi_u32_t ref_divider;
		kgi_u32_t feedback_divider;
		kgi_u32_t post_divider;
		kgi_u_t dclk;
		kgi_u32_t dda_config;
		kgi_u32_t dda_on_off;
	} r128;
	
} r128_clock_mode_t;

typedef enum
{
	R128_MEM_64BIT_SGRAM,
	R128_MEM_128BIT_SGRAM
} r128_clock_mem_t;

typedef struct
{
	kgim_clock_t	clock;
	
	kgi_u_t pclk_ref_freq;
	kgi_u_t pclk_min_freq;
	kgi_u_t pclk_max_freq;
	kgi_u_t pclk_ref_divider;
	kgi_u_t xclk;
	
	r128_clock_mem_t mem_type;
	
	struct { kgi_u32_t
	
		clock_cntl_index,
		ppll_ref_div,
		pll_div_3,
		htotal_cntl,
		dda_config,
		dda_on_off;
	} saved;
	
} r128_clock_t;

KGIM_META_MODE_CHECK_FN(r128_clock)
KGIM_META_INIT_FN(r128_clock)
KGIM_META_DONE_FN(r128_clock)
KGIM_META_MODE_ENTER_FN(r128_clock)

#endif	/* #ifndef _clock_ATI_R128_meta_h	*/
