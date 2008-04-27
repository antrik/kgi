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
**	MAINTAINER	Rodolphe_Ortalo
**
**	$Id: Gx00-meta.h,v 1.2 2002/11/30 18:23:14 ortalo Exp $
*/

#ifndef _clock_Matrox_Gx00_meta_h
#define	_clock_Matrox_Gx00_meta_h

#include "clock/pll-meta.h"

/* TODO: The meta language prefix is should be GX00_ and gx00_.
 * TODO: But this will be cumbersome to adapt to. -- ortalo
 */
#if 1
#include "chipset/Matrox/Gx00-meta.h"
typedef mgag_chipset_io_t mgag_clock_io_t;
#define MGAG_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(&(ctx->vga.kgim), val, reg)
#define MGAG_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(&(ctx->vga.kgim), reg)
#define MGAG_CLK_OUT8(ctx, val, reg)	KGIM_CLK_OUT8(&(ctx->vga.kgim), val, reg)
#define MGAG_CLK_IN8(ctx, val, reg)	KGIM_CLK_IN8(&(ctx->vga.kgim), reg)
#else
typedef pll_clock_io_t mgag_clock_io_t;

#define MGAG_PCIDEV(ctx)		KGIM_PCIDEV(ctx)

#define MGAG_DAC_OUT8(ctx, val, reg)	PLL_DAC_OUT8(ctx, val, reg)
#define MGAG_DAC_IN8(ctx, reg)		PLL_DAC_IN8(ctx, reg)
#define MGAG_CLK_OUT8(ctx, val, reg)	PLL_CLK_OUT8(ctx, val, reg)
#define MGAG_CLK_IN8(ctx, val, reg)	PLL_CLK_IN8(ctx, reg)
#endif

typedef struct {

	pll_clock_mode_t	pll;

	kgi_u_t			s;	/* Loop filter value */

} mgag_clock_mode_t;

typedef enum {

	/* chipsets */

	MGAG_CLOCK_1x64	= (0x01 << 1),
	MGAG_CLOCK_G200	= (0x01 << 2),
	MGAG_CLOCK_G400	= (0x01 << 3),

	/* capabilities */

	MGAG_CLOCK_SGRAM = (0x01 << 4)

} mgag_clock_flags_t;

typedef struct {

	pll_clock_t		pll;

	mgag_clock_flags_t	flags;

	struct {

		kgi_u8_t	pixclkctrl,
				pixcm,
				pixcn,
				pixcp;
	} saved;

} mgag_clock_t;

KGIM_META_INIT_FN(mgag_clock)
KGIM_META_DONE_FN(mgag_clock)
KGIM_META_MODE_PREPARE_FN(mgag_clock)
KGIM_META_MODE_ENTER_FN(mgag_clock)

#endif	/* #ifndef _clock_Matrox_Gx00_meta_h	*/
