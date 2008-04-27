/* ----------------------------------------------------------------------------
**	ATI MACH64 PLL meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Filip_Spacek
**
**	$Log: MACH64-meta.h,v $
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:35  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:17:14  phil
**	Autmatically generated ATI clock code
**	
**	Revision 1.2  2000/04/26 14:00:39  seeger_s
**	- wrong arguments to some I/O macros
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _clock_ATI_MACH64_meta_h
#define	_clock_ATI_MACH64_meta_h

#include "chipset/ATI/MACH64-meta.h"

typedef mach64_chipset_io_t mach64_clock_io_t;

typedef enum {
	MACH64_ODD_POST_DIVIDERS = 0x00000001,
	MACH64_DSP               = 0x00000002
}mach64_clock_flags_t;

typedef struct
{
	kgim_clock_mode_t	kgim;
	
	kgi_u_t f;                 /* produced frequency                */
	kgi_u_t fvco;              /* VCO frequency                     */
	kgi_s_t post_divider_val;  /* post-VCO divider numerical value  */
	kgi_s_t post_divider_num;  /* post-VCO divider register setting */
	kgi_u_t feedback_divider;  /* multiplier value                  */
	kgi_u32_t dsp_config, dsp_on_off;
	
} mach64_clock_mode_t;

typedef struct
{
	kgim_clock_t	clock;
	
	/* Capabilities of the clock generator */
	mach64_clock_flags_t flags;
	
	kgi_u_t fref;          /* reference frequency	    */
	kgi_urange_t fvco;     /* VCO frequency limits    */
	kgi_srange_t feedback; /* multiplier value range  */
	kgi_u8_t ref_div;      /* reference divider       */
	
	/* Initial state of the clock */
	struct {
		kgi_u8_t pll_ref_div, pll_gen_cntl, pll_vclk_cntl,
			vclk_post_div, vclk0_fb_div, pll_ext_cntl;
	} saved;
	
} mach64_clock_t;

KGIM_META_MODE_CHECK_FN(mach64_clock)
KGIM_META_INIT_FN(mach64_clock)
KGIM_META_DONE_FN(mach64_clock)
KGIM_META_MODE_ENTER_FN(mach64_clock)

#endif	/* #ifndef _clock_ATI_MACH64_meta_h	*/
