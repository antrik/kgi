/* ----------------------------------------------------------------------------
**	S3 virge register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Jon Taylor
**	Copyright (C)	2000		Jos Hulzink
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: ViRGE.h,v $
**	Revision 1.2  2003/01/30 13:44:45  foske
**	Mayor cleanup, fix textmode, drop secondary card support, it wasn't going to work anyway.
**	
**	Revision 1.1.1.1  2000/08/04 11:39:08  aldot
**	import of kgi-0.9 20020321
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
#ifndef	_chipset_S3_virge_h
#define	_chipset_S3_virge_h

/*	PCI config region
**
**	Please use the pci.h definitions for PCICFG register names.
*/
#ifndef	PCI_VENDOR_ID_S3
#define	PCI_VENDOR_ID_S3		0x5333
#endif
#ifndef	PCI_DEVICE_ID_VIRGE
#define	PCI_DEVICE_ID_VIRGE		0x5631
#endif

#define MC_INTERLACED			CR42_INTERLACED
#define MC_NONINTERLACED		0;



/* GC control region (Base0) */
#define	VIRGE_Base0_LittleEndianBase	0x00000
#define	VIRGE_Base0_BigEndianBase		0x10000
#define	VIRGE_Base0_Size			0x02000000

/* Register subtype I/O regions */

/* MMIO VGA */
#define	VIRGE_VGA_BASE		0x8000
#define	VIRGE_VGA_SIZE		0x0100

#include "chipset/IBM/VGA.h"

/* Registers */

#define ViRGE_NR_CRT_REGS	0x70
#define ViRGE_NR_GRC_REGS	0x09
#define ViRGE_NR_SEQ_REGS	0x1D
#define ViRGE_NR_ATC_REGS	0x15

/* MISC */
#undef	MISC_VIDEO_OFF
#define MISC_EXT_CLOCK			0x0C

/* CRT REGISTERS */

/*
**	CRT2D - Device ID high register
*/
#define CR2D_ViRGE_HIGH_ID		0x56
#define CR2D_ViRGE_VX_HIGH_ID		0x88
#define CR2D_ViRGE_DX_GX_HIGH_ID	0x8A
#define CR2D_ViRGE_GX2_HIGH_ID		0x8A
#define CR2D_ViRGE_MX_HIGH_ID		0x8C

/*
**	CRT2E - Device ID low register
*/
#define CR2E_ViRGE_LOW_ID		0x31
#define CR2E_ViRGE_VX_LOW_ID		0x3D
#define CR2E_ViRGE_DX_GX_LOW_ID		0x01
#define CR2E_ViRGE_GX2_LOW_ID		0x10
#define CR2E_ViRGE_MX_LOW_ID		0x01

/*
**	CRT2F - Revision register
*/
#define CRT_REVISION			0x2F

/*
**	CRT30 - Chip ID/rev register
*/
#define CRT_CHIPID			0x30
#define CR30_USE_CR2x			0xE1

/*
**	CRT31 - Memory configuration
*/
#define CR31_HISPEEDTXT			0x40
#define CR31_CPUABASE			0x01
#define CR31_ENHANCED			0x08
#define CR31_2PAGE_SCRN			0x02
#define CR31_VGA16B			0x04
/*
**	CRT32 - Backward compatibility 1
*/


/*
**	CRT33 - Backward Compatibility 2
*/
#define CR33_UNLOCK_VDE			0x02
#define CR33_LOCK_DAC			0x10
#define CR33_LOCK_PALETTE		0x40

/*
**	CRT34 - Backward compatibility register 3
*/
#define	CRT_BACKCOMPAT3			0x34
#define	CR34_PCI_SNOOP			0x01
#define	CR34_PCI_ABORT			0x02
#define	CR34_PCI_RETRY			0x04
#define	CR34_ENB_FIFO			0x10

/*
**	CRT36 - Configuration 1
*/
#define CRT_CONFIG1			0x36
#define CR36_VESALOCAL			0x01
#define CR36_PCI			0x02
#define CR36_BUSMASK			0x03
#define CR36_MEMSIZE_MASK		0xE0
#define CR36_1MB			0xC0
#define CR36_2MB			0x80
#define CR36_4MB			0x00
#define CR36_MEMMODE_MASK		0x0C
#define CR36_MEMMODE_1CYCLE		0x00
#define CR36_MEMMODE_2CYCLE		0x08
#define CR36_MEMMODE_SGRAM_SDRAM	0x40

/*
**	CRT37 - Configuration 2
*/
#define CRT_CONFIG2			0x37

/*
**	CRT38 - Lock 1
*/
#define CR38_MAGIC			0x48

/*
**	CRT39 - Lock 2
*/
#define CR39_MAGIC			0xA0
#define CR39_CR36MAGIC			0xA5

/*
**	CRT31 - Misc 1
*/
#define CR3A_REFRESHMASK		0x03
#define CR3A_PCIMASK			0x80
#define CR3A_NO4BPPMODE			0x10
#define CR3A_HISPEEDTXT			0x20
/*
**	CRT3B - Start display FIFO register
*/
#define CRT_FIFOSTART			0x3B

/*
**	CRT3C - Interlace retrace start
*/
#define CRT_INTERLACESTART		0x3C

/*
**	CRT42 - Mode control
*/
#define CR42_INTERLACED			0x20

/*
**	CRT50 - Extended system control 1 - Undocumented
*/
#define	CRT_SYSCONTROL1			0x50
#define	CR50_WIDTHMASK			0xC1
#define	CR50_1024			0x00
#define	CR50_2048			0x00
#define CR50_640			0x40
#define	CR50_800			0x80
#define	CR50_1280			0xC0
#define	CR50_1152			0x01
#define	CR50_1600			0x81
#define	CR50_4BPP			0x00
#define	CR50_8BPP			0x00
#define	CR50_16BPP			0x10
#define	CR50_32BPP			0x30

/*
**	CRT51 - Extended system control 2
*/
#define	CRT_SYSCONTROL2			0x51
#define	CR51_LOGICALWIDTH_B8		0x10
#define	CR51_LOGICALWIDTH_B9		0x20

/*
**	CRT53 - Extended memory control 1
*/
#define	CRT_MEMCONTROL1			0x53
#define	CR53_MMIO_DISABLE		0x00
#define CR53_MMIO_RELOC_MMIO		0x08
#define	CR53_MMIO_TRIO64_LIKE		0x10
#define	CR53_MMIO_TRIO_AND_RELOC	0x18
#define	CR53_MMIO_A0_TO_B0		0x00
#define	CR53_MMIO_A0_TO_C0		0x20
#define	CR53_SWAPNIBBLES		0x40

/*
**	CRT54 - Extended memory control 2
*/
#define	CRT_MEMCONTROL2			0x54
#define	CR54_M_MASK			0xF8
#define CR54_NO_BYTE_SWAP		0x00
#define	CR54_SWAP_BYTE_IN_WORD		0x01
#define	CR54_SWAP_BYTE_IN_DWORD		0x02
#define	CR54_SWAP_AS_PCI_OR_VL		0x03


/*
**	CRT56 - External sync control 1
*/
#define CRT_EXTSYNC1			0x56
#define CR56_HSYNC_OFF			0x02
#define CR56_VSYNC_OFF			0x04

/*
**	CRT58 - Linear address window control
*/
#define CR58_LAW_64KB			0x00
#define CR58_LAW_1MB			0x01
#define CR58_LAW_2MB			0x02
#define CR58_LAW_4MB			0x03
#define CR58_LAW_ENABLE			0x10
#define CR58_VL_T1_LATCH		0x40

/*
**	CRT5D - Extended horizontal overflow
*/
#define CR5D_HDISPLAYEND_B8		0x02
#define CR5D_HBLANKSTART_B8		0x04
#define CR5D_HSYNCSTART_B8		0x10
#define CR5D_HSYNCEND_B5		0x20
#define CR5D_HBLANKEND_B6		0x40
#define CR5D_HTOTAL_B8			0x01
#define CR5D_FIFOSTART_B8		0x40

/*
**	CRT5E - Extended vertical overflow
*/
#define CR5E_LINECOMPARE_B10		0x40
#define CR5E_VDISPLAYEND_B10		0x02
#define CR5E_VBLANKSTART_B10		0x04
#define CR5E_VSYNCSTART_B10		0x10
#define CR5E_VTOTAL_B10			0x01

/*
**	CRT60 - Extended memory control 3 - Undocumented
*/
#define	CRT_MEMCONTROL3			0x60

/*
**	CRT61 - Extended memory control 4
*/
#define	CRT_MEMCONTROL4			0x61
#define	CR61_NO_SWAP			0x00
#define	CR61_SWAP_BYTE_IN_WORD		0x20
#define	CR61_SWAP_BYTE_IN_DWORD		0x40

/*
**	CRT66 - Extended misc control 1
*/
#define CRT_EXTMISCCONTROL1		0x66
#define CR66_PCI_DISCONNECT		0x40
#define CR66_ENBL_ENH			0x01
/*
**	CRT67 - Extended misc control 2
*/

#define CRT_EXTMISCCONTROL2		0x67
#define CR67_VCLK_PHS			0x01
#define CR67_DAC_MODE0			0x00
#define CR67_DAC_MODE8			0x10
#define CR67_DAC_MODE9			0x30
#define CR67_DAC_MODEA			0x50
#define CR67_DAC_MODED			0xD0
#define CR67_FULL_STREAMS		0x0C
/*
**	CRT68 - Configuration 3
*/
#define CRT_CONFIG3			0x68

/*
**	CRT69 - Extended misc control 3
*/
#define CRT_EXTCONTROL3			0x69

/*
**	SEQ REGISTERS
*/

/* 
**	SRA - Extended Sequencer A 
*/
#define ESA_2MCLK			0x80
#define ESA_PIN50_RAS1			0x40

/* 
**	SRB - Extended Sequencer B 
*/
#define SR0B_NO_FEAT_INPUT		0x00

/*
**	SR08
*/
#define SR08_MAGIC			0x06

/*
**	SR09
*/
#define	SEQ_IO_CTRL			0x09
#define	SR09_MMIO_ONLY			0x80

/*
**	SR14 - ClkSyn Control 1
*/
#define SR14_NORMAL_OPERATION		0x00

/*
**	SR15 - ClkSyn Control 2
*/
#define SR15_2CYC_MWR			0x80
#define SR15_DCLKd2			0x10

/*
**	SR18 - Dac/Clksyn Control
*/
#define SR18_NORMAL_OPERATION		0x00
#define SR18_CLKx2			0x80

/*
**	STREAMS REGISTERS
*/

/*	MM8180 - Primary stream control
*/
#define STREAMS_PRIMARY_STREAM_CONTROL			0x8180
#define STREAMS_PrimaryStreamControl 		STREAMS_PRIMARY_STREAM_CONTROL
#define STREAMS_PSC_RGB8				0x00000000
#define STREAMS_PSC_KRGB16				0x03000000
#define STREAMS_PSC_RGB16				0x05000000
#define STREAMS_PSC_RGB24				0x06000000
#define STREAMS_PSC_XRGB32				0x07000000

/*	MM8184 - Color/Chroma key control
*/
#define STREAMS_COLORCHROMA_KEY_CONTROL			0x8184
#define STREAMS_ColorChromaKeyControl		STREAMS_COLORCHROMA_KEY_CONTROL

/*	MM8190 - Secondary stream control
*/
#define STREAMS_SECONDARY_STREAM_CONTROL		0x8190
#define STREAMS_SecondaryStreamControl	STREAMS_SECONDARY_STREAM_CONTROL

/*	MM8194 - Chroma key upper bound
*/
#define STREAMS_CHROMA_KEY_UPPER_BOUND			0x8194
#define STREAMS_ChromaKeyUpperBound	STREAMS_CHROMA_KEY_UPPER_BOUND

/*	MM8198 - Secondary stream stretch/filter constants
*/
#define STREAMS_SECONDARY_STRETCH_FILTER_CONSTANTS	0x8198
#define STREAMS_SecondaryStreamStretch STREAMS_SECONDARY_STRETCH_FILTER_CONSTANTS		

/*	MM81A0 - Blend control
*/
#define STREAMS_BLEND_CONTROL				0x81A0
#define STREAMS_BlendControl	STREAMS_BLEND_CONTROL	

/*	MM81C0 - Primary stream frame buffer address 0
*/
#define STREAMS_PRIMARY_STREAM_FB_ADDRESS0		0x81C0
#define STREAMS_PrimaryStreamAddress0 STREAMS_PRIMARY_STREAM_FB_ADDRESS0

/*	MM81C4 - Primary stream frame buffer address 1
*/
#define STREAMS_PRIMARY_STREAM_FB_ADDRESS1		0x81C4
#define STREAMS_PrimaryStreamAddress1 STREAMS_PRIMARY_STREAM_FB_ADDRESS1

/*	MM81C8 - Primary stream stride
*/
#define STREAMS_PRIMARY_STREAM_STRIDE			0x81C8
#define STREAMS_PrimaryStreamStride STREAMS_PRIMARY_STREAM_STRIDE

/*	MM81CC - Double buffer/LPB support
*/
#define STREAMS_DOUBLE_BUFFER_LPB_SUPPORT		0x81CC
#define STREAMS_DoubleBufferLpbSupport	STREAMS_DOUBLE_BUFFER_LPB_SUPPORT

/*	MM81D0 - Secondary stream frame buffer address 0
*/
#define STREAMS_SECONDARY_STREAM_FB_ADDRESS0		0x81D0
#define STREAMS_SecondaryStreamAddress0 STREAMS_SECONDARY_STREAM_FB_ADDRESS0

/*	MM81D4 - Secondary stream frame buffer address 1
*/
#define STREAMS_SECONDARY_STREAM_FB_ADDRESS1		0x81D4
#define STREAMS_SecondaryStreamAddress1 STREAMS_SECONDARY_STREAM_FB_ADDRESS1

/*	MM81D8 - Secondary stream stride
*/
#define STREAMS_SECONDARY_STREAM_STRIDE			0x81D8
#define STREAMS_SecondaryStreamStride STREAMS_SECONDARY_STREAM_STRIDE

/*	MM81DC - Opaque overlay control
*/
#define STREAMS_OPAQUE_OVERLAY_CONTROL			0x81DC
#define STREAMS_OpaqueOverlayControl STREAMS_OPAQUE_OVERLAY_CONTROL

/*	MM81E0 - K1 vertical scale factor
*/
#define STREAMS_K1_VERTICAL_SCALE_FACTOR		0x81E0
#define STREAMS_K1VerticalScaleFactor STREAMS_K1_VERTICAL_SCALE_FACTOR

/*	MM81E4 - K2 vertical scale factor
*/
#define STREAMS_K2_VERTICAL_SCALE_FACTOR		0x81E4
#define STREAMS_K2VerticalScaleFactor STREAMS_K2_VERTICAL_SCALE_FACTOR

/*	MM81E8 - DDA vertical accumulator initial value
*/
#define STREAMS_DDA_INITIAL_VALUE			0x81E8
#define STREAMS_DDAVerticalAccumulator	STREAMS_DDA_INITIAL_VALUE

/*	MM81EC - Streams FIFO control
*/
#define STREAMS_FIFO_CONTROL				0x81EC
#define STREAMS_StreamsFifoControl	STREAMS_FIFO_CONTROL

/*	MM81F0 - Primary stream window start coordinates
*/
#define STREAMS_PRIMARY_STREAM_WINDOW_START_COORD	0x81F0
#define STREAMS_PrimaryStreamStartCoordinates STREAMS_PRIMARY_STREAM_WINDOW_START_COORD

/*	MM81F4 - Primary stream window size
*/
#define STREAMS_PRIMARY_STREAM_WINDOW_SIZE		0x81F4
#define STREAMS_PrimaryStreamWindowSize STREAMS_PRIMARY_STREAM_WINDOW_SIZE

/*	MM81F8 - Secondary stream window start coordinates
*/
#define STREAMS_SECONDARY_STREAM_WINDOW_START_COORD	0x81F8
#define STREAMS_SecondaryStreamStartCoordinates STREAMS_SECONDARY_STREAM_WINDOW_START_COORD

/*	MM81FC - Secondary stream window size
*/
#define STREAMS_SECONDARY_STREAM_WINDOW_SIZE		0x81FC
#define STREAMS_SecondaryStreamWindowSize STREAMS_SECONDARY_STREAM_WINDOW_SIZE

#define STREAMS_FifoControl MPC_FIFOCONTROL
#define STREAMS_MiuControl  MPC_MIUCONTROL
#define STREAMS_StreamsTimeout	MPC_TIMEOUT1
#define STREAMS_MiscTimeout	MPC_TIMEOUT2

/*
**	MPC registers
*/
#define MPC_FIFOCONTROL		0x8200
#define MPC_MIUCONTROL		0x8204
#define MPC_TIMEOUT1		0x8208
#define MPC_TIMEOUT2		0x820c
#define MPC_DMAREAD_BASEADDR	0x8220
#define MPC_DMAREAD_STRIDEWIDTH	0x8220


/*	DMA registers
*/

#define DMA_SYSMEMADDR		0x8580
#define DMA_CTLDIRXFRLEN	0x8584
/* Missing 0x8588 */
#define DMA_PHYSADDR		0x8590
#define DMA_WRITEPTR		0x8594
#define DMA_READPTR		0x8598
#define DMA_ENABLE		0x859c


/*
**	VIRGE_3D accel registers
*/

#define VIRGE_3D_ZBUF_BASE		0xb0d4
#define VIRGE_3D_DEST_BASE		0xb0d8
#define VIRGE_3D_CLIP_LR		0xb0dc
#define VIRGE_3D_CLIP_TB		0xb0e0
#define VIRGE_3D_SRCDEST_STRIDE	0xb0e4
#define VIRGE_3D_Z_STRIDE		0xb0e8
#define VIRGE_3D_TEX_BASE		0xb0ec
#define VIRGE_3D_TEX_BDR_COLOR	0xb0f0
#define VIRGE_3D_DC_FADE_COLOR	0xb0f4
#define VIRGE_3D_TEX_BG_COLOR		0xb0f8
#define VIRGE_3D_TEX_FG_COLOR		0xb0fc

#define VIRGE_3D_LINE_DGDY_DBDY	0xb144
#define VIRGE_3D_LINE_DADY_DRDY	0xb148
#define VIRGE_3D_LINE_GS_BS		0xb14c
#define VIRGE_3D_LINE_AS_RS		0xb150
#define VIRGE_3D_LINE_DZ		0xb154
#define VIRGE_3D_LINE_ZSTART		0xb158
#define VIRGE_3D_LINE_FILL1		0xb15c
#define VIRGE_3D_LINE_FILL2		0xb160
#define VIRGE_3D_LINE_FILL3		0xb164
#define VIRGE_3D_LINE_XEND01		0xb168
#define VIRGE_3D_LINE_DX		0xb16c
#define VIRGE_3D_LINE_XSTART		0xb170
#define VIRGE_3D_LINE_YSTART		0xb174
#define VIRGE_3D_LINE_YCNT		0xb178

#define VIRGE_3D_TRI_BV		0xb504
#define VIRGE_3D_TRI_BU		0xb508
#define VIRGE_3D_TRI_DWDX		0xb50c
#define VIRGE_3D_TRI_DWDY		0xb510
#define VIRGE_3D_TRI_WS		0xb514
#define VIRGE_3D_TRI_DDDX		0xb518
#define VIRGE_3D_TRI_DVDX		0xb51c
#define VIRGE_3D_TRI_DUDX		0xb520
#define VIRGE_3D_TRI_DDDY		0xb524
#define VIRGE_3D_TRI_DVDY		0xb528
#define VIRGE_3D_TRI_DUDY		0xb52c
#define VIRGE_3D_TRI_DS		0xb530
#define VIRGE_3D_TRI_VS		0xb534
#define VIRGE_3D_TRI_US		0xb538
#define VIRGE_3D_TRI_DGDX_DBDX       	0xb53c
#define VIRGE_3D_TRI_DADX_DRDX       	0xb540
#define VIRGE_3D_TRI_DGDY_DBDY       	0xb544
#define VIRGE_3D_TRI_DADY_DRDY       	0xb548
#define VIRGE_3D_TRI_GS_BS		0xb54c
#define VIRGE_3D_TRI_AS_RS		0xb550
#define VIRGE_3D_TRI_DZDX		0xb554
#define VIRGE_3D_TRI_DZDY		0xb558
#define VIRGE_3D_TRI_ZS02		0xb55c
#define VIRGE_3D_TRI_DXDY12		0xb560
#define VIRGE_3D_TRI_XEND12		0xb564
#define VIRGE_3D_TRI_DXDY01		0xb568
#define VIRGE_3D_TRI_XEND01		0xb56c
#define VIRGE_3D_TRI_DXDY02		0xb570
#define VIRGE_3D_TRI_XSTART02		0xb574
#define VIRGE_3D_TRI_YSTART		0xb578
#define VIRGE_3D_TRI_Y01_Y12		0xb57c

/*
**	Miscellaneous Registers
*/

#define SUBSYSTEM_STATUS		0x8504
#define SSS_VSY_INT			0x00000001
#define SSS_S3D_DON			0x00000002
#define SSS_FIFO_OVF			0x00000004
#define SSS_FIFO_EMP			0x00000008
#define SSS_HD_DON			0x00000010
#define SSS_CD_DON			0x00000020
#define SSS_S3DF_FIF			0x00000040
#define SSS_LPB_INT			0x00000080
#define SSS_S3D_SLOTS_FREE_MASK		0x00001F00
#define SSS_S3D_ENG			0x00002000
#define SUBSYSTEM_CONTROL		0x8504
#define SSC_CLEAR_ALL_INTERRUPTS	0x0000007F
#define SSC_HDD_ENB			0x00000080
#define SSC_VSY_ENB			0x00000100
#define SSC_3DD_ENB			0x00000200
#define SSC_FIFO_ENB_OVF		0x00000400
#define SSC_FIFO_ENB_EMP		0x00000800
#define SSC_CCD_ENB			0x00001000
#define SSC_S3DF_ENB			0x00002000
#define SSC_S3D_ENG_ENABLED		0x00004000
#define SSC_S3D_ENG_RESET		0x00008000
#define ADVANCED_FUNCTION_CONTROL	0X850C

/*
**	LPB registers
*/

#define LPB_MODE		0xff00
#define LPB_OUTFIFO_STATUS     	0xff04
#define LPB_INTR_FLAGS		0xff08
#define LIF_SPS			0x00000008
#define LIF_SPM			0x00080000
#define LIF_SPW			0x01000000
#define LPB_FB_ADDR0		0xff0c
#define LPB_FB_ADDR1		0xff10
#define LPB_DIRREADWRITE_ADDR  	0xff14
#define LPB_DIRREADWRITE_DATA  	0xff18
#define LPB_GENIOPORT		0xff1c
#define LPB_SERIALPORT		0xff20
#define LPB_VIDEOIN_WINSIZE    	0xff24
#define LPB_VIDEODATA_OFF      	0xff28
#define LPB_H_DECIMATION_CTRL  	0xff2c
#define LPB_V_DECIMATION_CTRL  	0xff30
#define LPB_LINE_OFF		0xff38
#define LPB_OUTFIFO_DATA       	0xff3c

#endif	/* #ifdef _chipset_S3_virge_h */
