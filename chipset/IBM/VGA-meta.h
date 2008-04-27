/* ----------------------------------------------------------------------------
**	VGA meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Jon_Taylor
**
**	$Log: VGA-meta.h,v
*/

#ifndef	_chipset_IBM_VGA_meta_h
#define	_chipset_IBM_VGA_meta_h

#include "kgi/module.h"
#include "chipset/IBM/VGA-text-meta.h"

/*	I/O functionality required
*/

typedef enum
{
	VGA_IF_VGA_DAC	= 0x00000001

} vga_chipset_io_flags_t;

typedef struct
{
	vga_text_chipset_io_t	vgatxt;

	vga_chipset_io_flags_t	flags;

	io_region_t	ports;
	mem_region_t	pixelfb;
	mem_region_t	text16fb;

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

} vga_chipset_io_t;

#define VGA_PCIDEV(ctx)			((ctx)->vgatxt.kgim.pcidev)


/*	Mode dependent state
*/
typedef enum
{
	VGA_CMF_MODEX		= 0x00000002,
	VGA_CMF_8BPPMODE	= 0x00000004,
	VGA_CMF_LINEDOUBLE	= 0x00000008,
	VGA_CMF_DOUBLEWORD	= 0x00000010

} vga_chipset_mode_flags_t;

typedef union
{
	kgim_chipset_mode_t	kgim;
	vga_text_chipset_mode_t	vgatxt;

	struct {
		kgi_u8_t	MISC;
		kgi_u8_t	FCTRL;
		kgi_u8_t	SEQ[VGA_SEQ_REGS];
		kgi_u8_t	CRT[VGA_CRT_REGS];
		kgi_u8_t	GRC[VGA_GRC_REGS];
		kgi_u8_t	ATC[VGA_ATC_REGS];

		vga_chipset_mode_flags_t	flags;

		kgi_mmio_region_t	text16fb;
		kgi_mmio_region_t	pixelfb;
	} vga;

} vga_chipset_mode_t;


/*	Driver global state (per-instance data)
*/
typedef struct
{
	kgim_chipset_t		chipset;

	vga_text_chipset_t	vgatxt;

	kgi_u8_t	MISC;
	kgi_u8_t	FCTRL;
	kgi_u8_t	SEQ[VGA_SEQ_REGS];
	kgi_u8_t	CRT[VGA_CRT_REGS];
	kgi_u8_t	GRC[VGA_GRC_REGS];
	kgi_u8_t	ATC[VGA_ATC_REGS];

/*	vga_chipset_mode_t	*mode;*/

} vga_chipset_t;


KGIM_META_INIT_FN(vga_chipset)
KGIM_META_DONE_FN(vga_chipset)
KGIM_META_MODE_CHECK_FN(vga_chipset)
KGIM_META_MODE_RESOURCE_FN(vga_chipset)
KGIM_META_MODE_PREPARE_FN(vga_chipset)
KGIM_META_MODE_ENTER_FN(vga_chipset)
KGIM_META_MODE_LEAVE_FN(vga_chipset)
KGIM_META_IMAGE_RESOURCE_FN(vga_chipset)

#endif	/* #ifdef _chipset_IBM_VGA_meta_h */
