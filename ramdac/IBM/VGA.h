/* ---------------------------------------------------------------------------
**	VGA ramdac register definitions
** ---------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: VGA.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _ramdac_IBM_vga_h
#define _ramdac_IBM_vga_h

#define	VGA_DAC_PW_INDEX	0x0	/* palette write index register */
#define	VGA_DAC_P_DATA		0x1	/* palette data (r/w) register  */
#define	VGA_DAC_PIXEL_MASK	0x2	/* pixel read mask register     */
#define	VGA_DAC_PR_INDEX	0x3	/* palette read index register  */

#endif	/* #ifndef _ramdac_IBM_vga_h */
