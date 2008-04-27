/* ---------------------------------------------------------------------------
**	DDC monitor driver meta language definition
** ---------------------------------------------------------------------------
**	Copyright (C)	2001	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTAINER	Jos_Hulzink
*/
#ifndef	_monitor_VESA_DDC_meta_h
#define	_monitor_VESA_DDC_meta_h

/*
**	timing definitions
*/
#define	TIMELIST_HPOS	0x1000
#define	TIMELIST_HNEG	0x0000
#define	TIMELIST_VPOS	0x2000
#define	TIMELIST_VNEG	0x0000
#define	TIMELIST_POLARITY_MASK	0x3000

#define TIMELIST_HNVN	(TIMELIST_HNEG | TIMELIST_VNEG)
#define TIMELIST_HNVP	(TIMELIST_HNEG | TIMELIST_VPOS)
#define TIMELIST_HPVN	(TIMELIST_HPOS | TIMELIST_VNEG)
#define TIMELIST_HPVP	(TIMELIST_HPOS | TIMELIST_VPOS)

typedef kgi_u16_t edid_data_t[1024];

typedef struct
{
	kgi_ascii_t			name[8];

	kgi_u_t				htimings;
	const struct {
		kgi_u_t 		dclk;
		kgim_monitor_timing_t	tm;
	}				*htiming;
	kgi_u_t				vtimings;
	const kgim_monitor_timing_t	*vtiming;

} DDC_timing_t;

typedef struct
{
	kgim_monitor_mode_t	kgim;
	const DDC_timing_t	*timing;

} DDC_monitor_mode_t;

#define DMD_RANGE_MAX_N 4
#define DMD_FIXED_MAX_N 128

typedef struct
{
	char id[128];
	struct {
		kgi_u_t n;
		struct {
			kgi_urange_t hclk,vclk,pclk;
		} data [DMD_RANGE_MAX_N];
	} range;
	struct {
		kgi_u_t n;
		struct {
			kgi_u_t x,y,r;
		} data [DMD_FIXED_MAX_N];
	} fixed;
} DDC_monitor_data_t;

typedef struct {
} DDC_monitor_io_t;

typedef struct
{
	kgim_monitor_t		monitor;
	const DDC_timing_t	*timing;

} DDC_monitor_t;

KGIM_META_INIT_FN(DDC_monitor)
KGIM_META_MODE_CHECK_FN(DDC_monitor)

#endif	/* #ifndef _monitor_VESA_DDC_meta_h */
