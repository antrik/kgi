/* ----------------------------------------------------------------------------
**      IBM VGA (text mode only) meta language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-text-meta.c,v $
**	Revision 1.5  2004/02/15 15:25:20  aldot
**	- fix automatic ggiSetTextMode
**	
**	Revision 1.4  2003/03/12 06:15:57  fspacek
**	- vga text has only two possible dot clocks
**	
**	Revision 1.3  2002/09/21 14:35:10  aldot
**	- fix ATC border color reads
**	
**	Revision 1.2  2002/08/31 23:55:42  skids
**	
**	Cosmetic: remove ## macro cruft.  Please folks only use that if it
**	really saves a lot of code.  Makes it hard to grep through source tree.
**	
**	Revision 1.1.1.1  2001/07/18 14:57:09  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:54:21  seeger_s
**	- implemented text16 image resources
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:19  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.5 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include <kgi/module.h>

#include "chipset/IBM/VGA.h"
#include "chipset/IBM/VGA-text-meta.h"

kgi_error_t vga_text_chipset_init(vga_text_chipset_t *vga, 
	vga_text_chipset_io_t *vga_io, const kgim_options_t *options)
{
	kgi_u_t i;

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	kgim_memset(vga, 0, sizeof(*vga));
	kgim_strcpy(vga->chipset.vendor, "IBM");
	kgim_strcpy(vga->chipset.model, "VGA (text mode)");
	vga->chipset.revision	= KGIM_CHIPSET_REVISION;
	vga->chipset.flags	= KGIM_CF_NORMAL;
	vga->chipset.mode_size	= sizeof(vga_text_chipset_mode_t);

	vga->chipset.maxdots.x	= 0x3FF;
	vga->chipset.maxdots.y	= 0x3FF;
	vga->chipset.memory	= 256 KB;
	vga->chipset.dclk.min	= 25175000;
	vga->chipset.dclk.max	= 28322000;
	vga->textmemory		= 32 KB;

	vga->MISC = VGA_MISC_IN8(vga_io);

	vga->FCTRL = VGA_FCTRL_IN8(vga_io);

	for (i = 0; i < VGA_SEQ_REGS; i++)
		vga->SEQ[i] = VGA_SEQ_IN8(vga_io, i);
	for (i = 0; i < VGA_CRT_REGS; i++)
		vga->CRT[i] = VGA_CRT_IN8(vga_io, i);
	for (i = 0; i < VGA_GRC_REGS; i++)
		vga->GRC[i] = VGA_GRC_IN8(vga_io, i);
	for (i = 0; i < VGA_ATC_REGS; i++) {
		switch (i) {
		case VGA_ATC_BORDER:
			vga->ATC[i] = (VGA_ATC_IN8(vga_io,
				i | VGA_ATCI_ENABLE_DISPLAY))
				& ~VGA_ATCI_ENABLE_DISPLAY;
			break;
		default:
			vga->ATC[i] = VGA_ATC_IN8(vga_io, i);
			VGA_ATC_IN8(vga_io, VGA_ATCI_ENABLE_DISPLAY);
		}

	}

	KRN_NOTICE("%s %s driver " KGIM_CHIPSET_DRIVER,
		vga->chipset.vendor, vga->chipset.model);

	return KGI_EOK;
}

void vga_text_chipset_done(vga_text_chipset_t *vga, 
	vga_text_chipset_io_t *vga_io, const kgim_options_t *options)
{
	kgi_u_t i;

	for (i = 0; i < VGA_SEQ_REGS; i++)
		VGA_SEQ_OUT8(vga_io, vga->SEQ[i], i);

	VGA_CRT_OUT8(vga_io,
			VGA_CRT_IN8(vga_io,
				VGA_CRT_VSYNCEND) & ~VGA_CR11_LOCKTIMING,
			VGA_CRT_VSYNCEND);
	for (i = 0; i < VGA_CRT_REGS; i++)
		VGA_CRT_OUT8(vga_io, vga->CRT[i], i);
	for (i = 0; i < VGA_GRC_REGS; i++)
		VGA_GRC_OUT8(vga_io, vga->GRC[i], i);
	for (i = 0; i < VGA_ATC_REGS; i++) {
		VGA_ATC_OUT8(vga_io, vga->ATC[i], i);
		VGA_ATC_IN8(vga_io, VGA_ATCI_ENABLE_DISPLAY);	

	}


	VGA_FCTRL_OUT8(vga_io, vga->FCTRL);
	VGA_MISC_OUT8(vga_io, vga->MISC);

	KRN_NOTICE("%s %s driver removed",
		vga->chipset.vendor, vga->chipset.model);
}

#if 0
static void vga_text_chipset_adjust_timing(kgim_display_t *dpy, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_timing_command_t cmd,
	kgim_display_mode_t *kgim_mode)
{
	kgi_s_t foo;
	kgim_monitor_mode_t *crt_mode = KGIM_SUBSYSTEM_MODE(kgim_mode, monitor);

	KRN_ASSERT(img[0].out->dots.x == crt_mode->x.width); 
	KRN_ASSERT(img[0].out->dots.y == crt_mode->y.width);

	foo = img[0].out->dots.x / img[0].size.x;

	crt_mode->x.width	-= crt_mode->x.width % foo;
	crt_mode->x.blankstart	-= crt_mode->x.blankstart % foo;
	crt_mode->x.syncstart	-= crt_mode->x.syncstart % foo;
	crt_mode->x.syncend	-= crt_mode->x.syncend % foo;
	crt_mode->x.blankend	-= crt_mode->x.blankend % foo;

	if (crt_mode->x.total % foo) {

		crt_mode->x.total += foo - (crt_mode->x.total % foo);
	}

	if (((crt_mode->x.syncend / foo) & 0x3F) ==
		((crt_mode->x.syncstart / foo) & 0x3F)) {

		crt_mode->x.syncend -= foo;
	}

	if (((crt_mode->x.blankend / foo) & 0x7F) ==
		((crt_mode->x.blankstart / foo) & 0x7F)) {

		crt_mode->x.blankend -= foo;
	}

	if ((crt_mode->y.syncend & 0x0F) == (crt_mode->y.syncstart & 0x0F)) {

		crt_mode->y.syncend--;
	}

	if ((crt_mode->y.blankend & 0xFF) == (crt_mode->y.blankstart & 0xFF)) {

		crt_mode->y.blankend--;
	}
}
#endif


/* ----------------------------------------------------------------------------
**	text16 operations
** ----------------------------------------------------------------------------
*/

/*	hardware cursor
*/
static void vga_text_chipset_show_hc(kgi_marker_t *cur,
	kgi_u_t x, kgi_u_t y)
{
	vga_text_chipset_mode_t  *vga_mode = cur->meta;
	vga_text_chipset_io_t    *vga_io   = cur->meta_io;

	kgi_u_t pos = vga_mode->orig_offs  +  x  +  y * vga_mode->fb_stride;

	if (vga_mode->fb_size <= pos) {

		pos -= vga_mode->fb_size;
	}
	KRN_ASSERT(pos < vga_mode->fb_size);

	VGA_CRT_OUT8(vga_io, pos,      VGA_CRT_CURSORADDR_L);
	VGA_CRT_OUT8(vga_io, pos >> 8, VGA_CRT_CURSORADDR_H);
}

static void vga_text_chipset_hide_hc(kgi_marker_t *cursor)
{
	vga_text_chipset_io_t *vga_io = cursor->meta_io;

	VGA_CRT_OUT8(vga_io, 0xFF, VGA_CRT_CURSORADDR_H);
	VGA_CRT_OUT8(vga_io, 0xFF, VGA_CRT_CURSORADDR_L);
}

#define	vga_text_chipset_undo_hc	NULL


/*	software cursor
*/
static void vga_text_chipset_show_sc(kgi_marker_t *cur,
	kgi_u_t x, kgi_u_t y)
{
	vga_text_chipset_mode_t *vga_mode = cur->meta;

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
	kgi_u32_t new = vga_mode->orig_offs + x + y * vga_mode->fb_stride;

	if (vga_mode->fb_size < new) {

		new -= vga_mode->fb_size;
	}
	KRN_ASSERT(new < vga_mode->fb_size);

	if (vga_mode->cur.old & 0x00010000) {

		mem_out16(vga_mode->cur.old, 
			(mem_vaddr_t) (fb + (vga_mode->cur.old >> 17)));
	}

	fb += new;

	new = (new << 17) | 0x00010000;
	if ((vga_mode->ptr.old & 0xFFFF0000) == new) {

		new |= vga_mode->ptr.old & 0x0000FFFF;

	} else {

		new |= mem_in16((mem_vaddr_t) fb);
	}
	vga_mode->cur.old = new;

	mem_out16((new & vga_mode->cur.and) ^ vga_mode->cur.xor,
		(mem_vaddr_t) fb);
}

static void vga_text_chipset_hide_sc(kgi_marker_t *cur)
{
	vga_text_chipset_mode_t *vga_mode = cur->meta;

	if (vga_mode->cur.old & 0x00010000) {

		kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
		mem_out16(vga_mode->cur.old,
			(mem_vaddr_t) (fb + (vga_mode->cur.old >> 17)));
	}
	vga_mode->cur.old = 0;
}

#define vga_text_chipset_undo_sc	vga_text_chipset_hide_sc


/*	software pointer
*/
static void vga_text_chipset_show_sp(kgi_marker_t *ptr,
	kgi_u_t x, kgi_u_t y)
{
	vga_text_chipset_mode_t *vga_mode = ptr->meta;

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
	kgi_u32_t new;

	x /= vga_mode->text16.cell.x;
	y /= vga_mode->text16.cell.y;
	new = vga_mode->orig_offs + x + y * vga_mode->fb_stride;
	if (vga_mode->fb_size <= new) {

		new -= vga_mode->fb_size;
	}
	KRN_ASSERT(new < vga_mode->fb_size);

	if (vga_mode->ptr.old & 0x00010000) {

		mem_out16(vga_mode->ptr.old,
			(mem_vaddr_t) (fb + (vga_mode->ptr.old >> 17)));
	}

	fb += new;

	new = (new << 17) | 0x00010000;
	if ((vga_mode->cur.old & 0xFFFF0000) == new) {

		new |= vga_mode->cur.old & 0x0000FFFF;

	} else {

		new |= mem_in16((mem_vaddr_t) fb);
	}
	vga_mode->ptr.old = new;

	mem_out16((new & vga_mode->ptr.and) ^ vga_mode->ptr.xor,
		(mem_vaddr_t) fb);
}

static void vga_text_chipset_hide_sp(kgi_marker_t *marker)
{
	vga_text_chipset_mode_t *vga_mode = marker->meta;

	if (vga_mode->ptr.old & 0x00010000) {

		kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
		mem_out16(vga_mode->ptr.old,
			(mem_vaddr_t) (fb + (vga_mode->ptr.old >> 17)));
	}
	vga_mode->ptr.old = 0;
}

#define	vga_text_chipset_undo_sp	vga_text_chipset_hide_sp

/*	depending on the ((vga_text_chipset_mode_t *) vga_mode)->flags setting,
**	we use either the hard- or software cursor marker.
*/
#define	vga_text_chipset_cur_show(vga_mode)			\
	((vga_mode->flags & VGA_CMF_HW_CURSOR)			\
		? vga_text_chipset_show_hc : vga_text_chipset_show_sc)
#define	vga_text_chipset_cur_hide(vga_mode)			\
	((vga_mode->flags & VGA_CMF_HW_CURSOR)			\
		? vga_text_chipset_hide_hc : vga_text_chipset_hide_sc)
#define	vga_text_chipset_cur_undo(vga_mode)			\
	((vga_mode->flags & VGA_CMF_HW_CURSOR)			\
		? vga_text_chipset_undo_hc : vga_text_chipset_undo_sc)

#define	vga_text_chipset_ptr_show	vga_text_chipset_show_sp
#define	vga_text_chipset_ptr_hide	vga_text_chipset_hide_sp
#define	vga_text_chipset_ptr_undo	vga_text_chipset_undo_sp


/*	text rendering
*/
static void vga_text_chipset_put_text16(kgi_text16_t *text16,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
	vga_text_chipset_mode_t *vga_mode = text16->meta;

	kgi_u16_t *fb = (kgi_u16_t *) vga_mode->fb.win.virt;
	mem_put16((mem_vaddr_t) (fb + offset), text, count);

	KRN_ASSERT(! (vga_mode->cur.old & 0x10000));
	KRN_ASSERT(! (vga_mode->ptr.old & 0x10000));
}


/*	Texture look up table handling
*/
static void vga_text_chipset_set_tlut(kgi_tlut_t *tlut,
	kgi_u_t table, kgi_u_t index, kgi_u_t slots, const void *tdata)
{
	vga_text_chipset_mode_t *vga_mode = tlut->meta;
	vga_text_chipset_io_t   *vga_io   = tlut->meta_io;

	const kgi_u8_t *data = tdata;
	kgi_virt_addr_t fbuf = vga_mode->fb.win.virt;
	kgi_u8_t SR2, SR4, GR4, GR5, GR6, CR3A;

	fbuf += (table * 0x2000) + (index << 5);
	data += vga_mode->text16.font.y * index;

	/*	prepare hardware to access font plane
	*/

	/*	save register state
	*/
	SR2  = VGA_SEQ_IN8(vga_io, 0x02);
	SR4  = VGA_SEQ_IN8(vga_io, 0x04);
	GR4  = VGA_GRC_IN8(vga_io, 0x04);
	GR5  = VGA_GRC_IN8(vga_io, 0x05);
	GR6  = VGA_GRC_IN8(vga_io, 0x06);
	CR3A = VGA_CRT_IN8(vga_io, 0x3A);

	VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE2, 0x02); /* write to plane 2  */
	VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS | VGA_SR04_NO_ODDEVEN, 0x04);
	VGA_GRC_OUT8(vga_io, 2, 0x04); /* read from plane 2 */
	VGA_GRC_OUT8(vga_io, VGA_GR5_WRITEMODE0, 0x05);
	VGA_GRC_OUT8(vga_io, (GR6 & VGA_GR6_MAP_MASK) | VGA_GR6_GRAPHMODE,0x06);

	/* NOTE	no KRN_DEBUG/NOTICE/ASSERT allowed in here!
	*/
	if (tdata) {

		/*	set font if data is valid
		*/
		register kgi_s_t j, cnt = 32 - vga_mode->text16.font.y;

		while (slots--) {

			for (j = vga_mode->text16.font.y; j--; ) {

				mem_out8(*(data++), fbuf++);
			}
			for (j = cnt; j--; ) {

				mem_out8(0, fbuf++);
			}
		}

	} else {

		register kgi_u32_t *h = (void *) fbuf;

		/*	clear font
		*/
		while (slots--) {

			mem_out32(0, (mem_vaddr_t) (h + 0));
			mem_out32(0, (mem_vaddr_t) (h + 1));
			mem_out32(0, (mem_vaddr_t) (h + 2));
			mem_out32(0, (mem_vaddr_t) (h + 3));
			mem_out32(0, (mem_vaddr_t) (h + 4));
			mem_out32(0, (mem_vaddr_t) (h + 5));
			mem_out32(0, (mem_vaddr_t) (h + 6));
			mem_out32(0, (mem_vaddr_t) (h + 7));
			h += 8;
		}
	}

	/*	restore hardware state
	*/
/* !!!	This should go somewhere else!

	VGA_CRT_OUT8(vga_io,
		(VGA_CRT_IN8(vga_io, 0x0A) & ~VGA_CR0A_CURSOR_START_MASK) |
		((font->base + 1) & VGA_CR0A_CURSOR_START_MASK), 0x0A);
	VGA_CRT_OUT8(vga_io,
		(VGA_CRT_IN8(vga_io, 0x0B) & ~VGA_CR0B_CURSOR_END_MASK) |
		((font->base + 2) & VGA_CR0B_CURSOR_END_MASK), 0x0B);
*/
	VGA_SEQ_OUT8(vga_io, SR2,  0x02);
	VGA_SEQ_OUT8(vga_io, SR4,  0x04);
	VGA_GRC_OUT8(vga_io, GR4,  0x04);
	VGA_GRC_OUT8(vga_io, GR5,  0x05);
	VGA_GRC_OUT8(vga_io, GR6,  0x06);
	VGA_CRT_OUT8(vga_io, CR3A, 0x3A);
}

/* ----	begin of mode_check() functions ---------------------------------------
**
*/


/*	This is the SetOffset() handler for the exported memory mapped
**	I/O region. We export a 64KB region, the first 32K being a linear
**	view of the text buffer and the second 32K being a linear view
**	of the font buffer.
*/
static void vga_text16_fb_set_offset(kgi_mmio_region_t *r, kgi_size_t offset) 
{
	vga_text_chipset_io_t *vga_io = r->meta_io;

	/*	first half is text16 frame buffer, second half is font buffer
	*/
	if (offset < 32 KB) {

		/*	setup for text buffer access
		*/
		VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE0 | VGA_SR02_PLANE1, 0x02);
		VGA_SEQ_OUT8(vga_io, VGA_SR04_256K_ACCESS, 0x04);

		VGA_GRC_OUT8(vga_io, 0, 0x04);
		VGA_GRC_OUT8(vga_io, VGA_GR5_ODD_EVEN, 0x05);
		VGA_GRC_OUT8(vga_io, VGA_GR6_MAP_B8_32 | VGA_GR6_CHAIN_OE,0x06);

	} else {

		/*	setup for font buffer access
		*/
		VGA_SEQ_OUT8(vga_io, VGA_SR02_PLANE2, 0x02);
		VGA_SEQ_OUT8(vga_io, 
			VGA_SR04_256K_ACCESS | VGA_SR04_NO_ODDEVEN, 0x04);

		VGA_GRC_OUT8(vga_io, 2, 0x04);
		VGA_GRC_OUT8(vga_io, VGA_GR5_WRITEMODE0, 0x05);
		VGA_GRC_OUT8(vga_io,
			VGA_GR6_MAP_B8_32 | VGA_GR6_GRAPHMODE, 0x06);
	}
}

static const kgi_u8_t vga_default_SEQ[VGA_SEQ_REGS] =
{
	0x03, 0x00, 0x0F, 0x00, 0x02
};

static const kgi_u8_t vga_default_CRT[VGA_CRT_REGS] =
{
	0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x10,		/* 0x00	*/
	0x00, 0x40, 0x00, 0x1F, 0x00, 0x00, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0xE3,		/* 0x1x	*/
	0xFF
};

static const kgi_u8_t vga_default_GRC[VGA_GRC_REGS] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* GRC	*/
	0xFF
};

static const kgi_u8_t vga_default_ATC[VGA_ATC_REGS] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,		/* 0x00	*/
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x84, 0x00, 0x0F, 0x00, 0x00				/* 0x1x	*/
};

static const kgi_u8_t vga_text_chipset_448[] = { 4,4,8,0 };

kgi_error_t vga_text_chipset_mode_check(vga_text_chipset_t *vga,
	vga_text_chipset_io_t *vga_io, vga_text_chipset_mode_t *vga_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img[0].out;
	const kgim_monitor_mode_t *crt_mode = vga_mode->kgim.crt;
	kgi_ucoord_t text, maxsize;
	kgi_u_t foo, mul, bpp;
	kgi_u_t DCLKdiv;
	kgi_s_t dmax, dmin;

	if (images != 1) {

		KRN_ERROR("%i images are not supported", images);
		return -KGI_ERRNO(CHIPSET,NOSUP);
	}

	/*	for unsupported image flags, bail out
	*/
	if (img[0].flags & (KGI_IF_VIRTUAL | KGI_IF_VISIBLE |
		KGI_IF_TILE_X | KGI_IF_STEREO)) {

		KRN_ERROR("image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	common attributes are not possible
	*/
	if (img[0].cam) {

		KRN_ERROR("common attributes %.8x not supported",
			img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	frame attributes must be Index4 Foreground4 Texture8
	*/
	if ((img[0].fam != KGI_AM_TEXT) || 
		kgim_strcmp(img[0].bpfa, vga_text_chipset_448)) {

		KRN_ERROR("pixel attributes not supported");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	KRN_ASSERT(img[0].frames);
	bpp = img[0].frames * 16;

	if ((0 == img[0].size.x) || (0 == img[0].size.y)
	    || (0 == dpm->dots.x) || (0 == dpm->dots.y)) {

		img[0].size.x = 80;
		img[0].size.y = 25;
		text.x = 9;
		text.y = 16;

	} else {

		text.x = dpm->dots.x / img[0].size.x;
		text.y = dpm->dots.y / img[0].size.y;
	}

	if ((dpm->flags & KGI_DPF_TP_MASK) == KGI_DPF_TP_LRTB_I1) {

		KRN_ERROR("interlaced modes not supported");
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);
		KRN_ASSERT(text.x && text.y);

		maxsize.x = vga->chipset.maxdots.x / text.x;
		maxsize.y = vga->chipset.maxdots.y / text.y;
		KRN_DEBUG(2, "text grid %ix%i, maxsize %ix%i, maxdots %ix%i",
			text.x, text.y, maxsize.x, maxsize.y,
			vga->chipset.maxdots.x, vga->chipset.maxdots.y);

		/*	if virt.x and virt.y are zero, default to (size.x, max)
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
		}
		if (0 == img[0].virt.x) {

			img[0].virt.x =
				vga->chipset.memory / (bpp * img[0].virt.y);

			if (img[0].virt.x > maxsize.x) {

				img[0].virt.x = maxsize.x;
			}
		}
		if (0 == img[0].virt.y) {

			img[0].virt.y =
				vga->chipset.memory / (bpp * img[0].virt.x);

			if (img[0].virt.y > maxsize.y) {

				img[0].virt.y = maxsize.y;
			}
		}

		if ((img[0].size.x > maxsize.x) || 
			(img[0].virt.x > maxsize.x)) {

			KRN_ERROR("%i (%i) horizontal pixels exceed %i",
				img[0].size.x, img[0].virt.x, maxsize.x);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((img[0].size.y > maxsize.y) ||
			(img[0].virt.y > maxsize.y)) {

			KRN_ERROR("%i (%i) vertical pixels exceed %i",
				img[0].size.y, img[0].virt.y, maxsize.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 8*vga->textmemory) {

			KRN_ERROR("no text memory (%i byte) for %ix%i@%ibpp",
				8*vga->textmemory,
				img[0].virt.x, img[0].virt.y, bpp);
			return -KGI_ERRNO(CHIPSET, NOMEM);
		}

		if (((text.x != 8) && (text.x != 9)) || 
			(text.y < 1) || (text.y > 32)) {

			KRN_ERROR("invalid text font size (%ix%i)",
				text.x, text.y);
			return -KGI_ERRNO(CHIPSET, NOSUP);
		}

		dpm->dots.x = img[0].size.x * text.x;
		dpm->dots.y = img[0].size.y * text.y;
		KRN_DEBUG(2, "dotport mode with %ix%i dots", 
			dpm->dots.x, dpm->dots.y);

		return KGI_EOK;

	case KGI_TC_LOWER:

		dmin = (kgi_s_t)dpm->dclk - (kgi_s_t)vga->chipset.dclk.min;
		dmax = (kgi_s_t)dpm->dclk - (kgi_s_t)vga->chipset.dclk.max;
		if (dmin < -(kgi_s_t)vga->chipset.dclk.min/50) {
		
			KRN_ERROR("%i Hz DCLK is too low", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}
		else if (dmin > (kgi_s_t)vga->chipset.dclk.min/50 &&
			dmax < -(kgi_s_t)vga->chipset.dclk.max/50) {
		
			dpm->dclk = vga->chipset.dclk.min;
		}
		else if (dmax > (kgi_s_t)vga->chipset.dclk.max/50) {
			
			dpm->dclk = vga->chipset.dclk.max;
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		dmin = (kgi_s_t)dpm->dclk - (kgi_s_t)vga->chipset.dclk.min;
		dmax = (kgi_s_t)dpm->dclk - (kgi_s_t)vga->chipset.dclk.max;
		if (dmax > (kgi_s_t)vga->chipset.dclk.max/50) {
		
			KRN_ERROR("%i Hz DCLK is too high", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}
		else if (dmax < -(kgi_s_t)vga->chipset.dclk.max/50 &&
			dmin > (kgi_s_t)vga->chipset.dclk.min/50) {
		
			dpm->dclk = vga->chipset.dclk.max;
		}
		else if (dmin < -(kgi_s_t)vga->chipset.dclk.min/50) {
			
			dpm->dclk = vga->chipset.dclk.min;
		}
		return KGI_EOK;

	case KGI_TC_CHECK:

		if ((KGI_AM_TEXT != dpm->dam) || 
			kgim_strcmp(dpm->bpda, vga_text_chipset_448)) {

			KRN_ERROR("dot port attributes %.8x not supported",
				dpm->dam);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		dmin = (dpm->dclk > vga->chipset.dclk.min) ?
			(dpm->dclk - vga->chipset.dclk.min) :
			(vga->chipset.dclk.min - dpm->dclk);
		dmax = (dpm->dclk > vga->chipset.dclk.max) ?
			(dpm->dclk - vga->chipset.dclk.max) :
			(vga->chipset.dclk.max - dpm->dclk);
		if ((dmin > vga->chipset.dclk.min/50) &&
			(dmax > vga->chipset.dclk.max/50)) {

			KRN_ERROR("DCLK %i Hz is out of limits",
				dpm->dclk);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (((8 != text.x) && (9 != text.x)) || 
			((text.y < 1) || (32 < text.y))) {

			KRN_ERROR("font size %ix%i not supported",
				text.x, text.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) < vga->textmemory) {

			KRN_ERROR("not enough text memory for %ix%i virt",
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

#		define TM(X) \
			((vga->chipset.maxdots.x < crt_mode->x.X) || \
			 (vga->chipset.maxdots.y < crt_mode->y.X))

			if (TM(width) || TM(blankstart) || TM(syncstart) ||
				TM(syncend) || TM(blankend) || TM(total)) {

				KRN_ERROR("timing check failed");
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
#		undef TM

/*		vga_text_chipset_adjust_timing(img, mode); */
		img[0].flags |= KGI_IF_TEXT16;
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	/*	Now everthing is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
	KRN_DEBUG(2, "setting up for mode [%.8x  %ix%i] %ix%i (%ix%i)",
		dpm->dam, dpm->dots.x, dpm->dots.y,
		img[0].size.x, img[0].size.y,
		img[0].virt.x, img[0].virt.y);

	mul = crt_mode->in.rclk.mul;

	/*	set default values
	*/
	vga_mode->MISC		= 0x23;
	kgim_memcpy(vga_mode->SEQ, vga_default_SEQ, sizeof(vga_mode->SEQ));
	kgim_memcpy(vga_mode->CRT, vga_default_CRT, sizeof(vga_mode->CRT));
	kgim_memcpy(vga_mode->GRC, vga_default_GRC, sizeof(vga_mode->GRC));
	kgim_memcpy(vga_mode->ATC, vga_default_ATC, sizeof(vga_mode->ATC));
	vga_mode->flags		= /* VGA_CMF_HW_CURSOR */ 0;
	vga_mode->orig_dot_x	= 0;
	vga_mode->orig_dot_y	= 0;
	vga_mode->orig_offs	= 0;
	vga_mode->fb_size	= img[0].virt.x * img[0].virt.y;
	vga_mode->fb_stride	= img[0].virt.x;

	/*	frame buffer region
	*/
	vga_mode->fb.meta       = vga;
	vga_mode->fb.meta_io    = vga_io;
	vga_mode->fb.type	= KGI_RT_MMIO_FRAME_BUFFER;
	vga_mode->fb.prot	= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	vga_mode->fb.name	= "text16 buffer";
	vga_mode->fb.access	= 8 + 16 + 32 + 64;
	vga_mode->fb.align	= 0;
	vga_mode->fb.size	= 64 KB;
	vga_mode->fb.win.size	= 32 KB;
	vga_mode->fb.win.bus	= vga_io->aperture.base_bus;
	vga_mode->fb.win.phys	= vga_io->aperture.base_phys;
	vga_mode->fb.win.virt	= vga_io->aperture.base_virt;
	vga_mode->fb.SetOffset	= vga_text16_fb_set_offset;

	/*	text16 handling
	*/
	vga_mode->text16.meta		= vga_mode;
	vga_mode->text16.meta_io	= vga_io;
	vga_mode->text16.type		= KGI_RT_TEXT16_CONTROL;
	vga_mode->text16.prot		= KGI_PF_DRV_RWS;
	vga_mode->text16.name		= "VGA-text text16 control";
	vga_mode->text16.size.x		= img[0].size.x;
	vga_mode->text16.size.y		= img[0].size.y;
	vga_mode->text16.virt.x		= img[0].virt.x;
	vga_mode->text16.virt.y		= img[0].virt.y;
	vga_mode->text16.cell.x		= 9;
	vga_mode->text16.cell.y		= 16;
	vga_mode->text16.font.x		= 8;
	vga_mode->text16.font.y		= 16;
	vga_mode->text16.PutText16	= vga_text_chipset_put_text16;

	/*	texture look up table control
	*/
	vga_mode->tlut_ctrl.meta	= vga_mode;
	vga_mode->tlut_ctrl.meta_io	= vga_io;
	vga_mode->tlut_ctrl.type	= KGI_RT_TLUT_CONTROL;
	vga_mode->tlut_ctrl.prot	= KGI_PF_DRV_RWS;
	vga_mode->tlut_ctrl.name	= "VGA-text tlut control";
	vga_mode->tlut_ctrl.Set		= vga_text_chipset_set_tlut;

	/*	cursor setup
	*/
	vga_mode->cur.and	= 0x00FF;
	vga_mode->cur.xor	= 0x7F00;
	vga_mode->cur.old	= 0;

	vga_mode->cursor_ctrl.meta	= vga_mode;
	vga_mode->cursor_ctrl.meta_io	= vga_io;
	vga_mode->cursor_ctrl.type	= KGI_RT_CURSOR_CONTROL;
	vga_mode->cursor_ctrl.prot	= KGI_PF_DRV_RWS;
	vga_mode->cursor_ctrl.name	= "VGA-text cursor control";
	vga_mode->cursor_ctrl.size.x	= 1;
	vga_mode->cursor_ctrl.size.y	= 1;
	vga_mode->cursor_ctrl.Show	= vga_text_chipset_cur_show(vga_mode);
	vga_mode->cursor_ctrl.Hide	= vga_text_chipset_cur_hide(vga_mode);
	vga_mode->cursor_ctrl.Undo	= vga_text_chipset_cur_undo(vga_mode);

	/*	pointer setup
	*/
	vga_mode->ptr.and	= 0x00FF;
	vga_mode->ptr.xor	= 0x6F00;
	vga_mode->ptr.old	= 0;

	vga_mode->pointer_ctrl.meta	= vga_mode;
	vga_mode->pointer_ctrl.meta_io	= vga_io;
	vga_mode->pointer_ctrl.type	= KGI_RT_POINTER_CONTROL;
	vga_mode->pointer_ctrl.prot	= KGI_PF_DRV_RWS;
	vga_mode->pointer_ctrl.name	= "VGA-text pointer control";
	vga_mode->pointer_ctrl.size.x	= 1;
	vga_mode->pointer_ctrl.size.y	= 1;
	vga_mode->pointer_ctrl.Show	= vga_text_chipset_ptr_show;
	vga_mode->pointer_ctrl.Hide	= vga_text_chipset_ptr_hide;
	vga_mode->pointer_ctrl.Undo	= vga_text_chipset_ptr_undo;

	/*	Setup bits per pixel and logical screen width
	*/
	foo = img[0].virt.x / 2;
	vga_mode->CRT[0x13] |= foo & VGA_CR13_LOGICALWIDTH_MASK;
	vga_mode->CRT[0x14] |= VGA_CR14_UNDERLINE_MASK;
	vga_mode->CRT[0x17] = VGA_CR17_CGA_ADDR_WRAP | VGA_CR17_ENABLE_SYNC |
		VGA_CR17_CGA_BANKING | VGA_CR17_HGC_BANKING;

	vga_mode->SEQ[0x02] = VGA_SR02_PLANE0 | VGA_SR02_PLANE1;

	vga_mode->GRC[0x05] |= VGA_GR5_ODD_EVEN;
	vga_mode->GRC[0x06] |= VGA_GR6_MAP_B8_32 | VGA_GR6_CHAIN_OE;

	switch ((DCLKdiv = img[0].out->dots.x / img[0].size.x)) {

	case 8:
		vga_mode->SEQ[0x01] |= VGA_SR01_8DOT_CHCLK;
		break;

	case 9:
		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	vga_mode->CRT[0x09] |= img[0].out->dots.y/img[0].size.y - 1;
	vga_mode->ATC[0x13] = (DCLKdiv == 9) ? 0x08 : 0x00;

	/*	setup horizontal timing
	*/
	foo = (crt_mode->x.width * mul / DCLKdiv) - 1;
	vga_mode->CRT[0x01] |= foo & 0xFF;

	foo = (crt_mode->x.blankstart * mul / DCLKdiv);
	vga_mode->CRT[0x02] |= foo & 0xFF;

	foo = (crt_mode->x.syncstart * mul / DCLKdiv);
	vga_mode->CRT[0x04] |= foo & 0xFF;

	foo = (crt_mode->x.syncend * mul / DCLKdiv);
	vga_mode->CRT[0x05] |= foo & VGA_CR05_HSE_MASK;

	foo = (crt_mode->x.blankend * mul / DCLKdiv) - 1;
	vga_mode->CRT[0x03] |= foo & VGA_CR03_HBE_MASK;
	vga_mode->CRT[0x05] |= (foo & 0x20) ? VGA_CR05_HBLANKEND_B5 : 0;

	foo = (crt_mode->x.total * mul / DCLKdiv) - 5;
	vga_mode->CRT[0x00] |= foo & 0xFF;

	/*	setup vertical timing
	*/
	foo = crt_mode->y.width - 1;
	vga_mode->CRT[0x12] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VDISPLAYEND_B8 : 0;
	vga_mode->CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VDISPLAYEND_B9 : 0;

	foo = crt_mode->y.blankstart - 1;
	vga_mode->CRT[0x15] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VBLANKSTART_B8 : 0;
	vga_mode->CRT[0x09] |= (foo & 0x200) ? VGA_CR09_VBLANKSTART_B9 : 0;

	foo = crt_mode->y.syncstart;
	vga_mode->CRT[0x10] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VSYNCSTART_B8 : 0;
	vga_mode->CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VSYNCSTART_B9 : 0;

	foo = crt_mode->y.syncend;
	vga_mode->CRT[0x11] |= foo & VGA_CR11_VSYNCEND_MASK;

	foo = crt_mode->y.blankend - 1;
	vga_mode->CRT[0x16] |= foo & 0xFF;

	foo = crt_mode->y.total - 2;
	vga_mode->CRT[0x06] |= foo & 0xFF;
	vga_mode->CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VTOTAL_B8 : 0;
	vga_mode->CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VTOTAL_B9 : 0;

	vga_mode->MISC |= (crt_mode->x.polarity > 0) ? 0 : VGA_MISC_NEG_HSYNC;
	vga_mode->MISC |= (crt_mode->y.polarity > 0) ? 0 : VGA_MISC_NEG_VSYNC;

	return KGI_EOK;
}

kgi_resource_t *vga_text_chipset_mode_resource(vga_text_chipset_t *vga, 
	vga_text_chipset_mode_t *vga_mode, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	return (0 == index) ? (kgi_resource_t *) &vga_mode->fb : NULL;
}

kgi_resource_t *vga_text_chipset_image_resource(vga_text_chipset_t *vga,
	vga_text_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT(0 == image);

	switch (index) {

	case 0:	return (kgi_resource_t *) &(vga_mode->cursor_ctrl);
	case 1:	return (kgi_resource_t *) &(vga_mode->pointer_ctrl);
	case 2:	return (kgi_resource_t *) &(vga_mode->tlut_ctrl);
	case 3:	return (kgi_resource_t *) &(vga_mode->text16);

	}
	return NULL;
}

void vga_text_chipset_mode_prepare(vga_text_chipset_t *vga, 
	vga_text_chipset_io_t *vga_io, vga_text_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	VGA_SEQ_OUT8(vga_io,
		VGA_SEQ_IN8(vga_io, 0x01) | VGA_SR01_DISPLAY_OFF, 0x01);
}

void vga_text_chipset_mode_enter(vga_text_chipset_t *vga,
	vga_text_chipset_io_t *vga_io, vga_text_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t i;

	VGA_MISC_OUT8(vga_io,
		(VGA_MISC_IN8(vga_io) & VGA_MISC_CLOCK_MASK) | vga_mode->MISC);
	VGA_FCTRL_OUT8(vga_io, vga->FCTRL);

	for (i = 0; i < VGA_SEQ_REGS; i++)
		VGA_SEQ_OUT8(vga_io, vga_mode->SEQ[i], i);
	VGA_CRT_OUT8(vga_io,
			VGA_CRT_IN8(vga_io, VGA_CRT_VSYNCEND)
			& ~VGA_CR11_LOCKTIMING, VGA_CRT_VSYNCEND);
	for (i = 0; i < VGA_CRT_REGS; i++)
		VGA_CRT_OUT8(vga_io, vga_mode->CRT[i], i);
	for (i = 0; i < VGA_GRC_REGS; i++)
		VGA_GRC_OUT8(vga_io, vga_mode->GRC[i], i);
	for (i = 0; i < VGA_ATC_REGS; i++) {
		VGA_ATC_OUT8(vga_io, vga_mode->ATC[i], i);
		VGA_ATC_IN8(vga_io, VGA_ATCI_ENABLE_DISPLAY);
	}

	/* now everything should be checked and sane.. */
	*((kgi_u32_t *) vga_mode->fb.win.virt) = 0x12345678;
}
