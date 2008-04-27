/* ----------------------------------------------------------------------------
**	ATI MACH64 chipset register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64.h,v $
**	Revision 1.8  2003/07/26 18:45:12  cegger
**	merge improvements from the FreeBSD guys:
**	- improves OS independency
**	- add support for new MACH64 chip variants (GX and LM)
**	- bug fixes
**	
**	Revision 1.7  2002/10/05 18:34:39  fspacek
**	Merge all register definitions from ggi applications
**	
**	Revision 1.6  2002/06/13 18:39:48  fspacek
**	Fix for accelerator and PCI ids.
**	
**	Revision 1.5  2002/06/12 14:21:09  fspacek
**	PCI ids for Rage Pro, beginings of a cursor and lots of register definitions
**	
**	Revision 1.4  2002/06/07 01:24:35  fspacek
**	Export VGA-text resources, fixup of some register definition, implement
**	DAC io, and beginings of accelerator context handling
**	
**	Revision 1.3  2002/06/01 23:02:07  fspacek
**	Some reformating and PCI ids for Rage XL/XC
**	
**	Revision 1.2  2002/06/01 22:48:39  fspacek
**	Fantastic new accelerator. Buffers are now bus mastered asynchronously using
**	a circular table of buffers.
**	
**	Revision 1.1  2002/04/03 05:34:33  fspacek
**	Initial import of the ATI MACH64 driver. Should work for MACH64 cards newer
**	than GX (that is all with integrated RAMDAC). Includes hacked up accelerator
**	support (in serious need of cleaning up).
**	
**	Revision 1.2  2001/09/17 19:24:35  phil
**	Various changes. Works for the first time
**	
**	Revision 1.1  2001/08/17 01:16:34  phil
**	Automatically generated ATI chipset code
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_chipset_ATI_MACH64_h
#define	_chipset_ATI_MACH64_h

#define MHZ * 1024 * 1024

/*	PCI config region
**
**	Please use the pci.h definitions for PCICFG register names.
*/
#ifndef	PCI_VENDOR_ID_ATI
#define	PCI_VENDOR_ID_ATI		0x1002
#endif

/* Plain ol' MACH64 */
#define PCI_DEVICE_ID_ATI_MACH64GX      0x4758
#define PCI_DEVICE_ID_ATI_MACH64CX      0x4358
#define PCI_DEVICE_ID_ATI_MACH64CT      0x4354
#define PCI_DEVICE_ID_ATI_MACH64VT      0x5654
#define PCI_DEVICE_ID_ATI_MACH64VTB     0x5655
#define PCI_DEVICE_ID_ATI_MACH64VT4     0x5656

/* 3D Rage */
#define	PCI_DEVICE_ID_ATI_MACH64GT      0x4754
#define	PCI_DEVICE_ID_ATI_MACH64GTB     0x4755
#define PCI_DEVICE_ID_ATI_MACH64GT2C    0x4756

/* Rage Pro */
#define PCI_DEVICE_ID_ATI_MACH64GB      0x4742
#define PCI_DEVICE_ID_ATI_MACH64GD      0x4744
#define PCI_DEVICE_ID_ATI_MACH64GI      0x4749
#define PCI_DEVICE_ID_ATI_MACH64GP      0x4750
#define PCI_DEVICE_ID_ATI_MACH64GQ      0x4751

/* Rage XL/XC */
#define PCI_DEVICE_ID_ATI_MACH64GR      0x4752
#define PCI_DEVICE_ID_ATI_MACH64GS      0x4753
#define PCI_DEVICE_ID_ATI_MACH64GO      0x474F
#define PCI_DEVICE_ID_ATI_MACH64GL      0x474C
#define PCI_DEVICE_ID_ATI_MACH64GM      0x474D
#define PCI_DEVICE_ID_ATI_MACH64GN      0x474E

/* Rage Mobility */
#define PCI_DEVICE_ID_ATI_MACH64LM	0x4C4D 

#define	MACH64_Base0_Size	0x1000000
#define	MACH64_Base1_Size	0x100
#define	MACH64_ROM_Size	0x400

/*
 * MACH64 register definitions
 *
 * Most of these were taken from the Rage PRO and derivatives guide by ATI
 *
 * Some inspiration also came from XFree86
 */

/*
 * Translation from MM format to an offset from the start of the bank 0.
 *
 * Bank 0 registers (MM_xx <= MM_FF) are simply multiplied by 4 (each register
 * is 32-bit). For bank 1 registers 0x800 is substracted from this value
 * (so MM_100 becomes -0x400)
 *
 * Userspace processes should use the index itself as that is what
 * the DMA engine expects.
 */
#ifdef __KERNEL__
#define MM(r)   ((r<<2) - ((r & 0x100)<<3))
#define UNMM(r) ((r>>2) + (((r>>2) & 0x100)<<1))
#else
#define MM(r)   r
#define UNMM(r) r
#endif

/* Accelerator CRTC */
#define MACH64_CRTC_H_TOTAL_DISP              MM(0x00)
#define   MACH64_00_CRTC_H_TOTALMask          0x000001ff       
#define   MACH64_00_CRTC_H_TOTALShift                  0
#define   MACH64_00_CRTC_H_DISPMask           0x00ff0000
#define   MACH64_00_CRTC_H_DISPShift                  16
#define MACH64_CRTC_H_SYNC_STRT_WID           MM(0x01)
#define   MACH64_01_CRTC_H_SYNC_STRTMask      0x000000ff
#define   MACH64_01_CRTC_H_SYNC_STRTShift              0
#define   MACH64_01_CRTC_H_SYNC_DLYMask       0x00000700
#define   MACH64_01_CRTC_H_SYNC_DLYShift               8
#define   MACH64_01_CRTC_H_SYNC_STRT_HI       0x00001000
#define   MACH64_01_CRTC_H_SYNC_WIDMask       0x001f0000
#define   MACH64_01_CRTC_H_SYNC_WIDShift              16
#define   MACH64_01_CRTC_H_SYNC_POL           0x00200000
#define MACH64_CRTC_V_TOTAL_DISP              MM(0x02)
#define   MACH64_02_CRTC_V_TOTALMask          0x000007ff
#define   MACH64_02_CRTC_V_TOTALShift                  0
#define   MACH64_02_CRTC_V_DISPMask           0x07ff0000
#define   MACH64_02_CRTC_V_DISPShift                  16
#define MACH64_CRTC_V_SYNC_STRT_WID           MM(0x03)
#define   MACH64_03_CRTC_V_SYNC_STRTMask      0x000007ff
#define   MACH64_03_CRTC_V_SYNC_STRTShift              0
#define   MACH64_03_CRTC_V_SYNC_WIDMask       0x001f0000
#define   MACH64_03_CRTC_V_SYNC_WIDShift              16
#define   MACH64_03_CRTC_V_SYNC_POL           0x00200000
#define MACH64_CRTC_VLINE_CRNT_VLINE          MM(0x04)
#define   MACH64_04_CRTC_VLINEMask            0x000007ff
#define   MACH64_04_CRTC_VLINEShift                    0
#define   MACH64_04_CRTC_CRNT_VLINEMask       0x07ff0000
#define MACH64_CRTC_OFF_PITCH                 MM(0x05)
#define   MACH64_05_CRTC_OFFSETMask           0x000fffff
#define   MACH64_05_CRTC_OFFSETShift                   0
#define   MACH64_05_CRTC_PITCHMask            0xffc00000
#define   MACH64_05_CRTC_PITCHShift                   22
#define MACH64_CRTC_INT_CNTL                  MM(0x06)
#define   MACH64_06_CRTC_VBLANK               0x00000001
#define   MACH64_06_CRTC_VBLANK_INT_EN        0x00000002
#define   MACH64_06_CRTC_VBLANK_INT           0x00000004
#define   MACH64_06_CRTC_VLINE_INT_EN         0x00000008
#define   MACH64_06_CRTC_VLINE_INT            0x00000010
#define   MACH64_06_CRTC_VLINE_SYNC           0x00000020
#define   MACH64_06_CRTC_FRAME                0x00000040
#define   MACH64_06_SNAPSHOT_INT_EN           0x00000080
#define   MACH64_06_SHAPSHOT_INT              0x00000100
#define   MACH64_06_I2C_INT_EN                0x00000200
#define   MACH64_06_I2C_INT                   0x00000400
#define   MACH64_06_CAPBUF0_INT_EN            0x00010000
#define   MACH64_06_CAPBUF0_INT               0x00020000
#define   MACH64_06_CAPBUF1_INT_EN            0x00040000
#define   MACH64_06_CAPBUF1_INT               0x00080000
#define   MACH64_06_OVERLAY_EOF_INT_EN        0x00100000
#define   MACH64_06_OVERLAY_EOF_INT           0x00200000
#define   MACH64_06_ONESHOT_CAP_INT_EN        0x00400000
#define   MACH64_06_ONESHOT_CAP_INT           0x00800000
#define   MACH64_06_BUSMASTER_EOL_INT_EN      0x01000000
#define   MACH64_06_BUSMASTER_EOL_INT         0x02000000
#define   MACH64_06_GP_INT_EN                 0x04000000
#define   MACH64_06_GP_INT                    0x08000000
#define   MACH64_06_VBLANK_BIT2               0x80000000
#define MACH64_CRTC_GEN_CNTL                  MM(0x07)
#define   MACH64_07_CRTC_DBL_SCAN_EN          0x00000001
#define   MACH64_07_CRTC_INTERLACE_EN         0x00000002
#define   MACH64_07_CRTC_HSYNC_DIS            0x00000004
#define   MACH64_07_CRTC_VSYNC_DIR            0x00000008
#define   MACH64_07_CRTC_CSYNC_EN             0x00000010
#define   MACH64_07_CRTC_DISPLAY_DIS          0x00000040
#define   MACH64_07_CRTC_VGA_XOVERSCAN        0x00000080
#define   MACH64_07_CRTC_PIX_WIDTHMask        0x00000700
#define   MACH64_07_CRTC_PIX_WIDTHShift                8
#define     MACH64_07_CRTC_PIX_WIDTH4         0x00000100
#define     MACH64_07_CRTC_PIX_WIDTH8         0x00000200
#define     MACH64_07_CRTC_PIX_WIDTH15        0x00000300
#define     MACH64_07_CRTC_PIX_WIDTH16        0x00000400
#define     MACH64_07_CRTC_PIX_WIDTH24        0x00000500
#define     MACH64_07_CRTC_PIX_WIDTH32        0x00000600
#define   MACH64_07_CRTC_BYTE_PIX_ORDER       0x00000800
#define   MACH64_07_CRTC_FIFO_LWMMask         0x000f0000 /* is this right? */
#define   MACH64_07_CRTC_FIFO_LWMShift                16
#define   MACH64_07_VGA_128KAP_PAGING         0x00100000
#define   MACH64_07_VFC_SYNC_TRISTATE         0x00200000
#define   MACH64_07_CRTC_LOCK_REGS            0x00400000
#define   MACH64_07_CRTC_SYNC_TRISTATE        0x00800000
#define   MACH64_07_CRTC_EXT_DISP_EN          0x01000000
#define   MACH64_07_CRTC_ENABLE               0x02000000
#define   MACH64_07_CRTC_DISP_REQ_ENB         0x04000000
#define   MACH64_07_VGA_ATI_LINEAR            0x08000000
#define   MACH64_07_CRTC_VSYNC_FALL_EDGE      0x10000000
#define   MACH64_07_VGA_TEXT_132              0x20000000
#define   MACH64_07_VGA_XCRT_CNT_EN           0x40000000
#define   MACH64_07_VGA_CUR_B_TEST            0x80000000

/* Memory Buffer Control */
#define MACH64_DSP_CONFIG                     MM(0x08)
#define   MACH64_08_DSP_XCLKS_PER_QWShift              0
#define   MACH64_08_DSP_XCLKS_PER_QWMask      0x00003fff
#define   MACH64_08_DSP_FLUSH_WB              0x00008000
#define   MACH64_08_DSP_LOOP_LATENCYShift             16
#define   MACH64_08_DSP_LOOP_LATENCYMask      0x000f0000
#define   MACH64_08_DSP_PRECISIONShift                20
#define   MACH64_08_DSP_PRECISIONMask         0x00700000
#define MACH64_DSP_ON_OFF                     MM(0x09)
#define   MACH64_09_DSP_OFFShift                       0
#define   MACH64_09_DSP_OFFMask               0x000007ff
#define   MACH64_09_DSP_ONShift                       16
#define   MACH64_09_DSP_ONMask                0x07ff0000
#define MACH64_TIMER_CONFIG                   MM(0x0A)
#define   MACH64_0A_VID_INTRA_ACCESS_TIMERMask  0x000000ff
#define   MACH64_0A_VID_INTRA_ACCESS_TIMERShift          0
#define   MACH64_0A_SCL_INTRA_ACCESS_TIMERMask  0x0000ff00
#define   MACH64_0A_SCL_INTRA_ACCESS_TIMERShift          8
#define   MACH64_0A_VID_TIMER_MODE              0x01000000
#define MACH64_MEM_BUF_CNTL                   MM(0x0B)
#define   MACH64_0B_Z_WB_FLUSHMask            0x00000003
#define   MACH64_0B_Z_WB_FLUSHShift                    0
#define   MACH64_0B_VID_WB_FLUSH_MSB          0x00000100
#define   MACH64_0B_SCL_MIN_BURST_LENMask     0x001f0000
#define   MACH64_0B_SCL_MIN_BURST_LENShift            16
#define   MACH64_0B_INVALDATE_RB_CACHE        0x00800000
#define   MACH64_0B_HST_WB_FLUSHMask          0x03000000
#define   MACH64_0B_HST_WB_FLUSHShfit                 24
#define   MACH64_0B_VID_WB_FLUSHMask          0x1c000000
#define   MACH64_0B_VID_WB_FLUSHShift                 26
#define   MACH64_0B_GUI_WB_FLUSHMask          0xe0000000
#define   MACH64_0B_GUI_WB_FLUSHShift                 29
#define MACH64_MEM_ADDR_CONFIG                MM(0x0D)
#define   MACH64_0D_MEM_ROW_MAPPINGMask       0x00000007
#define   MACH64_0D_MEM_ROW_MAPPINGShift               0
#define   MACH64_0D_MEM_COL_MAPPINGMask       0x00000070
#define   MACH64_0D_MEM_COL_MAPPINGShift               4
#define   MACH64_0D_MEM_GROUP_SIZEMask        0x00000300
#define   MACH64_0D_MEM_GROUP_SIZEShift                8

/* Accelerator CRTC */
#define MACH64_CRT_TRAP                       MM(0x0E)

/* Overscan */
#define MACH64_OVR_CLR                        MM(0x10)
#define MACH64_OVR_WID_LEFT_RIGHT             MM(0x11)
#define MACH64_OVR_WID_TOP_BOTTOM             MM(0x12)

/* Memory Buffer Control */
#define MACH64_VGA_DSP_CONFIG                 MM(0x13)
#define   MACH64_13_VGA_DSP_XCLKS_PER_QWMask  0x00003fff
#define   MACH64_13_VGA_DSP_XCLKS_PER_QWShift          0
#define   MACH64_13_VGA_DSP_PREC_PCLKBY2Mask  0x00700000
#define   MACH64_13_VGA_DSP_PREC_PCLKBY2Shift         20
#define   MACH64_13_VGA_DSP_PREC_PCLKMask     0x07000000
#define   MACH64_13_VGA_DSP_PREC_PCLKShift            24
#define MACH64_VGA_DSP_ON_OFF                 MM(0x14)
#define   MACH64_14_VGA_DSP_OFFMask           0x000003ff
#define   MACH64_14_VGA_DSP_OFFShift                   0
#define   MACH64_14_VGA_DSP_ONMask            0x03ff0000
#define   MACH64_14_VGA_DSP_ONShift                   16

/* Cursor Control */
#define MACH64_CUR_CLR0                       MM(0x18)
#define   MACH64_18_CUR_CLR0_8Mask            0x000000ff
#define   MACH64_18_CUR_CLR0_8Shift                    0
#define   MACH64_18_CUR_CLR0_BMask            0x0000ff00
#define   MACH64_18_CUR_CLR0_BShift                    8
#define   MACH64_18_CUR_CLR0_GMask            0x00ff0000
#define   MACH64_18_CUR_CLR0_GShift                   16
#define   MACH64_18_CUR_CLR0_RMask            0xff000000
#define   MACH64_18_CUR_CLR0_RShift                   24
#define MACH64_CUR_CLR1                       MM(0x19)
#define   MACH64_19_CUR_CLR1_8Mask            0x000000ff
#define   MACH64_19_CUR_CLR1_8Shift                    0
#define   MACH64_19_CUR_CLR1_BMask            0x0000ff00
#define   MACH64_19_CUR_CLR1_BShift                    8
#define   MACH64_19_CUR_CLR1_GMask            0x00ff0000
#define   MACH64_19_CUR_CLR1_GShift                   16
#define   MACH64_19_CUR_CLR1_RMask            0xff000000
#define   MACH64_19_CUR_CLR1_RShift                   24
#define MACH64_CUR_OFFSET                     MM(0x1A)
#define   MACH64_1A_CUR_OFFSETMask            0x000fffff
#define   MACH64_1A_CUR_OFFSETShift                    0
#define MACH64_CUR_HORZ_VERT_POSN             MM(0x1B)
#define   MACH64_1B_CUR_HORZ_POSNMask         0x000007ff
#define   MACH64_1B_CUR_HORZ_POSNShift                 0
#define   MACH64_1B_CUR_VERT_POSNMask         0x07ff0000
#define   MACH64_1B_CUR_VERT_POSNShift                16
#define MACH64_CUR_HORZ_VERT_OFF              MM(0x1C)
#define   MACH64_1C_CUR_HORZ_OFFMask          0x0000003f
#define   MACH64_1C_CUR_HORZ_OFFShift                  0
#define   MACH64_1C_CUR_VERT_OFFMask          0x003f0000
#define   MACH64_1C_CUR_VERT_OFFShift                 16

/* General IO Control */
#define MACH64_GP_IO                          MM(0x1E)

/* Scratch Pad and Test */
#define MACH64_SCRATCH_REG0                   MM(0x20)
#define MACH64_SCRATCH_REG1                   MM(0x21)

/* Clock Control */
#define MACH64_CLOCK_CNTL                     MM(0x24)
#define   MACH64_24_CLOCK_SELMask             0x00000003
#define   MACH64_24_CLOCK_SELShift                     0
#define   MACH64_24_PLL_WR_EN                 0x00000200
#define   MACH64_24_PLL_ADDRMask              0x00007c00
#define   MACH64_24_PLL_ADDRShift                     10
#define   MACH64_24_PLL_DATA                  0x00ff0000
#define   MACH64_24_PLL_DATAShift                     16

/* Configuration */
#define MACH64_CONFIG_STAT1                   MM(0x25)
#define   MACH64_25_SUBSYS_DEV_IDMask         0x000000ff
#define   MACH64_25_SUBSYS_VEN_IDMask         0x00ffff00
#define   MACH64_25_DIMM_TYPEMask             0xe0000000
#define MACH64_CONFIG_STAT2                   MM(0x26)
#define   MACH64_26_FLASH_MEM                 0x00000020
#define   MACH64_26_AGPVCOGAINMask            0x000000c0
#define   MACH64_26_BUS_TYPE                  0x00000100
#define   MACH64_26_AGPSKEWMask               0x00000e00
#define   MACH64_26_X1CLKSKEWMask             0x00007000
#define   MACH64_26_CS_EN                     0x00008000
#define   MACH64_26_CFG_MEM_TYPEbMask         0x00070000
#define   MACH64_26_ID_DISABLE                0x00080000
#define   MACH64_26_CHGID0                    0x00100000
#define   MACH64_26_PREFETCH_EN               0x00200000
#define   MACH64_26_PRE_TESTEN                0x00800000
#define   MACH64_26_CHGID1                    0x01000000
#define   MACH64_26_VFC_SENSEb                0x02000000
#define   MACH64_26_VGA_DISABLE               0x04000000
#define   MACH64_26_ENINTB                    0x08000000
#define   MACH64_26_ROM_REMAP                 0x20000000
#define   MACH64_26_IDSEL                     0x40000000

/* Bus Control */
#define MACH64_BUS_CNTL                       MM(0x28)
#define   MACH64_28_BUS_DBL_RESYNC            0x00000001
#define   MACH64_28_BUS_MSTR_RESET            0x00000002
#define   MACH64_28_BUS_FLUSH_BUF             0x00000004
#define   MACH64_28_BUS_STOP_REQ_DIS          0x00000008
#define   MACH64_28_BUS_APER_REG_DIS          0x00000010
#define   MACH64_28_BUS_EXTRA_PIPE_DIS        0x00000020
#define   MACH64_28_BUS_MASTER_DIS            0x00000040
#define   MACH64_28_ROM_WRT_EN                0x00000080
#define   MACH64_28_BUS_PCI_READ_RETRY_EN     0x00002000
#define   MACH64_28_BUS_PCI_WRT_RETRY_EN      0x00008000
#define   MACH64_28_BUS_RETRY_WSMask          0x000f0000
#define   MACH64_28_BUS_RETRY_WSShift                 16
#define   MACH64_28_BUS_MSTR_RD_MULT          0x00100000
#define   MACH64_28_BUS_MSTR_RD_LINE          0x00200000
#define   MACH64_28_BUS_SUSPEND               0x00400000
#define   MACH64_28_LAT16X                    0x00800000
#define   MACH64_28_BUS_RD_DISCARD_EN         0x01000000
#define   MACH64_28_BUS_RD_ABORT_EN           0x02000000
#define   MACH64_28_BUS_MSTR_WS               0x04000000
#define   MACH64_28_BUS_EXT_REG_EN            0x08000000
#define   MACH64_28_BUS_MSTR_DISCONNECT_EN    0x10000000
#define   MACH64_28_BUS_WRT_BURST             0x20000000
#define   MACH64_28_BUS_READ_BURST            0x40000000
#define   MACH64_28_BUS_RDY_READ_DLY          0x80000000

/* Memory Control */
#define MACH64_EXT_MEM_CNTL                   MM(0x2B)
#define   MACH64_2B_MEM_CS                    0x00000001
#define   MACH64_2B_MEM_SDRAM_RESET           0x00000002
#define   MACH64_2B_MEM_CYC_TESTMask          0x0000000c
#define   MACH64_2B_MEM_TILE_SELECTMask       0x000000f0
#define   MACH64_2B_MEM_CLK_SELECTMask        0x00000300
#define   MACH64_2B_MEM_CAS_LATENCYMask       0x00000c00
#define   MACH64_2B_MEM_TILE_BOUNDARYMask     0x0000f000
#define   MACH64_2B_MEM_MDA_DRIVE             0x00010000
#define   MACH64_2B_MEM_MDB_DRIVE             0x00020000
#define   MACH64_2B_MEM_MDE_DELAY             0x00040000
#define   MACH64_2B_MEM_MDO_DELAY             0x00080000
#define   MACH64_2B_MEM_MA_DRIVE              0x00100000
#define   MACH64_2B_MEM_MA_DELAY              0x00200000
#define   MACH64_2B_MEM_GCMRSMask             0x0f000000
#define   MACH64_2B_MEM_CS_STRAP              0x10000000
#define   MACH64_2B_SDRAM_MEM_CFG             0x20000000
#define   MACH64_2B_MEM_ALLPAGE_DIS           0x40000000
#define   MACH64_2B_MEM_GROUP_FAULT_EN        0x80000000
#define MACH64_MEM_CNTL                       MM(0x2C)
#define   MACH64_2C_MEM_SIZEMask              0x0000000f
#define   MACH64_2C_MEM_SIZEShift                      0
#define   MACH64_2C_MEM_LATENCYMask           0x00000030
#define   MACH64_2C_MEM_LATENCYShift                   4
#define   MACH64_2C_MEM_LATCHMask             0x000000c0
#define   MACH64_2C_MEM_LATCHShift                     6
#define   MACH64_2C_MEM_TRPMask               0x00000300
#define   MACH64_2C_MEM_TRPShift                       8
#define   MACH64_2C_MEM_TRCDMask              0x00000c00
#define   MACH64_2C_MEM_TRCDShift                     10
#define   MACH64_2C_MEM_TCRD                  0x00001000
#define   MACH64_2C_MEM_TR2W                  0x00002000
#define   MACH64_2C_MEM_CAS_PHASE             0x00004000
#define   MACH64_2C_MEM_OE_PULLBACK           0x00008000
#define   MACH64_2C_MEM_TRASMask              0x00070000
#define   MACH64_2C_MEM_TRASShift                     16
#define   MACH64_2C_MEM_REFRESH_DIS           0x00080000
#define   MACH64_2C_MEM_REFRESH_RATEMask      0x00700000
#define   MACH64_2C_MEM_REFRESH_RATEShift             20
#define   MACH64_2C_MEM_LOWER_APER_ENDIANMask 0x03000000
#define   MACH64_2C_MEM_LOWER_APER_ENDIANShift        24
#define   MACH64_2C_MEM_UPPER_APER_ENDIANMask 0x0c000000
#define   MACH64_2C_MEM_UPPER_APER_ENDIANShift        26
#define   MACH64_2C_MEM_PAGE_SIZEMask         0x30000000
#define   MACH64_2C_MEM_PAGE_SIZEShift                28
#define   MACH64_2C_MEM_PAGE_SIZE2K           0x00000000
#define   MACH64_2C_MEM_PAGE_SIZE4K           0x10000000
#define   MACH64_2C_MEM_PAGE_SIZE8K           0x20000000
#define   MACH64_2C_MEM_PAGE_SIZE16K          0x30000000
#define   MACH64_2C_MEM_CAS_SKEWMask          0xc0000000
#define   MACH64_2C_MEM_CAS_SKEWShift                 30
#define MACH64_MEM_VGA_WP_SEL                 MM(0x2D)
#define   MACH64_2D_MEM_VGA_WPS0Mask          0x000000ff
#define   MACH64_2D_MEM_VGA_WPS0Shift                  0
#define   MACH64_2D_MEM_VGA_WPS1Mask          0x00ff0000
#define   MACH64_2D_MEM_VGA_WPS1Shift                 16
#define MACH64_MEM_VGA_RP_SEL                 MM(0x2E)
#define   MACH64_2E_MEM_VGA_RPS0Mask          0x000000ff
#define   MACH64_2E_MEM_VGA_RPS0Shift                  0
#define   MACH64_2E_MEM_VGA_RPS1Mask          0x00ff0000
#define   MACH64_2E_MEM_VGA_RPS1Shift                 16

/* DAC Control */
#define MACH64_DAC_CNTL                       MM(0x31)
#define   MACH64_31_DAC_RANGE_CNTLMask        0x00000003
#define   MACH64_31_DAC_RANGE_CNTLShift                0
#define   MACH64_31_DAC_BLANKING              0x00000004
#define   MACH64_31_DAC_CMP_DISABLE           0x00000008
#define   MACH64_31_DAC1_CLK_SEL              0x00000010
#define   MACH64_31_DAC_CMP_OUTPUT            0x00000080
#define   MACH64_31_DAC_8BIT_EN               0x00000100
#define   MACH64_31_DAC_DIRECT                0x00000400
#define   MACH64_31_DAC_PAL_CLK_SEL           0x00000800
#define   MACH64_31_DAC_VGA_ADR_EN            0x00002000
#define   MACH64_31_DAC_FEA_CON_EN            0x00004000
#define   MACH64_31_DAC_PDWN                  0x00008000
#define   MACH64_31_DAC_TYPEMask              0x00070000
#define MACH64_EXT_DAC_REGS                   MM(0x32)
#define   MACH64_32_REG_SELECTMask            0x0000000f
#define   MACH64_32_DATAMask                  0x0000ff00
#define   MACH64_32_EXT_DAC_EN                0x00010000
#define   MACH64_32_EXT_DAC_WID               0x00020000

/* Test and Debug */
#define MACH64_GEN_TEST_CNTL                  MM(0x34)
#define   MACH64_34_GEN_CUR_ENABLE            0x00000080
#define   MACH64_34_GEN_GUI_RESETB            0x00000100
#define   MACH64_34_GEN_SOFT_RESET            0x00000200
#define   MACH64_34_GEN_TEST_VECTOR_MODEMask  0x00003000
#define   MACH64_34_GEN_TEST_VECTOR_MODEShift         12
#define   MACH64_34_GEN_TEST_MODEMask         0x000f0000
#define   MACH64_34_GEN_TEST_MODEShift                16
#define   MACH64_34_GEN_CRC_EN                0x00200000
#define   MACH64_34_GEN_DEBUG_MODEMask        0xff000000
#define   MACH64_34_GEN_DEBUG_MODEShift               24

/* Configuration */
#define MACH64_CONFIG_CNTL                    MM(0x37)
#define   MACH64_37_CFG_MEM_AP_SIZEMask       0x00000002
#define   MACH64_37_CFG_MEM_VGA_AP_EN         0x00000004
#define   MACH64_37_CFG_MEM_AP_LOCMask        0x00003ff0
#define   MACH64_37_CFG_VGA_DIS               0x00080000
#define MACH64_CONFIG_CHIP_ID                 MM(0x38)
#define   MACH64_38_CFG_CHIP_TYPEMask         0x0000ffff
#define   MACH64_38_CFG_CHIP_CLASSMask        0x00ff0000
#define   MACH64_38_CFG_CHIP_MAJORMask        0x07000000
#define   MACH64_38_CFG_CHIP_FND_IDMask       0x38000000
#define   MACH64_38_CFG_CHIP_MINORMask        0xc0000000
#define MACH64_CONFIG_STAT0                   MM(0x39)
#define   MACH64_39_CFG_MEM_TYPEMask          0x00000007
#define   MACH64_39_ROM_REMAP                 0x00000008
#define   MACH64_39_CFG_VGA_EN                0x00000010
#define   MACH64_39_CFG_CLOCK_EN              0x00000020
#define   MACH64_39_VFC_SENSE                 0x00000080
#define   MACH64_39_BOARD_IDMask              0x0000ff00
#define   MACH64_39_PCI66                     0x00400000
#define   MACH64_39_PKGBGAb                   0x00800000

/* Draw Engine Destination Trajectory */
#define MACH64_DST_OFF_PITCH                  MM(0x40)
#define   MACH64_40_DST_OFFSETMask            0x000fffff
#define   MACH64_40_DST_OFFSETShift                    0
#define   MACH64_40_DST_PITCHMask             0xffc00000
#define   MACH64_40_DST_PITCHShift                    22
#define MACH64_DST_X                          MM(0x41)
#define   MACH64_41_DST_XMask                 0x00001fff
#define   MACH64_41_DST_XShift                         0
#define MACH64_DST_Y                          MM(0x42)
#define   MACH64_42_DST_YMask                 0x00007fff
#define   MACH64_42_DST_YShift                         0
#define MACH64_DST_Y_X                        MM(0x43)
#define   MACH64_43_DST_YMask                 0x00007fff
#define   MACH64_43_DST_YShift                         0
#define   MACH64_43_DST_XMask                 0x1fff0000
#define   MACH64_43_DST_XShift                        16
#define MACH64_DST_WIDTH                      MM(0x44)
#define   MACH64_44_DST_WIDTHMask             0x00001fff /*FIXME Diff on IIC */
#define   MACH64_44_DST_WIDTHShift                     0
#define MACH64_DST_HEIGHT                     MM(0x45)
#define   MACH64_45_DST_HEIGHTMask            0x00007fff
#define   MACH64_45_DST_HEIGHTShift                    0
#define MACH64_DST_HEIGHT_WIDTH               MM(0x46)
#define   MACH64_46_DST_HEIGHTMask            0x00007fff
#define   MACH64_46_DST_HEIGHTShift                    0
#define   MACH64_46_DST_WIDTHMask             0x1fff0000
#define   MACH64_46_DST_WIDTHShift                    16
#define MACH64_DST_X_WIDTH                    MM(0x47)
#define   MACH64_47_DST_XMask                 0x00001fff
#define   MACH64_47_DST_XShift                         0
#define   MACH64_47_DST_WIDTHMask             0x1fff0000
#define   MACH64_47_DST_WIDTHShift                    16
#define MACH64_DST_BRES_LNTH                  MM(0x48)
#define   MACH64_48_DST_BRES_LNTHMask         0x00007fff
#define   MACH64_48_DST_BRES_LNTHShift                 0
#define   MACH64_48_DRAW_TRAP                 0x00008000
#define   MACH64_48_TRAIL_XMask               0x1fff0000
#define   MACH64_48_TRAIL_XShift                      16
#define   MACH64_48_DST_BRES_LNTH_LINE_DIS    0x80000000
#define MACH64_DST_BRES_ERR                   MM(0x49)
#define   MACH64_49_DST_BRES_ERRMask          0x0003ffff
#define   MACH64_49_DST_BRES_ERRShift                  0
#define MACH64_DST_BRES_INC                   MM(0x4A)
#define   MACH64_4A_DST_BRES_INCMask          0x0003ffff
#define   MACH64_4A_DST_BRES_INCShift                  0
#define MACH64_DST_BRES_DEC                   MM(0x4B)
#define   MACH64_4B_DST_BRES_DECMask          0x0003ffff
#define   MACH64_4B_DST_BRES_DECShift                  0
#define MACH64_DST_CNTL                       MM(0x4C)
#define   MACH64_4C_DST_X_DIR                 0x00000001
#define   MACH64_4C_DST_Y_DIR                 0x00000002
#define   MACH64_4C_DST_Y_MAJOR               0x00000004
#define   MACH64_4C_DST_X_TILE                0x00000008
#define   MACH64_4C_DST_Y_TILE                0x00000010
#define   MACH64_4C_DST_LAST_PEL              0x00000020
#define   MACH64_4C_DST_POLYGON_EN            0x00000040
#define   MACH64_4C_DST_24_ROT_EN             0x00000080
#define   MACH64_4C_DST_24_ROTMask            0x00000700
#define   MACH64_4C_DST_24_ROTShift                    8
#define   MACH64_4C_DST_BRES_SIGN             0x00000800
#define   MACH64_4C_DST_POLYGON_RTEDGE_DIS    0x00001000
#define   MACH64_4C_TRAIL_X_DIR               0x00002000
#define   MACH64_4C_TRAIL_FILL_DIR            0x00004000
#define   MACH64_4C_TRAIL_BRES_SIGN           0x00008000
#define   MACH64_4C_BRES_SIGN_AUTO            0x00020000
#define MACH64_TRAIL_BRES_ERR                 MM(0x4E)
#define   MACH64_4E_TRAIL_BRES_ERRMask        0x0003ffff
#define   MACH64_4E_TRAIL_BRES_ERRShift                0
#define MACH64_TRAIL_BRES_INC                 MM(0x4F)
#define   MACH64_4F_TRAIL_BRES_INCMask        0x0003ffff
#define   MACH64_4F_TRAIL_BRES_INCShift                0
#define MACH64_TRAIL_BRES_DEC                 MM(0x50)
#define   MACH64_50_TRAIL_BRES_DECMask        0x0003ffff
#define   MACH64_50_TRAIL_BRES_DECShift                0
#define MACH64_Z_OFF_PITCH                    MM(0x52)
#define   MACH64_52_Z_OFFSETMask              0x000fffff
#define   MACH64_52_Z_OFFSETShift                      0
#define   MACH64_52_Z_PITCHMask               0xffc00000
#define   MACH64_52_Z_PITCHShift                      22
#define MACH64_Z_CNTL                         MM(0x53)
#define   MACH64_53_Z_EN                      0x00000001
#define   MACH64_53_Z_SRC                     0x00000002
#define   MACH64_53_Z_TESTMask                0x00000070
#define   MACH64_53_Z_TESTShift                        4
#define   MACH64_53_Z_MASK                    0x00000100

/* Draw Engine Source Trajectory */
#define MACH64_SRC_OFF_PITCH                  MM(0x60)
#define   MACH64_60_SRC_OFFSETMask            0x000fffff
#define   MACH64_60_SRC_OFFSETShift                    0
#define   MACH64_60_SRC_PITCHMask             0xffc00000
#define   MACH64_60_SRC_PITCHShift                    22
#define MACH64_SRC_X                          MM(0x61)
#define   MACH64_61_SRC_XMask                 0x00001fff
#define   MACH64_61_SRC_XShift                         0
#define MACH64_SRC_Y                          MM(0x62)
#define   MACH64_62_SRC_YMask                 0x00007fff
#define   MACH64_62_SRC_YShift                         0
#define MACH64_SRC_Y_X                        MM(0x63)
#define   MACH64_63_SRC_YMask                 0x00007fff
#define   MACH64_63_SRC_YShift                         0
#define   MACH64_63_SRC_XMask                 0x1fff0000
#define   MACH64_63_SRC_XShift                        16
#define MACH64_SRC_WIDTH1                     MM(0x64)
#define   MACH64_64_SRC_WIDTH1Mask            0x00001fff
#define   MACH64_64_SRC_WIDTH1Shift                    0
#define MACH64_SRC_HEIGHT1                    MM(0x65)
#define   MACH64_65_SRC_HEIGHT1Mask           0x00007fff
#define   MACH64_65_SRC_HEIGHT1Shift                   0
#define MACH64_SRC_HEIGHT1_WIDTH1             MM(0x66)
#define   MACH64_66_SRC_HEIGHT1Mask           0x00007fff
#define   MACH64_66_SRC_HEIGHT1Shift                   0
#define   MACH64_66_SRC_WIDTH1Mask            0x1fff0000
#define   MACH64_66_SRC_WIDTH1Shift                   16
#define MACH64_SRC_X_START                    MM(0x67)
#define   MACH64_67_SRC_X_STARTMask           0x00001fff
#define   MACH64_67_SRC_X_STARTShift                   0
#define MACH64_SRC_Y_START                    MM(0x68)
#define   MACH64_68_SRC_Y_STARTMask           0x00007fff
#define   MACH64_68_SRC_Y_STARTShift                   0
#define MACH64_SRC_Y_X_START                  MM(0x69)
#define   MACH64_69_SRC_Y_STARTMask           0x00007fff
#define   MACH64_69_SRC_Y_STARTShift                   0
#define   MACH64_69_SRC_X_STARTMask           0x1fff0000
#define   MACH64_69_SRC_X_STARTShift                  16
#define MACH64_SRC_WIDTH2                     MM(0x6A)
#define   MACH64_6A_SRC_WIDTH2Mask            0x00001fff
#define   MACH64_6A_SRC_WIDTH2Shift                    0
#define MACH64_SRC_HEIGHT2                    MM(0x6B)
#define   MACH64_6B_SRC_HEIGHT2Mask           0x00007fff
#define   MACH64_6B_SRC_HEIGHT2Shift                   0
#define MACH64_SRC_HEIGHT2_WIDTH2             MM(0x6C)
#define   MACH64_6C_SRC_HEIGHT2Mask           0x00007fff
#define   MACH64_6C_SRC_HEIGHT2Shift                   0
#define   MACH64_6C_SRC_WIDTH2Mask            0x1fff0000
#define   MACH64_6C_SRC_WIDTH2Shift                   16
#define MACH64_SRC_CNTL                       MM(0x6D)
#define   MACH64_6D_SRC_PATT_EN               0x00000001
#define   MACH64_6D_SRC_PATT_ROT_EN           0x00000002
#define   MACH64_6D_SRC_LINEAR_EN             0x00000004
#define   MACH64_6D_SRC_BYTE_ALIGN            0x00000008
#define   MACH64_6D_SRC_LINE_X_DIR            0x00000010
#define   MACH64_6D_SRC_8x8x8_BRUSH           0x00000020
#define   MACH64_6D_FAST_FILL_EN              0x00000040
#define   MACH64_6D_SRC_TRACK_DST_EN          0x00000080
#define   MACH64_6D_BUS_MASTER_EN             0x00000100
#define   MACH64_6D_BUS_MASTER_SYNC           0x00000200
#define   MACH64_6D_BUS_MASTER_OPMask         0x00000c00
#define   MACH64_6D_BUS_MASTER_OPShift                10
#define   MACH64_6D_SRC_8x8x8_BRUSH_LOADED    0x00001000
#define   MACH64_6D_COLOR_REG_WRITE_EN        0x00002000
#define   MACH64_6D_BLOCK_WRITE_EN            0x00004000

/* Host Data */
#define MACH64_HOST_CNTL                      MM(0x90)
#define   MACH64_90_HOST_BYTE_ALIGN           0x00000001
#define   MACH64_90_HOST_BIG_ENDIAN_EN        0x00000002

/* Pattern */
#define MACH64_PAT_REG0                       MM(0xA0)
#define MACH64_PAT_REG1                       MM(0xA1)
#define MACH64_PAT_CNTL                       MM(0xA2)
#define   MACH64_A2_PAT_MONO_EN               0x00000001
#define   MACH64_A2_PAT_CLR_4x2_EN            0x00000002
#define   MACH64_A2_PAT_CLR_8x1_EN            0x00000004

/* Scissors */
#define MACH64_SC_LEFT                        MM(0xA8)
#define   MACH64_A8_SC_LEFTMask               0x00001fff
#define   MACH64_A8_SC_LEFTShift                       0
#define MACH64_SC_RIGHT                       MM(0xA9)
#define   MACH64_A9_SC_RIGHTMask              0x00001fff
#define   MACH64_A9_SC_RIGHTShift                      0
#define MACH64_SC_LEFT_RIGHT                  MM(0xAA)
#define   MACH64_AA_SC_LEFTMask               0x00001fff
#define   MACH64_AA_SC_LEFTShift                       0
#define   MACH64_AA_SC_RIGHTMask              0x1fff0000
#define   MACH64_AA_SC_RIGHTShift                     16
#define MACH64_SC_TOP                         MM(0xAB)
#define   MACH64_AB_SC_TOPMask                0x00007fff
#define   MACH64_AB_SC_TOPShift                        0
#define MACH64_SC_BOTTOM                      MM(0xAC)
#define   MACH64_AC_SC_BOTTOMMask             0x00007fff
#define   MACH64_AC_SC_BOTTOMShift                     0
#define MACH64_SC_TOP_BOTTOM                  MM(0xAD)
#define   MACH64_AD_SC_TOPMask                0x00007fff
#define   MACH64_AD_SC_TOPShift                        0
#define   MACH64_AD_SC_BOTTOMMask             0x7fff0000
#define   MACH64_AD_SC_BOTTOMShift                    16

/* Data Path */
#define MACH64_DP_BKGD_CLR                    MM(0xB0)
#define MACH64_DP_FRGD_CLR                    MM(0xB1)
#define MACH64_DP_WRITE_MASK                  MM(0xB2)
#define MACH64_DP_PIX_WIDTH                   MM(0xB4)
#define   MACH64_B4_DP_DST_PIX_WIDTHMask      0x0000000f
#define   MACH64_B4_DP_DST_PIX_WIDTHShift              0
#define   MACH64_B4_DP_SRC_PIX_WIDTHMask      0x00000f00
#define   MACH64_B4_DP_SRC_PIX_WIDTHShift              8
#define   MACH64_B4_DP_HOST_TRIPLE_EN         0x00002000
#define   MACH64_B4_DP_SRC_AUTONA_FIX_DIS     0x00004000
#define   MACH64_B4_DP_FAST_SRCCOPY_DIS       0x00008000
#define   MACH64_B4_DP_HOST_PIX_WIDTHMask     0x000f0000
#define   MACH64_B4_DP_HOST_PIX_WIDTHShift            16
#define   MACH64_B4_DP_CI4_RGB_INDEXMask      0x00f00000
#define   MACH64_B4_DP_CI4_RGB_INDEXShift             20
#define   MACH64_B4_DP_BYTE_PIX_ORDER         0x01000000
#define   MACH64_B4_DP_CONVERSION_TEMP        0x02000000
#define   MACH64_B4_DP_CI4_RGB_LOW_NIBBLE     0x04000000
#define   MACH64_B4_DP_CI4_RGB_HIGH_NIBBLE    0x08000000
#define   MACH64_B4_DP_SCALE_PIX_WIDTHMask    0xf0000000
#define   MACH64_B4_DP_SCALE_PIX_WIDTHShift           28
#define MACH64_DP_MIX                         MM(0xB5)
#define   MACH64_B5_DP_BKGD_MIXMask           0x0000001f
#define   MACH64_B5_DP_BKGD_MIXShift                   0
#define   MACH64_B5_BKGD_MIX_NOT_D            0x00000000
#define   MACH64_B5_BKGD_MIX_ZERO             0x00000001
#define   MACH64_B5_BKGD_MIX_ONE              0x00000002
#define   MACH64_B5_BKGD_MIX_D                0x00000003
#define   MACH64_B5_BKGD_MIX_NOT_S            0x00000004
#define   MACH64_B5_BKGD_MIX_D_XOR_S          0x00000005
#define   MACH64_B5_BKGD_MIX_NOT_D_XOR_S      0x00000006
#define   MACH64_B5_BKGD_MIX_S                0x00000007
#define   MACH64_B5_BKGD_MIX_NOT_D_OR_NOT_S   0x00000008
#define   MACH64_B5_BKGD_MIX_D_OR_NOT_S       0x00000009
#define   MACH64_B5_BKGD_MIX_NOT_D_OR_S       0x0000000a
#define   MACH64_B5_BKGD_MIX_D_OR_S           0x0000000b
#define   MACH64_B5_BKGD_MIX_D_AND_S          0x0000000c
#define   MACH64_B5_BKGD_MIX_NOT_D_AND_S      0x0000000d
#define   MACH64_B5_BKGD_MIX_D_AND_NOT_S      0x0000000e
#define   MACH64_B5_BKGD_MIX_NOT_D_AND_NOT_S  0x0000000f
#define   MACH64_B5_BKGD_MIX_D_PLUS_S_DIV2    0x00000017
#define   MACH64_B5_DP_FRGD_MIXMask           0x001f0000
#define   MACH64_B5_DP_FRGD_MIXShift                  16
#define   MACH64_B5_FRGD_MIX_NOT_D            0x00000000
#define   MACH64_B5_FRGD_MIX_ZERO             0x00010000
#define   MACH64_B5_FRGD_MIX_ONE              0x00020000
#define   MACH64_B5_FRGD_MIX_D                0x00030000
#define   MACH64_B5_FRGD_MIX_NOT_S            0x00040000
#define   MACH64_B5_FRGD_MIX_D_XOR_S          0x00050000
#define   MACH64_B5_FRGD_MIX_NOT_D_XOR_S      0x00060000
#define   MACH64_B5_FRGD_MIX_S                0x00070000
#define   MACH64_B5_FRGD_MIX_NOT_D_OR_NOT_S   0x00080000
#define   MACH64_B5_FRGD_MIX_D_OR_NOT_S       0x00090000
#define   MACH64_B5_FRGD_MIX_NOT_D_OR_S       0x000a0000
#define   MACH64_B5_FRGD_MIX_D_OR_S           0x000b0000
#define   MACH64_B5_FRGD_MIX_D_AND_S          0x000c0000
#define   MACH64_B5_FRGD_MIX_NOT_D_AND_S      0x000d0000
#define   MACH64_B5_FRGD_MIX_D_AND_NOT_S      0x000e0000
#define   MACH64_B5_FRGD_MIX_NOT_D_AND_NOT_S  0x000f0000
#define   MACH64_B5_FRGD_MIX_D_PLUS_S_DIV2    0x00170000
#define MACH64_DP_SRC                         MM(0xB6)
#define   MACH64_B6_DP_BKGD_SRCMask           0x00000007
#define   MACH64_B6_DP_BKGD_SRCShift                   0
#define   MACH64_B6_BKGD_SRC_BKGD_CLR         0x00000000
#define   MACH64_B6_BKGD_SRC_FRGD_CLR         0x00000001
#define   MACH64_B6_BKGD_SRC_HOST             0x00000002
#define   MACH64_B6_BKGD_SRC_BLIT             0x00000003
#define   MACH64_B6_BKGD_SRC_PATTERN          0x00000004
#define   MACH64_B6_BKGD_SRC_3D               0x00000005
#define   MACH64_B6_DP_FRGD_SRCMask           0x00000700
#define   MACH64_B6_DP_FRGD_SRCShift                   8
#define   MACH64_B6_FRGD_SRC_BKGD_CLR         0x00000000
#define   MACH64_B6_FRGD_SRC_FRGD_CLR         0x00000100
#define   MACH64_B6_FRGD_SRC_HOST             0x00000200
#define   MACH64_B6_FRGD_SRC_BLIT             0x00000300
#define   MACH64_B6_FRGD_SRC_PATTERN          0x00000400
#define   MACH64_B6_FRGD_SRC_3D               0x00000500
#define   MACH64_B6_DP_MONO_SRCMask           0x00030000
#define   MACH64_B6_DP_MONO_SRCShift                  16
#define   MACH64_B6_MONO_SRC_ONE              0x00000000
#define   MACH64_B6_MONO_SRC_PATTERN          0x00010000
#define   MACH64_B6_MONO_SRC_HOST             0x00020000
#define   MACH64_B6_MONO_SRC_BLIT             0x00030000
#define MACH64_DP_FRGD_CLR_MIX                MM(0xB7)
#define   MACH64_B7_DP_FRGD_CLRMask           0x0000ffff
#define   MACH64_B7_DP_FRGD_CLRShift                   0
#define   MACH64_B7_DP_FRGD_MIXMask           0x001f0000
#define   MACH64_B7_DP_FRGD_MIXShift                  16
#define   MACH64_B7_DP_BKGD_MIXMask           0x1f000000
#define   MACH64_B7_DP_BKGD_MIXShift                  24
#define MACH64_DP_FRGD_BKGD_CLR               MM(0xB8)
#define   MACH64_B8_DP_FRGD_CLRMask           0x0000ffff
#define   MACH64_B8_DP_FRGD_CRLShift                   0
#define   MACH64_B8_DP_BKGD_CLRMask           0xffff0000
#define   MACH64_B8_DP_BKGD_CLRShift                  16

/* Draw Engine Destination Trajectory */
#define MACH64_DST_X_Y                        MM(0xBA)
#define   MACH64_BA_DST_XMask                 0x00001fff
#define   MACH64_BA_DST_XShift                         0
#define   MACH64_BA_DST_YMask                 0x1fff0000
#define   MACH64_BA_DST_YShift                        16
#define MACH64_DST_WIDTH_HEIGHT               MM(0xBB)
#define   MACH64_BB_DST_WIDTHMask             0x00001fff
#define   MACH64_BB_DST_WIDTHShift                     0
#define   MACH64_BB_DST_HEIGHTMask            0xffff0000
#define   MACH64_BB_DST_HEIGHTShift                   16

/* Data Path */
#define MACH64_DP_SET_GUI_ENGINE              MM(0xBF)
#define   MACH64_BF_SET_DP_DST_PIX_WIDTHMask  0x00000038
#define   MACH64_BF_SET_DP_DST_PIX_WIDTHShift          3
#define     MACH64_BF_SET_DP_DST_PIX_WIDTH1   0x00000000
#define     MACH64_BF_SET_DP_DST_PIX_WIDTH8   0x00000010
#define     MACH64_BF_SET_DP_DST_PIX_WIDTH15  0x00000018
#define     MACH64_BF_SET_DP_DST_PIX_WIDTH16  0x00000020
#define     MACH64_BF_SET_DP_DST_PIX_WIDTH32  0x00000030
#define   MACH64_BF_SET_DP_SRC_PIX_WIDTH      0x00000040
#define   MACH64_BF_SET_DST_OFFSETMask        0x00000380
#define   MACH64_BF_SET_DST_OFFSETShift                7
#define   MACH64_BF_SET_DST_PITCHMask         0x00003c00
#define   MACH64_BF_SET_DST_PITCHShift                10
#define   MACH64_BF_SET_DST_PITCH_BY_2        0x00004000
#define   MACH64_BF_SET_SRC_OFFPITCH_COPY     0x00008000
#define   MACH64_BF_SET_SRC_HGTWID1_2Mask     0x00030000
#define   MACH64_BF_SET_SRC_HGTWID1_2Shift            16
#define   MACH64_BF_SET_DRAWING_COMBOMask     0x00f00000
#define   MACH64_BF_SET_DRAWING_COMBOShift            20
#define   MACH64_BF_SET_BUS_MASTER_OPMask     0x03000000
#define   MACH64_BF_SET_BUS_MASTER_OPShift            24
#define   MACH64_BF_SET_BUS_MASTER_EN         0x04000000
#define   MACH64_BF_SET_BUS_MASTER_SYNC       0x08000000
#define   MACH64_BF_DP_HOST_TRIPLE_EN         0x10000000
#define   MACH64_BF_SET_FAST_FILL_EN          0x20000000
#define   MACH64_BF_BLOCK_WRITE_ENA           0x40000000

/* Color Compare */
#define MACH64_CLR_CMP_CLR                    MM(0xC0)
#define MACH64_CLR_CMP_MSK                    MM(0xC1)
#define MACH64_CLR_CMP_CNTL                   MM(0xC2)
#define   MACH64_C2_CLR_CMP_FCNMask           0x00000007
#define   MACH64_C2_CLR_CMP_FCNShift                   0
#define   MACH64_C2_CLR_CMP_SRCMask           0x03000000
#define   MACH64_C2_CLR_CMP_SRCShift                  24

/* Command FIFO */
#define MACH64_FIFO_STAT                      MM(0xC4)
#define   MACH64_C4_FIFO_STATMask             0x0000ffff

/* Engine Control */
#define MACH64_GUI_TRAJ_CNTL                  MM(0xCC)
#define   MACH64_CC_DST_X_DIR                 0x00000001
#define   MACH64_CC_DST_X_RIGHT_TO_LEFT       0x00000000
#define   MACH64_CC_DST_X_LEFT_TO_RIGHT       0x00000001
#define   MACH64_CC_DST_Y_DIR                 0x00000002
#define   MACH64_CC_DST_Y_BOTTOM_TO_TOP       0x00000000
#define   MACH64_CC_DST_Y_TOP_TO_BOTTOM       0x00000002
#define   MACH64_CC_DST_Y_MAJOR               0x00000004
#define   MACH64_CC_DST_X_TILE                0x00000008
#define   MACH64_CC_DST_Y_TILE                0x00000010
#define   MACH64_CC_DST_LAST_PEL              0x00000020
#define   MACH64_CC_DST_POLYGON_EN            0x00000040
#define   MACH64_CC_DST_24_ROT_EN             0x00000080
#define   MACH64_CC_DST_24_ROTMask            0x00000700
#define   MACH64_CC_DST_24_ROTShift                    8
#define   MACH64_CC_DST_BRES_ZERO             0x00000800
#define   MACH64_CC_DST_POLYGON_RTEDGE_DIS    0x00001000
#define   MACH64_CC_TRAIL_X_DIR               0x00002000
#define   MACH64_CC_TRAP_FILL_DIR             0x00004000
#define   MACH64_CC_TRAIL_BRES_SIGN           0x00008000
#define   MACH64_CC_SRC_PATT_EN               0x00010000
#define   MACH64_CC_SRC_PATT_ROT_EN           0x00020000
#define   MACH64_CC_SRC_LINEAR_EN             0x00040000
#define   MACH64_CC_SRC_BYTE_ALIGN            0x00080000
#define   MACH64_CC_SRC_LINE_X_DIR            0x00100000
#define   MACH64_CC_SRC_8x8x8_BRUSH           0x00200000
#define   MACH64_CC_FAST_FILL_EN              0x00400000
#define   MACH64_CC_SRC_TRACK_DST             0x00800000
#define   MACH64_CC_PAT_MONO_EN               0x01000000
#define   MACH64_CC_PAT_CLR_4x2_EN            0x02000000
#define   MACH64_CC_PAT_CLR_8x1_EN            0x04000000
#define   MACH64_CC_HOST_BYTE_ALIGN           0x10000000
#define   MACH64_CC_HOST_BIG_ENDIAN_EN        0x20000000

/* Engine Status */
#define MACH64_GUI_STAT                       MM(0xCE)
#define   MACH64_CE_GUI_ACTIVE                0x00000001
#define   MACH64_CE_DSTX_LT_SCISSOR_LEFT      0x00000100
#define   MACH64_CE_DSTX_GT_SCISSOR_RIGHT     0x00000200
#define   MACH64_CE_DSTY_LT_SCISSOR_TOP       0x00000400
#define   MACH64_CE_DSTY_GT_SCISSOR_BOTTOM    0x00000800
#define   MACH64_CE_GUI_FIFOMask              0x00ff0000
#define   MACH64_CE_GUI_FIFOShift                     16

/* System Bus Mastering */
#define MACH64_BM_FRAME_BUF_OFFSET            MM(0x160)
#define   MACH64_160_FRAME_BUF_OFFSETMask     0x00ffffff
#define   MACH64_160_FRAME_BUF_OFFSETShift             0
#define MACH64_BM_SYSTEM_MEM_ADDR             MM(0x161)
#define MACH64_BM_COMMAND                     MM(0x162)
#define   MACH64_162_BYTE_COUNTMask           0x00001fff
#define   MACH64_162_BYTE_COUNTShift                   0
#define   MACH64_162_FRAME_OFFSET_HOLD        0x40000000
#define   MACH64_162_END_OF_LIST_STATUS       0x80000000
#define MACH64_BM_STATUS                      MM(0x163)
#define MACH64_BM_SYSTEM_TABLE                MM(0x16F)
#define   MACH64_16F_SYSTEM_TRIGGERMask       0x00000007
#define   MACH64_16F_SYSTEM_TRIGGERShift               0
#define   MACH64_16F_STYSTEM_TABLE_ADDRMask   0xfffffff0
#define MACH64_BM_GUI_TABLE                   MM(0x16E)

/* These are not in MM format because they are never written directly */
#define MACH64_BM_HOST_DATA                   0x244
#define MACH64_BM_ADDR                        0x248
#define   MACH64_92_GUIREG_ADDRMask           0x000000ff
#define   MACH64_92_GUIREG_ADDRShift                   0
#define   MACH64_92_GUIREG_COUNTERMask        0x001f0000
#define   MACH64_92_GUIREG_COUNTERShift               16
#define MACH64_BM_DATA                        0x248

#define MACH64_GUI_CMDFIFO_DATA      0x174
#define MACH64_GUI_CNTL                       MM(0x5E)
#define   MACH64_5E_CMDFIFO_SIZE_MODEMask     0x00000003
#define   MACH64_5E_CMDFIFO_SIZE_MODEShift             0

/* 3D registers
   Taken from the DRI source, Copyright 2000 Gareth Hughes
*/
#define MACH64_ALPHA_TST_CNTL                 MM(0x54)
#define   MACH64_54_ALPHA_TEST_EN             0x00000001
#define   MACH64_54_ALPHA_TESTMask            0x00000070
#define   MACH64_54_ALPHA_TESTShift                    4
#define   MACH64_54_ALPHA_TEST_NEVER          0x00000000
#define   MACH64_54_ALPHA_TEST_LESS           0x00000010
#define   MACH64_54_ALPHA_TEST_LEQUAL         0x00000020
#define   MACH64_54_ALPHA_TEST_EQUAL          0x00000030
#define   MACH64_54_ALPHA_TEST_GEQUAL         0x00000040
#define   MACH64_54_ALPHA_TEST_GREATER        0x00000050
#define   MACH64_54_ALPHA_TEST_NOTEQUAL       0x00000060
#define   MACH64_54_ALPHA_TEST_ALWAYS         0x00000070
#define   MACH64_54_ALPHA_MOD_MSB             0x00000080
#define   MACH64_54_ALPHA_DSTMask             0x00000700
#define   MACH64_54_ALPHA_DSTShift                     8
#define   MACH64_54_ALPHA_DST_ZERO            0x00000000
#define   MACH64_54_ALPHA_DST_ONE             0x00000100
#define   MACH64_54_ALPHA_DST_SRCALPHA        0x00000400
#define   MACH64_54_ALPHA_DST_INVSRCALPHA     0x00000500
#define   MACH64_54_ALPHA_DST_DSTALPHA        0x00000600
#define   MACH64_54_ALPHA_DST_INVDSTALPHA     0x00000700
#define   MACH64_54_ALPHA_TST_SRC_TEXEL       0x00000000
#define   MACH64_54_ALPHA_TST_SRC_SRCALPHA    0x00001000
#define   MACH64_54_REF_ALPHAMask             0x00ff0000
#define   MACH64_54_REF_ALPHAShift                    16
#define   MACH64_54_COMPOSITE_SHADOW          0x40000000
#define   MACH64_54_SPECULAR_LIGHT_EN         0x80000000

#define MACH64_ONE_OVER_AREA                  MM(0x1A7) /* 0x197, 0x19F */
#define MACH64_ONE_OVER_AREA_UC               MM(0x1C0)

#define MACH64_SCALE_3D_CNTL                       MM(0x7F)
#define   MACH64_7F_SCALE_PIX_EXPAND_ZERO_EXTEND   0x00000000
#define   MACH64_7F_SCALE_PIX_EXPAND_DYNAMIC_RANGE 0x00000001
#define   MACH64_7F_SCALE_DITHER_ERROR_DIFFUSE     0x00000000
#define   MACH64_7F_SCALE_DITHER_2D_TABLE          0x00000002
#define   MACH64_7F_DITHER_EN                      0x00000004
#define   MACH64_7F_DITHER_INIT_CURRENT            0x00000000
#define   MACH64_7F_DITHER_INIT_RESET              0x00000008
#define   MACH64_7F_ROUND_EN                       0x00000010
#define   MACH64_7F_TEX_CACHE_DIS                  0x00000020
#define   MACH64_7F_SCALE_3D_FCNMask               0x000000c0
#define   MACH64_7F_SCALE_3D_FCN_NOP               0x00000000
#define   MACH64_7F_SCALE_3D_FCN_SCALE             0x00000040
#define   MACH64_7F_SCALE_3D_FCN_TEXTURE           0x00000080
#define   MACH64_7F_SCALE_3D_FCN_SHADE             0x000000c0
#define   MACH64_7F_TEXTURE_DISABLE                0x00000040
#define   MACH64_7F_EDGE_ANTI_ALIAS                0x00000100
#define   MACH64_7F_TEX_CACHE_SPLIT                0x00000200
#define   MACH64_7F_APPLE_YUV_MODE                 0x00000400
#define   MACH64_7F_ALPHA_FOG_ENMask               0x00001800
#define   MACH64_7F_ALPHA_FOG_DIS                  0x00000000
#define   MACH64_7F_ALPHA_FOG_EN_ALPHA             0x00000800
#define   MACH64_7F_ALPHA_FOG_EN_FOG               0x00001000
#define   MACH64_7F_ALPHA_BLEND_SAT                0x00002000
#define   MACH64_7F_RED_DITHER_MAX                 0x00004000
#define   MACH64_7F_SIGNED_DST_CLAMP               0x00008000
#define   MACH64_7F_ALPHA_BLEND_SRCMask            0x00070000
#define   MACH64_7F_ALPHA_BLEND_SRC_ZERO           0x00000000
#define   MACH64_7F_ALPHA_BLEND_SRC_ONE            0x00010000
#define   MACH64_7F_ALPHA_BLEND_SRC_DSTCOLOR       0x00020000
#define   MACH64_7F_ALPHA_BLEND_SRC_INVDSTCOLOR    0x00030000
#define   MACH64_7F_ALPHA_BLEND_SRC_SRCALPHA       0x00040000
#define   MACH64_7F_ALPHA_BLEND_SRC_INVSRCALPHA    0x00050000
#define   MACH64_7F_ALPHA_BLEND_SRC_DSTALPHA       0x00060000
#define   MACH64_7F_ALPHA_BLEND_SRC_INVDSTALPHA    0x00070000
#define   MACH64_7F_ALPHA_BLEND_DSTMask            0x00380000
#define   MACH64_7F_ALPHA_BLEND_DST_ZERO           0x00000000
#define   MACH64_7F_ALPHA_BLEND_DST_ONE            0x00080000
#define   MACH64_7F_ALPHA_BLEND_DST_SRCCOLOR       0x00100000
#define   MACH64_7F_ALPHA_BLEND_DST_INVSRCCOLOR    0x00180000
#define   MACH64_7F_ALPHA_BLEND_DST_SRCALPHA       0x00200000
#define   MACH64_7F_ALPHA_BLEND_DST_INVSRCALPHA    0x00280000
#define   MACH64_7F_ALPHA_BLEND_DST_DSTALPHA       0x00300000
#define   MACH64_7F_ALPHA_BLEND_DST_INVDSTALPHA    0x00380000
#define   MACH64_7F_TEX_LIGHT_FCNMask              0x00c00000
#define   MACH64_7F_TEX_LIGHT_FCN_REPLACE          0x00000000
#define   MACH64_7F_TEX_LIGHT_FCN_MODULATE         0x00400000
#define   MACH64_7F_TEX_LIGHT_FCN_ALPHA_DECAL      0x00800000
#define   MACH64_7F_MIP_MAP_DISABLE                0x01000000
#define   MACH64_7F_BILINEAR_TEX_EN                0x02000000
#define   MACH64_7F_TEX_BLEND_FCNMask              0x0c000000
#define   MACH64_7F_TEX_BLEND_FCN_NEAREST          0x00000000
#define   MACH64_7F_TEX_BLEND_FCN_LINEAR           0x08000000
#define   MACH64_7F_TEX_BLEND_FCN_TRILINEAR        0x0c000000
#define   MACH64_7F_TEX_AMASK_AEN                  0x10000000
#define   MACH64_7F_TEX_AMASK_BLEND_EDGE           0x20000000
#define   MACH64_7F_TEX_MAP_AEN                    0x40000000
#define   MACH64_7F_SRC_3D_HOST_FIFO               0x80000000

#define MACH64_SECONDARY_TEX_OFFSET                MM(0xDE)

#define MACH64_SETUP_CNTL                          MM(0x1C1)
#define   MACH64_1C1_DONT_START_TRI                0x00000000
#define   MACH64_1C1_DONT_START_ANY                0x00000004
#define   MACH64_1C1_FLAT_SHADEMask                0x00000018
#define   MACH64_1C1_FLAT_SHADE_OFF                0x00000000
#define   MACH64_1C1_FLAT_SHADE_VERTEX_1           0x00000008
#define   MACH64_1C1_FLAT_SHADE_VERTEX_2           0x00000010
#define   MACH64_1C1_FLAT_SHADE_VERTEX_3           0x00000018
#define   MACH64_1C1_SOLID_MODE_OFF                0x00000000
#define   MACH64_1C1_SOLID_MODE_ON                 0x00000020
#define   MACH64_1C1_LOG_MAX_INC_ADJ               0x00000040
#define   MACH64_1C1_SET_UP_CONTINUE               0x80000000

#define MACH64_TEX_0_OFF                           MM(0x70)
#define MACH64_TEX_1_OFF                           MM(0x71)
#define MACH64_TEX_2_OFF                           MM(0x72)
#define MACH64_TEX_3_OFF                           MM(0x73)
#define MACH64_TEX_4_OFF                           MM(0x74)
#define MACH64_TEX_5_OFF                           MM(0x75)
#define MACH64_TEX_6_OFF                           MM(0x76)
#define MACH64_TEX_7_OFF                           MM(0x77)
#define MACH64_TEX_8_OFF                           MM(0x78)
#define MACH64_TEX_9_OFF                           MM(0x79)
#define MACH64_TEX_10_OFF                          MM(0x7A)

#define MACH64_TEX_CNTL                            MM(0xDD)
#define   MACH64_DD_LOD_BIASMask                   0x0000000f
#define   MACH64_DD_LOD_BIAS_SHIFT                          0
#define   MACH64_DD_COMP_FACTOR_MASK               0x000000f0
#define   MACH64_DD_COMP_FACTOR_SHIFT                       4
#define   MACH64_DD_TEXTURE_COMPOSITE              0x00000100
#define   MACH64_DD_COMP_COMBINE_BLEND             0x00000000
#define   MACH64_DD_COMP_COMBINE_MODULATE          0x00000200
#define   MACH64_DD_COMP_BLEND_NEAREST             0x00000000
#define   MACH64_DD_COMP_BLEND_BILINEAR            0x00000800
#define   MACH64_DD_COMP_FILTER_NEAREST            0x00000000
#define   MACH64_DD_COMP_FILTER_BILINEAR           0x00001000
#define   MACH64_DD_COMP_ALPHA                     0x00002000
#define   MACH64_DD_TEXTURE_TILING                 0x00004000
#define   MACH64_DD_COMPOSITE_TEX_TILING           0x00008000
#define   MACH64_DD_TEX_COLLISION_DISABLE          0x00010000
#define   MACH64_DD_TEXTURE_CLAMP_S                0x00020000
#define   MACH64_DD_TEXTURE_CLAMP_T                0x00040000
#define   MACH64_DD_TEX_ST_MULT_W                  0x00000000
#define   MACH64_DD_TEX_ST_DIRECT                  0x00080000
#define   MACH64_DD_TEX_SRC_LOCAL                  0x00000000
#define   MACH64_DD_TEX_SRC_AGP                    0x00100000
#define   MACH64_DD_TEX_UNCOMPRESSED               0x00000000
#define   MACH64_DD_TEX_VQ_COMPRESSED              0x00200000
#define   MACH64_DD_COMP_TEX_UNCOMPRESSED          0x00000000
#define   MACH64_DD_COMP_TEX_VQ_COMPRESSED         0x00400000
#define   MACH64_DD_TEX_CACHE_FLUSH                0x00800000
#define   MACH64_DD_SEC_TEX_CLAMP_S                0x01000000
#define   MACH64_DD_SEC_TEX_CLAMP_T                0x02000000
#define   MACH64_DD_TEX_WRAP_S                     0x10000000
#define   MACH64_DD_TEX_WRAP_T                     0x20000000
#define   MACH64_DD_TEX_CACHE_SIZE_4K              0x40000000 
#define   MACH64_DD_TEX_CACHE_SIZE_2K              0x40000000 /* ??? */
#define   MACH64_DD_SECONDARY_STW                  0x80000000

#define MACH64_TEX_PALETTE                         MM(0xDF)
#define MACH64_TEX_PALETTE_INDEX                   MM(0xD0)
#define MACH64_TEX_SIZE_PITCH                      MM(0xDC)

#define MACH64_VERTEX_1_ARGB			   MM(0x195) /* 0x1BA */
#define MACH64_VERTEX_1_S			   MM(0x190) /* 0x1AB */
#define MACH64_VERTEX_1_SECONDARY_S		   MM(0x1CA)
#define MACH64_VERTEX_1_SECONDARY_T		   MM(0x1CB)
#define MACH64_VERTEX_1_SECONDARY_W		   MM(0x1CC)
#define MACH64_VERTEX_1_SPEC_ARGB		   MM(0x193) /* 0x1B4 */
#define MACH64_VERTEX_1_T			   MM(0x191) /* 0x1AC */
#define MACH64_VERTEX_1_W			   MM(0x192) /* 0x1AD */
#define MACH64_VERTEX_1_X_Y			   MM(0x196) /* 0x1BD */
#define MACH64_VERTEX_1_Z			   MM(0x194) /* 0x1B7 */
#define MACH64_VERTEX_2_ARGB			   MM(0x19D) /* 0x1BB */
#define MACH64_VERTEX_2_S			   MM(0x198) /* 0x1AE */
#define MACH64_VERTEX_2_SECONDARY_S		   MM(0x1CD)
#define MACH64_VERTEX_2_SECONDARY_T		   MM(0x1CE)
#define MACH64_VERTEX_2_SECONDARY_W		   MM(0x1CF)
#define MACH64_VERTEX_2_SPEC_ARGB		   MM(0x19B) /* 0x1B5 */
#define MACH64_VERTEX_2_T			   MM(0x199) /* 0x1AF */
#define MACH64_VERTEX_2_W			   MM(0x19A) /* 0x1B0 */
#define MACH64_VERTEX_2_X_Y			   MM(0x19E) /* 0x1BE */
#define MACH64_VERTEX_2_Z			   MM(0x19C) /* 0x1B8 */
#define MACH64_VERTEX_3_ARGB			   MM(0x1A5) /* 0x1BC */
#define MACH64_VERTEX_3_S			   MM(0x1A0) /* 0x1B1 */
#define MACH64_VERTEX_3_SECONDARY_S		0x02a0 ???
#define MACH64_VERTEX_3_SECONDARY_T		0x02a4 ???
#define MACH64_VERTEX_3_SECONDARY_W		0x02a8 ???
#define MACH64_VERTEX_3_SPEC_ARGB		   MM(0x1A3) /* 0x1B6 */
#define MACH64_VERTEX_3_T			   MM(0x1A1) /* 0x1B2 */
#define MACH64_VERTEX_3_W			   MM(0x1A2) /* 0x1B3 */
#define MACH64_VERTEX_3_X_Y			   MM(0x1A6) /* 0x1BF */
#define MACH64_VERTEX_3_Z			   MM(0x1A4) /* 0x1B9 */

#define MACH64_Z_CNTL                              MM(0x53)
#define   MACH64_53_Z_EN                           0x00000001
#define   MACH64_53_Z_SRC_2D                       0x00000002
#define   MACH64_53_Z_TESTMask                     0x00000070
#define   MACH64_53_Z_TEST_NEVER                   0x00000000
#define   MACH64_53_Z_TEST_LESS                    0x00000010
#define   MACH64_53_Z_TEST_LEQUAL                  0x00000020
#define   MACH64_53_Z_TEST_EQUAL                   0x00000030
#define   MACH64_53_Z_TEST_GEQUAL                  0x00000040
#define   MACH64_53_Z_TEST_GREATER                 0x00000050
#define   MACH64_53_Z_TEST_NOTEQUAL                0x00000060
#define   MACH64_53_Z_TEST_ALWAYS                  0x00000070
#define   MACH64_53_Z_MASK_EN                      0x00000100

#define MACH64_Z_OFF_PITCH                         MM(0x52)
#define   MACH64_52_Z_OFFSETMask                   0x000fffff
#define   MACH64_52_Z_OFFSETShift                           0
#define   MACH64_52_Z_PITCHMask                    0xffc00000
#define   MACH64_52_Z_PITCHShift                           22

/* Clock Registers */

#define MACH64_MPLL_CNTL                                        0x00
#define   MACH64_CLOCK00_MPLL_PC_GAINMask                       0x07
#define   MACH64_CLOCK00_MPLL_PC_GAINShift                         0
#define   MACH64_CLOCK00_MPLL_VC_GAINMask                       0x18
#define   MACH64_CLOCK00_MPLL_VC_GAINShift                         3
#define   MACH64_CLOCK00_MPLL_D_CYCMask                         0x60
#define   MACH64_CLOCK00_MPLL_C_CYCShift                           5
#define   MACH64_CLOCK00_MPLL_RANGE                             0x80
#define MACH64_VPLL_CNTL         0x01
#define   MACH64_CLOCK01_VPLL_PC_GAINMask                       0x07
#define   MACH64_CLOCK01_VPLL_PC_GAINShift                         0
#define   MACH64_CLOCK01_VPLL_VC_GAINMask                       0x18
#define   MACH64_CLOCK01_VPLL_VC_GAINShift                         3
#define   MACH64_CLOCK01_VPLL_D_CYCMask                         0x60
#define   MACH64_CLOCK01_VPLL_C_CYCShift                           5
#define   MACH64_CLOCK01_VPLL_RANGE                             0x80
#define MACH64_PLL_REF_DIV                                      0x02
#define MACH64_PLL_GEN_CNTL                                     0x03
#define   MACH64_CLOCK02_PLL_SLEEP                              0x01
#define   MACH64_CLOCK02_PLL_MRESET                             0x02
#define   MACH64_CLOCK02_OSC_EN                                 0x04
#define   MACH64_CLOCK02_EXT_CLK_EN                             0x08
#define   MACH64_CLOCK02_MCLK_SRC_SELMask                       0x70
#define   MACH64_CLOCK02_MCLK_SRC_SELShift                         4
#define   MACH64_CLOCK02_DLL_PWDN                               0x80
#define MACH64_MCLK_FB_DIV                                      0x04
#define MACH64_PLL_VCLK_CNTL                                    0x05
#define   MACH64_CLOCK05_VCLK_SRC_SELMask                       0x03
#define   MACH64_CLOCK05_VCLK_SRC_SELShift                         0
#define   MACH64_CLOCK05_PLL_PRESET                             0x04
#define   MACH64_CLOCK05_VCLK_INVERT                            0x08
#define   MACH64_CLOCK05_ECP_DIVMask                            0x30
#define   MACH64_CLOCK05_ECP_DIVShift                              4
#define MACH64_VCLK_POST_DIV                                    0x06
#define   MACH64_CLOCK06_VCLK0_POSTMask                         0x03
#define   MACH64_CLOCK06_VCLK0_POSTShift                           0
#define   MACH64_CLOCK06_VCLK1_POSTMask                         0x0c
#define   MACH64_CLOCK06_VCLK1_POSTShift                           2
#define   MACH64_CLOCK06_VCLK2_POSTMask                         0x30
#define   MACH64_CLOCK06_VCLK2_POSTShift                           4
#define   MACH64_CLOCK06_VCLK3_POSTMask                         0xc0
#define   MACH64_CLOCK06_VCLK3_POSTShift                           6
#define MACH64_VCLK0_FB_DIV                                     0x07
#define MACH64_VCLK1_FB_DIV                                     0x08
#define MACH64_VCLK2_FB_DIV                                     0x09
#define MACH64_VCLK3_FB_DIV                                     0x0a
#define MACH64_PLL_EXT_CNTL                                     0x0b
#define   MACH64_CLOCK0B_XCLK_SRC_SELMask                       0x07
#define   MACH64_CLOCK0B_XCLK_SRC_SELShift                         0
#define   MACH64_CLOCK0B_MFB_TIMES_4_2                          0x08
#define   MACH64_CLOCK0B_ALT_VCLK0_POST                         0x10
#define   MACH64_CLOCK0B_ALT_VCLK1_POST                         0x20
#define   MACH64_CLOCK0B_ALT_VCLK2_POST                         0x40
#define   MACH64_CLOCK0B_ALT_VCLK3_POST                         0x80
#define MACH64_DLL_CNTL                                         0x0c

/* LCD Registers */

#define MACH64_LCD_GEN_CNTL                                 0x01
#define   MACH64_LCD01_CRT_ON                               0x00000001
#define   MACH64_LCD01_LCD_ON                               0x00000002
#define   MACH64_LCD01_HORZ_DIVBY2_EN                       0x00000004
#define   MACH64_LCD01_DONT_DS_ICON                         0x00000008
#define   MACH64_LCD01_LOCK_8DOT                            0x00000010
#define   MACH64_LCD01_ICON_ENABLE                          0x00000020
#define   MACH64_LCD01_DONT_SHADOW_VPAR                     0x00000040
#define   MACH64_LCD01_V2CLK_ALWAYS_ON                      0x00000080
#define   MACH64_LCD01_RST_FM                               0x00000100
#define   MACH64_LCD01_DISABLE_PCLK_RESET                   0x00000200
#define   MACH64_LCD01_DIS_HOR_CRT_DIVBY2                   0x00000400
#define   MACH64_LCD01_SCLK_SEL                             0x00000800
#define   MACH64_LCD01_SCLK_DELAYMask                       0x0000f000
#define   MACH64_LCD01_SCLK_DELAYShift                              12
#define   MACH64_LCD01_TVCLK_ALWAYS_ON                      0x00010000
#define   MACH64_LCD01_VCLK_DAC_ALWAYS_ON                   0x00020000
#define   MACH64_LCD01_VCLK_LCD_OFF                         0x00040000
#define   MACH64_LCD01_XTALIN_ALWAYS_ON                     0x00080000
#define   MACH64_LCD01_V2CLK_DAC_ALWAYS_ON                  0x00100000
#define   MACH64_LCD01_LVDS_EN                              0x00200000
#define   MACH64_LCD01_LVDS_PLL_EN                          0x00400000
#define   MACH64_LCD01_LVDS_PLL_RESET                       0x00800000
#define   MACH64_LCD01_LVDS_RESEVED_BITSMask                0x07000000
#define   MACH64_LCD01_CRTC_RW_SELECT                       0x08000000
#define   MACH64_LCD01_USE_SHADOWED_VEND                    0x10000000
#define   MACH64_LCD01_USE_SHADOWED_ROWCUR                  0x20000000
#define   MACH64_LCD01_SHADOW_EN                            0x40000000
#define   MACH64_LCD01_SHADOW_RW_EN                         0x80000000
#define MACH64_HORZ_STRETCHING                              0x04
#define   MACH64_LCD04_HORZ_STRETCH_RATIOMask               0x0000ffff
#define   MACH64_LCD04_HORZ_STRETCH_RATIOShift                       0
#define   MACH64_LCD04_LOOP_STRETCHMask                     0x00070000
#define   MACH64_LCD04_LOOP_STRETCHShfit                            16
#define   MACH64_LCD04_HORZ_PANEL_SIZEMask                  0x0ff00000
#define   MACH64_LCD04_HORZ_PANEL_SIZEShift                         20
#define   MACH64_LCD04_AUTO_HORZ_RATIO                      0x20000000
#define   MACH64_LCD04_HORZ_STRETCH_MODE                    0x40000000
#define   MACH64_LCD04_HORZ_STRETCH_EN                      0x80000000
#define MACH64_VERT_STRETCHING                              0x05
#define   MACH64_LCD05_VERT_STRETCH_RATIO0Mask              0x000003ff
#define   MACH64_LCD05_VERT_STRETCH_RATIO0Shift                      0
#define   MACH64_LCD05_VERT_STRETCH_RATIO1Mask              0x000ffc00
#define   MACH64_LCD05_VERT_STRETCH_RATIO1Shift                     10
#define   MACH64_LCD05_VERT_STRETCH_RATIO2Mask              0x3ff00000
#define   MACH64_LCD05_VERT_STRETCH_RATIO2Shift                     20
#define   MACH64_LCD05_USE_RATIO0                           0x40000000
#define   MACH64_LCD05_VERT_STRETCH_EN                      0x80000000
#define MACH64_EXT_VERT_STRETCH                             0x06
#define   MACH64_LCD06_VERT_STRETCH_RATIO3Mask              0x000003ff
#define   MACH64_LCD06_VERT_STRETCH_RATIO3Shift                      0
#define   MACH64_LCD06_VERT_STRETCH_MODE                    0x00000400
#define   MACH64_LCD06_VERT_PANEL_SIZEMask                  0x003ff800
#define   MACH64_LCD06_VERT_PANEL_SIZEShift                         11
#define   MACH64_LCD06_AUTO_VERT_RATIO                      0x00400000
#define   MACH64_LCD06_USE_AUTO_FP_POS                      0x00800000
#define   MACH64_LCD06_USE_AUTO_LCD_VSYNC                   0x01000000

/* RAMDAC Registers */
#define MACH64_DAC_REGS                       MM(0x30)
#define   MACH64_DAC_W_INDEX                  0
#define   MACH64_DAC_DATA                     1
#define   MACH64_DAC_MASK                     2
#define   MACH64_DAC_R_INDEX                  3


#endif	/* #ifdef _chipset_ATI_MACH64_h	*/
