/* -----------------------------------------------------------------------------
**	S3 IDAC PLL meta language
** -----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** -----------------------------------------------------------------------------
**
**	$Log: IDAC-meta.c,v $
**	Revision 1.1.1.1  2000/08/04 11:51:37  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2000/08/04 11:51:37  seeger_s
**	- merged driver posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- driver status 30% reported by Jos Hulzink
**	
**	Revision 1.1.1.1  2000/04/18 08:51:14  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CLOCK_DRIVER	"$Revision: 1.1.1.1 $"
#define DEBUG_LEVEL 255

#include <kgi/module.h>
#include "clock/S3/IDAC-meta.h"

kgi_error_t idac_clock_init(idac_clock_t *idac, idac_clock_io_t *idac_io,
	const kgim_options_t *options)
{
	return KGI_EOK;
}

void idac_clock_done(idac_clock_t *idac, idac_clock_io_t *idac_io,
	const kgim_options_t *options)
{
}

void idac_clock_mode_enter(idac_clock_t *idac, idac_clock_io_t *idac_io,
	idac_clock_mode_t *idac_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t cnt;

	KRN_ASSERT(idac_mode->pll.mul < 128);
	KRN_ASSERT(idac_mode->pll.mul > 1);
	KRN_ASSERT(idac_mode->pll.div < 32);
	KRN_ASSERT(idac_mode->pll.div > 1 );
	KRN_ASSERT(idac_mode->pll.p   < 4);
	KRN_DEBUG (1,"S3 IDAC mode setup: m: %d, n: %d, r: %d",
			idac_mode->pll.mul, idac_mode->pll.div,
			idac_mode->pll.p);
	
	IDAC_CLK_OUT8(idac_io,3,0); /* Enable programmable clock */
	IDAC_CLK_OUT8(idac_io,(idac_mode->pll.p<<5) | (idac_mode->pll.div-2),3);
	IDAC_CLK_OUT8(idac_io,(idac_mode->pll.mul-2),4);
	IDAC_CLK_OUT8(idac_io,0,5); /* Normal clock operation (no testing)*/
	IDAC_CLK_OUT8(idac_io,2,6); /* Load new DCLK value */
}

void idac_clock_mode_leave(idac_clock_t *idac, idac_clock_io_t *idac_io,
	idac_clock_mode_t *idac_mode, kgi_image_mode_t *img, kgi_u_t images)
{
}
