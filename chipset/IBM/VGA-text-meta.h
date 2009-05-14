/* ----------------------------------------------------------------------------
**	IBM VGA (text mode only) core driver meta definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: VGA-text-meta.h,v $
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.2  2001/07/03 08:54:21  seeger_s
**	- implemented text16 image resources
**	
**	Revision 1.1.1.1  2000/04/18 08:51:19  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _chipset_IBM_vga_text_meta_h
#define	_chipset_IBM_vga_text_meta_h

#define	VGA_SEQ_REGS	0x05
#define	VGA_CRT_REGS	0x19
#define	VGA_GRC_REGS	0x09
#define	VGA_ATC_REGS	0x15

/*
**	I/O functionality required
*/
typedef struct
{
	kgim_chipset_io_t	kgim;

	io_region_t	ports;
	mem_region_t	aperture;

	kgim_io_out8_fn	*SeqOut8;
	kgim_io_in8_fn	*SeqIn8;

	kgim_io_out8_fn	*CrtOut8;
	kgim_io_in8_fn	*CrtIn8;

	kgim_io_out8_fn	*GrcOut8;
	kgim_io_in8_fn	*GrcIn8;

	kgim_io_out8_fn	*AtcOut8;
	kgim_io_in8_fn	*AtcIn8;

	void		(*MiscOut8)(void *context, kgi_u8_t val);
	kgi_u8_t	(*MiscIn8)(void *context);

	void		(*FctrlOut8)(void *context, kgi_u8_t val);
	kgi_u8_t	(*FctrlIn8)(void *context);

} vga_text_chipset_io_t;

#define	VGA_SEQ_OUT8(ctx, val, reg)	(ctx)->SeqOut8((ctx), (val), (reg))
#define	VGA_SEQ_IN8(ctx, reg)		(ctx)->SeqIn8((ctx), (reg))
#define	VGA_CRT_OUT8(ctx, val, reg)	(ctx)->CrtOut8((ctx), (val), (reg))
#define	VGA_CRT_IN8(ctx, reg)		(ctx)->CrtIn8((ctx), (reg))
#define	VGA_GRC_OUT8(ctx, val, reg)	(ctx)->GrcOut8((ctx), (val), (reg))
#define	VGA_GRC_IN8(ctx, reg)		(ctx)->GrcIn8((ctx), (reg))
#define	VGA_ATC_OUT8(ctx, val, reg)	(ctx)->AtcOut8((ctx), (val), (reg))
#define	VGA_ATC_IN8(ctx, reg)		(ctx)->AtcIn8((ctx), (reg))
#define	VGA_MISC_OUT8(ctx, val)		(ctx)->MiscOut8((ctx), (val))
#define	VGA_MISC_IN8(ctx)		(ctx)->MiscIn8(ctx)
#define	VGA_FCTRL_OUT8(ctx, val)	(ctx)->FctrlOut8((ctx), (val))
#define	VGA_FCTRL_IN8(ctx)		(ctx)->FctrlIn8(ctx)

typedef enum
{
	VGA_CF_RESTORE_INITIAL = 0x00000001
} vga_text_chipset_flags_t;

/*
**	driver global state (per-instance data) 
*/
typedef struct
{
	kgim_chipset_t		chipset;
	vga_text_chipset_flags_t	flags;

	kgi_u_t			textmemory;

	kgi_u8_t	MISC;
	kgi_u8_t	FCTRL;
	kgi_u8_t	SEQ[VGA_SEQ_REGS];
	kgi_u8_t	CRT[VGA_CRT_REGS];
	kgi_u8_t	GRC[VGA_GRC_REGS];
	kgi_u8_t	ATC[VGA_ATC_REGS];
	kgi_u8_t	MEM[256 KB];

} vga_text_chipset_t;

/*
**	mode dependent state
*/
typedef enum
{
	VGA_CMF_SW_CURSOR = 0x00000000,
	VGA_CMF_HW_CURSOR = 0x00000001

} vga_text_chipset_mode_flags_t;

typedef struct
{
	kgi_u32_t	old;
	kgi_u16_t	and, xor;

} vga_text_chipset_marker_t;

typedef struct
{
	kgim_chipset_mode_t	kgim;

	kgi_u8_t	MISC;
	kgi_u8_t	FCTRL;
	kgi_u8_t	SEQ[VGA_SEQ_REGS];
	kgi_u8_t	CRT[VGA_CRT_REGS];
	kgi_u8_t	GRC[VGA_GRC_REGS];
	kgi_u8_t	ATC[VGA_ATC_REGS];

	vga_text_chipset_mode_flags_t	flags;
	vga_text_chipset_marker_t	cur, ptr;
	kgi_u_t			orig_dot_x;
	kgi_u_t			orig_dot_y;
	kgi_u_t			orig_offs;
	kgi_u_t			fb_size;
	kgi_u_t			fb_stride;
	kgi_mmio_region_t	fb;

	kgi_marker_t	cursor_ctrl;
	kgi_marker_t	pointer_ctrl;
	kgi_tlut_t	tlut_ctrl;
	kgi_text16_t	text16;

} vga_text_chipset_mode_t;

KGIM_META_INIT_FN(vga_text_chipset)
KGIM_META_DONE_FN(vga_text_chipset)
KGIM_META_MODE_CHECK_FN(vga_text_chipset)
KGIM_META_MODE_RESOURCE_FN(vga_text_chipset)
KGIM_META_MODE_PREPARE_FN(vga_text_chipset)
KGIM_META_MODE_ENTER_FN(vga_text_chipset)
KGIM_META_IMAGE_RESOURCE_FN(vga_text_chipset)

#endif	/* #ifndef _chipset_IBM_vga_text_meta_h	*/
