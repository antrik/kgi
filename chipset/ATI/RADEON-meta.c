/* ----------------------------------------------------------------------------
**	ATI RADEON chipset implementation
** ----------------------------------------------------------------------------
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: RADEON-meta.c,v $
**	Revision 1.11  2004/02/15 22:10:49  aldot
**	- use kgim_
**	
**	Revision 1.10  2003/08/30 18:21:12  redmondp
**	- fix a bug from last commit that prevents ROM pll info from being used
**	
**	Revision 1.9  2003/08/27 21:46:44  redmondp
**	- try to read clock programming constants from the ROM
**	
**	Revision 1.8  2003/03/13 03:03:59  fspacek
**	- merge clock into chipset, remove atomic update, fix sync polarity,
**	  clear high bits on CRTC registers upon VGA mode enter
**	
**	Revision 1.7  2003/03/08 14:07:36  skids
**	
**	Progress to date on GC-related register save/restore.
**	
**	Revision 1.6  2002/12/09 18:31:47  fspacek
**	- radeon DDC support
**	
**	Revision 1.5  2002/11/02 19:26:25  redmondp
**	-Fix the default pitch calculation.
**	
**	Revision 1.4  2002/10/25 03:10:29  fspacek
**	PIO accelerator.
**	
**	Revision 1.3  2002/09/26 06:21:08  fspacek
**	Text really works now. I promise.
**	
**	Revision 1.2  2002/09/26 04:42:51  fspacek
**	RSOD fixups, text mode fix
**	
**	Revision 1.1  2002/09/22 01:39:46  fspacek
**	I didn't do it ;-)
**	
**	
*/
#include <kgi/maintainers.h>
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.11 $"

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	0
#endif

#define KGI_SYS_NEED_VM
#include <kgi/module.h>

#include "chipset/ATI/RADEON.h"
#include "chipset/ATI/RADEON-meta.h"

#include "chipset/ATI/RADEON-microcode.h"

static kgi_error_t radeon_pixcache_flush(radeon_chipset_io_t *radeon_io)
{
	kgi_u32_t tmp;
	int i;

	tmp  = RADEON_CTRL_IN32(radeon_io, RADEON_RB2D_DSTCACHE_CTLSTAT);
	tmp |= ((3 << RADEON_RB2D_DC_FLUSHShift) & RADEON_RB2D_DC_FLUSHShift) |
		((3 << RADEON_RB2D_DC_FREEShift) & RADEON_RB2D_DC_FREEShift);
	RADEON_CTRL_OUT32(radeon_io, tmp, RADEON_RB2D_DSTCACHE_CTLSTAT);

	for (i = 0; i < 100000; i++) {
		if (!(RADEON_CTRL_IN32(radeon_io, RADEON_RB2D_DSTCACHE_CTLSTAT)
		       & RADEON_RB2D_DC_BUSY)) {
			return KGI_EOK;
		}
	}

	KRN_ERROR("pixcache_flush failed!");
	return -1;
}

static kgi_error_t radeon_wait_for_fifo(radeon_chipset_io_t *radeon_io,
					int entries)
{
	int i;

	for (i = 0; i < 100000; i++) {
		int slots = (RADEON_CTRL_IN32(radeon_io, RADEON_RBBM_STATUS)
			     & RADEON_RBBM_CMDFIFO_AVAILMask);
		if (slots >= entries)
			return KGI_EOK;
	}

	KRN_ERROR("radeon_wait_for_fifo(%d) failed", entries);
	return -1;
}

static int radeon_wait_for_idle(radeon_chipset_io_t *radeon_io)
{
	int i, ret;

	ret = radeon_wait_for_fifo(radeon_io, 64);
	if (ret != KGI_EOK)
		return ret;

	for (i = 0; i < 100000; i++) {
		if (!(RADEON_CTRL_IN32(radeon_io, RADEON_RBBM_STATUS)
		      & RADEON_RBBM_ACTIVE) ) {
			radeon_pixcache_flush(radeon_io);
			return 0;
		}
	}

	KRN_ERROR("radeon_wait_for_idle failed");
	return -1;
}

static kgi_error_t radeon_reset_engine(radeon_chipset_io_t *radeon_io)
{
	kgi_u32_t clock_cntl_index, mclk_cntl, rbbm_soft_reset, val;
	KRN_DEBUG(2, "radeon_reset_engine");

	/* TODO: stop IRQ handler here (or caller should). */

	radeon_pixcache_flush(radeon_io);

	clock_cntl_index = RADEON_CTRL_IN32(radeon_io,RADEON_CLOCK_CNTL_INDEX);
	mclk_cntl = RADEON_CLOCK_IN32(radeon_io, RADEON_MCLK_CNTL);

	val = mclk_cntl |
		RADEON_FORCEON_MCLKA |
		RADEON_FORCEON_MCLKB |
		RADEON_FORCEON_YCLKA |
		RADEON_FORCEON_YCLKB |
		RADEON_FORCEON_MC |
		RADEON_FORCEON_AIC;
	RADEON_CLOCK_OUT32(radeon_io, val, RADEON_MCLK_CNTL);

	rbbm_soft_reset = RADEON_CTRL_IN32(radeon_io, RADEON_RBBM_SOFT_RESET);
	val = rbbm_soft_reset |
		RADEON_SOFT_RESET_CP |
		RADEON_SOFT_RESET_HI |
		RADEON_SOFT_RESET_SE |
		RADEON_SOFT_RESET_RE |
		RADEON_SOFT_RESET_PP |
		RADEON_SOFT_RESET_E2 |
		RADEON_SOFT_RESET_RB;
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_RBBM_SOFT_RESET);
	RADEON_CTRL_IN32(radeon_io, RADEON_RBBM_SOFT_RESET);
	val = rbbm_soft_reset &
		~(RADEON_SOFT_RESET_CP |
		  RADEON_SOFT_RESET_HI |
		  RADEON_SOFT_RESET_SE |
		  RADEON_SOFT_RESET_RE |
		  RADEON_SOFT_RESET_PP |
		  RADEON_SOFT_RESET_E2 |
		  RADEON_SOFT_RESET_RB);
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_RBBM_SOFT_RESET);
	RADEON_CTRL_IN32(radeon_io, RADEON_RBBM_SOFT_RESET);


	RADEON_CLOCK_OUT32(radeon_io, mclk_cntl, RADEON_MCLK_CNTL);
	RADEON_CTRL_OUT32(radeon_io,clock_cntl_index, RADEON_CLOCK_CNTL_INDEX);
	RADEON_CTRL_OUT32(radeon_io, rbbm_soft_reset, RADEON_RBBM_SOFT_RESET);

	/* TODO: restart IRQ handler here (or caller should). */

	return KGI_EOK;
}

/*
**	Helper functions
*/

static inline kgi_u_t radeon_next_supported_width(kgi_u_t width)
{
#warning	add supported image buffer widths here. 
	return (width + 7) & ~7;
}

static void radeon_cp_load_microcode(radeon_chipset_io_t *radeon_io,
				     kgi_u32_t microcode[][2])
{
	int i;
	KRN_DEBUG(2, "radeon_cp_load_microcode");

	radeon_wait_for_idle(radeon_io);

	RADEON_CTRL_OUT32(radeon_io, 0, RADEON_CP_ME_RAM_ADDR);
	for (i = 0; i < 256; i++) {
		RADEON_CTRL_OUT32(radeon_io, microcode[i][1],
				  RADEON_CP_ME_RAM_DATAH);
		RADEON_CTRL_OUT32(radeon_io, microcode[i][0],
				  RADEON_CP_ME_RAM_DATAL);
	}
}



/* Following are a few lists of registers (other than those set by the 
 * basic mode facilities) which should be saved during console system 
 * switch-away and restored on switch-back.  Which registers are saved 
 * depends on exactly which chipset is being driven.  Registers that 
 * never change aren't saved; Making sure all of those are restored 
 * even if they somehow get scrambled is another TODO item.  These 
 * tables only deal with registers that may vary between different 
 * /dev/accel mapping contexts.
 */

/*  
 * There will eventually be one universal list that applies to all 
 * Radeon series chips with a few small lists of extras for each 
 * individual chipset.  Right now only the list for r200 has been 
 * collated and it's not known which registers are universal.
 * But in order to get the code structure in place, we pick one we 
 * know all Radeons will have for sure to be in the global list.
 */

static kgi_u16_t radeon_all_saveregi_gc[RADEON_ALL_NUM_SAVEREGI_GC] = { 

	/* Register data is written in the order they appear here. */
	0x2c38 
};

static kgi_u32_t radeon_all_initdata_gc[RADEON_ALL_NUM_SAVEREGI_GC] = { 

	/* Register data is written in the order they appear here. */
	0x00000002
};

static kgi_u16_t radeon_r200_saveregi_gc[RADEON_R200_NUM_SAVEREGI_GC] = {

	/* Register data is written in the order they appear here. */


/*
  Reserve these regi for use by ring buffer code?
  SCRATCH_UMSK 770
  SCRATCH_ADDR 774
  SCRATCH_* 15e0-15f4
*/

	0x172c, 0x1780, 0x1d60, 0x1d64, 0x1d68, 0x1d6c, 0x1d7c, 0x1d80, 
	0x1d84, 0x2088, 0x208c, 0x2098, 0x209c, 0x20a0, 0x20a4, 0x20a8, 
	0x20ac, 0x20b8, 0x20c0, 0x20c4, 0x20c8, 0x20cc, 0x20d0, 0x20d4, 
	0x20d8, 0x20dc, 0x20e0, 0x20e4, 0x20e8, 0x20ec, 0x20f0, 0x20f4, 
	0x20f8, 0x20fc, 0x2100, 0x0104, 0x2108, 0x2140, 0x2200, 0x2208, 
	0x2210, 0x2214, 0x2218, 0x221c, 0x2220, 0x2224, 0x2228, 0x222c, 
	0x2230, 0x2234, 0x2238, 0x223c, 0x2240, 0x2244, 0x2248, 0x224c, 
	0x2254, 0x2258, 0x225c, 0x2260, 0x2264, 0x2268, 0x2270, 0x2274, 
	0x2278, 0x227c, 0x261c, 0x2644, 0x26c0, 0x26c4, 0x26c8, 0x26d0, 
	0x26d4, 0x2c14, 0x2c18, 0x2c54, 0x2c58, 0x2c5c, 0x2c60, 0x2c64, 
	0x2c68, 0x2c6c, 0x2c70, 0x2c74, 0x2c78, 0x2c7c, 0x2c80, 0x2c84, 
	0x2c88, 0x2c8c, 0x2c90, 0x2c94, 0x2c98, 0x2d00, 0x2d04, 0x2d08, 
	0x2d0c, 0x2d10, 0x2d14, 0x2d18, 0x2d24, 0x2d28, 0x2d2c, 0x2d34, 
	0x2d38, 0x2d3c, 0x2d40, 0x2d44, 0x2d48, 0x2d58, 0x2d5c, 0x2dd0, 
	0x2dd4, 0x2dd8, 0x2ddc, 0x2de0, 0x2e00, 0x2e04, 0x2e08, 0x2e0c, 
	0x2e10, 0x2e14, 0x2e18, 0x2e1c, 0x2e20, 0x2e24, 0x2e28, 0x2e2c, 
	0x2e30, 0x2e34, 0x2e38, 0x3220, 0x3224, 0x3228, 0x322c, 0x323c, 
	0x3240, 0x3248, 0x3400, 0x3408, 0x340c, 0x3410, 0x3414, 0x3418, 
	0x341c, 0x3430, 0x3434, 0x3450, 0x3454, 0x3500, 0x3504, 0x3508, 
	0x350c, 0x3510, 0x3514, 0x3518, 0x351c, 0x3520, 0x3524, 0x3528, 
	0x352c, 0x3530, 0x3534, 0x3538, 0x353c, 0x3540, 0x3544, 0x3548, 
	0x354c, 0x3550, 0x3554, 0x3558, 0x355c, 0x3560, 0x3564, 0x3568, 
	0x356c, 0x3570, 0x3574, 0x3578, 0x357c, 0x3580, 0x3584, 0x3588, 
	0x358c, 0x3590, 0x3594, 0x3598, 0x359c, 0x35a0, 0x35a4, 0x35a8, 
	0x35ac, 0x35b0, 0x35b4, 0x35b8, 0x35bc, 0x35c0, 0x35c4, 0x35c8, 
	0x35cc, 0x35d0, 0x35d4, 0x35d8, 0x35dc, 0x35e0, 0x35e4, 0x35e8, 
	0x35ec, 0x35f0, 0x35f4, 0x35f8, 0x35fc
};

static kgi_u32_t radeon_r200_initdata_gc[RADEON_R200_NUM_SAVEREGI_GC] = {


	/* These values are chosen to be purposefully obstructive,
	 * such that applications are forced to fully initialize the GC 
	 * and not rely on any default values.  For example, clipping is 
	 * set to prohibit any drawing from actually occurring until it is 
	 * explicitly set up.  We also avoid values marked "Reserved".
	 */

	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0x0800003e, 0x00000300, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0x01010000, 0, 0, 0, 0x00001000,
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};

/* "Leader GC" -- context only held by session leaders. */
static kgi_u16_t radeon_all_saveregi_mgc[RADEON_ALL_NUM_SAVEREGI_MGC] = { 

	0x050

};

static kgi_u32_t radeon_all_initdata_mgc[RADEON_ALL_NUM_SAVEREGI_MGC] = { 
  
	0x00000000
};

static kgi_u16_t radeon_r200_saveregi_mgc[RADEON_R200_NUM_SAVEREGI_MGC] = {

	/* Register data is written in the order they appear here. */

	0x00B0, 0x0190, 0x0210, 0x0218, 0x0220, 0x0224, 0x0228, 0x0230, 
	0x0234, 0x0238, 0x0260, 0x0264, 0x0268, 0x026c, 0x0270, 0x02b0, 
	0x02b4, 0x02b8, 0x02bc, 0x02c0, 0x02d8, 0x02dc, 0x0310, 0x0318, 
	0x0324, 0x0328, 0x0330, 0x0334, 0x0338, 0x033c, 0x0348, 0x0360, 
	0x0364, 0x0368, 0x036c, 0x0370, 0x03b0, 0x03b4, 0x03b8, 0x03bc, 
	0x03c0, 0x03f8, 0x0400, 0x0404, 0x0408, 0x0410, 0x0420, 0x0424, 
	0x0428, 0x042c, 0x0430, 0x0434, 0x043C, 0x0440, 0x0444, 0x0448, 
	0x044c, 0x0450, 0x0454, 0x0460, 0x0464, 0x0470, 0x0474, 0x0480, 
	0x0484, 0x0488, 0x048c, 0x0494, 0x0498, 0x049c, 0x04A0, 0x04B0, 
	0x04b4, 0x04b8, 0x04bc, 0x04C0, 0x04dc, 0x04e0, 0x04e4, 0x04e8, 
	0x04ec, 0x04f0, 0x04f4, 0x04F8, 0x0600, 0x0604, 0x0608, 0x0900, 
	0x0908, 0x0920, 0x0924, 0x0928, 0x092c, 0x0930, 0x0934, 0x0938, 
	0x093c, 0x0940, 0x0944, 0x0948, 0x094c, 0x0950, 0x0958, 0x095c, 
	0x0960, 0x0964, 0x0968, 0x096c, 0x0970, 0x0980, 0x0984, 0x0988, 
	0x098c, 0x0d20, 0x0d24, 0x0d28, 0x0d2c, 0x0d30, 0x0d34, 0x0d40, 
	0x0d44, 0x0d48, 0x0d4c, 0x0d50, 0x0d54, 0x0d60, 0x0d64, 0x0d68, 
	0x0dbc, 0x1720 
};

static kgi_u32_t radeon_r200_initdata_mgc[RADEON_R200_NUM_SAVEREGI_MGC] = {

	0, 0x00050000, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0x4a000000, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0x00000008, 0, 0x00000400, 0, 0x00000001, 
	0x00000001, 0x00000fff, 0x000007ff, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0x20000000, 0x20000000, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0x00000008, 0x00000010,
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0xffff0000, 0, 0, 
	0, 0
};


#if 0
/*
 * These registers aren't backed up in the GC so they do not appear in 
 * the saveregi lists, but any GC holder should be able to write them.
 */
static kgi_u16_t radeon_other_regi_gc [RADEON_NUM_OTHER_REGI_GC] = {
	0x005c, 0x00b4, 0x00b8, 0x03fc, 0x1784, 0x2204, 0x220c, 0x26cc
};

/*
 * These registers require special intervention by the driver when
 * a GC and/or MGC holder tries to write to them.
 */
static kgi_u16_t radeon_special_regi [RADEON_NUM_SPECIAL_REGI] = {
	0x0050, 0x0190, 0x0210, 0x0218, 0x0220, 0x0224, 0x0228, 0x0234, 
	0x0238, 0x0260, 0x02b0, 0x02f8, 0x0310, 0x0318, 0x0324, 0x0328, 
	0x0334, 0x0338, 0x033c, 0x0348, 0x0360, 0x03b0, 0x0410, 0x0420, 
	0x043c, 0x0440, 0x0444, 0x0448, 0x044c, 0x0454, 0x0458, 0x045c,
	0x0460, 0x0464, 0x0908, 0x0920, 0x0924, 0x0928, 0x092c, 0x0930, 
	0x0934, 0x0938, 0x093c, 0x0940, 0x0944, 0x0948, 0x094c, 0x0950, 
	0x0958, 0x095c, 0x0960, 0x0964, 0x0968, 0x096c, 0x0970, 0x0980, 
	0x0984, 0x0988, 0x098c, 0x0d68, 0x1720, 0x20c0, 0x20c4, 0x20c8,
	0x20cc, 0x20d0, 0x20d4, 0x20d8, 0x20dc, 0x20e0, 0x20e4, 0x20e8,
	0x20ec, 0x20f0, 0x20f4, 0x20f8, 0x20fc, 0x2100, 0x2104, 0x2108,
	0x2644, 0x26c0, 0x2c38, 0x2c54, 0x2c58, 0x2c5c, 0x2c6c, 0x2c70,
	0x2c74, 0x2c84, 0x2c88, 0x2c8c, 0x2d04, 0x2d08, 0x2d0c, 0x2d10,
	0x2d14, 0x2d18, 0x2d24, 0x2d28, 0x2d2c, 0x2dd0, 0x2dd4, 0x2dd8,
	0x2ddc, 0x2de0, 0x2e00, 0x2e04, 0x2e08, 0x2e0c, 0x2e10, 0x2e14,
	0x2e18, 0x2e1c, 0x2e20, 0x2e24, 0x2e28, 0x2e2c, 0x2e30, 0x2e34,
	0x2e38, 0x3224, 0x3228, 0x322c, 0x323c, 0x3240, 0x3248, 0x3450,
	0x3454, 0x2458
};
#endif


/*
 * Engine, FIFO control
 */

/*
**	Accelerator implementation
*/

static void radeon_chipset_accel_init(kgi_accel_t *accel, void *context)
{
	radeon_chipset_t *radeon = accel->meta;
	radeon_chipset_io_t *radeon_io = accel->meta_io;
	radeon_chipset_accel_context_t *radeon_ctx = context;
	radeon_chipset_mode_t *radeon_mode = radeon->mode;
	radeon_chipset_context_t *ctx;
	int i;

	KRN_ASSERT(radeon_mode);

	/* Anti-obfuscation hint: the data block pointed to by 
	 * *context has been zeroed, and then self-referential aperture 
	 * information has been placed in the first several words.
	 * Supposedly the block is DMA-capable, but careful: no AGP
	 * support in that part of the KGI core yet.
	 */

	ctx = &(radeon_ctx->state);

	/* Copy GC control structure from initial GC state. */
	ctx->gc_nregi       = radeon->init_gc.gc_nregi;
	ctx->gc_regi        = radeon->init_gc.gc_regi;
	ctx->gc_chip_nregi  = radeon->init_gc.gc_chip_nregi;
	ctx->gc_chip_regi   = radeon->init_gc.gc_chip_regi;
	ctx->mgc_nregi      = radeon->init_gc.mgc_nregi;
	ctx->mgc_regi       = radeon->init_gc.mgc_regi;
	ctx->mgc_chip_nregi = radeon->init_gc.mgc_chip_nregi;
	ctx->mgc_chip_regi  = radeon->init_gc.mgc_chip_regi;

	/* MGC data values are stored in the mode. */
	ctx->mgc_data       = radeon_mode->radeon.mgc_data;
	ctx->mgc_chip_data  = radeon_mode->radeon.mgc_chip_data;

	kgim_memcpy(ctx->gc_data, radeon_all_initdata_gc,
	       4 * RADEON_ALL_NUM_SAVEREGI_GC);	
	kgim_memcpy(ctx->gc_chip_data, radeon_r200_initdata_gc,
	       4 * RADEON_R200_NUM_SAVEREGI_GC);

	if (!accel->execution.context) {
		/* Initialize MGC */

		kgim_memcpy(ctx->mgc_data, radeon_all_initdata_mgc,
		       4 * RADEON_ALL_NUM_SAVEREGI_GC);
		kgim_memcpy(ctx->mgc_chip_data, 
		       radeon_r200_initdata_mgc,
		       4 * RADEON_R200_NUM_SAVEREGI_MGC);
			
		/* Initialise some bitfields in CRTC_GEN_CNTL.
		 * (MGC needs the register to turn doodads on/off)
		 * (Note assumes GC's device is mapped to display, but 
		 *  that should be a safe assumption... ?)
		 */		
		for (i = 0; i < RADEON_ALL_NUM_SAVEREGI_MGC; i++) {
			if (ctx->mgc_regi[i] == 0x50) {
				ctx->mgc_data[i] = 
				  radeon_mode->radeon.CrtcGenCntl;
				break;
			}
		}

		/* TODO: install correct default for 0x190 (VIDEOMUX_CNTL) */

	}

	/* PIO only for now. */
	RADEON_CTRL_OUT32(radeon_io, RADEON_CSQ_PRIPIO_INDDIS,
			RADEON_CP_CSQ_CNTL);
			
	KRN_DEBUG(2, "cp_csq_mode: 0x%.8x",
		RADEON_CTRL_IN32(radeon_io, RADEON_CP_CSQ_CNTL));

	KRN_DEBUG(2, "Radeon_chipset_accel_init");
}

static void radeon_chipset_accel_done(kgi_accel_t *accel, void *context)
{
	radeon_chipset_context_t *ctx;

	ctx = &(((radeon_chipset_accel_context_t *)context)->state);

	/* TODO: Stop all buffers and idle the accel. */

	if (context == accel->execution.context)
		accel->context = NULL;
	
}

/* Save the CLUT for 8bpp source textures. */
static inline void 
radeon_chipset_accel_save_sclut(radeon_chipset_io_t *io, 
				radeon_chipset_context_t *ctx)
{
	int i;


	for (i = 0; i < 256; i++) {
		RADEON_CTRL_OUT32(io, i, 0x1780 /* SRC_CLUT_ADDRESS */);
		ctx->sclut[i] = 
		  RADEON_CTRL_IN32(io, 0x1788 /* SRC_CLUT_DATA_RD */);
	}
}

/* Restore the CLUT for 8bpp source textures. */
static inline void 
radeon_chipset_accel_restore_sclut(radeon_chipset_io_t *io, 
				   radeon_chipset_context_t *ctx)
{
	int i;

	RADEON_CTRL_OUT32(io, 0, 0x1780 /* SRC_CLUT_ADDRESS */);
	for (i = 0; i < 256; i++) {
		RADEON_CTRL_OUT32(io, ctx->sclut[i], 0x1788 /*SRC_CLUT_DATA*/);
	}
}

/* Restore the TCL Vector RAM */
static inline void 
radeon_chipset_accel_restore_vectors(radeon_chipset_io_t *io, 
				     radeon_chipset_context_t *ctx)
{
	int i;

	RADEON_CTRL_OUT32(io, 0x00010000, 0x2200 /* SE_TCL_VECTOR_IDX_REG */);
	for (i = 0; i < RADEON_NUM_VECTORS * 4; i++) {
#if 0
	  /* TODO: Shadow these registers so we have data to restore */
		RADEON_CTRL_OUT32(io, ctx->vectors[i], 
				  0x2204 /*SE_TCL_VECTOR_DATA_REG */);

#endif	
	}
}

/* Restore the TCL Scalar RAM */
static inline void 
radeon_chipset_accel_restore_scalars(radeon_chipset_io_t *io, 
				     radeon_chipset_context_t *ctx)
{

	int i;

	RADEON_CTRL_OUT32(io, 0x00010000, 0x2208 /* SE_TCL_SCALAR_IDX_REG */);
	for (i = 0; i < RADEON_NUM_SCALARS; i++) {
#if 0
	  /* TODO: Shadow these registers so we have data to restore */
		RADEON_CTRL_OUT32(io, ctx->scalars[i], 
				  0x220c /*SE_TCL_SCALAR_DATA_REG */);
#endif
	}
}

static inline void 
radeon_chipset_accel_save(radeon_chipset_io_t *io, 
			  radeon_chipset_context_t *ctx)
{
	int i;

	for (i = 0; i < ctx->gc_nregi; i++) 
		ctx->gc_data[i] = 
		  RADEON_CTRL_IN32(io, ctx->gc_regi[i]);

	for (i = 0; i < ctx->gc_chip_nregi; i++)
		ctx->gc_chip_data[i] = 
		  RADEON_CTRL_IN32(io, ctx->gc_chip_regi[i]);

	radeon_chipset_accel_save_sclut(io, ctx);

	/* TODO: What does MC_INDEX/DATA do? */

}

static inline void 
radeon_chipset_accel_save_all(radeon_chipset_io_t *io, 
			      radeon_chipset_context_t *ctx)
{
	int i;

	for (i = 0; i < ctx->gc_nregi; i++) 
		ctx->gc_data[i] = RADEON_CTRL_IN32(io, ctx->gc_regi[i]);

	for (i = 0; i < ctx->gc_chip_nregi; i++)
		ctx->gc_chip_data[i] = 
		  RADEON_CTRL_IN32(io, ctx->gc_chip_regi[i]);

	for (i = 0; i < ctx->mgc_nregi; i++)
		ctx->mgc_data[i] = RADEON_CTRL_IN32(io, ctx->mgc_regi[i]);

	for (i = 0; i < ctx->mgc_chip_nregi; i++)
		ctx->mgc_chip_data[i] = 
		  RADEON_CTRL_IN32(io, ctx->mgc_chip_regi[i]);

	radeon_chipset_accel_save_sclut(io, ctx);

	/* TODO: What does MC_INDEX/DATA do? */

}

static inline void 
radeon_chipset_accel_restore(radeon_chipset_io_t *io, 
			     radeon_chipset_context_t *ctx)
{
	int i;

	radeon_chipset_accel_restore_sclut(io, ctx);
	radeon_chipset_accel_restore_vectors(io, ctx);
	radeon_chipset_accel_restore_scalars(io, ctx);

	/* TODO: What does MC_INDEX/DATA do? */

	for (i = 0; i < ctx->gc_nregi; i++)
		RADEON_CTRL_OUT32(io, ctx->gc_data[i], ctx->gc_regi[i]);

	for (i = 0; i < ctx->gc_chip_nregi; i++)
		RADEON_CTRL_OUT32(io, ctx->gc_chip_data[i], 
				  ctx->gc_chip_regi[i]);
}

static inline void 
radeon_chipset_accel_restore_all(radeon_chipset_io_t *io, 
				 radeon_chipset_context_t *ctx)
{
	int i;

	/* TODO: What does MC_INDEX do? */

	radeon_chipset_accel_restore_sclut(io, ctx);
	radeon_chipset_accel_restore_vectors(io, ctx);
	radeon_chipset_accel_restore_scalars(io, ctx);

	for (i = 0; i < ctx->gc_nregi; i++)
		RADEON_CTRL_OUT32(io, ctx->gc_data[i], ctx->gc_regi[i]);

	for (i = 0; i < ctx->gc_chip_nregi; i++)
		RADEON_CTRL_OUT32(io, ctx->gc_chip_data[i], 
				  ctx->gc_chip_regi[i]);

	for (i = 0; i < ctx->mgc_nregi; i++)
		RADEON_CTRL_OUT32(io, ctx->mgc_data[i], ctx->mgc_regi[i]);

	for (i = 0; i < ctx->mgc_chip_nregi; i++)
		RADEON_CTRL_OUT32(io, ctx->mgc_chip_data[i], 
				  ctx->mgc_chip_regi[i]);


}

static void radeon_chipset_accel_exec(kgi_accel_t *accel,
				      kgi_accel_buffer_t *buffer)
{
#warning check/validate validate data stream!!!
#warning this must not be interrupted!
  /*	radeon_chipset_t *radeon = accel->meta; */
	radeon_chipset_io_t *radeon_io = accel->meta_io;
	int i;
	
	KRN_ASSERT(KGI_AS_FILL == buffer->execution.state);
	KRN_DEBUG(2, "radeon_chipset_accel_exec");

	if (buffer->context != accel->execution.context) {
		radeon_chipset_accel_context_t *ctx;

		/* TODO: Stop all buffers and idle the accel. */

		ctx = accel->execution.context;
		if (ctx) radeon_chipset_accel_save(radeon_io, &(ctx->state));

		ctx = buffer->context;
		radeon_chipset_accel_restore(radeon_io, &(ctx->state));

		accel->execution.context = buffer->context;
	}

	buffer->execution.state = KGI_AS_WAIT;
/*	
	if (accel->execution.queue) {
		
		kgi_accel_buffer_t *queued = accel->execution.queue;
		kgi_accel_buffer_t *next   = queued->execution.next;
		
		while (next && (next->priority >= buffer->priority)) {
			
			queued = next;
			next   = queued->execution.next;
		}
		
		buffer->execution.next = next;
		queued->execution.next = buffer;
		
		return;
	}
	else {
		
		buffer->execution.next = NULL;
		accel->execution.queue = buffer;
		radeon_chipset_accel_schedule(accel);
	}
*/

	/* Push the buffer into the engine dword at a time. */
	for (i = 0; i < buffer->execution.size/4; ++i) {

		RADEON_CTRL_OUT32(radeon_io, 
			((kgi_u32_t*)buffer->aperture.virt)[i],
			RADEON_CP_CSQ_APER_PRIMARYBegin);
	}

	/* And this buffer is done. */
	buffer->execution.state = KGI_AS_IDLE;
    
}

/*
**	Clock
*/
static kgi_error_t radeon_clock_best_mode(radeon_chipset_t *radeon,
	radeon_chipset_mode_t *radeon_mode, kgi_u_t f)
{
	static const kgi_u_t post_div_table[] =
		{ 1, 2, 3, 4, 6, 8, 12 };
	static const kgi_u32_t post_div_bits[] =
		{ 0x0, 0x1, 0x4, 0x2, 0x6, 0x3, 0x7 };
	static const kgi_u_t num_post_divs =
		sizeof(post_div_table)/sizeof(kgi_u_t);
	kgi_u_t i;
	kgi_u32_t temp = 0;
	
	KRN_DEBUG(2, "Finding best mode");
	
	/* In order to avoid overflow, work with kHz/10 */
	f /= 10000;
	
	if (f > radeon->fvco.max) {
		
		f = radeon->fvco.max;
	}
		
	if (f * post_div_table[num_post_divs-1] < radeon->fvco.min) {
		
		f = radeon->fvco.min / post_div_table[num_post_divs-1];
	}
		
	for (i = 0; i < num_post_divs; i++) {
	
		temp = post_div_table[i] * f;
		if ((temp <= radeon->fvco.max) && (temp >= radeon->fvco.min)) 
			break;
	}
	
	KRN_ASSERT(i < num_post_divs);	
	
	/* Add half the denominator to the numerator to round off division */
	KRN_DEBUG(2,"fb_div = (%d*%d)/%d",radeon->ref_div,temp,radeon->fref);
	radeon_mode->radeon.feedback_divider = 
		(radeon->ref_div * temp + (radeon->fref/2)) / radeon->fref;

	KRN_DEBUG(2,"i=%d-->post_div = %d (bits=%.1x)",i,post_div_table[i],
		post_div_bits[i]);
	radeon_mode->radeon.post_divider = post_div_bits[i];

	KRN_DEBUG(2,"f = (%d*%d)/(%d*%d)",
		  radeon->fref, radeon_mode->radeon.feedback_divider,
		  radeon->ref_div, post_div_table[i]);
	radeon_mode->radeon.dclk = 
		(radeon->fref * radeon_mode->radeon.feedback_divider) /
		(radeon->ref_div * post_div_table[i]) * 10000;

	return KGI_EOK;
}

/*
**	IRQ Handler
*/

kgi_error_t radeon_chipset_irq_handler(radeon_chipset_t *radeon, 
	radeon_chipset_io_t *radeon_io, irq_system_t *system)
{
	kgi_u32_t handled = 0;
#warning read IRQ status flags here.
	kgi_u32_t flags = 0;

	KRN_ASSERT(radeon);
	KRN_ASSERT(radeon_io);

	if (flags & ~handled) {

		KRN_ERROR("unhandled interrupt flag(s) %.8x (pcidev %.8x)",
			flags & ~handled, RADEON_PCIDEV(radeon_io));
	}

	if (! flags) {

		KRN_ERROR("interrupt but no reason indicated.");
	}

#warning clean handled interrupts here.

	return KGI_EOK;
}

/* Try to read the pll parameters from the ROM */
static kgi_error_t radeon_chipset_read_rom(radeon_chipset_t *radeon,
	radeon_chipset_io_t *radeon_io)
{
	const kgi_u8_t *ati_sig = "761295520";
	const kgi_u_t ati_sig_len = 9;
	kgi_u32_t seg_start;
	mem_region_t rom;
	const struct {
		kgi_u8_t *sig;
		kgi_u_t len;
	} radeon_sigs[] = {
		{ "RG6", 3 },
		{ "RADEON", 6 }
	};

	memset(&rom, 0, sizeof(rom));
	
	rom.name      = "Radeon rom";
	rom.device    = RADEON_PCIDEV(radeon_io);
	rom.size      = 0x1000;
	rom.decode    = MEM_DECODE_ALL;

	for (seg_start = 0x0C0000; seg_start < 0x0F0000; seg_start += 0x01000) {

		kgi_u8_t *p;
		kgi_u_t i;

		rom.base_virt = MEM_NULL;
		rom.base_io = seg_start;
		rom.rid = 1;

#if (HOST_OS == HOST_OS_FreeBSD)
		rom.base_virt = (mem_vaddr_t)kgi_map_buffer(rom.base_io, rom.size);

#else
		mem_claim_region(&rom);
#endif
		p = (kgi_u8_t *)rom.base_virt;
		
		if ((*p == 0x55) || (((*(p + 1)) & 0xFF) == 0xAA))
			goto find_ati_sig;
			
		goto not_found;
		
	find_ati_sig:
		KRN_DEBUG(1, "Found 0x55AA at 0x%p", p);
		p = (kgi_u8_t *)rom.base_virt;
		
		for (i = 0; i < 118; i++) {
		
			if (*p == ati_sig[0])
				if (kgim_memcmp(ati_sig, p, ati_sig_len) == 0)
					goto find_radeon_sig;
			p++;
		}
		goto not_found;
		
	find_radeon_sig:
		KRN_DEBUG(1, "Found ATI signature at 0x%p", p);
		p = (kgi_u8_t *)rom.base_virt;
		
		for (i = 0; i < 512; i++) {
		
			kgi_u_t n;
			for (n = 0; n < (sizeof(radeon_sigs) / 
				sizeof(radeon_sigs[0])); n++) {
			
				if (*p == radeon_sigs[n].sig[0]) {
				
					if (kgim_memcmp(radeon_sigs[n].sig, p, 
						radeon_sigs[n].len) == 0) {
						
						goto found;
					}
				}
			}
			p++;
		}
		goto not_found;	
		
	found:
		KRN_DEBUG(1, "Found Radeon signature at 0x%p", p);
		{
		struct {
		
			kgi_u8_t	clock_chip_type;
			kgi_u8_t	struct_size;
			kgi_u8_t	accel_entry;
			kgi_u8_t	vga_entry;
			kgi_u16_t	vga_table_offset;
			kgi_u16_t	post_table_offset;
			kgi_u16_t	xclk;
			kgi_u16_t	mclk;
			kgi_u8_t	num_pll_blocks;
			kgi_u8_t	size_pll_blocks;
			kgi_u16_t	pclk_ref_freq;
			kgi_u16_t	pclk_ref_divider;
			kgi_u32_t	pclk_min_freq;
			kgi_u32_t	pclk_max_freq;
			kgi_u16_t	mclk_ref_freq;
			kgi_u16_t	mclk_ref_divider;
			kgi_u32_t	mclk_min_freq;
			kgi_u32_t	mclk_max_freq;
			kgi_u16_t	xclk_ref_freq;
			kgi_u16_t	xclk_ref_divider;
			kgi_u32_t	xclk_min_freq;
			kgi_u32_t	xclk_max_freq;

		} __attribute__ ((packed)) *pll_info;
		
		kgi_u32_t addr;
		void *ptr;
		kgi_u16_t bios_header_offset, pll_info_offset;
		
		addr = (kgi_u32_t)rom.base_virt + 0x48;
		ptr = (void*)addr;
		bios_header_offset = mem_in16((mem_vaddr_t)ptr);
		
		addr = (kgi_u32_t)rom.base_virt + (kgi_u32_t)bios_header_offset + 0x30;
		ptr = (void*)addr;
		pll_info_offset = mem_in16((mem_vaddr_t)ptr);
		
		addr = (kgi_u32_t)rom.base_virt + (kgi_u32_t)pll_info_offset;
		ptr = (void*)addr;
		pll_info = ptr;
#warning should use mem_in to read values
		/* All frequencies from pll_info are in 10 KHz. */
		radeon->fref     = pll_info->pclk_ref_freq;
		radeon->fvco.min = pll_info->pclk_min_freq;
		radeon->fvco.max = pll_info->pclk_max_freq;
		radeon->ref_div  = pll_info->pclk_ref_divider;
#if 1
		KRN_DEBUG(1, "PLL Information");
		KRN_DEBUG(1, "clock_chip_type   =%d", pll_info->clock_chip_type);
		KRN_DEBUG(1, "struct_size       =%d", pll_info->struct_size);
		KRN_DEBUG(1, "accel_entry       =%d", pll_info->accel_entry);
		KRN_DEBUG(1, "vga_entry         =%d", pll_info->vga_entry);
		KRN_DEBUG(1, "vga_table_offset  =%d", pll_info->vga_table_offset);
		KRN_DEBUG(1, "post_table_offset =%d", pll_info->post_table_offset);
		KRN_DEBUG(1, "xclk              =%d", pll_info->xclk);
		KRN_DEBUG(1, "mclk              =%d", pll_info->mclk);
		KRN_DEBUG(1, "num_pll_blocks    =%d", pll_info->num_pll_blocks);
		KRN_DEBUG(1, "size_pll_blocks   =%d", pll_info->size_pll_blocks);
		KRN_DEBUG(1, "pclk_ref_freq     =%d", pll_info->pclk_ref_freq);
		KRN_DEBUG(1, "pclk_ref_divider  =%d", pll_info->pclk_ref_divider);
		KRN_DEBUG(1, "pclk_min_freq     =%d", pll_info->pclk_min_freq);
		KRN_DEBUG(1, "pclk_max_freq     =%d", pll_info->pclk_max_freq);
		KRN_DEBUG(1, "mclk_ref_freq     =%d", pll_info->mclk_ref_freq);
		KRN_DEBUG(1, "mclk_ref_divider  =%d", pll_info->mclk_ref_divider);
		KRN_DEBUG(1, "mclk_min_freq     =%d", pll_info->mclk_min_freq);
		KRN_DEBUG(1, "mclk_max_freq     =%d", pll_info->mclk_max_freq);
		KRN_DEBUG(1, "xclk_ref_freq     =%d", pll_info->xclk_ref_freq);
		KRN_DEBUG(1, "xclk_ref_divider  =%d", pll_info->xclk_ref_divider);
		KRN_DEBUG(1, "xclk_min_freq     =%d", pll_info->xclk_min_freq);
		KRN_DEBUG(1, "xclk_max_freq     =%d", pll_info->xclk_max_freq);
#endif
		}
#if (HOST_OS == HOST_OS_FreeBSD)
		kgi_unmap_buffer(rom.base_virt, rom.size);
#else
		mem_free_region(&rom);
#endif
		return -KGI_EOK;
		
	not_found:
#if (HOST_OS == HOST_OS_FreeBSD)
		kgi_unmap_buffer(rom.base_virt, rom.size);
#else
		mem_free_region(&rom);
#endif
	}
	
	KRN_ERROR("No ATI Radeon ROM found");
	return -KGI_ERRNO(CHIPSET, INVAL);
}

kgi_error_t radeon_chipset_init(radeon_chipset_t *radeon,
	radeon_chipset_io_t *radeon_io, const kgim_options_t *options)
{
  /*	pcicfg_vaddr_t pcidev = RADEON_PCIDEV(radeon_io); */

	KRN_ASSERT(radeon);
	KRN_ASSERT(radeon_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "initializing %s %s",
		radeon->chipset.vendor, radeon->chipset.model);
/*
#warning	You might have to set PCICFG base registers here...

	PCICFG_SET_BASE32(radeon_io->io.base_io, pcidev+PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE32(radeon_io->mem.base_io, pcidev+PCI_BASE_ADDRESS_1);
	PCICFG_SET_BASE32(radeon_io->rom.base_io | PCI_ROM_ADDRESS_ENABLE,
		pcidev + PCI_ROM_ADDRESS);
*/
	KRN_DEBUG(2, "PCI (re-)configuration done");

#warning	enable chipset here

	/* save all the registers we mess with */	
	radeon->Ctrl.CrtcGenCntl = 
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_GEN_CNTL);
	radeon->Ctrl.CrtcExtCntl =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_EXT_CNTL);
	radeon->Ctrl.DacCntl =
		RADEON_CTRL_IN32(radeon_io, RADEON_DAC_CNTL);
	radeon->Ctrl.CrtcHTotalDisp =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_H_TOTAL_DISP);
	radeon->Ctrl.CrtcHSyncStrtWid =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_H_SYNC_STRT_WID);
	radeon->Ctrl.CrtcVTotalDisp =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_V_TOTAL_DISP);
	radeon->Ctrl.CrtcVSyncStrtWid =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_V_SYNC_STRT_WID);
	radeon->Ctrl.CrtcOffset =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_OFFSET);
	radeon->Ctrl.CrtcOffsetCntl =
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_OFFSET_CNTL);
	radeon->Ctrl.CrtcPitch = 
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_PITCH);

	radeon->init_gc.gc_nregi = RADEON_ALL_NUM_SAVEREGI_GC;
	radeon->init_gc.mgc_nregi = RADEON_ALL_NUM_SAVEREGI_MGC;
	radeon->init_gc.gc_regi = radeon_all_saveregi_gc;
	radeon->init_gc.mgc_regi = radeon_all_saveregi_mgc;

	/* TODO: Choose based on chipset model */
	radeon->init_gc.gc_chip_nregi = RADEON_R200_NUM_SAVEREGI_GC;
	radeon->init_gc.mgc_chip_nregi = RADEON_R200_NUM_SAVEREGI_MGC;
	radeon->init_gc.gc_chip_regi = radeon_r200_saveregi_gc;
	radeon->init_gc.mgc_chip_regi = radeon_r200_saveregi_mgc;

	radeon->init_gc.mgc_data = radeon->init_data_mgc;
	radeon->init_gc.mgc_chip_data = radeon->init_data_chip_mgc;
	radeon_chipset_accel_save_all(radeon_io, &(radeon->init_gc));

	KRN_DEBUG(2, "chipset enabled");

	radeon->chipset.memory = options->chipset->memory
		? options->chipset->memory
		: RADEON_CTRL_IN32(radeon_io, RADEON_CONFIG_MEMSIZE) &
		  RADEON_CONFIG_MEMSIZEMask;

	KRN_DEBUG(2, "detected %d bytes (%d MB) of video memory", 
		radeon->chipset.memory, radeon->chipset.memory>>20);

	if (radeon_chipset_read_rom(radeon, radeon_io) != KGI_EOK) {
	
		radeon->fref = KGIM_DEFAULT(options->clock->fref, 2700);
		radeon->fvco.min = KGIM_DEFAULT(options->clock->fvco_min, 12000);
		radeon->fvco.max = KGIM_DEFAULT(options->clock->fvco_max, 35000);
		radeon->ref_div = 60;
	}

	vga_text_chipset_init(&radeon->vga, &radeon_io->vga, options);

	if (radeon->flags & RADEON_CF_IRQ_CLAIMED) {

		KRN_DEBUG(2, "enabling interrupts");
	}

	return KGI_EOK;
}

void radeon_chipset_done(radeon_chipset_t *radeon, 
			 radeon_chipset_io_t *radeon_io,
			 const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = RADEON_PCIDEV(radeon_io);

	if (radeon->flags & RADEON_CF_IRQ_CLAIMED) {

	}

	radeon_chipset_accel_restore_all(radeon_io, &(radeon->init_gc));

	/* put our toys back now that we're done playing with them */
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcGenCntl, 
		RADEON_CRTC_GEN_CNTL);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcExtCntl,
		RADEON_CRTC_EXT_CNTL);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.DacCntl,
		RADEON_DAC_CNTL);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcHTotalDisp, 
		RADEON_CRTC_H_TOTAL_DISP);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcHSyncStrtWid, 
		RADEON_CRTC_H_SYNC_STRT_WID);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcVTotalDisp,
		RADEON_CRTC_V_TOTAL_DISP);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcVSyncStrtWid,
		RADEON_CRTC_V_SYNC_STRT_WID);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcOffset, 
		RADEON_CRTC_OFFSET);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcOffsetCntl, 
		RADEON_CRTC_OFFSET_CNTL);
	RADEON_CTRL_OUT32(radeon_io, radeon->Ctrl.CrtcPitch,
		RADEON_CRTC_PITCH);

	vga_text_chipset_done(&radeon->vga, &radeon_io->vga, options);

#warning	restore initial pcicfg base addresses here.
	PCICFG_SET_BASE32(radeon->pci.BaseAddr0, pcidev+PCI_BASE_ADDRESS_0);
	PCICFG_SET_BASE32(radeon->pci.BaseAddr1, pcidev+PCI_BASE_ADDRESS_1);
}

kgi_error_t radeon_chipset_mode_check(radeon_chipset_t *radeon,
	radeon_chipset_io_t *radeon_io,
	radeon_chipset_mode_t *radeon_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = radeon_mode->kgim.crt;
	kgi_u_t bpf, bpc, bpp, width;
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_u_t i, bpd;

	struct {
		kgi_u8_t ilut;
		kgi_u8_t alut;

		kgi_attribute_mask_t dam;
		const kgi_u8_t       *bpda;

		kgi_u32_t crtc_pix_width;
	} *pixfmt, pixfmts[] = {
		{ 1, 0, KGI_AM_I,                      KGI_AS_4,    1 },
		{ 1, 0, KGI_AM_I,                      KGI_AS_8,    2 },
		{ 0, 1, KGI_AM_RGBX,                   KGI_AS_5551, 3 },
		{ 0, 1, KGI_AM_RGBA,                   KGI_AS_5551, 3 },
		{ 0, 1, KGI_AM_RGB,                    KGI_AS_565,  4 },
		{ 0, 1, KGI_AM_RGB,                    KGI_AS_888,  5 },
		{ 0, 1, KGI_AM_RGBX,                   KGI_AS_8888, 6 },
		{ 0, 1, KGI_AM_RGBA,                   KGI_AS_8888, 6 },
		{ 0, 1, KGI_AM_RGBX,                   KGI_AS_4444, 7 },
		{ 0, 1, KGI_AM_RGBA,                   KGI_AS_4444, 7 },
		{ 0, 1, KGI_AM_I | KGI_AM_APPLICATION, KGI_AS_88, 8 },
		{ 0, 1, KGI_AM_I | KGI_AM_ALPHA,       KGI_AS_88, 8 },
	};		
	
	/* 
	 * Integrated ramdac means no fancy clock scaling, ever.
	 */
	dpm->lclk.mul = dpm->lclk.div = 1;
	dpm->rclk.mul = dpm->rclk.div = 1;

	
	if (images != 1) {

		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	if (img[0].frames == 0) {

		img[0].frames = 1;
	}
	
	if (img[0].frames != 1) {

		KRN_DEBUG(1, "multiple frames not supported");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
		(img[0].fam & KGI_AM_TEXTURE_INDEX)) {

		if (cmd == KGI_TC_PROPOSE) {

			dpm->dam = KGI_AM_TEXT;
			dpm->bpda = KGI_AS_448;
			dpm->flags = KGI_DPF_CH_ILUT;
		}
		
		return vga_text_chipset_mode_check(&radeon->vga,
		        &radeon_io->vga, &radeon_mode->vga, cmd, img, images);
	}
	
	if (img[0].flags & (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL)) {

		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	if (img[0].cam) {

		KRN_DEBUG(1, "common attributes %.8x not supported",
			  img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
		

	/*
	 * Find the right pixel format
	 */
	pixfmt = NULL;
	
	for (i = 0; i < sizeof(pixfmts)/sizeof(pixfmts[0]); ++i) {

		if (pixfmts[i].dam == img[0].fam &&
		    kgim_strcmp(pixfmts[i].bpda, img[0].bpfa) == 0) {

			pixfmt = &pixfmts[i];
		}
	}

	if (pixfmt == NULL) {

		KRN_DEBUG(1, "could not handle dot attributes");
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	/*	total bits per dot
	*/
	bpf = kgim_attr_bits(img[0].bpfa);
	bpc = kgim_attr_bits(img[0].bpca);
	bpd = kgim_attr_bits(dpm->bpda);
	bpp = (img[0].flags & KGI_IF_STEREO)
		? (bpc + bpf*img[0].frames*2)
		: (bpc + bpf*img[0].frames);

	switch (cmd) {

	case KGI_TC_PROPOSE:

		KRN_ASSERT(img[0].frames);
		KRN_ASSERT(bpp);

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

			img[0].virt.x = (8 * radeon->chipset.memory) / 
				(img[0].virt.y * bpp);

			if (img[0].virt.x > radeon->chipset.maxdots.x) {

				img[0].virt.x = radeon->chipset.maxdots.x;
			}
		}
#warning	restrict img[0].virt.x to supported width here.

		if (0 == img[0].virt.y) {

			img[0].virt.y = (8 * radeon->chipset.memory) /
				(img[0].virt.x * bpp);
		}

		/*	Are we beyond the limits of the H/W?
		*/
		if ((img[0].size.x >= radeon->chipset.maxdots.x) || 
			(img[0].virt.x >= radeon->chipset.maxdots.x)) {

			KRN_DEBUG(1, "%i (%i) horizontal pixels are too many",
				img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].size.y >= radeon->chipset.maxdots.y) ||
			(img[0].virt.y >= radeon->chipset.maxdots.y)) {

			KRN_DEBUG(1, "%i (%i) vertical pixels are too many",
				img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if ((img[0].virt.x * img[0].virt.y * bpp) > 
			(8 * radeon->chipset.memory)) {

			KRN_DEBUG(1, "not enough memory for (%ipf*%if + %ipc)@"
				"%ix%i", bpf, img[0].frames, bpc,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}

		if (0 == img[0].frames) {

			img[0].frames = 1;
		}


#warning	Take screen visible width up to next supported width

		/*	set CRT visible fields
		*/
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;

		dpm->flags &= ~KGI_DPF_TP_2XCLOCK;
		dpm->flags |= pixfmt->alut ? KGI_DPF_CH_ALUT : 0;
		dpm->flags |= pixfmt->ilut ? KGI_DPF_CH_ILUT : 0;

		dpm->dam  = pixfmt->dam;
		dpm->bpda = pixfmt->bpda;

		if (img[0].size.y < 400) {

			dpm->dots.y += img[0].size.y;
		}
		return KGI_EOK;

	case KGI_TC_LOWER:

		if (dpm->dclk < radeon->chipset.dclk.min) {
			
			KRN_DEBUG(1, "DCLK = %i Hz is too low",
				  dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if (dpm->dclk > radeon->chipset.dclk.max) {

			dpm->dclk = radeon->chipset.dclk.max;
		}

		if (radeon_clock_best_mode(radeon, radeon_mode, dpm->dclk)){

			KRN_DEBUG(1, "lower failed: DCLK = %i Hz", dpm->dclk);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		dpm->dclk = radeon_mode->radeon.dclk;

		return KGI_EOK;

	case KGI_TC_RAISE:
			
		if (dpm->dclk > radeon->chipset.dclk.max) {
			
			KRN_DEBUG(1, "DCLK = %i Hz is too high",
				  dpm->dclk);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}

		if (dpm->dclk < radeon->chipset.dclk.min) {

			dpm->dclk = radeon->chipset.dclk.min;
		}

		if (radeon_clock_best_mode(radeon, radeon_mode, dpm->dclk)) {

			KRN_DEBUG(1, "raise failed: DCLK = %i Hz", dpm->dclk);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		dpm->dclk = radeon_mode->radeon.dclk;

		return KGI_EOK;

	case KGI_TC_CHECK:

#warning Do proper checking of hardware limits etc. here.

		width = radeon_next_supported_width(img[0].virt.x);

		if (width != img[0].virt.x) {

			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((img[0].size.x >= radeon->chipset.maxdots.x) ||
			(img[0].size.y >= radeon->chipset.maxdots.y) || 
			(img[0].virt.x >= radeon->chipset.maxdots.x) ||
			((img[0].virt.y * img[0].virt.x * bpp) >
				(8 * radeon->chipset.memory))) {

			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
				img[0].size.x, img[0].size.y,
				img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}

		if (radeon_clock_best_mode(radeon, radeon_mode, dpm->dclk)) {

			KRN_DEBUG(1, "clock check failed: DCLK = %i Hz",
				dpm->dclk);
			return -KGI_ERRNO(CLOCK, UNKNOWN);
		}

		break;

	default:
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}


	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
	radeon_mode->radeon.DacCntl =
		(bpp == 8 ? RADEON_DAC_8BIT_EN : 0) |
		0xFF << RADEON_DAC_MASKShift |
		RADEON_DAC_VGA_ADR_EN;
		
	radeon_mode->radeon.CrtcGenCntl =
		RADEON_CRTC_EXT_DISP_EN | RADEON_CRTC_EN |
		pixfmt->crtc_pix_width << RADEON_CRTC_PIX_WIDTHShift;
	
	radeon_mode->radeon.CrtcHTotalDisp = 
		((crt_mode->x.width/8)-1) << RADEON_CRTC_H_DISPShift |
		((crt_mode->x.total/8)-1) << RADEON_CRTC_H_TOTALShift,

	radeon_mode->radeon.CrtcHSyncStrtWid =
		(crt_mode->x.syncstart/8) << RADEON_CRTC_H_SYNC_STRT_CHARShift |
		((crt_mode->x.syncend - crt_mode->x.syncstart)/8) << RADEON_CRTC_H_SYNC_WIDShift |
		(crt_mode->x.polarity ? 0 : RADEON_CRTC_H_SYNC_POL);

	radeon_mode->radeon.CrtcVTotalDisp =
		(crt_mode->y.width - 1) << RADEON_CRTC_V_DISPShift |
		(crt_mode->y.total - 1) << RADEON_CRTC_V_TOTALShift;

	radeon_mode->radeon.CrtcVSyncStrtWid = 
		crt_mode->y.syncstart << RADEON_CRTC_V_SYNC_STRTShift |
		(crt_mode->y.syncend - crt_mode->y.syncstart) << RADEON_CRTC_V_SYNC_WIDShift |
		(crt_mode->y.polarity ? 0 : RADEON_CRTC_V_SYNC_POL);
		
	radeon_mode->radeon.CrtcPitch = img[0].size.x / 8;
	
	radeon_mode->radeon.DefaultPitchOffset = 
		((((img[0].virt.x * bpp/8) + 63) & ~63) / 64) << 
		RADEON_DEFAULT_PITCHShift;

	/*	initialize exported resources
	*/
	r = &radeon_mode->radeon.video_memory_aperture;
	r->meta     = radeon;
	r->meta_io  = radeon_io;
	r->type     = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot     = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name     = "Radeon video memory aperture";
	r->access   = 64 + 32 + 16 + 8;
	r->align    = 64 + 32 + 16 + 8;
	r->size     = r->win.size = radeon->chipset.memory;
	r->win.bus  = radeon_io->fb.base_bus;
	r->win.phys = radeon_io->fb.base_phys;
	r->win.virt = radeon_io->fb.base_virt;

	a = &radeon_mode->radeon.accelerator;
	a->meta = radeon;
	a->meta_io = radeon_io;
	a->type = KGI_RT_ACCELERATOR;
	a->prot = KGI_PF_LIB | KGI_PF_DRV; 
	a->name = "Radeon graphics processor";
	/* a->flags |= KGI_AF_DMA_BUFFERS; */
	a->buffers = 3;
	a->buffer_size = 4096;
	a->context_size = sizeof(radeon_chipset_accel_context_t);
	a->execution.context = NULL;
	a->execution.queue = NULL;	
	a->Init = radeon_chipset_accel_init;
	a->Done = radeon_chipset_accel_done;
	a->Exec = radeon_chipset_accel_exec;

	/* Set up the MGC */
	radeon_mode->radeon.mgc.gc_nregi = radeon->init_gc.gc_nregi;
	radeon_mode->radeon.mgc.gc_regi = radeon->init_gc.gc_regi;
	radeon_mode->radeon.mgc.gc_chip_nregi = radeon->init_gc.gc_chip_nregi;
	radeon_mode->radeon.mgc.gc_chip_regi = radeon->init_gc.gc_chip_regi;
	radeon_mode->radeon.mgc.mgc_nregi = radeon->init_gc.mgc_nregi;
	radeon_mode->radeon.mgc.mgc_regi = radeon->init_gc.mgc_regi;
	radeon_mode->radeon.mgc.mgc_chip_nregi = 
	  radeon->init_gc.mgc_chip_nregi;
	radeon_mode->radeon.mgc.mgc_chip_regi = radeon->init_gc.mgc_chip_regi;
	radeon_mode->radeon.mgc.mgc_data = radeon_mode->radeon.mgc_data;
	radeon_mode->radeon.mgc.mgc_chip_data = 
	  radeon_mode->radeon.mgc_chip_data;

	/* We keep floating state until an accel is actually initialized. */
	kgim_memcpy(radeon_mode->radeon.mgc.gc_data, 
	       radeon->init_gc.gc_data, 4 * radeon->init_gc.gc_nregi);
	kgim_memcpy(radeon_mode->radeon.mgc.gc_chip_data, 
	       radeon->init_gc.gc_chip_data, 
	       4 * radeon->init_gc.gc_chip_nregi);
	kgim_memcpy(radeon_mode->radeon.mgc_data, 
	       radeon->init_gc.mgc_data, 4 * radeon->init_gc.mgc_nregi);
	kgim_memcpy(radeon_mode->radeon.mgc_chip_data, 
	       radeon->init_gc.mgc_chip_data, 
	       4 * radeon->init_gc.mgc_chip_nregi);

	/* Initialise some bitfields in CRTC_GEN_CNTL.
	 * (MGC needs the register to turn doodads on/off)
	 * (Note assumes GC's device is mapped to display, but 
	 *  that should be a safe assumption... ?)
	 */
	for (i = 0; i < RADEON_ALL_NUM_SAVEREGI_MGC; i++) {
		if (radeon_mode->radeon.mgc.mgc_regi[i] == 0x50) {
			radeon_mode->radeon.mgc_data[i] = 
			  radeon_mode->radeon.CrtcGenCntl;
			break;
		}
	}

	return KGI_EOK;
}

kgi_resource_t *radeon_chipset_mode_resource(radeon_chipset_t *radeon, 
	radeon_chipset_mode_t *radeon_mode, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		return vga_text_chipset_mode_resource(&radeon->vga, 
			&radeon_mode->vga, img, images, index);
	}

	switch (index) {

	case 0:
		return (kgi_resource_t *) &radeon_mode->radeon.video_memory_aperture;
	case 1:
		return (kgi_resource_t *) &radeon_mode->radeon.accelerator;
	}
	return NULL;
}

kgi_resource_t *radeon_chipset_image_resource(radeon_chipset_t *radeon, 
	radeon_chipset_mode_t *radeon_mode, 
	kgi_image_mode_t *img, kgi_u_t image, kgi_u_t index)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX)
		return vga_text_chipset_image_resource(&radeon->vga,
						       &radeon_mode->vga,
						       img, image, index);
	
	return NULL;
}

void radeon_chipset_mode_prepare(radeon_chipset_t *radeon, radeon_chipset_io_t *radeon_io,
	radeon_chipset_mode_t *radeon_mode, kgi_image_mode_t *img, kgi_u_t images)
{
	radeon_wait_for_idle(radeon_io);

	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		kgi_u32_t val;

		val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_GEN_CNTL);
		val &= ~RADEON_CRTC_EXT_DISP_EN;
		RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_GEN_CNTL);

		vga_text_chipset_mode_prepare(&radeon->vga, &radeon_io->vga,
			&radeon_mode->vga, img, images);

		return;
	}
	
	/* disable the display before programming CRTC regs */
	RADEON_CTRL_OUT32(radeon_io, 
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_EXT_CNTL) |
		RADEON_CRTC_HSYNC_DIS | RADEON_CRTC_VSYNC_DIS |
		RADEON_CRTC_DISPLAY_DIS,
		RADEON_CRTC_EXT_CNTL);
		
	/* clear common registers */
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_OVR_CLR);
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_OVR_WID_LEFT_RIGHT);
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_OVR_WID_TOP_BOTTOM);
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_OV0_SCALE_CNTL);
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_SUBPIC_CNTL);
	if (RADEON_CHIP_M6 == radeon->chip) {
	
		RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_DVI_I2C_CNTL_1);
	}
	else {
	
		RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_VIPH_CONTROL);
		RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_I2C_CNTL_1);
	}
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_GEN_INT_CNTL);
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_CAP0_TRIG_CNTL);
	
	KRN_DEBUG(2, "prepared for radeon mode");
}

void radeon_chipset_mode_enter(radeon_chipset_t *radeon, 
	radeon_chipset_io_t *radeon_io, radeon_chipset_mode_t *radeon_mode, 
	kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u32_t val;
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) {

		vga_text_chipset_mode_enter(&radeon->vga, &radeon_io->vga,
			&radeon_mode->vga, img, images);

		/* 
		 * VGA CRTC registers mirror the radeon CRTC registers but
		 * have fewer bits than the radeon registers so if the most
		 * significant bits are set before setting VGA mode, they
		 * will still be set afterwards. Clear those bits.
		 */
		val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_H_TOTAL_DISP);
		val &= ~0x01000100;
		RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_H_TOTAL_DISP);
		val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_H_SYNC_STRT_WID);
		val &= ~0x00201800;
		RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_H_SYNC_STRT_WID);
		val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_V_TOTAL_DISP);
		val &= ~0x0c000c00;
		RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_V_TOTAL_DISP);
		val = RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_V_SYNC_STRT_WID);
		val &= ~0x00100c00;
		RADEON_CTRL_OUT32(radeon_io, val, RADEON_CRTC_V_SYNC_STRT_WID);

		return;
	}

	/*
	 * First set clock related registers
	 */

	/* select clock 3 */
	RADEON_CTRL_OUT32(radeon_io,
		RADEON_CTRL_IN32(radeon_io, RADEON_CLOCK_CNTL_INDEX) |
		0x00000003 << RADEON_PPLL_DIV_SELShift,
		RADEON_CLOCK_CNTL_INDEX);
	
	/* reset */
	val = RADEON_CLOCK_IN32(radeon_io, RADEON_PPLL_CNTL);
	val |= RADEON_PPLL_RESET;
	val &= ~RADEON_PPLL_ATOMIC_UPDATE_EN;
	val &= ~RADEON_PPLL_VGA_ATOMIC_UPDATE_EN;
	RADEON_CLOCK_OUT32(radeon_io, val, RADEON_PPLL_CNTL);
		
	/* set reference divider */	
	val = RADEON_CLOCK_IN32(radeon_io, RADEON_PPLL_REF_DIV);
	val &= ~(RADEON_PPLL_REF_DIVMask);	
	val |= radeon->ref_div << RADEON_PPLL_REF_DIVShift;
	RADEON_CLOCK_OUT32(radeon_io, val, RADEON_PPLL_REF_DIV);
	
	/* set feedback divider */
	val = RADEON_CLOCK_IN32(radeon_io, RADEON_PPLL_DIV_3);
	val &= ~(RADEON_PPLL_FB3_DIVMask);
	val |= radeon_mode->radeon.feedback_divider << RADEON_PPLL_FB3_DIVShift;
	/* set post divider */
	val &= ~(RADEON_PPLL_POST3_DIVMask);
	val |= radeon_mode->radeon.post_divider << RADEON_PPLL_POST3_DIVShift;
	RADEON_CLOCK_OUT32(radeon_io, val, RADEON_PPLL_DIV_3);
		
	/* clear HTOTAL_CNTL */
	RADEON_CLOCK_OUT32(radeon_io, 0x0, RADEON_HTOTAL_CNTL);
	
	val = RADEON_CLOCK_IN32(radeon_io, RADEON_PPLL_CNTL);
	val &= ~(RADEON_PPLL_RESET);
	RADEON_CLOCK_OUT32(radeon_io, val, RADEON_PPLL_CNTL);

	/*
	 * Setup RAMDAC related registers
	 */

	/* whatever? */
	RADEON_CTRL_OUT32(radeon_io,
		(RADEON_CTRL_IN32(radeon_io, RADEON_DAC_CNTL) &
		(RADEON_DAC_8BIT_EN | 
		(RADEON_DAC_RANGE_CNTLMask << RADEON_DAC_RANGE_CNTLShift) |
		RADEON_DAC_BLANKING)) | radeon_mode->radeon.DacCntl,
		RADEON_DAC_CNTL);
	
	/*
	 * Setup CRTC registers
	 */
		
	RADEON_CTRL_OUT32(radeon_io, radeon_mode->radeon.CrtcGenCntl, 
		RADEON_CRTC_GEN_CNTL);

	RADEON_CTRL_OUT32(radeon_io, radeon_mode->radeon.CrtcHTotalDisp, 
		RADEON_CRTC_H_TOTAL_DISP);
	
	RADEON_CTRL_OUT32(radeon_io, radeon_mode->radeon.CrtcHSyncStrtWid, 
		RADEON_CRTC_H_SYNC_STRT_WID);
	
	RADEON_CTRL_OUT32(radeon_io, radeon_mode->radeon.CrtcVTotalDisp,
		RADEON_CRTC_V_TOTAL_DISP);
	
	RADEON_CTRL_OUT32(radeon_io, radeon_mode->radeon.CrtcVSyncStrtWid,
		RADEON_CRTC_V_SYNC_STRT_WID);
	
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_CRTC_OFFSET);
	
	RADEON_CTRL_OUT32(radeon_io, 0x0, RADEON_CRTC_OFFSET_CNTL);
	
	RADEON_CTRL_OUT32(radeon_io, radeon_mode->radeon.CrtcPitch,
		RADEON_CRTC_PITCH);

	/* Turn the CRTC back on */
	RADEON_CTRL_OUT32(radeon_io, 
		RADEON_CTRL_IN32(radeon_io, RADEON_CRTC_EXT_CNTL) &
		~(RADEON_CRTC_HSYNC_DIS | RADEON_CRTC_VSYNC_DIS | 
		RADEON_CRTC_DISPLAY_DIS),
		RADEON_CRTC_EXT_CNTL);

	/*
	 * Setup acclerator engine registers
	 */

	if(radeon_mode->radeon.accelerator.execution.context) {
		radeon_chipset_accel_context_t *context;
		context = radeon_mode->radeon.accelerator.execution.context;

		radeon_chipset_accel_restore_all(radeon_io, &(context->state));
	} else {

		radeon_chipset_accel_restore_all(radeon_io,
						 &(radeon_mode->radeon.mgc));
	}

	radeon_cp_load_microcode(radeon_io, radeon_cp_microcode);
	radeon_reset_engine(radeon_io);

	/* Setup Primary PIO, Indirect disabled*/
	RADEON_CTRL_OUT32(radeon_io, RADEON_CSQ_PRIPIO_INDDIS,
			  RADEON_CP_CSQ_CNTL);

	/* Run free! */
	val = RADEON_CTRL_IN32(radeon_io, RADEON_CP_ME_CNTL);
	val |= RADEON_ME_MODE_FREE_RUN;
	RADEON_CTRL_OUT32(radeon_io, val, RADEON_CP_ME_CNTL);

	/* Get the size of the Primary CSQ */
	radeon_mode->radeon.CSQPrimarySize =
		(RADEON_CTRL_IN32(radeon_io, RADEON_CP_CSQ_CNTL) &
		 RADEON_CSQ_CNT_PRIMARYMask) >> RADEON_CSQ_CNT_PRIMARYShift;

	radeon->mode = radeon_mode;
}

void radeon_chipset_mode_leave(radeon_chipset_t *radeon,
	radeon_chipset_io_t *radeon_io, radeon_chipset_mode_t *radeon_mode,
	kgi_image_mode_t *img, kgi_u_t images)
{
	radeon_wait_for_idle(radeon_io);

	if(radeon_mode->radeon.accelerator.execution.context) {
		radeon_chipset_accel_context_t *context;

		context = radeon_mode->radeon.accelerator.execution.context;
		radeon_chipset_accel_save_all(radeon_io,&(context->state));
	} else {
		radeon_chipset_accel_save_all(radeon_io,
					      &(radeon_mode->radeon.mgc));
	}

	radeon->mode = NULL;
}
