/* ---------------------------------------------------------------------------
**	generic fixed frequency monitor driver meta language definition
** ---------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTIANER	Steffen_Seeger
**
**	$Log: monosync-meta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:08  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_monitor_monosync_monosync_meta_h
#define	_monitor_monosync_monosync_meta_h

/*
**	timing definitions
*/
#define	MONOSYNC_HPOS	0x1000
#define	MONOSYNC_HNEG	0x0000
#define	MONOSYNC_VPOS	0x2000
#define	MONOSYNC_VNEG	0x0000
#define	MONOSYNC_POLARITY_MASK	0x3000

#define MONOSYNC_HNVN	(MONOSYNC_HNEG | MONOSYNC_VNEG)
#define MONOSYNC_HNVP	(MONOSYNC_HNEG | MONOSYNC_VPOS)
#define MONOSYNC_HPVN	(MONOSYNC_HPOS | MONOSYNC_VNEG)
#define MONOSYNC_HPVP	(MONOSYNC_HPOS | MONOSYNC_VPOS)

typedef struct
{
	kgi_ascii_t			name[8];

	kgi_u_t				htimings;
	const kgim_monitor_timing_t	*htiming;
	kgi_u_t				vtimings;
	const kgim_monitor_timing_t	*vtiming;

} monosync_timing_t;

typedef struct
{
	kgim_monitor_mode_t	kgim;
	const monosync_timing_t	*timing;

} monosync_monitor_mode_t;

typedef struct
{
} monosync_monitor_io_t;

typedef struct
{
	kgim_monitor_t		monitor;
	const monosync_timing_t	*timing;

} monosync_monitor_t;

KGIM_META_INIT_FN(monosync_monitor)
KGIM_META_MODE_CHECK_FN(monosync_monitor)

#endif	/* #ifndef _monitor_monosync_monosync_meta_h */
