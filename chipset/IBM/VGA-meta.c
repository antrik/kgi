/* ----------------------------------------------------------------------------
**	IBM VGA chipset meta-language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: VGA-meta.c,v $
**	Revision 1.5  2003/07/26 18:59:20  cegger
**	merge fixes from FreeBSD folk
**	
**	Revision 1.4  2003/03/25 03:38:25  fspacek
**	- vga driver update, todo: planar modes (4-bit, 2-bit, mode-X) will require
**	  support in display-kgi, adjust virtual res to accommodate restrictions
**	  on the stride
**	
**	Revision 1.3  2003/01/26 23:16:57  aldot
**	- remove some debug output
**	
**	Revision 1.2  2003/01/25 02:27:57  aldot
**	- sync up to current API, (first part -- 20021001)
**	
**	Revision 1.1.1.1  2001/07/18 14:57:08  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2001/07/03 08:55:02  seeger_s
**	- image resource stuff fixed
**	
**	Revision 1.2  2000/09/21 09:57:15  seeger_s
**	- name space cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:21  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		The_KGI_Project
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.5 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	3
#endif

#include <kgi/module.h>

#include "chipset/IBM/VGA.h"
#include "chipset/IBM/VGA-meta.h"

#if 0
#define DUMP_REGS(X) \
for (i=0;i<VGA_##X##_REGS;i++){\
KRN_DEBUG(2, "vga->%s[%i]=%i",#X,i,vga->X[i]);}
#else
#define DUMP_REGS(X)
#endif


static inline void vga_chipset_sync(vga_chipset_io_t *vga_io)
{
	KRN_DEBUG(2, "vga_chipset_sync()");
	
	return;
}


void vga_chipset_mode_leave(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "vga_chipset_mode_leave()");
	
	vga_chipset_sync(vga_io);

/*	vga->mode = NULL;
*/
}

kgi_error_t vga_chipset_init(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = VGA_PCIDEV(vga_io);

	KRN_ASSERT(vga);
	KRN_ASSERT(vga_io);
	KRN_ASSERT(options);

	KRN_NOTICE("%s %s Chipset driver" KGIM_CHIPSET_DRIVER,
		vga->chipset.vendor, vga->chipset.model);
	KRN_DEBUG(2, "on %.2x:%.2x.%.2x", PCICFG_BUS(pcidev),
			PCICFG_DEV(pcidev), PCICFG_FN(pcidev));

	vga_text_chipset_init(&(vga->vgatxt),&(vga_io->vgatxt),options);

	vga->MISC	= vga->vgatxt.MISC;
	vga->FCTRL	= vga->vgatxt.FCTRL;
	kgim_strncpy(vga->SEQ, vga->vgatxt.SEQ, VGA_SEQ_REGS);
	kgim_strncpy(vga->CRT, vga->vgatxt.CRT, VGA_CRT_REGS);
	kgim_strncpy(vga->GRC, vga->vgatxt.GRC, VGA_GRC_REGS);
	kgim_strncpy(vga->ATC, vga->vgatxt.ATC, VGA_ATC_REGS);

DUMP_REGS(SEQ)
DUMP_REGS(CRT)
DUMP_REGS(GRC)
DUMP_REGS(ATC)
	return KGI_EOK;
}

void vga_chipset_done(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	const kgim_options_t *options)
{
	kgi_u_t i;

	KRN_DEBUG(2, "vga_chipset_done()");

/*XXX*/for (i = 0; i < VGA_SEQ_REGS; i++)
		VGA_SEQ_OUT8(vga_io, vga->SEQ[i], i);

	vga_text_chipset_done(&(vga->vgatxt), &(vga_io->vgatxt), options);

#if 0
	kgim_strcpy(vgatxt->MISC, vga->vga->MISC);
	kgim_strcpy(vgatxt->FCTRL, vga->vga->FCTRL);
	kgim_strncpy(vgatxt->SEQ, vga->vga->SEQ,(VGA_SEQ_REGS + 1));
	kgim_strncpy(vgatxt->CRT, vga->vga->CRT,(VGA_CRT_REGS + 1));
	kgim_strncpy(vgatxt->GRC, vga->vga->GRC,(VGA_GRC_REGS + 1));
	kgim_strncpy(vgatxt->ATC, vga->vga->ATC,(VGA_ATC_REGS + 1));
#endif

DUMP_REGS(SEQ)
DUMP_REGS(CRT)
DUMP_REGS(GRC)
DUMP_REGS(ATC)
	KRN_NOTICE("%s %s driver removed.", 
		vga->chipset.vendor, vga->chipset.model);
}

/* ----	begin of mode_check() functions ------------------------------------- */




/*	FIXME: Do all text16 checking in another function,
**	and call it from here when we get a text16 mode [taylor_j]
**
**	FIX: That's what the VGA-text meta language is for! Don't reinvent
**	the wheel, reuse the VGA text meta language or improve it if
**	neccessary. VGA-meta.c should only care about graphic modes and
**	fall back to VGA-text-meta.c wherever possible.	[seeger_s]
*/
kgi_error_t vga_chipset_mode_check(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_timing_command_t cmd, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img[0].out;
	const kgim_monitor_mode_t *crt_mode = vga_mode->kgim.crt;
	kgi_u_t foo, mul, bpp, lclk;
	kgi_u_t DCLKdiv;
	kgi_u_t bpf, bpc, bpd, shift, pgm = 0;

	if ((img[0].flags & KGI_IF_TEXT16) ||
		(img[0].fam & KGI_AM_TEXTURE_INDEX)) {

		return
		vga_text_chipset_mode_check(&(vga->vgatxt), &(vga_io->vgatxt),
			&(vga_mode->vgatxt), cmd, img, images);

	}

	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (bpc + bpf * img[0].frames);

	KRN_DEBUG(2, "bpf = %i, bpc = %i, bpd = %i, bpp = %i",
			bpf, bpc, bpd, bpp);

	/*	Check if common attributes are supported 
	*/
	if (img[0].cam) {

		KRN_ERROR("Common attributes %.8x not supported", 
			img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}


	shift = 0;
	switch (bpd) {

		case  1:	shift++;	/* fall through	*/
		case  2:	shift++;	/* fall through */
		case  4:	shift++;	/* fall through	*/
		case  8:	shift++;	/* fall through	*/
			pgm = (pgm << shift) - 1;
			break;			

		default:
			KRN_ERROR("%i bpd not supported", bpd);
			return -KGI_ERRNO(CHIPSET, FAILED);
	}

	KRN_DEBUG(2, "bpd = %i, shift = %i pgm %i(%.8x)", bpd, shift,pgm,pgm);

	lclk = (cmd == KGI_TC_PROPOSE)
		? dpm->dclk : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	KRN_DEBUG(2, "lclk = %i", lclk);

	if ((dpm->flags & KGI_DPF_TP_MASK) == KGI_DPF_TP_LRTB_I1) {

		KRN_ERROR("Interlaced modes not supported.");
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}
	
	switch (cmd) {

	case KGI_TC_PROPOSE:
		KRN_DEBUG(2, "KGI_TC_PROPOSE :");
				
		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);
		
		dpm->lclk.mul = dpm->lclk.div =
		dpm->rclk.mul = dpm->rclk.div = 1;

		vga_mode->vga.flags = 0;

		/*	if size.x or size.y are zero, default to 640x400
		*/
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/*	if virt.x and virt.y are zero, default to size;
		**	if either virt.x xor virt.y is zero, maximize the other
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x) {

			img[0].virt.x = (8 * vga->chipset.memory) / 
				(img[0].virt.y * bpp);

			if (img[0].virt.x > vga->chipset.maxdots.x) {

				img[0].virt.x = vga->chipset.maxdots.x;
			}
		}
#warning	restrict img[0].virt.x to supported width here.

		if (0 == img[0].virt.y) {

			img[0].virt.y = (8 * vga->chipset.memory) /
				(img[0].virt.x * bpp);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp / 8) >
			vga->chipset.memory) {

			KRN_ERROR("Has %ib graphics memory, "
				"would need %ib for %ix%i@%i",
				vga->chipset.memory,
				img[0].virt.x * img[0].virt.y * bpp/8,
				img[0].virt.x, img[0].virt.y, bpp);

			return -KGI_ERRNO(CHIPSET, NOMEM);
		}
		
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;
		
		/* 8 bit modes need two loads per pixel, so double the width */
		if (bpd == 8) {
		
			dpm->dots.x *= 2;
		}
		
		/* Small vertical resolutions require double scan */
		if (img[0].size.y < 350) {
		
			dpm->dots.y *= 2;
			vga_mode->vga.flags |= VGA_CMF_LINEDOUBLE;
		}

		KRN_DEBUG(1, "Dotport mode with %ix%i dots",
			dpm->dots.x, dpm->dots.y);
		
		return KGI_EOK;

#warning REVISIT! read/fix LOWER & RAISE..
	case KGI_TC_LOWER:
		KRN_DEBUG(2, "KGI_TC_LOWER:");
#if 0
/*		dpm->lclk.mul = 1;
**		dpm->lclk.div = 1;
**		dpm->rclk.mul = 1;
*/		dpm->rclk.div = 1;		
#endif
		if (dpm->dclk < vga->chipset.dclk.min) {

			KRN_ERROR("%i Hz DCLK is too low.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		dpm->dclk = (dpm->dclk > vga->chipset.dclk.max) 
			? vga->chipset.dclk.max : dpm->dclk;

		return KGI_EOK;

	case KGI_TC_RAISE:
		KRN_DEBUG(2, "KGI_TC_RAISE:");
#if 0
/*		dpm->lclk.mul = 1;
**		dpm->lclk.div = 1 + pgm;
**		dpm->rclk.mul = 1;
*/		dpm->rclk.div = 1;
#endif
		if (dpm->dclk > vga->chipset.dclk.max) {

			KRN_ERROR("%i Hz DCLK is too high.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		dpm->dclk = (dpm->dclk < vga->chipset.dclk.min) 
			? vga->chipset.dclk.min : dpm->dclk;

		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(2, "Checking a graphics mode...");

			/* FIXME: Check something here */

		if ((dpm->dclk > vga->chipset.dclk.max) || 
			(dpm->dclk < vga->chipset.dclk.min)) {

			KRN_ERROR("DCLK %i Hz is out of limits.", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}


#		define TM(X) 						\
			((vga->chipset.maxdots.x < crt_mode->x.X) ||	\
			 (vga->chipset.maxdots.y < crt_mode->y.X))

			if (TM(width) || TM(blankstart) || TM(syncstart) ||
				TM(syncend) || TM(blankend) || TM(total)) {
	
				KRN_ERROR("timing check failed");
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}

#		undef TM

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
	KRN_DEBUG(2, "Setting up for mode [%.8x@%ix%i] %ix%i (virt %ix%i)", 
		  dpm->dam, dpm->dots.x, dpm->dots.y, img[0].size.x, 
		  img[0].size.y, img[0].virt.x, img[0].virt.y);

	mul = crt_mode->in.rclk.mul;

	/*
	**	Set default values
	*/

	/*	Text16 framebuffer region
	*/
	vga_mode->vga.text16fb.meta	= vga;
	vga_mode->vga.text16fb.meta_io	= vga_io;
	vga_mode->vga.text16fb.type	= KGI_RT_MMIO_FRAME_BUFFER;
	vga_mode->vga.text16fb.prot	= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	vga_mode->vga.text16fb.name	= "text16 framebuffer";
	vga_mode->vga.text16fb.access	= 8 + 16 + 32 + 64;
	vga_mode->vga.text16fb.align	= 0;
	vga_mode->vga.text16fb.size	= 64 KB; /* FIXME: do this properly */
	vga_mode->vga.text16fb.win.size	= 32 KB;
/*XXX	vga_mode->vga.text16fb.SetOffset= vga_text16_fb_set_offset; */

	/*	Graphics framebuffer region
	*/
	vga_mode->vga.pixelfb.meta	= vga;
	vga_mode->vga.pixelfb.meta_io	= vga_io;
	vga_mode->vga.pixelfb.type	= KGI_RT_MMIO_FRAME_BUFFER;
	vga_mode->vga.pixelfb.prot	= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	vga_mode->vga.pixelfb.name	= "pixel framebuffer";
	vga_mode->vga.pixelfb.access	= 8 + 16 + 32 + 64;
	vga_mode->vga.pixelfb.align	= 0;
	vga_mode->vga.pixelfb.size	= 128 KB;
	vga_mode->vga.pixelfb.win.size	= 128 KB;
	vga_mode->vga.pixelfb.win.bus	= vga_io->pixelfb.base_bus;
	vga_mode->vga.pixelfb.win.phys	= vga_io->pixelfb.base_phys;
	vga_mode->vga.pixelfb.win.virt	= vga_io->pixelfb.base_virt;
/*XXX	vga_mode->vga.pixelfb.SetOffset	= vga_text16_fb_set_offset;*/

	/* Graphics mode always use 8 char load clock */
	DCLKdiv = 8;

	kgim_memset(vga_mode->vga.SEQ, 0, sizeof(vga_mode->vga.SEQ));
	kgim_memset(vga_mode->vga.ATC, 0, sizeof(vga_mode->vga.ATC));
	kgim_memset(vga_mode->vga.GRC, 0, sizeof(vga_mode->vga.GRC));
	kgim_memset(vga_mode->vga.CRT, 0, sizeof(vga_mode->vga.CRT));
	
	/* Run, sequencer, run */
	vga_mode->vga.SEQ[VGA_SEQ_RESET] |=
		VGA_SR00_ASYNCRESET | VGA_SR00_SYNCRESET;
	/* Graphics always uses 8 dot character clock */
	vga_mode->vga.SEQ[VGA_SEQ_CLOCK] |= VGA_SR01_8DOT_CHCLK;
	/* Enable writes to all planes */
	vga_mode->vga.SEQ[VGA_SEQ_WPLANE] |= VGA_SR02_PLANE_MASK;
	/* Always access all available memory */
	vga_mode->vga.SEQ[VGA_SEQ_MEMMODE] |= VGA_SR04_256K_ACCESS;
	/* Odd/even mode is used for text modes */
	vga_mode->vga.SEQ[VGA_SEQ_MEMMODE] |= VGA_SR04_NO_ODDEVEN;

	/* Setup default palette */
	for (foo = 0; foo <= VGA_ATC_LAST_PALETTE; ++foo) {

		vga_mode->vga.ATC[foo] = foo;
	}
	
	/* Graphics mode */
	vga_mode->vga.ATC[VGA_ATC_MODECONTROL] |= VGA_AR10_GRAPHICS;

	/* Graphics mode with 128K address space */
	vga_mode->vga.GRC[VGA_GRC_MMAP_MODE] |= 
		VGA_GR6_GRAPHMODE | VGA_GR6_MAP_A0_128;
	
	/* We only use write mode 0 */
	vga_mode->vga.GRC[VGA_GRC_MODECONTROL] |= VGA_GR5_WRITEMODE0;
	/* Enable all bits */
	vga_mode->vga.GRC[VGA_GRC_BITMASK] = 0xFF;
	
	vga_mode->vga.MISC |= VGA_MISC_COLOR_IO | VGA_MISC_ENB_RAM;
	
	/* Setup the CRTC */
	vga_mode->vga.CRT[VGA_CRT_MODE] = 
		VGA_CR17_CGA_BANKING | VGA_CR17_HGC_BANKING |
		VGA_CR17_ENABLE_SYNC | VGA_CR17_CGA_ADDR_WRAP |
		VGA_CR17_BYTE_MODE;
	vga_mode->vga.CRT[VGA_CRT_HBLANKEND] |= VGA_CR03_IS_VGA;
	vga_mode->vga.CRT[VGA_CRT_CURSORSTART] |= VGA_CR0A_CURSOR_OFF;
	vga_mode->vga.CRT[0x13] = img[0].virt.x / 16;

	switch (bpd) {

	case 1:
		/* Display only data from plane 0 */
		vga_mode->vga.ATC[VGA_ATC_PLANEENABLE] |=
			0x01 & VGA_AR12_PLANEMASK;
		break;
			
	case 2:
		/* Display data from planes 0 and 1 */
		vga_mode->vga.ATC[VGA_ATC_PLANEENABLE] |=
			0x03 & VGA_AR12_PLANEMASK;
		break;
			
	case 4:
		/* Display data from all planes */
		vga_mode->vga.ATC[VGA_ATC_PLANEENABLE] |=
			0x0F & VGA_AR12_PLANEMASK;
		break;

	case 8:
		/* Display data from all planes */
		vga_mode->vga.ATC[VGA_ATC_PLANEENABLE] |=
			0x0F & VGA_AR12_PLANEMASK;
		/* Enable 8 bit operation */
		vga_mode->vga.ATC[VGA_ATC_MODECONTROL] |=
			VGA_AR10_8BIT_MODE;
		vga_mode->vga.GRC[VGA_GRC_MODECONTROL] |=
			VGA_GR5_8BIT_COLOR;
		
		/* If we can fit all of the visible image into 64K, we can
		** enable 'chain 4' and make each plane writeable separately.
		*/
		if (img[0].virt.x * img[0].virt.y <= 64 KB) {
	
			vga_mode->vga.SEQ[VGA_SEQ_MEMMODE] |= VGA_SR04_CHAINED;
			/* Adjust the crtc pitch values */
			vga_mode->vga.CRT[0x14] |= VGA_CR14_DOUBLEWORDMODE;
			vga_mode->vga.CRT[0x13] = img[0].virt.x / 8;
		}

		break;

		
	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	
	}	

	/*	Setup horizontal timing
	*/
	foo = (crt_mode->x.width * mul / DCLKdiv) - 1;
	vga_mode->vga.CRT[0x01] |= foo & 0xFF;

	foo = (crt_mode->x.blankstart * mul / DCLKdiv);
	vga_mode->vga.CRT[0x02] |= foo & 0xFF;

	foo = (crt_mode->x.syncstart * mul / DCLKdiv);
	vga_mode->vga.CRT[0x04] |= foo & 0xFF;

	foo = (crt_mode->x.syncend * mul / DCLKdiv);
	vga_mode->vga.CRT[0x05] |= foo & VGA_CR05_HSE_MASK;

	foo = (crt_mode->x.blankend * mul / DCLKdiv) - 1;
	vga_mode->vga.CRT[0x03] |= foo & VGA_CR03_HBE_MASK;
	vga_mode->vga.CRT[0x05] |= (foo & 0x20) ? VGA_CR05_HBLANKEND_B5 : 0;

	foo = (crt_mode->x.total * mul / DCLKdiv) - 5;
	vga_mode->vga.CRT[0x00] |= foo & 0xFF;

	/*	Setup vertical timing
	*/
	foo = crt_mode->y.width - 1;
	vga_mode->vga.CRT[0x12] |= foo & 0xFF;
	vga_mode->vga.CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VDISPLAYEND_B8 : 0;
	vga_mode->vga.CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VDISPLAYEND_B9 : 0;

	foo = crt_mode->y.blankstart - 1;
	vga_mode->vga.CRT[0x15] |= foo & 0xFF;
	vga_mode->vga.CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VBLANKSTART_B8 : 0;
	vga_mode->vga.CRT[0x09] |= (foo & 0x200) ? VGA_CR09_VBLANKSTART_B9 : 0;

	foo = crt_mode->y.syncstart;
	vga_mode->vga.CRT[0x10] |= foo & 0xFF;
	vga_mode->vga.CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VSYNCSTART_B8 : 0;
	vga_mode->vga.CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VSYNCSTART_B9 : 0;

	foo = crt_mode->y.syncend;
	vga_mode->vga.CRT[0x11] |= foo & VGA_CR11_VSYNCEND_MASK;

	foo = crt_mode->y.blankend - 1;
	vga_mode->vga.CRT[0x16] |= foo & 0xFF;

	foo = crt_mode->y.total - 2;
	vga_mode->vga.CRT[0x06] |= foo & 0xFF;
	vga_mode->vga.CRT[0x07] |= (foo & 0x100) ? VGA_CR07_VTOTAL_B8 : 0;
	vga_mode->vga.CRT[0x07] |= (foo & 0x200) ? VGA_CR07_VTOTAL_B9 : 0;

	vga_mode->vga.MISC |= (crt_mode->x.polarity > 0) ? 0 : VGA_MISC_NEG_HSYNC;
	vga_mode->vga.MISC |= (crt_mode->y.polarity > 0) ? 0 : VGA_MISC_NEG_VSYNC;

	if (vga_mode->vga.flags & VGA_CMF_LINEDOUBLE) {

		vga_mode->vga.CRT[0x09] |= VGA_CR09_DOUBLESCAN;
	}
	
/*
	if (vga_mode->vga.flags & VGA_CMF_DOUBLEWORD) {

		vga_mode->vga.CRT[0x17] = VGA_CR17_WORDMODE;
	}
*/
	KRN_DEBUG(0, "vga_chipset_mode_check EXIT cmd=%.8x",cmd);

	return KGI_EOK;
}

kgi_resource_t *vga_chipset_mode_resource(vga_chipset_t *vga, 
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, 
	kgi_u_t images, kgi_u_t index)
{
	KRN_DEBUG(2, "vga_chipset_mode_resource(index = %d)", index);

	if (img[0].fam & KGI_AM_TEXTURE_INDEX) {

		return vga_text_chipset_mode_resource(&(vga->vgatxt),
				&(vga_mode->vgatxt), img, images, index);
	}
	switch (index) {

	case 0:	return (kgi_resource_t *) &(vga_mode->vga.pixelfb);
	case 1:	return (kgi_resource_t *) &(vga_mode->vga.text16fb);

	default:
		KRN_ERROR("Unknown resource %i requested", index);
		return NULL;
	}
}

void vga_chipset_mode_prepare(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "vga_chipset_mode_prepare()");
	if (img[0].fam & KGI_AM_TEXTURE_INDEX) {
		vga_text_chipset_mode_prepare(&(vga->vgatxt),
				&(vga_io->vgatxt), &(vga_mode->vgatxt),
				img, images);
		return;
	}

	VGA_SEQ_OUT8(vga_io, VGA_SEQ_IN8(vga_io, 0x01) | VGA_SR01_DISPLAY_OFF, 0x01);
}

void vga_chipset_mode_enter(vga_chipset_t *vga, vga_chipset_io_t *vga_io,
	vga_chipset_mode_t *vga_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t i;

	KRN_DEBUG(2, "vga_chipset_mode_enter()");
	if (img[0].fam & KGI_AM_TEXTURE_INDEX) {

		vga_text_chipset_mode_enter(&(vga->vgatxt),
				&(vga_io->vgatxt), &(vga_mode->vgatxt),
				img, images);
		return;
	}

	VGA_MISC_OUT8(vga_io,
		(VGA_MISC_IN8(vga_io) & VGA_MISC_CLOCK_MASK)
		| vga_mode->vga.MISC);
	VGA_FCTRL_OUT8(vga_io, vga->FCTRL);

	VGA_CRT_OUT8(vga_io,
		VGA_CRT_IN8(vga_io, VGA_CRT_VSYNCEND) & ~VGA_CR11_LOCKTIMING,
		VGA_CRT_VSYNCEND);

	for (i = 0; i < VGA_CRT_REGS; i++)
		VGA_CRT_OUT8(vga_io, vga_mode->vga.CRT[i], i);

	for (i = 0; i < VGA_GRC_REGS; i++)
		VGA_GRC_OUT8(vga_io, vga_mode->vga.GRC[i], i);

	for (i = 0; i < VGA_ATC_REGS; i++)
		VGA_ATC_OUT8(vga_io, vga_mode->vga.ATC[i], i);
	VGA_ATC_IN8(vga_io, VGA_ATCI_ENABLE_DISPLAY);

	for (i = 0; i < VGA_SEQ_REGS; i++)
		VGA_SEQ_OUT8(vga_io, vga_mode->vga.SEQ[i], i);
	
	*((kgi_u32_t *) vga_mode->vga.pixelfb.win.virt) = 0x12345678;
}

kgi_resource_t *vga_chipset_image_resource(
	vga_chipset_t *vga, vga_chipset_mode_t *vga_mode,
	kgi_image_mode_t *img, kgi_u_t image, kgi_u_t index)
{
	KRN_ASSERT((0 == image) && (NULL != img));
	
	if (img[image].fam & KGI_AM_TEXTURE_INDEX) {

		return vga_text_chipset_image_resource(&(vga->vgatxt),
				&(vga_mode->vgatxt),
				img, image, index);
	}
	return NULL;
}
