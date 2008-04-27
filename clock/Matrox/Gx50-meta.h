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
**	MAINTAINER	Rodolphe_Ortalo
**
**	$Id: Gx50-meta.h,v 1.1 2002/11/30 18:23:14 ortalo Exp $
*/

#ifndef _clock_Matrox_Gx50_meta_h
#define	_clock_Matrox_Gx50_meta_h

typedef kgim_chipset_io_t Gx50_clock_io_t;

#define Gx50_PCIDEV(ctx)		KGIM_PCIDEV(ctx)

#define Gx50_DAC_OUT8(ctx, val, reg)	KGIM_DAC_OUT8(ctx, val, reg)
#define Gx50_DAC_IN8(ctx, reg)		KGIM_DAC_IN8(ctx, reg)
#define Gx50_CLK_OUT8(ctx, val, reg)	KGIM_CLK_OUT8(ctx, val, reg)
#define Gx50_CLK_IN8(ctx, val, reg)	KGIM_CLK_IN8(ctx, reg)

/*
**	IO helpers
*/
#include "chipset/Matrox/Gx00.h"

static inline void Gx50_EDAC_OUT8(Gx50_clock_io_t *Gx50_io, kgi_u8_t val,
				  kgi_u8_t reg)
{
  KRN_DEBUG(3, "EDAC[%.2x] <= %.2x", reg, val);
  Gx50_DAC_OUT8(Gx50_io, reg, PALWTADD);
  Gx50_DAC_OUT8(Gx50_io, val, X_DATAREG);
}

static inline kgi_u8_t Gx50_EDAC_IN8(Gx50_clock_io_t *Gx50_io, kgi_u8_t reg)
{
  kgi_u8_t val;
  Gx50_DAC_OUT8(Gx50_io, reg, PALWTADD);
  val = Gx50_DAC_IN8(Gx50_io, X_DATAREG);
  KRN_DEBUG(3, "EDAC[%.2x] => %.2x", reg, val);
  return val;
}

/*
** Gx50 PLL-specific include
*/
#define MGAG_EDAC_OUT8(ctx,val,reg)   Gx50_EDAC_OUT8(ctx,val,reg)
#define MGAG_EDAC_IN8(ctx,reg)        Gx50_EDAC_IN8(ctx,reg)
#include "chipset/Matrox/Gx50-pll.inc"
#undef MGAG_EDAC_OUT8
#undef MGAG_EDAC_IN8

/*
** Data-structures definition
*/

typedef struct {

	kgim_clock_mode_t	kgim;

	Gx50_pll_mode_t		pll_mode;

} Gx50_clock_mode_t;

typedef enum {

	/* chipsets */

	Gx50_CF_G450	= (0x01 << 1),
	Gx50_CF_G550	= (0x01 << 2),


} Gx50_clock_flags_t;

typedef struct {

	kgim_clock_t	clock;

	Gx50_pll_t	pll;

	Gx50_clock_flags_t	flags;

	struct {

		kgi_u8_t	pixclkctrl,
				pixcm,
				pixcn,
				pixcp;
	} saved;

} Gx50_clock_t;

KGIM_META_INIT_FN(Gx50_clock)
KGIM_META_DONE_FN(Gx50_clock)
KGIM_META_MODE_PREPARE_FN(Gx50_clock)
KGIM_META_MODE_CHECK_FN(Gx50_clock)
KGIM_META_MODE_ENTER_FN(Gx50_clock)

#endif	/* #ifndef _clock_Matrox_Gx50_meta_h	*/
