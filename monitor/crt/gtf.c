/* ----------------------------------------------------------------------------
**	Generalized Timing Formula implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2002		Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: gtf.c,v $
**	Revision 1.3  2003/03/15 20:40:03  redmondp
**	-define MAINTAINER and KGIM_MONITOR_DRIVER for all c files
**	
**	Revision 1.2  2003/02/02 23:02:40  cegger
**	fixed compiler warning 'comparisons like X<=Y<=Z do not have their mathematical meaning'
**	
**	Revision 1.1  2002/12/09 18:20:29  fspacek
**	- initial version of the unified monitor driver
**	
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Filip_Spacek
#define	KGIM_MONITOR_DRIVER	"$Revision: 1.3 $"

#define KGI_SYS_NEED_i64 1
#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include "monitor/crt/crt-meta.h"

/*
 * Default duty cycle calculation parameters
 */
#define K 128
#define J 20
#define C 40
#define M 600

/*
 * Derived parameters actually used for calculation
 */
#define C_PRIME (((C-J)*K / 256) + J)
#define M_PRIME ((K * M)/256)

/*
 * Margin size (fraction of the horizontal or vertical size)
 */
#define MARGIN_MUL        18
#define MARGIN_DIV        1000

/*
 * H sync size as a fraction of the total horizontal size
 */
#define H_SYNC_MUL        8
#define H_SYNC_DIV        100

/*
 * Cell size is the minimum number of addressable (horzontal) pixels
 * FIXME: Are there cards that have different values? Is it worth exporting
 * this value to the chipset driver?
 */
#define CELL_SIZE         8

/*
 * Minimum time (in nanoseconds) for the vertical sync and vertical back
 * porch.
 */
#define MIN_VSYNC_PLUS_BP 550000

/*
 * Number of lines in a vertical sync
 */
#define V_SYNC            3

/*
 * Minimum number of lines in a vertical front porch, or character cells
 * in a horizontal front porch.
 */
#define MIN_PORCH_LINES   1	

#define ROUND_DIV(n, d) (((n) + ((d)/2))/(d))

#define KRN_DEBUG_TIMING(t, name) \
	KRN_DEBUG(4, "%s.width = %d", name, t.width); \
	KRN_DEBUG(4, "%s.blankstart = %d", name, t.blankstart); \
	KRN_DEBUG(4, "%s.syncstart = %d", name, t.syncstart); \
	KRN_DEBUG(4, "%s.syncend = %d", name, t.syncend); \
	KRN_DEBUG(4, "%s.blankend = %d", name, t.blankend); \
	KRN_DEBUG(4, "%s.total = %d", name, t.total); \
	KRN_DEBUG(4, "%s.polarity = %d", name, t.polarity);

static kgi_error_t crt_monitor_gtf_from_vfreq(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode, kgi_u32_t vfreq)
{
	kgi_u32_t top_margin, bottom_margin, left_margin, right_margin;
	kgi_u32_t dots_x, dots_y;
	kgi_u32_t h_blank, h_sync;
	kgi_u32_t v_sync_plus_bp, v_lines_total; 
	kgi_u32_t h_period_est, h_period;
	kgi_u32_t ideal_duty_cycle;
	
	kgi_u32_t v_field_rate_req = vfreq;
	
	/* Width has to be a multiple of the cell size */
	dots_x = (crt_mode->kgim.in.dots.x / CELL_SIZE) * CELL_SIZE;
	
	/* Height is halved for interlaced modes */
	dots_y = crt_mode->kgim.in.dots.y;
	if (crt_mode->kgim.in.flags & KGI_DPF_TP_LRTB_I1) {
	
		dots_y /= 2;
		v_field_rate_req /= 2;
	}
	
	KRN_DEBUG(5, "x: %d, y: %d", dots_x, dots_y);
	
	/* All margins are calculated as a fraction of the total resolution */
	if (0) {
		left_margin = (dots_x * MARGIN_MUL) / MARGIN_DIV;
		right_margin = (dots_x * MARGIN_MUL) / MARGIN_DIV;
		top_margin = (dots_y * MARGIN_MUL) / MARGIN_DIV;
		bottom_margin = (dots_y * MARGIN_MUL) / MARGIN_DIV;
	}
	else {
		left_margin = 0;
		right_margin = 0;
		top_margin = 0;
		bottom_margin = 0;
	}
	
	KRN_DEBUG(5, "margins l: %d, r: %d, t: %d, b: %d",
		  left_margin, right_margin, top_margin, bottom_margin);
	
	/* First we estimate the horizontal period. To do this we take the
	 * required vertical field rate minus the smallest required time
	 * for vertical flyback and divide it by the total number of active
	 * lines. (result in nanoseconds)
	 */
	/* FIXME: add interlace to the bottom */
	h_period_est = (1000000000/v_field_rate_req - MIN_VSYNC_PLUS_BP) /
		(dots_y + top_margin + bottom_margin + MIN_PORCH_LINES);
	
	KRN_DEBUG(5, "h_period_est: %d", h_period_est);
	
	/* Calculate the number of lines for vertical sync plus back porch. */
	v_sync_plus_bp = ROUND_DIV(MIN_VSYNC_PLUS_BP, h_period_est);
	
	KRN_DEBUG(5, "v_sync_plus_bp: %d", v_sync_plus_bp);

	/* Total of vertical lines. */
	// FIXME: add interlace to the total
	v_lines_total = dots_y + top_margin + bottom_margin +
			 v_sync_plus_bp + MIN_PORCH_LINES;
			 
	KRN_DEBUG(5, "v_lines_total: %d", v_lines_total);
			 
	/* The actual horizontal period. The specs have this calculation
	 * in several steps, this is the simplified formula. (nanoseconds)
	 */
	h_period = 1000000000/(v_field_rate_req*v_lines_total);

	KRN_DEBUG(5, "h_period: %d", h_period);
	
	/* The ideal duty cycle is a curve depending on the horizontal
	 * period and some constants. Result in 1/100000 of visible width.
	 */
	ideal_duty_cycle = C_PRIME*1000 - (M_PRIME*h_period / 1000);
	
	KRN_DEBUG(5, "ideal_duty_cycle: %d", ideal_duty_cycle);
	
	/* The size of horizontal blank is given by the ideal duty cycle
	 * fraction of the total horizontal width.
	 */ 
	h_blank = 
	ROUND_DIV((dots_x + left_margin + right_margin)*ideal_duty_cycle,
		(100000 - ideal_duty_cycle));
	
	KRN_DEBUG(5, "h_blank: %d", h_blank);
	
	/* Horizontal blank needs to be rounded to twice the cell size
	 * (h sync end sits at exactly middle of h blank so h blank needs
	 * to be nicely divisible by 2)
	 */
	h_blank = ROUND_DIV(h_blank, (2*CELL_SIZE)) * (2*CELL_SIZE);
	
	KRN_DEBUG(5, "h_blank (rounded) : %d", h_blank);

	/* Horizontal sync is a percentage of the total horizontal width
	 * (cell size rounded)
	 */
	//h_sync = ROUND_DIV((dots_x + left_margin + right_margin + h_blank) * 
	//	H_SYNC_MUL, H_SYNC_DIV);
	
	h_sync = (dots_x + left_margin + right_margin + h_blank) *
		H_SYNC_MUL / H_SYNC_DIV;
	
	KRN_DEBUG(5, "h_sync: %d", h_sync);
	
	h_sync = ROUND_DIV(h_sync, CELL_SIZE) * CELL_SIZE;
	
	KRN_DEBUG(5, "h_sync (rounded): %d", h_sync);

	crt_mode->kgim.x.width = dots_x; //crt_mode->kgim.in.x;
	crt_mode->kgim.x.blankstart = dots_x + right_margin;
	crt_mode->kgim.x.syncstart =
		crt_mode->kgim.x.blankstart + h_blank/2 - h_sync;
	crt_mode->kgim.x.syncend =
		crt_mode->kgim.x.blankstart + h_blank/2;
	crt_mode->kgim.x.blankend =
		crt_mode->kgim.x.blankstart + h_blank;
	crt_mode->kgim.x.total =
		crt_mode->kgim.x.blankend + left_margin;
	crt_mode->kgim.x.polarity = 0;
	
	KRN_DEBUG_TIMING(crt_mode->kgim.x, "x");
	
	crt_mode->kgim.y.width = crt_mode->kgim.in.dots.y;
	crt_mode->kgim.y.blankstart = 
		crt_mode->kgim.in.dots.y + bottom_margin;
	crt_mode->kgim.y.syncstart = 
		crt_mode->kgim.y.blankstart + MIN_PORCH_LINES;
	crt_mode->kgim.y.syncend = 
		crt_mode->kgim.y.syncstart + V_SYNC;
	crt_mode->kgim.y.blankend =
		crt_mode->kgim.y.syncstart + v_sync_plus_bp;
	crt_mode->kgim.y.total =
		crt_mode->kgim.y.blankend + top_margin;
	crt_mode->kgim.y.polarity = 1;
	
	KRN_DEBUG_TIMING(crt_mode->kgim.y, "y");

	crt_mode->kgim.in.dclk =
		(crt_mode->kgim.x.total * 100000)/h_period * 10000;
		
	KRN_DEBUG(4, "dclk: %d", crt_mode->kgim.in.dclk);
	
	return KGI_EOK;
}

static kgi_error_t crt_monitor_gtf_from_hfreq(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode, kgi_u32_t hfreq)
{
	kgi_u32_t top_margin, bottom_margin, left_margin, right_margin;
	kgi_u32_t dots_x, dots_y;
	kgi_u32_t h_blank, h_sync;
	kgi_u32_t v_sync_plus_bp, v_lines_total; 
	kgi_u32_t ideal_duty_cycle;
	
	/* Width has to be a multiple of the cell size */
	dots_x = (crt_mode->kgim.in.dots.x / CELL_SIZE) * CELL_SIZE;
	
	/* Height is halved for interlaced modes */
	dots_y = crt_mode->kgim.in.dots.y;
	if (crt_mode->kgim.in.flags & KGI_DPF_TP_LRTB_I1) {
	
		dots_y /= 2;
	}
	
	KRN_DEBUG(5, "x: %d, y: %d", dots_x, dots_y);
	
	/* All margins are calculated as a fraction of the total resolution */
	if (0) {
		left_margin = (dots_x * MARGIN_MUL) / MARGIN_DIV;
		right_margin = (dots_x * MARGIN_MUL) / MARGIN_DIV;
		top_margin = (dots_y * MARGIN_MUL) / MARGIN_DIV;
		bottom_margin = (dots_y * MARGIN_MUL) / MARGIN_DIV;
	}
	else {
		left_margin = 0;
		right_margin = 0;
		top_margin = 0;
		bottom_margin = 0;
	}
	
	KRN_DEBUG(5, "margins l: %d, r: %d, t: %d, b: %d",
		  left_margin, right_margin, top_margin, bottom_margin);
	
	/* Calculate the number of lines for vertical sync plus back porch. */
	v_sync_plus_bp = ROUND_DIV(MIN_VSYNC_PLUS_BP, hfreq);
		
	KRN_DEBUG(5, "v_sync_plus_bp: %d", v_sync_plus_bp);

	/* Total of vertical lines. */
	/* FIXME: add interlace to the total */
	v_lines_total = dots_y + top_margin + bottom_margin +
			 v_sync_plus_bp + MIN_PORCH_LINES;
			 
	KRN_DEBUG(5, "v_lines_total: %d", v_lines_total);
	
	/* The ideal duty cycle is a curve depending on the horizontal
	 * period and some constants. Result in 1/100000 of visible width.
	 */
	ideal_duty_cycle = C_PRIME*1000 - (M_PRIME*hfreq / 1000);
	
	KRN_DEBUG(5, "ideal_duty_cycle: %d", ideal_duty_cycle);
	
	/* The size of horizontal blank is given by the ideal duty cycle
	 * fraction of the total horizontal width.
	 */ 
	h_blank = 
	ROUND_DIV((dots_x + left_margin + right_margin)*ideal_duty_cycle,
		(100000 - ideal_duty_cycle));
	
	KRN_DEBUG(5, "h_blank: %d", h_blank);
	
	/* Horizontal blank needs to be rounded to twice the cell size
	 * (h sync end sits at exactly middle of h blank so h blank needs
	 * to be nicely divisible by 2)
	 */
	h_blank = ROUND_DIV(h_blank, (2*CELL_SIZE)) * (2*CELL_SIZE);
	
	KRN_DEBUG(5, "h_blank (rounded) : %d", h_blank);

	/* Horizontal sync is a percentage of the total horizontal width
	 * (cell size rounded)
	 */
	//h_sync = ROUND_DIV((dots_x + left_margin + right_margin + h_blank) * 
	//	H_SYNC_MUL, H_SYNC_DIV);
	
	h_sync = (dots_x + left_margin + right_margin + h_blank) *
		H_SYNC_MUL / H_SYNC_DIV;
	
	KRN_DEBUG(5, "h_sync: %d", h_sync);
	
	h_sync = ROUND_DIV(h_sync, CELL_SIZE) * CELL_SIZE;
	
	KRN_DEBUG(5, "h_sync (rounded): %d", h_sync);

	crt_mode->kgim.x.width = dots_x; //crt_mode->kgim.in.x;
	crt_mode->kgim.x.blankstart = dots_x + right_margin;
	crt_mode->kgim.x.syncstart =
		crt_mode->kgim.x.blankstart + h_blank/2 - h_sync;
	crt_mode->kgim.x.syncend =
		crt_mode->kgim.x.blankstart + h_blank/2;
	crt_mode->kgim.x.blankend =
		crt_mode->kgim.x.blankstart + h_blank;
	crt_mode->kgim.x.total =
		crt_mode->kgim.x.blankend + left_margin;
	crt_mode->kgim.x.polarity = 0;
	
	KRN_DEBUG_TIMING(crt_mode->kgim.x, "x");
	
	crt_mode->kgim.y.width = crt_mode->kgim.in.dots.y;
	crt_mode->kgim.y.blankstart = 
		crt_mode->kgim.in.dots.y + bottom_margin;
	crt_mode->kgim.y.syncstart = 
		crt_mode->kgim.y.blankstart + MIN_PORCH_LINES;
	crt_mode->kgim.y.syncend = 
		crt_mode->kgim.y.syncstart + V_SYNC;
	crt_mode->kgim.y.blankend =
		crt_mode->kgim.y.syncstart + v_sync_plus_bp;
	crt_mode->kgim.y.total =
		crt_mode->kgim.y.blankend + top_margin;
	crt_mode->kgim.y.polarity = 1;
	
	KRN_DEBUG_TIMING(crt_mode->kgim.y, "y");

	crt_mode->kgim.in.dclk =
		(crt_mode->kgim.x.total * 100000)/hfreq * 10000;
		
	KRN_DEBUG(4, "dclk: %d", crt_mode->kgim.in.dclk);
	
	return KGI_EOK;
}

static kgi_u32_t isqrt(kgi_u64_t val)
{
	kgi_u64_t guess = 0;
	kgi_u64_t bit = 1ULL << 31;
	
	do {
		guess ^= bit;
		
		if (guess * guess > val){
		
			guess ^= bit;
		}
		
	} while (bit >>= 1);
	
	return (kgi_u32_t)guess;
}

static kgi_error_t crt_monitor_gtf_from_dclk(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io,
	crt_monitor_mode_t *crt_mode)
{
	kgi_u32_t top_margin, bottom_margin, left_margin, right_margin;
	kgi_u32_t dots_x, dots_y, total_active_pixels;
	kgi_u32_t dclk;
	kgi_u32_t ideal_h_period, ideal_duty_cycle;
	kgi_u32_t h_blank, h_sync, v_sync_plus_bp;
	kgi_u64_t temp;
	
	dclk = crt_mode->kgim.in.dclk;
	
	KRN_DEBUG(4, "dclk: %d", dclk);

	/* Width has to be a multiple of the cell size */
	dots_x = (crt_mode->kgim.in.dots.x / CELL_SIZE) * CELL_SIZE;
	
	/* Height is halved for interlaced modes */
	dots_y = crt_mode->kgim.in.dots.y;
	if (crt_mode->kgim.in.flags & KGI_DPF_TP_LRTB_I1) {
	
		dots_y /= 2;
	}
	
	KRN_DEBUG(5, "x: %d, y: %d", dots_x, dots_y);
	
	/* All margins are calculated as a fraction of the total resolution */
	if (0) {
		left_margin = (dots_x * MARGIN_MUL) / MARGIN_DIV;
		right_margin = (dots_x * MARGIN_MUL) / MARGIN_DIV;
		top_margin = (dots_y * MARGIN_MUL) / MARGIN_DIV;
		bottom_margin = (dots_y * MARGIN_MUL) / MARGIN_DIV;
	}
	else {
		left_margin = 0;
		right_margin = 0;
		top_margin = 0;
		bottom_margin = 0;
	}
	
	KRN_DEBUG(5, "margins l: %d, r: %d, t: %d, b: %d",
		  left_margin, right_margin, top_margin, bottom_margin);
	
	total_active_pixels = dots_x + left_margin + right_margin;
	
	KRN_DEBUG(5, "total_active_pixels: %d", total_active_pixels);

	/*
	 * Finding the ideal horizontal period is the hardest part. The
	 * official formula is
	 *
	 *    (C'-100) + sqrt((100-C')^2 - (.4 * M' * h_total/dclk))
	 *    ------------------------------------------------------ * 1000
	 *                          2*M'
	 *
	 * In order to obtain the necessary accuaracy 64-bit numbers are
	 * used for the calcuation of the square root. The above formula
	 * produces h_period in usec. As always we work with nsec. Further,
	 * we use the fact that M' is a multiple of 100 to produce:
	 *
	 *
	 *    (C'-100)*10000 + sqrt((100-C')^2*100000000 - 
	 *                          (40000000 * M' * h_total / dclk))
	 *     ------------------------------------------------------
	 *                        2*(M'/100)
	 *
	 * Finally, the above formula assumes dclk in MHz while ours is in Hz.
	 * So we divide by dclk/1000 and multiply by 1000 hoping that losing
	 * the 3 digits on the dclk isn't too much.
	 */
	
	temp = 40000000ULL * M_PRIME * total_active_pixels + (dclk/2000);
	/* do_div(temp, (dclk/1000)); */
	temp = temp / (dclk/1000);
	temp = (100 - C_PRIME)*(100 - C_PRIME) * 100000000ULL + temp*1000;
	
	/*
	temp = (100 - C_PRIME)*(100 - C_PRIME) * 100000000ULL +
		((40000000ULL * M_PRIME * total_active_pixels + (dclk/2000)) /
		 (dclk/1000) * 1000);
	*/

	KRN_DEBUG(5, "sqrt(temp): %d", isqrt(temp));
		 
	ideal_h_period = ((C_PRIME-100)*10000 + isqrt(temp)) / (2*(M_PRIME/100));
	
	KRN_DEBUG(5, "ideal_h_period: %d", ideal_h_period);
	
	/* Find the ideal blanking duty cycle. */
	ideal_duty_cycle = C_PRIME*1000 - M_PRIME*ideal_h_period/1000;
	
	KRN_DEBUG(5, "ideal_duty_cycle: %d", ideal_duty_cycle);
	
	/* Get the number of pixels in the blanking time */
	h_blank = total_active_pixels*ideal_duty_cycle / 
		(100000 - ideal_duty_cycle);
	
	KRN_DEBUG(5, "h_blank: %d", h_blank);
	
	/* Round the blanking time to the nearest 2 character cell sizes. */
	h_blank = ROUND_DIV(h_blank, (2*CELL_SIZE)) * (2*CELL_SIZE);
	
	KRN_DEBUG(5, "h_blank (rounded): %d", h_blank);

	/* Horizontal sync is a percentage of the total horizontal width
	 * (cell size rounded)
	 */
	//h_sync = ROUND_DIV((dots_x + left_margin + right_margin + h_blank) * 
	//	H_SYNC_MUL, H_SYNC_DIV);
	
	h_sync = (dots_x + left_margin + right_margin + h_blank) *
		H_SYNC_MUL / H_SYNC_DIV;
	
	KRN_DEBUG(5, "h_sync: %d", h_sync);
	
	h_sync = ROUND_DIV(h_sync, CELL_SIZE) * CELL_SIZE;
	
	KRN_DEBUG(5, "h_sync (rounded): %d", h_sync);

	/*
	 * The number of lines in v sync plus vertical back porch is given
	 * by MIN_VSYNC_PLUS_BP*dclk/total_pixels. According to the
	 * standard the first should be in usec and second in MHz while
	 * we have the first in nsec and second in Hz so lotsa dividing
	 * is necessary.
	 */
	v_sync_plus_bp = ROUND_DIV((MIN_VSYNC_PLUS_BP/10000)*(dclk/1000),
				   (total_active_pixels + h_blank));
	v_sync_plus_bp = ROUND_DIV(v_sync_plus_bp, 100);
	
	KRN_DEBUG(5, "v_sync_plus_bp: %d", v_sync_plus_bp);
	
	crt_mode->kgim.x.width = dots_x; //crt_mode->kgim.in.x;
	crt_mode->kgim.x.blankstart = dots_x + right_margin;
	crt_mode->kgim.x.syncstart =
		crt_mode->kgim.x.blankstart + h_blank/2 - h_sync;
	crt_mode->kgim.x.syncend =
		crt_mode->kgim.x.blankstart + h_blank/2;
	crt_mode->kgim.x.blankend =
		crt_mode->kgim.x.blankstart + h_blank;
	crt_mode->kgim.x.total =
		crt_mode->kgim.x.blankend + left_margin;
	crt_mode->kgim.x.polarity = 0;
	
	KRN_DEBUG_TIMING(crt_mode->kgim.x, "x");
	
	crt_mode->kgim.y.width = crt_mode->kgim.in.dots.y;
	crt_mode->kgim.y.blankstart = 
		crt_mode->kgim.in.dots.y + bottom_margin;
	crt_mode->kgim.y.syncstart = 
		crt_mode->kgim.y.blankstart + MIN_PORCH_LINES;
	crt_mode->kgim.y.syncend = 
		crt_mode->kgim.y.syncstart + V_SYNC;
	crt_mode->kgim.y.blankend =
		crt_mode->kgim.y.syncstart + v_sync_plus_bp;
	crt_mode->kgim.y.total =
		crt_mode->kgim.y.blankend + top_margin;
	crt_mode->kgim.y.polarity = 1;
	
	KRN_DEBUG_TIMING(crt_mode->kgim.y, "y");

	KRN_DEBUG(4, "dclk: %d", crt_mode->kgim.in.dclk);
	
	return KGI_EOK;
}


kgi_error_t crt_monitor_gtf_propose(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode)
{
	kgi_u32_t hfreq, vfreq, clamped;
	kgi_u_t i;
	
	/* No ranges means no GTF. */
	if (crt->num_hfreq == 0 || crt->num_vfreq == 0) {
	
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
	
	/* FIXME: This needs to be user configurable */
	vfreq = 85;
	clamped = 0xffffffff;
	
	for (i = 0; i < crt->num_vfreq; ++i) {
		
		/* If vfreq fits in the range all is good. */
		if (crt->vfreq[i].min <= vfreq && crt->vfreq[i].max >= vfreq)
			
			break;
		
		/* Otherwise try to clamp the vfreq to the closest limit */
		if ((crt->vfreq[i].min >= vfreq)
		  && ((crt->vfreq[i].min - vfreq) < 
			((clamped > vfreq) ?
				clamped - vfreq : vfreq - clamped)))
		{	
			clamped = crt->vfreq[i].min;
		}
		
		if ((crt->vfreq[i].max <= vfreq)
		  && ((vfreq - crt->vfreq[i].max) < 
			((clamped > vfreq) ?
				clamped - vfreq : vfreq - clamped)))
		{
			clamped = crt->vfreq[i].max;
		}
	}
	
	/* We didn't find any matching range for vfreq */
	if (i == crt->num_vfreq) {
	
		vfreq = clamped;
	}
	
	crt_monitor_gtf_from_vfreq(crt, crt_io, crt_mode, vfreq);
	
	/* Now we need to verify that the computed hfreq is within limits */
	
	hfreq = ROUND_DIV(crt_mode->kgim.in.dclk, crt_mode->kgim.x.total);
	clamped = 0xffffffff;
	
	for (i = 0; i < crt->num_hfreq; ++i) {
	
		if (crt->hfreq[i].min <= hfreq && crt->hfreq[i].max >= hfreq)
		
			break;
		
		if ((crt->hfreq[i].min >= hfreq)
		  && ((crt->hfreq[i].min - hfreq) < 
			((clamped > hfreq) ?
				clamped - hfreq : hfreq - clamped)))
		{		
			clamped = crt->hfreq[i].min;
		}

		if ((crt->hfreq[i].max <= hfreq)
		  && ((hfreq - crt->hfreq[i].max) < 
			((clamped > hfreq) ?
				clamped - hfreq : hfreq - clamped)))
		{		
			clamped = crt->hfreq[i].max;
		}
	}
	
	/* If the computed hfreq is out of bounds recalculate the timings */
	if (i == crt->num_hfreq) {
	
		crt_monitor_gtf_from_hfreq(crt, crt_io, crt_mode, clamped);
		
		/* Finally check the vfreq again */
		vfreq = ROUND_DIV(ROUND_DIV(crt_mode->kgim.in.dclk,
			crt_mode->kgim.x.total), crt_mode->kgim.y.total);
		
		for (i = 0; i < crt->num_vfreq; ++i) {
		
			if (crt->vfreq[i].min <= vfreq &&
				crt->vfreq[i].max >= vfreq)
			
				break;
		}
		
		/* If the vfreq doesn't fit then there's nothing more to try */
		if (i == crt->num_vfreq) {
		
			return -KGI_ERRNO(MONITOR, UNKNOWN);
		}
	}
	
	return KGI_EOK;
}

kgi_error_t crt_monitor_gtf_adjust(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io,
	crt_monitor_mode_t *crt_mode)
{
	kgi_u32_t hfreq, vfreq;
	kgi_u_t i;

	/* First check whether the dclk we are trying to use is valid */
	for (i = 0; i < crt->num_dclk; ++i) {
	
		if (crt->dclk[i].min <= crt_mode->kgim.in.dclk &&
			crt->dclk[i].min >= crt_mode->kgim.in.dclk)
		
			break;
	}
	if (i == crt->num_dclk) {
	
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}

	/* Calculate the new timings using only the dot clock */
	crt_monitor_gtf_from_dclk(crt, crt_io, crt_mode);
	
	/* Check whether the generated hfreq and vfreq are within limits */
	
	hfreq = ROUND_DIV(crt_mode->kgim.in.dclk, crt_mode->kgim.x.total);
	
	for (i = 0; i < crt->num_hfreq; ++i) {
	
		if (crt->hfreq[i].min <= hfreq && crt->hfreq[i].max >= hfreq)
		
			break;
	}
	if (i == crt->num_hfreq) {
	
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
	
	vfreq = ROUND_DIV(hfreq, crt_mode->kgim.y.total);

	for (i = 0; i < crt->num_vfreq; ++i) {

		if (crt->vfreq[i].min <= vfreq && crt->vfreq[i].max >= vfreq)
		
			break;
	}
	
	if (i == crt->num_vfreq) {
	
		return -KGI_ERRNO(MONITOR, UNKNOWN);
	}
	
	return KGI_EOK;
}

