/* ----------------------------------------------------------------------------
**	KGI kernel module internal interface definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**	Copyright (C)	1995-1998	Andreas Beck
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: module.h,v $
**	Revision 1.9  2004/02/15 22:07:48  aldot
**	- redo redefinition-fix properly
**	
**	Revision 1.8  2003/02/09 11:20:09  foske
**	Added DDC2 stuff.
**	
**	Revision 1.7  2003/02/06 09:37:10  foske
**	Dropped VESA standard for faster DDC1 detection. New module option added
**	monitor_ddc: 0 = no ddc, 1 = ddc2 + fast ddc1, 2 = ddc2 + slow ddc1
**	
**	Revision 1.6  2003/01/17 20:36:48  cegger
**	fix redefinition of DEBUG_LEVEL bug. This bug occured, when <kgi/module.h> was included, when DEBUG_LEVEL was declared before.
**	
**	Revision 1.5  2002/12/09 18:24:44  fspacek
**	- added monitor options and ddc io
**	
**	Revision 1.4  2002/10/15 00:34:27  aldot
**	- include cleanup
**	
**	Revision 1.3  2002/09/19 22:37:26  aldot
**	- debugging fixes
**	
**	Revision 1.2  2002/06/11 15:33:41  aldot
**	- spelling fix
**	
**	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:57:33  seeger_s
**	- cleaned up subsystem macros (KGI need not know about subsystems)
**	- added ImageResource function to meta languages
**	
**	Revision 1.1.1.1  2000/04/18 08:51:10  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _kgi_module_h
#define _kgi_module_h

#include <string.h>    /* for modulelib.h in fact, but must happen here, because later the preprocessor is screwed up... */

#define __KGI_STRINGIFY_(s) #s
#define __KGI_STRINGIFY(s) __KGI_STRINGIFY_(s)

/*
**	debugging management
*/

#ifdef	KGIM_CHIPSET_DRIVER
#	ifdef	CONFIG_KGIM_CHIPSET_DEBUG_LEVEL
#		undef DEBUG_LEVEL
#		define	DEBUG_LEVEL	CONFIG_KGIM_CHIPSET_DEBUG_LEVEL
#	endif
#endif

#ifdef	KGIM_RAMDAC_DRIVER
#	ifdef	CONFIG_KGIM_RAMDAC_DEBUG_LEVEL
#		undef DEBUG_LEVEL
#		define	DEBUG_LEVEL	CONFIG_KGIM_RAMDAC_DEBUG_LEVEL
#	endif
#endif

#ifdef	KGIM_CLOCK_DRIVER
#	ifdef	CONFIG_KGIM_CLOCK_DEBUG_LEVEL
#		undef DEBUG_LEVEL
#		define	DEBUG_LEVEL	CONFIG_KGIM_CLOCK_DEBUG_LEVEL
#	endif
#endif

#ifdef	KGIM_MONITOR_DRIVER
#	ifdef	CONFIG_KGIM_MONITOR_DEBUG_LEVEL
#		undef DEBUG_LEVEL
#		define	DEBUG_LEVEL	CONFIG_KGIM_MONITOR_DEBUG_LEVEL
#	endif
#endif

#if (defined(DEBUG_LEVEL) && (DEBUG_LEVEL < 0))
#undef DEBUG_LEVEL
#endif

#define	KGI_SYS_NEED_IO
#include <kgi/kgi.h>

/*
**	subsystems
*/
typedef enum
{
	KGIM_SUBSYSTEM_monitor = 0,
	KGIM_SUBSYSTEM_chipset,
	KGIM_SUBSYSTEM_ramdac,
	KGIM_SUBSYSTEM_clock,

	KGIM_LAST_SUBSYSTEM,
	__KGIM_MAX_NR_SUBSYSTEMS = 8

} kgim_subsystem_type_t;

#define	__KGIM_SUBSYSTEM		\
	kgi_u_t		revision;	/* KGI/driver revision	*/\
	kgi_ascii_t	vendor[64];	/* manufacturer		*/\
	kgi_ascii_t	model[64];	/* model/trademark	*/\
	kgi_u32_t	flags;		/* special capabilities	*/\
	kgi_u_t		mode_size;	/* private mode size	*/

#define	__KGIM_SUBSYSTEM_MODE

typedef struct {

	__KGIM_SUBSYSTEM

} kgim_subsystem_t;


/* -----------------------------------------------------------------------------
**	module internal I/O binding
** -----------------------------------------------------------------------------
*/

#define	KGIM_IO_FN(s)	\
typedef	kgi_u##s##_t kgim_io_in##s##_fn(void *ctx, kgi_u##s##_t reg); \
typedef void kgim_io_out##s##_fn(void *ctx, kgi_u##s##_t val, kgi_size_t reg);\
typedef kgi_u##s##_t kgim_io_get##s##_fn(void *ctx); \
typedef void kgim_io_set##s##_fn(void *ctx, kgi_u##s##_t val);

KGIM_IO_FN(8)
KGIM_IO_FN(16)
KGIM_IO_FN(32)

#undef	KGIM_IO_FN

typedef void kgim_io_ins8_fn(void *ctx, kgi_u8_t reg, void *buf, kgi_u_t count);
typedef void kgim_io_outs8_fn(void *ctx, kgi_u8_t reg, void *buf, kgi_u_t count);
typedef void kgim_io_init_fn(void *ctx);

/* -----------------------------------------------------------------------------
**	monitor subsystem
** -----------------------------------------------------------------------------
*/
#define	KGIM_MONITOR_REVISION	1

typedef enum
{
	KGIM_MF_NORMAL		= 0x00000000,	/* normal monitor	*/
	KGIM_MF_RIGHT_EYE	= 0x00000001,	/* stereo right screen	*/
	KGIM_MF_LEFT_EYE	= 0x00000002,	/* stereo left screen	*/
	KGIM_MF_STEREO		= 0x00000003,	/* is stereo?		*/

	KGIM_MF_POWERSAVE	= 0x00000004,	/* can do powersaving	*/
	KGIM_MF_PROPSIZE	= 0x00000008 	/* size depends on mode */

} kgim_monitor_flags_t;

typedef enum
{
	KGIM_MT_SIGNAL_MASK	= 0x40000000,	/* video signal type	*/
		KGIM_MT_DIGITAL	= 0x00000000,	/* digital input	*/
		KGIM_MT_ANALOG	= 0x40000000,	/* analog input		*/

	KGIM_MT_COLORSPACE_MASK	= 0x1F000000,	/* color spaces		*/
		KGIM_MT_MONO	= 0x00000000,	/* monochrome input	*/
		KGIM_MT_RGB	= 0x00000000,	/* RGB input signal	*/
		KGIM_MT_YUV	= 0x01000000,	/* YUV input signal	*/

	KGIM_MT_TECHNOLOGY_MASK	= 0x000000FF,	/* display technology	*/
		KGIM_MT_CRT	= 0x00000000,	/* Cathod Ray Tube	*/
		KGIM_MT_LCD	= 0x00000001,	/* Liquid Crystal	*/
		KGIM_MT_TFT	= 0x00000002,	/* Thin Film Transistor */
		KGIM_MT_DSTN	= 0x00000003,	/* Double S.T. Nematic?	*/
		KGIM_MT_PLASMA	= 0x00000004	/* Plasma		*/

} kgim_monitor_type_t;

typedef enum
{
	KGIM_ST_SYNC_SEPARATE 	= 0x0001,	/* normal separate sync	*/
	KGIM_ST_SYNC_ON_RED	= 0x0002,	/* sync on r line	*/
	KGIM_ST_SYNC_ON_GREEN	= 0x0004,	/* sync on g line	*/
	KGIM_ST_SYNC_ON_BLUE	= 0x0008,	/* sync on b line	*/
	KGIM_ST_SYNC_SERRATE	= 0x0010,	/* do hsync during vsync*/
	KGIM_ST_SYNC_COMPOSITE	= 0x0020,	/* digital composite	*/
	KGIM_ST_SYNC_TYPEMASK	= 0x003F,
	KGIM_ST_SYNC_NORMAL	= KGIM_ST_SYNC_SERRATE | KGIM_ST_SYNC_SEPARATE,

	KGIM_ST_SYNC_PEDESTRAL	= 0x0020,	/* need blank pedestral	*/
	KGIM_ST_SYNC_0700_0300	= 0x0000,	/* <white>_<sync> level	*/
	KGIM_ST_SYNC_0714_0286	= 0x0040,
	KGIM_ST_SYNC_0100_0400	= 0x0080,
	KGIM_ST_SYNC_UNKNOWNLEVEL = 0x00C0,	/* not defined by VESA!	*/
	KGIM_ST_SYNC_SIGNALLEVEL = 0x00C0,	/* mask for signal level*/

	KGIM_ST_SYNC_STANDBY	= 0x0100,	/* standby mode allowed	*/
	KGIM_ST_SYNC_SUSPEND	= 0x0200,	/* suspend mode allowed */
	KGIM_ST_SYNC_OFF	= 0x0400,	/* switch off allowed	*/
	KGIM_ST_SYNC_VESA_DPMS	= 0x0700,	/* has (full) VESA-DPMS	*/

	KGIM_ST_SYNC_SSYNC_RIGHT = 0x0000,	/* right image on SSYNC	*/
	KGIM_ST_SYNC_SSYNC_LEFT	= 0x1000,	/* left image on SSYNC	*/
	KGIM_ST_SYNC_MULTISYNC	= 0x2000	/* multisync monitor	*/

} kgim_sync_type_t;

#define	KGIM_MONITOR_MAX_HFREQ	8
#define	KGIM_MONITOR_MAX_VFREQ	8

typedef struct
{
	__KGIM_SUBSYSTEM

	kgi_ucoord_t	maxdots;		/* maximum resolution	*/
	kgi_ucoord_t	size;			/* physical size in mm	*/
	kgim_monitor_type_t	type;		/* display type		*/
	kgim_sync_type_t	sync;		/* type of syncing	*/
	kgi_urange_t	dclk;			/* bandwidth		*/
	kgi_urange_t	hfreq[KGIM_MONITOR_MAX_HFREQ];	/* horiz. freq.	*/
	kgi_urange_t	vfreq[KGIM_MONITOR_MAX_VFREQ];	/* vert. freq.	*/

	kgi_color_t	white;			/* white point		*/
	kgi_u_t		gamma;			/* gamma value		*/

} kgim_monitor_t;

/*	We assume a 'standard' CRT here. Most non-CRT displays appear to be
**	compatible anyway.
*/
typedef struct
{
	kgi_u16_t		width;
	kgi_u16_t		blankstart;
	kgi_u16_t		syncstart;
	kgi_u16_t		syncend;
	kgi_u16_t		blankend;
	kgi_u16_t		total;
	kgi_s16_t		polarity;

} kgim_monitor_timing_t;

typedef enum
{
	KGI_MMF_MASK	= 0x00000000

} kgim_monitor_mode_flags_t;

typedef struct
{
	__KGIM_SUBSYSTEM_MODE

	kgi_dot_port_mode_t	in;	/* input port		*/
	kgi_ucoord_t		size;	/* physical size in mm	*/
	kgim_monitor_mode_flags_t flags;/* special info		*/
	kgim_monitor_timing_t	x,y;	/* timing values	*/
	kgim_sync_type_t	sync;	/* sync operation mode	*/

} kgim_monitor_mode_t;

/* -----------------------------------------------------------------------------
**	ramdac subsystem
** -----------------------------------------------------------------------------
*/
#define	KGIM_RAMDAC_REVISION	1

#define	KGIM_DAC_OUT8(ctx, val, reg)	(ctx)->DacOut8((ctx), (val), (reg))
#define	KGIM_DAC_IN8(ctx, reg)		(ctx)->DacIn8((ctx), (reg))
#define	KGIM_DAC_OUTS8(ctx, r, b, c)	(ctx)->DacOuts8((ctx), (r), (b), (c))
#define	KGIM_DAC_INS8(ctx, r, b, c)	(ctx)->DacIns8((ctx), (r), (b), (c))

typedef struct
{
	__KGIM_SUBSYSTEM

	kgi_ucoord_t	maxdots;		/* maximum resolution	*/
	kgi_urange_t	lclk;			/* lclk range		*/
	kgi_urange_t	dclk;			/* dclk range		*/

} kgim_ramdac_t;

typedef struct
{
	__KGIM_SUBSYSTEM_MODE

	const kgim_monitor_mode_t	*crt;

} kgim_ramdac_mode_t;

/* -----------------------------------------------------------------------------
**	clock subsystem
** -----------------------------------------------------------------------------
*/
#define	KGIM_CLOCK_REVISION	1

#define	__KGIM_MAX_NR_FIXED_CLOCKS	16
#define	__KGIM_MAX_NR_PROG_CLOCKS	8

typedef enum
{
	KGIM_CT_FIXED,
	KGIM_CT_PROG

} kgim_clock_type_t;

typedef struct
{
	__KGIM_SUBSYSTEM

	kgim_clock_type_t	type;
	union
	{
		kgi_s_t		freq[__KGIM_MAX_NR_FIXED_CLOCKS];
		kgi_urange_t	range[__KGIM_MAX_NR_PROG_CLOCKS];
	} dclk;

} kgim_clock_t;

typedef struct
{
	__KGIM_SUBSYSTEM_MODE

} kgim_clock_mode_t;

/* -----------------------------------------------------------------------------
**	chipset subsystem
** -----------------------------------------------------------------------------
*/
#define	KGIM_CHIPSET_REVISION	1

typedef struct {
    char *		DevName;

    kgi_u8_t		BitTimeout;	/* usec */
    kgi_u8_t		ByteTimeout;	/* usec */
    kgi_u8_t		AcknTimeout;    /* usec */
    kgi_u8_t 		StartTimeout;	/* usec */

    kgi_u_t		SlaveAddr;
    void	       *pI2CBus;
} I2CDevRec;

typedef struct {
	kgi_error_t	(*WriteRead)(I2CDevRec *, kgi_u8_t*, kgi_u_t, kgi_u8_t *, kgi_u_t);
	
	kgi_error_t	(*PutByte)(I2CDevRec *, kgi_u8_t);
	kgi_error_t	(*GetByte)(I2CDevRec *, kgi_u8_t* , kgi_u8_t);
	kgi_error_t	(*Address)(I2CDevRec *, kgi_u_t);
	kgi_error_t	(*Stop)(I2CDevRec *);

	kgi_error_t	(*PutBits)(void *, kgi_u_t, kgi_u_t);
	kgi_error_t	(*GetBits)(void *, kgi_u_t *, kgi_u_t *);

	void		(*UDelay)(void *, kgi_u_t);
	kgi_u8_t	HoldTime;
	kgi_u8_t	BitTimeout;
	kgi_u8_t	ByteTimeout;
	kgi_u8_t	AcknTimeout;
	kgi_u8_t	StartTimeout;
	kgi_u8_t	RiseFallTime;
	void 		* chipset_io;
} kgim_i2c_io_t ; 

typedef struct
{
	pcicfg_vaddr_t		pcidev;

	kgim_io_out8_fn		*DacOut8;
	kgim_io_in8_fn		*DacIn8;
	kgim_io_outs8_fn	*DacOuts8;
	kgim_io_ins8_fn		*DacIns8;

	kgim_io_out8_fn		*ClkOut8;
	kgim_io_in8_fn		*ClkIn8;

	kgim_io_init_fn		*DDCInit;
	kgim_io_init_fn		*DDCDone;
	kgim_io_set8_fn		*DDCSetSCL;
	kgim_io_get8_fn		*DDCGetSDA;
	kgim_io_set8_fn		*DDCSetVSYNC;
	kgim_io_get8_fn		*DDCGetVSYNC;
	
	kgim_i2c_io_t		DDC2_io;
} kgim_chipset_io_t;

#define	KGIM_PCIDEV(ctx)		(ctx)->pcidev

#define	KGIM_DAC_OUT8(ctx, val, reg)	(ctx)->DacOut8((ctx), (val), (reg))
#define	KGIM_DAC_IN8(ctx, reg)		(ctx)->DacIn8((ctx), (reg))
#define	KGIM_DAC_OUTS8(ctx, r, b, c)	(ctx)->DacOuts8((ctx), (r), (b), (c))
#define	KGIM_DAC_INS8(ctx, r, b, c)	(ctx)->DacIns8((ctx), (r), (b), (c))

#define	KGIM_CLK_OUT8(ctx, val, reg)	(ctx)->ClkOut8((ctx), (val), (reg))
#define	KGIM_CLK_IN8(ctx, reg)		(ctx)->ClkIn8((ctx), (reg))

typedef enum
{
	KGIM_CF_NORMAL = 0

} kgim_chipset_flags_t;

typedef struct
{
	__KGIM_SUBSYSTEM

	kgi_ucoord_t	maxdots;		/* maximum resolution	*/
	kgi_u_t		memory;			/* (total) memory 	*/
	kgi_urange_t	dclk;			/* DCLK range		*/

	kgi_u32_t	vendor_id;		/* (PCI) vendor ID	*/
 	kgi_u32_t	device_id;		/* (PCI) device ID	*/

} kgim_chipset_t;

typedef struct
{
	__KGIM_SUBSYSTEM_MODE

	const kgim_monitor_mode_t	*crt;

} kgim_chipset_mode_t;

/* -----------------------------------------------------------------------------
**	options handling
** -----------------------------------------------------------------------------
*/
typedef struct
{
	pcicfg_vaddr_t	dev;
	mem_paddr_t	base0;
	mem_paddr_t	base1;
	mem_paddr_t	base2;
	mem_paddr_t	base3;
	mem_paddr_t	base4;
	mem_paddr_t	base5;
	mem_paddr_t	base6;
	mem_paddr_t	base7;
	mem_paddr_t	baseR;

} kgim_options_pci_t;

typedef struct
{
	mem_paddr_t	law_base;
	kgi_u_t		display;

} kgim_options_misc_t;

typedef struct
{
	const char	*chipset;
	kgi_u_t		memory;

} kgim_options_chipset_t;

typedef struct
{
	kgi_u_t	lclk_min, lclk_max;
	kgi_u_t	dclk_min, dclk_max;

} kgim_options_ramdac_t;

typedef struct
{
	kgi_u_t	fref;
	kgi_u_t	fvco_min, fvco_max;

} kgim_options_clock_t;

#define KGIM_OPTIONS_MAX_TIMINGS	8
#define KGIM_OPTIONS_MAX_RANGES		8

struct kgim_options_monitor_s;
typedef struct kgim_options_monitor_s kgim_options_monitor_t;

typedef kgi_error_t kgim_monitor_init_module_fn
     (kgim_monitor_t *monitor, const kgim_options_monitor_t *options);

struct kgim_options_monitor_s
{
	kgi_u_t			timings;
	kgim_monitor_timing_t	x[KGIM_OPTIONS_MAX_TIMINGS];
	kgim_monitor_timing_t	y[KGIM_OPTIONS_MAX_TIMINGS];
	
	kgi_u_t			num_hfreq;
	kgi_urange_t		hfreq[KGIM_OPTIONS_MAX_RANGES];
	kgi_u_t			num_vfreq;
	kgi_urange_t		vfreq[KGIM_OPTIONS_MAX_RANGES];
	kgi_u_t			num_dclk;
	kgi_urange_t		dclk[KGIM_OPTIONS_MAX_RANGES];
	kgi_u_t			ddc;

	kgim_sync_type_t	sync;
	kgim_monitor_type_t	type;
	
	kgi_ucoord_t		size;

	kgim_monitor_init_module_fn *init_module;
};

typedef struct
{
	const kgim_options_misc_t	*misc;
	const kgim_options_pci_t	*pci;
	const kgim_options_chipset_t	*chipset;
	const kgim_options_ramdac_t	*ramdac;
	const kgim_options_clock_t	*clock;
	const kgim_options_monitor_t	*monitor;

} kgim_options_t;

#define	KGIM_DEFAULT(option, default) ((option) ? (option) : (default))

/* ----------------------------------------------------------------------------
**	display driver global data
** ----------------------------------------------------------------------------
*/
typedef struct kgim_meta_s kgim_meta_t;

typedef struct
{
	kgi_display_t		kgi;

	kgim_options_t		options;
	kgim_options_misc_t	options_misc;
	kgim_options_pci_t	options_pci;
	kgim_options_chipset_t	options_chipset;
	kgim_options_ramdac_t	options_ramdac;
	kgim_options_clock_t	options_clock;
	kgim_options_monitor_t	options_monitor;

	struct {
		const kgim_meta_t	*meta_lang;
		void			*meta_io;
		void			*meta_data;

	} subsystem[__KGIM_MAX_NR_SUBSYSTEMS];

} kgim_display_t;

typedef struct
{
	void *subsystem_mode[__KGIM_MAX_NR_SUBSYSTEMS];

} kgim_display_mode_t;

/* -----------------------------------------------------------------------------
**	meta-languages
** -----------------------------------------------------------------------------
*/
typedef kgi_error_t kgim_meta_init_module_fn (void *meta, void *meta_io,
		const kgim_options_t *options);

#define	KGIM_META_INIT_MODULE_FN(meta)					\
	kgi_error_t meta##_init_module(meta##_t *, meta##_io_t *,	\
		const kgim_options_t *);


typedef void kgim_meta_done_module_fn(void *meta, void *meta_io,
		const kgim_options_t *options);

#define	KGIM_META_DONE_MODULE_FN(meta)					\
	void meta##_done_module(meta##_t *, meta##_io_t *,		\
		const kgim_options_t *);


typedef kgi_error_t kgim_meta_init_fn(void *meta, void *meta_io,
		const kgim_options_t *options);

#define	KGIM_META_INIT_FN(meta)						\
	kgi_error_t meta##_init(meta##_t *, meta##_io_t *,		\
		const kgim_options_t *);


typedef void kgim_meta_done_fn(void *meta, void *meta_io,
		const kgim_options_t *options);

#define	KGIM_META_DONE_FN(meta)						\
	void meta##_done(meta##_t *, meta##_io_t *,			\
		const kgim_options_t *);


typedef kgi_error_t kgim_meta_mode_check_fn(void *meta, void *meta_io,
		void *meta_mode, kgi_timing_command_t cmd,
		kgi_image_mode_t *img, kgi_u_t images);

#define	KGIM_META_MODE_CHECK_FN(meta)					\
	kgi_error_t meta##_mode_check(meta##_t *, meta##_io_t *,	\
		meta##_mode_t *, kgi_timing_command_t,			\
		kgi_image_mode_t *, kgi_u_t);


typedef kgi_resource_t *kgim_meta_mode_resource_fn(void *meta, void *meta_mode,
		kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index);

#define	KGIM_META_MODE_RESOURCE_FN(meta)				\
	kgi_resource_t *meta##_mode_resource(meta##_t *, meta##_mode_t *,\
		kgi_image_mode_t *, kgi_u_t, kgi_u_t);


typedef void kgim_meta_mode_prepare_fn(void *meta, void *meta_io,
		void *meta_mode, kgi_image_mode_t *img, kgi_u_t images);

#define	KGIM_META_MODE_PREPARE_FN(meta)					\
	void meta##_mode_prepare(meta##_t *, meta##_io_t *,		\
		meta##_mode_t *, kgi_image_mode_t *, kgi_u_t);


typedef void kgim_meta_mode_enter_fn(void *meta, void *meta_io,
		void *meta_mode, kgi_image_mode_t *img, kgi_u_t images);

#define	KGIM_META_MODE_ENTER_FN(meta)					\
	void meta##_mode_enter(meta##_t *, meta##_io_t *,		\
		meta##_mode_t *, kgi_image_mode_t *, kgi_u_t);


typedef void kgim_meta_mode_leave_fn(void *meta, void *meta_io,
		void *meta_mode, kgi_image_mode_t *img, kgi_u_t images);

#define	KGIM_META_MODE_LEAVE_FN(meta)					\
	void meta##_mode_leave(meta##_t *, meta##_io_t *,		\
		meta##_mode_t *, kgi_image_mode_t *, kgi_u_t);

typedef kgi_resource_t *kgim_meta_image_resource_fn(void *meta, void *meta_mode,
		kgi_image_mode_t *img, kgi_u_t image, kgi_u_t index);

#define	KGIM_META_IMAGE_RESOURCE_FN(meta)				\
	kgi_resource_t *meta##_image_resource(meta##_t *, meta##_mode_t *,\
		kgi_image_mode_t *, kgi_u_t, kgi_u_t);
		
		
/*	IRQ handlers are not really part of a meta-language (yet).
**	However, use this macro to get a consistent prototype definition.
*/

#define	KGIM_META_IRQ_HANDLER_FN(meta)					\
	kgi_error_t meta##_irq_handler(meta##_t *, meta##_io_t *,	\
		irq_system_t *);



struct kgim_meta_s
{
	kgim_meta_init_module_fn	*InitModule;
	kgim_meta_done_module_fn	*DoneModule;
	kgim_meta_init_fn		*Init;
	kgim_meta_done_fn		*Done;
	kgim_meta_mode_check_fn		*ModeCheck;
	kgim_meta_mode_resource_fn	*ModeResource;
	kgim_meta_mode_prepare_fn	*ModePrepare;
	kgim_meta_mode_enter_fn		*ModeEnter;
	kgim_meta_mode_leave_fn		*ModeLeave;
	kgim_meta_image_resource_fn	*ImageResource;

	kgi_size_t			data_size;
	kgi_size_t			io_size;
	kgi_size_t			mode_size;
};

#define	KGIM_META(meta) \
	extern const kgim_meta_t meta##_meta;

#include <kgi/modulelib.h>

#endif	/* #ifdef _kgi_module_h */
