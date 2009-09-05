/* ----------------------------------------------------------------------------
**	KGI manager OS kernel dependent stuff
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: kgi.h,v $
**	Revision 1.11  2004/02/15 22:08:54  aldot
**	- put include inside guard
**	
**	Revision 1.10  2003/11/16 21:46:15  aldot
**	- add another attribute string
**	
**	Revision 1.9  2003/01/16 02:16:48  fspacek
**	- new display event type
**	
**	Revision 1.8  2002/12/04 18:16:01  foske
**	Fixed missing include file
**	
**	Revision 1.7  2002/10/15 00:02:43  aldot
**	- include cleanup
**	
**	Revision 1.6  2002/10/10 21:18:28  redmondp
**	- clean up clut set command
**	- add new get clut info command
**	
**	Revision 1.5  2002/09/23 20:37:28  aldot
**	- yet another attribute string
**	
**	Revision 1.4  2002/09/22 01:45:14  fspacek
**	One more attribute string
**	
**	Revision 1.3  2002/07/28 17:48:24  aldot
**	- back out old args
**	
**	Revision 1.2  2002/07/27 00:53:11  aldot
**	- typo fix (closes severe bug)
**	- add some documentation; whitespace cleanup
**	
**	Revision 1.1.1.1  2001/10/01 14:16:11  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/09/17 20:34:35  seeger_s
**	- restructured kgi_accel_buffer_s to make driver-owned state more obvious
**	
**	Revision 1.2  2001/07/03 08:47:52  seeger_s
**	- image resources added
**	- minor clean ups and reordering
**	
**	Revision 1.1.1.1  2000/04/18 08:50:43  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#ifndef _kgi_kgi_h
#define _kgi_kgi_h

#include <kgi/config.h>

#include <kgi/system.h>
KGI_SYS_DECLARE_INTEGRAL_TYPES(kgi)

typedef void *kgi_wait_queue_t;

#include <kgi/errno.h>

#include <kgi/debug.h>

#ifdef __KERNEL__
/* TODO	Later extend to full mutex handling:
**
**	typedef ... kgi_mutex;
**	void kgi_mutex_init(kgi_mutex *m);
**	void kgi_mutex_done(kgi_mutex *m);
**	void kgi_mutex_lock(kgi_mutex *m);
**	int kgi_mutex_trylock(kgi_mutex *m);
**	void kgi_mutex_unlock(kgi_mutex *m);
*/
#	define	KGI_IS_CRITICAL		kgi_is_critical __kgi_is_critical
#	define	KGI_ENTER_CRITICAL	kgi_enter_critical(&__kgi_is_critical)
#	define	KGI_LEAVE_CRITICAL	kgi_leave_critical(&__kgi_is_critical)
#endif

typedef struct { kgi_u8_t major, minor, patch, extra; } kgi_version_t;
typedef struct { kgi_s_t x,y; }		kgi_scoord_t;
typedef struct { kgi_u_t x,y; }		kgi_ucoord_t;
typedef struct { kgi_u8_t x,y; }	kgi_u8_coord_t;
typedef struct { kgi_s_t min, max; }	kgi_srange_t;
typedef struct { kgi_u_t min, max; }	kgi_urange_t;

typedef struct { kgi_u_t mul, div; }	kgi_ratio_t;

typedef struct { kgi_u16_t r,g,b; }	kgi_rgb_color_t;
typedef union {

	kgi_rgb_color_t	rgb;

} kgi_color_t;


#define	KGI_MAX_NR_FOCUSES	CONFIG_KGII_MAX_NR_FOCUSES
#define	KGI_MAX_NR_CONSOLES	CONFIG_KGII_MAX_NR_CONSOLES
#define	KGI_MAX_NR_DEVICES	CONFIG_KGII_MAX_NR_DEVICES
#define	KGI_MAX_NR_DISPLAYS	CONFIG_KGII_MAX_NR_DISPLAYS

#define	KGI_VALID_FOCUS_ID(x)	((x) < KGI_MAX_NR_FOCUSES)
#define	KGI_VALID_CONSOLE_ID(x)	((x) < KGI_MAX_NR_CONSOLES)
#define	KGI_VALID_DEVICE_ID(x)	((x) < KGI_MAX_NR_DEVICES)
#define	KGI_VALID_DISPLAY_ID(x)	((x) < KGI_MAX_NR_DISPLAYS)

#define	KGI_INVALID_FOCUS	INVALID_FOCUS
#define	KGI_INVALID_CONSOLE	INVALID_CONSOLE
#define	KGI_INVALID_DISPLAY	INVALID_DISPLAY
#define	KGI_INVALID_DEVICE	INVALID_DEVICE

/* ----------------------------------------------------------------------------
**	mode handling
** ----------------------------------------------------------------------------
*/

/* 	Attributes
** NOTE	Attribute ordering __must__ not be changed! 
*/
typedef enum
{
	KGI_A_COLOR1,		/* intensity of color channel 1	*/
	KGI_A_COLOR2,		/* intensity of color channel 2 */
	KGI_A_COLOR3,		/* intensity of color channel 3	*/
	KGI_A_COLOR_INDEX,	/* color index value		*/
	KGI_A_ALPHA,		/* alpha value			*/
	KGI_A_PRIVATE,		/* hardware or driver private	*/
	KGI_A_APPLICATION,	/* store what you want here	*/

	KGI_A_STENCIL,		/* stencil buffer		*/
	KGI_A_Z,		/* z-value			*/

	KGI_A_FOREGROUND_INDEX,	/* foreground color index	*/
	KGI_A_TEXTURE_INDEX,	/* texture index		*/
	KGI_A_BLINK,		/* blink bit/frequency		*/

	KGI_A_LAST,
	__KGI_MAX_NR_ATTRIBUTES = 32

} kgi_attribute_t;

#define	__KGI_AM(x)	KGI_AM_##x = (1 << KGI_A_##x)
typedef enum
{
	__KGI_AM(PRIVATE),
	__KGI_AM(APPLICATION),
	__KGI_AM(STENCIL),
	__KGI_AM(Z),
	__KGI_AM(COLOR_INDEX),
	__KGI_AM(ALPHA),
	__KGI_AM(COLOR1),
	__KGI_AM(COLOR2),
	__KGI_AM(COLOR3),
	__KGI_AM(FOREGROUND_INDEX),
	__KGI_AM(TEXTURE_INDEX),
	__KGI_AM(BLINK),

	KGI_AM_COLORS = KGI_AM_COLOR1 | KGI_AM_COLOR2 | KGI_AM_COLOR3,
	KGI_AM_ALL = (1 << KGI_A_LAST) - 1

} kgi_attribute_mask_t;
#undef __KGI_AM
#define	KGI_ATTRIBUTE_MASK(attr)	(1 << (attr))

#define	KGI_AM_TEXT	(kgi_attribute_mask_t)(KGI_AM_COLOR_INDEX | KGI_AM_FOREGROUND_INDEX | KGI_AM_TEXTURE_INDEX)
#define	KGI_AM_I	(kgi_attribute_mask_t)(KGI_AM_COLOR_INDEX)
#define	KGI_AM_RGB	(kgi_attribute_mask_t)(KGI_AM_COLORS)
#define	KGI_AM_RGBI	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_COLOR_INDEX)
#define	KGI_AM_RGBA	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_ALPHA)
#define	KGI_AM_RGBX	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_APPLICATION)
#define	KGI_AM_RGBP	(kgi_attribute_mask_t)(KGI_AM_COLORS | KGI_AM_PRIVATE)

/*	attribute strings
*/
#define	KGI_AS_8888	((const kgi_u8_t[]) { 8,8,8,8,0 })
#define	KGI_AS_4444	((const kgi_u8_t[]) { 4,4,4,4,0 })
#define	KGI_AS_5551	((const kgi_u8_t[]) { 5,5,5,1,0 })
#define	KGI_AS_2321	((const kgi_u8_t[]) { 2,3,2,1,0 })
#define	KGI_AS_4642	((const kgi_u8_t[]) { 4,6,4,3,0 })
#define	KGI_AS_AAA2	((const kgi_u8_t[]) { 10,10,10,2,0 })
#define	KGI_AS_332	((const kgi_u8_t[]) { 3,3,2,0 })
#define	KGI_AS_565	((const kgi_u8_t[]) { 5,6,5,0 })
#define	KGI_AS_448	((const kgi_u8_t[]) { 4,4,8,0 })
#define	KGI_AS_664	((const kgi_u8_t[]) { 6,6,4,0 })
#define	KGI_AS_888	((const kgi_u8_t[]) { 8,8,8,0 })
#define	KGI_AS_ACA	((const kgi_u8_t[]) { 10,12,10,0 })
#define	KGI_AS_88	((const kgi_u8_t[]) { 8,8,0 })
#define	KGI_AS_8	((const kgi_u8_t[]) { 8,0 })
#define	KGI_AS_4	((const kgi_u8_t[]) { 4,0 })
#define	KGI_AS_2	((const kgi_u8_t[]) { 2,0 })
#define	KGI_AS_1	((const kgi_u8_t[]) { 1,0 })


/*
**	Resources
*/
typedef struct
{
	kgi_size_t	size;		/* aperture size		*/
	kgi_bus_addr_t	bus;		/* bus address			*/
	kgi_phys_addr_t	phys;		/* physical address		*/
	kgi_virt_addr_t	virt;		/* virtual address		*/

} kgi_aperture_t;


typedef enum
{
	/*	may be mapped by applications (others)
	*/ 
	KGI_PF_APP_R		= 0x00000001,	/* read access allowed	*/
	KGI_PF_APP_W		= 0x00000002,	/* write access allowed	*/
	KGI_PF_APP_S		= 0x00000004,	/* may be shared	*/
	KGI_PF_APP_RWS		= 0x00000007,	/* read, write, shared	*/
	KGI_PF_APP_WP		= 0x00000008,	/* write protected	*/
	KGI_PF_APP_RP		= 0x00000010,	/* read protected	*/
	KGI_PF_APP		= 0x0000001F,	/* application flags	*/

	/*	may be mapped by trusted applications (group)
	*/
	KGI_PF_LIB_R		= 0x00000100,	/* read access allowed	*/
	KGI_PF_LIB_W		= 0x00000200,	/* write access allowed	*/
	KGI_PF_LIB_S		= 0x00000400,	/* may be shared	*/
	KGI_PF_LIB_RWS		= 0x00000700,	/* read, write, shared	*/
	KGI_PF_LIB_WP		= 0x00000800,	/* write protected	*/
	KGI_PF_LIB_RP		= 0x00001000,	/* read protected	*/
	KGI_PF_LIB		= 0x00001F00,	/* library flags	*/

	/*	may be mapped only by session leader (owner)
	*/
	KGI_PF_DRV_R		= 0x00010000,	/* read access allowed	*/
	KGI_PF_DRV_W		= 0x00020000,	/* write access allowed	*/
	KGI_PF_DRV_S		= 0x00040000,	/* may be shared	*/
	KGI_PF_DRV_RWS		= 0x00070000,	/* read, write, shared	*/
	KGI_PF_DRV_WP		= 0x00080000,	/* write protected	*/
	KGI_PF_DRV_RP		= 0x00100000,	/* read protected	*/
	KGI_PF_DRV		= 0x001F0000,	/* driver flags		*/

	/*	common (global) attributes
	*/
	KGI_PF_WRITE_ORDER	= 0x01000000,	/* write order matters	*/
	KGI_PF_READ_ORDER	= 0x02000000,	/* read order matters	*/
	KGI_PF_WRITE_CACHING	= 0x04000000,	/* allowed if set	*/
	KGI_PF_READ_CACHING	= 0x08000000	/* allowed if set	*/

} kgi_protection_flags_t;

typedef enum
{
	KGI_RT_MMIO	= 0x00000000,
		KGI_RT_MMIO_FRAME_BUFFER,	/* linear frame buffer	*/
		KGI_RT_MMIO_LOCAL_BUFFER,	/* linear local buffer	*/
		KGI_RT_MMIO_PRIVATE,		/* private MMIO region	*/

	KGI_RT_SHMEM	= 0x20000000,
		KGI_RT_SHARED_MEMORY,

	KGI_RT_ACCEL	= 0x40000000,
		KGI_RT_ACCELERATOR,

	KGI_RT_COMMAND	= 0x60000000,
		KGI_RT_CURSOR_CONTROL,
		KGI_RT_POINTER_CONTROL,
		KGI_RT_ILUT_CONTROL,
		KGI_RT_ALUT_CONTROL,
		KGI_RT_TLUT_CONTROL,
		KGI_RT_TEXT16_CONTROL,

	KGI_RT_MASK	= 0x70000000,	/* WARNING: offset encoding depends! */

	KGI_RT_LAST

} kgi_resource_type_t;

#define	__KGI_MAX_NR_RESOURCES	16
#define	__KGI_RESOURCE	\
	void		*meta;		/* meta language object		*/\
	void		*meta_io;	/* meta language I/O context	*/\
	kgi_resource_type_t	type;	/* type ID			*/\
	kgi_protection_flags_t	prot;	/* protection info		*/\
	const kgi_ascii_t	*name;	/* name/identifier		*/

typedef struct
{
	__KGI_RESOURCE

} kgi_resource_t;

/*	MMIO regions
*/

typedef struct kgi_mmio_region_s kgi_mmio_region_t;

typedef void kgi_mmio_set_offset_fn(kgi_mmio_region_t *r, kgi_size_t offset);

struct kgi_mmio_region_s
{
	__KGI_RESOURCE

	kgi_u_t	access;			/* access widths allowed	*/
	kgi_u_t	align;			/* alignment requirements	*/

	kgi_size_t	size;		/* total size of the region     */
	kgi_aperture_t	win;		/* window aperture		*/

	kgi_size_t	offset;		/* window device-local address  */
	kgi_mmio_set_offset_fn	(*SetOffset);
};

/*	Accelerators
*/
typedef enum
{
	KGI_AS_IDLE,			/* has nothing to do		*/
	KGI_AS_FILL,			/* gets filled by application	*/
	KGI_AS_QUEUED,			/* is queued			*/
	KGI_AS_EXEC,			/* being executed		*/
	KGI_AS_WAIT			/* wait for execution to finish	*/

} kgi_accel_state_t;

typedef struct
{
	kgi_aperture_t	aperture;	/* context buffer aperture info	*/

} kgi_accel_context_t;

typedef struct kgi_accel_buffer_s kgi_accel_buffer_t;

struct kgi_accel_buffer_s
{
	kgi_accel_buffer_t *next;	/* next of same mapping		*/
	kgi_aperture_t	aperture;	/* buffer aperture location	*/
	void		*context;	/* mapping context		*/
	kgi_u_t		priority;	/* execution priority		*/
	kgi_wait_queue_t executed;	/* wakeup when buffer executed	*/

	struct {

		kgi_accel_state_t	state;	/* current buffer state	*/
		kgi_accel_buffer_t	*next;	/* next in exec queue	*/
		kgi_size_t		size;	/* bytes to execute	*/

	} execution;
};

struct kgi_accel_s;

typedef void kgi_accel_init_fn(struct kgi_accel_s *a, void *ctx);
typedef	void kgi_accel_done_fn(struct kgi_accel_s *a, void *ctx);
typedef void kgi_accel_exec_fn(struct kgi_accel_s *a,
		kgi_accel_buffer_t *b);
typedef void kgi_accel_wait_fn(struct kgi_accel_s *a);

typedef enum
{
	KGI_AF_DMA_BUFFERS	= 0x00000001	/* uses DMA to exec buffers */

} kgi_accel_flags_t;

typedef struct kgi_accel_s
{
	__KGI_RESOURCE

	kgi_accel_flags_t	flags;	/* accelerator flags		*/
	kgi_u_t		buffers;	/* recommended number buffers	*/
	kgi_u_t		buffer_size;	/* recommended buffer size	*/

	void		*context;	/* current context		*/
	kgi_u_t		context_size;	/* context buffer size		*/

	struct {

		kgi_accel_buffer_t *queue;	/* buffers to execute	*/
		void *context;			/* current context	*/

	} execution;			/* dynamic state		*/

	kgi_wait_queue_t	idle;	/* wakeup when becoming idle	*/

	kgi_accel_init_fn	*Init;
	kgi_accel_done_fn	*Done;
	kgi_accel_exec_fn	*Exec;

} kgi_accel_t;


/*
**	Shared Memory
*/
typedef struct kgi_shmem_context_s kgi_shmem_context_t;
typedef struct kgi_shmem_s kgi_shmem_t;

typedef kgi_error_t kgi_shmem_context_init_fn(kgi_shmem_t *shmem,
	kgi_shmem_context_t *ctx, kgi_size_t size);

typedef void kgi_shmem_context_fn(kgi_shmem_t *shmem, kgi_shmem_context_t *ctx);
typedef void kgi_shmem_art_fn(kgi_shmem_t *shmem, kgi_aperture_t *range);
typedef void kgi_shmem_art_map_fn(kgi_shmem_t *shmem, kgi_aperture_t *dst,
	kgi_aperture_t *src);

struct kgi_shmem_context_s
{
	kgi_aperture_t	aperture;	/* aperture window exported	*/
	kgi_aperture_t	art_memory;	/* address relocation table (art) */
	kgi_aperture_t	null_page;	/* 'null' (invalid) memory page	*/
};

struct kgi_shmem_s
{
	__KGI_RESOURCE

	kgi_size_t	aperture_size;	/* (maximum) aperture size	*/

	void		*ctx;		/* current context		*/
	kgi_size_t	ctx_size;	/* context struct size		*/

	kgi_shmem_context_init_fn	*ContextInit;
	kgi_shmem_context_fn		*ContextDone;
	kgi_shmem_context_fn		*ContextMap;

	kgi_shmem_context_fn		*ARTInit;
	kgi_shmem_art_fn		*ARTFlush;
	kgi_shmem_art_map_fn		*ARTMapPages;
	kgi_shmem_art_fn		*ARTUnmapPages;
};

/*
**	command resources
*/
typedef enum
{
	KGI_MM_TEXT16	= 0x00000001,
	KGI_MM_WINDOWS	= 0x00000002,
	KGI_MM_X11	= 0x00000004,
	KGI_MM_3COLOR	= 0x00000008

} kgi_marker_mode_t;

typedef struct kgi_marker_s kgi_marker_t;
struct kgi_marker_s
{
	__KGI_RESOURCE

	kgi_marker_mode_t	modes;	/* possible operation modes	*/
	kgi_u8_t		shapes;	/* number of shapes		*/
	kgi_u8_coord_t		size;	/* pattern size			*/

	void (*SetMode)(kgi_marker_t *marker, kgi_marker_mode_t mode);
	void (*Select)(kgi_marker_t *marker, kgi_u_t shape);
	void (*SetShape)(kgi_marker_t *marker, kgi_u_t shape,
		kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
		const kgi_rgb_color_t *color);

	void (*Show)(kgi_marker_t *marker, kgi_u_t x, kgi_u_t y);
	void (*Hide)(kgi_marker_t *marker);
	void (*Undo)(kgi_marker_t *marker);
};


typedef struct kgi_text16_s kgi_text16_t;
struct kgi_text16_s
{
	__KGI_RESOURCE

	kgi_ucoord_t	size;	/* visible text cells	*/
	kgi_ucoord_t	virt;	/* virtual text cells	*/
	kgi_ucoord_t	cell;	/* dots per text cell	*/
	kgi_ucoord_t	font;	/* dots per font cell	*/

	void (*PutText16)(kgi_text16_t *text16, kgi_u_t offset,
		const kgi_u16_t *text, kgi_u_t count);
};


typedef struct kgi_clut_s kgi_ilut_t;	/* image look up table		*/
typedef struct kgi_clut_s kgi_alut_t;	/* attribute look up table	*/

typedef struct kgi_clut_s kgi_clut_t;	/* color look up table		*/
struct kgi_clut_s
{
	__KGI_RESOURCE

	kgi_u_t tables;
	kgi_u_t entries;

	void (*Select)(kgi_clut_t *lut, kgi_u_t table);
	void (*Set)(kgi_clut_t *lut, kgi_u_t table, kgi_u_t index,
		kgi_u_t count, kgi_attribute_mask_t am, const kgi_u16_t *data);
};

typedef struct kgi_tlut_s kgi_tlut_t;	/* texture look up table	*/
struct kgi_tlut_s
{
	__KGI_RESOURCE

	void (*Select)(kgi_tlut_t *tlut, kgi_u_t table);
	void (*Set)(kgi_tlut_t *tlut, kgi_u_t table, kgi_u_t index,
		kgi_u_t count, const void *data);
};

/*
**	timing/modes
*/
typedef enum
{
	KGI_TC_PROPOSE,			/* propose a timing		*/
	KGI_TC_LOWER,			/* only lower clock rate	*/
	KGI_TC_RAISE,			/* only raise clock rate	*/
	KGI_TC_CHECK,			/* don't modify but check	*/
	KGI_TC_READY			/* don't care.			*/

} kgi_timing_command_t;

/*	dot ports are used to connect dot stream sources (image serializers),
**	dot stream converters (DACs, ...) and dot stream sinks (monitors,...)
*/
typedef enum
{
	/*	color space
	*/
	KGI_DPF_CS_LIN_RGB	= 0x00000000,	/* linear rgb space	*/
	KGI_DPF_CS_LIN_BGR	= 0x00000001,	/* linear bgr space	*/
	KGI_DPF_CS_LIN_YUV	= 0x00000002,	/* linear yuv space	*/
	KGI_DPF_CS_MASK		= 0x000000FF,

	/*	changeable (after mode set) properties
	*/
	KGI_DPF_CH_ORIGIN	= 0x00000100,	/* image orgin		*/
	KGI_DPF_CH_SIZE		= 0x00000200,	/* image size		*/
	KGI_DPF_CH_ILUT		= 0x00000400,	/* index->attr look up	*/
	KGI_DPF_CH_ALUT		= 0x00000800,	/* attr->attr look up	*/
	KGI_DPF_CH_TLUT		= 0x00001000,	/* tindex->pixel texture*/
	KGI_DPF_CH_MASK		= 0x0000FF00,

	/*	dot transfer protocol
	*/
	KGI_DPF_TP_LRTB_I0	= 0x00000000,	/* l>r, t>b, noninterl.	*/
	KGI_DPF_TP_LRTB_I1	= 0x00010000,	/* l>r, t>b, interl.	*/
	KGI_DPF_TP_2XCLOCK	= 0x00080000,	/* load twice per cycle	*/
	KGI_DPF_TP_MASK		= 0x000F0000,
	/* ??? stereo handling ? */

	KGI_DPF_MASK		= 0x000FFFFF

} kgi_dot_port_flags_t;

typedef struct
{
	kgi_dot_port_flags_t	flags;	/* flags		*/
	kgi_ucoord_t		dots;	/* image size in dots	*/
	kgi_u_t			dclk;	/* (max) dot clock	*/
	kgi_ratio_t		lclk;	/* load clock per dclk	*/
	kgi_ratio_t		rclk;	/* ref clock per dclk	*/
	kgi_attribute_mask_t	dam;	/* dot attr mask */
	const kgi_u8_t		*bpda;	/* bits per dot attribute */

} kgi_dot_port_mode_t;

/*	A dot stream converter (DSC) reads data from it's input dot port(s),
**	performs a conversion on it and outputs the result on the output port.
*/
typedef struct
{
	kgi_dot_port_mode_t	*out;
	kgi_u_t			inputs;
	kgi_dot_port_mode_t	in[1];

} kgi_dsc_mode_t;

/*	images
*/
typedef enum
{
	KGI_IF_ORIGIN	= 0x00000001,	/* origin can be changed	*/
	KGI_IF_VIRTUAL	= 0x00000002,	/* virtual size can be changed	*/
	KGI_IF_VISIBLE	= 0x00000004,	/* visible size can be changed	*/
	KGI_IF_TILE_X	= 0x00000008,	/* can do virtual x tiling	*/
	KGI_IF_TILE_Y	= 0x00000010,	/* can do virtual y tiling	*/
	KGI_IF_ILUT	= 0x00000020,	/* can do index -> attribute	*/
	KGI_IF_ALUT	= 0x00000040, 	/* can do attribute->attribute	*/
	KGI_IF_TLUT	= 0x00000080,	/* can do pixel texture look-up	*/
	KGI_IF_STEREO	= 0x00000100,	/* stereo image			*/
	KGI_IF_TEXT16	= 0x00000200,	/* can do text16 output		*/

	KGI_IF_ALL	= 0x000003FF	/* all flags known		*/ 

} kgi_image_flags_t;

#define	__KGI_MAX_NR_IMAGE_RESOURCES	16
typedef struct
{
	kgi_dot_port_mode_t *out;
	kgi_image_flags_t flags;

	kgi_ucoord_t	virt;
	kgi_ucoord_t	size;

	kgi_u8_t	frames;
	kgi_u8_t	tluts;
	kgi_u8_t	iluts;
	kgi_u8_t	aluts;
	kgi_attribute_mask_t	ilutm;
	kgi_attribute_mask_t	alutm;

	kgi_attribute_mask_t	fam, cam; /* frame, common attribute mask     */
	kgi_u8_t	bpfa[__KGI_MAX_NR_ATTRIBUTES];/* bits per frame attr  */
	kgi_u8_t	bpca[__KGI_MAX_NR_ATTRIBUTES];/* bits per common attr */

	kgi_resource_t	*resource[__KGI_MAX_NR_IMAGE_RESOURCES];

} kgi_image_mode_t;


#define	KGI_MODE_REVISION	0x00010000
typedef struct
{
	kgi_u_t			revision;  /* data structure revision	*/
	void			*dev_mode; /* device dependent mode	*/

	const kgi_resource_t	*resource[__KGI_MAX_NR_RESOURCES];

	kgi_u_t			images;	   /* number of images		*/
	kgi_image_mode_t	img[1];	   /* image(s)			*/

} kgi_mode_t;


/*
**	displays
*/

#define	KGI_DISPLAY_REVISION	0x00010000

typedef struct kgi_display_s kgi_display_t;

typedef void kgi_display_refcount_fn(kgi_display_t *);

typedef kgi_error_t kgi_display_check_mode_fn(kgi_display_t *dpy,
		kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images,
		void *dev_mode, const kgi_resource_t **r, kgi_u_t rsize);

typedef	void kgi_display_set_mode_fn(kgi_display_t *dpy,
		kgi_image_mode_t *img, kgi_u_t images, void *dev_mode);

typedef kgi_error_t kgi_display_command_fn(kgi_display_t *dpy,
		kgi_u_t cmd, void *in, void **out, kgi_size_t *out_size);

struct kgi_display_s
{
	kgi_u_t		revision;	/* KGI/driver revision 	*/
	kgi_ascii_t	vendor[64];	/* manufacturer		*/
	kgi_ascii_t	model[64];	/* model/trademark	*/
	kgi_u32_t	flags;		/* special capabilities	*/
	kgi_u_t		mode_size;	/* private mode size	*/


	kgi_mode_t	*mode;	/* currently set mode			*/

	kgi_u_t	id;		/* display number, init to -1		*/
	kgi_u_t	graphic;	/* non-console devices attached		*/
	struct kgi_display_s *prev; /* previous driver 		*/

	kgi_display_refcount_fn		*IncRefcount;
	kgi_display_refcount_fn		*DecRefcount;

	kgi_display_check_mode_fn	*CheckMode;
	kgi_display_set_mode_fn		*SetMode;
	kgi_display_set_mode_fn		*UnsetMode;
	kgi_display_command_fn		*Command;

	struct kgi_device_s	*focus;	/* current focus	*/
};

#include <kgi/cmd.h>

/*
**	KGI events
*/
#define KGI_EVENT_NOTICE_NEW_DISPLAY	0x00000001

typedef struct
{
	kgi_u_t		command;

} kgi_event_notice_t;

typedef union
{
	kgi_event_notice_t	notice;

} kgi_event_t;

/*
**	devices
*/
typedef enum
{
	KGI_DF_FOCUSED			= 0x00000001,
	KGI_DF_CONSOLE			= 0x00000002

} kgi_device_flags_t;

typedef void kgi_device_map_device_fn(struct kgi_device_s *);
typedef kgi_s_t kgi_device_unmap_device_fn(struct kgi_device_s *);
typedef void kgi_device_handle_event_fn(struct kgi_device_s *, kgi_event_t *);


typedef struct kgi_device_s
{
	kgi_u_t			id;	/* device number 		*/
	kgi_u_t			dpy_id;	/* display number		*/
	kgi_device_flags_t	flags;	/* device flags			*/

	kgi_device_map_device_fn	*MapDevice;
	kgi_device_unmap_device_fn	*UnmapDevice;
	kgi_device_handle_event_fn	*HandleEvent;

	kgi_mode_t	*mode;	/* currently set mode			*/

	kgi_private_t	priv;	/* device private state			*/

} kgi_device_t;

#ifdef __KERNEL__

extern void    kgi_map_device(kgi_u_t dev_id);
extern kgi_s_t kgi_unmap_device(kgi_u_t dev_id);

extern kgi_s_t kgi_register_device(kgi_device_t *dev, kgi_u_t index);
extern void    kgi_unregister_device(kgi_device_t *dev);

extern kgi_s_t kgi_register_display(kgi_display_t *dpy, kgi_u_t id);
extern void    kgi_unregister_display(kgi_display_t *dpy);
extern kgi_s_t kgi_display_registered(kgi_u_t id);

extern kgi_device_t *kgi_current_focus(kgi_u_t dpy_id);
extern void kgidev_show_gadgets(kgi_device_t *dev);
extern void kgidev_undo_gadgets(kgi_device_t *dev);

extern kgi_error_t kgidev_display_command(kgi_device_t *dev,
	kgi_u_t cmd, void *in, void **out, kgi_size_t *out_size);

#endif

#endif	/* #ifndef _kgi_kgi_h */
