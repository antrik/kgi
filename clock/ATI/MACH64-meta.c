/* ----------------------------------------------------------------------------
**	ATI MACH64 PLL implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64-meta.c,v $
**	Revision 1.2  2003/07/26 18:46:11  cegger
**	merge improvements from the FreeBSD guys:
**	- improves OS independency
**	- add support for new MACH64 chip variants (GX and LM)
**	- bug fixes
**	
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
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Filip_Spacek
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/MACH64.h"
#include "clock/ATI/MACH64-meta.h"

#undef MAX
#undef MIN
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

kgi_error_t mach64_clock_init(mach64_clock_t *mach64, 
			      mach64_clock_io_t *mach64_io, 
			      const kgim_options_t *options)
{
	/*
	 * Save initial state of registers
	 */
	mach64->saved.pll_ref_div =
		MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_REF_DIV);
	mach64->saved.pll_gen_cntl =
		MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_GEN_CNTL);
	mach64->saved.pll_vclk_cntl =
		MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_VCLK_CNTL);
	mach64->saved.vclk_post_div =
		MACH64_CLOCK_IN8(mach64_io, MACH64_VCLK_POST_DIV);
	mach64->saved.vclk0_fb_div =
		MACH64_CLOCK_IN8(mach64_io, MACH64_VCLK0_FB_DIV);
	mach64->saved.pll_ext_cntl =
		MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_EXT_CNTL);

	mach64->ref_div = mach64->saved.pll_ref_div;
	
	return KGI_EOK;
}

void mach64_clock_done(mach64_clock_t *mach64,
		       mach64_clock_io_t *mach64_io, 
		       const kgim_options_t *options)
{
	/* This is removed for the moment. The problem is, that if the module
	 * is removed while the chipset is in VGA mode it will totally screw
	 * up the card.
	 */
	MACH64_CLOCK_OUT8(mach64_io,
			  mach64->saved.pll_ref_div, MACH64_PLL_REF_DIV);
	MACH64_CLOCK_OUT8(mach64_io,
			  mach64->saved.pll_gen_cntl, MACH64_PLL_GEN_CNTL);
	MACH64_CLOCK_OUT8(mach64_io,
			  mach64->saved.pll_vclk_cntl, MACH64_PLL_VCLK_CNTL);
	MACH64_CLOCK_OUT8(mach64_io,
			  mach64->saved.vclk_post_div,MACH64_VCLK_POST_DIV);
	MACH64_CLOCK_OUT8(mach64_io,
			  mach64->saved.vclk0_fb_div, MACH64_VCLK0_FB_DIV);
	MACH64_CLOCK_OUT8(mach64_io,
			  mach64->saved.pll_ext_cntl, MACH64_PLL_EXT_CNTL);
}

/*
 * DSP registers calculations for MACH64GT/B and above. It might not seem
 * to have much to do with clock but there are certain vital bits that
 * are available only in the clock code. To calculate the xclkc_per_qw
 * we absolutely have to have vlkc_fb_div and vclk_post_div but during
 * mode checking these values are not in registers yet, only in the clock
 * mode structure. So we have to put this code here to get them. As a
 * result, the clock code needs to access most of the chipset and thus
 * the mach64_clock_io is typedef'd to mach64_chipset_io to gain access
 * to all necessary registers. Note that it actually is not that big of a
 * deal that the code is here as all chipsets implementing DSP also have
 * an internal clock.
 */
static kgi_error_t mach64_calculate_dsp(mach64_clock_t *mach64,
					mach64_clock_io_t *mach64_io,
					mach64_clock_mode_t *mach64_mode,
					kgi_dot_port_mode_t *dpm)
{
	kgi_u_t precision, page_fault_clocks, rcc;
	kgi_u_t dsp_on, dsp_off, dsp_loop_latency;
	kgi_u_t bpd, xclks_per_qw;
	kgi_u_t mfb_times, mclk_fb_div, ref_div, xclk_post;
	kgi_u_t xclk_post_div[] = { 1, 2, 4, 8, 3, 1, 1, 1 };

	bpd = kgim_attr_bits(dpm->bpda);
	KRN_DEBUG(2, "bpd: %d", bpd);
  
	/* FIXME: add defines for clock register components */
	mfb_times =
		(MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_EXT_CNTL) & 8) ? 4 : 2;
	mclk_fb_div = MACH64_CLOCK_IN8(mach64_io, MACH64_MCLK_FB_DIV);
	ref_div = MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_REF_DIV);
	xclk_post =
	  xclk_post_div[MACH64_CLOCK_IN8(mach64_io, MACH64_PLL_EXT_CNTL)&7];

	xclks_per_qw =
	((mclk_fb_div * mfb_times * 64 * mach64_mode->post_divider_val)<< 11) /
		(mach64_mode->feedback_divider * xclk_post * bpd * 2);
  
	KRN_DEBUG(2, "XCLKS_PER_QW %d", xclks_per_qw);

	/* First of all we need to calculate the highest possible precision.
	 *
	 * DSP_ON and DSP_OFF are 11 bits wide, while XCLKS_PER_QW is 14 bits
	 * wide. They are represented as fixed point numbers with DSP_ON and
	 * DSP_OFF represented as (5+p).(6-p) and XCLKS_PER_QW is (3+p).(11-p)
	 * where p is some precision (0-7)
	 *
	 * Since we know the value of XCLKS_PER_QW from above and DSP_ON and
	 * DSP_OFF can be at most XCLKS_PER_QW * d, where d is the depth of
	 * the fifo (32, or is it 24?), we can calculate the most suitable
	 * value for p.
	 */
	
	/* if even the worst precision isn't capable of holding the integer
	   portion of XCLKS_PER_QW then that's bad news */
	if(xclks_per_qw >> (14 + 7)){
		KRN_DEBUG(0, "XCLKS_PER_QW %d (%d integer part) too high",
			  xclks_per_qw, xclks_per_qw >> 11);
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}

	/* Now we need to possibly worsen the precision depending on the
	 * largest possible values for DSP_ON and DSP_OFF
	 * We multiply xclks_per_qw (which is in 21.11 format) by the depth of
	 * the FIFO to give us the largest possible value in 21.11 format. Then
	 * we check what kind of precision we need considering that the
	 * format is (5+precision).(6-precision)
	 */
#warning Check for 32 entry wide fifos
	precision = 0;
	while(xclks_per_qw*24 >> (5 + precision + 11))
		++precision;
	
	KRN_DEBUG(2, "by DSP_ON/OFF, precision is %d", precision);
	
	//fifo_size = MIN((1 << (5 + precision + 11))/xclks_per_qw, 32);
	dsp_off = xclks_per_qw*(24-1);
	
	KRN_DEBUG(2, "dsp_off %d", dsp_off);
	
	page_fault_clocks =
		(MACH64_CTRL_IN32(mach64_io, MACH64_MEM_CNTL) &
		 MACH64_2C_MEM_TRPMask >> MACH64_2C_MEM_TRPShift) + 2 +
		(MACH64_CTRL_IN32(mach64_io, MACH64_MEM_CNTL) &
		 MACH64_2C_MEM_TRCDMask >> MACH64_2C_MEM_TRCDShift) + 2 +
		((MACH64_CTRL_IN32(mach64_io, MACH64_MEM_CNTL) &
		  MACH64_2C_MEM_TCRD) ? 1 : 0);

	KRN_DEBUG(2, "Page fault clocks %d", page_fault_clocks);

	/* FIXME: this can be lower depending on RAM type */
	dsp_loop_latency = 11;
  
	rcc = MAX((MACH64_CTRL_IN32(mach64_io, MACH64_MEM_CNTL) &
		   MACH64_2C_MEM_TRPMask >> MACH64_2C_MEM_TRPShift) + 2 +
		  (MACH64_CTRL_IN32(mach64_io, MACH64_MEM_CNTL) &
		   MACH64_2C_MEM_TRASMask >> MACH64_2C_MEM_TRASShift) + 3,
		  page_fault_clocks + 4);

	KRN_DEBUG(2, "rcc %d", rcc);

	/* FIXME: once again, can improve depending on RAM type */
	dsp_on = 2*rcc + page_fault_clocks + 4;
	
	KRN_DEBUG(2, "dsp_on %d", dsp_on);
	
	mach64_mode->dsp_on_off =
		(((dsp_off >> (5 + precision)) << MACH64_09_DSP_OFFShift) &
		 MACH64_09_DSP_OFFMask) |
		(((dsp_on << (6 - precision)) << MACH64_09_DSP_ONShift) &
		 MACH64_09_DSP_ONMask);

	mach64_mode->dsp_config =
		(((xclks_per_qw >> precision)
		  << MACH64_08_DSP_XCLKS_PER_QWShift) &
		 MACH64_08_DSP_XCLKS_PER_QWMask) |
		((dsp_loop_latency << MACH64_08_DSP_LOOP_LATENCYShift) &
		 MACH64_08_DSP_LOOP_LATENCYMask) |
		((precision << MACH64_08_DSP_PRECISIONShift) &
		 MACH64_08_DSP_PRECISIONMask);

	KRN_DEBUG(2, "dsp_on_off 0x%08x, dsp_config 0x%08x",
		  mach64_mode->dsp_on_off, mach64_mode->dsp_config);
	
	return KGI_EOK;
}

/*	find the best paramter set to produce a certain frequency f
*/
static kgi_error_t mach64_clock_best_mode(mach64_clock_t *mach64,
        mach64_clock_mode_t *mach64_mode, kgi_u_t f, kgi_timing_command_t cmd)
{
	/*
	 * Table of post dividers possible on mach64 (integrated clock)
	 * (Does the /5 divider exist?)
	 */
	struct post_divider_s {
		kgi_u8_t val, num;
	} post_divider_tbl[] = { { 12, 0x7}, { 8, 0x3}, { 6, 0x6}, { 5, 0x5},
				 {  4, 0x2}, { 3, 0x4}, { 2, 0x1}, { 1, 0x0} };
	/*
	 * Maximum address of post divider (CT has 4 post dividers, GTB has 8)
	 */
	int max_post_divider =
		(mach64->clock.flags & MACH64_ODD_POST_DIVIDERS) ?8:4;
	unsigned int i, post_divider = 0xff, feedback_divider, remainder;

	KRN_DEBUG(2, "Setting f: %d", f);
	KRN_DEBUG(2, "Reference divider %d", mach64->ref_div);

	/*
	 * Walk the list of possible post divider, and find the
	 * highest one which will put us into valid range. This is
	 * done by calculating the feedback divider (N) and checking
	 * it against the possible range.
	 *
	 * To avoid possible overflow we work with 1/256 of the actual
	 * frequency (Highest frequency is 240Mhz, reference divider
	 * is max 255 and post divider max 12)
	 *
	 * Note that F = (2*R*N)/(N*P)
	 */
	for(i = 0; i < sizeof(post_divider_tbl)/sizeof(struct post_divider_s); ++i){
		/* N = (F*M*P)/256 */
		feedback_divider =
			(f>>8) * mach64->ref_div * post_divider_tbl[i].val;
		remainder = feedback_divider % (mach64->fref>>7);
		
		/* N = ((F*M*P)/256)/(R/128) = (F*M*P)/(2*R) */
		feedback_divider /= mach64->fref>>7;
		
		/* Add .5 */
		if(feedback_divider < mach64->feedback.max &&
		   remainder >= (mach64->fref>>8))
			++feedback_divider;
    
		if(feedback_divider <= mach64->feedback.max &&
		   feedback_divider >= mach64->feedback.min &&
		   post_divider_tbl[i].num < max_post_divider){
			post_divider = i;
			break;
		}
	}
	/* No possible post divider, bail out. */
	if (post_divider == 0xff){
		KRN_DEBUG(2, "Could not find a proper post divider");
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}
	
	/* Set up the mode structure */
	mach64_mode->post_divider_val = post_divider_tbl[post_divider].val;
	mach64_mode->post_divider_num = post_divider_tbl[post_divider].num;
	mach64_mode->feedback_divider = feedback_divider;
	mach64_mode->f = (((mach64->fref*2)/mach64->ref_div)*
			  mach64_mode->feedback_divider) /
		         post_divider_tbl[post_divider].val;
  
	KRN_DEBUG(2, "Clock: f: %d, ref: %d, fb: %d, p: %d (0x%.2x)", f,
		  mach64->ref_div, feedback_divider,
		  post_divider_tbl[post_divider].val,
		  post_divider_tbl[post_divider].num);
  
	return KGI_EOK;
}
  

kgi_error_t mach64_clock_mode_check(mach64_clock_t *mach64,
        mach64_clock_io_t *mach64_io, mach64_clock_mode_t *mach64_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	kgi_s_t newfreq = (cmd == KGI_TC_PROPOSE) 
		? 0 : dpm->dclk * dpm->rclk.mul / dpm->rclk.div;
	const kgi_s_t min_freq = mach64->clock.dclk.range[0].min;
	const kgi_s_t max_freq = mach64->clock.dclk.range[0].max;
	
	KRN_ASSERT(images == 1);
	
	switch (cmd) {
		
	case KGI_TC_PROPOSE:
		KRN_DEBUG(2, "KGI_TC_PROPOSE:");
		return KGI_EOK;
		
	case KGI_TC_LOWER:
		KRN_DEBUG(2, "KGI_TC_LOWER:");
		if (newfreq < min_freq) {
			
			KRN_DEBUG(1, "DCLK = %i Hz, CLK = %i Hz is too low",
				  dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		if (newfreq > max_freq) {
			
			newfreq = max_freq;
		}
		
		if (mach64_clock_best_mode(mach64, mach64_mode, newfreq, cmd)){
			
			KRN_DEBUG(1, "lower failed: DCLK = %i Hz, CLK = %i Hz",
				  dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		dpm->dclk = mach64_mode->f * dpm->rclk.div / dpm->rclk.mul;
		return KGI_EOK;
		
	case KGI_TC_RAISE:
		KRN_DEBUG(2, "KGI_TC_RAISE:");
		if (max_freq < newfreq) {
			
			KRN_DEBUG(1, "DCLK = %i Hz, CLK = %i Hz is too high",
				  dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		if (newfreq < min_freq) {
			
			newfreq = min_freq;
		}
		
		if (mach64_clock_best_mode(mach64, mach64_mode, newfreq, cmd)){
			
			KRN_DEBUG(1, "raise failed: DCLK = %i Hz, CLK = %i Hz",
				  dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		dpm->dclk = mach64_mode->f * dpm->rclk.div / dpm->rclk.mul;
		return KGI_EOK;
		
	case KGI_TC_CHECK:
		KRN_DEBUG(2, "KGI_TC_CHECK:");
		
		if (mach64_clock_best_mode(mach64, mach64_mode,
					   newfreq, KGI_TC_LOWER)) {
			
			KRN_DEBUG(1, "check failed: DCLK = %i Hz, CLK = %i Hz",
				  dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		/* If the card has DSP registers, calculate their values */
		if(mach64->flags & MACH64_DSP)
			mach64_calculate_dsp(mach64,mach64_io,mach64_mode,dpm);
    
		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}
}

void mach64_clock_mode_enter(mach64_clock_t *mach64,
	mach64_clock_io_t *mach64_io,
	mach64_clock_mode_t *mach64_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
    	/* Select clock 0. Note the difference CLK vs CLOCK */
	MACH64_CLK_OUT8(mach64_io, 0, 0);
	
	/* Debuggin' stuff */
	MACH64_CLOCK_IN8(mach64_io, 0);
	MACH64_CLOCK_IN8(mach64_io, 1);
	MACH64_CLOCK_IN8(mach64_io, 2);
	MACH64_CLOCK_IN8(mach64_io, 3);
	MACH64_CLOCK_IN8(mach64_io, 4);
	MACH64_CLOCK_IN8(mach64_io, 5);
	MACH64_CLOCK_IN8(mach64_io, 6);
	MACH64_CLOCK_IN8(mach64_io, 7);
	MACH64_CLOCK_IN8(mach64_io, 8);
	MACH64_CLOCK_IN8(mach64_io, 9);
	MACH64_CLOCK_IN8(mach64_io, 10);
	MACH64_CLOCK_IN8(mach64_io, 11);
	MACH64_CLOCK_IN8(mach64_io, 12);
	MACH64_CLOCK_IN8(mach64_io, 13);
	MACH64_CLOCK_IN8(mach64_io, 14);

	/* Program the VCLK */
	MACH64_CLOCK_OUT8(mach64_io, mach64_mode->post_divider_num & 0x03,
			  MACH64_VCLK_POST_DIV);
	MACH64_CLOCK_OUT8(mach64_io, mach64_mode->feedback_divider,
			  MACH64_VCLK0_FB_DIV);

	/* Special post dividers for GT/B and above */
	if(mach64->clock.flags & MACH64_ODD_POST_DIVIDERS){
		int ext_cntl = MACH64_CLOCK_IN8(mach64_io,MACH64_PLL_EXT_CNTL);
		ext_cntl |= (mach64_mode->post_divider_num & ~0x03) << 2;
		MACH64_CLOCK_OUT8(mach64_io, ext_cntl, MACH64_PLL_EXT_CNTL);
	}

	/* Set the VCLK source to PLL_VCLK */
	MACH64_CLOCK_OUT8(mach64_io, 3, MACH64_PLL_VCLK_CNTL);

	/* Set up DSP. See note above about why is this here */
	if(mach64->flags & MACH64_DSP){
		MACH64_CTRL_OUT32(mach64_io, mach64_mode->dsp_on_off,
				  MACH64_DSP_ON_OFF);
		MACH64_CTRL_OUT32(mach64_io, mach64_mode->dsp_config,
				  MACH64_DSP_CONFIG);
	}
}
