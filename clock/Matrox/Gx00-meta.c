/* ----------------------------------------------------------------------------
**	Matrox Gx00 clock driver meta
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2002	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx00-meta.c,v 1.2 2002/11/30 18:23:14 ortalo Exp $
*/

#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.2 $"

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#include <kgi/module.h>

#include "chipset/Matrox/Gx00.h"
#include "clock/Matrox/Gx00-meta.h"
#if 0
/*
**	IO helpers
*/

static inline void MGAG_EDAC_OUT8(mgag_clock_io_t *mgag_io, kgi_u8_t val,
				  kgi_u8_t reg)
{
  KRN_DEBUG(3, "EDAC[%.2x] <= %.2x", reg, val);
  MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
  MGAG_DAC_OUT8(mgag_io, val, X_DATAREG);
}

static inline kgi_u8_t MGAG_EDAC_IN8(mgag_clock_io_t *mgag_io, kgi_u8_t reg)
{
  kgi_u8_t val;
  MGAG_DAC_OUT8(mgag_io, reg, PALWTADD);
  val = MGAG_DAC_IN8(mgag_io, X_DATAREG);
  KRN_DEBUG(3, "EDAC[%.2x] => %.2x", reg, val);
  return val;
}
#endif
/*	KGI interface
*/

kgi_error_t mgag_clock_init(mgag_clock_t *mgag,	mgag_clock_io_t *mgag_io,
	const kgim_options_t *options) 
{
  mgag->saved.pixclkctrl = MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL);
  mgag->saved.pixcm = MGAG_EDAC_IN8(mgag_io, XPIXPLLCM);
  mgag->saved.pixcn = MGAG_EDAC_IN8(mgag_io, XPIXPLLCN);
  mgag->saved.pixcp = MGAG_EDAC_IN8(mgag_io, XPIXPLLCP);

  KRN_DEBUG(1, "original clock mode saved");

  return KGI_EOK;
}

void mgag_clock_done(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io, 
	const kgim_options_t *options) 
{
  KRN_DEBUG(2, "entered.");

  MGAG_EDAC_OUT8(mgag_io, (XPIXCLKCTRL_PIXCLKSEL_PIXPLL | 
			   XPIXCLKCTRL_PIXCLKDIS | XPIXCLKCTRL_PIXPLLPDN),
		 XPIXCLKCTRL);

  MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixcm, XPIXPLLCM);
  MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixcn, XPIXPLLCN);
  MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixcp, XPIXPLLCP);

  {
    kgi_u_t pixpll_done_lock = PLL_DELAY;
    while (--pixpll_done_lock &&	
	   !(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    /* Can fail when restoring a power-down mode (e.g. secondary card)
      KRN_ASSERT(pixpll_done_lock);
    */
  }

  MGAG_EDAC_OUT8(mgag_io, mgag->saved.pixclkctrl, XPIXCLKCTRL);

  KRN_DEBUG(1, "original clock mode restored and locked");

  KRN_DEBUG(2, "completed.");
}


void mgag_clock_mode_prepare(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io,
	mgag_clock_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images) 
{
  KRN_DEBUG(2, "entered");

  /*	Get loop filter value (s) - 1x64 and G200 uses the same values
   */

  if (mgag->flags & MGAG_CLOCK_G400) {

    if ((mgag_mode->pll.fvco > 50 MHZ) && 
	(mgag_mode->pll.fvco < 110 MHZ)) {
      
      mgag_mode->s = 0;
      
    } else if (mgag_mode->pll.fvco < 170 MHZ) {

      mgag_mode->s = 1;

    } else if (mgag_mode->pll.fvco < 240 MHZ) {

      mgag_mode->s = 2;
      
    } else if (mgag_mode->pll.fvco < mgag->pll.fvco.max) {

      mgag_mode->s = 3;

    } else {

      KRN_INTERNAL_ERROR;
    }

  } else {

    if ((mgag_mode->pll.fvco > 50 MHZ) && 
	(mgag_mode->pll.fvco < 100 MHZ)) {
      
      mgag_mode->s = 0;

    } else if (mgag_mode->pll.fvco < 140 MHZ) {

      mgag_mode->s = 1;

    } else if (mgag_mode->pll.fvco < 180 MHZ) {

      mgag_mode->s = 2;

    } else if (mgag_mode->pll.fvco < mgag->pll.fvco.max) {

      mgag_mode->s = 3;

    } else {

      KRN_INTERNAL_ERROR;
    }
  }

  KRN_DEBUG(2, "completed");
}

void mgag_clock_mode_enter(mgag_clock_t *mgag, mgag_clock_io_t *mgag_io, 
	mgag_clock_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images) 
{
  kgi_u_t actual_p;

  KRN_DEBUG(2, "entered");

  /*	display must be disabled by chipset driver
   */

  /* Disable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);
  /* Selects the PIXPLL */
  MGAG_EDAC_OUT8(mgag_io, XPIXCLKCTRL_PIXCLKSEL_PIXPLL
		 | XPIXCLKCTRL_PIXPLLPDN,
		 XPIXCLKCTRL);

  /*	setting pll values
   */
  actual_p = ( (0x01 << (mgag_mode->pll.p)) - 1);
  KRN_DEBUG(1,"programming the PIXPLL Set C with mul=%i div=%i p=%i s=%i (P=%i)",
	    mgag_mode->pll.mul, mgag_mode->pll.div,
	    mgag_mode->pll.p, mgag_mode->s, actual_p);
      
  MGAG_EDAC_OUT8(mgag_io, ((mgag_mode->pll.mul - 1) & XPIXPLLN_MASK),
		 XPIXPLLCN);
  MGAG_EDAC_OUT8(mgag_io, ((mgag_mode->pll.div - 1) & XPIXPLLM_MASK),
		 XPIXPLLCM);
  MGAG_EDAC_OUT8(mgag_io, ((actual_p & XPIXPLLP_PIXPLLP_MASK) |
			   (mgag_mode->s << XPIXPLLP_PIXPLLS_SHIFT))
		 /* & XPIXPLLP_MASK */,
		 XPIXPLLCP);

  /* Select Set C (unused at boot) as the pixel clock source */
  MGAG_CLK_OUT8(mgag_io, 3, 0); /* clksel = 0x1X, for the clk reg */

  {
    kgi_u_t pixpll_enter_lock = PLL_DELAY;
    while (--pixpll_enter_lock &&
	   !(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    KRN_ASSERT(pixpll_enter_lock);
  }

  KRN_DEBUG(1, "clock PLL Set C. programmed, locked, and selected");

  /* Enable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);

  /*	display must be reenabled by chipset driver
   */
  KRN_DEBUG(2, "completed");
}
