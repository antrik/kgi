/* ----------------------------------------------------------------------------
**	Gx00 chipset driver meta
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2001	Johan Karlberg
**					Rodolphe Ortalo
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Id: Gx00-meta.c,v 1.6 2003/01/17 22:44:30 ortalo Exp $
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER		Rodolphe_Ortalo
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.6 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#define KGI_SYS_NEED_PROC
#define KGI_SYS_NEED_MUTEX
#include <kgi/module.h>

#define	__Matrox_Gx00
#include "chipset/Matrox/Gx00.h"
#include "chipset/Matrox/Gx00-meta.h"
#include "chipset/IBM/VGA.h"

/* TODO: Compile the micro-code definitions separately? -- ortalo */
#include "chipset/Matrox/Gx00-ucode.c"

/*
 * TODO: A meta language prefix of Gx00, not mgag seems more suitable. seeger
 * TODO: However, the conversion of function names will be cumbersome. ortalo 
 */

/* ----------------------------------------------------------------------------
**	Local constants and parameters
** ----------------------------------------------------------------------------
*/
#define DO_GX50_POWER_UP

/* Value sent with the SOFTRAP interrupt at the end of a DMA
 * command buffer associated to the drawing engine.
 * (NB: 2 LSBs should be 0)
 */
#define MGAG_SOFTRAP_ENGINE (0xFEDCBA98 & SOFTRAPHAND)

/*
** Maximum number of busy-wait iterations
** If this barrier is broken, usually a soft-reset is performed!
*/
#define MAX_WAIT_LOOP 1000000

#ifdef DO_GX50_POWER_UP
/* ----------------------------------------------------------------------------
**	Integration of Gx50 PLL functions (for system clock init)
** ----------------------------------------------------------------------------
*/
/*
 * TODO: Try to find a way to remove Gx50-pll.inc from the chipset driver...
 * TODO: (maybe simply via conditional compilation?) -- ortalo
 */
#define _Gx50_PLL_FOR_CHIPSET
#include "chipset/Matrox/Gx50-pll.inc"
#endif /* DO_GX50_POWER_UP */

/* ----------------------------------------------------------------------------
**	Probing and analysis code
** ----------------------------------------------------------------------------
*/

#ifdef DEBUG_LEVEL
/*
** Reads and prints verbose chipset configuration for debugging
** This is a low level debugging function normally not called
** from the code. But may be useful for some devel. work.
*/
static inline void mgag_chipset_probe(mgag_chipset_t *mgag,
				      mgag_chipset_io_t *mgag_io)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

#define PROBE_PCI(dev, reg) { \
  kgi_u32_t v = pcicfg_in32((dev)+(reg)); \
  KRN_DEBUG(1, #reg " = %.8x", v); \
  }
#define PROBE_GC(io, reg) { \
  kgi_u32_t v = MGAG_GC_IN32((io),(reg)); \
  KRN_DEBUG(1, #reg " = %.8x", v); \
  }
#define PROBE_VGA(io, mod, reg) { \
  kgi_u8_t v = MGAG_##mod##_IN8((io),(reg)); \
  KRN_DEBUG(1, #mod "[" #reg "] = %.2x", v); \
  }
#define PROBE_EDAC(io, reg) { \
  kgi_u8_t v = MGAG_EDAC_IN8((io),(reg)); \
  KRN_DEBUG(1, #reg " = %.2x", v); \
  }
#define PROBE_ECRT(io, reg) { \
  kgi_u8_t v = MGAG_ECRT_IN8((io),(reg)); \
  KRN_DEBUG(1, "ECRT[" #reg "] = %.2x", v); \
  }

  KRN_DEBUG(1, "=== Start of Matrox probe ===");

  PROBE_PCI(pcidev, PCI_COMMAND);
  PROBE_PCI(pcidev, PCI_INTERRUPT_LINE);
  PROBE_PCI(pcidev, PCI_BASE_ADDRESS_0);
  PROBE_PCI(pcidev, PCI_BASE_ADDRESS_1);
  PROBE_PCI(pcidev, PCI_BASE_ADDRESS_2);
  PROBE_PCI(pcidev, MGAG_PCI_OPTION1);
  PROBE_PCI(pcidev, MGAG_PCI_OPTION2);
  PROBE_PCI(pcidev, MGAG_PCI_OPTION3);
  PROBE_PCI(pcidev, MGAG_PCI_PM_CSR);

  PROBE_PCI(pcidev, MGAG_AGP_CMD);
  PROBE_PCI(pcidev, MGAG_AGP_IDENT);
  PROBE_PCI(pcidev, MGAG_AGP_STS);

  PROBE_GC(mgag_io, AGP_PLL);
  PROBE_GC(mgag_io, FIFOSTATUS);
  PROBE_GC(mgag_io, STATUS);
  PROBE_GC(mgag_io, IEN);
  PROBE_GC(mgag_io, VCOUNT);

  PROBE_EDAC(mgag_io, XPIXCLKCTRL);
  PROBE_EDAC(mgag_io, XGENCTRL);
  PROBE_EDAC(mgag_io, XMISCCTRL);
  PROBE_EDAC(mgag_io, XMULCTRL);
  PROBE_EDAC(mgag_io, XVREFCTRL);

  PROBE_EDAC(mgag_io, XSYSPLLM);
  PROBE_EDAC(mgag_io, XSYSPLLN);
  PROBE_EDAC(mgag_io, XSYSPLLP);
  if (mgag->flags & MGAG_CF_G550)
    { /* Video: M,N,P */
      PROBE_EDAC(mgag_io, 0x8E);
      PROBE_EDAC(mgag_io, 0x8F);
      PROBE_EDAC(mgag_io, 0x8D);
    }

  PROBE_ECRT(mgag_io, 0x01);
  PROBE_ECRT(mgag_io, 0x02);
  PROBE_ECRT(mgag_io, 0x03);
  PROBE_ECRT(mgag_io, 0x04);
  PROBE_ECRT(mgag_io, 0x05);

  PROBE_VGA(mgag_io, SEQ, 0);
  PROBE_VGA(mgag_io, SEQ, 1);
  PROBE_VGA(mgag_io, SEQ, 2);
  PROBE_VGA(mgag_io, SEQ, 3);
  PROBE_VGA(mgag_io, SEQ, 4);

  {
    kgi_u8_t v = MGAG_MISC_IN8(mgag_io);
    KRN_DEBUG(1, "MISC = %.2x", v);
  }

#if 0
  /* Probing the ATC may induce lots of problems (black screen effect
   * probably due to the enable display pb that should be fixed now)
   */
  PROBE_VGA(mgag_io, ATC, 0x00);
  PROBE_VGA(mgag_io, ATC, 0x01);
  PROBE_VGA(mgag_io, ATC, 0x02);
  PROBE_VGA(mgag_io, ATC, 0x03);
  PROBE_VGA(mgag_io, ATC, 0x04);
  PROBE_VGA(mgag_io, ATC, 0x05);
  PROBE_VGA(mgag_io, ATC, 0x06);
  PROBE_VGA(mgag_io, ATC, 0x07);
  PROBE_VGA(mgag_io, ATC, 0x08);
  PROBE_VGA(mgag_io, ATC, 0x09);
  PROBE_VGA(mgag_io, ATC, 0x0A);
  PROBE_VGA(mgag_io, ATC, 0x0B);
  PROBE_VGA(mgag_io, ATC, 0x0C);
  PROBE_VGA(mgag_io, ATC, 0x0D);
  PROBE_VGA(mgag_io, ATC, 0x0E);
  PROBE_VGA(mgag_io, ATC, 0x0F);
  PROBE_VGA(mgag_io, ATC, 0x10);
  PROBE_VGA(mgag_io, ATC, 0x11 | VGA_ATCI_ENABLE_DISPLAY);
  PROBE_VGA(mgag_io, ATC, 0x12);
  PROBE_VGA(mgag_io, ATC, 0x13);
  PROBE_VGA(mgag_io, ATC, 0x14);

  MGAG_ATC_IN8(mgag_io, VGA_ATCI_ENABLE_DISPLAY);
#endif

  /* The following is sometimes useful for debugging but
   * usually it simply pollutes kernel logs... */
#if 0
  PROBE_VGA(mgag_io, GRC, 0x00);
  PROBE_VGA(mgag_io, GRC, 0x01);
  PROBE_VGA(mgag_io, GRC, 0x02);
  PROBE_VGA(mgag_io, GRC, 0x03);
  PROBE_VGA(mgag_io, GRC, 0x04);
  PROBE_VGA(mgag_io, GRC, 0x05);
  PROBE_VGA(mgag_io, GRC, 0x06);
  PROBE_VGA(mgag_io, GRC, 0x07);
  PROBE_VGA(mgag_io, GRC, 0x08);

  PROBE_VGA(mgag_io, CRT, 0x00);
  PROBE_VGA(mgag_io, CRT, 0x01);
  PROBE_VGA(mgag_io, CRT, 0x02);
  PROBE_VGA(mgag_io, CRT, 0x03);
  PROBE_VGA(mgag_io, CRT, 0x04);
  PROBE_VGA(mgag_io, CRT, 0x05);
  PROBE_VGA(mgag_io, CRT, 0x06);
  PROBE_VGA(mgag_io, CRT, 0x07);
  PROBE_VGA(mgag_io, CRT, 0x08);
  PROBE_VGA(mgag_io, CRT, 0x09);
  PROBE_VGA(mgag_io, CRT, 0x0A);
  PROBE_VGA(mgag_io, CRT, 0x0B);
  PROBE_VGA(mgag_io, CRT, 0x0C);
  PROBE_VGA(mgag_io, CRT, 0x0D);
  PROBE_VGA(mgag_io, CRT, 0x0E);
  PROBE_VGA(mgag_io, CRT, 0x0F);
  PROBE_VGA(mgag_io, CRT, 0x10);
  PROBE_VGA(mgag_io, CRT, 0x11);
  PROBE_VGA(mgag_io, CRT, 0x12);
  PROBE_VGA(mgag_io, CRT, 0x13);
  PROBE_VGA(mgag_io, CRT, 0x14);
  PROBE_VGA(mgag_io, CRT, 0x15);
  PROBE_VGA(mgag_io, CRT, 0x16);
  PROBE_VGA(mgag_io, CRT, 0x17);
  PROBE_VGA(mgag_io, CRT, 0x18);
  PROBE_VGA(mgag_io, CRT, 0x22);
  PROBE_VGA(mgag_io, CRT, 0x24);
  PROBE_VGA(mgag_io, CRT, 0x26);
#endif

  KRN_DEBUG(1, "=== End of Matrox probe ===");

#undef PROBE_ECRT
#undef PROBE_EDAC
#undef PROBE_VGA
#undef PROBE_GC
#undef PROBE_PCI
}
#endif

#ifdef DEBUG_LEVEL
/*
** Print verbose chipset configuration for debugging
*/
static inline void mgag_chipset_examine(mgag_chipset_t *mgag)
{
  KRN_DEBUG(1, "DEVCTRL (aka pci.Command) = %.8x", mgag->pci.Command);
}
#endif

#ifdef DEBUG_LEVEL
static inline void mgag_chipset_log_status(mgag_chipset_io_t *mgag_io,
					   kgi_u_t cnt)
{
  kgi_u_t i;
  cnt = (cnt <= 0) ? 1 : cnt;
  for (i = 0; i < cnt; ++i)
    {
      kgi_u32_t status = MGAG_GC_IN32(mgag_io, STATUS);
      KRN_DEBUG(1,"%i- STATUS = %.8x", i, status);
      kgi_udelay(100);
    }
}

#endif

/* ----------------------------------------------------------------------------
**	Compatibility *UNSUPPORTED* text16 operations
** ----------------------------------------------------------------------------
*/
/* We provide some hacked "unsupported" resources to offer a console text
 * mode on a secondary display even if this is not supported according to
 * the documentation.
 * The functions defined afterwards do not use the fixed address 0xA0000
 * aperture but goes through the usual fb aperture modulo some address
 * mangling.
 * They are provided via resources that *replaces* the resources of the
 * VGA-text driver (used normally for VGA text mode). Hence, when the card
 * is a secondary adapter, we fall back to the VGA-text driver only for mode
 * (un)setting.
 */

/* Stores a byte in the VGA fb via the NORMAL Matrox fb
*/
static void mgag_vgatrick_out8(mgag_chipset_mode_t *mgag_mode,
			       mgag_chipset_io_t *mgag_io,
			       kgi_u32_t vga_idx, kgi_u8_t val)
{
  if (mgag_mode->compat.flags & MGAG_CF_64BITS_BUS) {
    mem_out8(val, mgag_io->fb.base_virt + (((vga_idx & ~0x3) << 1) | (vga_idx & 0x3)));
  } else if (mgag_mode->compat.flags & MGAG_CF_128BITS_BUS) {
    mem_out8(val, mgag_io->fb.base_virt + (((vga_idx & ~0x3) << 2) | (vga_idx & 0x3)));
  } else {
    KRN_ERROR("Unknown bus size when accessing (compat) fb");
    KRN_INTERNAL_ERROR;
  }
}

/*	hardware cursor
*/
static void mgag_chipset_compat_show_hc(kgi_marker_t *cur,
	kgi_u_t x, kgi_u_t y)
{
  mgag_chipset_mode_t  *mgag_mode = cur->meta;
  mgag_chipset_io_t    *mgag_io   = cur->meta_io;

  kgi_u_t pos = mgag_mode->compat.vga.orig_offs
    +  x  +  y * mgag_mode->compat.vga.fb_stride;
  
  if (mgag_mode->compat.vga.fb_size <= pos) {

    pos -= mgag_mode->compat.vga.fb_size;
  }
  KRN_ASSERT(pos < mgag_mode->compat.vga.fb_size);

  MGAG_CRT_OUT8(mgag_io, pos,      VGA_CRT_CURSORADDR_L);
  MGAG_CRT_OUT8(mgag_io, pos >> 8, VGA_CRT_CURSORADDR_H);
}

static void mgag_chipset_compat_hide_hc(kgi_marker_t *cursor)
{
  mgag_chipset_io_t *mgag_io = cursor->meta_io;

  MGAG_CRT_OUT8(mgag_io, 0xFF, VGA_CRT_CURSORADDR_H);
  MGAG_CRT_OUT8(mgag_io, 0xFF, VGA_CRT_CURSORADDR_L);
}

#define	mgag_chipset_compat_undo_hc NULL

/*	no software cursor implemented for compat text mode
*/

/*	Compatibility text rendering
*/
static void mgag_chipset_compat_put_text16(kgi_text16_t *text16,
	kgi_u_t offset, const kgi_u16_t *text, kgi_u_t count)
{
  mgag_chipset_mode_t *mgag_mode = text16->meta;
  mgag_chipset_io_t *mgag_io = text16->meta_io;
  kgi_u_t i;

  KRN_DEBUG(4, "Outputting %i characters at offset %.4x "
	    "via compatibility put_text16", count, offset);

  for (i = 0; i < count; i++) {
    kgi_u8_t vl = (*(text+i)) & 0xFF;
    kgi_u8_t vh = ((*(text+i)) & 0xFF00) >> 8;
    kgi_u32_t idx_p0 = (offset+i) << 3; /* Plane 0 */
    kgi_u32_t idx_p1 = ((offset+i) << 3) | 0x0001; /* Plane 1 */
    mgag_vgatrick_out8(mgag_mode,mgag_io,idx_p0,vl);
    mgag_vgatrick_out8(mgag_mode,mgag_io,idx_p1,vh);
  }
}

/*	Texture look up table handling
*/
static void mgag_chipset_compat_set_tlut(kgi_tlut_t *tlut,
	kgi_u_t table, kgi_u_t index, kgi_u_t slots, const void *tdata)
{
  mgag_chipset_mode_t *mgag_mode = tlut->meta;
  mgag_chipset_io_t *mgag_io = tlut->meta_io;
  const kgi_u8_t *data = tdata;
  kgi_u32_t vgafb = (table * 0x2000) + (index << 5);
	
  data += mgag_mode->compat.text16.font.y * index;
  vgafb += 2; /* Plane 2 */

  if (tdata) {
    /*	set font if data is valid
     */
    kgi_s_t j, cnt = 32 - mgag_mode->compat.text16.font.y;

    while (slots--) {

      for (j = mgag_mode->compat.text16.font.y; j--; ) {
	mgag_vgatrick_out8(mgag_mode, mgag_io, vgafb, *(data++));
	vgafb += 4; /* (linear view) */
      }
      for (j = cnt; j--; ) {
	mgag_vgatrick_out8(mgag_mode, mgag_io, vgafb, 0x00);
	vgafb += 4; /* (linear view) */
      }
    }  
  } else {
    kgi_s_t j;
    /*	clear font
     */
    while (slots--) {
      for (j = 32; j--; ) {
	mgag_vgatrick_out8(mgag_mode, mgag_io, vgafb, 0x00);
	vgafb += 4; /* (linear view) */
      }
    }
  }
}

/* ----------------------------------------------------------------------------
**	Interrupt management
** ----------------------------------------------------------------------------
*/

typedef struct
{
  kgi_u32_t ien;
  /* Unhandled: always reactivated */
  /* kgi_u8_t vga_crt_vsyncend; */
}
mgag_chipset_irq_state_t;

/*
** Enable, disable, save and restore IRQs
*/
#define DO_VSYNC_IRQ 0
static void mgag_chipset_irq_enable(mgag_chipset_io_t *mgag_io)
{
  KRN_DEBUG(2, "enabling some IRQs");
#if DO_VSYNC_IRQ
  MGAG_GC_OUT32(mgag_io, IEN_SOFTRAPIEN | IEN_VLINEIEN /*| IEN_EXTIEN */,
		IEN);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		& ~(VGA_CR11_DISABLE_VSYNC_IRQ | VGA_CR11_CLEAR_VSYNC_IRQ),
		VGA_CRT_VSYNCEND);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
#else
  /* Disable vsync and line int. */
  MGAG_GC_OUT32(mgag_io, IEN_SOFTRAPIEN /* | IEN_VLINEIEN | IEN_EXTIEN */,
		IEN);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_DISABLE_VSYNC_IRQ,
		VGA_CRT_VSYNCEND);
#endif
}

static void mgag_chipset_irq_block(mgag_chipset_io_t *mgag_io,
				   mgag_chipset_irq_state_t *mgag_irq_state)
{
  KRN_DEBUG(2, "saving and disabling IRQs");

  if (mgag_irq_state != NULL)
    {
      /* Saving IRQs state */
      mgag_irq_state->ien = MGAG_GC_IN32(mgag_io, IEN);
      /*  KRN_DEBUG(1, "saved: ien=%.8x", mgag_irq_state->ien); */
    }
  /* Disabling all interrupts sources */
  MGAG_GC_OUT32(mgag_io, 0, IEN);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_DISABLE_VSYNC_IRQ,
		VGA_CRT_VSYNCEND);
}

static void mgag_chipset_irq_restore(mgag_chipset_io_t *mgag_io,
				     mgag_chipset_irq_state_t *mgag_irq_state)
{
  KRN_DEBUG(2, "restoring previously saved IRQs");
  MGAG_GC_OUT32(mgag_io, mgag_irq_state->ien, IEN);
  /* TODO: Check this VSYNC interrupt issue... -- ortalo */
#if 1
#if DO_VSYNC_IRQ
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		& ~(VGA_CR11_DISABLE_VSYNC_IRQ | VGA_CR11_CLEAR_VSYNC_IRQ),
		VGA_CRT_VSYNCEND);
  MGAG_CRT_OUT8(mgag_io, MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND)
		| VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
#endif
#else
  {
    kgi_u8_t r = MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND);
    if (mgag_irq_state->vga_crt_vsyncend)
      {
	r |= VGA_CR11_DISABLE_VSYNC_IRQ;
	MGAG_GC_OUT8(mgag_io, r, VGA_CRT_VSYNCEND);
	KRN_DEBUG(1, "VSYNC IRQ disabled");
      }
    else
      {
	r &= ~(VGA_CR11_DISABLE_VSYNC_IRQ | VGA_CR11_CLEAR_VSYNC_IRQ);
	MGAG_GC_OUT8(mgag_io, r, VGA_CRT_VSYNCEND);
	KRN_DEBUG(1, "VSYNC IRQ enabled");
      }
  }
#endif
}

/*
** Chipset interrupt handler
*/

static void mgag_chipset_accel_schedule(kgi_accel_t *accel);

kgi_error_t mgag_chipset_irq_handler(mgag_chipset_t *mgag, 
				     mgag_chipset_io_t *mgag_io,
				     irq_system_t *system)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

  kgi_u32_t iclear = 0x00000000;
  kgi_u32_t flags;
  int do_schedule = 0; /* TODO: Do cleanly!!! */
  mgag_chipset_irq_state_t irq_state;

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);

  KRN_DEBUG(5, "chipset IRQ handler initiated.");

  mgag_chipset_irq_block(mgag_io, &irq_state);

  flags = MGAG_GC_IN32(mgag_io, STATUS);

  /*
  ** All Matrox chipsets interrupts
  */

  if (flags & STATUS_PICKPEN)
    {
      iclear |= ICLEAR_PICKPICLR;
      KRN_TRACE(0, mgag->interrupt.pick++);
      KRN_DEBUG(2, "Pick interrupt (pcidev %.8x)", pcidev);
    }

  if (flags & STATUS_VSYNCPEN)
    {
#if 0
      /* This is done by mgag_chipset_irq_restore() when reenable the IRQ */
      kgi_u8_t cr11;
      /* NB: Clears the interrupt directly from here */
      cr11 = MGAG_CRT_IN8(mgag_io, VGA_CRT_VSYNCEND);
      MGAG_CRT_OUT8(mgag_io, cr11 & ~VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
      MGAG_CRT_OUT8(mgag_io, cr11 | VGA_CR11_CLEAR_VSYNC_IRQ, VGA_CRT_VSYNCEND);
#endif
      KRN_TRACE(0, mgag->interrupt.vsync++);
      KRN_DEBUG(5, "Vertical Sync interrupt (pcidev %.8x)", pcidev);
    }

  if (flags & STATUS_VLINEPEN)
    {
      iclear |= ICLEAR_VLINEICLR;
      KRN_TRACE(0, mgag->interrupt.vline++);
      KRN_DEBUG(5, "1st CRTC Vertical line interrupt (pcidev %.8x)", pcidev);
    }

  if (flags & STATUS_EXTPEN)
    {
      /* Uncleared, because I don't know how, probably device dependent */
      KRN_TRACE(0, mgag->interrupt.ext++);
      KRN_DEBUG(1, "External interrupt (pcidev %.8x)", pcidev);
    }

  /*
  ** G200 or G4{0,5}0 or G550 interrupts
  */

  if ((mgag->flags & MGAG_CF_G200) || (mgag->flags & MGAG_CF_G400)
      || (mgag->flags & MGAG_CF_G550))
    {
      if (flags & STATUS_SOFTRAPEN)
	{
	  iclear |= ICLEAR_SOFTRAPICLR;
	  KRN_TRACE(0, mgag->interrupt.softtrap++);
	  KRN_DEBUG(2, "soft trap interrupt (pcidev %.8x) softrap=%.8x",
		    pcidev, MGAG_GC_IN32(mgag_io,SOFTRAP));
	  if (mgag->mode)
	    {
	      kgi_u32_t softrap = MGAG_GC_IN32(mgag_io, SOFTRAP) & SOFTRAPHAND;
	      switch (softrap)
		{
		case MGAG_SOFTRAP_ENGINE:
		  do_schedule = 1;
		  //mgag_chipset_accel_schedule(&(mgag->mode->mgag.engine));
		  break;
		default:
		  KRN_ERROR("Unknown softrap origin!");
		  break;
		}
	    }
	}

      if (flags & STATUS_WPEN)
	{
	  iclear |= ICLEAR_WICLR;
	  KRN_TRACE(0, mgag->interrupt.warp++);
	  KRN_DEBUG(1, "WARP pipe 0 interrupt (pcidev %.8x)", pcidev);
	  KRN_DEBUG(1, "Watching status after WARP0 interrrupt");
	  KRN_TRACE(1, mgag_chipset_log_status(mgag_io, 2));
	}

      if (flags & STATUS_WCPEN)
	{
	  iclear |= ICLEAR_WCICLR;
	  KRN_TRACE(0, mgag->interrupt.warpcache++);
	  KRN_DEBUG(1, "WARP pipe 0 cache interrupt (pcidev %.8x)", pcidev);
	}
    }

  /*
  ** G400 or G450 or G550 specific interrupts
  */

  if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G550))
    {
      if (flags & STATUS_C2VLINEPEN)
	{
	  iclear |= ICLEAR_C2VLINEICLR;
	  KRN_TRACE(0, mgag->interrupt.c2vline++);
	  KRN_DEBUG(5, "2nd CRTC Vertical line interrupt "
		    "(pcidev %.8x)", pcidev);
	}

      if (flags & STATUS_WPEN1)
	{
	  iclear |= ICLEAR_WICLR1;
	  KRN_TRACE(0, mgag->interrupt.warp1++);
	  KRN_DEBUG(1, "WARP pipe 1 interrupt (pcidev %.8x)", pcidev);
	  KRN_DEBUG(1, "Watching status after WARP1 interrrupt");
	  KRN_TRACE(1, mgag_chipset_log_status(mgag_io, 2));
	}

      if (flags & STATUS_WCPEN1)
	{
	  iclear |= ICLEAR_WCICLR1;
	  KRN_TRACE(0, mgag->interrupt.warpcache1++);
	  KRN_DEBUG(1, "WARP pipe 1 cache interrupt (pcidev %.8x)", pcidev);
	}
    }

  { /* Display error message if necessary */
    kgi_u_t check = ((flags & 0xFFFF) & ~iclear)
      & ~(STATUS_VSYNCPEN | STATUS_VSYNCSTS); /* Omit VSYNC irqs */
    if (check)
      {
	KRN_TRACE(0, mgag->interrupt.not_handled++);
	KRN_ERROR("MATROX: unhandled interrupt flag(s) %.4x "
		  "(pcidev %.8x)", check, pcidev);
      }
  }

  /* Finally, clear the interrupt(s) */
  MGAG_GC_OUT32(mgag_io, iclear, ICLEAR);

  KRN_TRACE(0, mgag->interrupt.handler_total++);

  mgag_chipset_irq_restore(mgag_io, &irq_state);

  if (do_schedule)
    {
      mgag_chipset_accel_schedule(&(mgag->mode->mgag.engine));
    }

  KRN_DEBUG(5, "chipset IRQ handler completed.");

  return KGI_EOK;
}

/* ----------------------------------------------------------------------------
**	Miscellaneous accelerator-related functions
** ----------------------------------------------------------------------------
*/
static void mgag_chipset_softreset(mgag_chipset_io_t *mgag_io)
{
  /* Does a softreset of the accel engine (graphics engine and WARPs)*/
  MGAG_GC_OUT32(mgag_io, RST_SOFTRESET, RST);
  /* Wait delay */
  kgi_udelay(11);
  MGAG_GC_OUT32(mgag_io, 0, RST);
}

/* Wait for the (drawing) engine to become idle */
static void mgag_chipset_wait_engine_idle(mgag_chipset_io_t *mgag_io)
{
  kgi_u32_t status;
  kgi_u_t cnt = 0;

  do
    {
      status = MGAG_GC_IN32(mgag_io, STATUS);
    }
  while ((status & STATUS_DWGENGSTS) && (cnt++ < MAX_WAIT_LOOP));

  if (status & STATUS_DWGENGSTS)
    {
      kgi_u32_t status2;
      kgi_u_t cnt2 = 0;
      KRN_ERROR("Drawing engine infinite loop!");
      mgag_chipset_softreset(mgag_io);
      /* TODO: Check if a soft reset is enough (DMA?) -- ortalo */
      do
	{
	  status2 = MGAG_GC_IN32(mgag_io, STATUS);
	}
      while ((status2 & STATUS_DWGENGSTS) && (cnt2++ < MAX_WAIT_LOOP));
      if (status2 & STATUS_DWGENGSTS)
	KRN_ERROR("Softreset did NOT solve the engine infinite loop!!");
      /* TODO: else reprogram the WARP? */
    }
}

/* Wait for the WARPs to become idle */
static void mgag_chipset_wait_warp_idle(mgag_chipset_io_t *mgag_io)
{
  /* NIY */  
}

/* Wait for the (primary) DMA to become idle */
static void mgag_chipset_wait_dma_idle(mgag_chipset_io_t *mgag_io)
{
  /* NIY */
}

/* ----------------------------------------------------------------------------
**	Graphics accelerator related data types
** ----------------------------------------------------------------------------
*/

/* TODO: Reorganize the 1x64 context to reduce size! */
typedef struct 
{
  kgi_u32_t

#define MGA_1x64_CONTEXT_INDEX0 MGA_DMA4(MACCESS,ZORG,PITCH,DMAPAD)
  index0,
    maccess, zorg, pitch, pad0_1,
#define MGA_1x64_CONTEXT_INDEX1 MGA_DMA4(FCOL,BCOL,PLNWT,DWGCTL)
    index1,
    fcol, bcol, plnwt, dwgctl,
#define MGA_1x64_CONTEXT_INDEX2 MGA_DMA4(CXBNDRY,YTOP,YBOT,DMAPAD)
    index2,
    cxbndry, ytop, ybot, pad2_1,
#define MGA_1x64_CONTEXT_INDEX3 MGA_DMA4(DMAPAD,YDSTORG,DMAPAD,SRCORG)
    index3,
    pad3_1, ydstorg, pad3_2, srcorg,
#define MGA_1x64_CONTEXT_INDEX4 MGA_DMA4(DR0,DR2,DR3,DR4)
    index4,
    dr0, dr2, dr3, dr4,
#define MGA_1x64_CONTEXT_INDEX5 MGA_DMA4(DR6,DR7,DR8,DR10)
    index5,
    dr6, dr7, dr8, dr10,
#define MGA_1x64_CONTEXT_INDEX6 MGA_DMA4(DR11,DR12,DR14,DR15)
    index6,
    dr11, dr12, dr14, dr15,
#define MGA_1x64_CONTEXT_INDEX7 MGA_DMA4(DMAPAD,DMAPAD,DMAPAD,DMAPAD)
    index7,
    pad7_1, pad7_2, pad7_3, pad7_4,
#define MGA_1x64_CONTEXT_INDEX8 MGA_DMA4(DMAPAD,DMAPAD,DMAPAD,DMAPAD)
    index8,
    pad8_1,pad8_2,pad8_3,pad8_4,
#define MGA_1x64_CONTEXT_INDEX9 MGA_DMA4(DMAPAD,DMAPAD,DMAPAD,DMAPAD)
    index9,
    pad9_1,pad9_2,pad9_3,pad9_4,
#define MGA_1x64_CONTEXT_INDEXA MGA_DMA4(FXBNDRY,SGN,SHIFT,DMAPAD)
    indexA,
    fxbndry, sgn, shift, padA_1,
#define MGA_1x64_CONTEXT_INDEXB_SRC MGA_DMA4(SRC0,SRC1,SRC2,SRC3)
#define MGA_1x64_CONTEXT_INDEXB_PAT MGA_DMA4(PAT0,PAT1,DMAPAD,DMAPAD)
    indexB,
    src0_or_pat0, src1_or_pat1, src2, src3,
    /* NOTE: Do NOT change the order of regs here without changing macros
     * NOTE: due to alias handling in state management.
     */
#define MGA_1x64_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN \
 MGA_DMA4(XYEND,YDSTLEN,XYSTRT,DMAPAD)
#define MGA_1x64_CONTEXT_INDEXC_YDSTLEN_AFTER_XYSTRT \
 MGA_DMA4(XYEND,DMAPAD,XYSTRT,YDSTLEN)
#define MGA_1x64_CONTEXT_INDEXC MGA_1x64_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN
    indexC,
    xyend, ydstlen1, xystrt, ydstlen2,
#define MGA_1x64_CONTEXT_INDEXD MGA_DMA4(AR0,AR2,AR3,AR4)
    indexD,
    ar0, ar2, ar3, ar4,
#define MGA_1x64_CONTEXT_INDEXE MGA_DMA4(AR5,AR6,XDST,YDST)
    indexE,
    ar5, ar6, xdst, ydst,
#define MGA_1x64_CONTEXT_INDEXF MGA_DMA4(LEN,AR1,DMAPAD,DMAPAD)
    indexF,
    len, ar1, padF_1, padF_2,

    /*
    ** Some tricky macros
    */
#define M1x64_XYSTRT_AFTER_YDSTLEN(ctx) \
  (ctx)->indexC = MGA_1x64_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN;
#define M1x64_YDSTLEN_AFTER_XYSTRT(ctx) \
  (ctx)->indexC = MGA_1x64_CONTEXT_INDEXC_YDSTLEN_AFTER_XYSTRT;

#define M1x64_PASSIVATE_AR5_AR6_XDST_YDST(ctx) \
  (ctx)->indexE = MGA_DMA4(DMAPAD, DMAPAD, DMAPAD, DMAPAD);
#define M1x64_PASSIVATE_AR0_AR2(ctx) \
  (ctx)->indexD &= 0xFFFF0000; \
  (ctx)->indexD |= (MGA_DMA(DMAPAD) << 8) | MGA_DMA(DMAPAD);
#define M1x64_PASSIVATE_YDST_LEN(ctx) \
  (ctx)->indexE &= 0x00FFFFFF; (ctx)->indexE |= (MGA_DMA(DMAPAD) << 24); \
  (ctx)->indexF &= 0xFFFFFF00; (ctx)->indexF |= MGA_DMA(DMAPAD);

#define M1x64_ACTIVATE_AR0(ctx) \
  (ctx)->indexD &= 0xFFFFFF00; (ctx)->indexD |= MGA_DMA(AR0);
#define M1x64_ACTIVATE_AR2(ctx) \
  (ctx)->indexD &= 0xFFFF00FF; (ctx)->indexD |= (MGA_DMA(AR2) << 8);
#define M1x64_ACTIVATE_AR5(ctx) \
  (ctx)->indexE &= 0xFFFFFF00; (ctx)->indexE |= MGA_DMA(AR5);
#define M1x64_ACTIVATE_AR6(ctx) \
  (ctx)->indexE &= 0xFFFF00FF; (ctx)->indexE |= (MGA_DMA(AR5) << 8);
#define M1x64_ACTIVATE_XDST(ctx) \
  (ctx)->indexE &= 0xFF00FFFF; (ctx)->indexE |= (MGA_DMA(XDST) << 16);
#define M1x64_ACTIVATE_YDST(ctx) \
  (ctx)->indexE &= 0x00FFFFFF; (ctx)->indexE |= (MGA_DMA(YDST) << 24);
#define M1x64_ACTIVATE_LEN(ctx) \
  (ctx)->indexF &= 0xFFFFFF00; (ctx)->indexE |= MGA_DMA(LEN);

    /* Now the part used to schedule execution */
    index_sec,
    secaddress, secend,
    /* No other regs, end of secondary DMA will reset the DMA engine */
    index_softrap,
    softrap;
    /* No other regs, SOFTRAP will reset the DMA engine */
} mga_chipset_1x64_context_t;

/* TODO: Reorganize the G200 context to reduce size! */
typedef struct 
{
  kgi_u32_t

#define MGA_G200_CONTEXT_INDEX0 MGA_DMA4(MACCESS,ZORG,PITCH,TEXTRANS)
  index0,
    maccess, zorg, pitch, textrans,
#define MGA_G200_CONTEXT_INDEX1 MGA_DMA4(FCOL,BCOL,PLNWT,DWGCTL)
    index1,
    fcol, bcol, plnwt, dwgctl,
#define MGA_G200_CONTEXT_INDEX2 MGA_DMA4(CXBNDRY,YTOP,YBOT,TEXTRANSHIGH)
    index2,
    cxbndry, ytop, ybot, textranshigh,
#define MGA_G200_CONTEXT_INDEX3 MGA_DMA4(DSTORG,YDSTORG,DMAPAD,SRCORG)
    index3,
    dstorg, ydstorg, pad3_2, srcorg,
#define MGA_G200_CONTEXT_INDEX4 MGA_DMA4(WFLAG,DMAPAD,WGETMSB,DMAPAD)
    index4,
    wflag, pad4_1, wgetmsb, pad4_2,
#define MGA_G200_CONTEXT_INDEX5 MGA_DMA4(TEXORG,TEXWIDTH,TEXHEIGHT,TEXCTL)
    index5,
    texorg,texwidth,texheight,texctl,
#define MGA_G200_CONTEXT_INDEX6 MGA_DMA4(TEXCTL2,TEXFILTER,TEXBORDERCOL,ALPHACTRL)
    index6,
    texctl2, texfilter, texbordercol, alphactrl,
#define MGA_G200_CONTEXT_INDEX7 MGA_DMA4(TEXORG1,TEXORG2,TEXORG3,TEXORG4)
    index7,
    texorg1, texorg2, texorg3, texorg4,
#define MGA_G200_CONTEXT_INDEX8 MGA_DMA4(DMAPAD,DMAPAD,DMAPAD,DMAPAD)
    index8,
    pad8_1,pad8_2,pad8_3,pad8_4,
#define MGA_G200_CONTEXT_INDEX9 MGA_DMA4(WARPREG(24),WARPREG(34),DMAPAD,FOGCOL)
    index9,
    warp24, warp34, pad9_1, fogcol,
#define MGA_G200_CONTEXT_INDEXA MGA_DMA4(FXBNDRY,SGN,SHIFT,DMAPAD)
    indexA,
    fxbndry, sgn, shift, padA_1,
#define MGA_G200_CONTEXT_INDEXB_SRC MGA_DMA4(SRC0,SRC1,SRC2,SRC3)
#define MGA_G200_CONTEXT_INDEXB_PAT MGA_DMA4(PAT0,PAT1,DMAPAD,DMAPAD)
    indexB,
    src0_or_pat0, src1_or_pat1, src2, src3,
    /* NOTE: Do NOT change the order of regs here without changing macros
     * NOTE: due to alias handling in state management.
     */
#define MGA_G200_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN \
 MGA_DMA4(XYEND,YDSTLEN,XYSTRT,DMAPAD)
#define MGA_G200_CONTEXT_INDEXC_YDSTLEN_AFTER_XYSTRT \
 MGA_DMA4(XYEND,DMAPAD,XYSTRT,YDSTLEN)
#define MGA_G200_CONTEXT_INDEXC MGA_G200_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN
    indexC,
    xyend, ydstlen1, xystrt, ydstlen2,
#define MGA_G200_CONTEXT_INDEXD MGA_DMA4(AR0,AR2,AR3,AR4)
    indexD,
    ar0, ar2, ar3, ar4,
#define MGA_G200_CONTEXT_INDEXE MGA_DMA4(AR5,AR6,XDST,YDST)
    indexE,
    ar5, ar6, xdst, ydst,
#define MGA_G200_CONTEXT_INDEXF MGA_DMA4(LEN,AR1,DMAPAD,DMAPAD)
    indexF,
    len, ar1, padF_1, padF_2,

    /*
    ** Some tricky macros
    */
#define G200_XYSTRT_AFTER_YDSTLEN(ctx) \
  (ctx)->indexC = MGA_G200_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN;
#define G200_YDSTLEN_AFTER_XYSTRT(ctx) \
  (ctx)->indexC = MGA_G200_CONTEXT_INDEXC_YDSTLEN_AFTER_XYSTRT;

#define G200_PASSIVATE_AR5_AR6_XDST_YDST(ctx) \
  (ctx)->indexE = MGA_DMA4(DMAPAD, DMAPAD, DMAPAD, DMAPAD);
#define G200_PASSIVATE_AR0_AR2(ctx) \
  (ctx)->indexD &= 0xFFFF0000; \
  (ctx)->indexD |= (MGA_DMA(DMAPAD) << 8) | MGA_DMA(DMAPAD);
#define G200_PASSIVATE_YDST_LEN(ctx) \
  (ctx)->indexE &= 0x00FFFFFF; (ctx)->indexE |= (MGA_DMA(DMAPAD) << 24); \
  (ctx)->indexF &= 0xFFFFFF00; (ctx)->indexF |= MGA_DMA(DMAPAD);

#define G200_ACTIVATE_AR0(ctx) \
  (ctx)->indexD &= 0xFFFFFF00; (ctx)->indexD |= MGA_DMA(AR0);
#define G200_ACTIVATE_AR2(ctx) \
  (ctx)->indexD &= 0xFFFF00FF; (ctx)->indexD |= (MGA_DMA(AR2) << 8);
#define G200_ACTIVATE_AR5(ctx) \
  (ctx)->indexE &= 0xFFFFFF00; (ctx)->indexE |= MGA_DMA(AR5);
#define G200_ACTIVATE_AR6(ctx) \
  (ctx)->indexE &= 0xFFFF00FF; (ctx)->indexE |= (MGA_DMA(AR5) << 8);
#define G200_ACTIVATE_XDST(ctx) \
  (ctx)->indexE &= 0xFF00FFFF; (ctx)->indexE |= (MGA_DMA(XDST) << 16);
#define G200_ACTIVATE_YDST(ctx) \
  (ctx)->indexE &= 0x00FFFFFF; (ctx)->indexE |= (MGA_DMA(YDST) << 24);
#define G200_ACTIVATE_LEN(ctx) \
  (ctx)->indexF &= 0xFFFFFF00; (ctx)->indexE |= MGA_DMA(LEN);

    /* Now the part used to schedule execution */
    index_sec,
    secaddress, secend,
    /* No other regs, end of secondary DMA will reset the DMA engine */
    index_softrap,
    softrap;
    /* No other regs, SOFTRAP will reset the DMA engine */
} mga_chipset_g200_context_t;

typedef struct 
{
  kgi_u32_t

#define MGA_G400_CONTEXT_INDEX0 MGA_DMA4(MACCESS,ZORG,PITCH,TEXTRANS)
  index0,
    maccess, zorg, pitch, textrans,
#define MGA_G400_CONTEXT_INDEX1 MGA_DMA4(FCOL,BCOL,PLNWT,DWGCTL)
    index1,
    fcol, bcol, plnwt, dwgctl,
#define MGA_G400_CONTEXT_INDEX2 MGA_DMA4(CXBNDRY,YTOP,YBOT,TEXTRANSHIGH)
    index2,
    cxbndry, ytop, ybot, textranshigh,
#define MGA_G400_CONTEXT_INDEX3 MGA_DMA4(DSTORG,TDUALSTAGE0,TDUALSTAGE1,SRCORG)
    index3,
    dstorg, tdualstage0, tdualstage1, srcorg,
#define MGA_G400_CONTEXT_INDEX4 MGA_DMA4(WFLAG,WFLAG1,WGETMSB,WACCEPTSEQ)
    index4,
    wflag, wflag1, wgetmsb, wacceptseq,
#define MGA_G400_CONTEXT_INDEX5 MGA_DMA4(TEXORG,TEXWIDTH,TEXHEIGHT,TEXCTL)
    index5,
    texorg,texwidth,texheight,texctl,
#define MGA_G400_CONTEXT_INDEX6 MGA_DMA4(TEXCTL2,TEXFILTER,TEXBORDERCOL,ALPHACTRL)
    index6,
    texctl2, texfilter, texbordercol, alphactrl,
#define MGA_G400_CONTEXT_INDEX7 MGA_DMA4(TEXORG1,TEXORG2,TEXORG3,TEXORG4)
    index7,
    texorg1, texorg2, texorg3, texorg4,
#define MGA_G400_CONTEXT_INDEX8 MGA_DMA4(STENCIL,STENCILCTL,TBUMPMAT,TBUMPFMT)
    index8,
    stencil, stencilctl, tbumpmat, tbumpfmt,
#define MGA_G400_CONTEXT_INDEX9 MGA_DMA4(WARPREG(54),WARPREG(62),WARPREG(52),FOGCOL)
    index9,
    warp54, warp62, warp52, fogcol,
#define MGA_G400_CONTEXT_INDEXA MGA_DMA4(FXBNDRY,SGN,SHIFT,DMAPAD)
    indexA,
    fxbndry, sgn, shift, padA_1,
#define MGA_G400_CONTEXT_INDEXB_SRC MGA_DMA4(SRC0,SRC1,SRC2,SRC3)
#define MGA_G400_CONTEXT_INDEXB_PAT MGA_DMA4(PAT0,PAT1,DMAPAD,DMAPAD)
    indexB,
    src0_or_pat0, src1_or_pat1, src2, src3,
    /* NOTE: Do NOT change the order of regs here without changing macros
     * NOTE: due to alias handling in state management.
     */
#define MGA_G400_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN \
 MGA_DMA4(XYEND,YDSTLEN,XYSTRT,DMAPAD)
#define MGA_G400_CONTEXT_INDEXC_YDSTLEN_AFTER_XYSTRT \
 MGA_DMA4(XYEND,DMAPAD,XYSTRT,YDSTLEN)
#define MGA_G400_CONTEXT_INDEXC MGA_G400_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN
    indexC,
    xyend, ydstlen1, xystrt, ydstlen2,
#define MGA_G400_CONTEXT_INDEXD MGA_DMA4(AR0,AR2,AR3,AR4)
    indexD,
    ar0, ar2, ar3, ar4,
#define MGA_G400_CONTEXT_INDEXE MGA_DMA4(AR5,AR6,XDST,YDST)
    indexE,
    ar5, ar6, xdst, ydst,
#define MGA_G400_CONTEXT_INDEXF MGA_DMA4(LEN,AR1,DMAPAD,DMAPAD)
    indexF,
    len, ar1, padF_1, padF_2,

    /*
    ** Some tricky macros
    */
#define G400_XYSTRT_AFTER_YDSTLEN(ctx) \
  (ctx)->indexC = MGA_G400_CONTEXT_INDEXC_XYSTRT_AFTER_YDSTLEN;
#define G400_YDSTLEN_AFTER_XYSTRT(ctx) \
  (ctx)->indexC = MGA_G400_CONTEXT_INDEXC_YDSTLEN_AFTER_XYSTRT;

#define G400_PASSIVATE_AR5_AR6_XDST_YDST(ctx) \
  (ctx)->indexE = MGA_DMA4(DMAPAD, DMAPAD, DMAPAD, DMAPAD);
#define G400_PASSIVATE_AR0_AR2(ctx) \
  (ctx)->indexD &= 0xFFFF0000; \
  (ctx)->indexD |= (MGA_DMA(DMAPAD) << 8) | MGA_DMA(DMAPAD);
#define G400_PASSIVATE_YDST_LEN(ctx) \
  (ctx)->indexE &= 0x00FFFFFF; (ctx)->indexE |= (MGA_DMA(DMAPAD) << 24); \
  (ctx)->indexF &= 0xFFFFFF00; (ctx)->indexF |= MGA_DMA(DMAPAD);

#define G400_ACTIVATE_AR0(ctx) \
  (ctx)->indexD &= 0xFFFFFF00; (ctx)->indexD |= MGA_DMA(AR0);
#define G400_ACTIVATE_AR2(ctx) \
  (ctx)->indexD &= 0xFFFF00FF; (ctx)->indexD |= (MGA_DMA(AR2) << 8);
#define G400_ACTIVATE_AR5(ctx) \
  (ctx)->indexE &= 0xFFFFFF00; (ctx)->indexE |= MGA_DMA(AR5);
#define G400_ACTIVATE_AR6(ctx) \
  (ctx)->indexE &= 0xFFFF00FF; (ctx)->indexE |= (MGA_DMA(AR5) << 8);
#define G400_ACTIVATE_XDST(ctx) \
  (ctx)->indexE &= 0xFF00FFFF; (ctx)->indexE |= (MGA_DMA(XDST) << 16);
#define G400_ACTIVATE_YDST(ctx) \
  (ctx)->indexE &= 0x00FFFFFF; (ctx)->indexE |= (MGA_DMA(YDST) << 24);
#define G400_ACTIVATE_LEN(ctx) \
  (ctx)->indexF &= 0xFFFFFF00; (ctx)->indexE |= MGA_DMA(LEN);

    /* Now the part used to schedule execution */
    index_sec,
    secaddress, secend,
    /* No other regs, end of secondary DMA will reset the DMA engine */
    index_softrap,
    softrap;
    /* No other regs, SOFTRAP will reset the DMA engine */
} mga_chipset_g400_context_t;


typedef struct
{
  kgi_accel_context_t		kgi;
  kgi_aperture_t		aperture_fast;
  kgi_aperture_t		aperture_with_ctx;

  struct {
    kgi_u32_t index1;
    kgi_u32_t secaddress;
    kgi_u32_t secend;
    /* No other regs, end of secondary DMA will reset the DMA engine */
    kgi_u32_t index2;
    kgi_u32_t softrap;
    /* No other regs, SOFTRAP will reset the DMA engine */
  } dma_fast;
  
  /* No state: most regs are Write-only */
  
  union {
      mga_chipset_g400_context_t g400;
      mga_chipset_g200_context_t g200;
      mga_chipset_1x64_context_t m1x64;
  } dma_with_context;

} mgag_chipset_accel_context_t;

/* ----------------------------------------------------------------------------
**	Setup engine specific code
** ----------------------------------------------------------------------------
*/

/* Load ucode into the WARPs instruction memory.
** The WARPs *MUST* be suspended when entering this function
** and WMISC had better be (binary)...00
*/
static void mgag_chipset_warp_load_ucode(mgag_chipset_t *mgag,
					 mgag_chipset_io_t *mgag_io,
					 kgi_u8_t *pvwarpcode,
					 kgi_u_t warpcodesize)
{
  kgi_u32_t *pwarpcode = (kgi_u32_t*)pvwarpcode;

  mgag_chipset_wait_engine_idle(mgag_io);
  /* TODO: We could even try a softreset in case of problems? */

  KRN_DEBUG(1, "Watching status before Warp ucode load");
  KRN_TRACE(1, mgag_chipset_log_status(mgag_io, 2));

  if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200)
      || (mgag->flags & MGAG_CF_G550))
    {
      MGAG_GC_OUT32(mgag_io, 0x0, WIMEMADDR);
#if 0
      /* This reg does not exists in the doc?!? -- ortalo
       * (and the WARPs run anyway)
       */
      MGAG_GC_OUT32(mgag_io, 0x0, WIMEMADDR1);
#endif

      /* TODO: Check size! */
      /* rounds size */
      warpcodesize = (warpcodesize + 0x7) & ~0x7;

      KRN_DEBUG(1, "Loading %.4x bytes @ %p of ucode into WARPs",
		warpcodesize, pwarpcode);

      while (warpcodesize != 0)
	{
	  kgi_u32_t ins = (*pwarpcode);
	  MGAG_GC_OUT32(mgag_io, ins, WIMEMDATA);
	  if ((mgag->flags & MGAG_CF_G400)
	      || (mgag->flags & MGAG_CF_G550))
	    {
	      MGAG_GC_OUT32(mgag_io, ins, WIMEMDATA1);
	    }
	  warpcodesize -= 4;
	  pwarpcode++;
	}
    }
  else
    {
      KRN_ERROR("Trying to load WARP ucode on old chipset (w/o WARP)");
    }
}

static void mgag_chipset_warp_setup_pipe(mgag_chipset_t *mgag,
					 mgag_chipset_io_t *mgag_io)
{
  /*
  ** Setup the warps to access a micro-code
  */
  KRN_DEBUG(1, "Watching status before Warp initialization");
  KRN_TRACE(1, mgag_chipset_log_status(mgag_io, 2));
  
  if (mgag->flags & MGAG_CF_G200)
    {
      mgag_chipset_wait_engine_idle(mgag_io);
      /*
      ** Directly programs the regs
      */
      MGAG_GC_OUT32(mgag_io, WIADDR_WMODE_SUSPEND, WIADDR);
      MGAG_GC_OUT32(mgag_io, WMISC_WCACHEFLUSH, WMISC);
      MGAG_GC_OUT32(mgag_io,
		    ((6 << WGETMSB_WGETMSBMIN_SHIFT) & WGETMSB_WGETMSBMIN_MASK)
		    | ((16 << WGETMSB_WGETMSBMAX_SHIFT) & WGETMSB_WGETMSBMAX_MASK),
		    WGETMSB);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0x00001606, WGETMSB); */
      
      /* We load the microcode directly in the WARPs instruction memory
      ** It could be fetched by the WARPs themselves via bus mastering,
      ** but that's not done for the moment.
      */
      mgag_chipset_warp_load_ucode(mgag, mgag_io,
				   WARP_G200_tgz, sizeof(WARP_G200_tgz));
      
      /*
      ** We initialize directly the WARPs registers (not via DMA)
      */
      MGAG_GC_OUT32(mgag_io,
		    ((7 << WVRTXSZ_WVRTXSZ_SHIFT) & WVRTXSZ_WVRTXSZ_MASK),
		    WVRTXSZ);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0x00000007, WVRTXSZ); */
      MGAG_GC_OUT32(mgag_io, 0x00000100, WARPREG(25));
      MGAG_GC_OUT32(mgag_io, 0x0000FFFF, WARPREG(42));
      MGAG_GC_OUT32(mgag_io, 0x0000FFFF, WARPREG(60));
      MGAG_GC_OUT32(mgag_io, 0x0, WARPREG(24));
      MGAG_GC_OUT32(mgag_io, 0x0, WARPREG(34));
      MGAG_GC_OUT32(mgag_io, 0, WFLAG);
      
      MGAG_GC_OUT32(mgag_io, WIADDR_WMODE_START, WIADDR); /* Start @ 0x0 */
      
      KRN_DEBUG(1, "Watching status after Warp initialization");
      KRN_TRACE(1, mgag_chipset_log_status(mgag_io, 5));
    }
  else if ((mgag->flags & MGAG_CF_G400)
	   /* TODO: We re-use the G400 WARP pipes for G550: check! */
	   || (mgag->flags & MGAG_CF_G550))
    {
      mgag_chipset_wait_engine_idle(mgag_io);
      /*
      ** Directly programs the regs
      */
      MGAG_GC_OUT32(mgag_io, WIADDR2_WMODE_SUSPEND, WIADDR2);
      MGAG_GC_OUT32(mgag_io, WMISC_WCACHEFLUSH, WMISC);
      MGAG_GC_OUT32(mgag_io,
		    ((0 << WGETMSB_WGETMSBMIN_SHIFT) & WGETMSB_WGETMSBMIN_MASK)
		    | ((14 << WGETMSB_WGETMSBMAX_SHIFT) & WGETMSB_WGETMSBMAX_MASK)
		    | WGETMSB_WBRKLEFTTOP
		    | WGETMSB_WFASTCROP
		    | WGETMSB_WCENTERSNAP
		    | WGETMSB_WBRKRIGHTTOP,
		    WGETMSB);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0xF0E00, WGETMSB); */
      
      /* We load the microcode directly in the WARPs instruction memory
      ** It could be fetched by the WARPs themselves via bus mastering,
      ** but that's not done for the moment.
      */
      mgag_chipset_warp_load_ucode(mgag, mgag_io,
				   WARP_G400_tgz, sizeof(WARP_G400_tgz));
      
      /*
      ** We initialize directly the WARPs registers (not via DMA)
      */
#if 1
      MGAG_GC_OUT32(mgag_io,
		    ((7 << WVRTXSZ_WVRTXSZ_SHIFT) & WVRTXSZ_WVRTXSZ_MASK)
		    | ((24 << WVRTXSZ_PRIMSZ_SHIFT) & WVRTXSZ_PRIMSZ_MASK),
		    WVRTXSZ);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0x1807, WVRTXSZ); */
#else
      MGAG_GC_OUT32(mgag_io,
		    ((9 << WVRTXSZ_WVRTXSZ_SHIFT) & WVRTXSZ_WVRTXSZ_MASK)
		    | ((30 << WVRTXSZ_PRIMSZ_SHIFT) & WVRTXSZ_PRIMSZ_MASK),
		    WVRTXSZ);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0x1E09, WVRTXSZ); */
#endif
#if 1
      MGAG_GC_OUT32(mgag_io, WACCEPTSEQ_SEQOFF | WACCEPTSEQ_WSAMETAG,
		    WACCEPTSEQ);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0x18000000, WACCEPTSEQ); */
#else
      MGAG_GC_OUT32(mgag_io, WACCEPTSEQ_SEQOFF | WACCEPTSEQ_WSAMETAG
		    | WACCEPTSEQ_WFIRSTTAG, WACCEPTSEQ);
      /* i.e.: MGAG_GC_OUT32(mgag_io, 0x1e000000, WACCEPTSEQ); */
#endif
      MGAG_GC_OUT32(mgag_io, 0x46480000, WARPREG(56)); /* 12800.0f */
      MGAG_GC_OUT32(mgag_io, 0, WARPREG(49));
      MGAG_GC_OUT32(mgag_io, 0, WARPREG(57));
      MGAG_GC_OUT32(mgag_io, 0, WARPREG(53));
      MGAG_GC_OUT32(mgag_io, 0, WARPREG(61));
      MGAG_GC_OUT32(mgag_io, 0x40, WARPREG(54));
      MGAG_GC_OUT32(mgag_io, 0x40, WARPREG(62));
      MGAG_GC_OUT32(mgag_io, 0x40, WARPREG(52));
      MGAG_GC_OUT32(mgag_io, 0x40, WARPREG(60));
      MGAG_GC_OUT32(mgag_io, 0, WFLAG1);
      MGAG_GC_OUT32(mgag_io, 0, WFLAG);
      
      MGAG_GC_OUT32(mgag_io, WIADDR2_WMODE_START, WIADDR2); /* Start @ 0x0 */
      
      KRN_DEBUG(1, "Watching status after Warp initialization");
      KRN_TRACE(1, mgag_chipset_log_status(mgag_io, 5));
    }
  else
    {
      KRN_ERROR("Unknown chipset wrt WARP setup engine");
    }
}

static void mgag_chipset_warp_do_buffer(mgag_chipset_io_t *mgag_io,
					kgi_accel_buffer_t *buffer,
					mgag_chipset_accel_context_t *mgag_ctx)
{
  /* Sets start adress and end address in the primary DMA list */
  mgag_ctx->dma_fast.secaddress = (buffer->aperture.bus
				   + MGAG_ACCEL_TAG_LENGTH) /* skip tag */
    | SECADDRESS_DMAMOD_VERTEX_WRITE;
  mgag_ctx->dma_fast.secend = buffer->aperture.bus + buffer->execution.size;
  mgag_ctx->dma_fast.softrap = MGAG_SOFTRAP_ENGINE;
  KRN_DEBUG(2,"Executing one WARP buffer "
	    "(primaddress=%.8x,primend=%.8x,"
	    "secaddress=%8.x,secend=%.8x,size=%.4x)",
	    mgag_ctx->aperture_fast.bus,
	    mgag_ctx->aperture_fast.bus + mgag_ctx->aperture_fast.size,
	    buffer->aperture.bus + MGAG_ACCEL_TAG_LENGTH,
	    buffer->aperture.bus + buffer->execution.size,
	    buffer->execution.size);
  /* For testing, sets a trapezoid with no textures TODO: Remove! */
#if 0
  MGAG_GC_OUT32(mgag_io, 0x00000400, PITCH);
  MGAG_GC_OUT32(mgag_io, 0x00000000, DSTORG);
  MGAG_GC_OUT32(mgag_io, 0x00800000, ZORG); /* !!! */
  MGAG_GC_OUT32(mgag_io, 0xFFFFFFFF, PLNWT);
  MGAG_GC_OUT32(mgag_io, 0x40000001, MACCESS);
  MGAG_GC_OUT32(mgag_io, 0x800C4074, DWGCTL); /* No Z compare */
#endif
  /* Starts execution of the context primary dma (precomputed area) */
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_fast.bus
		| PRIMADDRESS_DMAMOD_GENERAL_WRITE,
		PRIMADDRESS);
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_fast.bus
		+ mgag_ctx->aperture_fast.size,
		PRIMEND);
}

/* ----------------------------------------------------------------------------
**	Drawing engine specific functions
** ----------------------------------------------------------------------------
*/
static void mgag_chipset_engine_do_buffer(mgag_chipset_t *mgag,
					  mgag_chipset_io_t *mgag_io,
					  kgi_accel_buffer_t *buffer,
					  mgag_chipset_accel_context_t *mgag_ctx)
{
  /* Sets start adress and end address in the primary DMA list */
  mgag_ctx->dma_fast.secaddress = (buffer->aperture.bus
				      + MGAG_ACCEL_TAG_LENGTH) /* skip tag */
    | SECADDRESS_DMAMOD_GENERAL_WRITE;
  mgag_ctx->dma_fast.secend = buffer->aperture.bus + buffer->execution.size;
  if (mgag->flags & MGAG_CF_AGP_ENABLED)
    {
#if 0
      /* Does not work yet !!! */
      KRN_DEBUG(1, "Using AGP");
      mgag_ctx->dma_fast.secend |= SECEND_PAGPXFER;
#endif
    }
  mgag_ctx->dma_fast.softrap = MGAG_SOFTRAP_ENGINE;
  KRN_DEBUG(2,"Executing one accel buffer "
	    "(primaddress=%.8x,primend=%.8x,"
	    "secaddress=%8.x,secend=%.8x,size=%.4x)",
	    mgag_ctx->aperture_fast.bus,
	    mgag_ctx->aperture_fast.bus + mgag_ctx->aperture_fast.size,
	    buffer->aperture.bus + MGAG_ACCEL_TAG_LENGTH,
	    buffer->aperture.bus + buffer->execution.size,
	    buffer->execution.size);
  /* Starts execution of the context primary dma (precomputed area) */
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_fast.bus
		| PRIMADDRESS_DMAMOD_GENERAL_WRITE,
		PRIMADDRESS);
  MGAG_GC_OUT32(mgag_io, ((mgag_ctx->aperture_fast.bus
			  + mgag_ctx->aperture_fast.size)
			  & PRIMEND_ADDRESS_MASK),
		PRIMEND);
}

static void mgag_chipset_g200_do_buffer_ctx(mgag_chipset_io_t *mgag_io,
					    kgi_accel_buffer_t *buffer,
					    mgag_chipset_accel_context_t *mgag_ctx)
{
  /* Sets start adress and end address in the primary DMA list */
  mgag_ctx->dma_with_context.g200.secaddress = (buffer->aperture.bus
				   + MGAG_ACCEL_TAG_LENGTH) /* skip tag */
    | SECADDRESS_DMAMOD_GENERAL_WRITE;
  mgag_ctx->dma_with_context.g200.secend = buffer->aperture.bus + buffer->execution.size;
  mgag_ctx->dma_with_context.g200.softrap = MGAG_SOFTRAP_ENGINE;
  KRN_DEBUG(2,"Executing one accel buffer "
	    "(primaddress=%.8x,primend=%.8x,"
	    "secaddress=%8.x,secend=%.8x,size=%.4x)",
	    mgag_ctx->aperture_with_ctx.bus,
	    mgag_ctx->aperture_with_ctx.bus + mgag_ctx->aperture_with_ctx.size,
	    buffer->aperture.bus + MGAG_ACCEL_TAG_LENGTH,
	    buffer->aperture.bus + buffer->execution.size,
	    buffer->execution.size);
  KRN_DEBUG(2,"pitch = %.8x dwgctl=%.8x",
	    mgag_ctx->dma_with_context.g200.pitch,
	    mgag_ctx->dma_with_context.g200.dwgctl);
  /* Starts execution of the context primary dma (precomputed area) */
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_with_ctx.bus
		| PRIMADDRESS_DMAMOD_GENERAL_WRITE,
		PRIMADDRESS);
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_with_ctx.bus
		+ mgag_ctx->aperture_with_ctx.size,
		PRIMEND);
}

static void mgag_chipset_g400_do_buffer_ctx(mgag_chipset_io_t *mgag_io,
					    kgi_accel_buffer_t *buffer,
					    mgag_chipset_accel_context_t *mgag_ctx)
{
  /* Sets start adress and end address in the primary DMA list */
  mgag_ctx->dma_with_context.g400.secaddress = (buffer->aperture.bus
				   + MGAG_ACCEL_TAG_LENGTH) /* skip tag */
    | SECADDRESS_DMAMOD_GENERAL_WRITE;
  mgag_ctx->dma_with_context.g400.secend = buffer->aperture.bus + buffer->execution.size;
  mgag_ctx->dma_with_context.g400.softrap = MGAG_SOFTRAP_ENGINE;
  KRN_DEBUG(2,"Executing one accel buffer "
	    "(primaddress=%.8x,primend=%.8x,"
	    "secaddress=%8.x,secend=%.8x,size=%.4x)",
	    mgag_ctx->aperture_with_ctx.bus,
	    mgag_ctx->aperture_with_ctx.bus + mgag_ctx->aperture_with_ctx.size,
	    buffer->aperture.bus + MGAG_ACCEL_TAG_LENGTH,
	    buffer->aperture.bus + buffer->execution.size,
	    buffer->execution.size);
  KRN_DEBUG(2,"pitch = %.8x dwgctl=%.8x",
	    mgag_ctx->dma_with_context.g400.pitch,
	    mgag_ctx->dma_with_context.g400.dwgctl);
  /* Starts execution of the context primary dma (precomputed area) */
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_with_ctx.bus
		| PRIMADDRESS_DMAMOD_GENERAL_WRITE,
		PRIMADDRESS);
  MGAG_GC_OUT32(mgag_io, mgag_ctx->aperture_with_ctx.bus
		+ mgag_ctx->aperture_with_ctx.size,
		PRIMEND);
}

/* ----------------------------------------------------------------------------
**	Validation/state persistence specific functions
** ----------------------------------------------------------------------------
*/
static void mga_chipset_accel_1x64_check(mga_chipset_1x64_context_t *ctx,
					 kgi_u8_t *reg, kgi_u32_t *val)
{
  kgi_u_t has_go = 0;
  kgi_u8_t regv = *reg;
  KRN_DEBUG(5, "Checking reg_ptr=%p (%.2x)"
	    " val_ptr=%p (%.8x) ctx=%p",
	    reg, *reg, val, *val, ctx);
  /* Check overall range */
  if (*reg >= 0x80)
    {
      KRN_DEBUG(2,"Probable access to Gx00-specific register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD);
      return; /* Early return */
    }

  /* Checks if this is a "GO" register */
  if ((*reg >= 0x40) && (*reg <= 0x7F))
    {
      has_go = 1;
      regv &= ~0x40;
      KRN_DEBUG(2, "We have an ACCEL_GO reg_ptr=%p (%.2x aka %.2x)",
		reg, *reg, regv);
    }
  /*
   * Saves/validates
   */
  switch(regv)
    {
      /* Regs are usually ordered by increasing index value */
      /*
       * Registers (or alias) recorded in state.
       */
    case MGA_DMA(DWGCTL):
      ctx->dwgctl = *val;
      if ((*val) & DWGCTL_SGNZERO)
	ctx->sgn = 0x0;
      if ((*val) & DWGCTL_SHFTZERO)
	ctx->shift = 0x0;
      if ((*val) & DWGCTL_SOLID)
	{
	  ctx->src0_or_pat0 = 0xFFFFFFFF;
	  ctx->src1_or_pat1 = 0xFFFFFFFF;
	  ctx->src2 = 0xFFFFFFFF;
	  ctx->src3 = 0xFFFFFFFF;
	  ctx->indexB = MGA_1x64_CONTEXT_INDEXB_SRC;
	}
      if ((*val) & DWGCTL_ARZERO)
	{
	  ctx->ar0 = 0x0;
	  ctx->ar1 = 0x0;
	  ctx->ar2 = 0x0;
	  ctx->ar3 = 0x0;
	  ctx->ar4 = 0x0;
	  ctx->ar5 = 0x0;
	  ctx->ar6 = 0x0;
	  M1x64_ACTIVATE_AR0(ctx);
	  M1x64_ACTIVATE_AR2(ctx);
	  M1x64_ACTIVATE_AR5(ctx);
	  M1x64_ACTIVATE_AR6(ctx);
	}
      break;
    case MGA_DMA(MACCESS):
      ctx->maccess = *val;
      break;
    case MGA_DMA(ZORG):
      ctx->zorg = *val;
      break;
    case MGA_DMA(PLNWT):
      ctx->plnwt = *val;
      break;
    case MGA_DMA(BCOL):
      ctx->bcol = *val;
      break;
    case MGA_DMA(FCOL):
      ctx->fcol = *val;
      break;
    case MGA_DMA(SRC0):
      ctx->indexB = MGA_1x64_CONTEXT_INDEXB_SRC;
      ctx->src0_or_pat0 = *val;
      break;
    case MGA_DMA(SRC1):
      ctx->indexB = MGA_1x64_CONTEXT_INDEXB_SRC;
      ctx->src1_or_pat1 = *val;
      break;
    case MGA_DMA(SRC2):
      ctx->indexB = MGA_1x64_CONTEXT_INDEXB_SRC;
      ctx->src2 = *val;
      break;
    case MGA_DMA(SRC3):
      ctx->indexB = MGA_1x64_CONTEXT_INDEXB_SRC;
      ctx->src3 = *val;
      break;
    case MGA_DMA(PAT0):
      ctx->indexB = MGA_1x64_CONTEXT_INDEXB_PAT;
      ctx->src0_or_pat0 = *val;
      break;
    case MGA_DMA(PAT1):
      ctx->indexB = MGA_1x64_CONTEXT_INDEXB_PAT;
      ctx->src1_or_pat1 = *val;
      break;
    case MGA_DMA(SHIFT):
      ctx->shift = *val;
      break;
    case MGA_DMA(SGN):
      ctx->sgn = *val;
      break;
    case MGA_DMA(AR0):
      ctx->ar0 = *val;
      M1x64_ACTIVATE_AR0(ctx);
      break;
    case MGA_DMA(AR1):
      ctx->ar1 = *val;
      break;
    case MGA_DMA(AR2):
      ctx->ar2 = *val;
      M1x64_ACTIVATE_AR2(ctx);
      break;
    case MGA_DMA(AR3):
      ctx->ar3 = *val;
      break;
    case MGA_DMA(AR4):
      ctx->ar4 = *val;
      break;
    case MGA_DMA(AR5):
      ctx->ar5 = *val;
      M1x64_ACTIVATE_AR5(ctx);
      break;
    case MGA_DMA(AR6):
      ctx->ar6 = *val;
      M1x64_ACTIVATE_AR6(ctx);
      break;
    case MGA_DMA(XDST):
      ctx->xdst = *val;
      M1x64_ACTIVATE_XDST(ctx);
      break;
    case MGA_DMA(YDST):
      ctx->ydst = *val;
      M1x64_ACTIVATE_YDST(ctx);
      break;
    case MGA_DMA(LEN):
      ctx->len = *val;
      M1x64_ACTIVATE_LEN(ctx);
      break;
    case MGA_DMA(XYSTRT):
      ctx->xystrt = *val;
      M1x64_XYSTRT_AFTER_YDSTLEN(ctx);
      M1x64_PASSIVATE_AR5_AR6_XDST_YDST(ctx);
      break;
    case MGA_DMA(XYEND):
      ctx->xyend = *val;
      M1x64_PASSIVATE_AR0_AR2(ctx);
      break;
    case MGA_DMA(YDSTLEN):
      ctx->ydstlen1 = *val;
      ctx->ydstlen2 = *val;
      M1x64_YDSTLEN_AFTER_XYSTRT(ctx);
      M1x64_PASSIVATE_YDST_LEN(ctx);
      break;
    case MGA_DMA(CXBNDRY): /* NB: Faster than CXLEFT and CXRIGHT */
      ctx->cxbndry = *val;
      break;
    case MGA_DMA(CXLEFT): /* NB: Slower than CXBNDRY */
      ctx->cxbndry &= ~CXBNDRY_CXLEFT_MASK;
      ctx->cxbndry |= ((*val) << CXBNDRY_CXLEFT_SHIFT) & CXBNDRY_CXLEFT_MASK;
      break;
    case MGA_DMA(CXRIGHT):
      ctx->cxbndry &= ~CXBNDRY_CXRIGHT_MASK;
      ctx->cxbndry |= ((*val) << CXBNDRY_CXRIGHT_SHIFT) & CXBNDRY_CXRIGHT_MASK;
      break;
    case MGA_DMA(FXBNDRY): /* NB: Faster than FXLEFT and FXRIGHT */
      ctx->fxbndry = *val;
      break;
    case MGA_DMA(FXLEFT): /* NB: Slower than FXBNDRY */
      ctx->fxbndry &= ~FXBNDRY_FXLEFT_MASK;
      ctx->fxbndry |= ((*val) << FXBNDRY_FXLEFT_SHIFT) & FXBNDRY_FXLEFT_MASK;
      break;
    case MGA_DMA(FXRIGHT): /* NB: Slower than FXBNDRY */
      ctx->fxbndry &= ~FXBNDRY_FXRIGHT_MASK;
      ctx->fxbndry |= ((*val) << FXBNDRY_FXRIGHT_SHIFT) & FXBNDRY_FXRIGHT_MASK;
      break;
    case MGA_DMA(PITCH):
      ctx->pitch = *val;
      break;
    case MGA_DMA(YTOP):
      ctx->ytop = *val;
      break;
    case MGA_DMA(YBOT):
      ctx->ybot = *val;
      break;
    case MGA_DMA(YDSTORG):
      ctx->ydstorg = *val;
      break;
      /*
       * Direct drawing registers
       */
    case MGA_DMA(DR0):
      ctx->dr0 = *val;
      break;
    case MGA_DMA(DR2):
      ctx->dr2 = *val;
      break;
    case MGA_DMA(DR3):
      ctx->dr3 = *val;
      break;
    case MGA_DMA(DR4):
      ctx->dr4 = *val;
      break;
    case MGA_DMA(DR6):
      ctx->dr6 = *val;
      break;
    case MGA_DMA(DR7):
      ctx->dr7 = *val;
      break;
    case MGA_DMA(DR8):
      ctx->dr8 = *val;
      break;
    case MGA_DMA(DR10):
      ctx->dr10 = *val;
      break;
    case MGA_DMA(DR11):
      ctx->dr11 = *val;
      break;
    case MGA_DMA(DR12):
      ctx->dr12 = *val;
      break;
    case MGA_DMA(DR14):
      ctx->dr14 = *val;
      break;
    case MGA_DMA(DR15):
      ctx->dr15 = *val;
      break;
      /*
       * Forbidden access registers: padded out
       */
    case MGA_DMA(MCTLWTST):
      KRN_DEBUG(1,"Trying to access a driver-only register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD);
      break;
      /*
       * Compatibility hack(s)
       */
    case MGA_DMA(DSTORG):
      /* Set to YDSTORG so that Gx00 code (hopefully) runs */
      (*reg) = MGA_DMA(YDSTORG);
      ctx->ydstorg = *val;
      KRN_DEBUG(1,"Changed DSTORG to YDSTORG! (Gx00 code?)");
      break;
      /*
       * Matrox reserved values: padded out
       */
    case 0x06:
    case 0x0A:
    case 0x0B:
    case 0x12:
    case 0x13:
    case 0x1F:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x31:
    case 0x35:
    case 0x39:
    case 0x3D:
      KRN_DEBUG(1,"Access to Matrox reserved register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD); 
     break;
    default:
      KRN_ERROR("Unhandled register for checking (DMA index: %.2x)."
		"Padded out. Please, inform " MAINTAINER ".", regv);
      (*reg) = MGA_DMA(DMAPAD); 
      break;
    }

  /* Restores the go flag (except on a pad) */
  if (has_go && ((*reg) != MGA_DMA(DMAPAD)))
    {
      (*reg) |= 0x40;
    }
}

static void mga_chipset_accel_g200_check(mga_chipset_g200_context_t *ctx,
					 kgi_u8_t *reg, kgi_u32_t *val)
{
  kgi_u_t has_go = 0;
  kgi_u8_t regv = *reg;
  KRN_DEBUG(5, "Checking reg_ptr=%p (%.2x)"
	    " val_ptr=%p (%.8x) ctx=%p",
	    reg, *reg, val, *val, ctx);
  /* Checks if this is a "GO" register */
  if ((*reg >= 0x40) && (*reg <= 0x6F))
    {
      has_go = 1;
      regv &= ~0x40;
      KRN_DEBUG(2, "We have an ACCEL_GO reg_ptr=%p (%.2x aka %.2x)",
		reg, *reg, regv);
    }
  /*
   * Saves/validates
   */
  switch(regv)
    {
      /* Regs are usually ordered by increasing index value */
      /*
       * Registers (or alias) recorded in state.
       */
    case MGA_DMA(DWGCTL):
      ctx->dwgctl = *val;
      if ((*val) & DWGCTL_SGNZERO)
	ctx->sgn = 0x0;
      if ((*val) & DWGCTL_SHFTZERO)
	ctx->shift = 0x0;
      if ((*val) & DWGCTL_SOLID)
	{
	  ctx->src0_or_pat0 = 0xFFFFFFFF;
	  ctx->src1_or_pat1 = 0xFFFFFFFF;
	  ctx->src2 = 0xFFFFFFFF;
	  ctx->src3 = 0xFFFFFFFF;
	  ctx->indexB = MGA_G200_CONTEXT_INDEXB_SRC;
	}
      if ((*val) & DWGCTL_ARZERO)
	{
	  ctx->ar0 = 0x0;
	  ctx->ar1 = 0x0;
	  ctx->ar2 = 0x0;
	  ctx->ar3 = 0x0;
	  ctx->ar4 = 0x0;
	  ctx->ar5 = 0x0;
	  ctx->ar6 = 0x0;
	  G200_ACTIVATE_AR0(ctx);
	  G200_ACTIVATE_AR2(ctx);
	  G200_ACTIVATE_AR5(ctx);
	  G200_ACTIVATE_AR6(ctx);
	}
      break;
    case MGA_DMA(MACCESS):
      ctx->maccess = *val;
      break;
    case MGA_DMA(ZORG):
      ctx->zorg = *val;
      break;
    case MGA_DMA(PLNWT):
      ctx->plnwt = *val;
      break;
    case MGA_DMA(BCOL):
      ctx->bcol = *val;
      break;
    case MGA_DMA(FCOL):
      ctx->fcol = *val;
      break;
    case MGA_DMA(SRC0):
      ctx->indexB = MGA_G200_CONTEXT_INDEXB_SRC;
      ctx->src0_or_pat0 = *val;
      break;
    case MGA_DMA(SRC1):
      ctx->indexB = MGA_G200_CONTEXT_INDEXB_SRC;
      ctx->src1_or_pat1 = *val;
      break;
    case MGA_DMA(SRC2):
      ctx->indexB = MGA_G200_CONTEXT_INDEXB_SRC;
      ctx->src2 = *val;
      break;
    case MGA_DMA(SRC3):
      ctx->indexB = MGA_G200_CONTEXT_INDEXB_SRC;
      ctx->src3 = *val;
      break;
    case MGA_DMA(PAT0):
      ctx->indexB = MGA_G200_CONTEXT_INDEXB_PAT;
      ctx->src0_or_pat0 = *val;
      break;
    case MGA_DMA(PAT1):
      ctx->indexB = MGA_G200_CONTEXT_INDEXB_PAT;
      ctx->src1_or_pat1 = *val;
      break;
    case MGA_DMA(SHIFT):
      ctx->shift = *val;
      break;
    case MGA_DMA(SGN):
      ctx->sgn = *val;
      break;
    case MGA_DMA(AR0):
      ctx->ar0 = *val;
      G200_ACTIVATE_AR0(ctx);
      break;
    case MGA_DMA(AR1):
      ctx->ar1 = *val;
      break;
    case MGA_DMA(AR2):
      ctx->ar2 = *val;
      G200_ACTIVATE_AR2(ctx);
      break;
    case MGA_DMA(AR3):
      ctx->ar3 = *val;
      break;
    case MGA_DMA(AR4):
      ctx->ar4 = *val;
      break;
    case MGA_DMA(AR5):
      ctx->ar5 = *val;
      G200_ACTIVATE_AR5(ctx);
      break;
    case MGA_DMA(AR6):
      ctx->ar6 = *val;
      G200_ACTIVATE_AR6(ctx);
      break;
    case MGA_DMA(XDST):
      ctx->xdst = *val;
      G200_ACTIVATE_XDST(ctx);
      break;
    case MGA_DMA(YDST):
      ctx->ydst = *val;
      G200_ACTIVATE_YDST(ctx);
      break;
    case MGA_DMA(LEN):
      ctx->len = *val;
      G200_ACTIVATE_LEN(ctx);
      break;
    case MGA_DMA(XYSTRT):
      ctx->xystrt = *val;
      G200_XYSTRT_AFTER_YDSTLEN(ctx);
      G200_PASSIVATE_AR5_AR6_XDST_YDST(ctx);
      break;
    case MGA_DMA(XYEND):
      ctx->xyend = *val;
      G200_PASSIVATE_AR0_AR2(ctx);
      break;
    case MGA_DMA(YDSTLEN):
      ctx->ydstlen1 = *val;
      ctx->ydstlen2 = *val;
      G200_YDSTLEN_AFTER_XYSTRT(ctx);
      G200_PASSIVATE_YDST_LEN(ctx);
      break;
    case MGA_DMA(CXBNDRY): /* NB: Faster than CXLEFT and CXRIGHT */
      ctx->cxbndry = *val;
      break;
    case MGA_DMA(CXLEFT): /* NB: Slower than CXBNDRY */
      ctx->cxbndry &= ~CXBNDRY_CXLEFT_MASK;
      ctx->cxbndry |= ((*val) << CXBNDRY_CXLEFT_SHIFT) & CXBNDRY_CXLEFT_MASK;
      break;
    case MGA_DMA(CXRIGHT):
      ctx->cxbndry &= ~CXBNDRY_CXRIGHT_MASK;
      ctx->cxbndry |= ((*val) << CXBNDRY_CXRIGHT_SHIFT) & CXBNDRY_CXRIGHT_MASK;
      break;
    case MGA_DMA(FXBNDRY): /* NB: Faster than FXLEFT and FXRIGHT */
      ctx->fxbndry = *val;
      break;
    case MGA_DMA(FXLEFT): /* NB: Slower than FXBNDRY */
      ctx->fxbndry &= ~FXBNDRY_FXLEFT_MASK;
      ctx->fxbndry |= ((*val) << FXBNDRY_FXLEFT_SHIFT) & FXBNDRY_FXLEFT_MASK;
      break;
    case MGA_DMA(FXRIGHT): /* NB: Slower than FXBNDRY */
      ctx->fxbndry &= ~FXBNDRY_FXRIGHT_MASK;
      ctx->fxbndry |= ((*val) << FXBNDRY_FXRIGHT_SHIFT) & FXBNDRY_FXRIGHT_MASK;
      break;
    case MGA_DMA(PITCH):
      ctx->pitch = *val;
      break;
    case MGA_DMA(YTOP):
      ctx->ytop = *val;
      break;
    case MGA_DMA(YBOT):
      ctx->ybot = *val;
      break;
    case MGA_DMA(FOGCOL):
      ctx->fogcol = *val;
      break;
    case MGA_DMA(WFLAG):
      ctx->wflag = *val;
      break;
    case MGA_DMA(WGETMSB):
      ctx->wgetmsb = *val;
      break;
    case MGA_DMA(TEXORG):
      ctx->texorg = *val;
      break;
    case MGA_DMA(TEXWIDTH):
      ctx->texwidth = *val;
      ctx->warp24 = (*val) | 0x40; /* Enforces the | 0x40 */
      break;
    case MGA_DMA(TEXHEIGHT):
      ctx->texheight = *val;
      ctx->warp34 = (*val) | 0x40; /* Enforces the | 0x40 */
      break;
    case MGA_DMA(TEXCTL):
      ctx->texctl = *val;
      break;
    case MGA_DMA(TEXTRANS):
      ctx->textrans = *val;
      break;
    case MGA_DMA(TEXTRANSHIGH):
      ctx->textranshigh = *val;
      break;
    case MGA_DMA(TEXCTL2):
      ctx->texctl2 = *val;
      break;
    case MGA_DMA(TEXFILTER):
      ctx->texfilter = *val;
      break;
    case MGA_DMA(TEXBORDERCOL):
      ctx->texbordercol = *val;
      break;
    case MGA_DMA(ALPHACTRL):
      ctx->alphactrl = *val;
      break;
    case MGA_DMA(SRCORG):
      ctx->srcorg = *val;
      break;
    case MGA_DMA(DSTORG):
      ctx->dstorg = *val;
      break;
    case MGA_DMA(YDSTORG): /* 0x25: reserved on the G400 */
      /* NOTE: Use of DSTORG is recommended, but YDSTORG *must*
       * NOTE: be initialized!
       */
      ctx->ydstorg = *val;
      break;
    case MGA_DMA(TEXORG1):
      ctx->texorg1 = *val;
      break;
    case MGA_DMA(TEXORG2):
      ctx->texorg2 = *val;
      break;
    case MGA_DMA(TEXORG3):
      ctx->texorg3 = *val;
      break;
    case MGA_DMA(TEXORG4):
      ctx->texorg4 = *val;
      break;
    case MGA_DMA(WARPREG(24)):
      /* This driver wants TEXWIDTH to be programmed first! */
      if (((*val) & ~0x40) != (ctx->texwidth & ~0x40))
	{
	  KRN_DEBUG(1,"WARPREG(24) (%.8x) is different from"
		    " current TEXWIDTH value (%.8x): passivated."
		    " (Rq: check programming sequence order.)",
		    (*val), ctx->texwidth);
	  *val = ctx->texwidth | 0x40;
	}
#if 0 /* Not needed: already enforced by TEXWIDTH case */
      else
	{
	  ctx->warp24 = *val | 0x40; /* Enforces the | 0x40 */
	}
#endif
      break;
    case MGA_DMA(WARPREG(34)):
      /* This driver wants TEXHEIGHT to be programmed first! */
      if (((*val) & ~0x40) != (ctx->texheight & ~0x40))
	{
	  KRN_DEBUG(1,"WARPREG(34) (%.8x) is different from"
		    " current TEXHEIGHT value (%.8x): passivated."
		    " (Rq: check programming sequence order.)",
		    (*val), ctx->texheight);
	  *val = ctx->texheight | 0x40;
	}
#if 0 /* Not needed: already enforced by TEXHEIGHT case */
      else
	ctx->warp34 = *val | 0x40; /* Enforces the | 0x40 */
#endif
      break;
      /*
       * Some registers that need checking
       */
    case MGA_DMA(WARPREG(42)):
    case MGA_DMA(WARPREG(60)):
      if ((*val) != 0x0000FFFF)
	{
	  KRN_DEBUG(1,"WARPREG(%i) should be set to 0x0000FFFF only:"
		    " padded out!",
		    regv - MGA_DMA(WARPREG(0)));
	  *reg = MGA_DMA(DMAPAD);
	}
      break;
    case MGA_DMA(WARPREG(25)):
      if ((*val) != 0x00000100)
	{
	  KRN_DEBUG(1,"WARPREG(25) should be set to 0x00000100 only:"
		    " padded out!");
	  *reg = MGA_DMA(DMAPAD);
	}
      break;
      /*
       * Direct drawing registers: allowed but not saved in state
       * accross different DMA buffers.
       */
      /* None */
      /*
       * Registers usually computed by the WARP pipes.
       * Allowed with a warning, but not saved.
       */
    case MGA_DMA(DR4):
    case MGA_DMA(DR6):
    case MGA_DMA(DR7):
    case MGA_DMA(DR8):
    case MGA_DMA(DR10):
    case MGA_DMA(DR11):
    case MGA_DMA(DR12):
    case MGA_DMA(DR14):
    case MGA_DMA(DR15):
    case MGA_DMA(ALPHASTART):
    case MGA_DMA(ALPHAXINC):
    case MGA_DMA(ALPHAYINC):
    case MGA_DMA(DR0):
    case MGA_DMA(DR2):
    case MGA_DMA(DR3):
    case MGA_DMA(SPECRSTART):
    case MGA_DMA(SPECRXINC):
    case MGA_DMA(SPECRYINC):
    case MGA_DMA(SPECGSTART):
    case MGA_DMA(SPECGXINC):
    case MGA_DMA(SPECGYINC):
    case MGA_DMA(SPECBSTART):
    case MGA_DMA(SPECBXINC):
    case MGA_DMA(SPECBYINC):
    case MGA_DMA(FOGSTART):
    case MGA_DMA(FOGXINC):
    case MGA_DMA(FOGYINC):
    case MGA_DMA(TMR0):
    case MGA_DMA(TMR1):
    case MGA_DMA(TMR6):
    case MGA_DMA(TMR2):
    case MGA_DMA(TMR3):
    case MGA_DMA(TMR7):
    case MGA_DMA(TMR4):
    case MGA_DMA(TMR5):
    case MGA_DMA(TMR8):
      KRN_DEBUG(1,"Using a pipe-managed register (DMA index: %.2x)."
		" Normally, you do not need to use it.",
		regv);
      break;
      /*
       * Forbidden access registers: padded out
       */
    case MGA_DMA(MCTLWTST):
    case MGA_DMA(WIADDR):
    case MGA_DMA(WVRTXSZ): /* Constant - Pipe-dependent */
    case MGA_DMA(SECADDRESS):
    case MGA_DMA(SECEND):
    case MGA_DMA(SOFTRAP):
    case MGA_DMA(DWGSYNC):
    case MGA_DMA(SETUPADDRESS):
    case MGA_DMA(SETUPEND):
      KRN_DEBUG(1,"Trying to access a driver-only register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD);
      break;
      /*
       * WARP general registers: not to be used normally.
       */
    case MGA_DMA(WARPREG(0)):
    case MGA_DMA(WARPREG(1)):
    case MGA_DMA(WARPREG(2)):
    case MGA_DMA(WARPREG(3)):
    case MGA_DMA(WARPREG(4)):
    case MGA_DMA(WARPREG(5)):
    case MGA_DMA(WARPREG(6)):
    case MGA_DMA(WARPREG(7)):
    case MGA_DMA(WARPREG(8)):
    case MGA_DMA(WARPREG(9)):
    case MGA_DMA(WARPREG(10)):
    case MGA_DMA(WARPREG(11)):
    case MGA_DMA(WARPREG(12)):
    case MGA_DMA(WARPREG(13)):
    case MGA_DMA(WARPREG(14)):
    case MGA_DMA(WARPREG(15)):
    case MGA_DMA(WARPREG(16)):
    case MGA_DMA(WARPREG(17)):
    case MGA_DMA(WARPREG(18)):
    case MGA_DMA(WARPREG(19)):
    case MGA_DMA(WARPREG(20)):
    case MGA_DMA(WARPREG(21)):
    case MGA_DMA(WARPREG(22)):
    case MGA_DMA(WARPREG(23)):
    case MGA_DMA(WARPREG(26)):
    case MGA_DMA(WARPREG(27)):
    case MGA_DMA(WARPREG(28)):
    case MGA_DMA(WARPREG(29)):
    case MGA_DMA(WARPREG(30)):
    case MGA_DMA(WARPREG(31)):
    case MGA_DMA(WARPREG(32)):
    case MGA_DMA(WARPREG(33)):
    case MGA_DMA(WARPREG(35)):
    case MGA_DMA(WARPREG(36)):
    case MGA_DMA(WARPREG(37)):
    case MGA_DMA(WARPREG(38)):
    case MGA_DMA(WARPREG(39)):
    case MGA_DMA(WARPREG(40)):
    case MGA_DMA(WARPREG(41)):
    case MGA_DMA(WARPREG(43)):
    case MGA_DMA(WARPREG(44)):
    case MGA_DMA(WARPREG(45)):
    case MGA_DMA(WARPREG(46)):
    case MGA_DMA(WARPREG(47)):
    case MGA_DMA(WARPREG(48)):
    case MGA_DMA(WARPREG(49)):
    case MGA_DMA(WARPREG(50)):
    case MGA_DMA(WARPREG(51)):
    case MGA_DMA(WARPREG(52)):
    case MGA_DMA(WARPREG(53)):
    case MGA_DMA(WARPREG(54)):
    case MGA_DMA(WARPREG(55)):
    case MGA_DMA(WARPREG(56)):
    case MGA_DMA(WARPREG(57)):
    case MGA_DMA(WARPREG(58)):
    case MGA_DMA(WARPREG(59)):
    case MGA_DMA(WARPREG(61)):
    case MGA_DMA(WARPREG(62)):
    case MGA_DMA(WARPREG(63)):
      KRN_DEBUG(1,"WARPREG(%i) should not be used normally: padded out!",
		    regv - MGA_DMA(WARPREG(0)));
      *reg = MGA_DMA(DMAPAD);
      break;
      /*
       * Compatibility hack(s)
       */
      /* none */
      /*
       * Matrox reserved values: padded out
       */
    case 0x06:
    case 0x0A:
    case 0x0B:
    case 0x12:
    case 0x13:
    case 0x1F:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
    case 0x7F:
    case 0xAF:
    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB6:
    case 0xB7:
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF:
      KRN_DEBUG(1,"Access to Matrox reserved register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD);
      break;
    default:
      KRN_ERROR("Unhandled register for checking (DMA index: %.2x)."
		"Please, inform " MAINTAINER ".", regv);
      break;
    }

  /* Restores the go flag (except on a pad) */
  if (has_go && ((*reg) != MGA_DMA(DMAPAD)))
    {
      (*reg) |= 0x40;
    }
}

static void mga_chipset_accel_g400_check(mga_chipset_g400_context_t *ctx,
					 kgi_u8_t *reg, kgi_u32_t *val)
{
  kgi_u_t has_go = 0;
  kgi_u8_t regv = *reg;
  KRN_DEBUG(5, "Checking reg_ptr=%p (%.2x)"
	    " val_ptr=%p (%.8x) ctx=%p",
	    reg, *reg, val, *val, ctx);
  /* Checks if this is a "GO" register */
  if ((*reg >= 0x40) && (*reg <= 0x6F))
    {
      has_go = 1;
      regv &= ~0x40;
      KRN_DEBUG(2, "We have an ACCEL_GO reg_ptr=%p (%.2x aka %.2x)",
		reg, *reg, regv);
    }
  /*
   * Saves/validates
   */
  switch(regv)
    {
      /* Regs are usually ordered by increasing index value */
      /*
       * Registers (or alias) recorded in state.
       */
    case MGA_DMA(DWGCTL):
      ctx->dwgctl = *val;
      if ((*val) & DWGCTL_SGNZERO)
	ctx->sgn = 0x0;
      if ((*val) & DWGCTL_SHFTZERO)
	ctx->shift = 0x0;
      if ((*val) & DWGCTL_SOLID)
	{
	  ctx->src0_or_pat0 = 0xFFFFFFFF;
	  ctx->src1_or_pat1 = 0xFFFFFFFF;
	  ctx->src2 = 0xFFFFFFFF;
	  ctx->src3 = 0xFFFFFFFF;
	  ctx->indexB = MGA_G400_CONTEXT_INDEXB_SRC;
	}
      if ((*val) & DWGCTL_ARZERO)
	{
	  ctx->ar0 = 0x0;
	  ctx->ar1 = 0x0;
	  ctx->ar2 = 0x0;
	  ctx->ar3 = 0x0;
	  ctx->ar4 = 0x0;
	  ctx->ar5 = 0x0;
	  ctx->ar6 = 0x0;
	  G400_ACTIVATE_AR0(ctx);
	  G400_ACTIVATE_AR2(ctx);
	  G400_ACTIVATE_AR5(ctx);
	  G400_ACTIVATE_AR6(ctx);
	}
      break;
    case MGA_DMA(MACCESS):
      ctx->maccess = *val;
      break;
    case MGA_DMA(ZORG):
      ctx->zorg = *val;
      break;
    case MGA_DMA(PLNWT):
      ctx->plnwt = *val;
      break;
    case MGA_DMA(BCOL):
      ctx->bcol = *val;
      break;
    case MGA_DMA(FCOL):
      ctx->fcol = *val;
      break;
    case MGA_DMA(SRC0):
      ctx->indexB = MGA_G400_CONTEXT_INDEXB_SRC;
      ctx->src0_or_pat0 = *val;
      break;
    case MGA_DMA(SRC1):
      ctx->indexB = MGA_G400_CONTEXT_INDEXB_SRC;
      ctx->src1_or_pat1 = *val;
      break;
    case MGA_DMA(SRC2):
      ctx->indexB = MGA_G400_CONTEXT_INDEXB_SRC;
      ctx->src2 = *val;
      break;
    case MGA_DMA(SRC3):
      ctx->indexB = MGA_G400_CONTEXT_INDEXB_SRC;
      ctx->src3 = *val;
      break;
    case MGA_DMA(PAT0):
      ctx->indexB = MGA_G400_CONTEXT_INDEXB_PAT;
      ctx->src0_or_pat0 = *val;
      break;
    case MGA_DMA(PAT1):
      ctx->indexB = MGA_G400_CONTEXT_INDEXB_PAT;
      ctx->src1_or_pat1 = *val;
      break;
    case MGA_DMA(SHIFT):
      ctx->shift = *val;
      break;
    case MGA_DMA(SGN):
      ctx->sgn = *val;
      break;
    case MGA_DMA(AR0):
      ctx->ar0 = *val;
      G400_ACTIVATE_AR0(ctx);
      break;
    case MGA_DMA(AR1):
      ctx->ar1 = *val;
      break;
    case MGA_DMA(AR2):
      ctx->ar2 = *val;
      G400_ACTIVATE_AR2(ctx);
      break;
    case MGA_DMA(AR3):
      ctx->ar3 = *val;
      break;
    case MGA_DMA(AR4):
      ctx->ar4 = *val;
      break;
    case MGA_DMA(AR5):
      ctx->ar5 = *val;
      G400_ACTIVATE_AR5(ctx);
      break;
    case MGA_DMA(AR6):
      ctx->ar6 = *val;
      G400_ACTIVATE_AR6(ctx);
      break;
    case MGA_DMA(XDST):
      ctx->xdst = *val;
      G400_ACTIVATE_XDST(ctx);
      break;
    case MGA_DMA(YDST):
      ctx->ydst = *val;
      G400_ACTIVATE_YDST(ctx);
      break;
    case MGA_DMA(LEN):
      ctx->len = *val;
      G400_ACTIVATE_LEN(ctx);
      break;
    case MGA_DMA(XYSTRT):
      ctx->xystrt = *val;
      G400_XYSTRT_AFTER_YDSTLEN(ctx);
      G400_PASSIVATE_AR5_AR6_XDST_YDST(ctx);
      break;
    case MGA_DMA(XYEND):
      ctx->xyend = *val;
      G400_PASSIVATE_AR0_AR2(ctx);
      break;
    case MGA_DMA(YDSTLEN):
      ctx->ydstlen1 = *val;
      ctx->ydstlen2 = *val;
      G400_YDSTLEN_AFTER_XYSTRT(ctx);
      G400_PASSIVATE_YDST_LEN(ctx);
      break;
    case MGA_DMA(CXBNDRY): /* NB: Faster than CXLEFT and CXRIGHT */
      ctx->cxbndry = *val;
      break;
    case MGA_DMA(CXLEFT): /* NB: Slower than CXBNDRY */
      ctx->cxbndry &= ~CXBNDRY_CXLEFT_MASK;
      ctx->cxbndry |= ((*val) << CXBNDRY_CXLEFT_SHIFT) & CXBNDRY_CXLEFT_MASK;
      break;
    case MGA_DMA(CXRIGHT):
      ctx->cxbndry &= ~CXBNDRY_CXRIGHT_MASK;
      ctx->cxbndry |= ((*val) << CXBNDRY_CXRIGHT_SHIFT) & CXBNDRY_CXRIGHT_MASK;
      break;
    case MGA_DMA(FXBNDRY): /* NB: Faster than FXLEFT and FXRIGHT */
      ctx->fxbndry = *val;
      break;
    case MGA_DMA(FXLEFT): /* NB: Slower than FXBNDRY */
      ctx->fxbndry &= ~FXBNDRY_FXLEFT_MASK;
      ctx->fxbndry |= ((*val) << FXBNDRY_FXLEFT_SHIFT) & FXBNDRY_FXLEFT_MASK;
      break;
    case MGA_DMA(FXRIGHT): /* NB: Slower than FXBNDRY */
      ctx->fxbndry &= ~FXBNDRY_FXRIGHT_MASK;
      ctx->fxbndry |= ((*val) << FXBNDRY_FXRIGHT_SHIFT) & FXBNDRY_FXRIGHT_MASK;
      break;
    case MGA_DMA(PITCH):
      ctx->pitch = *val;
      break;
    case MGA_DMA(YTOP):
      ctx->ytop = *val;
      break;
    case MGA_DMA(YBOT):
      ctx->ybot = *val;
      break;
    case MGA_DMA(FOGCOL):
      ctx->fogcol = *val;
      break;
    case MGA_DMA(WFLAG):
      ctx->wflag = *val;
      break;
    case MGA_DMA(WFLAG1):
      ctx->wflag1 = *val;
      break;
    case MGA_DMA(WGETMSB):
      ctx->wgetmsb = *val;
      break;
    case MGA_DMA(WACCEPTSEQ):
      /* TODO: clean up hex values! */
      if (((*val) != 0x18000000)
	  && ((*val) != 0x02010200)
	  && ((*val) != 0x01000408)
	  && ((*val) != 0x02020400)
	  && ((*val) != 0x01000810))
	{
	  KRN_DEBUG(1,"Incorrect WACCEPTSEQ value (%.8x): padded out",(*val));
	  (*reg) = MGA_DMA(DMAPAD);
	}
      else
	{
	  ctx->wacceptseq = *val;
	}
      break;
    case MGA_DMA(TEXORG):
      ctx->texorg = *val;
      break;
    case MGA_DMA(TEXWIDTH):
      ctx->texwidth = *val;
      ctx->warp54 = (*val) | 0x40; /* Enforces the | 0x40 */
      ctx->warp52 = (*val) | 0x40;
      break;
    case MGA_DMA(TEXHEIGHT):
      ctx->texheight = *val;
      ctx->warp62 = (*val) | 0x40; /* Enforces the | 0x40 */
      break;
    case MGA_DMA(TEXCTL):
      ctx->texctl = *val;
      break;
    case MGA_DMA(TEXTRANS):
      ctx->textrans = *val;
      break;
    case MGA_DMA(TEXTRANSHIGH):
      ctx->textranshigh = *val;
      break;
    case MGA_DMA(TEXCTL2):
      ctx->texctl2 = *val;
      break;
    case MGA_DMA(TEXFILTER):
      ctx->texfilter = *val;
      break;
    case MGA_DMA(TEXBORDERCOL):
      ctx->texbordercol = *val;
      break;
    case MGA_DMA(ALPHACTRL):
      ctx->alphactrl = *val;
      break;
    case MGA_DMA(SRCORG):
      ctx->srcorg = *val;
      break;
    case MGA_DMA(DSTORG):
      ctx->dstorg = *val;
      break;
    case MGA_DMA(TEXORG1):
      ctx->texorg1 = *val;
      break;
    case MGA_DMA(TEXORG2):
      ctx->texorg2 = *val;
      break;
    case MGA_DMA(TEXORG3):
      ctx->texorg3 = *val;
      break;
    case MGA_DMA(TEXORG4):
      ctx->texorg4 = *val;
      break;
    case MGA_DMA(STENCIL):
      ctx->stencil = *val;
      break;
    case MGA_DMA(STENCILCTL):
      ctx->stencilctl = *val;
      break;
    case MGA_DMA(TBUMPMAT):
      ctx->tbumpmat = *val;
      break;
    case MGA_DMA(TBUMPFMT):
      ctx->tbumpfmt = *val;
      break;
    case MGA_DMA(TDUALSTAGE0):
      ctx->tdualstage0 = *val;
      break;
    case MGA_DMA(TDUALSTAGE1):
      ctx->tdualstage1 = *val;
      break;
    case MGA_DMA(WARPREG(54)):
      /* This driver wants TEXWIDTH to be programmed first! */
      if (((*val) & ~0x40) != (ctx->texwidth & ~0x40))
	{
	  KRN_DEBUG(1,"WARPREG(54) (%.8x) is different from"
		    " current TEXWIDTH value (%.8x): passivated."
		    " (Rq: check programming sequence order.)",
		    (*val), ctx->texwidth);
	  *val = ctx->texwidth | 0x40;
	}
#if 0 /* Not needed: already enforced by TEXWIDTH case */
      else
	{
	  ctx->warp54 = *val | 0x40; /* Enforces the | 0x40 */
	  ctx->warp52 = *val | 0x40;
	}
#endif
      break;
    case MGA_DMA(WARPREG(62)):
      /* This driver wants TEXHEIGHT to be programmed first! */
      if (((*val) & ~0x40) != (ctx->texheight & ~0x40))
	{
	  KRN_DEBUG(1,"WARPREG(62) (%.8x) is different from"
		    " current TEXHEIGHT value (%.8x): passivated."
		    " (Rq: check programming sequence order.)",
		    (*val), ctx->texheight);
	  *val = ctx->texheight | 0x40;
	}
#if 0 /* Not needed: already enforced by TEXHEIGHT case */
      else
	ctx->warp62 = *val | 0x40; /* Enforces the | 0x40 */
#endif
      break;
    case MGA_DMA(WARPREG(52)):
      /* This driver wants TEXWIDTH to be programmed first! */
      if (((*val) & ~0x40) != (ctx->texwidth & ~0x40))
	{
	  KRN_DEBUG(1,"WARPREG(52) (%.8x) is different from"
		    " current TEXWIDTH value (%.8x): passivated."
		    " (Rq: check programming sequence order.)",
		    (*val), ctx->texwidth);
	  *val = ctx->texwidth | 0x40;
	}
#if 0 /* Not needed: already enforced by TEXWIDTH case */
      else
	{
	  ctx->warp52 = *val | 0x40; /* Enforces the | 0x40 */
	  ctx->warp54 = *val | 0x40;
	}
#endif
      break;
      /*
       * Some registers that need checking
       */
    case MGA_DMA(WARPREG(49)):
    case MGA_DMA(WARPREG(57)):
    case MGA_DMA(WARPREG(53)):
    case MGA_DMA(WARPREG(61)):
      if ((*val) != 0)
	{
	  KRN_DEBUG(1,"WARPREG(%i) should be set to 0 only: padded out!",
		    regv - MGA_DMA(WARPREG(0)));
	  *reg = MGA_DMA(DMAPAD);
	}
      break;
      /*
       * Direct drawing registers: allowed but not saved in state
       * accross different DMA buffers.
       */
      /* None */
      /*
       * Registers usually computed by the WARP pipes.
       * Allowed with a warning, but not saved.
       */
    case MGA_DMA(DR4):
    case MGA_DMA(DR6):
    case MGA_DMA(DR7):
    case MGA_DMA(DR8):
    case MGA_DMA(DR10):
    case MGA_DMA(DR11):
    case MGA_DMA(DR12):
    case MGA_DMA(DR14):
    case MGA_DMA(DR15):
    case MGA_DMA(ALPHASTART):
    case MGA_DMA(ALPHAXINC):
    case MGA_DMA(ALPHAYINC):
    case MGA_DMA(DR0):
    case MGA_DMA(DR2):
    case MGA_DMA(DR3):
    case MGA_DMA(SPECRSTART):
    case MGA_DMA(SPECRXINC):
    case MGA_DMA(SPECRYINC):
    case MGA_DMA(SPECGSTART):
    case MGA_DMA(SPECGXINC):
    case MGA_DMA(SPECGYINC):
    case MGA_DMA(SPECBSTART):
    case MGA_DMA(SPECBXINC):
    case MGA_DMA(SPECBYINC):
    case MGA_DMA(FOGSTART):
    case MGA_DMA(FOGXINC):
    case MGA_DMA(FOGYINC):
    case MGA_DMA(TMR0):
    case MGA_DMA(TMR1):
    case MGA_DMA(TMR6):
    case MGA_DMA(TMR2):
    case MGA_DMA(TMR3):
    case MGA_DMA(TMR7):
    case MGA_DMA(TMR4):
    case MGA_DMA(TMR5):
    case MGA_DMA(TMR8):
      KRN_DEBUG(1,"Using a pipe-managed register (DMA index: %.2x)."
		" Normally, you do not need to use it.",
		regv);
      break;
      /*
       * Forbidden access registers: padded out
       */
    case MGA_DMA(MCTLWTST):
    case MGA_DMA(WIADDR):
    case MGA_DMA(WVRTXSZ): /* Constant - Pipe-dependent */
    case MGA_DMA(WIADDR2):
    case MGA_DMA(SECADDRESS):
    case MGA_DMA(SECEND):
    case MGA_DMA(SOFTRAP):
    case MGA_DMA(DWGSYNC):
    case MGA_DMA(SETUPADDRESS):
    case MGA_DMA(SETUPEND):
    case MGA_DMA(WARPREG(56)): /* Constant - New pipe only */
      KRN_DEBUG(1,"Trying to access a driver-only register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD);
      break;
      /*
       * WARP general registers: not to be used normally.
       */
    case MGA_DMA(WARPREG(0)):
    case MGA_DMA(WARPREG(1)):
    case MGA_DMA(WARPREG(2)):
    case MGA_DMA(WARPREG(3)):
    case MGA_DMA(WARPREG(4)):
    case MGA_DMA(WARPREG(5)):
    case MGA_DMA(WARPREG(6)):
    case MGA_DMA(WARPREG(7)):
    case MGA_DMA(WARPREG(8)):
    case MGA_DMA(WARPREG(9)):
    case MGA_DMA(WARPREG(10)):
    case MGA_DMA(WARPREG(11)):
    case MGA_DMA(WARPREG(12)):
    case MGA_DMA(WARPREG(13)):
    case MGA_DMA(WARPREG(14)):
    case MGA_DMA(WARPREG(15)):
    case MGA_DMA(WARPREG(16)):
    case MGA_DMA(WARPREG(17)):
    case MGA_DMA(WARPREG(18)):
    case MGA_DMA(WARPREG(19)):
    case MGA_DMA(WARPREG(20)):
    case MGA_DMA(WARPREG(21)):
    case MGA_DMA(WARPREG(22)):
    case MGA_DMA(WARPREG(23)):
    case MGA_DMA(WARPREG(24)):
    case MGA_DMA(WARPREG(25)):
    case MGA_DMA(WARPREG(26)):
    case MGA_DMA(WARPREG(27)):
    case MGA_DMA(WARPREG(28)):
    case MGA_DMA(WARPREG(29)):
    case MGA_DMA(WARPREG(30)):
    case MGA_DMA(WARPREG(31)):
    case MGA_DMA(WARPREG(32)):
    case MGA_DMA(WARPREG(33)):
    case MGA_DMA(WARPREG(34)):
    case MGA_DMA(WARPREG(35)):
    case MGA_DMA(WARPREG(36)):
    case MGA_DMA(WARPREG(37)):
    case MGA_DMA(WARPREG(38)):
    case MGA_DMA(WARPREG(39)):
    case MGA_DMA(WARPREG(40)):
    case MGA_DMA(WARPREG(41)):
    case MGA_DMA(WARPREG(42)):
    case MGA_DMA(WARPREG(43)):
    case MGA_DMA(WARPREG(44)):
    case MGA_DMA(WARPREG(45)):
    case MGA_DMA(WARPREG(46)):
    case MGA_DMA(WARPREG(47)):
    case MGA_DMA(WARPREG(48)):
    case MGA_DMA(WARPREG(50)):
    case MGA_DMA(WARPREG(51)):
    case MGA_DMA(WARPREG(55)):
    case MGA_DMA(WARPREG(58)):
    case MGA_DMA(WARPREG(59)):
    case MGA_DMA(WARPREG(60)):
    case MGA_DMA(WARPREG(63)):
      KRN_DEBUG(1,"WARPREG(%i) should not be used normally: padded out!",
		    regv - MGA_DMA(WARPREG(0)));
      *reg = MGA_DMA(DMAPAD);
      break;
      /*
       * Compatibility hack(s)
       */
    case MGA_DMA(YDSTORG): /* 0x25: reserved on the G400 */
      /* Set to DSTORG so that Mystique/G200 code (hopefully) runs */
      (*reg) = MGA_DMA(DSTORG);
      ctx->dstorg = *val;
      KRN_DEBUG(1,"Changed YDSTORG to DSTORG! (1x64/G200 legacy?)");
      break;
      /*
       * Matrox reserved values: padded out
       */
    case 0x06:
    case 0x0A:
    case 0x0B:
    case 0x12:
    case 0x13:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x74:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
    case 0x7F:
    case 0xAF:
    case 0xB0:
    case 0xB1:
    case 0xB6:
    case 0xB7:
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
      KRN_DEBUG(1,"Access to Matrox reserved register (DMA index: %.2x)"
		": padded out!",
		regv);
      (*reg) = MGA_DMA(DMAPAD);
      break;
    default:
      KRN_ERROR("Unhandled register for checking (DMA index: %.2x)."
		"Please, inform " MAINTAINER ".", regv);
      break;
    }

  /* Restores the go flag (except on a pad) */
  if (has_go && ((*reg) != MGA_DMA(DMAPAD)))
    {
      (*reg) |= 0x40;
    }
}

/* Walks and validate the buffer: changes some commands if needed,
 * records the state-relevant modifications.
 * Returns 0 if ok for execution, 1 if not.
 */
static kgi_u_t mga_chipset_accel_validate(kgi_accel_t *accel,
					  kgi_accel_buffer_t *buffer)
{
  mgag_chipset_t *mgag = accel->meta;
  kgi_u32_t tag = (*(kgi_u32_t *)buffer->aperture.virt);
  kgi_u32_t *base_ptr = (kgi_u32_t*)buffer->aperture.virt + 1;
  kgi_u_t dword_size = (buffer->execution.size >> 2) - 1;
  mgag_chipset_accel_context_t *ctx = buffer->context;
  kgi_u_t ret = 1; /* Error by default */

  if ((buffer->execution.size & 0x3) || (buffer->aperture.bus & 0x3))
    {
      KRN_ERROR("Matrox: invalid buffer start adress (%.8x) or size (%.8x)",
		buffer->aperture.bus, buffer->execution.size);
      return ret; /* Early exit */
    }

  switch (tag)
    {
    case MGAG_ACCEL_TAG_DRAWING_ENGINE:
      {
	kgi_u_t current_index = 0;
	kgi_u_t current_value = 1;
	if (dword_size < 2)
	  {
	    if (buffer->execution.size == 4)
	      { /* We allow empty buffers */
		ret = 2;
	      }
	    else
	      {
		KRN_DEBUG(1, "General purpose DMA buffer"
			  " too small for execution (size=%i)",
			  buffer->execution.size);
		ret = 1; /* Buffer too small */
	      }
	    break;
	  }
	while ((current_index + current_value) < dword_size)
	  {
	    kgi_u32_t* val = base_ptr + current_index + current_value;
	    kgi_u8_t* reg = (kgi_u8_t*)(base_ptr + current_index)
	      + (current_value - 1);

	    /* One call for specific */
	    if ((mgag->flags & MGAG_CF_G400)
		/* TODO: We re-use the G400 validation for G550: check! */
		|| (mgag->flags & MGAG_CF_G550))
	      {
		mga_chipset_accel_g400_check(&(ctx->dma_with_context.g400),
					     reg, val);
	      }
	    else if (mgag->flags & MGAG_CF_G200)
	      {
		mga_chipset_accel_g200_check(&(ctx->dma_with_context.g200),
					     reg, val);
	      }
	    else if (mgag->flags & MGAG_CF_1x64)
	      {
		mga_chipset_accel_1x64_check(&(ctx->dma_with_context.m1x64),
					     reg, val);
	      }
	    else
	      {
		KRN_DEBUG(1, "Unknown chipset type for validation");
		return 1; /* Early exit */
	      }
	    /* NB: function should be able to change the reg */

	    current_value++;
	    if (current_value == 5)
	      {
		current_value = 1;
		current_index += 5;
	      }
	  }
	ret = 0;
      }
      break;
    case MGAG_ACCEL_TAG_WARP_TRIANGLE_LIST:
      if (dword_size < (3 * 8))
	{
	  KRN_DEBUG(1, "Skipping warp TGZ DMA buffer"
		    " too small for execution (size=%i)",
		    buffer->execution.size);
	  ret = 1;
	  break; /* Early exit */
	}
      ret = 0;
      break;
    default:
      KRN_DEBUG(1, "Unknown DMA buffer tag (%.8x)", tag);
      ret = 1;
      break;
    }
  return ret;
}

/* ----------------------------------------------------------------------------
**	Graphics accelerator engine
** ----------------------------------------------------------------------------
*/
static void mgag_chipset_accel_init(kgi_accel_t *accel, void *ctx)
{
  mgag_chipset_t *mgag = accel->meta;
  mgag_chipset_io_t *mgag_io = accel->meta_io;
  mgag_chipset_accel_context_t *mgag_ctx = ctx;
  kgi_size_t offset;

  /*
  ** Nothing to do on the Mystique for the moment -- ortalo
  */
  if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200)
      || (mgag->flags & MGAG_CF_G550))
    {
      /* To be able to use ctx->dma_fast for DMA we precalculate the
      ** apertures info needed to have it at hand when needed.
      */
      mgag_ctx->aperture_fast.size = sizeof(mgag_ctx->dma_fast);
      offset = (mem_vaddr_t) &mgag_ctx->dma_fast - (mem_vaddr_t) mgag_ctx;
      mgag_ctx->aperture_fast.bus  = mgag_ctx->kgi.aperture.bus  + offset;
      mgag_ctx->aperture_fast.phys = mgag_ctx->kgi.aperture.phys + offset;
      mgag_ctx->aperture_fast.virt = mgag_ctx->kgi.aperture.virt + offset;
      if ((mgag_ctx->aperture_fast.size & ~PRIMADDRESS_ADDRESS_MASK)
	  || (mgag_ctx->aperture_fast.bus & ~PRIMADDRESS_ADDRESS_MASK))
	{
	  KRN_ERROR("Matrox: invalid (fast) primary DMA start (%.8x) or size (%.8x)",
		    mgag_ctx->aperture_fast.bus, mgag_ctx->aperture_fast.size);
	  KRN_INTERNAL_ERROR;
	}
      /* Initialize the primary dma list used for sending buffers
      ** (via the secondary DMA)
      */
      mgag_ctx->dma_fast.index1 = MGA_DMA4(SECADDRESS,SECEND,DMAPAD,DMAPAD);
      mgag_ctx->dma_fast.secaddress = 0x00000000; /* value of SECADDRESS */
      mgag_ctx->dma_fast.secend = 0x00000000; /* value of SECEND */
      mgag_ctx->dma_fast.index2 = MGA_DMA4(SOFTRAP,DMAPAD,DMAPAD,DMAPAD);
      mgag_ctx->dma_fast.softrap = MGAG_SOFTRAP_ENGINE;

      if (mgag->flags & MGAG_CF_G200)
	{
	  /* To be able to use ctx->dma_with_context.g200 for DMA we
	  ** precalculate the apertures info needed to have it at hand
	  ** when needed.
	  */
	  mgag_ctx->aperture_with_ctx.size =
	    sizeof(mgag_ctx->dma_with_context.g200);
	  offset = (mem_vaddr_t) &(mgag_ctx->dma_with_context.g200)
	    - (mem_vaddr_t) mgag_ctx;
	  mgag_ctx->aperture_with_ctx.bus  = mgag_ctx->kgi.aperture.bus
	    + offset;
	  mgag_ctx->aperture_with_ctx.phys = mgag_ctx->kgi.aperture.phys
	    + offset;
	  mgag_ctx->aperture_with_ctx.virt = mgag_ctx->kgi.aperture.virt
	    + offset;
	  if ((mgag_ctx->aperture_with_ctx.size & ~PRIMADDRESS_ADDRESS_MASK)
	      || (mgag_ctx->aperture_with_ctx.bus & ~PRIMADDRESS_ADDRESS_MASK))
	    {
	      KRN_ERROR("Matrox: invalid (ctx) primary DMA start (%.8x) or size (%.8x)",
			mgag_ctx->aperture_with_ctx.bus,
			mgag_ctx->aperture_with_ctx.size);
	      KRN_INTERNAL_ERROR;
	    }
	  /* Initialize the primary dma context used for sending buffers
	  ** (via the secondary DMA)
	  */
	  mgag_ctx->dma_with_context.g200.index0 = MGA_G200_CONTEXT_INDEX0;
	  mgag_ctx->dma_with_context.g200.index1 = MGA_G200_CONTEXT_INDEX1;
	  mgag_ctx->dma_with_context.g200.index2 = MGA_G200_CONTEXT_INDEX2;
	  mgag_ctx->dma_with_context.g200.index3 = MGA_G200_CONTEXT_INDEX3;
	  mgag_ctx->dma_with_context.g200.index4 = MGA_G200_CONTEXT_INDEX4;
	  mgag_ctx->dma_with_context.g200.index5 = MGA_G200_CONTEXT_INDEX5;
	  mgag_ctx->dma_with_context.g200.index6 = MGA_G200_CONTEXT_INDEX6;
	  mgag_ctx->dma_with_context.g200.index7 = MGA_G200_CONTEXT_INDEX7;
	  mgag_ctx->dma_with_context.g200.index8 = MGA_G200_CONTEXT_INDEX8;
	  mgag_ctx->dma_with_context.g200.index9 = MGA_G200_CONTEXT_INDEX9;
	  mgag_ctx->dma_with_context.g200.indexA = MGA_G200_CONTEXT_INDEXA;
	  mgag_ctx->dma_with_context.g200.indexB = MGA_G200_CONTEXT_INDEXB_SRC;
	  mgag_ctx->dma_with_context.g200.indexC = MGA_G200_CONTEXT_INDEXC;
	  mgag_ctx->dma_with_context.g200.indexD = MGA_G200_CONTEXT_INDEXD;
	  mgag_ctx->dma_with_context.g200.indexE = MGA_G200_CONTEXT_INDEXE;
	  mgag_ctx->dma_with_context.g200.indexF = MGA_G200_CONTEXT_INDEXF;
	  mgag_ctx->dma_with_context.g200.index_sec =
	    MGA_DMA4(SECADDRESS,SECEND,DMAPAD,DMAPAD);
	  mgag_ctx->dma_with_context.g200.secaddress =
	    0x00000000; /* value of SECADDRESS */
	  mgag_ctx->dma_with_context.g200.secend =
	    0x00000000; /* value of SECEND */
	  mgag_ctx->dma_with_context.g200.index_softrap =
	    MGA_DMA4(SOFTRAP,DMAPAD,DMAPAD,DMAPAD);
	  mgag_ctx->dma_with_context.g200.softrap = MGAG_SOFTRAP_ENGINE;
	  /*
	   * Initialize the DMA context
	   */
	  mgag_ctx->dma_with_context.g200.maccess = MACCESS_ZWIDTH_ZW16
	    | MACCESS_PWIDTH_PW16;
	  mgag_ctx->dma_with_context.g200.pitch = 1024;
	  mgag_ctx->dma_with_context.g200.dstorg = 0;
	  mgag_ctx->dma_with_context.g200.ydstorg = 0; /* Mandatory! */
	  mgag_ctx->dma_with_context.g200.ybot = 0xFFFFF0;
	  mgag_ctx->dma_with_context.g200.plnwt = 0xFFFFFFFF;
	  mgag_ctx->dma_with_context.g200.zorg = 1024 * 768 * 2;
	  mgag_ctx->dma_with_context.g200.alphactrl = 0x1;

	  mgag_ctx->dma_with_context.g200.fcol = 0x12345678;
	  mgag_ctx->dma_with_context.g200.bcol = 0x11111111;
	  mgag_ctx->dma_with_context.g200.sgn = 0x00000000;
	  mgag_ctx->dma_with_context.g200.srcorg = 0x00000000;
	}
      else if ((mgag->flags & MGAG_CF_G400)
	       || (mgag->flags & MGAG_CF_G550))
	{
	  /* To be able to use ctx->dma_with_context.g400 for DMA we
	  ** precalculate the apertures info needed to have it at hand
	  ** when needed.
	  */
	  mgag_ctx->aperture_with_ctx.size =
	    sizeof(mgag_ctx->dma_with_context.g400);
	  offset = (mem_vaddr_t) &(mgag_ctx->dma_with_context.g400)
	    - (mem_vaddr_t) mgag_ctx;
	  mgag_ctx->aperture_with_ctx.bus  = mgag_ctx->kgi.aperture.bus
	    + offset;
	  mgag_ctx->aperture_with_ctx.phys = mgag_ctx->kgi.aperture.phys
	    + offset;
	  mgag_ctx->aperture_with_ctx.virt = mgag_ctx->kgi.aperture.virt
	    + offset;
	  if ((mgag_ctx->aperture_with_ctx.size & ~PRIMADDRESS_ADDRESS_MASK)
	      || (mgag_ctx->aperture_with_ctx.bus & ~PRIMADDRESS_ADDRESS_MASK))
	    {
	      KRN_ERROR("Matrox: invalid (ctx) primary DMA start (%.8x) or size (%.8x)",
			mgag_ctx->aperture_with_ctx.bus,
			mgag_ctx->aperture_with_ctx.size);
	      KRN_INTERNAL_ERROR;
	    }
	  /* Initialize the primary dma context used for sending buffers
	  ** (via the secondary DMA)
	  */
	  mgag_ctx->dma_with_context.g400.index0 = MGA_G400_CONTEXT_INDEX0;
	  mgag_ctx->dma_with_context.g400.index1 = MGA_G400_CONTEXT_INDEX1;
	  mgag_ctx->dma_with_context.g400.index2 = MGA_G400_CONTEXT_INDEX2;
	  mgag_ctx->dma_with_context.g400.index3 = MGA_G400_CONTEXT_INDEX3;
	  mgag_ctx->dma_with_context.g400.index4 = MGA_G400_CONTEXT_INDEX4;
	  mgag_ctx->dma_with_context.g400.index5 = MGA_G400_CONTEXT_INDEX5;
	  mgag_ctx->dma_with_context.g400.index6 = MGA_G400_CONTEXT_INDEX6;
	  mgag_ctx->dma_with_context.g400.index7 = MGA_G400_CONTEXT_INDEX7;
	  mgag_ctx->dma_with_context.g400.index8 = MGA_G400_CONTEXT_INDEX8;
	  mgag_ctx->dma_with_context.g400.index9 = MGA_G400_CONTEXT_INDEX9;
	  mgag_ctx->dma_with_context.g400.indexA = MGA_G400_CONTEXT_INDEXA;
	  mgag_ctx->dma_with_context.g400.indexB = MGA_G400_CONTEXT_INDEXB_SRC;
	  mgag_ctx->dma_with_context.g400.indexC = MGA_G400_CONTEXT_INDEXC;
	  mgag_ctx->dma_with_context.g400.indexD = MGA_G400_CONTEXT_INDEXD;
	  mgag_ctx->dma_with_context.g400.indexE = MGA_G400_CONTEXT_INDEXE;
	  mgag_ctx->dma_with_context.g400.indexF = MGA_G400_CONTEXT_INDEXF;
	  mgag_ctx->dma_with_context.g400.index_sec =
	    MGA_DMA4(SECADDRESS,SECEND,DMAPAD,DMAPAD);
	  mgag_ctx->dma_with_context.g400.secaddress =
	    0x00000000; /* value of SECADDRESS */
	  mgag_ctx->dma_with_context.g400.secend =
	    0x00000000; /* value of SECEND */
	  mgag_ctx->dma_with_context.g400.index_softrap =
	    MGA_DMA4(SOFTRAP,DMAPAD,DMAPAD,DMAPAD);
	  mgag_ctx->dma_with_context.g400.softrap = MGAG_SOFTRAP_ENGINE;
	  /*
	   * Initialize the DMA context
	   */
	  mgag_ctx->dma_with_context.g400.maccess = MACCESS_ZWIDTH_ZW16
	    | MACCESS_PWIDTH_PW16;
	  mgag_ctx->dma_with_context.g400.pitch = 1024;
	  mgag_ctx->dma_with_context.g400.dstorg = 0;
	  mgag_ctx->dma_with_context.g400.plnwt = 0xFFFFFFFF;
	  mgag_ctx->dma_with_context.g400.zorg = 1024 * 768 * 2;
	  mgag_ctx->dma_with_context.g400.alphactrl = 0x1;
	  mgag_ctx->dma_with_context.g400.tdualstage0 = 0;
	  mgag_ctx->dma_with_context.g400.tdualstage1 = 0;
	}
      /*
      ** Initialize the WARP pipe (setup engine)
      */
      mgag_chipset_warp_setup_pipe(mgag, mgag_io);
    }
}

static void mgag_chipset_accel_done(kgi_accel_t *accel, void *ctx)
{
  mgag_chipset_io_t *mgag_io = accel->meta_io;

  mgag_chipset_wait_engine_idle(mgag_io);
  if (ctx == accel->context)
    {
      accel->context = NULL;
    }
}

/* This must not be interrupted and must be protected! */
static void mgag_chipset_accel_schedule(kgi_accel_t *accel)
{
  mgag_chipset_t *mgag = accel->meta;
  mgag_chipset_io_t *mgag_io = accel->meta_io;
  kgi_accel_buffer_t *buffer = accel->execution.queue;
#if 0
  KRN_ASSERT(buffer);
#else
  if (buffer == NULL)
    {
      return;
    }
#endif

  switch (buffer->execution.state)
    {

    case KGI_AS_EXEC:
      /*	Execution of the current buffer finished (one per one), so we 
      **	mark it KGI_AS_IDLE and advance the queue.
      */
      buffer->execution.state = KGI_AS_IDLE;

      {
	kgi_accel_buffer_t *cur = buffer->execution.next;
	buffer->execution.next = NULL;
	accel->execution.queue = cur;

	KRN_DEBUG(2,"before wake_up: buffer:%p adress:%p value: %i",
		  buffer, &(buffer->execution.state), buffer->execution.state);

	kgi_wakeup(buffer->executed);

#if (HOST_OS == HOST_OS_FreeBSD)
	if (buffer->flags & KGI_BF_USEMUTEX)
	  kgi_mutex_signal(&buffer->mtx, 1);
#endif

	if (cur == NULL)
	  {
	    /* no further buffers queued, thus we are done.
	     */
	    accel->execution.queue = NULL;
	    /* TODO: wakeup mgag_accel->idle -- ortalo
	     * TODO: Need to delay also?
	     */
	    /* on linux: wake_up(accel->idle); */
	    break;
	  }

	if (cur->execution.state != KGI_AS_WAIT)
	  KRN_DEBUG(1, "Exec logic problem!");

	buffer = cur;
	KRN_DEBUG(2,"going to next: buffer:%p adress:%p value: %i",
		  buffer, &(buffer->execution.state), buffer->execution.state);
      }
      /*
      ** FALL THROUGH! (to execute next buffer)
      */
    case KGI_AS_WAIT:
      /* We do not do GP context switch on the Matrox.
      ** We swap the data structs and initiate the buffer transfer.
      */
      {
	/* TODO: check the context switch logic! */
	kgi_u_t do_ctx_switch = 0;
	if (accel->context != buffer->context)
	  {
	    KRN_DEBUG(1,"DMA with context update needed!");
	    do_ctx_switch = 1;
	    accel->context = buffer->context;
	  }
	
	buffer->execution.state = KGI_AS_EXEC;

	/* Wait for the engine to be idle in case a command is still
	** in progress (impossible theoretically but...)
	** WARNING: This wait causes an erroneous BLOCK on the G400.
	** DO NOT ACTIVATE if you do not know how to solve this issue.
	** (ortalo, 2003/01/17)
	** TODO: Study this STATUS strangeness?
	*/
	/*
	  mgag_chipset_wait_engine_idle(mgag_io);
	*/

	{
	  mgag_chipset_accel_context_t *mgag_ctx = accel->context;
	  /* Recovers the buffer tag */
	  kgi_u32_t tag = (*(kgi_u32_t *)buffer->aperture.virt);
	  
	  /* Dispatch buffer according to tag */
	  switch (tag)
	    {
	    case MGAG_ACCEL_TAG_DRAWING_ENGINE:
	      if (do_ctx_switch == 0)
		{
		  mgag_chipset_engine_do_buffer(mgag, mgag_io, buffer, mgag_ctx);
		}
	      else
		{
		  if (mgag->flags & MGAG_CF_G200) {
		    mgag_chipset_g200_do_buffer_ctx(mgag_io, buffer, mgag_ctx);
		  } else if ((mgag->flags & MGAG_CF_G400)
			     || (mgag->flags & MGAG_CF_G550)) {
		    mgag_chipset_g400_do_buffer_ctx(mgag_io, buffer, mgag_ctx);
		  } else {
		    KRN_DEBUG(1, "Context switches only active on the Gx00 for now");
		  }
		}
	      break;
	    case MGAG_ACCEL_TAG_WARP_TRIANGLE_LIST:
	      if ((mgag->flags & MGAG_CF_G400)
		  || (mgag->flags & MGAG_CF_G200)
		  || (mgag->flags & MGAG_CF_G550)) {
		mgag_chipset_warp_do_buffer(mgag_io, buffer, mgag_ctx);
	      } else {
		KRN_DEBUG(1, "WARPs only active on the G200/G400/G550 for now");
	      }
	      break;
	    default:
	      KRN_ERROR("Unknown accel buffer tag: %.8x", tag);
	      break;
	    }
	}
      }
      break;

    default:
      KRN_ERROR("Matrox: invalid state %i for queued buffer",
		buffer->execution.state);
      KRN_INTERNAL_ERROR;
      break;
    }
}

/*
** Place a buffer in the execution queue for DMA (or executes
** it on the Mystique).
*/
static void mgag_chipset_accel_exec(kgi_accel_t *accel,
				    kgi_accel_buffer_t *buffer)
{
  int err;
  mgag_chipset_t *mgag = accel->meta;
  mgag_chipset_io_t *mgag_io = accel->meta_io;
  /* Recovers the buffer tag */
  kgi_u32_t tag = (*(kgi_u32_t *)buffer->aperture.virt);

  KRN_ASSERT(KGI_AS_FILL == buffer->execution.state);

  err = mga_chipset_accel_validate(accel, buffer);
  if (err != 0)
    {
      /* Error or empty detected: returns */
      /* TODO: Do this inside a compilation directive */
      if (err != 2)
	KRN_DEBUG(1,"Validation function detected error (%i): no exec!",
		  err);
      buffer->execution.state = KGI_AS_IDLE;
      return;
    }

  if (mgag->flags & MGAG_CF_1x64)
    {
      /* Dispatch buffer according to tag */
      switch (tag)
	{
	case MGAG_ACCEL_TAG_DRAWING_ENGINE:
	  /*
	  ** We directly write the given buffer to the pseudo-dma
	  ** window of the chipset.
	  */
	  /* buffer->exec_state = KGI_AS_QUEUED; */
	  /* buffer->exec_state = KGI_AS_EXEC; */
	  /* Resets pseudo-DMA, selects General Purpose Write */
	  MGAG_GC_OUT32(mgag_io,
			(MGAG_GC_IN32(mgag_io, OPMODE) & ~OPMODE_DMAMOD_MASK)
			| OPMODE_DMAMOD_GENERAL_WRITE,
			OPMODE);
	  /* Transfers the buffer: ILOAD or DMAWIN (control) areas can
	  ** be used... (8Mo or 7Ko apertures).
	  ** TODO: Should I use a different instruction to take advantage
	  ** TODO: of PCI bursts ?
	  */
	  mem_outs32(mgag_io->iload.base_virt,
		     (void *)(buffer->aperture.virt + MGAG_ACCEL_TAG_LENGTH),
		     ((buffer->execution.size - MGAG_ACCEL_TAG_LENGTH) >> 2));
	  /* Again */
	  MGAG_GC_OUT32(mgag_io,
			(MGAG_GC_IN32(mgag_io, OPMODE) & ~OPMODE_DMAMOD_MASK)
			| OPMODE_DMAMOD_GENERAL_WRITE,
			OPMODE);
	  break;
	case MGAG_ACCEL_TAG_WARP_TRIANGLE_LIST:
	  KRN_ERROR("There is no WARP engine on the 1x64!");
	  break;
	default:
	  KRN_ERROR("Unknown accel buffer tag: %.8x", tag);
	  break;
	}
      /* Mark buffer as idle */
      buffer->execution.state = KGI_AS_IDLE;
    }
  else if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200)
	   || (mgag->flags & MGAG_CF_G550))
    {
      kgi_u_t do_schedule = 0;
      mgag_chipset_irq_state_t irq_state;
      /*
      ** The buffer is fetched by the chipset itself via DMA.
      ** We schedule the work.
      */
      mgag_chipset_irq_block(mgag_io, &irq_state);

      /* TODO: should not this be KGI_AS_QUEUED? It seems to me that the
       * TODO: semantics of WAIT and QUEUED has not really been used
       * TODO: within the KGI logic (because both buffers are treated
       * TODO: in the same way in it)... -- ortalo
       */
      buffer->execution.state = KGI_AS_WAIT;
      buffer->execution.next = NULL;

      if (accel->execution.queue)
	{
	  /* We add the buffer to the execution queue (IRQs are blocked)
	  **
	  ** NOTE: We probably *cannot* manage priorities easily...
	  ** NOTE: Accel context is built after parsing command buffers
	  ** NOTE: in this function, so reordering buffers will invalidate
	  ** NOTE: the context.
	  */
	  kgi_accel_buffer_t *cur = accel->execution.queue;
	  /* Goes to the end of the list */
	  while (cur->execution.next != NULL)
	    cur = cur->execution.next;
	  /* Links the buffer at the end */
	  cur->execution.next = buffer;
	}
      else
	{
	  /* Points the beginning of the exec queue on this buffer */
	  accel->execution.queue = buffer;
	  /* Starts execution if no other buffer running */
	  do_schedule = 1;
	}

      mgag_chipset_irq_restore(mgag_io, &irq_state);

      if (do_schedule)
	  mgag_chipset_accel_schedule(accel);

      /* TODO: Fixme! (We wait for the engine to be idle before returning
      ** TODO: while we should let the DMA buffers go...
      */
      /*
	mgag_chipset_wait_engine_idle(mgag_io);
      */
    }
  else
    {
      KRN_ERROR("Unknown chipset for accelerator! (Buffer not executed.)");
      buffer->execution.state = KGI_AS_IDLE;
    }

  KRN_DEBUG(2,"completed");
}

/* ----------------------------------------------------------------------------
**	General chipset management procedures
** ----------------------------------------------------------------------------
*/

/*
** Find the amount of installed memory
*/
static inline kgi_u_t mgag_chipset_memory_count(mgag_chipset_t *mgag,
	mgag_chipset_io_t *mgag_io)
{
	kgi_u_t  ret   = 0;
	kgi_u8_t ecrt3 = MGAG_ECRT_IN8(mgag_io, ECRT3);

	MGAG_ECRT_OUT8(mgag_io, ecrt3 | ECRT3_MGAMODE, ECRT3);

	if ((mgag->flags & MGAG_CF_G400)
	    || (mgag->flags & MGAG_CF_G550)) {

		mem_out8(0x99, mgag_io->fb.base_virt + 31 MB);
		ret = mem_in8( mgag_io->fb.base_virt + 31 MB) == 0x99 ? 32 : 16;

	} else if (mgag->flags & MGAG_CF_G200) {

		mem_out8(0x99, mgag_io->fb.base_virt + 15 MB);
		ret = mem_in8( mgag_io->fb.base_virt + 15 MB) == 0x99 ? 16 : 8;

	} else if (mgag->flags & MGAG_CF_1x64) {

		mem_out8(0x99, mgag_io->fb.base_virt + 5 MB);
		mem_out8(0x88, mgag_io->fb.base_virt + 3 MB);
		ret = mem_in8( mgag_io->fb.base_virt + 5 MB) == 0x99 ? 6 :
		      mem_in8( mgag_io->fb.base_virt + 3 MB) == 0x88 ? 4 : 2;
	} else {

		KRN_INTERNAL_ERROR;
	}

	MGAG_ECRT_OUT8(mgag_io, ecrt3, ECRT3);

	return ret MB;
}

/*
** Power-up the chipset (usually needed when the board is the secondary
** board and the BIOS did not touch it at all).
*/
static void mgag_chipset_power_up(mgag_chipset_t *mgag,
				  mgag_chipset_io_t *mgag_io)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);

  KRN_DEBUG(2, "entered");

  /* NOTE: we do not touch the flash rom, endianess, refresh counter
   * NOTE: and vgaboot strap.
   */

  /*
  ** First, we power up the system PLL, the pixel PLL, the
  ** LUT and the DAC.
  ** This section should be valid for Matrox Mystique, G200 and G400.
  */
  /* NOTE: We assume an off-chip voltage reference is used, we do
   * not touch XVREFCTRL.
   */
  /* Power-up the system PLL */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_SYSPLLPDN,
	       pcidev + MGAG_PCI_OPTION1);
  {
    int syspll_lock_cnt = PLL_DELAY; /* Wait for the system PLL to lock */
    while (--syspll_lock_cnt &&
	   !(MGAG_EDAC_IN8(mgag_io, XSYSPLLSTAT) & XSYSPLLSTAT_SYSLOCK));
    KRN_ASSERT(syspll_lock_cnt);
  }
  /* Power up the pixel PLL */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXPLLPDN,
		 XPIXCLKCTRL);
  {
    int pixpll_lock_cnt = PLL_DELAY; /* Wait for the pixel PLL to lock */
    while (--pixpll_lock_cnt &&
	   !(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    KRN_ASSERT(pixpll_lock_cnt);
  }
  /* Power up the LUT */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XMISCCTRL) | XMISCCTRL_RAMCS,
		 XMISCCTRL);
  /* Power up the DAC */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XMISCCTRL) | XMISCCTRL_DACPDN,
		 XMISCCTRL);

  /*
  ** Then, we select the PLLs. According to the documentation, the
  ** following steps will set:
  ** - G400: MCLK = 150MHz, GCLK = 90MHz, WCLK = 100, PIXCLK = 25.175MHz
  ** - G200: MCLK = 143MHz, GCLK = 71.5MHz, PIXCLK = 25.175MHz
  ** - Mystique: MCLK = 66MHz, GCLK = 44MHz, PIXCLK = 25.175MHz
  ** NB: The 6 following steps must be done one after the other. They
  ** cannot be combined (according to the documentation).
  ** NB: PLLs parameters (M,N,P,etc.) are not touched. They stay to
  **  their reset value. IMO, clocks reprogramming belong to the clock
  **  subsystem. Not here. -- ortalo
  */
  /* Disable the system clocks */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_SYSCLKDIS,
	       pcidev + MGAG_PCI_OPTION1);
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* Select the system PLL */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_SYSCLKSEL_MASK)
		   | M1x64_O1_SYSCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & MGAG_CF_G200)
    {
      /* Select the system PLL */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_SYSCLKSEL_MASK)
		   | G200_O1_SYSCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION1);      
    }
  else if (mgag->flags & MGAG_CF_G400)
    {
      /* Select the system PLL: MCLK, GCLK, WCLK */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION3) & ~G400_O3_MCLKSEL_MASK)
		   | G400_O3_MCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION3);      
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION3) & ~G400_O3_GCLKSEL_MASK)
		   | G400_O3_GCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION3);      
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION3) & ~G400_O3_WCLKSEL_MASK)
		   | G400_O3_WCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION3);
    }
  /* Enable the system clocks */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_SYSCLKDIS,
	       pcidev + MGAG_PCI_OPTION1);
  /* Disable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);
  /* Select the pixel PLL */
  MGAG_EDAC_OUT8(mgag_io,
		 (MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKSEL_MASK)
		 | XPIXCLKCTRL_PIXCLKSEL_PIXPLL,
		 XPIXCLKCTRL);
  /* Enable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);

  /* Codec clock frequency selection (G400+) */
  if (mgag->flags & MGAG_CF_G400)
    {
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION2) & ~G400_O2_CODCLKSEL_MASK)
		   | G400_O2_CODCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION2);
    }
  /*
  ** Finally, we initialize the memory.
  */
  /* Disable the video */
  MGAG_SEQ_OUT8(mgag_io,
		MGAG_SEQ_IN8(mgag_io,VGA_SEQ_CLOCK) | VGA_SR01_DISPLAY_OFF,
		VGA_SEQ_CLOCK);
  /* TODO: we should also disable CRTC2 for G400 and higher, but it did
   * TODO: not hurt me yet. -- ortalo
   */
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, M1x64_MCTLWTST_CASLTCNY | M1x64_MCTLWTST_RCDELAY
		    | M1x64_MCTLWTST_RASMIN_MASK, MCTLWTST);
      /* We do not touch the memconfig field of OPTION (SDRAM/SGRAM bound) */
      if (mgag->flags & MGAG_CF_SGRAM) {
	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1)
		     | MGAG_O1_HARDPWMSK, pcidev + MGAG_PCI_OPTION1);
      }
    }
  else if (mgag->flags & (MGAG_CF_G400 | MGAG_CF_G200))
    {
      if (mgag->flags & MGAG_CF_G200)
	{
	  /* (Re)Writes the reset value into mem. control wait state (OK?) */
	  /* TODO: Find a good reset value for MCTLWTST */
	  MGAG_GC_OUT32(mgag_io, G200_MCTLWTST_RESET_VALUE, MCTLWTST);
	  /* We should write the memconfig field (PCI_OPTION1 reg)
	   * according to the type of memory used. But this requires
	   * parsing the BIOS and such a power_up style has been left
	   * to an alternate implementation in this file. -- ortalo
	   */
	  /* TODO: We could use user-provide value however (module parms) */
	  /* We setup things for 8Mo SDRAM memory with 1 bank statically */
	  pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_MEMCONFIG_MASK)
		       | (0x5 << G200_O1_MEMCONFIG_SHIFT),
		       pcidev + MGAG_PCI_OPTION1);
	  /* Not indicated in the doc - but we do it anyway */
	  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~ MGAG_O1_HARDPWMSK,
		       pcidev + MGAG_PCI_OPTION1);
	  /* TODO: We should set the mbuftype of the OPTION2 reg ? */
	}
      else if (mgag->flags & MGAG_CF_G400)
	{
	  /* (Re)Writes the reset value into mem. control wait state (OK?) */
	  MGAG_GC_OUT32(mgag_io, G400_MCTLWTST_RESET_VALUE, MCTLWTST);
	  /* We should write the memconfig field (PCI_OPTION1 reg)
	   * according to the type of memory used. But this requires
	   * parsing the BIOS and such a power_up style has been left
	   * to an alternate implementation in this file. -- ortalo
	   */
	  /* TODO: We could use user-provide value however (module parms) */
	  /* We setup things for 16Mo SDRAM memory... */
	  pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_MEMCONFIG_MASK)
		       | (0x0 << G400_O1_MEMCONFIG_SHIFT),
		       pcidev + MGAG_PCI_OPTION1);
	  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_HARDPWMSK,
		       pcidev + MGAG_PCI_OPTION1);
	}
      /* Set the OPCODe of MEMRDBK for normal operation (0x0) */
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, MEMRDBK) & ~MRSOPCOD_MASK)
		    | (0x0 << MRSOPCOD_SHIFT),
		    MEMRDBK);
      /* Set the read delays 0 and 1 to average values (like reset) */
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, MEMRDBK) & ~MCLKBRD0_MASK)
		    | (0x8 << MCLKBRD0_SHIFT),
		    MEMRDBK);
      MGAG_GC_OUT32(mgag_io,
		    (MGAG_GC_IN32(mgag_io, MEMRDBK) & ~MCLKBRD1_MASK)
		    | (0x8 << MCLKBRD1_SHIFT),
		    MEMRDBK);
    }

  /* Wait delay */
  kgi_udelay(200);
  
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET & ~MACCESS_JEDECRST, MACCESS);
      /* Wait delay */
      /* TODO: do a wait delay of minimum (100 * MCLK period)... */
      kgi_udelay(5);
    }

  /* Start the memory reset */
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET | MACCESS_JEDECRST, MACCESS);
    }
  else if (mgag->flags & (MGAG_CF_G400 | MGAG_CF_G200))
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET, MACCESS);
    }
  /* Program the memory refresh cycle counter */
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* We use 0xF here as the refresh counter (like my BIOS) */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_RFHCNT_MASK)
		   | (0xF << M1x64_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & MGAG_CF_G200)
    {
      /* TODO: Check the value to use here: 1 or 8 or ? */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_RFHCNT_MASK)
		   | (0x8 << G200_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & MGAG_CF_G400)
    {
      /* TODO: Check the value to use here: 1 or 8 or ? */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_RFHCNT_MASK)
		   | (0x8 << G400_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  /* On the 1x64, we also initialize the VGA frame buffer mask */
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* We map the VGA fb location to 0x000000-0x7FFFFF */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_FBMASKN_MASK)
		   | (0x7 << M1x64_O1_FBMASKN_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }

  /* Finally, does a softreset of the accel engine */
  mgag_chipset_softreset(mgag_io);

#if 1
  /* TODO: No need to re-enable the video in theory, remove? */
  /* TODO: Remove definitely after checking. STILL NEEDED! */
  MGAG_SEQ_OUT8(mgag_io,
	       MGAG_SEQ_IN8(mgag_io,VGA_SEQ_CLOCK) & ~VGA_SR01_DISPLAY_OFF,
	       VGA_SEQ_CLOCK);
#endif

  KRN_DEBUG(2, "completed.");
}

/*
** Power-up the chipset according to the parameters indicated in the
** Matrox video BIOS itself.
*/
static void mgag_chipset_power_up_per_vbios(mgag_chipset_t *mgag,
					    mgag_chipset_io_t *mgag_io)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);

  KRN_DEBUG(2, "entered");

  /*
  ** First, we power up the system PLL, the pixel PLL, the
  ** LUT and the DAC.
  */

  /* Power-up the system PLL */
  {
    kgi_u32_t opt1;
    /* Selects start value for OPTION register: VBIOS or probed */
    if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
      opt1 = mgag->vbios.opt;
    else
      opt1 = pcicfg_in32(pcidev + MGAG_PCI_OPTION1);
    opt1 &= 0xC0000100; /* Taken from matroxfb... (preserve straps) */
    opt1 &= ~0x03400040; /* Taken from matroxfb... (apparently preserves */
    opt1 |= mgag->vbios.opt & 0x03400040; /* non-specified bits from VBIOS */
    opt1 |= MGAG_O1_SYSPLLPDN; /* Power up the system PLL */
    pcicfg_out32(opt1, pcidev + MGAG_PCI_OPTION1);
  }
  {
    int syspll_lock_cnt = PLL_DELAY; /* Wait for the system PLL to lock */
    while (--syspll_lock_cnt &&
	   !(MGAG_EDAC_IN8(mgag_io, XSYSPLLSTAT) & XSYSPLLSTAT_SYSLOCK));
    KRN_ASSERT(syspll_lock_cnt);
  }
  /* Power up the pixel PLL */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXPLLPDN,
		 XPIXCLKCTRL);
  {
    int pixpll_lock_cnt = PLL_DELAY; /* Wait for the pixel PLL to lock */
    while (--pixpll_lock_cnt &&
	   !(MGAG_EDAC_IN8(mgag_io, XPIXPLLSTAT) & XPIXPLLSTAT_PIXLOCK));
    KRN_ASSERT(pixpll_lock_cnt);
  }
  /* Power up the LUT */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XMISCCTRL) | XMISCCTRL_RAMCS,
		 XMISCCTRL);
  /* Power up the DAC */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XMISCCTRL) | XMISCCTRL_DACPDN,
		 XMISCCTRL);

  /*
  ** Then, we select the PLLs using the predefined values parsed in the
  ** vbios.
  */
  /* Disable the system clocks */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_SYSCLKDIS,
	       pcidev + MGAG_PCI_OPTION1);
  if (mgag->flags & MGAG_CF_1x64)
    {
    }
  if (mgag->flags & MGAG_CF_G200)
    {
      /* Select the system PLL */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_SYSCLKSEL_MASK)
		   | G200_O1_SYSCLKSEL_SYS,
		   pcidev + MGAG_PCI_OPTION1);      
    }
  /* NB: A G450 is also a G400... */
  if ((mgag->flags & MGAG_CF_G400) && (!(mgag->flags & MGAG_CF_G450)))
    {
      /* Force the selection of the system PLL: MCLK, GCLK, WCLK
       * (it should be done but just to be sure...)
       */
      pcicfg_out32((mgag->vbios.opt3 & ~(G400_O3_MCLKSEL_MASK | G400_O3_GCLKSEL_MASK | G400_O3_WCLKSEL_MASK))
		    | G400_O3_MCLKSEL_SYS | G400_O3_GCLKSEL_SYS | G400_O3_WCLKSEL_SYS,
		    pcidev + MGAG_PCI_OPTION3);
    }
  if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
    {
      /* Program the various clocks parameters using the information
       * from the vbios.
       */
#ifdef DO_GX50_POWER_UP
      /* Re enable clocks (on PCI clock) for pll programming */
      pcicfg_out32(mgag->vbios.opt3 & ~0x00300C03, pcidev + MGAG_PCI_OPTION3);
      pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_SYSCLKDIS,
		   pcidev + MGAG_PCI_OPTION1);
      /* Program the video clock PLL to 0 */
      MGAG_EDAC_OUT8(mgag_io, 0x02, XVIDPLLM); /* Like my VBIOS -- ortalo */
      MGAG_EDAC_OUT8(mgag_io, 0x23, XVIDPLLN);
      MGAG_EDAC_OUT8(mgag_io, 0x00, XVIDPLLP);
      /* TODO: do program the _video_ clock PLL of Gx50 to 0 if needed. */
      /* Program the system clock */
      {
	Gx50_pll_t oneclock;
	Gx50_pll_mode_t onemode;
	
	oneclock.fref = mgag->vbios.fref KHZ;
	oneclock.fvco.min = 256 MHZ;
	oneclock.fvco.max = 1200 MHZ;
	oneclock.p.min = 0;
	oneclock.p.max = 4;
	oneclock.div.min = 1;
	oneclock.div.max = 10;
	oneclock.mul.min = 9;
	oneclock.mul.max = 125;
	oneclock.a.mul = 2;
	oneclock.a.div = 1;
	/* NB: We know the system VCO (not raw frequency) */
	Gx50_pll_set(&oneclock, mgag->vbios.fsystem KHZ / 2,
		     Gx50_SYSTEM_PLL, mgag_io,
		     &onemode);
      }
      /* Disable them for final config */
      pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_SYSCLKDIS,
		   pcidev + MGAG_PCI_OPTION1);
      pcicfg_out32(mgag->vbios.opt3, pcidev + MGAG_PCI_OPTION3);
#endif
    }
  /* Enable the system clocks */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_SYSCLKDIS,
	       pcidev + MGAG_PCI_OPTION1);
  /* Disable the pixel clock and video clock */
  /* TODO: check why disabling then re-enabling the pixel and video clocks */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) | XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);
  if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
    {
      /* Selects the 2nd PLL: we only handle the digital output, connected
	 to the second output. (I think... Hmm, Petr?) -- ortalo */
      MGAG_EDAC_OUT8(mgag_io,
		     (MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKSEL_MASK)
		     | 0x3, /* nor XPIXCLKCTRL_PIXCLKSEL_PIXPLL ? */
		     XPIXCLKCTRL);
      
    }
  else
    {
      /* Select the pixel PLL */
      MGAG_EDAC_OUT8(mgag_io,
		     (MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKSEL_MASK)
		     | XPIXCLKCTRL_PIXCLKSEL_PIXPLL,
		     XPIXCLKCTRL);
    }
  /* Enable the pixel clock and video clock */
  MGAG_EDAC_OUT8(mgag_io,
		 MGAG_EDAC_IN8(mgag_io, XPIXCLKCTRL) & ~XPIXCLKCTRL_PIXCLKDIS,
		 XPIXCLKCTRL);

  /* TODO: do codec clock frequency selection for G400? (see power_up) */

  /*
  ** Finally, we initialize the memory.
  */
  /* Disable the video */
  MGAG_SEQ_OUT8(mgag_io,
		MGAG_SEQ_IN8(mgag_io,VGA_SEQ_CLOCK) | VGA_SR01_DISPLAY_OFF,
		VGA_SEQ_CLOCK);
  /* TODO: We should also disable CRTC2 for G400 and higher */
  if (mgag->flags & MGAG_CF_1x64)
    {
    }
  else if (mgag->flags & MGAG_CF_G200)
    {
      /* Uses video BIOS values */
      MGAG_GC_OUT32(mgag_io, mgag->vbios.mctlwtst, MCTLWTST);
      /* We setup things using the memconfig from video BIOS... */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_MEMCONFIG_MASK)
		   | (mgag->vbios.opt & G200_O1_MEMCONFIG_MASK),
		   pcidev + MGAG_PCI_OPTION1);
      /* TODO: Use the global MGAG_CF_SGRAM flag ? */
      if (mgag->vbios.sgram)
	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_HARDPWMSK,
		     pcidev + MGAG_PCI_OPTION1);
      else
	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_HARDPWMSK,
		     pcidev + MGAG_PCI_OPTION1);
      /* TODO: Set the mbuftype of the OPTION2 reg as matrox fb */
#if 0
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION2) & ~(G200_O2_MBUFTYPE0 | G200_O2_MBUFTYPE1)),
		   pcidev + MGAG_PCI_OPTION2);
#else
      /* TODO: should we use the vbios value for opt2? */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION2) & ~(0x3F << 12))
		   | (mgag->vbios.opt2 & (0x3F << 12)),
		   pcidev + MGAG_PCI_OPTION2);
#endif
      /* Set memrdbk with the video bios value */
      MGAG_GC_OUT32(mgag_io, mgag->vbios.memrdbk, MEMRDBK);
    }
  /* NB: A G450 is also a G400... */
  else if ((mgag->flags & MGAG_CF_G400) && (!(mgag->flags & MGAG_CF_G450)))
    {
      /* Uses video BIOS values */
      MGAG_GC_OUT32(mgag_io, mgag->vbios.mctlwtst, MCTLWTST);
      /* We setup things using the memconfig from video BIOS... */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_MEMCONFIG_MASK)
		   | (mgag->vbios.opt & G400_O1_MEMCONFIG_MASK),
		   pcidev + MGAG_PCI_OPTION1);
#if 1
      /* TODO: Use the global MGAG_CF_SGRAM flag ? */
      if (mgag->vbios.sgram)
	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_HARDPWMSK,
		     pcidev + MGAG_PCI_OPTION1);
      else
	pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_HARDPWMSK,
		     pcidev + MGAG_PCI_OPTION1);
#else
      /* _If_ that memconfig is 000, then 16Mo SDRAM is triggered by
	 default as hardpwmsk is cleared. Enforces this behavior: */
      pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~MGAG_O1_HARDPWMSK,
		   pcidev + MGAG_PCI_OPTION1);
#endif
      /* Set memrdbk with the video bios value */
      MGAG_GC_OUT32(mgag_io, mgag->vbios.memrdbk, MEMRDBK);
    }
  if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
    { /* Taken from matroxfb */
      /* Disable memory refresh */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_RFHCNT_MASK),
		   pcidev + MGAG_PCI_OPTION1);
      /* Set memory interface parameters using vbios value */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~0x00207E00)
		   | (mgag->vbios.opt & 0x00207E00),
		   pcidev + MGAG_PCI_OPTION1);
      /* Now writes setup parameters from vbios */
      pcicfg_out32(mgag->vbios.opt2, pcidev + MGAG_PCI_OPTION2);
      MGAG_GC_OUT32(mgag_io, mgag->vbios.mctlwtst, MCTLWTST);
      /* Set up memory interface with disabled interface memory clock */
      pcicfg_out32(mgag->vbios.memmisc & ~0x80000000U,
		   pcidev + MGAG_PCI_MEMMISC);
      MGAG_GC_OUT32(mgag_io, mgag->vbios.memrdbk, MEMRDBK);
      MGAG_GC_OUT32(mgag_io, mgag->vbios.maccess, MACCESS);
      /* Reenable memory clock */
      pcicfg_out32(mgag->vbios.memmisc | 0x80000000U,
		   pcidev + MGAG_PCI_MEMMISC);
    }

  /* Wait delay */
  /* TODO: do a wait delay of minimum 200 micro-seconds... */
#if 0
  udelay(200);
#else
  { int cnt = 100000; while (cnt--) { }; }
#endif
  
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET & ~MACCESS_JEDECRST, MACCESS);
      /* Wait delay */
      /* TODO: do a wait delay of minimum (100 * MCLK period)... */
#if 0
      udelay(5);
#else
      { int cnt = 10000; while (cnt--) { }; }
#endif      
    }
  else if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
    {
      if (mgag->vbios.ddr && (!mgag->vbios.dll || !mgag->vbios.emrswen))
	{
	  KRN_DEBUG(1, "Updating MEMRDBK vbios value");
	  MGAG_GC_OUT32(mgag_io, mgag->vbios.memrdbk & ~0x1000, MEMRDBK);
	}
    }

  /* Start the memory reset */
  if (mgag->flags & MGAG_CF_1x64)
    {
      MGAG_GC_OUT32(mgag_io, MACCESS_MEMRESET | MACCESS_JEDECRST, MACCESS);
    }
  else if ((mgag->flags & MGAG_CF_G400) || (mgag->flags & MGAG_CF_G200)
	   || (mgag->flags & MGAG_CF_G550))
    { /* NB: A G450 is also a G400 */
      MGAG_GC_OUT32(mgag_io, MGAG_GC_IN32(mgag_io, MACCESS) | MACCESS_MEMRESET,
		    MACCESS);
    }
  
  /* Wait delay */
  /* TODO: do a wait delay of minimum 200 micro-seconds... */
#if 0
  udelay(200);
#else
  { int cnt = 100000; while (cnt--) { }; }
#endif
  
  /* Program the memory refresh cycle counter */
  if (mgag->flags & MGAG_CF_1x64)
    {
    }
  else if (mgag->flags & MGAG_CF_G200)
    {
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G200_O1_RFHCNT_MASK)
		   | (0x8 << G200_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if ((mgag->flags & MGAG_CF_G400) && (!(mgag->flags & MGAG_CF_G450)))
    {
      /* NB: A G450 is also a G400... */
      /* TODO: Check the value to use here: 1 or 8 or ? */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_RFHCNT_MASK)
		   | (0x8 << G400_O1_RFHCNT_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
    {
      /* Enable memory refresh */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~G400_O1_RFHCNT_MASK)
		   | (mgag->vbios.opt | G400_O1_RFHCNT_MASK),
		   pcidev + MGAG_PCI_OPTION1);
    }
  /* Additional initializations */
  if (mgag->flags & MGAG_CF_1x64)
    {
      /* On the 1x64, we also initialize the VGA frame buffer mask */
      /* We map the VGA fb location to 0x000000-0x7FFFFF */
      pcicfg_out32((pcicfg_in32(pcidev + MGAG_PCI_OPTION1) & ~M1x64_O1_FBMASKN_MASK)
		   | (0x7 << M1x64_O1_FBMASKN_SHIFT),
		   pcidev + MGAG_PCI_OPTION1);
    }
  else if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450))
    {
      /* XSYNCCTRL(0x8B) to 0xCC as per matroxfb and Petr... */
      MGAG_EDAC_OUT8(mgag_io, 0xCC, XSYNCCTRL);
      /* XPWRCTRL(0xA0) to 0x1F as per matroxfb and Petr... */
      MGAG_EDAC_OUT8(mgag_io, 0x1F, XPWRCTRL);
      /* XOUTPUTCONN(0x8A) to ? (DAC1->output1) as per matroxfb and Petr... */
      MGAG_EDAC_OUT8(mgag_io, 5, XOUTPUTCONN);
  
      /* Some translation (taken from matroxfb) */
      if (!(mgag->vbios.no_wtst_xlat))
	{
	  kgi_u32_t wtst_xlat[] = { 0, 1, 5, 6, 7, 5, 2, 3 }; /* 8 values */
	  kgi_u32_t new;
	  new = (mgag->vbios.mctlwtst & ~Gx00_MCTLWTST_CASLTCNY_MASK)
	    | wtst_xlat[mgag->vbios.mctlwtst & Gx00_MCTLWTST_CASLTCNY_MASK];
	  KRN_DEBUG(1, "Doing wtst xlat from %.8x (old) to %.8x (new)",
		    mgag->vbios.mctlwtst, new);
	  MGAG_GC_OUT32(mgag_io, new, MCTLWTST);
	}
    }
  /* Resets the plane mask */
  MGAG_GC_OUT32(mgag_io, 0x00000000, PLNWT);
  MGAG_GC_OUT32(mgag_io, 0xFFFFFFFF, PLNWT);

  /* Finally, does a softreset of the accel engine */
  mgag_chipset_softreset(mgag_io);

#if 1
  /* TODO: No need to re-enable the video in theory, remove? */
  /* TODO: Remove definitely after checking. STILL NEEDED! */
  MGAG_SEQ_OUT8(mgag_io,
	       MGAG_SEQ_IN8(mgag_io,VGA_SEQ_CLOCK) & ~VGA_SR01_DISPLAY_OFF,
	       VGA_SEQ_CLOCK);
#endif

  KRN_DEBUG(2, "completed.");
}

/* ----------------------------------------------------------------------------
**	Matrox video BIOS parsing
** ----------------------------------------------------------------------------
*/
static void pins_info_345(mgag_chipset_io_t *mgag_io, kgi_u_t pins_offset)
{
#define PINS_IN8(idx) MGAG_ROM_IN8(mgag_io, pins_offset + (idx))
#define PINS_IN16(idx) MGAG_ROM_IN16(mgag_io, pins_offset + (idx))

#define SERIAL_NO_LENGTH 16
#define PART_INFO_LENGTH 6
  kgi_u8_t serial_no[SERIAL_NO_LENGTH+1];
  kgi_u8_t part_info[PART_INFO_LENGTH+1];

  KRN_DEBUG(1, "Program date: %u-%u-%u, program count: %x",
	    1900 + (PINS_IN16(6) >> 9),
	    (PINS_IN16(6) >> 5) & 0xF, PINS_IN16(6) & 0x1F,
	    PINS_IN16(8));
  KRN_DEBUG(1, "Product ID: %.4x", PINS_IN16(10));
  {
    kgi_u_t i;
    for (i = 0; i < SERIAL_NO_LENGTH; i++)
	serial_no[i] = PINS_IN8(12+i);
    serial_no[SERIAL_NO_LENGTH] = '\0';
    for (i = 0; i < PART_INFO_LENGTH; i++)
	part_info[i] = PINS_IN8(28+i);
    part_info[PART_INFO_LENGTH] = '\0';
  }
  KRN_DEBUG(1, "Serial No.: \"%s\"", serial_no);
  KRN_DEBUG(1, "Part info.: \"%s\"", part_info);
  KRN_DEBUG(1, "PCB info.: %.4x, DVD info.: %.2x",
	    PINS_IN16(34), PINS_IN8(3));

#undef PINS_IN8
#undef PINS_IN16
}

static void parse_pins2(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io)
{
  kgi_u_t _pins_offset = mgag->vbios.pins.offset;
#define PINS_IN8(idx) MGAG_ROM_IN8(mgag_io, _pins_offset + (idx))
#define PINS_IN16(idx) MGAG_ROM_IN16(mgag_io, _pins_offset + (idx))
#define PINS_IN32(idx) MGAG_ROM_IN32(mgag_io, _pins_offset + (idx))

  KRN_DEBUG(1, "Max pixel/system VCO: %u kHz", (PINS_IN8(41) == 0xFF) ?
	    230000 : ((PINS_IN8(41) + 100) * 1000));

  mgag->vbios.fref = 14318;
  mgag->vbios.fsystem = 
    (PINS_IN8(43) == 0xFF) ? 50000 : ((PINS_IN8(43) + 100) * 1000);

  mgag->vbios.mctlwtst = 
    ((PINS_IN8(51) & 0x01) ? 0x00000001 : 0) |
    ((PINS_IN8(51) & 0x02) ? 0x00000100 : 0) |
    ((PINS_IN8(51) & 0x04) ? 0x00010000 : 0) |
    ((PINS_IN8(51) & 0x08) ? 0x00020000 : 0);

#define DISPLAY_SAVED_FREQ(fre) KRN_DEBUG(1, #fre " = %u kHz", mgag->vbios.fre)
#define DISPLAY_SAVED_REG(reg) KRN_DEBUG(1, #reg " = %.8x", mgag->vbios.reg)

  DISPLAY_SAVED_FREQ(fsystem);
  DISPLAY_SAVED_REG(mctlwtst);

#undef DISPLAY_SAVED_FREQ
#undef DISPLAY_SAVED_REG

#undef PINS_IN8
#undef PINS_IN16
#undef PINS_IN32
}

static void parse_pins3(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io)
{
  kgi_u_t _pins_offset = mgag->vbios.pins.offset;
#define PINS_IN8(idx) MGAG_ROM_IN8(mgag_io, _pins_offset + (idx))
#define PINS_IN16(idx) MGAG_ROM_IN16(mgag_io, _pins_offset + (idx))
#define PINS_IN32(idx) MGAG_ROM_IN32(mgag_io, _pins_offset + (idx))

  KRN_DEBUG(1, "Max pixel/system VCO: %u kHz", (PINS_IN8(36) == 0xFF) ?
	    230000 : ((PINS_IN8(36) + 100) * 1000));

  KRN_DEBUG(1, "Max Pixel Clock ( 8bpp): %u MHz", PINS_IN8(37) + 100);
  KRN_DEBUG(1, "Max Pixel Clock (16bpp): %u MHz", PINS_IN8(38) + 100);
  KRN_DEBUG(1, "Max Pixel Clock (24bpp): %u MHz", PINS_IN8(39) + 100);
  KRN_DEBUG(1, "Max Pixel Clock (32bpp): %u MHz", PINS_IN8(40) + 100);

  mgag->vbios.fref = (PINS_IN8(52) & 0x20) ? 14318 : 27000;
  /* Not used on G200: mgag->vbios.fsystem */

  mgag->vbios.opt = (PINS_IN8(54) & 0x7) << 10; /* (only memconfig) */
  mgag->vbios.opt2 = PINS_IN8(58) << 12;
  mgag->vbios.mctlwtst = PINS_IN32(48);
  if (mgag->vbios.mctlwtst == 0xFFFFFFFF)
    mgag->vbios.mctlwtst = 0x01250A21; /* matroxfb default value */
  mgag->vbios.memrdbk = 
    ((PINS_IN8(57) << 21) & 0x1E000000)
    |((PINS_IN8(57) << 22) & 0x00C00000)
    |((PINS_IN8(56) << 1) & 0x000001E0)
    |(PINS_IN8(56) & 0x0000000F);
  KRN_DEBUG(1, "MemRdBk    : %.4x", PINS_IN16(56));
  mgag->vbios.sgram = (PINS_IN8(52) & 0x10) ? 1 : 0;

  KRN_DEBUG(1, "Memory size: %u MB", (PINS_IN8(55) & 0xC0) >> 4);
  KRN_DEBUG(1, "Memory type: %u (0=SDRAM, 1=SGRAM)", (PINS_IN8(52) & 0x10));

  pins_info_345(mgag_io, _pins_offset);

#define DISPLAY_SAVED_FREQ(fre) KRN_DEBUG(1, #fre " = %u kHz", mgag->vbios.fre)
#define DISPLAY_SAVED_REG(reg) KRN_DEBUG(1, #reg " = %.8x", mgag->vbios.reg)

  DISPLAY_SAVED_FREQ(fref);
  DISPLAY_SAVED_REG(opt);
  DISPLAY_SAVED_REG(opt2);
  DISPLAY_SAVED_REG(mctlwtst);
  DISPLAY_SAVED_REG(memrdbk);
  DISPLAY_SAVED_REG(sgram);

#undef DISPLAY_SAVED_FREQ
#undef DISPLAY_SAVED_REG

#undef PINS_IN8
#undef PINS_IN16
#undef PINS_IN32
}

static void parse_pins4(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io)
{
  kgi_u_t _pins_offset = mgag->vbios.pins.offset;
#define PINS_IN8(idx) MGAG_ROM_IN8(mgag_io, _pins_offset + (idx))
#define PINS_IN16(idx) MGAG_ROM_IN16(mgag_io, _pins_offset + (idx))
#define PINS_IN32(idx) MGAG_ROM_IN32(mgag_io, _pins_offset + (idx))

  KRN_DEBUG(1, "Max pixel VCO: %u kHz", (PINS_IN8(39) == 0xFF) ?
	    230000 : (PINS_IN8(39) * 4000));
  KRN_DEBUG(1, "Max system VCO: %u kHz", (PINS_IN8(38) == 0xFF) ?
	    0 : (PINS_IN8(38) * 4000));

  KRN_DEBUG(1, "Max CRTC1 speed (8bpp): %u kHz (%.2x)",
	    PINS_IN8(40) * 4000, PINS_IN8(40));
  KRN_DEBUG(1, "Max CRTC1 speed (16bpp): %u kHz (%.2x)",
	    PINS_IN8(41) * 4000, PINS_IN8(41));
  KRN_DEBUG(1, "Max CRTC2 speed (16bpp): %u kHz (%.2x)",
	    PINS_IN8(44) * 4000, PINS_IN8(44));

  mgag->vbios.fref = (PINS_IN8(92) & 0x01) ? 14318 : 27000;
  mgag->vbios.fsystem = 
    (PINS_IN8(65) == 0xFF) ? 200000 : (PINS_IN8(65) * 4000);
  KRN_DEBUG(2, "VBIOS Freq. system info: %.2x", PINS_IN8(65));

  mgag->vbios.opt =
    ((PINS_IN8(53) << 15) & 0x00400000)
    |((PINS_IN8(53) << 22) & 0x10000000)
    |((PINS_IN8(53) << 10) & 0x00001C00);
  mgag->vbios.opt3 = PINS_IN32(67);
  mgag->vbios.mctlwtst = PINS_IN32(71);
  mgag->vbios.memrdbk = 
    ((PINS_IN8(87) << 21) & 0x1E000000)
    |((PINS_IN8(87) << 22) & 0x00C00000)
    |((PINS_IN8(86) << 1) & 0x000001E0)
    |(PINS_IN8(86) & 0x0000000F);
  KRN_DEBUG(1, "MemRdBk    : %.4x", PINS_IN16(86));
  KRN_DEBUG(1, "MemRdBkMod : %.4x", PINS_IN16(88));

  mgag->vbios.sgram = (PINS_IN8(92) & 0x10) ? 1 : 0;

  KRN_DEBUG(1, "Memory size idx: %u (4,8,16,32MB)", (PINS_IN8(92) >> 2) & 3);
  KRN_DEBUG(1, "Memory type: %u (0=SDRAM, 1=SGLVTTL16)", (PINS_IN8(92) & 16));

  pins_info_345(mgag_io, _pins_offset);

#define DISPLAY_SAVED_FREQ(fre) KRN_DEBUG(1, #fre " = %u kHz", mgag->vbios.fre)
#define DISPLAY_SAVED_REG(reg) KRN_DEBUG(1, #reg " = %.8x", mgag->vbios.reg)

  DISPLAY_SAVED_FREQ(fref);
  DISPLAY_SAVED_FREQ(fsystem);
  DISPLAY_SAVED_REG(opt);
  DISPLAY_SAVED_REG(opt3);
  DISPLAY_SAVED_REG(mctlwtst);
  DISPLAY_SAVED_REG(memrdbk);

#undef DISPLAY_SAVED_FREQ
#undef DISPLAY_SAVED_REG


#undef PINS_IN8
#undef PINS_IN16
#undef PINS_IN32
}

static void parse_pins5(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io)
{
  kgi_u_t _pins_offset = mgag->vbios.pins.offset;
#define PINS_IN8(idx) MGAG_ROM_IN8(mgag_io, _pins_offset + (idx))
#define PINS_IN16(idx) MGAG_ROM_IN16(mgag_io, _pins_offset + (idx))
#define PINS_IN32(idx) MGAG_ROM_IN32(mgag_io, _pins_offset + (idx))

  kgi_u_t vcomult;

  vcomult = PINS_IN8(4) ? 8000 : 6000;
  KRN_DEBUG(1, "Max system VCO: %u kHz (mult=%u)",
	      PINS_IN8(36) * vcomult, vcomult);
  KRN_DEBUG(1, "Max video VCO: %u kHz", PINS_IN8(37) * vcomult);
  KRN_DEBUG(1, "Max pixel VCO: %u kHz", PINS_IN8(38) * vcomult);
  KRN_DEBUG(1, "Min system VCO: %u kHz", PINS_IN8(121) * vcomult);
  KRN_DEBUG(1, "Min video VCO: %u kHz", PINS_IN8(122) * vcomult);
  KRN_DEBUG(1, "Min pixel VCO: %u kHz", PINS_IN8(123) * vcomult);

  KRN_DEBUG(1, "Max DAC1 speed: %u kHz (%.2x)",
	    PINS_IN8(118) * 4000, PINS_IN8(118));
  KRN_DEBUG(1, "Max DAC2 speed: %u kHz (%.2x)",
	    PINS_IN8(119) * 4000, PINS_IN8(119));

  KRN_DEBUG(1, "Max CRTC1 speed (8bpp): %u kHz (%.2x)",
	    PINS_IN8(39) * 4000, PINS_IN8(39));
  KRN_DEBUG(1, "Max CRTC1 speed (16bpp): %u kHz (%.2x)",
	    PINS_IN8(40) * 4000, PINS_IN8(40));
  KRN_DEBUG(1, "Max CRTC2 speed (16bpp): %u kHz (%.2x)",
	    PINS_IN8(43) * 4000, PINS_IN8(43));

  pins_info_345(mgag_io, _pins_offset);

  mgag->vbios.fref = (PINS_IN8(110) & 0x01) ? 14318 : 27000;
  mgag->vbios.fsystem = mgag->vbios.fvideo =
    (PINS_IN8(92) == 0xFF) ? 284000 : (PINS_IN8(92) * 4000);
  KRN_DEBUG(2, "VBIOS Freq. system & video info: %.2x", PINS_IN8(92));

  mgag->vbios.opt = PINS_IN32(48);
  mgag->vbios.opt2 = PINS_IN32(52);
  mgag->vbios.opt3 = PINS_IN32(94);
  mgag->vbios.mctlwtst = PINS_IN32(98);
  mgag->vbios.memmisc = PINS_IN32(102);
  mgag->vbios.memrdbk = PINS_IN32(106);

  mgag->vbios.ddr = ((PINS_IN8(114) & 0x60) == 0x20);
  mgag->vbios.dll = ((PINS_IN8(115) & 0x02) != 0);
  mgag->vbios.emrswen = ((PINS_IN8(115) & 0x01) != 0);
  mgag->vbios.no_wtst_xlat = ((PINS_IN8(115) & 0x04) != 0);

#define DISPLAY_SAVED_FREQ(fre) KRN_DEBUG(1, #fre " = %u kHz", mgag->vbios.fre)
#define DISPLAY_SAVED_REG(reg) KRN_DEBUG(1, #reg " = %.8x", mgag->vbios.reg)
#define DISPLAY_SAVED_FLAG(fla) KRN_DEBUG(1, #fla ": %u", mgag->vbios.fla)

  DISPLAY_SAVED_FREQ(fref);
  DISPLAY_SAVED_FREQ(fsystem);
  DISPLAY_SAVED_FREQ(fvideo);
#if 0
  DISPLAY_SAVED_REG(opt);
  DISPLAY_SAVED_REG(opt2);
  DISPLAY_SAVED_REG(opt3);
  DISPLAY_SAVED_REG(mctlwtst);
  DISPLAY_SAVED_REG(memmisc);
  DISPLAY_SAVED_REG(memrdbk);
  DISPLAY_SAVED_FLAG(ddr);
  DISPLAY_SAVED_FLAG(dll);
  DISPLAY_SAVED_FLAG(emrswen);
  DISPLAY_SAVED_FLAG(no_wtst_xlat);
#endif

#undef DISPLAY_SAVED_FREQ
#undef DISPLAY_SAVED_REG
#undef DISPLAY_SAVED_FLAG

  KRN_DEBUG(1, "ddr: %u, dll: %u, emrswen: %u",
	    ((PINS_IN8(114) & 0x60) == 0x20),
	    ((PINS_IN8(115) & 0x02) != 0),
	    ((PINS_IN8(115) & 0x01) != 0));

#undef PINS_IN8
#undef PINS_IN16
#undef PINS_IN32
}

static void parse_bios(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io)
{
  kgi_u16_t pins_offset;
  kgi_u8_t t1,t2;

  /* Check VBIOS validity against its first two bytes */
  t1 = MGAG_ROM_IN8(mgag_io, 0);
  t2 = MGAG_ROM_IN8(mgag_io, 1);
  KRN_DEBUG(2, "2 first bytes of Matrox VBIOS: %.2x %.2x", t1, t2);
  if ((t1 != 0x55) && (t2 != 0xAA))
    {
      KRN_ERROR("Matrox VBIOS invalid fingerprint");
      return;
    }
  /* Get VBIOS version */
  {
    kgi_u16_t pcir_offset;
    kgi_u8_t tmp, rev;
    
    pcir_offset = MGAG_ROM_IN16(mgag_io, 24);
    if (pcir_offset >= 26 && pcir_offset < 0xFFE0 &&
	MGAG_ROM_IN8(mgag_io, pcir_offset) == 'P' &&
	MGAG_ROM_IN8(mgag_io, pcir_offset + 1) == 'C' &&
	MGAG_ROM_IN8(mgag_io, pcir_offset + 2) == 'I' &&
	MGAG_ROM_IN8(mgag_io, pcir_offset + 3) == 'R')
      {
	  tmp = MGAG_ROM_IN8(mgag_io, pcir_offset + 0x12);
	  rev = MGAG_ROM_IN8(mgag_io, pcir_offset + 0x13);
      }
    else
      {
	tmp = MGAG_ROM_IN8(mgag_io, 5);
	rev = 0;
      }
    mgag->vbios.version.major = (tmp >> 4) & 0xF;
    mgag->vbios.version.minor = tmp & 0xF;
    mgag->vbios.version.revision = rev;
    KRN_DEBUG(1, "Matrox Video Bios is version %u.%u, revision %u",
	      mgag->vbios.version.major, mgag->vbios.version.minor,
	      mgag->vbios.version.revision);
  }
  /* Get pins version and length */
  pins_offset = MGAG_ROM_IN16(mgag_io, 0x7FFC);
  {
    kgi_u8_t plength, pversion;
    kgi_u16_t pcheck;

    KRN_DEBUG(2, "Matrox VBIOS pins location: %.4x", pins_offset);
    pcheck = MGAG_ROM_IN16(mgag_io, pins_offset);
    KRN_DEBUG(2, "Matrox VBIOS pins fingerprint: %.4x", pcheck);
    if (pcheck == 0x412E)
      {
	plength = MGAG_ROM_IN8(mgag_io, pins_offset + 2);
	pversion = MGAG_ROM_IN8(mgag_io, pins_offset + 5);
      }
    else if (pcheck == 0x0040)
      {
	plength = 0x40;
	pversion = 1; /* We assume version 1 */
      }
    else
      {
	KRN_ERROR("Unknown Matrox pins fingerprint %.4x", pcheck);
	pversion = plength = 0;
      }
    KRN_DEBUG(1, "Matrox pins length: %.2x, version: %.2x", plength, pversion);
    /* Saves data */
    mgag->vbios.pins.version = pversion;
    mgag->vbios.pins.length = plength;
    mgag->vbios.pins.offset = pins_offset;
  }
  {
    /* Check/validate pins version against known length (1,2,3:64, 4,5:128) */
    static const kgi_u8_t known_length[] = { 64, 64, 64, 128, 128 };
    if (mgag->vbios.pins.length != known_length[mgag->vbios.pins.version - 1])
      {
	KRN_ERROR("Abnormal length of pins info"
		  " (version:%u, length: %u, expected:%u)",
		  mgag->vbios.pins.version, mgag->vbios.pins.length,
		  known_length[mgag->vbios.pins.version - 1]);
	mgag->vbios.pins.version = 0;
      }
  }
  /* Now goes to pins parsing */
  switch (mgag->vbios.pins.version)
    {
    case 0:
      KRN_ERROR("Erroneous/unknown pins data, powerup info parsing cancelled.");
      break;
    case 1:
      KRN_ERROR("Parsing powerup information for pins version %u"
		" not yet implemented!", mgag->vbios.pins.version);
      break;
    case 2:
      parse_pins2(mgag, mgag_io);
      break;
    case 3:
      parse_pins3(mgag, mgag_io);
      break;
    case 4:
      parse_pins4(mgag, mgag_io);
      break;
    case 5:
      parse_pins5(mgag, mgag_io);
      break;
    default:
      KRN_ERROR("Powerup info version %u not yet supported",
		mgag->vbios.pins.version);
      break;
    }
}


/* ----------------------------------------------------------------------------
**	Main functions
** ----------------------------------------------------------------------------
*/

kgi_error_t mgag_chipset_init(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
			      const kgim_options_t *options)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);

  KRN_ASSERT(mgag);
  KRN_ASSERT(mgag_io);
  KRN_ASSERT(options);

  KRN_DEBUG(2, "initializing %s %s", mgag->chipset.vendor,
	    mgag->chipset.model);

  if (mgag->flags & MGAG_CF_OLD)
    {
      PCICFG_SET_BASE32(mgag_io->control.base_io,
			pcidev + PCI_BASE_ADDRESS_0);
      PCICFG_SET_BASE32(mgag_io->fb.base_io,
			pcidev + PCI_BASE_ADDRESS_1);
    }
  else
    {
      PCICFG_SET_BASE32(mgag_io->control.base_io,
			pcidev + PCI_BASE_ADDRESS_1);
      PCICFG_SET_BASE32(mgag_io->fb.base_io,
			pcidev + PCI_BASE_ADDRESS_0);
    }

  PCICFG_SET_BASE32(mgag_io->iload.base_io, pcidev + PCI_BASE_ADDRESS_2);
  PCICFG_SET_BASE32(mgag_io->rom.base_io | PCI_ROM_ADDRESS_ENABLE, pcidev + PCI_ROM_ADDRESS);

  /* initialize PCI command register */

  pcicfg_out16(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER, pcidev + PCI_COMMAND);

  /* enable/force rom access */
  pcicfg_out32(pcicfg_in32(pcidev + MGAG_PCI_OPTION1) | MGAG_O1_BIOSEN,
	       pcidev + MGAG_PCI_OPTION1);
  /* TODO: Restore the original state of ROM access? (and unmap region
     TODO: if biosen is 0?) */
  /* Matrox video BIOS analysis */
  parse_bios(mgag, mgag_io);

  /* if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450)) */
    {
      /* According to matroxfb source code, this is always needed
       * to prevent unexpected accelerator failure
       */
      MGAG_GC_OUT32(mgag_io, 0, ZORG);
    }

  if (mgag->flags & MGAG_CF_PRIMARY)
    {
      KRN_DEBUG(2, "chipset already initialized.");	  
    }
  else
    {
      KRN_DEBUG(2, "chipset not initialized: doing reset/power-up");
      /* Power up the chipset */
      if (mgag->flags & (MGAG_CF_G550 | MGAG_CF_G450 | MGAG_CF_G400 | MGAG_CF_G200))
	{
	  KRN_DEBUG(1, "Starting (matroxfb+VBIOS)-based power-up sequence");
	  mgag_chipset_power_up_per_vbios(mgag, mgag_io);
	}
      else
	{
	  KRN_DEBUG(1, "Trying hand-made power-up sequence");
	  mgag_chipset_power_up(mgag, mgag_io);
	}
    }

  /* Reading configuration and saving it
  ** (NB: Most of the VGA part is done by vga_text_chipset_init())
  */
  KRN_DEBUG(2, "chipset initialized, saving configuration...");
  {
    kgi_u_t i;

    for (i = 0; i < MGAG_ECRT_REGS; ++i)
      mgag->ECRT[i] = MGAG_ECRT_IN8(mgag_io, i);
  }

  /* Now proceeds with initialisation
   */
  mgag->chipset.memory = mgag_chipset_memory_count(mgag, mgag_io);

  KRN_NOTICE("%i bytes framebuffer detected.", mgag->chipset.memory);

  /*
  ** Initialize pointer shape access related fields
  */
  mgag_io->ptr_fboffset = mgag->chipset.memory - POINTER_FB_AREA_SIZE;
  mgag_io->flags |= MGAG_POINTER_FB_READY;
  MGAG_EDAC_OUT8(mgag_io, ((mgag_io->ptr_fboffset >> 10) >> 8) & 0x3F,
		 XCURADDH);
  MGAG_EDAC_OUT8(mgag_io, (mgag_io->ptr_fboffset >> 10) & 0xFF, XCURADDL);

  KRN_DEBUG(1, "pointer shape fb offset = %.8x", mgag_io->ptr_fboffset);

  KRN_TRACE(2, mgag_chipset_examine(mgag));

  KRN_TRACE(1, mgag_chipset_probe(mgag,mgag_io));

  /* Calls the VGA-text driver initialization procedure
   */
  vga_text_chipset_init(&(mgag->vga),&(mgag_io->vga),options);

  if (mgag->flags & MGAG_CF_AGP_ENABLED)
    {
      /* Ensures that the AGP PLL 2X oscillations are enabled */
      MGAG_GC_OUT32(mgag_io, AGP_PLL_AGP2XPLLEN, AGP_PLL);
    }

  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
    {
      mgag_chipset_irq_enable(mgag_io);
    }

  KRN_DEBUG(2, "chipset enabled");

  return KGI_EOK;
}

void mgag_chipset_done(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
		       const kgim_options_t *options)
{
  pcicfg_vaddr_t pcidev = MGAG_PCIDEV(mgag_io);
  kgi_u_t i;

  mgag_chipset_wait_engine_idle(mgag_io);

  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
    {
      KRN_DEBUG(2, "Disabling all interrupts");
      /* Disable all interrupts */
      mgag_chipset_irq_block(mgag_io, NULL);

      /* Output IRQ stats */
      KRN_DEBUG(1, "IRQ stats: handler_total=%i, not_handled=%i",
		mgag->interrupt.handler_total,
		mgag->interrupt.not_handled);
      KRN_DEBUG(1, "IRQ stats: pick=%i, vsync=%i, vline=%i, ext=%i",
		mgag->interrupt.pick,
		mgag->interrupt.vsync,
		mgag->interrupt.vline,
		mgag->interrupt.ext);
      KRN_DEBUG(1, "IRQ stats (G200+): softrap=%i, warp=%i, warpcache=%i",
		mgag->interrupt.softtrap,
		mgag->interrupt.warp,
		mgag->interrupt.warpcache);
      KRN_DEBUG(1, "IRQ stats (G400+): c2vline=%i, warp1=%i, warpcache1=%i",
		mgag->interrupt.c2vline,
		mgag->interrupt.warp1,
		mgag->interrupt.warpcache1);
    }

  /* Restoring chipset state
   */
  KRN_DEBUG(2, "restoring initial chipset state");

  for (i = 0; i < MGAG_ECRT_REGS; ++i)
    mgag->ECRT[i] = MGAG_ECRT_IN8(mgag_io, i);

  /* Calling the VGA-text driver restore procedure */
  vga_text_chipset_done(&(mgag->vga), &(mgag_io->vga), options);

  /* Finally restoring PCI config regs (saved by the binding driver) */
  pcicfg_out32(mgag->pci.Command, pcidev + PCI_COMMAND);
  pcicfg_out32(mgag->pci.LatTimer, pcidev + PCI_LATENCY_TIMER);
  pcicfg_out32(mgag->pci.IntLine, pcidev + PCI_INTERRUPT_LINE);

  PCICFG_SET_BASE32(mgag->pci.BaseAddr0, pcidev + PCI_BASE_ADDRESS_0);
  PCICFG_SET_BASE32(mgag->pci.BaseAddr1, pcidev + PCI_BASE_ADDRESS_1);
  PCICFG_SET_BASE32(mgag->pci.BaseAddr2, pcidev + PCI_BASE_ADDRESS_2);
}


kgi_error_t mgag_chipset_mode_check(mgag_chipset_t *mgag,
	mgag_chipset_io_t *mgag_io, mgag_chipset_mode_t *mgag_mode,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	/*	most of the first part of this functiion is a big mess, 
	**	and completely incorrect, but somehow it compiles..
	*/

	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = mgag_mode->kgim.crt;

	kgi_u_t	shift	= 0,
		bpf	= 0,
		bpc	= 0,
		bpp	= 0,
		pgm	= 0,
		lclk	= 0,
		pp[3]	= { 0, 0, 0 };

	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t bpd;

	KRN_DEBUG(1, "mgag_chipset_mode_check initiated");

	if (images != 1) {

		KRN_ERROR("%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
	    (img[0].fam & KGI_AM_TEXTURE_INDEX)) {
	  KRN_DEBUG(2, "doing VGA-text mode_check");
	  if (mgag->flags & MGAG_CF_PRIMARY) {
	    return vga_text_chipset_mode_check(&mgag->vga, &mgag_io->vga,
					    &mgag_mode->vga, cmd, img,images);
	  } else {
	    kgi_error_t err;
	    err = vga_text_chipset_mode_check(&mgag->vga, &mgag_io->vga,
					      &(mgag_mode->compat.vga),
					      cmd, img,images);
	    /* Provide some hacked "unsupported" resources to offer
	     * a text mode on a secondary display even if theoretically
	     * unsupported.
	     * These do not use the fixed address 0xA0000 aperture but
	     * goes through the usual fb aperture modulo some address
	     * mangling.
	     */
	    KRN_DEBUG(1, "providing unofficial text mode "
		      "on a secondary Gx00/1x64 chipset");
	    /* Init flags */
	    mgag_mode->compat.flags = 0;
	    /* Stores bus sizes */
	    if ((mgag->flags & MGAG_CF_1x64) || (mgag->flags & MGAG_CF_G200))
	      {
		mgag_mode->compat.flags |= MGAG_CF_64BITS_BUS;
	      }
	    else if ((mgag->flags & MGAG_CF_G400)
		     /* TODO: Check the bus size of G550! */
		     || (mgag->flags & MGAG_CF_G550))
	      {
		mgag_mode->compat.flags |= MGAG_CF_128BITS_BUS;
	      }
	    else
	      {
		KRN_ERROR("Unknown chipset bus size!");
		return -KGI_ERRNO(CHIPSET, NOSUP);
	      }

	    /*	(compat) text16 handling
	     */
	    mgag_mode->compat.text16.meta	= mgag_mode;
	    mgag_mode->compat.text16.meta_io	= mgag_io;
	    mgag_mode->compat.text16.type	= KGI_RT_TEXT16_CONTROL;
	    mgag_mode->compat.text16.prot	= KGI_PF_DRV_RWS;
	    mgag_mode->compat.text16.name	=
	      "Gx00/1x64 *UNSUPPORTED* text16 control";
	    mgag_mode->compat.text16.size.x	= img[0].size.x;
	    mgag_mode->compat.text16.size.y	= img[0].size.y;
	    mgag_mode->compat.text16.virt.x	= img[0].virt.x;
	    mgag_mode->compat.text16.virt.y	= img[0].virt.y;
	    mgag_mode->compat.text16.cell.x	= 9;
	    mgag_mode->compat.text16.cell.y	= 16;
	    mgag_mode->compat.text16.font.x	= 8;
	    mgag_mode->compat.text16.font.y	= 16;
	    mgag_mode->compat.text16.PutText16	=
	      mgag_chipset_compat_put_text16;

	    /*	(compat) texture look up table control
	     */
	    mgag_mode->compat.tlut_ctrl.meta	= mgag_mode;
	    mgag_mode->compat.tlut_ctrl.meta_io	= mgag_io;
	    mgag_mode->compat.tlut_ctrl.type	= KGI_RT_TLUT_CONTROL;
	    mgag_mode->compat.tlut_ctrl.prot	= KGI_PF_DRV_RWS;
	    mgag_mode->compat.tlut_ctrl.name	=
	      "Gx00/1x64 *UNSUPPORTED* tlut control";
	    mgag_mode->compat.tlut_ctrl.Set	= mgag_chipset_compat_set_tlut;

	    /*	(compat) cursor setup
	     */
	    mgag_mode->compat.cursor_ctrl.meta = mgag_mode;
	    mgag_mode->compat.cursor_ctrl.meta_io = mgag_io;
	    mgag_mode->compat.cursor_ctrl.type = KGI_RT_CURSOR_CONTROL;
	    mgag_mode->compat.cursor_ctrl.prot = KGI_PF_DRV_RWS;
	    mgag_mode->compat.cursor_ctrl.name	=
	      "Gx00/1x64 *UNSUPPORTED* text cursor control";
	    mgag_mode->compat.cursor_ctrl.size.x = 1;
	    mgag_mode->compat.cursor_ctrl.size.y = 1;
	    mgag_mode->compat.cursor_ctrl.Show = mgag_chipset_compat_show_hc;
	    mgag_mode->compat.cursor_ctrl.Hide = mgag_chipset_compat_hide_hc;
	    mgag_mode->compat.cursor_ctrl.Undo = mgag_chipset_compat_undo_hc;

	    return err;
	  }
	}

	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL))
	{
		KRN_ERROR("image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* check if common attributes are supported.
	 */
	switch (img[0].cam) {

	case 0:
		break;

	default:
		KRN_ERROR("common attributes %.8x not supported", img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* total bits per dot */

	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO) ?
		(bpc + bpf*img[0].frames*2) :
		(bpc + bpf*img[0].frames);

	shift = 0;

	switch (bpd) {

	case  1:	shift++;	/* fall through	*/
	case  2:	shift++;	/* fall through */
	case  4:	shift++;	/* fall through	*/
	case  8:	shift++;	/* fall through	*/
	case 16:	shift++;	/* fall through	*/
	case 32:	shift++;
			pgm = (pgm << shift) - 1;
			break;

	default:	KRN_ERROR("%i bpd not supported", bpd);
			return -KGI_ERRNO(CHIPSET, FAILED);
	}

	lclk = (cmd == KGI_TC_PROPOSE) ? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		/* if size.x or size.y are zero, default to 640x400 */

		if ((0 == img[0].size.x) || (0 == img[0].size.y)) {

			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/* if virt.x and virt.y are zero, default to size. if either virt.x xor virt.y is zero, maximize the other */

		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) {

			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x)	{

			img[0].virt.x = (8 * mgag->chipset.memory) / (img[0].virt.y * bpp);

			if (img[0].virt.x > mgag->chipset.maxdots.x) {

				img[0].virt.x = mgag->chipset.maxdots.x;
			}
		}

		if (0 == img[0].virt.y)	{

			img[0].virt.y = (8 * mgag->chipset.memory) / (img[0].virt.x * bpp);
		}

		/* Are we beyond the limits of the H/W?	*/

		if ((img[0].size.x >= mgag->chipset.maxdots.x)
		    || (img[0].virt.x >= mgag->chipset.maxdots.x)) {

			KRN_ERROR("%i (%i) x pixels exceed %i",
				  img[0].size.x, img[0].virt.x,
				  mgag->chipset.maxdots.x);

			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= mgag->chipset.maxdots.y)
		    || (img[0].virt.y >= mgag->chipset.maxdots.y)) {

			KRN_ERROR("%i (%i) y pixels exceed %i",
				  img[0].size.y, img[0].virt.y,
				  mgag->chipset.maxdots.y);

			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > (8 * mgag->chipset.memory)) {

			KRN_ERROR("not enough memory (%ipf*%if + %ipc)@%ix%i",
				  bpf, img[0].frames, bpc,
				  img[0].virt.x, img[0].virt.y);

			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

		/* set CRT visible fields */

		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		if (img[0].size.y < 400) {

			dpm->dots.y += img[0].size.y;
		}

		return KGI_EOK;

	case KGI_TC_LOWER:
		if (dpm->dclk < mgag->chipset.dclk.min) {

			KRN_ERROR("DCLK = %i Hz is too low", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		}
		return KGI_EOK;

	case KGI_TC_RAISE:

		if (dpm->dclk > mgag->chipset.dclk.max) {

			KRN_ERROR("DCLK = %i Hz is too high", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		}
		return KGI_EOK;

	case KGI_TC_CHECK:

		KRN_ASSERT(pp[0] < 8);
		KRN_ASSERT(pp[1] < 8);
		KRN_ASSERT(pp[2] < 8);

#if 0
		if (width != img[0].virt.x) {

KRN_DEBUG(1, "width == %i different from img[0].virt.x == %i",
          width, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
#endif

		if ((img[0].size.x >= mgag->chipset.maxdots.x) ||
		    (img[0].size.y >= mgag->chipset.maxdots.y) ||
		    (img[0].virt.x >= mgag->chipset.maxdots.x) ||
		   ((img[0].virt.y * img[0].virt.x * bpp) > (8 * mgag->chipset.memory))) {

			KRN_ERROR("resolution too high: %ix%i (%ix%i)",
					img[0].size.x,
					img[0].size.y,
					img[0].virt.x,
					img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}

	/* Now everything is checked and should be sane. proceed to setup device dependent mode. */

	bpd = kgim_attr_bits(dpm->bpda);

	mgag_mode->mgag.Misc	   = ((crt_mode->x.polarity > 0) ? 0 : VGA_MISC_NEG_HSYNC |
			      (crt_mode->y.polarity > 0) ? 0 : VGA_MISC_NEG_VSYNC | MISC_CLOCK_1X | VGA_MISC_COLOR_IO);
	mgag_mode->mgag.Misc |= VGA_MISC_ENB_RAM;

	mgag_mode->mgag.Offset  = ((img[0].virt.x * bpp) / 128);
#warning check constraints on offset/pitch (see p4-74)

	/* Clearly, the character clocks for VGA mode corresponds here
	 * to 8 pixels in Power Graphic (aka SVGA) Mode.
	 * But this is also linked to the video clock divider (CRTCEXT3).
	 */
	mgag_mode->mgag.HTotal  = (crt_mode->x.total     / 8) - 5;
	mgag_mode->mgag.HdEnd   = (crt_mode->x.width     / 8) - 1;
	mgag_mode->mgag.HsStart = (crt_mode->x.syncstart / 8);
	mgag_mode->mgag.HsEnd   = (crt_mode->x.syncend   / 8);
	mgag_mode->mgag.HbStart = (crt_mode->x.width     / 8) - 1;
	mgag_mode->mgag.HbEnd   = (crt_mode->x.total     / 8) - 1;

	mgag_mode->mgag.VTotal  = crt_mode->y.total - 2;
	mgag_mode->mgag.VdEnd   = crt_mode->y.width - 1;
	mgag_mode->mgag.VsStart = crt_mode->y.syncstart;
	mgag_mode->mgag.VsEnd   = crt_mode->y.syncend;
	mgag_mode->mgag.VbStart = crt_mode->y.width - 1;
	mgag_mode->mgag.VbEnd   = crt_mode->y.total - 1;

	mgag_mode->mgag.CRTC[0x00] =   mgag_mode->mgag.HTotal  & 0xFF;
	mgag_mode->mgag.CRTC[0x01] =   mgag_mode->mgag.HdEnd   & 0xFF;
	mgag_mode->mgag.CRTC[0x02] =   mgag_mode->mgag.HbStart & 0xFF;
	/* NB: Do not touch VGA_CR03_IS_VGA in CRTC[0x03], it freezes
	   the Matrox Mystique (at least a 1064 rev.2) */
	mgag_mode->mgag.CRTC[0x03] =   mgag_mode->mgag.HbEnd   & 0x1F;
	mgag_mode->mgag.CRTC[0x04] =   mgag_mode->mgag.HsStart & 0xFF;
	mgag_mode->mgag.CRTC[0x05] =  (mgag_mode->mgag.HsEnd   & 0x1F) |
				((mgag_mode->mgag.HbEnd   & 0x20) << 2);
	mgag_mode->mgag.CRTC[0x06] =   mgag_mode->mgag.VTotal  & 0xFF;
	mgag_mode->mgag.CRTC[0x07] = ((mgag_mode->mgag.VTotal  & 0x100) >> 8) |
				((mgag_mode->mgag.VdEnd   & 0x100) >> 7) |
				((mgag_mode->mgag.VsStart & 0x100) >> 6) |
				((mgag_mode->mgag.VbStart & 0x100) >> 5) |
				((mgag_mode->mgag.LnComp  & 0x100) >> 4) |
				((mgag_mode->mgag.VTotal  & 0x200) >> 4) |
				((mgag_mode->mgag.VdEnd   & 0x200) >> 3) |
				((mgag_mode->mgag.VsStart & 0x200) >> 2);

	mgag_mode->mgag.CRTC[0x08] = 0x00;
	mgag_mode->mgag.CRTC[0x09] = ((mgag_mode->mgag.LnComp  & 0x200) >> 3) |
				((mgag_mode->mgag.VbStart & 0x200) >> 4);
	mgag_mode->mgag.CRTC[0x0A] = 0x00; /* No text cursor */
	mgag_mode->mgag.CRTC[0x0B] = 0x00; /* No text cursor */
	/* TODO: do we need to set the Start Address? */
	mgag_mode->mgag.CRTC[0x0C] = 0x00;
	mgag_mode->mgag.CRTC[0x0D] = 0x00;
	mgag_mode->mgag.CRTC[0x0E] = 0x00; /* No text cursor */
	mgag_mode->mgag.CRTC[0x0F] = 0x00; /* No text cursor */
	mgag_mode->mgag.CRTC[0x10] =   mgag_mode->mgag.VsStart & 0xFF;
	mgag_mode->mgag.CRTC[0x11] =  (mgag_mode->mgag.VsEnd   & 0x0F)
	  | ((mgag->flags & MGAG_CF_IRQ_CLAIMED) ? VGA_CR11_CLEAR_VSYNC_IRQ : VGA_CR11_DISABLE_VSYNC_IRQ);
	mgag_mode->mgag.CRTC[0x12] =   mgag_mode->mgag.VdEnd   & 0xFF;
	mgag_mode->mgag.CRTC[0x13] =   mgag_mode->mgag.Offset  & 0xFF;
	mgag_mode->mgag.CRTC[0x14] =   0x00;
	mgag_mode->mgag.CRTC[0x15] =   mgag_mode->mgag.VbStart & 0xFF;
	mgag_mode->mgag.CRTC[0x16] =   mgag_mode->mgag.VbEnd   & 0xFF;
	mgag_mode->mgag.CRTC[0x17] =   VGA_CR17_CGA_BANKING | VGA_CR17_HGC_BANKING
	  | VGA_CR17_BYTE_MODE | VGA_CR17_ENABLE_SYNC;
	mgag_mode->mgag.CRTC[0x18] =   mgag_mode->mgag.LnComp & 0xFF;
	/* CRTC22, CRTC24, CRTC26 not needed/handled */

	/* TODO: do we need to set the Start Address? */
	mgag_mode->mgag.ECRTC[0]   =  (mgag_mode->mgag.Offset  & 0x300) >> 4;
	mgag_mode->mgag.ECRTC[1]   = ((mgag_mode->mgag.HTotal  & 0x100) >> 8) |
				((mgag_mode->mgag.HbStart & 0x100) >> 7) |
				((mgag_mode->mgag.HsStart & 0x100) >> 6) |
				 (mgag_mode->mgag.HbEnd   & 0x40);
	mgag_mode->mgag.ECRTC[2]   = ((mgag_mode->mgag.VTotal  & 0xC00) >> 10) |
				((mgag_mode->mgag.VdEnd   & 0x400) >> 8)  |
				((mgag_mode->mgag.VbStart & 0xC00) >> 7)  |
				((mgag_mode->mgag.VsStart & 0xC00) >> 5)  |
				((mgag_mode->mgag.LnComp  & 0x400) >> 3);
	{
	  int vcsf = 0; /* Video Clocking Scale Factor */
	  switch (bpp) {
	  case 8:
	    vcsf = 0; break;
	  case 16:
	    vcsf = 1; break;
	  case 24:
	    vcsf = 2; break;
	  case 32:
	    vcsf = 3; break;
	  default:
	    KRN_ERROR("%i bpp not supported", bpp);
	    return -KGI_ERRNO(CHIPSET, FAILED);
	  }
	  mgag_mode->mgag.ECRTC[3] = (vcsf & ECRT3_SCALE_MASK)
	    | ECRT3_MGAMODE;
	}
	mgag_mode->mgag.ECRTC[4] = 0x00;
	mgag_mode->mgag.ECRTC[5] = 0x00; /* Not used in non-interlaced mode */
	/* Registers 6 to 8 are left at their reset values */
	mgag_mode->mgag.ECRTC[6] = 0x00;
	mgag_mode->mgag.ECRTC[7] = 0x00;
	mgag_mode->mgag.ECRTC[8] = 0x00;

	/* initialize exported resources */

	r		= &mgag_mode->mgag.fb;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_FRAME_BUFFER;
	r->prot		= KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name		= "MGA 1x64/Gx00 Framebuffer";
	r->access	= 64 + 32 + 16 + 8;
	r->align	= 64 + 32 + 16 + 8;
	r->size		= r->win.size
	  = mgag->chipset.memory /* - POINTER_FB_AREA_SIZE */;
	r->win.bus	= mgag_io->fb.base_bus;
	r->win.phys	= mgag_io->fb.base_phys;
	r->win.virt	= mgag_io->fb.base_virt;

	r		= &mgag_mode->mgag.iload;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_PRIVATE;
	r->prot		= KGI_PF_LIB | KGI_PF_DRV | KGI_PF_WRITE_ORDER;
	r->name		= "MGA 1x64/Gx00 Pseudo-DMA ILOAD aperture";
	/* TODO: I wonder if accesses should not be on dwords... */
	r->access	= 64 + 32 + 16 + 8;
	r->align	= 64 + 32 + 16 + 8;
	/*	r->size		= r->win.size = MGAG_MGABASE3_SIZE;*/
	r->size		= r->win.size = mgag_io->iload.size;
	r->win.bus	= mgag_io->iload.base_bus;
	r->win.phys	= mgag_io->iload.base_phys;
	r->win.virt	= mgag_io->iload.base_virt;

	r		= &mgag_mode->mgag.control;
	r->meta		= mgag;
	r->meta_io	= mgag_io;
	r->type		= KGI_RT_MMIO_PRIVATE;
	r->prot		= KGI_PF_LIB | KGI_PF_DRV | KGI_PF_WRITE_ORDER;
	r->name		= "MGA 1x64/Gx00 graphics control";
	r->access	= 32 + 16 + 8;
	r->align	= 32 + 16 + 8;
	r->size		= r->win.size = mgag_io->control.size;
	r->win.bus	= mgag_io->control.base_bus;
	r->win.phys	= mgag_io->control.base_phys;
	r->win.virt	= mgag_io->control.base_virt;

	a = &mgag_mode->mgag.engine;
	a->meta = mgag;
	a->meta_io = mgag_io;
	a->type = KGI_RT_ACCELERATOR;
	a->prot = KGI_PF_LIB | KGI_PF_DRV; 
	a->name = "Gx00 engine"; /* shorter for libggi sublib "Matrox Gx00 engine"; */
	a->flags |= KGI_AF_DMA_BUFFERS;
	a->buffers = 3;
	a->buffer_size = 8 KB; /* TODO: Should be 7 KB ? */
	a->context = NULL;
	a->context_size = sizeof(mgag_chipset_accel_context_t);
	a->execution.queue = NULL;
#warning initialize a->idle!!!
	a->Init = mgag_chipset_accel_init;
	a->Done = mgag_chipset_accel_done;
	a->Exec = mgag_chipset_accel_exec;

	return KGI_EOK;
}

kgi_resource_t *mgag_chipset_mode_resource(mgag_chipset_t *mgag, 
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, 
	kgi_u_t images, kgi_u_t index) 
{
  /* Handles (VGA) text mode specific issues first */
  if (img->fam & KGI_AM_TEXTURE_INDEX) {
    if (mgag->flags & MGAG_CF_PRIMARY) {
      /* Calls the VGA-text driver procedure for a VGA text mode */
      return vga_text_chipset_mode_resource(&(mgag->vga), &(mgag_mode->vga),
					    img, images, index);
    } else {
      /* If we are a secondary card, we do not provide any mode
       * resource, especially, no MMIO fb.
       */
      return NULL;
    }
  }

  /* Then normal graphic mode resources */
  switch (index) {

  case 0:	return (kgi_resource_t *) &mgag_mode->mgag.fb;
  case 1:	return (kgi_resource_t *) &mgag_mode->mgag.iload;
  case 2:	return (kgi_resource_t *) &mgag_mode->mgag.engine;

  }

  return NULL;
}

kgi_resource_t *mgag_chipset_image_resource(mgag_chipset_t *mgag,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
  KRN_ASSERT(image == 0);

  /* Handles (VGA) text mode specific issues first */
  if (img[0].fam & KGI_AM_TEXTURE_INDEX) {
    if (mgag->flags & MGAG_CF_PRIMARY) {
      /* We fall back to the VGA-text driver */
      return vga_text_chipset_image_resource(&(mgag->vga),
			&(mgag_mode->vga), img, image, index);
    } else {
      /* We provide our own "unsupported officially" resources. */
      switch (index) {
      case 0: return (kgi_resource_t *) &(mgag_mode->compat.cursor_ctrl);
      case 1: return (kgi_resource_t *) &(mgag_mode->compat.tlut_ctrl);
      case 2: return (kgi_resource_t *) &(mgag_mode->compat.text16);
      }
      return NULL;
    }
  }

  /* Then normal graphic image resources */

  /* NONE YET */

  return NULL;
}

void mgag_chipset_mode_prepare(mgag_chipset_t *mgag,
			       mgag_chipset_io_t *mgag_io,
			       mgag_chipset_mode_t *mgag_mode,
			       kgi_image_mode_t *img,
			       kgi_u_t images)
{
  mgag_chipset_wait_engine_idle(mgag_io);

  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
    {
      mgag_chipset_irq_block(mgag_io, NULL);
    }

  /* Handle text mode first */
  if (img->fam & KGI_AM_TEXTURE_INDEX)
    {
      kgi_u_t i;
      
      /* VGA mode preparation */
      mgag_io->flags |= MGAG_IF_VGA_MODE;
      
      /* Sets all CRTCEXT registers to their reset value (0x00) */
      for (i = 0; i < MGAG_ECRT_REGS; ++i)
	{
	  MGAG_ECRT_OUT8(mgag_io, 0x00, i);
	}
   
      /* Prepare specifically for a VGA mode (probably redundant) */
      MGAG_ECRT_OUT8(mgag_io,
		     MGAG_ECRT_IN8(mgag_io, ECRT3) & ~ECRT3_MGAMODE,
		     ECRT3);

      /* Now calls the VGA-text driver function */
      if (mgag->flags & MGAG_CF_PRIMARY)
	{
	  vga_text_chipset_mode_prepare(&mgag->vga, &mgag_io->vga,
					&mgag_mode->vga, img, images);
	}
      else
	{
	  vga_text_chipset_mode_prepare(&mgag->vga, &mgag_io->vga,
					&(mgag_mode->compat.vga), img, images);
	}

      KRN_DEBUG(2, "prepared for VGA mode");
    
      return; /* Early return */
    }

  /* Normal mode preparation */
  mgag_io->flags &= ~MGAG_IF_VGA_MODE;

  /* Prepare for a SVGA mode */
  MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT3) | ECRT3_MGAMODE,
		 ECRT3);
  
  KRN_DEBUG(2, "prepared for MGA/Gx00 mode");
}

void mgag_chipset_mode_enter(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u_t	i = 0;

	mgag_chipset_wait_engine_idle(mgag_io);

	/* Manages a VGA text mode
	 */
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

	  /* Calls the VGA-text driver function */
	  if (mgag->flags & MGAG_CF_PRIMARY) {
	    vga_text_chipset_mode_enter(&mgag->vga, &mgag_io->vga,
					&mgag_mode->vga, img, images);
	  } else {
	    vga_text_chipset_mode_enter(&mgag->vga, &mgag_io->vga,
					&(mgag_mode->compat.vga), img, images);
	  }
#if 0
	  KRN_TRACE(2, mgag_chipset_probe(mgag,mgag_io));
#endif

	  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
	    {
	      mgag_chipset_irq_enable(mgag_io);
	    }

	  return; /* Early return */
	}

#if 0
	/* turning off display */

	MGAG_SEQ_OUT8(mgag_io, MGAG_SEQ_IN8(mgag_io, VGA_SEQ_CLOCK) | VGA_SR01_DISPLAY_OFF, VGA_SEQ_CLOCK);

	MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT1) | ECRT1_HSYNCOFF | ECRT1_VSYNCOFF, ECRT1);
#endif
	/* programming mode. */

	/* Programming the Misc - preserve the clock selection bits */
	MGAG_MISC_OUT8(mgag_io,
		      (MGAG_MISC_IN8(mgag_io) & VGA_MISC_CLOCK_MASK)
		      | (mgag_mode->mgag.Misc & ~VGA_MISC_CLOCK_MASK));

	for (i = 0; i < NrCRTRegs; i++) {

		MGAG_CRT_OUT8(mgag_io, mgag_mode->mgag.CRTC[i], i);
	}

	for (i = 0; i < NrECRTRegs; i++) {

		MGAG_ECRT_OUT8(mgag_io, mgag_mode->mgag.ECRTC[i], i);
	}

	/* need to reenable the display, but this can't be the space.
	   everything else has to be complete by that time. */

#if 0
	/* turning ON display */

	MGAG_SEQ_OUT8(mgag_io, MGAG_SEQ_IN8(mgag_io, VGA_SEQ_CLOCK) & ~VGA_SR01_DISPLAY_OFF, VGA_SEQ_CLOCK);

	MGAG_ECRT_OUT8(mgag_io, MGAG_ECRT_IN8(mgag_io, ECRT1) & ~ECRT1_HSYNCOFF & ~ECRT1_VSYNCOFF, ECRT1);
#endif

	/* Double-checking alpha control initial value */
	MGAG_GC_OUT32(mgag_io, 0x00000001, ALPHACTRL);

	/* TODO: Should set an initial value for PITCH, MACCESS and DSTORG */

	if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
	  {
	    mgag_chipset_irq_enable(mgag_io);
	  }

	mgag->mode = mgag_mode;

#if 0
	KRN_TRACE(2, mgag_chipset_probe(mgag,mgag_io));
#endif
}

void mgag_chipset_mode_leave(mgag_chipset_t *mgag, mgag_chipset_io_t *mgag_io,
	mgag_chipset_mode_t *mgag_mode, kgi_image_mode_t *img, kgi_u_t images)
{
  mgag_chipset_wait_engine_idle(mgag_io);

  if (mgag->flags & MGAG_CF_IRQ_CLAIMED)
    {
      mgag_chipset_irq_block(mgag_io, NULL);
    }

  mgag->mode = NULL;
}
