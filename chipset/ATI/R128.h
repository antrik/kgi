/* ----------------------------------------------------------------------------
**	ATI Rage 128 chipset register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	Paul Redmond
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: R128.h,v $
**	Revision 1.1  2003/09/14 17:49:53  redmondp
**	- start of Rage 128 driver
**	
**	
*/
#ifndef	_chipset_ATI_R128_h
#define	_chipset_ATI_R128_h

/* PCI config region */
#ifndef	PCI_VENDOR_ID_ATI
#define	PCI_VENDOR_ID_ATI		0x1002
#endif

#define	PCI_DEVICE_ID_ATI_RAGE128RE 	0x5245
#define	PCI_DEVICE_ID_ATI_RAGE128RF 	0x5246
#define	PCI_DEVICE_ID_ATI_RAGE128RG 	0x5247
#define	PCI_DEVICE_ID_ATI_RAGE128RK 	0x524B
#define	PCI_DEVICE_ID_ATI_RAGE128RL 	0x524C

#define MHZ *1000000
#define KHZ *1000

/* PCI aperture sizes */
#define	R128_Base0_Size	0x4000000	/* framebuffer */
#define R128_Base1_Size 0x100		/* io */
#define	R128_Base2_Size	0x4000		/* control regs */

/* Rage 128 register definitions */

#define R128_CLOCK_CNTL_INDEX					0x08
#define   R128_08_PLL_ADDRMask					0x0000001F
#define   R128_08_PLL_ADDRShift					0
#define   R128_08_PLL_WR_EN					0x00000080
#define   R128_08_PPLL_DIV_SELMask				0x00000300
#define   R128_08_PPLL_DIV_SELShift				0

#define R128_CLOCK_CNTL_DATA					0x0C

#define R128_GEN_INT_CNTL					0x40

#define R128_CRTC_GEN_CNTL					0x50
#define   R128_50_CRTC_DBL_SCAN_EN				0x00000001
#define   R128_50_CRTC_INTERLACE_EN				0x00000002
#define   R128_50_CRTC_C_SYNC_EN				0x00000010
#define   R128_50_CRTC_PIX_WIDTHMask				0x00000700
#define   R128_50_CRTC_PIX_WIDTHShift				8
#define   R128_50_CRTC_CUR_EN					0x00010000
#define   R128_50_CRTC_CUR_MODEMask				0x7
#define   R128_50_CRTC_CUR_MODEShift				17
#define   R128_50_CRTC_EXT_DISP_EN				0x01000000
#define   R128_50_CRTC_EN					0x02000000
#define   R128_50_CRTC_DISP_REQ_EN_B				0x04000000

#define R128_CRTC_EXT_CNTL					0x54
#define   R128_54_CRTC_VGA_XOVERSCAN				0x00000001
#define   R128_54_VGA_BLINK_RATEMask				0x3
#define   R128_54_VGA_BLINK_RATEShift				1
#define   R128_54_VGA_ATI_LINEAR				0x00000008
#define   R128_54_VGA_128KAP_PAGING				0x00000010
#define   R128_54_VGA_TEXT_132					0x00000020
#define   R128_54_VGA_XCRT_CNT_EN				0x00000040
#define   R128_54_CRTC_HSYNC_DIS				0x00000100
#define   R128_54_CRTC_VSYNC_DIS				0x00000200
#define   R128_54_CRTC_DISPLAY_DIS				0x00000400
#define   R128_54_CRTC_SYNC_TRISTATE				0x00000800
#define   R128_54_VGA_CUR_B_TEST				0x00020000
#define   R128_54_VGA_PACK_DIS					0x00040000
#define   R128_54_VGA_MEM_PS_EN					0x00080000
#define   R128_54_VGA_READ_PREFETCH_DIS				0x00100000
#define   R128_54_DFIFO_EXTSENSE				0x00200000
#define   R128_54_FP_OUT_EN					0x00400000
#define   R128_54_FP_ACTIVE					0x00800000
#define   R128_54_VCRTC_IDX_MASTERMask				0x7F
#define   R128_54_VCRTC_IDX_MASTERShift				24

#define R128_DAC_CNTL						0x58
#define   R128_58_DAC_RANGE_CNTLMask				0x00000003
#define   R128_58_DAC_RANGE_CNTLShift				0
#define   R128_58_DAC_BLANKING					0x00000004
#define   R128_58_DAC_COMP_EN					0x00000008
#define   R128_58_DAC_CMP_OUTPUT				0x00000080
#define   R128_58_DAC_8BIT_EN					0x00000100
#define   R128_58_DAC_4BPP_PIX_ORDER				0x00000200
#define   R128_58_DAC_TVO_EN					0x00000400
#define   R128_58_DAC_TVO_OVR_EXCL				0x00000800
#define   R128_58_DAC_TVO_16BPP_DITH_EN				0x00001000
#define   R128_58_DAC_VGA_ADR_EN				0x00002000
#define   R128_58_DAC_PDWN					0x00008000
#define   R128_58_DAC_CRC_EN					0x00080000
#define   R128_58_DAC_MASKMask					0xFF000000
#define   R128_58_DAC_MASKShift					24

#define R128_GPIO_MONID						0x68
#define   R128_68_MONID_AMask					0x0000000F
#define   R128_68_MONID_AShift					0
#define   R128_68_MONID_YMask   				0x00000F00
#define   R128_68_MONID_YShift					8
#define   R128_68_MONID_ENMask					0x000F0000
#define   R128_68_MONID_ENShift					16
#define   R128_68_MONID_MASKMask				0x0F000000
#define   R128_68_MONID_MASKShift				24

#define R128_GPIO_MONIDB					0x6C
#define   R128_6C_MONIDB_AMask					0x00000003
#define   R128_6C_MONIDB_AShift					0
#define   R128_6C_MONIDB_YMask   				0x00000300
#define   R128_6C_MONIDB_YShift					8
#define   R128_6C_MONIDB_ENMask					0x00030000
#define   R128_6C_MONIDB_ENShift				16
#define   R128_6C_MONIDB_MASKMask				0x03000000
#define   R128_6C_MONIDB_MASKShift				24

#define R128_I2C_CNTL_1						0x94

#define R128_GEN_RESET_CNTL					0xF0
#define   R128_F0_SOFT_RESET_GUI				0x00000001

#define R128_CONFIG_MEMSIZE					0xF8
#define   R128_F8_CONFIG_MEMSIZEMask				0x03FFFFFF
#define   R128_F8_CONFIG_MEMSIZEShift				0

#define R128_MEM_CNTL						0x140
#define   R128_140_MEM_CFG_TYPEMask				0x00000003
#define   R128_140_MEM_CFG_TYPEShift				0
#define   R128_140_MEM_BW_COL					0x00000008
#define   R128_140_MEM_ERST_CNTLMask				0x00000030
#define   R128_140_MEM_ERST_CNTLShift				4
#define   R128_140_MEM_DREN_CNTLMask				0x000000C0
#define   R128_140_MEM_DREN_CNTLShift				6
#define   R128_140_MEM_LATENCYMask				0x00000700
#define   R128_140_MEM_LATENCYShift				8
#define   R128_140_MEM_WR_LATENCYMask				0x00003000
#define   R128_140_MEM_WR_LATENCYShift				12
#define   R128_140_MEM_WDOE_CNTLMask				0x0000C000
#define   R128_140_MEM_WDOE_CNTLShift				14
#define   R128_140_MEM_OPER_MODEMask				0x000F0000
#define   R128_140_MEM_OPER_MODEShift				16
#define   R128_140_MEM_CTRL_STATUS				0x00100000
#define   R128_140_MEM_SEQNCR_STATUS				0x00200000
#define   R128_140_MEM_ARBITER_STATUS				0x00400000
#define   R128_140_MEM_REQ_LOCK					0x00800000
#define   R128_140_MEM_EXTND_ERST				0x01000000
#define   R128_140_MEM_EXTND_DREN				0x02000000
#define   R128_140_MEM_DQM_RD_DIS				0x04000000
#define   R128_140_MEM_REFRESH_DIS				0x08000000
#define   R128_140_MEM_REFRESH_RATEMask				0xF0000000
#define   R128_140_MEM_REFRESH_RATEShift			28

#define R128_PC_NGUI_CTLSTAT					0x184
#define   R128_184_PC_FLUSH_GUIMask				0x00000003
#define   R128_184_PC_FLUSH_GUIShift				0
#define   R128_184_PC_RI_GUIMask				0x0000000C
#define   R128_184_PC_RI_GUIShift				2
#define   R128_184_PC_FLUSH_NONGUIMask				0x00000030
#define   R128_184_PC_FLUSH_NONGUIShift				4
#define   R128_184_PC_RI_NONGUIMask				0x000000C0
#define   R128_184_PC_RI_NONGUIShift				6
#define   R128_184_PC_PURGE_GUI					0x00000100
#define   R128_184_PC_PURGE_NONGUI				0x00000200
#define   R128_184_PC_DIRTY					0x01000000
#define   R128_184_PC_PURGE_DOPURGE				0x02000000
#define   R128_184_PC_FLUSH_DOFLUSH				0x04000000
#define   R128_184_PC_BUSY_INIT					0x08000000
#define   R128_184_PC_BUSY_FLUSH				0x1
#define   R128_184_PC_BUSY_GUI					0x20000000
#define   R128_184_PC_BUSY_NGUI					0x40000000
#define   R128_184_PC_BUSY					0x80000000

#define R128_MPP_TB_CONFIG					0x1C0

#define R128_MPP_GP_CONFIG					0x1C8

#define R128_VIPH_CONTROL					0x1D0

#define R128_CRTC_H_TOTAL_DISP					0x200
#define   R128_200_CRTC_H_TOTALMask				0x000001FF
#define   R128_200_CRTC_H_TOTALShift				0
#define   R128_200_CRTC_H_DISPMask				0x00FF0000
#define   R128_200_CRTC_H_DISPShift				16

#define R128_CRTC_H_SYNC_STRT_WID				0x204
#define   R128_204_CRTC_H_SYNC_STRT_PIXMask			0x00000007
#define   R128_204_CRTC_H_SYNC_STRT_PIXShift			0
#define   R128_204_CRTC_H_SYNC_STRT_CHARMask			0x00000FF8
#define   R128_204_CRTC_H_SYNC_STRT_CHARShift			3
#define   R128_204_CRTC_H_SYNC_WIDMask				0x003F0000
#define   R128_204_CRTC_H_SYNC_WIDShift				16
#define   R128_204_CRTC_H_SYNC_POL				0x00800000

#define R128_CRTC_V_TOTAL_DISP					0x208
#define   R128_208_CRTC_V_TOTALMask				0x000007FF
#define   R128_208_CRTC_V_TOTALShift				0
#define   R128_208_CRTC_V_DISPMask				0x00FF0000
#define   R128_208_CRTC_V_DISPShift				16

#define R128_CRTC_V_SYNC_STRT_WID				0x20C
#define   R128_20C_CRTC_V_SYNC_STRTMask				0x000007FF
#define   R128_20C_CRTC_V_SYNC_STRTShift			0
#define   R128_20C_CRTC_V_SYNC_WIDMask				0x001F0000
#define   R128_20C_CRTC_V_SYNC_WIDShift				16
#define   R128_20C_CRTC_V_SYNC_POL				0x00800000

#define R128_CRTC_OFFSET					0x224
#define   R128_224_CRTC_OFFSETMask				0x01FFFFFF
#define   R128_224_CRTC_OFFSETShift				0
#define   R128_224_CRTC_GUI_TRIG_OFFSET				0x40000000
#define   R128_224_CRTC_OFFSET_LOCK				0x80000000

#define R128_CRTC_OFFSET_CNTL					0x228
#define   R128_228_CRTC_TILE_LINEMask				0x0000001F
#define   R128_228_CRTC_TILE_LINEShift				0
#define   R128_228_CRTC_TILE_ALIGNMask				0x00000700
#define   R128_228_CRTC_TILE_ALIGNShift				8
#define   R128_228_CRTC_TILE_EN					0x00008000
#define   R128_228_CRTC_OFFSET_FLIP_CNTL			0x00010000
#define   R128_228_CRTC_GUI_TRIG_OFFSET				0x40000000
#define   R128_228_CRTC_OFFSET_LOCK				0x80000000

#define R128_CRTC_PITCH						0x22C
#define   R128_22C_CRTC_PITCHMask				0x000003FF
#define   R128_22C_CRTC_PITCHShift				0

#define R128_OVR_CLR						0x230

#define R128_OVR_WID_LEFT_RIGHT					0x234

#define R128_OVR_WID_TOP_BOTTOM					0x238

#define R128_DDA_CONFIG						0x2E0
#define   R128_2E0_DDA_XCLKS_PER_XFERMask			0x00003FFF
#define   R128_2E0_DDA_XCLKS_PER_XFERShift			0
#define   R128_2E0_DDA_PRECISIONMask				0x000F0000
#define   R128_2E0_DDA_PRECISIONShift				16
#define   R128_2E0_DDA_LOOP_LATENCYMask				0x01F00000
#define   R128_2E0_DDA_LOOP_LATENCYShift			20

#define R128_DDA_ON_OFF						0x2E4
#define   R128_2E4_DDA_OFFMask					0x0000FFFF
#define   R128_2E4_DDA_OFFShift					0
#define   R128_2E4_DDA_ONMask					0xFFFF0000
#define   R128_2E4_DDA_ONShift					16

#define R128_OV0_SCALE_CNTL					0x420

#define R128_SUBPIC_CNTL					0x540

#define R128_PM4_BUFFER_CNTL					0x704

#define R128_PM4_STAT						0x7B4
#define   R128_7B4_PM4_BUSY					0x00010000
#define   R128_7B4_GUI_ACTIVE					0x80000000

#define R128_PM4_MICROCODE_ADDR					0x7D4

#define R128_PM4_MICROCODE_DATAH				0x7DC

#define R128_PM4_MICROCODE_DATAL				0x7E0

#define R128_PM4_MICRO_CNTL					0x7FC

#define R128_CAP0_TRIG_CNTL					0x950

#define R128_CAP1_TRIG_CNTL					0x9C0

#define R128_PM4_FIFO_DATA_EVEN					0x1000

#define R128_PM4_FIFO_DATA_ODD					0x1004

#define R128_DP_GUI_MASTER_CNTL					0x146C

#define R128_DP_BRUSH_BKGD_CLR					0x1478
#define R128_DP_BRUSH_FRGD_CLR					0x147C

#define R128_DP_SRC_FRGD_CLR					0x15D8
#define R128_DP_SRC_BKGD_CLR					0x15DC

#define R128_DST_BRES_ERR					0x1628
#define R128_DST_BRES_INC					0x162C
#define R128_DST_BRES_DEC					0x1630

#define R128_DP_WRITE_MASK					0x16CC

#define R128_DEFAULT_OFFSET					0x16E0
#define R128_DEFAULT_PITCH					0x16E4
#define R128_DEFAULT_SC_BOTTOM_RIGHT				0x16E8

#define R128_PC_GUI_CTLSTAT					0x1748
#define   R128_1748_PC_FLUSH_GUIMask				0x00000003
#define   R128_1748_PC_FLUSH_GUIShift				0
#define   R128_1748_PC_RI_GUIMask				0x0000000C
#define   R128_1748_PC_RI_GUIShift				2
#define   R128_1748_PC_FLUSH_NONGUIMask				0x00000030
#define   R128_1748_PC_FLUSH_NONGUIShift			4
#define   R128_1748_PC_RI_NONGUIMask				0x000000C0
#define   R128_1748_PC_RI_NONGUIShift				6
#define   R128_1748_PC_PURGE_GUI				0x00000100
#define   R128_1748_PC_PURGE_NONGUI				0x00000200
#define   R128_1748_PC_DIRTY					0x01000000
#define   R128_1748_PC_PURGE_DOPURGE				0x02000000
#define   R128_1748_PC_FLUSH_DOFLUSH				0x04000000
#define   R128_1748_PC_BUSY_INIT				0x08000000
#define   R128_1748_PC_BUSY_FLUSH				0x1
#define   R128_1748_PC_BUSY_GUI					0x20000000
#define   R128_1748_PC_BUSY_NGUI				0x40000000
#define   R128_1748_PC_BUSY					0x80000000

#define R128_SCALE_3D_CNTL					0x1A00

#endif	/* #ifdef _chipset_ATI_R128_h	*/
