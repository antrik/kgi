/* ----------------------------------------------------------------------------
**	generic PLL routines
** ----------------------------------------------------------------------------
**	Copyright	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	Generic routines to calculate PLL parameter settings. The formula
**	implemented is 
**		fout = (fref * a.mul/a.div * mul/div) / 2^p  .
**	Please check your manual for further reference.
** ----------------------------------------------------------------------------
**	$Log: pll-meta.c,v $
**	Revision 1.2  2002/06/25 02:39:17  aldot
**	- typo in comment
**	
**	Revision 1.1.1.1  2000/09/21 09:57:15  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:13  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#include <kgi/module.h>

#include "clock/pll-meta.h"

/*	find the best parameter set to produce a certain frequency f
*/
static kgi_error_t pll_clock_best_mode(pll_clock_t *pll,
	pll_clock_mode_t *pll_mode, kgi_u_t f, kgi_timing_command_t cmd)
{
	kgi_ratio_t	ratio;
	kgi_u_t		shift;
	
	KRN_DEBUG(2, "pll_clock_best_mode()");

	KRN_ASSERT(0 < pll->fvco.min);
	KRN_ASSERT(pll->fvco.min <= pll->fvco.max);

	KRN_ASSERT(pll->p.min <= pll->p.max);

	KRN_ASSERT(0 < pll->mul.min);
	KRN_ASSERT(pll->mul.min <= pll->mul.max);

	KRN_ASSERT(0 < pll->div.min);
	KRN_ASSERT(pll->div.min <= pll->div.max);

	/*	clear state
	*/
	kgim_memset(pll_mode, 0, sizeof(*pll_mode));

	/*	determine post-VCO divider/multiplier
	*/
	if ((f < pll->fvco.min) || (pll->fvco.max < f)) {

		if (f < pll->fvco.min) {

			pll_mode->p = pll->p.min;
			pll_mode->fvco = f << pll_mode->p;

			while ((pll_mode->fvco < pll->fvco.min) &&
				(pll_mode->p < pll->p.max)) {

				pll_mode->p++;
				pll_mode->fvco = f << pll_mode->p;
			}

		} else {

			pll_mode->p = (pll->p.max > 0) ? 0 : pll->p.max;
			pll_mode->fvco = f >> (- pll_mode->p);

			while ((pll_mode->fvco > pll->fvco.min) &&
				(pll_mode->p > pll->p.min)) {

				pll_mode->p--;
				pll_mode->fvco = f >> (- pll_mode->p);
			}
		}

		if ((pll_mode->fvco < pll->fvco.min) ||
			(pll->fvco.max < pll_mode->fvco) ||
			(pll_mode->p < pll->p.min) ||
			(pll->p.max < pll_mode->p)) {

			KRN_DEBUG(1, "fvco = %i Hz, p = %i is out of range!", \
				pll_mode->fvco, pll_mode->p);
			return -KGI_ERRNO(CLOCK, INVAL);
		}

	} else {

		pll_mode->fvco = f;
		pll_mode->p = 0;
	}

	if (((pll_mode->fvco << 1) < pll->fvco.max) &&
		(pll_mode->p < pll->p.max)) {

		pll_mode->p++;
		pll_mode->fvco = f << pll_mode->p;
	}

	ratio.mul = pll_mode->fvco * pll->a.div;
	ratio.div = pll->fref * pll->a.mul;

	KRN_ASSERT(ratio.mul > 0);
	KRN_ASSERT(ratio.div > 0);

	/*	avoid integer overflow
	*/
	shift = 0;
	while (((ratio.mul >> shift) > 0x00100000) &&
		((ratio.div >> shift) > 0x00100000)) {

		shift++;
	}

	if (shift) {

		if (cmd == KGI_TC_LOWER) {
	
			ratio.mul = ratio.mul >> shift;
			ratio.div = (ratio.div - ((1 << shift) - 1)) >> shift;

		} else {

			ratio.mul = (ratio.mul - ((1 << shift) - 1)) >> shift;
			ratio.div = ratio.div >> shift;
		}
	}

	if (0 == ratio.mul  ||  0 == ratio.div) {

		KRN_DEBUG(0, "bad parameter set!!");
		return -KGI_ERRNO(CLOCK, INVAL);
	}

	/*	now determine the best mul and div values.
	*/
	pll_mode->div = pll_mode->mul = -1;
	if ((pll->mul.max - pll->mul.min) < (pll->div.max - pll->div.min)) {

		kgi_s_t best_err = 0x7FFFFFFF;
		kgi_u_t mul = pll->mul.min;
		kgi_u_t div = ((ratio.div * mul) / ratio.mul) +
			(cmd == KGI_TC_LOWER ? 1 : 0);

		while (mul < pll->mul.max) {

			register kgi_s_t err = (cmd == KGI_TC_LOWER)
				? div * ratio.mul - mul * ratio.div
				: mul * ratio.div - div * ratio.mul;

			if ((0 <= err) && (err < best_err)) {

				best_err = err;
				pll_mode->mul = mul;
				pll_mode->div = div;
			}

			mul++;
			div = ((ratio.div * mul) / ratio.mul) +
				(cmd == KGI_TC_LOWER ? 1 : 0);

			if (pll->div.max < div) {

				break;
			}
		}
		
	} else {

		kgi_s_t best_err = 0x7FFFFFFF;
		kgi_u_t div = pll->div.min;
		kgi_u_t mul = ((ratio.mul * div) / ratio.div) +
			(cmd == KGI_TC_LOWER ? 0 : 1);

		while (div < pll->div.max) {

			register kgi_s_t err = (cmd == KGI_TC_LOWER)
				? div * ratio.mul - mul * ratio.div
				: mul * ratio.div - div * ratio.mul;

			if ((0 <= err) && (err < best_err)) {

				best_err = err;
				pll_mode->mul = mul;
				pll_mode->div = div;
			}

			div++;
			mul = ((ratio.mul * div) / ratio.div) +
				(cmd == KGI_TC_LOWER ? 0 : 1);

			if (pll->mul.max < mul) {

				break;
			}
		}
	}

	if (pll_mode->mul < pll->mul.min  ||  pll->mul.max < pll_mode->mul ||
		pll_mode->div < pll->div.min || pll->div.max < pll_mode->div) {

		KRN_DEBUG(1, "parameters out of range");
		return -KGI_ERRNO(CLOCK, INVAL);
	}

	pll_mode->f = pll->fref * pll_mode->mul / pll_mode->div;
	pll_mode->f = pll_mode->f * pll->a.mul / pll->a.div;
	pll_mode->f = (pll_mode->p >= 0)
		? pll_mode->f >> pll_mode->p : pll_mode->f << (-pll_mode->p);

	KRN_DEBUG(1, "fout = %i, fvco = %i, mul = %i, div = %i, p = %i", \
		pll_mode->f, pll_mode->fvco, pll_mode->mul, pll_mode->div, \
		pll_mode->p);
	return KGI_EOK;
}

kgi_error_t pll_clock_mode_check(pll_clock_t *pll, pll_clock_io_t *pll_io,
	pll_clock_mode_t *pll_mode, kgi_timing_command_t cmd,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	kgi_s_t newfreq = (cmd == KGI_TC_PROPOSE) 
		? 0 : dpm->dclk * dpm->rclk.mul / dpm->rclk.div;
	const kgi_s_t min_freq = pll->clock.dclk.range[0].min;
	const kgi_s_t max_freq = pll->clock.dclk.range[0].max;

	KRN_DEBUG(2, "pll_clock_mode_check()");

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

		if (pll_clock_best_mode(pll, pll_mode, newfreq, cmd)) {

			KRN_DEBUG(1, "lower failed: DCLK = %i Hz, CLK = %i Hz",
				dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		dpm->dclk = pll_mode->f * dpm->rclk.div / dpm->rclk.mul;
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

		if (pll_clock_best_mode(pll, pll_mode, newfreq, cmd)) {

			KRN_DEBUG(1, "raise failed: DCLK = %i Hz, CLK = %i Hz",
				dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		dpm->dclk = pll_mode->f * dpm->rclk.div / dpm->rclk.mul;
		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(2, "KGI_TC_CHECK:");

		if (pll_clock_best_mode(pll, pll_mode, newfreq, 
			KGI_TC_LOWER)) {

			KRN_DEBUG(1, "check failed: DCLK = %i Hz, CLK = %i Hz",
				dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}
		return KGI_EOK;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CLOCK, UNKNOWN);
	}
}
