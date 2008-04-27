/* ----------------------------------------------------------------------------
**	S3 ViRGE chipset meta-language implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	1999	Jon Taylor
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: ViRGE-meta.c,v $
**	Revision 1.3  2003/01/30 13:44:45  foske
**	Mayor cleanup, fix textmode, drop secondary card support, it wasn't going to work anyway.
**	
**	Revision 1.2  2002/04/03 11:29:53  foske
**	Test
**	
**	Revision 1.1.1.1  2000/09/21 12:32:26  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.3  2000/09/21 12:32:26  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.2  2000/08/04 11:39:08  seeger_s
**	- merged version posted by Jos to kgi-develop
**	- transferred maintenance to Steffen_Seeger
**	- status is reported to be 30% by Jos.
**	
**	Revision 1.1.1.1  2000/04/18 08:51:27  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#include <kgi/maintainers.h>
#define	DEBUG_LEVEL 	4	
#define	MAINTAINER	Steffen_Seeger
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.3 $"

#include <kgi/module.h>

#define	__S3_ViRGE
#include "chipset/S3/ViRGE.h"
#include "chipset/S3/ViRGE-meta.h"
#include "chipset/S3/ViRGE-bind.h"

#define	MUCH	1274231

#define VIRGE_MMIO_BASE		(virge_io->aperture.base_virt)+0x1000000
#define VIRGE_VGA_IO_BASE      	VGA_IO_BASE
#define VIRGE_VGA_MEM_BASE	(VIRGE_MMIO_BASE + 0x8000)
#define	VIRGE_VGA_DAC		(virge_io->flags & VIRGE_IF_VGA_DAC)

#ifdef DEBUG_LEVEL
/* Print verbose chipset configuration for debugging purposes */
static inline void virge_chipset_examine(virge_chipset_mode_t *virge_mode, kgi_u32_t flags)
{
	kgi_u32_t mclk, memsize;
	kgi_u32_t foo;
	
	KRN_DEBUG(1, "virge_chipset_examine()");
	
	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "PCI Registers:");
	KRN_DEBUG(1, "--------------");
//	KRN_DEBUG(1, "PCIBASE0 = %.8x", virge->pci.BaseAddr0);

	KRN_DEBUG(1, "");
	
	KRN_DEBUG(1, "CRT Registers:");
	KRN_DEBUG(1, "--------------");
	//KRN_DEBUG(1, "HTOTAL = %.2x", virge->crt.htotal);
	KRN_DEBUG(1, "");
#define PR(r) KRN_DEBUG(1,"##r = 0x%.8x", virge_mode->virge.##r)
	KRN_DEBUG(1, "STREAMS Registers:");
	KRN_DEBUG(1, "--------------");
	PR(PrimaryStreamControl);
	PR(ColorChromaKeyControl);
	PR(SecondaryStreamControl);
	PR(ChromaKeyUpperBound);
	PR(SecondaryStreamStretch);
	PR(BlendControl);
	PR(PrimaryStreamAddress0);
	PR(PrimaryStreamAddress1);
	PR(PrimaryStreamStride);
	PR(DoubleBufferLpbSupport);
	PR(SecondaryStreamAddress0);
	PR(SecondaryStreamAddress1);
	PR(SecondaryStreamStride);
	PR(OpaqueOverlayControl);
	PR(K1VerticalScaleFactor);
	PR(K2VerticalScaleFactor);
	PR(DDAVerticalAccumulator);
	PR(StreamsFifoControl);
	PR(PrimaryStreamStartCoordinates);
	PR(PrimaryStreamWindowSize);
	PR(SecondaryStreamStartCoordinates);
	PR(SecondaryStreamWindowSize);
}
#endif	/* #if (DEBUG_LEVEL > 1) */

#define KRN_DEBUG_TIMING(t, name) \
	KRN_DEBUG(4, "%s.width = %d", name, t.width); \
	KRN_DEBUG(4, "%s.blankstart = %d", name, t.blankstart); \
	KRN_DEBUG(4, "%s.syncstart = %d", name, t.syncstart); \
	KRN_DEBUG(4, "%s.syncend = %d", name, t.syncend); \
	KRN_DEBUG(4, "%s.blankend = %d", name, t.blankend); \
	KRN_DEBUG(4, "%s.total = %d", name, t.total); \
	KRN_DEBUG(4, "%s.polarity = %d", name, t.polarity);

static inline void virge_chipset_accel_sync(virge_chipset_io_t *virge_io)
{
	kgi_u32_t foo;
	
	KRN_DEBUG(2, "virge_chipset_accel_sync()");
	
	/* Wait for S3d Engine */
	if (CRT_IN8(virge_io, CRT_EXTMISCCONTROL1) & CR66_ENBL_ENH) {
	  for (foo=MUCH;--foo && !(MMIO_IN32(SUBSYSTEM_STATUS)& SSS_S3D_ENG);); 
	  if (!foo) KRN_DEBUG (1,"ViRGE: S3d Engine seems to be crashed.");
	}
	return;
}

static void virge_chipset_accel_shutdown (virge_chipset_io_t *virge_io) {
	kgi_u_t foo;

	if (!(CRT_IN8(virge_io, CRT_EXTMISCCONTROL1) & CR66_ENBL_ENH))
	  return; /* S3D Engine already disabled.*/
	virge_chipset_accel_sync (virge_io);
	/* Reset S3d Engine */
  	MMIO_OUT32(0x00008000, SUBSYSTEM_CONTROL);
  	MMIO_OUT32(0x00004000, SUBSYSTEM_CONTROL);
	/* Wait till S3d Engine is ready. */
	for (foo = MUCH; --foo && !(MMIO_IN32(SUBSYSTEM_CONTROL) & 0x02000);); 
	if (0 == foo) KRN_DEBUG (0,"ViRGE: S3D Engine reset failed.");
}
/* Accelerator implementation */

typedef struct
{
	kgi_accel_context_t kgi;
	kgi_aperture_t aperture;

	struct 
	{
		kgi_u32_t S3d_Reg[44];
		kgi_u32_t Color_Pattern_Reg[48];
	} state;

} virge_chipset_accel_context_t;

static void virge_chipset_accel_init(kgi_accel_t *accel, void *ctx)
{
	KRN_DEBUG(2, "virge_chipset_accel_init()");
}

static void virge_chipset_accel_done(kgi_accel_t *accel, void *ctx)
{
	KRN_DEBUG(2, "virge_chipset_accel_done()");
}

static inline void virge_chipset_accel_save(kgi_accel_t *accel)
{
	KRN_DEBUG(2, "virge_chipset_accel_save()");
}

static inline void virge_chipset_accel_restore(kgi_accel_t *accel)
{
	KRN_DEBUG(2, "virge_chipset_accel_restore()");
}

static void virge_chipset_accel_schedule(kgi_accel_t *accel)
{
}

static void virge_chipset_accel_exec(kgi_accel_t *accel, kgi_accel_buffer_t *buffer)
{
}

/* DisplayPowerManagement */

/* FIXME : Fix that annoying switch (santaclaus) */
static void virge_set_dpm_mode (virge_chipset_io_t *virge_io, 
			 virge_chipset_mode_t *virge_mode,
			 kgi_u32_t santaclaus) {
	const kgim_monitor_mode_t *crt_mode = virge_mode->kgim.crt;
	/* The switch statment uses conditional breaks. The idea:
	 * If the display can handle the requested dpm mode, it is set.
	 * Otherwise, the most power-saving mode below the requested that the 
	 * display CAN handle is set.
	 */
	switch (santaclaus) {
		case 0: /* Off */
		  if (KGIM_ST_SYNC_OFF & crt_mode->sync) {
		    CRT_OUT8 (virge_io, CR56_VSYNC_OFF|CR56_HSYNC_OFF, CRT_EXTSYNC1);
		    break;
		  }
		case 1: /* Suspend */
		  if (KGIM_ST_SYNC_SUSPEND & crt_mode->sync) {
		    CRT_OUT8 (virge_io, CR56_VSYNC_OFF, CRT_EXTSYNC1);
		    break;
		  }
		case 2: /* Standby */
		  if (KGIM_ST_SYNC_STANDBY & crt_mode->sync) {
		    CRT_OUT8 (virge_io, CR56_HSYNC_OFF, CRT_EXTSYNC1);
		    break;
		  }
		case 3:
		default: /* Normal */
		  CRT_OUT8 (virge_io, 0x00, CRT_EXTSYNC1);
		  break;
	}
}

/* Streams processor procedures */

static void virge_chipset_streams_disable (virge_chipset_io_t *virge_io)
{
  kgi_u32_t foo;
  /* Find VSYNC */
  for (foo=MUCH; --foo && !(MMIO_IN8(0x83DA) & 0x08); ); 
  SPR_OUT32(virge_io,0x0200,0x0000C000);
  CRT_OUT8 (virge_io, CRT_IN8 (virge_io, CRT_EXTMISCCONTROL2) & 
		      ~CR67_FULL_STREAMS, CRT_EXTMISCCONTROL2); 
}

/* Interrupt procedures */

kgi_error_t virge_chipset_irq_handler(virge_chipset_t *virge, virge_chipset_io_t *virge_io, irq_system_t *system)
{
	kgi_u32_t handled = 0;

	/* Get interrupt status and clear pending interrupts immediately */
	kgi_u32_t sss = MMIO_IN32 (SUBSYSTEM_STATUS);
	kgi_u32_t lif = MMIO_IN32 (LPB_INTR_FLAGS);

  	MMIO_OUT32(SSC_CLEAR_ALL_INTERRUPTS, SUBSYSTEM_CONTROL);
  	MMIO_OUT32(virge->misc.subsys_ctl, SUBSYSTEM_CONTROL);
  	CRT_OUT8(virge_io, CRT_IN8(virge_io, 0x11)&~0x30,0x11);  
  	if (virge->misc.subsys_ctl & SSC_VSY_ENB)
    	  CRT_OUT8(virge_io, CRT_IN8(virge_io, 0x11)|0x10,0x11);     

  	/* Now act on pending interrupts. */
   
	KRN_DEBUG(3, "Interrupt: SSS=0x%.8x",sss);
	if (sss & SSS_VSY_INT) 
	{
    	/* Vertical Sync Interrupt */
	  handled |= SSS_VSY_INT;
	  KRN_TRACE(0, virge->interrupt.VSY++);
	  KRN_DEBUG(1, "VSY interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
	}
	if (sss & SSS_S3D_DON) 
	{
    	/* 3D Engine done */
	  handled |= SSS_S3D_DON;
	  KRN_TRACE(0, virge->interrupt.S3D_DON++);
	  KRN_DEBUG(1, "S3D_DON interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (sss & SSS_FIFO_OVF) 
	{
    	/* Command FIFO Overflow */
	  handled |= SSS_FIFO_OVF;
	  KRN_TRACE(0, virge->interrupt.FIFO_OVF++);
	  KRN_DEBUG(1, "FIFO_OVF interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (sss & SSS_FIFO_EMP) 
	{
    	/* Command FIFO Empty */
	  handled |= SSS_FIFO_EMP;
	  KRN_TRACE(0, virge->interrupt.FIFO_EMP++);
	  KRN_DEBUG(1, "FIFO_EMP interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (sss & SSS_HD_DON) 
	{
    	/* Host DMA Done */
	  handled |= SSS_HD_DON;
	  KRN_TRACE(0, virge->interrupt.HD_DON++);
	  KRN_DEBUG(1, "HD_DON interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (sss & SSS_CD_DON) 
	{
    	/* Command DMA Done */
	  handled |= SSS_CD_DON;
	  KRN_TRACE(0, virge->interrupt.CD_DON++);
	  KRN_DEBUG(1, "CD_DON interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (sss & SSS_S3DF_FIF) 
	{
    	/* S3d FIFO Empty */
	  handled |= SSS_S3DF_FIF;
	  KRN_TRACE(0, virge->interrupt.S3DF_FIF++);
	  KRN_DEBUG(1, "S3DF_FIF interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (sss & SSS_LPB_INT) 
	{
    	/* LPB Interrupt */
	  handled |= SSS_LPB_INT;
	  KRN_TRACE(0, virge->interrupt.LPB_INT++);
	  KRN_DEBUG(1, "LPB_INT interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}
  	if (lif & LIF_SPS) 
	{
    	/* Serial Port Start */
	  KRN_TRACE(0, virge->interrupt.SPS++);
	  KRN_DEBUG(1, "SPS interrupt (pcidev %.8x)",
			VIRGE_PCIDEV(virge_io));
  	}

	if (sss & ~handled & 0x000000ff) 
	{
		KRN_TRACE(0, virge->interrupt.not_handled++);
		KRN_ERROR("virge: unhandled interrupt flag(s) %.8x (pcidev %.8x)", sss & ~handled, VIRGE_PCIDEV(virge_io));
	}
	if (!(sss & 0x000000ff)) 
	{
		KRN_TRACE(0, virge->interrupt.no_reason++);
		KRN_ERROR("virge: interrupt but no reason indicated.");
	}

	return KGI_EOK;
}

static void virge_chipset_interrupts_disable (virge_chipset_io_t *virge_io)
{
	/* Switch off ViRGE interrupt generator and vertical retrace */
	CRT_OUT8(virge_io, (CRT_IN8(virge_io, 0x11) & 0xDF) | 0x10, 0x11);
  	/* Disable all enhanced interrupts. */
  	MMIO_OUT32(0, SUBSYSTEM_CONTROL);	
}

static void virge_chipset_interrupts_enable (virge_chipset_io_t *virge_io, virge_chipset_t *virge)
{
	kgi_u8_t crt11;
	
	/* Clear vertical sync interrupt and enable it. */
  	if (virge->misc.subsys_ctl & SSC_VSY_ENB) {
   	  crt11 = CRT_IN8(virge_io,0x11);
  	  CRT_OUT8(virge_io, crt11 & ~0x30,0x11);  
  	  CRT_OUT8(virge_io, crt11 | 0x10,0x11);
  	}  
  
  	/* Clear all pending enhanced interrupts and enable them */
  	MMIO_OUT32(0x0000007F, SUBSYSTEM_CONTROL);
  	MMIO_OUT32(virge->misc.subsys_ctl, SUBSYSTEM_CONTROL);
}

extern kgi_u8_t virge_chipset_vga_crt_in8(virge_chipset_io_t *io, kgi_u_t reg);
extern kgi_u32_t virge_chipset_ctl_in32(virge_chipset_io_t *mem, kgi_u_t reg);

kgi_error_t virge_chipset_init(virge_chipset_t *virge, virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = VIRGE_PCIDEV(virge_io);
	kgi_u32_t	DCLKdiv, m,n,r;
	kgi_u8_t	cr58,i;
	float		f;

	KRN_DEBUG(2, "virge_chipset_init()");
	
	KRN_ASSERT(virge);
	KRN_ASSERT(virge_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "Initializing %s %s", virge->chipset.vendor, virge->chipset.model);

	/* Make sure all IO is done in IO space, not in MMIO space */
	virge_io->flags = 0;
	/* Save all registers */
	virge_chipset_save_state (&virge->initial_state, virge_io);
	
#define	PCICFG_SET_BASE(value, reg)		\
	pcicfg_out32(0xFFFFFFFF, reg);	\
	pcicfg_in32(reg);		\
	pcicfg_out32((value), reg)

	PCICFG_SET_BASE(virge_io->aperture.base_io,   pcidev + PCI_BASE_ADDRESS_0);
	
#undef	PCICFG_SET_BASE

	KRN_DEBUG(1, "PCI (re-)configuration done");

	/* When we arrive here. We don't know anything about the current state
	 * of the chipset. The chipset might already be the initial console
	 * driver, but it also can be asleep... It actually even might be the
	 * cause of that smoke coming out of your computer right now.
	 */

	if (PCI_COMMAND_MEMORY == 
	    (virge->pci.Command & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY))) {	
		/* If only COMMAND_MEMORY is on, there might be another driver 
		 * for this chipset, so we have to abort.
		 */
		KRN_NOTICE ("Sorry, your ViRGE is in a state that only should occur when another driver is   present. Bailing out to prevent problems."); 
		return -KGI_ERRNO(CHIPSET, FAILED);
	}
	
	if (virge->pci.Command & (PCI_COMMAND_IO | PCI_COMMAND_MEMORY)) 
	{
		/* The ViRGE has access to the PCI bus. COMMAND_IO is on, so
		 * it is the primary display adapter. 
		 */
		pcicfg_out32 (virge->pci.Command | PCI_COMMAND_MEMORY | 
				PCI_COMMAND_IO,	pcidev +  PCI_COMMAND); 
		/* Enable access to all registers */
		CRT_OUT8(virge_io, CRT_IN8(virge_io,0x11) & 0x7f, 0x11);
		/* Set the magic numbers */
		CRT_OUT8(virge_io, CR38_MAGIC, 0x38);
		CRT_OUT8(virge_io, CR39_MAGIC, 0x39);
		CRT_OUT8(virge_io, 0x22,       0x33);
		CRT_OUT8(virge_io, CRT_IN8 (virge_io, 0x35) & 0x0f, 0x35);
		/* 0x40 is wrong in the ViRGE books */
		CRT_OUT8(virge_io, CRT_IN8(virge_io, 0X40) & ~0x01, 0x40);
		SEQ_OUT8(virge_io, SR08_MAGIC, 0x08);
		
		KRN_DEBUG(2, "Chipset initialized, reading configuration");
		virge->flags |= VIRGE_CF_RESTORE_INITIAL;
		
		vga_text_chipset_init(&(virge->vga), &(virge_io->vga), options);
	} 
	else {
		KRN_NOTICE("Sorry, ViRGE secondary adapters will not work.");
		return -KGI_ERRNO(CHIPSET,NOSUP); 
	}

	/* Current state of the driver: All register access enabled.
	 * Command IO works, Framebuffer size is still unknown.
	 */
	
	KRN_DEBUG(2, "Chipset enabled");

	/* Detect RAM (Framebuffer) size */
  
	if (!IS_VIRGE_VX(virge) && !IS_VIRGE_GX2(virge) && !IS_VIRGE_MX(virge))
	{ 
    	  virge->chipset.memory = 1 MB;
    	  switch (CRT_IN8(virge_io, CRT_CONFIG1) & CR36_MEMSIZE_MASK) {
	    case CR36_4MB:
	      virge->chipset.memory *= 2; /* Fallthrough */
       	    case CR36_2MB:
       	      virge->chipset.memory *= 2; /* Fallthrough */
       	    case CR36_1MB:
	      KRN_NOTICE("ViRGE: Detected %d KB of video memory",
	         virge->chipset.memory >> 10);
	      break;
       	    default:
	      KRN_NOTICE("ViRGE: Unknown amount of memory. Assuming 1 MB");
	      break;
          } /* switch */
 	} else if IS_VIRGE_VX(virge) {
	  /* The VX is a weirdo. If I understand correctly, it has some 
	   * memory, that can't be used for display images. Forget this memory
	   * for now, someone pass me a VX manual please ?
	   */
          virge->chipset.memory = 2 MB*(1+((CRT_IN8(virge_io,CRT_CONFIG1) & 0x60) >> 5));
          KRN_NOTICE("ViRGE: VX: Detected %d KB of video memory",
	         virge->chipset.memory >> 10);
          switch (CRT_IN8 (virge_io,0x37) & 0x60) {
            case 1:
              virge->chipset.memory -= 2 MB;
	    case 2:
	      virge->chipset.memory -= 2 MB;
          } /* switch */
          KRN_NOTICE("ViRGE: VX: On-screen memory: %d", virge->chipset.memory >> 10);
	} else {
          /* Gx2 or Mx chipsets use an incompatible CONFIG1 register */
    	  virge->chipset.memory = 2 MB;
    	  switch ((CRT_IN8(virge_io, CRT_CONFIG1) & 0xC0) >> 6) {
            case 3:
              virge->chipset.memory <<= 1;
      	    case 1:
	      KRN_NOTICE("ViRGE: GX2 / MX: Detected %d KB of video memory",
	         virge->chipset.memory >> 10);
	      break;
            default:
	      KRN_NOTICE("ViRGE: GX2 / MX: Unknown memory size. Assuming 2 MB");
	      break;
	  } /* switch */
	} /* else */
	
	cr58= 	(CRT_IN8 (virge_io, 0x58) & 0x80)|
	      	(1 MB == virge->chipset.memory ? 0x01 :
		(2 MB == virge->chipset.memory ? 0x02 : 0x03)) |
		(IS_VIRGE_VX(virge) ? 0x50: 0x10);
	CRT_OUT8 (virge_io, cr58, 0x58);
	CRT_OUT8 (virge_io, 0x08, 0x53);
	
	/*
	** Enable MMIO access
	*/
	CRT_OUT8(virge_io, 0x10, 0x32);

	CRT_OUT8(virge_io, 0x1B, 0x37); // ????
	// MOve this CRT_OUT8(virge_io, 0x10, 0x55);
	CRT_OUT8(virge_io, 0x00, 0x56);
	CRT_OUT8(virge_io, 0x00, 0x65); /*XFree86 source says this is right*/

	/* Endianness support (FIXME) */
	CRT_OUT8(virge_io, 0x00, 0x54); 
	CRT_OUT8(virge_io, 0x00, 0x61); 
	
	io_out8 (0x01, VGA_IO_BASE + 0x003); 

	/* MMIO mode should be up and running now.*/
	virge_io->flags |= VIRGE_IF_MMIO_ENABLED;
	
	virge->misc.subsys_ctl=0;
#if DEBUG_LEVEL < 1
	if (virge->flags & VIRGE_CF_IRQ_CLAIMED) 
	{
	/* Enable interrupts */
	  /* Standard the next interrupts are enabled:
	   * Vertical Sync (to be the most-called interrupt routine ?)
	   * S3D Engine Done (To sync the accelleration engine)
	   * Fifo overflow (To detect errors in the driver)
	   */
	  virge->misc.subsys_ctl = SSC_VSY_ENB | SSC_3DD_ENB |
		  		   SSC_FIFO_ENB_OVF;  
	  MMIO_OUT32(SSC_CLEAR_ALL_INTERRUPTS, SUBSYSTEM_CONTROL);
  	  MMIO_OUT32(virge->misc.subsys_ctl, SUBSYSTEM_CONTROL);
  	  CRT_OUT8(virge_io, CRT_IN8(virge_io, 0x11)&~0x30,0x11);  
  	  if (virge->misc.subsys_ctl & SSC_VSY_ENB)
    	    CRT_OUT8(virge_io, CRT_IN8(virge_io, 0x11)|0x10,0x11);     
	  /* We don't have to clear the interrupt counters here, for 
	   * in the bind.c file, the complete virge structure is initated 
	   * at zeroes.
	   */
	}
#endif
	CRT_OUT8(virge_io, 0x20, 0x0A);
	SEQ_OUT8(virge_io, ESA_2MCLK | ESA_PIN50_RAS1, 0x0A);
	SEQ_OUT8(virge_io, SR14_NORMAL_OPERATION, 0x14);
	SEQ_OUT8(virge_io, 0, 0x15); /*FIXME:Check this*/

	CRT_OUT8(virge_io, 0x08, 0x31);
	
	/* Find out current MCLK settings */
	n = SEQ_IN8 (virge_io, 0x10);
	r = (n & 0x60) >> 5;
	n &= 0x1f;
	m = SEQ_IN8 (virge_io, 0x11) & 0x7f;
	f = ((1431818 * (m + 2)) / (n + 2) / (1 << r) + 50) / 1000000; 
	KRN_NOTICE ("ViRGE: MCLK running at %.3f MHz", f);
	return KGI_EOK;
}

void virge_chipset_done(virge_chipset_t *virge, virge_chipset_io_t *virge_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = VIRGE_PCIDEV(virge_io);

	KRN_DEBUG(2, "virge_chipset_done()");
	
	if (virge->flags & VIRGE_CF_IRQ_CLAIMED) 
	{
		virge_chipset_interrupts_disable (virge_io);
	}

	if (virge->flags & VIRGE_CF_RESTORE_INITIAL) {
		KRN_DEBUG(2, "Restoring initial chipset state");
		virge_chipset_restore_state (&virge->initial_state, virge_io);
		vga_text_chipset_done((&virge->vga), &(virge_io->vga), options);
	}

	KRN_DEBUG(1, "Restoring PCI registers.");
	pcicfg_out32(virge->pci.Command, pcidev + PCI_COMMAND);
	pcicfg_out32(virge->pci.LatTimer, pcidev + PCI_LATENCY_TIMER);
	pcicfg_out32(virge->pci.IntLine, pcidev + PCI_INTERRUPT_LINE);
	pcicfg_out32(virge->pci.BaseAddr0, pcidev + PCI_BASE_ADDRESS_0);
	pcicfg_out32(virge->pci.RomAddr, pcidev + PCI_ROM_ADDRESS);
}

/* Mode setup procedures */

static void virge_chipset_set_screen_base (kgi_u_t x, kgi_u_t y, 
					   virge_chipset_io_t *virge_io,
					   kgi_image_mode_t *img)
{
	kgi_u32_t a;	
	/* Check if the visible area crosses the border of the virtual
	 * area. If so, bail out.
	 */
	if ((img[0].virt.x < img[0].size.x + x) ||
	    (img[0].virt.y < img[0].size.y + y))
	{
		KRN_DEBUG(1,"Image out of virtual range.");
		return;
	}
	a = (img[0].virt.x * y + x) * kgim_attr_bits(img[0].out->bpda) >> 3;
	CRT_OUT8(virge_io, a & 0xff, 0x0D);
	CRT_OUT8(virge_io, (a >> 8) & 0xff, 0x0C);
	CRT_OUT8(virge_io, (a >> 16) & 0x0f, 0x69);
	/*FIXME: Streams modes. Problem here is that STREAMS needs base address
	 * quadword aligned, someone tell me how to deal with 24 bpp please.*/	
}

static void virge_adjust_timing(kgim_monitor_mode_t *mode, kgi_u_t div)
{

	#define ADJUST(X) mode->x.X -= (mode->x.X % div)

	ADJUST(width);
	ADJUST(blankstart);
	ADJUST(syncstart);
	ADJUST(syncend);
	ADJUST(blankend);

	if (mode->x.total % div) 
		mode->x.total += div - (mode->x.total % div);

	if (((mode->x.syncend / div) & 0x3F) ==
		((mode->x.syncstart / div) & 0x3F)) 
		mode->x.syncend -= div;

	if (((mode->x.blankend / div) & 0x7F) ==
		((mode->x.blankstart / div) & 0x7F)) 
		mode->x.blankend -= div;

	if ((mode->y.syncend & 0x0F) == (mode->y.syncstart & 0x0F)) 
		mode->y.syncend--;

	if ((mode->y.blankend & 0xFF) == (mode->y.blankstart & 0xFF)) 
		mode->y.blankend--;

	#undef ADJUST
}

kgi_error_t virge_chipset_mode_check(virge_chipset_t *virge, virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = virge_mode->kgim.crt;

	kgi_u_t shift, bpf, bpc, bpp, psc, width, lclk;
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t mul, div, bpd;
	kgi_u_t DCLKdiv=8;
	kgi_u_t AddrUnit = 8;
	
	KRN_DEBUG(2, "virge_chipset_mode_check()");
	
	KRN_DEBUG_TIMING(crt_mode->x, "adjust x");
	KRN_DEBUG_TIMING(crt_mode->y, "adjust y");
	/* for text16 support we fall back to VGA mode
	*/
	if (img[0].flags & KGI_IF_TEXT16) 
	{
		KRN_DEBUG(2,"falling back to vga_text_chipset_mode_check");
		return vga_text_chipset_mode_check(&virge->vga, &virge_io->vga, &virge_mode->vga, cmd, img, images);
	}
	
	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO) ? (bpc + bpf * img[0].frames * 2) : (bpc + bpf*img[0].frames);
	
	/* for unsupported colour depths, bail out. The ramdac driver will
	 * do a decent check, all we have to check here is if the requested
	 * depth is supported by the ViRGEs Enhanced Mode 
	 */
	
	switch (bpd) 
	{
	case  8:		
	case 16:	
	case 24:	
	case 32:
		break;

	default:	
		KRN_DEBUG(0, "%i bpd not supported", bpd);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	if (images != 1) {
		KRN_DEBUG(2, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL)) {
		KRN_DEBUG(2, "Image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/* Check if common attributes are supported */
	switch (img[0].cam) {
		case 0:
			KRN_DEBUG(2, "img[0].cam = 0");
			break;

		case KGI_AM_STENCIL | KGI_AM_Z:
			
			if ((1 != img[0].bpca[0]) || (15 != img[0].bpca[1])) 
			{
				KRN_DEBUG(2, "S%iZ%i local buffer not supported", img[0].bpca[0], img[0].bpca[1]);
				return -KGI_ERRNO(CHIPSET, INVAL);
			}
			break;

		case KGI_AM_Z:
			if (16 != img[0].bpca[1]) 
			{
				KRN_DEBUG(2, "Z%i local buffer not supported", img[0].bpca[0]);
				return -KGI_ERRNO(CHIPSET, INVAL);
			}
			
		default:
			KRN_DEBUG(2, "Common attributes %.8x not supported", img[0].cam);
			return -KGI_ERRNO(CHIPSET, INVAL);
	}

	shift = 0;
	
	lclk = (cmd == KGI_TC_PROPOSE) ? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;

	switch (cmd) {
	case KGI_TC_PROPOSE:
		KRN_DEBUG(3, "KGI_TC_PROPOSE");
		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

		dpm->rclk.mul = dpm->rclk.div = 1;
		
		/* If size.x or size.y are zero, default to 640x400 */
		if ((0 == img[0].size.x) || (0 == img[0].size.y)) 
		{
			KRN_DEBUG(2, "Defaulting to 640x400");
			img[0].size.x = 640;
			img[0].size.y = 400;
		}

		/*	if virt.x and virt.y are zero, default to size;
		**	if either virt.x xor virt.y is zero, maximize the other
		*/
		if ((0 == img[0].virt.x) && (0 == img[0].virt.y)) 
		{
			KRN_DEBUG(2, "Defaulting to size,x, size.y");
			
			img[0].virt.x = img[0].size.x;
			img[0].virt.y = img[0].size.y;
		}

		if (0 == img[0].virt.x) 
		{
			KRN_DEBUG(2, "virt.x = 0");
			
			img[0].virt.x = (8 * virge->chipset.memory) / (img[0].virt.y * bpp);

			if (img[0].virt.x > virge->chipset.maxdots.x) 
			{
				KRN_DEBUG(2, "virt.x > maxdots.x");
				img[0].virt.x = virge->chipset.maxdots.x;
			}
		}
		
		if (img[0].virt.x % DCLKdiv) 
			img[0].virt.x += DCLKdiv - img[0].virt.x % DCLKdiv;
		if (img[0].size.x % DCLKdiv) 
			img[0].size.x += DCLKdiv - img[0].size.x % DCLKdiv;
		
		if (0 == img[0].virt.y) 
		{
			KRN_DEBUG(2, "virt.y = 0");
			img[0].virt.y = (8 * virge->chipset.memory) / (img[0].virt.x * bpp);
		}

		/* Are we beyond the limits of the H/W? */
		if ((img[0].size.x >= virge->chipset.maxdots.x) || 
		    (img[0].virt.x >= virge->chipset.maxdots.x)) 
		{
			KRN_DEBUG(2, "%i (%i) horizontal pixels are too many", 
				  img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= virge->chipset.maxdots.y) || 
		    (img[0].virt.y >= virge->chipset.maxdots.y)) 
		{

			KRN_DEBUG(2, "%i (%i) vertical pixels are too many", 
				  img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 
		    (8 * virge->chipset.memory)) 
		{
			KRN_DEBUG(2, "not enough memory for (%ipf*%if + %ipc)@%ix%i", bpf, img[0].frames, bpc, img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

		/* Set CRT visible fields */
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		/* If the requested resolution is very low, we fall back to 
		 * pixel doubling. 
		 * The values 600 and 350 are no documented values, just some
		 * guesses. Many monitors go crazy if you use much lower values
	 	 */
	
		if (img[0].size.x < 600) {
			dpm->dots.x *= 2;
			/* If doubling doesn't help, bail out */
			if (dpm->dots.x < 600) return -KGI_ERRNO(CHIPSET,INVAL);
		}
		
		if (img[0].size.y < 350) {
			dpm->dots.y *= 2;
			/* If doubling doesn't help, bail out */
			if (dpm->dots.y < 350) return -KGI_ERRNO(CHIPSET,INVAL);
			
		}
		
		KRN_DEBUG(2, "dots.x = %d, dots.y = %d", dpm->dots.x, dpm->dots.y);
		
		return KGI_EOK;

	case KGI_TC_LOWER:
		KRN_DEBUG(3, "KGI_TC_LOWER");
		if (dpm->dclk < virge->chipset.dclk.min) 
		{
			KRN_DEBUG(1, "DCLK = %i Hz is too low", dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if (lclk > 50000000) 
		{
			dpm->dclk = 50000000 * dpm->lclk.div / dpm->lclk.mul;
		}
		return KGI_EOK;
	
	case KGI_TC_RAISE:
		KRN_DEBUG(3, "KGI_TC_RAISE");
		if (lclk > 50000000) 
		{
			KRN_DEBUG(1, "LCLK = %i Hz is too high", lclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}
		return KGI_EOK;

	case KGI_TC_CHECK:
		KRN_DEBUG(3, "KGI_TC_CHECK");
		if ((img[0].size.x >= virge->chipset.maxdots.x) ||
		    (img[0].size.y >= virge->chipset.maxdots.y) || 
		    (img[0].virt.x >= virge->chipset.maxdots.x) ||
		    ((img[0].virt.y * img[0].virt.x * bpp) >
		     (8 * virge->chipset.memory))) 
		{
			KRN_DEBUG(1, "Resolution too high: %ix%i (%ix%i)", img[0].size.x, img[0].size.y, img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if (lclk > 50000000) 
		{
			KRN_DEBUG(1, "LCLK = %i Hz is too high\n", lclk);
			return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		}

		if (img[0].flags & KGI_IF_STEREO) 
		{
			KRN_DEBUG(1, "stereo modes not supported on virge");
			return -KGI_ERRNO(CHIPSET, NOSUP);
		}
		
		virge_adjust_timing (crt_mode, DCLKdiv);

		break;

	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}


	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/

	KRN_ASSERT (dpm->lclk.mul < dpm->lclk.div);
	DCLKdiv = 8; /*FIXME*/
	bpd = kgim_attr_bits(dpm->bpda);

	switch (DCLKdiv)
	{
		case 8: 
			virge_mode->virge.ClockingMode = 1;
			break;
		case 9: 
			virge_mode->virge.ClockingMode = 0;
			break;
		default:
			KRN_ERROR("Unsupported dclk / cclk ratio.");
			return -KGI_ERRNO(CHIPSET, NOSUP);
	}
	
	/* Based on CCLKs (DCLK / 8 (Or 9, depends on SR1 bit 0)) */
	
	//virge_adjust_timing (crt_mode, DCLKdiv);
	
	virge_mode->virge.HTotal = (crt_mode->x.total / DCLKdiv) - 5;
	virge_mode->virge.HdEnd = (crt_mode->x.width/ DCLKdiv) - 1;
	virge_mode->virge.HbStart = crt_mode->x.blankstart / DCLKdiv;
	virge_mode->virge.HbEnd = (crt_mode->x.blankend - crt_mode->x.blankstart) / DCLKdiv; 
			   	   
	virge_mode->virge.HsStart = crt_mode->x.syncstart / DCLKdiv;
	virge_mode->virge.HsEnd = (crt_mode->x.syncend - crt_mode->x.syncstart) / DCLKdiv;
				  
	virge_mode->virge.Offset = bpd * img[0].virt.x / (8 * AddrUnit);
	
	/* Based on lines */
	virge_mode->virge.VTotal = crt_mode->y.total - 2;
	virge_mode->virge.VdEnd = crt_mode->y.width - 1;
	virge_mode->virge.VbStart = crt_mode->y.blankstart - 1;
	virge_mode->virge.VbEnd = crt_mode->y.blankend;
	virge_mode->virge.VsStart = crt_mode->y.syncstart;
	virge_mode->virge.VsEnd = crt_mode->y.syncend;

	/* Sync polarity */
	virge_mode->virge.Misc = (crt_mode->x.polarity > 0 ? 0 : 0x40) |
				 (crt_mode->y.polarity > 0 ? 0 : 0x80) |
				 0x2F;
	
	/* Interlace modes */
	/* FIXME */
	virge_mode->virge.InterlaceRetraceStart = crt_mode->y.total/2; 
	virge_mode->virge.ModeControl = (0/*Interlaced?*/?CR42_INTERLACED:0);
	
	virge_mode->virge.ExtendedSequencerB = SR0B_NO_FEAT_INPUT;
	virge_mode->virge.DacClkSynControl = SR18_NORMAL_OPERATION;

	virge_mode->virge.MemoryConfiguration = 0x4D;
	if ((bpd > 4) &&  
	   !((img[0].flags & KGI_IF_TEXT16) || (img[0].fam & KGI_AM_TEXTURE_INDEX)))
	{
		virge_mode->virge.Miscellaneous1 = CR3A_NO4BPPMODE;
		if (IS_VIRGE_VX(virge)) 
		  virge_mode->virge.ExtendedMiscellaneousControl1 = 0x90;
		else
		  virge_mode->virge.ExtendedMiscellaneousControl1 = 0x89;
		virge_mode->virge.MemoryModeControl = 0x02; 
		virge_mode->virge.UnderlineLocation = 0x1f; 
		virge_mode->virge.MemoryConfiguration |= 0x0C;
		virge_mode->virge.EnableWritePlane = 0x0F;
		virge_mode->virge.HorizontalPixelPanning = 0x00;
	} 
	else
	{
		virge_mode->virge.Miscellaneous1 = CR3A_HISPEEDTXT;
		virge_mode->virge.ExtendedMiscellaneousControl1 = 0x88;
		virge_mode->virge.ExtendedMiscellaneousControl2 = 0x00;
		virge_mode->virge.MemoryModeControl = 0x02;
		virge_mode->virge.MemoryConfiguration |= 0x40;
		virge_mode->virge.EnableWritePlane = 0x03;
	}
		
	virge_mode->virge.MaximumScanLine = 0;
	virge_mode->virge.PresetRowScan   = 0;
	
	virge_mode->virge.CRTCModeControl = 	VGA_CR17_CGA_BANKING |
					    	VGA_CR17_HGC_BANKING |
						VGA_CR17_ENABLE_SYNC |
						VGA_CR17_WORDMODE    |
						VGA_CR17_CGA_ADDR_WRAP;
	
	virge_mode->virge.ExtendedMode = (16 == bpp ? 0x80: 0x00);
	virge_mode->virge.ExtendedRAMDACControl = 0;
	virge_mode->virge.LineCompare = -1;
	switch (bpd) 
	{
	case  8:
		if (KGI_AM_I == img[0].out->dam)
		{
			if (0/*DCLK > DCLKMax*/) 
			{
			  virge_mode->virge.ExtendedMiscellaneousControl2 |= 
				  CR67_DAC_MODE8;
			  virge_mode->virge.DacClkSynControl  |= SR18_CLKx2;
			  virge_mode->virge.ClkSynControl2 |= SR15_DCLKd2;
			} else 
			{  
		  	  virge_mode->virge.ExtendedMiscellaneousControl2 |= 
				  CR67_DAC_MODE0;// | CR67_FULL_STREAMS;
			}
			psc = STREAMS_PSC_RGB8;
		}
		else
		{
			/* Uh ? */
			KRN_ERROR("Illegal dot attribute mask for 8 bpd!");
			return -KGI_ERRNO(CHIPSET,NOSUP);
		}	
		break;	
	case 16:
		if ((KGI_AM_RGBA == img[0].out->dam) ||
		    (KGI_AM_RGBX == img[0].out->dam))
		{
			/* 15 bit colourdepth */
			virge_mode->virge.ExtendedMiscellaneousControl2 |= 
				CR67_DAC_MODE9/* | CR67_FULL_STREAMS*/;
		  	psc = STREAMS_PSC_KRGB16;
		}
		else if (KGI_AM_RGB == img[0].out->dam)
		{
			/* 16 bit colourdepth */
			virge_mode->virge.ExtendedMiscellaneousControl2 |= 
				CR67_DAC_MODEA/* | CR67_FULL_STREAMS*/;
			psc = STREAMS_PSC_RGB16;
		}
		else
		{
			/* Uh ? */
			KRN_ERROR("Illegal dot attribute mask for 16 bpd!");
			return -KGI_ERRNO(CHIPSET,NOSUP);
		}
		break;
	case 24:	
		if (KGI_AM_RGB == img[0].out->dam)
		{
			virge_mode->virge.ExtendedMiscellaneousControl2 |= 
				CR67_DAC_MODED | CR67_FULL_STREAMS;
			psc = STREAMS_PSC_RGB24;
		}
		else
		{
			/* Uh ? */
			KRN_ERROR("Illegal dot attribute mask for 24 bpd!");
			return -KGI_ERRNO(CHIPSET,NOSUP);
		}
		break;
	case 32:
		if ((KGI_AM_RGBA == img[0].out->dam) ||
		    (KGI_AM_RGBX == img[0].out->dam))
		{
			virge_mode->virge.ExtendedMiscellaneousControl2 |= 
				CR67_DAC_MODED | CR67_FULL_STREAMS;
			psc = STREAMS_PSC_XRGB32;
		}
		else
		{
			/* Uh ? */
			KRN_ERROR("Illegal dot attribute mask for 32 bpd!");
			return -KGI_ERRNO(CHIPSET,NOSUP);
		}			
		break;
	default:	
		KRN_ERROR("Driver should have bailed out already.");
		return -KGI_ERRNO(CHIPSET, FAILED);
	}
	virge_mode->virge.PrimaryStreamControl 	  = psc & 0x77000000;
	virge_mode->virge.ColorChromaKeyControl	  = 0;
	if (2*img[0].size.x == crt_mode->x.width)
	{
		virge_mode->virge.PrimaryStreamControl   |= 0x20000000;
		virge_mode->virge.SecondaryStreamControl  = 0x03000fff;
	}
	else
		virge_mode->virge.SecondaryStreamControl  = 0x03000000;
//					(img[0].size.x -1)&0xfff; 
	virge_mode->virge.ChromaKeyUpperBound	  = 0;
	virge_mode->virge.SecondaryStreamStretch  = 0;
	virge_mode->virge.BlendControl		  = 0x01000000;
	virge_mode->virge.PrimaryStreamAddress0	  = 0;
	virge_mode->virge.PrimaryStreamAddress1	  = 0;
	virge_mode->virge.PrimaryStreamStride     = img[0].virt.x * bpd / 8;
	virge_mode->virge.DoubleBufferLpbSupport  = 0;  
	virge_mode->virge.SecondaryStreamAddress0 = 0;
	virge_mode->virge.SecondaryStreamAddress1 = 0;
	virge_mode->virge.SecondaryStreamStride   = 0x00000001;
	virge_mode->virge.OpaqueOverlayControl    = 0xC0000000;
	virge_mode->virge.K1VerticalScaleFactor   = 0;//img[0].size.y -1;
	virge_mode->virge.K2VerticalScaleFactor   = 0;//(img[0].size.y-crt_mode->y.width)&0x7ff ;
	virge_mode->virge.DDAVerticalAccumulator  = 0;//crt_mode->y.width - 1;
	virge_mode->virge.StreamsFifoControl      = 0x00000000; 
	virge_mode->virge.PrimaryStreamStartCoordinates=0x00010001;
	virge_mode->virge.PrimaryStreamWindowSize = img[0].size.y + 
						    ((img[0].size.x - 1) << 16);
	virge_mode->virge.SecondaryStreamStartCoordinates=0x07ff07ff;
	virge_mode->virge.SecondaryStreamWindowSize=0x00010001;
	virge_mode->virge.FifoControl = 0x00010400;
	virge_mode->virge.MiuControl  = 0x0200;
	virge_mode->virge.StreamsTimeout = 0x00001808;
	virge_mode->virge.MiscTimeout = 0x08080810;
	/* End of STREAMS setup */
	
	virge_mode->virge.StartFifoFetch = crt_mode->x.total / DCLKdiv -10;
	virge_mode->virge.BackwardCompatability3 = 0x10;	
	
	/*FIXME*/
	//virge_chipset_accel_init ();
	
	/* Initialize exported resources */
	r = &virge_mode->virge.aperture1;
	r->meta = virge;
	r->meta_io = virge_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name = "virge framebuffer";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = virge->chipset.memory;
	r->win.bus  = virge_io->aperture.base_bus;
	r->win.phys = virge_io->aperture.base_phys;
	r->win.virt = virge_io->aperture.base_virt;
	
	KRN_TRACE(2, virge_chipset_examine(virge_mode, 1));

	return KGI_EOK;
}

kgi_resource_t *virge_chipset_mode_resource(virge_chipset_t *virge, virge_chipset_mode_t *virge_mode, kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	KRN_DEBUG(2, "virge_chipset_mode_resource()");
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) 
		return vga_text_chipset_mode_resource(&virge->vga, &virge_mode->vga, img, images, index);

	switch (index) {
		case 0:	return (kgi_resource_t *) &virge_mode->virge.aperture1;
		default: return NULL;
	}
}

kgi_resource_t *virge_chipset_image_resource(virge_chipset_t *virge,
	virge_chipset_mode_t *virge_mode, kgi_image_mode_t *img,
	kgi_u_t image, kgi_u_t index)
{
  KRN_ASSERT(image == 0);

  /* Handles (VGA) text mode specific issues first */
  if (img[0].fam & KGI_AM_TEXTURE_INDEX) {
    /* We fall back to the VGA-text driver */
    return vga_text_chipset_image_resource(&(virge->vga),
			&(virge_mode->vga), img, image, index);
  } 

  /* Then normal graphic image resources */

  return NULL;
}

void virge_chipset_mode_prepare(virge_chipset_t *virge, virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "virge_chipset_mode_prepare()");
	
	virge_chipset_accel_sync(virge_io);
	virge_chipset_streams_disable (virge_io);
	virge_chipset_interrupts_disable (virge_io);
	/* Switch off display */
	SEQ_OUT8(virge_io, SEQ_IN8(virge_io,0x01) | VGA_SR01_DISPLAY_OFF, 0x01);
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) {
		virge_io->flags |= VIRGE_IF_VGA_DAC;
		vga_text_chipset_mode_prepare(&virge->vga, &virge_io->vga,
					 &virge_mode->vga, img, images);

		KRN_DEBUG(2, "Prepared for VGA-mode");
		return;
	}

	virge_io->flags &= ~VIRGE_IF_VGA_DAC;
	KRN_DEBUG(2, "prepared for virge mode");
}

void virge_chipset_mode_enter(virge_chipset_t *virge, virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u8_t	seq01, crt11, foo;
	
	KRN_DEBUG(2, "virge_chipset_mode_enter()");
	
	if ((img[0].flags & KGI_IF_TEXT16) || (img[0].fam & KGI_AM_TEXTURE_INDEX)) 
	{
		vga_text_chipset_mode_enter(&virge->vga, &virge_io->vga, 
				       &virge_mode->vga, img, images);
		/* Fall through to turn off all enhanced stuff */
		//KRN_DEBUG(0,"SEQ0x4: %x",SEQ_IN8(virge_io, 0x4));
		CRT_OUT8 (virge_io, 0x04, 0x31);
		CRT_OUT8 (virge_io, CRT_IN8(virge_io,0x58) & 0xef, 0x58);
		CRT_OUT8 (virge_io, CRT_IN8(virge_io,0x66) & 0xfe, 0x66);

		return;
	}
	KRN_DEBUG(2, "After vga_text_chipset_mode_enter()");
	
	/* Turn on the Streams Processor if necessary 
	 * Assuming screen is disabled here !!!
	 */
	CRT_OUT8 (virge_io, virge_mode->virge.ExtendedMiscellaneousControl2, 0x67);
	if (0/*FIXMEvirge_mode->enhanced*/)
	{
	/* Setup the streams registers. 
	 * Warning: Some ViRGEs are known to crash when writing to
	 * streams processor while it is disabled
	 */
#define STREAMS_OUT(r) KRN_DEBUG (3,"Streams 0x%.4x <- 0x%.8x",STREAMS_##r,virge_mode->virge.r); MMIO_OUT32(virge_mode->virge.r,STREAMS_##r)
	if ((virge_mode->virge.ExtendedMiscellaneousControl2 & CR67_FULL_STREAMS) == CR67_FULL_STREAMS)
	{
	  KRN_DEBUG(1,"---STREAMS PROCESSOR REGISTERS---");
	  STREAMS_OUT(PrimaryStreamControl);
	  STREAMS_OUT(ColorChromaKeyControl);
	  STREAMS_OUT(SecondaryStreamControl);
	  STREAMS_OUT(ChromaKeyUpperBound);
	  STREAMS_OUT(SecondaryStreamStretch);
	  STREAMS_OUT(BlendControl);
	  STREAMS_OUT(PrimaryStreamAddress0);
	  STREAMS_OUT(PrimaryStreamAddress1);
	  STREAMS_OUT(PrimaryStreamStride);
	  STREAMS_OUT(DoubleBufferLpbSupport);
	  STREAMS_OUT(SecondaryStreamAddress0);
	  STREAMS_OUT(SecondaryStreamAddress1);
	  STREAMS_OUT(SecondaryStreamStride);
	  STREAMS_OUT(OpaqueOverlayControl);
	  STREAMS_OUT(K1VerticalScaleFactor);
	  STREAMS_OUT(K2VerticalScaleFactor);
	  STREAMS_OUT(DDAVerticalAccumulator);
	  //STREAMS_OUT(StreamsFifoControl);
	  STREAMS_OUT(PrimaryStreamStartCoordinates);
	  STREAMS_OUT(PrimaryStreamWindowSize);
	  STREAMS_OUT(SecondaryStreamStartCoordinates);
	  STREAMS_OUT(SecondaryStreamWindowSize);
	  STREAMS_OUT(FifoControl);
	  STREAMS_OUT(MiuControl);
	  STREAMS_OUT(StreamsTimeout);
	  STREAMS_OUT(MiscTimeout);
	}
	}
	if (0/*FIXMEvirge_mode->enhanced*/) {
	  /* Setup Misc and FCtrl */
	  virge_io->vga.MiscOut8(virge_io, virge_mode->virge.Misc);
	  /* Setup SEQ registers */
	  /* SEQ 0x00 : Obsolete */
	  SEQ_OUT8(virge_io, virge_mode->virge.ClockingMode | 0x20 , 0x01);
	  SEQ_OUT8(virge_io, virge_mode->virge.EnableWritePlane	 , 0x02);
	  /*SEQ 0x02 - 0x03 : Obsolete for 8bpp + modes */
	  SEQ_OUT8(virge_io, virge_mode->virge.MemoryModeControl   , 0x04);
	}
	/* SEQ 0x05 - 0x07 : Undocumented
	 * SEQ 0x08 - 0x0A : Handled by driver setup
	 */
	SEQ_OUT8(virge_io, virge_mode->virge.ExtendedSequencerB  , 0x0B);
	/* SEQ 0x0C : Undocumented */
	SEQ_OUT8(virge_io, virge_mode->virge.ExtendedSequencerD  , 0x0D);
	/* SEQ 0x0E - 0x0F : Undocumented
	 * SEQ 0x10 - 0x11 : Don't know what to do with them
	 * SEQ 0x12 - 0x13 : Handled by clock driver
	 * SEQ 0x14 : Handled by driver setup
	 * SEQ 0x15 : Handled by driver setup and clock driver
	 * SEQ 0x16 - 0x17 : Feature not implemented in this driver
	 * SEQ 0x18 : Handled by clock driver (???)
	 * SEQ 0x19 - 0x1B : Undocumented
	 * SEQ 0x1C : For Future Implementation
	 */

	/* Setup CRT registers */
	if (0/*virge_mode->enhanced*/) {
		CRT_OUT8(virge_io, virge_mode->virge.HTotal  & 0xff, 	0x00);
		CRT_OUT8(virge_io, virge_mode->virge.HdEnd   & 0xff, 	0x01);
		CRT_OUT8(virge_io, virge_mode->virge.HbStart & 0xff, 	0x02);
		/* FIXME */
		CRT_OUT8(virge_io, (virge_mode->virge.HbEnd & 0x1f)| 0x80,0x03);
		CRT_OUT8(virge_io, virge_mode->virge.HsStart & 0xff, 	0x04);
		CRT_OUT8(virge_io, (virge_mode->virge.HsEnd  & 0x1f)|
		     		((virge_mode->virge.HbEnd & 0x20)<<2) , 0x05);
		
		CRT_OUT8(virge_io, virge_mode->virge.VTotal  & 0xff, 	0x06);
		foo =   ((virge_mode->virge.VTotal 	& 0x0100) >> 8) |
			((virge_mode->virge.VdEnd  	& 0x0100) >> 7) |
			((virge_mode->virge.VsStart	& 0x0100) >> 6) |
			((virge_mode->virge.VbStart	& 0x0100) >> 5) |
			((virge_mode->virge.LineCompare	& 0x0100) >> 4) |
			((virge_mode->virge.VTotal  	& 0x0200) >> 4) |
			((virge_mode->virge.VdEnd  	& 0x0200) >> 3) |
			((virge_mode->virge.VsStart  	& 0x0200) >> 2);
		CRT_OUT8(virge_io, foo,				 	0x07);
		CRT_OUT8(virge_io, virge_mode->virge.PresetRowScan, 	0x08);
		foo =   (virge_mode->virge.MaximumScanLine & 0x1f)	|
			((virge_mode->virge.VbStart	& 0x0200) >> 4) |
			((virge_mode->virge.LineCompare & 0x0200) >> 3);
		CRT_OUT8(virge_io, foo, 				0x09);
		/* 0X0A, 0X0B : Handled by VGA driver if necessary */
		/* 0x0C, 0X0D: Set by SetOrigin */
		CRT_OUT8 (virge_io, 0x00, 0x0C); // TEMP
		CRT_OUT8 (virge_io, 0x00, 0x0D); // TEMP
		CRT_OUT8 (virge_io, 0x00, 0x69); // TEMP !!!
		/* 0X0E, 0X0F: Set by textcursor handling procedures */
		CRT_OUT8(virge_io, virge_mode->virge.VsStart & 0xff, 	0x10);
		/* CRT 0x11: High nibble is set by interrupt handlers 
		* What about the !3/5 bit?? 
		*/
		CRT_OUT8(virge_io, virge_mode->virge.VsEnd   & 0x0f, 	0x11);
		CRT_OUT8(virge_io, virge_mode->virge.VdEnd   & 0xff, 	0x12);
		CRT_OUT8(virge_io, virge_mode->virge.Offset  & 0xff,	0x13);
		CRT_OUT8(virge_io, virge_mode->virge.UnderlineLocation, 0x14);
		CRT_OUT8(virge_io, virge_mode->virge.VbStart & 0xff,	0x15);
		CRT_OUT8(virge_io, virge_mode->virge.VbEnd   & 0xff,	0x16);
		CRT_OUT8(virge_io, virge_mode->virge.CRTCModeControl,	0x17);
		CRT_OUT8(virge_io, virge_mode->virge.LineCompare & 0xff,0x18);
	}
	/* CRT 0x19 - 0x21 + 0x23: Undocumented. 
	 * CRT 0x22 & 0x24: Read only.
	 * CRT 0x25 - 0x2E: Undocumented.
	 * CRT 0x2F - 0x30: Read only
	 */
	//CRT_OUT8(virge_io, virge_mode->virge.MemoryConfiguration,0x31);
	///CRT_OUT8(virge_io, virge_mode->virge.BackwardCompatability1,0x32);
	//CRT_OUT8(virge_io, virge_mode->virge.BackwardCompatability2,0x33);
	CRT_OUT8(virge_io, virge_mode->virge.BackwardCompatability3, 0x34);
	/* CRT 0x35 - 0x39: Don't touch, handled at driver setup */
	CRT_OUT8(virge_io, virge_mode->virge.Miscellaneous1,	0x3A);
	CRT_OUT8(virge_io, virge_mode->virge.StartFifoFetch,	0x3B);
	CRT_OUT8(virge_io, virge_mode->virge.InterlaceRetraceStart,0x3C);
	/* CRT 0x3D - 0x3F: Undocumented
	 * CRT 0x40 : Handled at driver setup	
	 * CRT 0x41 : Obsolete
	 */
	CRT_OUT8(virge_io, virge_mode->virge.ModeControl,	0x42);
	CRT_OUT8(virge_io, virge_mode->virge.ExtendedMode,	0x43);
	/* CRT 0x44 : Undocumented 
	 * CRT 0x45 - 0x4F : Handled by hardware cursor routines
	 * CRT 0x50 : Undocumented, ViRGE VX Fix follows
	 */
	/* CRT 0x51 : Bit 0-1 Handled by SetOrigin
	 * 	      Bit 2-3 Obsolete
	 */
	CRT_OUT8(virge_io, (virge_mode->virge.Offset >> 4) & 0x30, 0x51);
	/* CRT 0x52 : Obsolete 
	 * CRT 0x53 - /0x54 : Handled at driver setup */
	CRT_OUT8(virge_io, virge_mode->virge.ExtendedRAMDACControl, 0x55);	
	/* CRT 0x56 : Handled by VESA DPM procedures 
	 * CRT 0x57 : Undocumented
	 * CRT 0x58 - 0x5A: Handled by driver setup.
	 * CRT 0x5B : Undocumented
	 * CRT 0x5C : General Out Port, must be handled some day some how
	 */
	foo =	((virge_mode->virge.HTotal & 0x0100) >> 8) | 
		((virge_mode->virge.HdEnd  & 0x0100) >> 7) | 
		((virge_mode->virge.HbStart& 0x0100) >> 6) | 
		((virge_mode->virge.HbEnd  & 0x0040) >> 3) | 
		((virge_mode->virge.HsStart& 0x0100) >> 4) | 
		((virge_mode->virge.HsEnd  & 0x0020)) 	   | 
		((virge_mode->virge.StartFifoFetch & 0x0100) >> 2);
	CRT_OUT8(virge_io, foo,					0x5D);	
	foo =	((virge_mode->virge.VTotal & 0x0400) >>10) | 
		((virge_mode->virge.VdEnd  & 0x0400) >> 9) | 
		((virge_mode->virge.VbStart& 0x0400) >> 8) | 
		((virge_mode->virge.VsEnd  & 0x0400) >> 6) | 
		((virge_mode->virge.LineCompare & 0x0400) >> 4); 
	CRT_OUT8(virge_io, foo,					0x5E);
	CRT_OUT8(virge_io, 0xff, 0x60);
	/* CRT 0x5F - 0x60 : Undocumented
	 * CRT 0x61 : Handled by driver setup
	 * CRT 0x62 : Undocumented
	 */
	if (!IS_VIRGE_VX(virge)) {
	  CRT_OUT8(virge_io, 0x00, 0x63);
	}
	/*
	 * CRT 0x64-0x65 : Handled by setup ???
	 */
	CRT_OUT8(virge_io, virge_mode->virge.ExtendedMiscellaneousControl1, 0x66);	
	//CRT_OUT8(virge_io, virge_mode->virge.ExtendedMiscellaneousControl2, 0x67);	
	/* CRT 0x68 : Handled by driver setup
	 * CRT 0x69 - 0x6E : Obsolete
	 * CRT 0x6F : Handled by VESA DDC procedures
	 */
 		
	/* Setup GRC registers */
	if (0/*FIXMEvirge_mode->enhanced*/) {
		GRC_OUT8 (virge_io, 0x00, 0x00); //NEW
		GRC_OUT8 (virge_io, 0x00, 0x01);
		GRC_OUT8 (virge_io, 0x00, 0x02); //NEW 
		GRC_OUT8 (virge_io, 0x00, 0x03);
		GRC_OUT8 (virge_io, 0x00, 0x04);
		GRC_OUT8 (virge_io, 0x00, 0x05); // 0x40 ???
		GRC_OUT8 (virge_io, 0x05, 0x06);
		GRC_OUT8 (virge_io, 0x00, 0x07); //NEW
		GRC_OUT8 (virge_io, 0xFF, 0x08);
	}
	/* Setup ATC registers */
	if (0/*virge_mode->enhanced*/) {
		ATC_OUT8 (virge_io, 0x05, 0x10); // CHANGED
		ATC_OUT8 (virge_io, 0xff, 0x11); // NEW
		ATC_OUT8 (virge_io, 0x0f, 0x12); // NEW
		ATC_OUT8 (virge_io, virge_mode->virge.HorizontalPixelPanning, 	0x13); 
		ATC_OUT8 (virge_io, 0x00, 0x14); // NEW
	}
	//SEQ_OUT8 (virge_io, 0x0F, 0x02);
	//CRT_OUT8 (virge_io, 0xFF, 0x60);
	if (IS_VIRGE_DXGX(virge) || IS_VIRGE_GX2(virge)) {
	  CRT_OUT8 (virge_io, 0x80, 0x86); /* Disable DAC power savings */
	  CRT_OUT8 (virge_io, 0x00, 0x90); /* Disable Stream display fetch length control */
	}
	if IS_VIRGE_VX(virge) {
	  /* The famous unknown register. XFree does this, and that driver
	   * seems to work...
	   */
	  CRT_OUT8 (virge_io, 0x00, 0x50);
	  /* ExtendedMiscellaneousControl has moved from 0x66 to 0x63.
	   */
	  CRT_OUT8 (virge_io, virge_mode->virge.ExtendedMiscellaneousControl1,0x63);
	}
	KRN_DEBUG(1,"chip set");
	
	virge_chipset_accel_restore (&(virge_mode->virge.accel));
	KRN_DEBUG(1,"accellerator restored");
	
	/* Turn on sync signals */
	CRT_OUT8(virge_io,CRT_IN8(virge_io, CRT_EXTSYNC1) & 0xf9, CRT_EXTSYNC1);
	/* Enable screen */
	seq01 = SEQ_IN8(virge_io, 0x01); 
	SEQ_OUT8 (virge_io, seq01 & ~VGA_SR01_DISPLAY_OFF, 0x01);
        MMIO_OUT8(0x20,0x83c0);
	MMIO_OUT8(0x01,0x83c3);
	virge_chipset_interrupts_enable (virge_io, virge);
	virge->mode = virge_mode;

}

void virge_chipset_mode_leave(virge_chipset_t *virge, virge_chipset_io_t *virge_io, virge_chipset_mode_t *virge_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	KRN_DEBUG(2, "virge_chipset_mode_leave()");
	virge_chipset_accel_sync(virge_io);
	virge_chipset_accel_save(&(virge_mode->virge.accel));
	virge_chipset_accel_shutdown(virge_io);
	virge_chipset_interrupts_disable(virge_io);	

	/* Screen off */
	SEQ_OUT8 (virge_io, SEQ_IN8(virge_io, 0x01) | 0x20, 0x01);
	virge->mode = NULL;
}
