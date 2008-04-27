/* ----------------------------------------------------------------------------
**	ATI Rage 128 PLL implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128-meta.c,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Paul_Redmond
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/R128.h"
#include "clock/ATI/R128.h"
#include "clock/ATI/R128-meta.h"

static inline kgi_u32_t r128_round_div(kgi_u32_t n, kgi_u32_t d)
{
	return (n + (d / 2)) / d;
}

static inline kgi_error_t r128_clock_calc_dda_values(r128_clock_t *r128,
	r128_clock_io_t *r128_io, r128_clock_mode_t *r128_mode, 
	kgi_u_t dclk, kgi_u_t bpp)
{
	const kgi_u32_t fifo_width = 128;
	const kgi_u32_t fifo_depth = 32;
	kgi_u32_t mem_cfg_type, xclk, xpt, xpx, prec, Ron, Roff;
	struct {

		kgi_u32_t ml, mb, trcd, trp, twr, cl, 
			 tr2w, loop_latency, dsp_on, rloop;
			 
	} *mem_info, memory_table[] = {
		{ 4, 4, 3, 3, 1, 3, 1, 16, 30, 16 },
		{ 4, 8, 3, 3, 1, 3, 1, 17, 46, 17 },
		{ 4, 8, 3, 3, 1, 3, 1, 17, 46, 16 },
		{ 4, 4, 1, 2, 1, 2, 1, 16, 24, 16 },
		{ 4, 4, 3, 3, 2, 3, 1, 16, 31, 16 }
	};

	KRN_DEBUG(1, "calculating dda registers");
		
	KRN_DEBUG(1, "determining memory type");
	mem_cfg_type = R128_CTRL_IN32(r128_io, R128_MEM_CNTL);
	switch (mem_cfg_type & R128_140_MEM_CFG_TYPEMask) {
	
	case 0: 
		switch (r128->mem_type) {
		
		case R128_MEM_128BIT_SGRAM:
			mem_info = &memory_table[0]; 
			break;
		case R128_MEM_64BIT_SGRAM:
			mem_info = &memory_table[1]; 
			break;
		default:
			mem_info = &memory_table[2];
		}
		break;
	case 1:
		mem_info = &memory_table[3]; break;
	case 2: 
		mem_info = &memory_table[4]; break;
	default:
		mem_info = &memory_table[1];
	}
	
	bpp = (bpp + 0x7) & ~0x7;
	
	/* dclk is in Hz.  Do calcs in Khz */
	KRN_DEBUG(1, "dclk = %d Hz xclk = %d Hz", dclk, r128->xclk);
	dclk /= 1000;
	xclk = r128->xclk / 1000;
	KRN_DEBUG(1, "[%d * %d] / [%d * %d]", xclk, fifo_width, dclk, bpp);
	
	xpt = r128_round_div(xclk * fifo_width, dclk * bpp);
	KRN_DEBUG(1, "xpt == %d", xpt);
	
	if (xpt == 0)
		prec = 2;
	else {
		kgi_u32_t tmp = xpt;
		prec = 1;
		while (tmp) {
			tmp >>= 1;
			prec++;
		}
	}
	KRN_DEBUG(1, "precision == %d", prec);
	
	KRN_DEBUG(1, "[(%d * %d) << (11 - %d)] / [%d * %d]",
		xclk, fifo_width, prec, dclk, bpp);
	xpx = r128_round_div((xclk * fifo_width) << (11 - prec),
		dclk * bpp);
	KRN_DEBUG(1, "xpx == %d", xpx);
	
	Ron = 	4 * mem_info->mb +
		3 * (mem_info->trcd - 2 < 0 ? 0 : mem_info->trcd - 2) +
		2 * mem_info->trp +
		mem_info->twr +
		mem_info->cl +
		mem_info->tr2w +
		xpt;
	
	Ron <<= (11 - prec);
	Roff = xpx * (fifo_depth - 4);
	
	if ((Ron + mem_info->rloop) >= Roff) {
	
		KRN_DEBUG(1, "on + rloop >= off (%d + %d) >= %d",
			Ron, mem_info->rloop, Roff);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	r128_mode->r128.dda_config = 
		(prec << R128_2E0_DDA_PRECISIONShift) | 
		(mem_info->rloop << R128_2E0_DDA_LOOP_LATENCYShift) |
		(xpx << R128_2E0_DDA_XCLKS_PER_XFERShift);
		
	r128_mode->r128.dda_on_off =
		(Ron << R128_2E4_DDA_ONShift) |
		(Roff << R128_2E4_DDA_OFFShift);
	
	return -KGI_EOK;
}

static kgi_error_t r128_clock_best_mode(r128_clock_t *r128,
	r128_clock_mode_t *r128_mode, kgi_u_t freq)
{
	static const kgi_u_t post_div_table[] = { 1, 2, 3, 4, 6, 8, 12 };
	static const kgi_u32_t post_div_bits[] = 
		{ 0x0, 0x1, 0x4, 0x2, 0x6, 0x3, 0x7 };
	static const kgi_u_t n = sizeof(post_div_table)/sizeof(kgi_u_t);
	
	kgi_u32_t temp, i;
	kgi_u_t pclk_ref_freq, pclk_min_freq, pclk_max_freq;
	
	KRN_DEBUG(1, "calculating dividers");

#define SCALE (1 KHZ)
	freq     = freq / SCALE;
	pclk_ref_freq = r128->pclk_ref_freq / SCALE;
	pclk_max_freq = r128->pclk_max_freq / SCALE;
	pclk_min_freq = r128->pclk_min_freq / SCALE;

	if (freq > pclk_max_freq)
		freq = pclk_max_freq;

	if (freq * post_div_table[n-1] < pclk_min_freq) {
	
		freq = pclk_min_freq / post_div_table[n-1];
	}
	KRN_DEBUG(1, "freq=%d", freq);

	for (i = 0; i < n; i++) {

		temp = post_div_table[i] * freq;
		if ((temp <= pclk_max_freq) && (temp >= pclk_min_freq))
			break;
	}
	KRN_DEBUG(1, "temp=%d", temp);
	
	/* reference divider */
	r128_mode->r128.ref_divider = r128->pclk_ref_divider;
	KRN_DEBUG(1, "ref_div=%d", r128_mode->r128.ref_divider);

	/* feedback divider */
	r128_mode->r128.feedback_divider = 
		r128_round_div(r128->pclk_ref_divider * temp, pclk_ref_freq);
	KRN_DEBUG(1, "feedback_div=%d", r128_mode->r128.feedback_divider);

	/* post divider */
	r128_mode->r128.post_divider = post_div_bits[i];
	KRN_DEBUG(1, "post_div=%d", post_div_table[i]);

	/* dot clock */
	r128_mode->r128.dclk = 
		(pclk_ref_freq * r128_mode->r128.feedback_divider) /
		(r128_mode->r128.ref_divider * post_div_table[i]) * SCALE;

	KRN_DEBUG(1,"dclk=%d", r128_mode->r128.dclk);

	return KGI_EOK;
#undef SCALE
}

static inline void r128_pll_wait_until_updated(r128_clock_io_t *r128_io)
{
	while (R128_PLL_IN32(r128_io, R128_PPLL_REF_DIV) &
		R128_03_PPLL_ATOMIC_UPDATE_R) 
	{
		/* busy wait */
	}
}

static inline void r128_pll_update(r128_clock_io_t *r128_io)
{
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io, R128_PLL_IN32(r128_io, R128_PPLL_REF_DIV) | 
		R128_03_PPLL_ATOMIC_UPDATE_W, R128_PPLL_REF_DIV);
}

static inline void r128_clock_save_mode(r128_clock_t *r128, 
	r128_clock_io_t *r128_io)
{
	r128->saved.clock_cntl_index = R128_CTRL_IN32(r128_io, R128_CLOCK_CNTL_INDEX);
	r128->saved.ppll_ref_div = R128_PLL_IN32(r128_io, R128_PPLL_REF_DIV);
	r128->saved.pll_div_3 = R128_PLL_IN32(r128_io, R128_PLL_DIV_3);
	r128->saved.htotal_cntl = R128_PLL_IN32(r128_io, R128_HTOTAL_CNTL);
	r128->saved.dda_config = R128_PLL_IN32(r128_io, R128_DDA_CONFIG);
	r128->saved.dda_on_off = R128_PLL_IN32(r128_io, R128_DDA_ON_OFF);
}

static inline void r128_clock_restore_mode(r128_clock_t *r128,
	r128_clock_io_t *r128_io)
{
	R128_CTRL_OUT32(r128_io, r128->saved.clock_cntl_index, 
		R128_CLOCK_CNTL_INDEX);
		
	R128_PLL_OUT32(r128_io,
		R128_PLL_IN32(r128_io, R128_PPLL_CNTL) |
		R128_02_PPLL_RESET | R128_02_PPLL_ATOMIC_UPDATE_EN |
		R128_02_PPLL_VGA_ATOMIC_UPDATE_EN, 
		R128_PPLL_CNTL);
	
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io, r128->saved.ppll_ref_div, R128_PPLL_REF_DIV);
	r128_pll_update(r128_io);
	
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io, r128->saved.htotal_cntl, R128_HTOTAL_CNTL);
	r128_pll_update(r128_io);
	
	/* clear reset again */
	R128_PLL_OUT32(r128_io, 
		R128_PLL_IN32(r128_io, R128_PPLL_CNTL) & ~R128_02_PPLL_RESET, 
		R128_PPLL_CNTL);
		
	R128_CTRL_OUT32(r128_io, r128->saved.dda_config, R128_DDA_CONFIG);
	R128_CTRL_OUT32(r128_io, r128->saved.dda_on_off, R128_DDA_ON_OFF);

	/* disable atomic updates */
	KRN_DEBUG(1, "disabling atomic updates");
	R128_PLL_OUT32(r128_io,
		R128_PLL_IN32(r128_io, R128_PPLL_CNTL) &
		~(R128_02_PPLL_ATOMIC_UPDATE_EN | 
		R128_02_PPLL_VGA_ATOMIC_UPDATE_EN),
		R128_PPLL_CNTL);
}

kgi_error_t r128_clock_init(r128_clock_t *r128, 
			      r128_clock_io_t *r128_io, 
			      const kgim_options_t *options)
{
	r128_clock_save_mode(r128, r128_io);
	
	return KGI_EOK;
}

void r128_clock_done(r128_clock_t *r128,
		       r128_clock_io_t *r128_io, 
		       const kgim_options_t *options)
{
	r128_clock_restore_mode(r128, r128_io);
}

kgi_error_t r128_clock_mode_check(r128_clock_t *r128,
        r128_clock_io_t *r128_io, r128_clock_mode_t *r128_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	kgi_s_t freq = dpm->dclk;
	kgi_u_t bpf, bpc, bpp, width;
	kgi_u_t bpd;

	
	const kgi_s_t min_freq = r128->clock.dclk.range[0].min;
	const kgi_s_t max_freq = r128->clock.dclk.range[0].max;
	
	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CLOCK, NOSUP);
	}

	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO)
		? (bpc + bpf*img[0].frames*2)
		: (bpc + bpf*img[0].frames);

	switch (cmd) {

	case KGI_TC_PROPOSE:
	
		KRN_DEBUG(1, "KGI_TC_PROPOSE");
	
		return KGI_EOK;
		
	case KGI_TC_LOWER:
	
		KRN_DEBUG(1, "KGI_TC_LOWER");
	
		if (freq < min_freq) {
			
			KRN_DEBUG(1, "DCLK = %i Hz, CLK = %i Hz is too low",
				  dpm->dclk, freq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		if (freq > max_freq) {
			
			freq = max_freq;
		}
		
		if (r128_clock_best_mode(r128, r128_mode, freq)){
			
			KRN_DEBUG(1, "lower failed: DCLK = %i Hz, CLK = %i Hz",
				  dpm->dclk, freq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		dpm->dclk = r128_mode->r128.dclk;
		return KGI_EOK;
		
	case KGI_TC_RAISE:
	
		KRN_DEBUG(1, "KGI_TC_RAISE");
	
		if (max_freq < freq) {
			
			KRN_DEBUG(1, "DCLK = %i Hz, CLK = %i Hz is too high",
				  dpm->dclk, freq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		if (freq < min_freq) {
			
			freq = min_freq;
		}
		
		if (r128_clock_best_mode(r128, r128_mode, freq)){
			
			KRN_DEBUG(1, "raise failed: DCLK = %i Hz, CLK = %i Hz",
				  dpm->dclk, freq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		
		dpm->dclk = r128_mode->r128.dclk;
		return KGI_EOK;
		
	case KGI_TC_CHECK:
	
		KRN_DEBUG(1, "KGI_TC_CHECK");
		
		if (r128_clock_best_mode(r128, r128_mode, freq)) {
			
			KRN_DEBUG(1, "check failed: DCLK = %i Hz, CLK = %i Hz",
				  dpm->dclk, freq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
	
		if (r128_clock_calc_dda_values(r128, r128_io, r128_mode, 
			dpm->dclk, bpp) != KGI_EOK) {
		
			return -KGI_ERRNO(CHIPSET, INVAL);	
		}
		
		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}
}

void r128_clock_mode_enter(r128_clock_t *r128,
	r128_clock_io_t *r128_io,
	r128_clock_mode_t *r128_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	/* select clock 3 */
	KRN_DEBUG(1, "selecting clock 3 for programming");
	R128_CTRL_OUT32(r128_io,
		R128_CTRL_IN32(r128_io, R128_CLOCK_CNTL_INDEX) |
		0x3 << R128_08_PPLL_DIV_SELShift, R128_CLOCK_CNTL_INDEX);
		
	/* reset */
	KRN_DEBUG(1, "resetting pll");
	R128_PLL_OUT32(r128_io,
		R128_PLL_IN32(r128_io, R128_PPLL_CNTL) |
		R128_02_PPLL_RESET | R128_02_PPLL_ATOMIC_UPDATE_EN |
		R128_02_PPLL_VGA_ATOMIC_UPDATE_EN, 
		R128_PPLL_CNTL);
	
	/* set the reference divider */
	KRN_DEBUG(1, "programming reference divider");
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io, 
		R128_PLL_IN32(r128_io, R128_PPLL_REF_DIV) |
		(r128_mode->r128.ref_divider << R128_03_PPLL_REF_DIVShift),
		R128_PPLL_REF_DIV);
	r128_pll_update(r128_io);
	
	/* set the feedback divider */
	KRN_DEBUG(1, "programming feedback divider");
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io, (R128_PLL_IN32(r128_io, R128_PLL_DIV_3) & 
		~R128_07_PPLL_FB3_DIVMask) | 
		(r128_mode->r128.feedback_divider << R128_07_PPLL_FB3_DIVShift), 
		R128_PLL_DIV_3);
	r128_pll_update(r128_io);
	
	/* set the post divider */
	KRN_DEBUG(1, "programming post divider");
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io,	(R128_PLL_IN32(r128_io, R128_PLL_DIV_3) &
		~R128_07_PPLL_POST3_DIVMask) |
		(r128_mode->r128.post_divider << R128_07_PPLL_POST3_DIVShift),
		R128_PLL_DIV_3);
	r128_pll_update(r128_io);
	
	/* clear HTOTAL_CNTL */
	KRN_DEBUG(1, "clearing HTOTAL_CNTL");
	r128_pll_wait_until_updated(r128_io);
	R128_PLL_OUT32(r128_io, 0x0, R128_HTOTAL_CNTL);
	r128_pll_update(r128_io);
	
	/* clear reset again */
	R128_PLL_OUT32(r128_io, 
		R128_PLL_IN32(r128_io, R128_PPLL_CNTL) & ~R128_02_PPLL_RESET, 
		R128_PPLL_CNTL);
		
	KRN_DEBUG(1, "programming dda registers");
	R128_CTRL_OUT32(r128_io, r128_mode->r128.dda_config,
		R128_DDA_CONFIG);
	
	R128_CTRL_OUT32(r128_io, r128_mode->r128.dda_on_off,
		R128_DDA_ON_OFF);

	/* disable atomic updates */
	KRN_DEBUG(1, "disabling atomic updates");
	R128_PLL_OUT32(r128_io,
		R128_PLL_IN32(r128_io, R128_PPLL_CNTL) &
		~(R128_02_PPLL_ATOMIC_UPDATE_EN | 
		R128_02_PPLL_VGA_ATOMIC_UPDATE_EN),
		R128_PPLL_CNTL);
}
