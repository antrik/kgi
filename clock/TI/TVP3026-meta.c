/* ----------------------------------------------------------------------------
**	TI TVP3026 PLL implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: TVP3026-meta.c,v $
**	Revision 1.1.1.1  2000/04/26 14:12:18  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1  2000/04/26 14:12:18  seeger_s
**	- added TI TVP3026 driver
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"

#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "ramdac/TI/TVP3026.h"
#include "clock/TI/TVP3026-meta.h"

kgi_error_t tvp3026_clock_init(tvp3026_clock_t *tvp3026, 
	tvp3026_clock_io_t *tvp3026_io, 
	const kgim_options_t *options)
{
	/*	save CS line state
	*/
	tvp3026->ClockSelect = TVP3026_CLK_IN8(tvp3026_io, 0);

	/*	save PLLAddress register and prepare for PLL parameter readout
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PLLAddress, 
		TVP3026_EDAC_ADDR);
	tvp3026->PLLAddress = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);
	TVP3026_DAC_OUT8(tvp3026_io, 0, TVP3026_EDAC_DATA);

	/*	save PClk PLL paramters
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PClkData, TVP3026_EDAC_ADDR);
	tvp3026->PClkN = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);
	tvp3026->PClkM = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);
	tvp3026->PClkP = TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA);
	
	/*	restore PLLAddress register
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PLLAddress, 
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026->PLLAddress, TVP3026_EDAC_DATA);

	return KGI_EOK;
}

void tvp3026_clock_done(tvp3026_clock_t *tvp3026,
	tvp3026_clock_io_t *tvp3026_io, 
	const kgim_options_t *options)
{
	/*	prepare PLLAddress register to restore PClk PLL paramters
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PLLAddress,
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, 0, TVP3026_EDAC_DATA);

	/*	restore PClk PLL paramters
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PClkData, TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026->PClkN, TVP3026_EDAC_DATA);
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026->PClkM, TVP3026_EDAC_DATA);
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026->PClkP, TVP3026_EDAC_DATA);

	/*	restore PLL Address register
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PLLAddress, 
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, tvp3026->PLLAddress, TVP3026_EDAC_DATA);

	/*	restore Clock Select lines
	*/
	TVP3026_CLK_OUT8(tvp3026_io, tvp3026->ClockSelect, 0);
}

void tvp3026_clock_mode_enter(tvp3026_clock_t *tvp3026,
	tvp3026_clock_io_t *tvp3026_io,
	tvp3026_clock_mode_t *tvp3026_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u8_t PClkM, PClkN, PClkP;

	PClkM = 65 - tvp3026_mode->pll.mul;
	PClkN = 65 - tvp3026_mode->pll.div;
	PClkP = tvp3026_mode->pll.p;

	KRN_ASSERT((40 <= PClkN) && (PClkN <= 62));
	KRN_ASSERT(( 1 <= PClkM) && (PClkM <= 62));
	KRN_ASSERT(PClkP <= 3);

	/*	select PClk PLL as clock source
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_ClockSelect,
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC1A_ClkPClkPLL |
		(TVP3026_DAC_IN8(tvp3026_io, TVP3026_EDAC_DATA) & 
			~TVP3026_EDAC1A_ClkMask),
		TVP3026_EDAC_DATA);

	/*	disable PClk PLL
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PLLAddress,
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, 
		(TVP3026_PLL_P << TVP3026_EDAC2C_PClkShift) |
			(TVP3026_PLL_P << TVP3026_EDAC2C_LClkShift),
		TVP3026_EDAC_DATA);
	
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PClkData,
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, 0, TVP3026_EDAC_DATA);

	/*	select PLL on CS address lines
	*/
	TVP3026_CLK_OUT8(tvp3026_io, 3, 0);

	/*	set PClk N, M, and P values
	*/
	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PLLAddress,
		TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, 0, TVP3026_EDAC_DATA);

	TVP3026_DAC_OUT8(tvp3026_io, TVP3026_EDAC_PClkData, TVP3026_EDAC_ADDR);
	TVP3026_DAC_OUT8(tvp3026_io, PClkN | TVP3026_PClkN_Bias, 
		TVP3026_EDAC_DATA);
	TVP3026_DAC_OUT8(tvp3026_io, PClkM | TVP3026_PClkM_Bias, 
		TVP3026_EDAC_DATA);
	TVP3026_DAC_OUT8(tvp3026_io, PClkP | TVP3026_PClkP_Bias |
		TVP3026_PClkP_PClkEnable | TVP3026_PClkP_PLLEnable,
		TVP3026_EDAC_DATA);

	/*	LClk is handled by the DAC driver
	*/
}
