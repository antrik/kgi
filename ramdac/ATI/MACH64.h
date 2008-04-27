/* ----------------------------------------------------------------------------
**	ATI MACH64 register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64.h,v $
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.1.1.1  2001/09/16 21:51:14  phil
**	Initial import
**	
**	Revision 1.1  2001/08/17 01:17:59  phil
**	Template generated code for ATI ramdac
**	
**	Revision 1.1.1.1  2000/04/18 08:51:12  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_ramdac_ATI_MACH64_h
#define	_ramdac_ATI_MACH64_h

/***	We assume a VGA compatible DAC here. If is not, change the 'base'
****	register definitions below accordingly.
***/
#warning check/add definitions for direct DAC registers here.
#define MACH64_DAC_PW_INDEX		0x0
#define MACH64_DAC_P_DATA		0x1
#define MACH64_DAC_PIXEL_MASK		0x2
#define MACH64_DAC_PR_INDEX		0x3

#define MACH64_DAC_EXT_ADDR		0x0
#define MACH64_DAC_CW_INDEX		0x4
#define MACH64_DAC_C_DATA		0x5

/***	Add any indexed registers below.
****
***/
#warning check/add definitions for indirect DAC registers here.

#endif	/* #ifdef _ramdac_ATI_MACH64_h	*/
