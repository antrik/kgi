/* ---------------------------------------------------------------------------
**	generic crt monitor driver meta language definition
** ---------------------------------------------------------------------------
**	Copyright (C)	2002		Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTIANER	Filip_Spacek
**
**	$Log: crt-meta.h,v $
**	Revision 1.2  2003/02/02 22:49:34  cegger
**	fix warnings about implicit function calls
**	
**	Revision 1.1  2002/12/09 18:20:28  fspacek
**	- initial version of the unified monitor driver
**	
**	
*/
#ifndef	_monitor_crt_crt_meta_h
#define	_monitor_crt_crt_meta_h

/*
**	timing definitions
*/
#define	CRT_HPOS		0x1
#define	CRT_HNEG		0x0
#define	CRT_VPOS		0x1
#define	CRT_VNEG		0x0

typedef struct
{
	char *				name;
	kgim_monitor_timing_t		x, y;
	kgi_u_t				dclk;

} crt_monitor_timing_t;

typedef struct
{
	kgi_ascii_t			name[8];
	
	kgi_u_t				num;
	const crt_monitor_timing_t	*t;
	
} crt_monitor_timing_set_t;

/*
**	IO structure is that of the chipset
*/
typedef kgim_chipset_io_t crt_monitor_io_t;

#define CRT_MONITOR_DDC_INIT(ctx)		((ctx)->DDCInit((ctx)))
#define CRT_MONITOR_DDC_DONE(ctx)		((ctx)->DDCDone((ctx)))
#define CRT_MONITOR_DDC_SET_SCL(ctx, val)	((ctx)->DDCSetSCL((ctx),(val)))
#define CRT_MONITOR_DDC_GET_SDA(ctx)		((ctx)->DDCGetSDA((ctx)))
#define CRT_MONITOR_DDC_SET_VSYNC(ctx, val)	((ctx)->DDCSetVSYNC((ctx),(val)))
#define CRT_MONITOR_DDC_GET_VSYNC(ctx)		((ctx)->DDCGetVSYNC((ctx)))

typedef struct
{
	kgim_monitor_mode_t	kgim;

} crt_monitor_mode_t;


#define CRT_MONITOR_MAX_FREQ_RANGES	(KGIM_OPTIONS_MAX_RANGES + 8)
#define CRT_MONITOR_MAX_FIXED_TIMINGS 	(KGIM_OPTIONS_MAX_TIMINGS + 64)

typedef struct
{
	kgim_monitor_t			kgim;
	
	kgi_u_t				num_fixed;
	const crt_monitor_timing_t	*fixed[CRT_MONITOR_MAX_FIXED_TIMINGS];

	kgi_u_t				num_vfreq;
	kgi_urange_t			vfreq[CRT_MONITOR_MAX_FREQ_RANGES];
	kgi_u_t				num_hfreq;
	kgi_urange_t			hfreq[CRT_MONITOR_MAX_FREQ_RANGES];
	kgi_u_t				num_dclk;
	kgi_urange_t			dclk[CRT_MONITOR_MAX_FREQ_RANGES];

} crt_monitor_t;


void crt_monitor_fixed_configuration(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, const kgim_options_t *options);
kgi_error_t crt_monitor_fixed_propose(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode);
kgi_error_t crt_monitor_fixed_adjust(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode);

kgi_error_t crt_monitor_gtf_propose(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode);
kgi_error_t crt_monitor_gtf_adjust(crt_monitor_t *crt,
	crt_monitor_io_t *crt_io, crt_monitor_mode_t *crt_mode);

KGIM_META_INIT_FN(crt_monitor)
KGIM_META_MODE_CHECK_FN(crt_monitor)

#endif	/* #ifndef _monitor_crt_crt_meta_h */
