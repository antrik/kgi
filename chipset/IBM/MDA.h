/* ----------------------------------------------------------------------------
**	MDA register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1996-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: MDA.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:18  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:18  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _IBM_mda_h
#define _IBM_mda_h

#define	CRTIm	0x3B4
#define	CRTDm	0x3B5

#define CTRLm	0x3B8
#	define	CTRL_ENB_CRT	0x08
#	define	CTRL_ENB_BLINK	0x20

#define	STATm	0x3BA
#	define	STAT_HSYNC	0x01
#	define	STAT_VIDEO_ENB	0x08

#	define	CRT_HTOTAL	0x00
#	define	CRT_HDISPLAYEND	0x01
#	define	CRT_HSYNCSTART	0x02
#	define	CRT_HSYNCEND	0x03
#	define	CRT_VTOTAL	0x04
#	define	CRT_VSKEW	0x05
#	define	CRT_VDISPLAYEND	0x06
#	define	CRT_VSYNCSTART	0x07
#	define	CRT_INTERLACE   0x08
#		define CR08_NON_INTERLACED    0x02
#		define CR08_INTERLACED        0x01
#		define CR08_INTERLACED_VIDEO  0x03
#	define	CRT_CHAR_HEIGHT	0x09
#	define	CRT_CURSORSTART	0x0A
#	define	CRT_CURSOREND	0x0B
#	define	CRT_STARTADDR_H	0x0C
#	define	CRT_STARTADDR_L	0x0D
#	define	CRT_CURSORADDR_H 0x0E
#	define	CRT_CURSORADDR_L 0x0F

#define	MDA_IOBASE	0x03B0
#define	MDA_IOSIZE	0x0010
#define	MDA_FBBASE	0x000B0000
#define	MDA_FBSIZE	0x1000

#endif /* #ifdef _IBM_mda_h */
