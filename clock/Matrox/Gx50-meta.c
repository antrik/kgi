/* ----------------------------------------------------------------------------
**	Matrox Gx50 clock driver meta
** ----------------------------------------------------------------------------
**	Copyright (C)	2002		Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx50-meta.c,v 1.1 2002/11/30 18:23:14 ortalo Exp $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1 $"

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#include <kgi/module.h>

#include "chipset/Matrox/Gx00.h"
#include "clock/Matrox/Gx50-meta.h"

/*	KGI interface
*/

kgi_error_t Gx50_clock_init(Gx50_clock_t *Gx50,	Gx50_clock_io_t *Gx50_io,
	const kgim_options_t *options) 
{
  Gx50->saved.pixclkctrl = Gx50_EDAC_IN8(Gx50_io, XPIXCLKCTRL);
  Gx50->saved.pixcm = Gx50_EDAC_IN8(Gx50_io, XPIXPLLCM);
  Gx50->saved.pixcn = Gx50_EDAC_IN8(Gx50_io, XPIXPLLCN);
  Gx50->saved.pixcp = Gx50_EDAC_IN8(Gx50_io, XPIXPLLCP);

  KRN_DEBUG(1, "original clock mode saved");

  return KGI_EOK;
}

void Gx50_clock_done(Gx50_clock_t *Gx50, Gx50_clock_io_t *Gx50_io, 
	const kgim_options_t *options) 
{
  KRN_DEBUG(2, "entered.");

  Gx50_EDAC_OUT8(Gx50_io, (XPIXCLKCTRL_PIXCLKSEL_PIXPLL | 
			   XPIXCLKCTRL_PIXCLKDIS | XPIXCLKCTRL_PIXPLLPDN),
		 XPIXCLKCTRL);

  Gx50_EDAC_OUT8(Gx50_io, Gx50->saved.pixcm, XPIXPLLCM);
  Gx50_EDAC_OUT8(Gx50_io, Gx50->saved.pixcn, XPIXPLLCN);
  Gx50_EDAC_OUT8(Gx50_io, Gx50->saved.pixcp, XPIXPLLCP);

  {
    kgi_u_t pixpll_done_lock = PLL_DELAY;
    while (--pixpll_done_lock &&	
	   !(Gx50_EDAC_IN8(Gx50_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    /* Can fail when restoring a power-down mode (e.g. secondary card)
      KRN_ASSERT(pixpll_done_lock);
    */
  }

  Gx50_EDAC_OUT8(Gx50_io, Gx50->saved.pixclkctrl, XPIXCLKCTRL);

  KRN_DEBUG(1, "original clock mode restored and locked");

  KRN_DEBUG(2, "completed.");
}


void Gx50_clock_mode_prepare(Gx50_clock_t *Gx50, Gx50_clock_io_t *Gx50_io,
	Gx50_clock_mode_t *Gx50_mode, kgi_image_mode_t *img, kgi_u_t images) 
{
  KRN_DEBUG(2, "entered");

  KRN_DEBUG(2, "completed");
}

void Gx50_clock_mode_enter(Gx50_clock_t *Gx50, Gx50_clock_io_t *Gx50_io, 
	Gx50_clock_mode_t *Gx50_mode, kgi_image_mode_t *img, kgi_u_t images) 
{
  KRN_DEBUG(2, "entered");

  /*	display must be disabled by chipset driver
   */

  /* Disable the pixel clock and video clock */
  Gx50_EDAC_OUT8(Gx50_io,
		 Gx50_EDAC_IN8(Gx50_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);
  /* Selects the PIXPLL */
  Gx50_EDAC_OUT8(Gx50_io, XPIXCLKCTRL_PIXCLKSEL_PIXPLL
		 | XPIXCLKCTRL_PIXPLLPDN,
		 XPIXCLKCTRL);

  /*	setting pll values
   */
  Gx50_pll_mode_program(&(Gx50_mode->pll_mode), Gx50_io, Gx50_PIXEL_C_PLL);

  /* Select Set C (unused at boot) as the pixel clock source */
  Gx50_CLK_OUT8(Gx50_io, 3, 0); /* clksel = 0x1X, for the clk reg */

  {
    kgi_u_t pixpll_enter_lock = PLL_DELAY;
    while (--pixpll_enter_lock &&
	   !(Gx50_EDAC_IN8(Gx50_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    KRN_ASSERT(pixpll_enter_lock);
  }
  KRN_DEBUG(1, "clock PLL Set C. programmed, locked, and selected");

  /* Enable the pixel clock and video clock */
  Gx50_EDAC_OUT8(Gx50_io,
		 Gx50_EDAC_IN8(Gx50_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);

  /*	display must be reenabled by chipset driver
   */
  KRN_DEBUG(2, "completed");
}

kgi_error_t Gx50_clock_mode_check(Gx50_clock_t *Gx50, Gx50_clock_io_t *Gx50_io,
	Gx50_clock_mode_t *Gx50_mode, kgi_timing_command_t cmd,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	kgi_s_t newfreq = (cmd == KGI_TC_PROPOSE) 
		? 0 : dpm->dclk * dpm->rclk.mul / dpm->rclk.div;
	const kgi_s_t min_freq = Gx50->clock.dclk.range[0].min;
	const kgi_s_t max_freq = Gx50->clock.dclk.range[0].max;

	KRN_DEBUG(2, "Gx50_clock_mode_check()");
	
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

		if (Gx50_pll_best_mode(&(Gx50->pll), newfreq, Gx50_PIXEL_C_PLL,
				       Gx50_io, &(Gx50_mode->pll_mode))) {

			KRN_DEBUG(1, "lower failed: DCLK = %i Hz, CLK = %i Hz",
				dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		dpm->dclk = Gx50_mode->pll_mode.f * dpm->rclk.div / dpm->rclk.mul;
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

		if (Gx50_pll_best_mode(&(Gx50->pll), newfreq, Gx50_PIXEL_C_PLL,
				       Gx50_io, &(Gx50_mode->pll_mode))) {

			KRN_DEBUG(1, "raise failed: DCLK = %i Hz, CLK = %i Hz",
				dpm->dclk, newfreq);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		dpm->dclk = Gx50_mode->pll_mode.f * dpm->rclk.div / dpm->rclk.mul;
		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(2, "KGI_TC_CHECK:");

		if (Gx50_pll_best_mode(&(Gx50->pll), newfreq, Gx50_PIXEL_C_PLL,
				       Gx50_io, &(Gx50_mode->pll_mode))) {

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


