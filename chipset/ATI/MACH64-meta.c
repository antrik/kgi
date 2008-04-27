/* ----------------------------------------------------------------------------
**	ATI MACH64 chipset implementation
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Filip Spacek
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: MACH64-meta.c,v $
**	Revision 1.10  2003/07/26 18:45:12  cegger
**	merge improvements from the FreeBSD guys:
**	- improves OS independency
**	- add support for new MACH64 chip variants (GX and LM)
**	- bug fixes
**	
**	Revision 1.9  2003/02/02 23:13:07  cegger
**	fix warnings about 'comparing of signed and unsigned values' and 'implicit declaration'
**	
**	Revision 1.8  2002/09/22 00:51:47  fspacek
**	Do not disable VGA upon insmod.
**	
**	Revision 1.7  2002/06/13 18:39:48  fspacek
**	Fix for accelerator and PCI ids.
**	
**	Revision 1.6  2002/06/12 14:21:09  fspacek
**	PCI ids for Rage Pro, beginings of a cursor and lots of register definitions
**	
**	Revision 1.5  2002/06/07 02:09:16  fspacek
**	C'n'P typo. Thanks to cow.
**	
**	Revision 1.4  2002/06/07 01:24:35  fspacek
**	Export VGA-text resources, fixup of some register definition, implement
**	DAC io, and beginings of accelerator context handling
**	
**	Revision 1.3  2002/06/02 02:18:31  fspacek
**	Accelerator bug fixes.
**	
**	Revision 1.2  2002/06/01 22:48:38  fspacek
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
**	Revision 1.2  2000/09/21 10:06:40  seeger_s
**	- namespace cleanup: E() -> KGI_ERRNO()
**	
**	Revision 1.1.1.1  2000/04/18 08:51:11  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Filip_Spacek
#define	KGIM_CHIPSET_DRIVER	"$Revision: 1.10 $"

#define KGI_SYS_NEED_ATOMIC
#define KGI_SYS_NEED_VM
#define KGI_SYS_NEED_MALLOC
#define KGI_SYS_NEED_PROC
#define KGI_SYS_NEED_MUTEX
#include <kgi/module.h>

#ifndef	DEBUG_LEVEL
#define	DEBUG_LEVEL	2
#endif

#include "chipset/ATI/MACH64.h"
#include "chipset/ATI/MACH64-meta.h"

#define MACH64_APERTURE_SIZE	8192
#define MACH64_ACCEL_BUFSIZE	4096

/*
**	Helper functions
*/

static void mach64_free_all_buffers(void);

static void mach64_chipset_examine(mach64_chipset_io_t *mach64_io)
{
#define PROBE(reg) \
  KRN_DEBUG(2, #reg ": 0x%.8x", MACH64_CTRL_IN32(mach64_io, MACH64_##reg))

	PROBE(GP_IO);
	PROBE(BUS_CNTL);
	PROBE(SCRATCH_REG0);
	PROBE(SCRATCH_REG1);
	PROBE(DSP_CONFIG);
	PROBE(DSP_ON_OFF);
	PROBE(TIMER_CONFIG);
	PROBE(MEM_BUF_CNTL);
	PROBE(VGA_DSP_CONFIG);
	PROBE(VGA_DSP_ON_OFF);
	PROBE(MEM_ADDR_CONFIG);
	PROBE(EXT_MEM_CNTL);
	PROBE(MEM_CNTL);
	PROBE(MEM_VGA_WP_SEL);
	PROBE(MEM_VGA_RP_SEL);
	PROBE(GEN_TEST_CNTL);
	PROBE(CONFIG_CNTL);
	PROBE(CONFIG_CHIP_ID);
	PROBE(CONFIG_STAT0);
	PROBE(CONFIG_STAT1);
	PROBE(CONFIG_STAT1);
	
#undef PROBE
}

static inline void mach64_wait_for_fifo(mach64_chipset_io_t *mach64_io, int n)
{
	while((MACH64_CTRL_IN32(mach64_io, MACH64_FIFO_STAT) &
	       MACH64_C4_FIFO_STATMask) > ((unsigned int)0x8000 >> n));
}

static inline void mach64_wait_for_idle(mach64_chipset_io_t *mach64_io)
{
	mach64_wait_for_fifo(mach64_io, 16);
}


/*
**	Accelerator implementation
*/

#define TABLE_FB_ADDR(i)  ((i)*4) 
#define TABLE_SYS_ADDR(i) ((i)*4 + 1)
#define TABLE_COMMAND(i)  ((i)*4 + 2)
#define TABLE_RESERVED(i) ((i)*4 + 3)

typedef struct {
	/* Table pointing to queued up buffers in accelerator format */
	kgi_aperture_t table_aperture;
	/* Queued KGI buffers */
	kgi_accel_buffer_t **buffers;
	
	kgi_u_t num_entries;
	kgi_u_t head, tail;

	/* Number of mappings of this accelerator */
	kgi_u_t count;

} mach64_chipset_accelerator_t;

static mach64_chipset_accelerator_t mach64_accel;
static kgi_atomic_t mach64_accel_lock;

typedef struct
{
#define MACH64_CONTEXT_GROUP(start, num) \
	(MACH64_##start | (num << MACH64_92_GUIREG_COUNTERShift))

	kgi_u32_t
	
#define MACH64_CONTEXT_GROUP00 MACH64_CONTEXT_GROUP(DST_OFF_PITCH, 3)
	group00,
		DST_OFF_PITCH, DST_X, DST_Y,

#define MACH64_CONTEXT_GROUP01 MACH64_CONTEXT_GROUP(DST_HEIGHT, 1)
	group01,
		DST_HEIGHT,

#define MACH64_CONTEXT_GROUP02 MACH64_CONTEXT_GROUP(DST_BRES_LNTH, 5)
	group02,
		DST_BRES_LNTH, DST_BRES_ERR, DST_BRES_INC, DST_BRES_DEC,
		DST_CNTL,

#define MACH64_CONTEXT_GROUP03 MACH64_CONTEXT_GROUP(TRAIL_BRES_ERR, 3)
	group03,
		TRAIL_BRES_ERR, TRAIL_BRES_INC, TRAIL_BRES_DEC,
		
#define MACH64_CONTEXT_GROUP04 MACH64_CONTEXT_GROUP(Z_OFF_PITCH, 2)
	group04,
	        Z_OFF_PITCH, Z_CNTL,

#define MACH64_CONTEXT_GROUP05 MACH64_CONTEXT_GROUP(SRC_OFF_PITCH, 3)
	group05,
		SRC_OFF_PITCH, SRC_X, SRC_Y,

#define MACH64_CONTEXT_GROUP06 MACH64_CONTEXT_GROUP(SRC_WIDTH1, 2)
	group06,
		SRC_WIDTH1, SRC_HEIGHT1,

#define MACH64_CONTEXT_GROUP07 MACH64_CONTEXT_GROUP(SRC_X_START, 2)
	group07,
		SRC_X_START, SRC_Y_START,

#define MACH64_CONTEXT_GROUP08 MACH64_CONTEXT_GROUP(SRC_WIDTH2, 2)
	group08,
		SRC_WIDTH2, SRC_HEIGHT2,

#define MACH64_CONTEXT_GROUP09 MACH64_CONTEXT_GROUP(SRC_CNTL, 1)
	group09,
		SRC_CNTL,

#define MACH64_CONTEXT_GROUP10 MACH64_CONTEXT_GROUP(HOST_CNTL, 1)
	group10,
		HOST_CNTL,
		
#define MACH64_CONTEXT_GROUP11 MACH64_CONTEXT_GROUP(PAT_REG0, 3)
	group11,
		PAT_REG0, PAT_REG1, PAT_CNTL,

#define MACH64_CONTEXT_GROUP12 MACH64_CONTEXT_GROUP(SC_LEFT, 2)
	group12,
		SC_LEFT, SC_RIGHT,

#define MACH64_CONTEXT_GROUP13 MACH64_CONTEXT_GROUP(SC_TOP, 2)
	group13,
		SC_TOP, SC_BOTTOM,

#define MACH64_CONTEXT_GROUP14 MACH64_CONTEXT_GROUP(DP_BKGD_CLR, 3)
	group14,
		DP_BKGD_CLR, DP_FRGD_CLR, DP_WRITE_MASK,

#define MACH64_CONTEXT_GROUP15 MACH64_CONTEXT_GROUP(DP_PIX_WIDTH, 3)
	group15,
		DP_PIX_WIDTH, DP_MIX, DP_SRC,

#define MACH64_CONTEXT_GROUP16 MACH64_CONTEXT_GROUP(CLR_CMP_CLR, 3)
	group16,
		CLR_CMP_CLR, CLR_CMP_MSK, CLR_CMP_CNTL,

#define MACH64_CONTEXT_GROUP17 MACH64_CONTEXT_GROUP(GUI_TRAJ_CNTL, 1)
	group17,
		GUI_TRAJ_CNTL;

} mach64_chipset_context_t;

typedef struct
{
	kgi_accel_context_t kgi;
	
	/* Aperture pointing at the mach64 specific state only */
	kgi_aperture_t      aperture;

	mach64_chipset_context_t state;

} mach64_chipset_accel_context_t;

static void mach64_chipset_accel_init(kgi_accel_t *kgi_accel, void *context)
{
	mach64_chipset_accel_context_t *mach64_ctx = context;
	kgi_size_t offset;
	
	KRN_DEBUG(2, "mach64_chipset_accel_init");
	
	kgi_atomic_set(&mach64_accel_lock, 1);

	/* If this is the first mapping of the accelerator initialize the
	 * buffer table.
	 */
	if (!mach64_accel.count) {
		mach64_accel.table_aperture.virt = 
			(kgi_virt_addr_t)kgi_cmalloc(MACH64_APERTURE_SIZE);
		mach64_accel.table_aperture.phys= 
			virt_to_phys(mach64_accel.table_aperture.virt);
		mach64_accel.table_aperture.bus = 
			virt_to_bus(mach64_accel.table_aperture.virt);
		mach64_accel.table_aperture.size = 16*1024;

		mach64_accel.buffers = 
			(kgi_accel_buffer_t **)kgi_cmalloc(MACH64_ACCEL_BUFSIZE);

		mach64_accel.head = 0;
		mach64_accel.tail = 0;
		mach64_accel.num_entries = 1024;
	}
	mach64_accel.count++;
	
	kgi_atomic_set(&mach64_accel_lock, 0);

	/* Ininitialize the aperture pointing at mach64 specific context */
	mach64_ctx->aperture.size = sizeof(mach64_ctx->state);
	offset = (mem_vaddr_t) &mach64_ctx->state - (mem_vaddr_t) mach64_ctx;
	mach64_ctx->aperture.bus  = mach64_ctx->kgi.aperture.bus  + offset;
	mach64_ctx->aperture.phys = mach64_ctx->kgi.aperture.phys + offset;
	mach64_ctx->aperture.virt = mach64_ctx->kgi.aperture.virt + offset;

	mach64_ctx->state.group00 = MACH64_CONTEXT_GROUP00;
	mach64_ctx->state.group01 = MACH64_CONTEXT_GROUP01;
	mach64_ctx->state.group02 = MACH64_CONTEXT_GROUP02;
	mach64_ctx->state.group03 = MACH64_CONTEXT_GROUP03;
	mach64_ctx->state.group04 = MACH64_CONTEXT_GROUP04;
	mach64_ctx->state.group05 = MACH64_CONTEXT_GROUP05;
	mach64_ctx->state.group06 = MACH64_CONTEXT_GROUP06;
	mach64_ctx->state.group07 = MACH64_CONTEXT_GROUP07;
	mach64_ctx->state.group08 = MACH64_CONTEXT_GROUP08;
	mach64_ctx->state.group09 = MACH64_CONTEXT_GROUP09;
	mach64_ctx->state.group10 = MACH64_CONTEXT_GROUP10;
	mach64_ctx->state.group11 = MACH64_CONTEXT_GROUP11;
	mach64_ctx->state.group12 = MACH64_CONTEXT_GROUP12;
	mach64_ctx->state.group13 = MACH64_CONTEXT_GROUP13;
	mach64_ctx->state.group14 = MACH64_CONTEXT_GROUP14;
	mach64_ctx->state.group15 = MACH64_CONTEXT_GROUP15;
	mach64_ctx->state.group16 = MACH64_CONTEXT_GROUP16;
	mach64_ctx->state.group17 = MACH64_CONTEXT_GROUP17;
}

static void mach64_chipset_accel_done(kgi_accel_t *kgi_accel, void *context)
{
	mach64_chipset_io_t *mach64_io = kgi_accel->meta_io;

	kgi_atomic_set(&mach64_accel_lock, 1);

	if (context == kgi_accel->execution.context)
		kgi_accel->context = NULL;

	/* Make sure the engine is idle before we deallocate everything */
	while((MACH64_CTRL_IN32(mach64_io, MACH64_GUI_STAT) & 0x00000001))
		KRN_DEBUG(2, "Waiting for idle");

	mach64_free_all_buffers();

	mach64_accel.count--;
	if (!mach64_accel.count) {
		kgi_cfree((void *)mach64_accel.table_aperture.virt, MACH64_APERTURE_SIZE);
		kgi_cfree(mach64_accel.buffers, MACH64_ACCEL_BUFSIZE);
	}

	kgi_atomic_set(&mach64_accel_lock, 0);
}

static inline void mach64_chipset_accel_save(kgi_accel_t *kgi_accel)
{
	mach64_chipset_t *mach64 = kgi_accel->meta;
	mach64_chipset_io_t *mach64_io = kgi_accel->meta_io;
	mach64_chipset_accel_context_t *mach64_ctx = kgi_accel->execution.context;
	//mem_vaddr_t gpr = pgc_io->control.base_virt + PGC_GPRegisterBase;
	
	KRN_ASSERT(mach64);
	KRN_ASSERT(mach64_io);
	KRN_ASSERT(mach64_ctx);
	//KRN_ASSERT(mach64_io->control.base_virt);
	
#define	MACH64_SAVE(reg)	\
	mach64_ctx->state.reg = MACH64_CTRL_IN32(mach64_io, MACH64_##reg)

	MACH64_SAVE(DST_OFF_PITCH);
	MACH64_SAVE(DST_X);
	MACH64_SAVE(DST_Y);
	MACH64_SAVE(DST_BRES_LNTH);
	MACH64_SAVE(DST_BRES_ERR);
	MACH64_SAVE(DST_BRES_INC);
	MACH64_SAVE(DST_BRES_DEC);
	MACH64_SAVE(DST_CNTL);
	MACH64_SAVE(TRAIL_BRES_ERR);
	MACH64_SAVE(TRAIL_BRES_INC);
	MACH64_SAVE(TRAIL_BRES_DEC);
	MACH64_SAVE(Z_OFF_PITCH);
	MACH64_SAVE(Z_CNTL);
	MACH64_SAVE(SRC_OFF_PITCH);
	MACH64_SAVE(SRC_X);
	MACH64_SAVE(SRC_Y);
	MACH64_SAVE(SRC_WIDTH1);
	MACH64_SAVE(SRC_HEIGHT1);
	MACH64_SAVE(SRC_X_START);
	MACH64_SAVE(SRC_Y_START);
	MACH64_SAVE(SRC_WIDTH2);
	MACH64_SAVE(SRC_HEIGHT2);
	MACH64_SAVE(SRC_CNTL);
	MACH64_SAVE(HOST_CNTL);
	MACH64_SAVE(PAT_REG0);
	MACH64_SAVE(PAT_REG1);
	MACH64_SAVE(PAT_CNTL);
	MACH64_SAVE(SC_LEFT);
	MACH64_SAVE(SC_RIGHT);
	MACH64_SAVE(SC_TOP);
	MACH64_SAVE(SC_BOTTOM);
	MACH64_SAVE(DP_BKGD_CLR);
	MACH64_SAVE(DP_FRGD_CLR);
	MACH64_SAVE(DP_WRITE_MASK);
	MACH64_SAVE(DP_PIX_WIDTH);
	MACH64_SAVE(DP_MIX);
	MACH64_SAVE(DP_SRC);
	MACH64_SAVE(CLR_CMP_CLR);
	MACH64_SAVE(CLR_CMP_MSK);
	MACH64_SAVE(CLR_CMP_CNTL);
	MACH64_SAVE(GUI_TRAJ_CNTL);

#undef	MACH64_SAVE
}

static inline void mach64_chipset_accel_restore(kgi_accel_t *kgi_accel)
{
#warning flush cache-lines of the context buffer!

	/* PGC_CS_OUT32(pgc_io, pgc_ctx->aperture.bus, PGC_CS_DMAAddress); */
	/* PGC_CS_OUT32(pgc_io, pgc_ctx->aperture.size >> 2, PGC_CS_DMACount); */
}

static void mach64_check_buffer(kgi_accel_t *kgi_accel,
				kgi_accel_buffer_t *buffer)
{
	mach64_chipset_accel_context_t *ctx =
		(mach64_chipset_accel_context_t *)buffer->context;
	kgi_u32_t *data = (kgi_u32_t *)buffer->aperture.virt;
	unsigned int i;

	KRN_ASSERT(ctx);
	
#define REG(i) data[i]
#define VAL(i) data[i+1]
#define CASE(reg) case UNMM(MACH64_##reg)
	
	for (i = 0; i < buffer->execution.size; i += 2) {
		switch (REG(i)) {
		CASE(SC_LEFT):
			if(VAL(i) > 1000)
				REG(i) = MACH64_SCRATCH_REG0;
			break;
		CASE(SC_RIGHT):
			if(VAL(i) > 1000)
				REG(i) = MACH64_SCRATCH_REG0;
			break;
		}
	}
}
/*
 * If an entry is a leading entry of a buffer, mark the buffer as idle
 * and wake up processes waiting on the buffer
 */
static void mach64_free_buffer(kgi_u_t buffer)
{
	if (mach64_accel.buffers[buffer]) {

		mach64_accel.buffers[buffer]->execution.state = KGI_AS_IDLE;
		kgi_wakeup(mach64_accel.buffers[buffer]->executed);

#if (HOST_OS == HOST_OS_FreeBSD)
		if (mach64_accel.buffers[buffer]->flags & KGI_BF_USEMUTEX)
			kgi_mutex_signal(&mach64_accel.buffers[buffer]->mtx, 1);
#endif
	}
}

/*
 * Mark all buffers as idle. The engine is assumed idle and no
 * checking is done.
 */
static void mach64_free_all_buffers(void)
{
	kgi_u_t head = mach64_accel.head;
	kgi_u_t tail = mach64_accel.tail;
	kgi_u_t i;

	if (head <= tail) {
		for(i = head; i < tail; ++i)
			mach64_free_buffer(i);
	}
	else {
		for(i = head; i < mach64_accel.num_entries; ++i)
			mach64_free_buffer(i);
		for(i = 0; i < tail; ++i)
			mach64_free_buffer(i);
	}
	mach64_accel.head = mach64_accel.tail = 0;
}

/*
 * Mark a buffer from the table as idle if it isn't currently being
 * executed
 */
static int mach64_try_free_buffer(kgi_u_t num, kgi_u32_t executing)
{
	kgi_u32_t *table = (kgi_u32_t *)mach64_accel.table_aperture.virt;

	/* Cannot free executing buffer */
	if(table[TABLE_SYS_ADDR(num)] == executing) {
		
    		return 0;
	}

	mach64_free_buffer(num);
	return 1;
}	

/*
 * Walk the table starting at the head and mark buffers as idle up to the
 * one currently being executed.
 * This function assumes that the egine is not idle
 */
static void mach64_free_finished_buffers(mach64_chipset_io_t *mach64_io,
					 kgi_u32_t system_addr)
{
	/* FIXME: use pagesize constants */
	system_addr &= 0xfffff000;

	if (mach64_accel.head <= mach64_accel.tail) {

		while(mach64_accel.head < mach64_accel.tail &&
		      mach64_try_free_buffer(mach64_accel.head, system_addr)){
			
			mach64_accel.head++;
		}
	}
	else {
		
		while(mach64_accel.head < mach64_accel.num_entries &&
		      mach64_try_free_buffer(mach64_accel.head, system_addr)){
		      
		        mach64_accel.head++;
		}

		if (mach64_accel.head == mach64_accel.num_entries) {
			mach64_accel.head = 0;

			while(mach64_accel.head < mach64_accel.tail &&
			      mach64_try_free_buffer(mach64_accel.head,
						     system_addr)){
				
				mach64_accel.head++;
			}
		}
	}
	if (mach64_accel.head == mach64_accel.tail) {

		KRN_DEBUG(1, "Didn't find executing buffer current: 0x%.8x",
			  system_addr);
		KRN_DEBUG(1, "Table 0x%.8x", mach64_accel.table_aperture.bus);
	}
}

/*
 * Queue up an accel buffer at the tail of the table
 */
static void mach64_queue_buffer(kgi_accel_buffer_t *buf)
{
	kgi_u32_t *table = (kgi_u32_t *)mach64_accel.table_aperture.virt;
	kgi_phys_addr_t start = buf->aperture.bus;
	kgi_phys_addr_t end = start + buf->execution.size;
	kgi_u_t buf_size, last;
	
	while(start < end) {
		/* Each buffer can be up to 4096 bytes long on MACH64 */
		buf_size = end - start;
		if (buf_size > 4096) {
			
			buf_size = 4096;
			last = 0;
		}
		else {

			last = 1;
		}
		
		KRN_ASSERT(mach64_accel.tail < mach64_accel.num_entries);
		
		table[TABLE_FB_ADDR(mach64_accel.tail)] =
			0x7FFC00 + MACH64_BM_ADDR;
		table[TABLE_SYS_ADDR(mach64_accel.tail)] = start;
		table[TABLE_COMMAND(mach64_accel.tail)] =
			buf_size | MACH64_162_FRAME_OFFSET_HOLD |
			(last ? MACH64_162_END_OF_LIST_STATUS : 0);
		table[TABLE_RESERVED(mach64_accel.tail)] = 0;

		/* Only the last table entry for this buffer gets the pointer
		   to the buffer. This way, we won't mark the buffer idle
		   until all of its parts have executed.
		*/
		if (last)
			mach64_accel.buffers[mach64_accel.tail] = buf;
		else
			mach64_accel.buffers[mach64_accel.tail] = NULL;

		mach64_accel.tail++;
		if (mach64_accel.tail == mach64_accel.num_entries)
			mach64_accel.tail = 0;
		
		start += buf_size;
	}
}

/*
 * Check if the engine is executing the last or the second last buffer.
 * If it is, busy wait till it's done.
 */
static int mach64_engine_nearly_idle(mach64_chipset_io_t *mach64_io,
				     kgi_u32_t system_addr)
{
	kgi_u32_t *table = (kgi_u32_t *)mach64_accel.table_aperture.virt;
	kgi_u_t entry = mach64_accel.tail;

	/* FIXME: use pagesize constants */
	system_addr &= 0xfffff000;

	if (mach64_accel.tail >= mach64_accel.head){
		if (mach64_accel.tail - mach64_accel.head <= 2)
			goto wait_for_idle;
	}
	else {
		if (mach64_accel.tail + mach64_accel.num_entries -
		    mach64_accel.head <= 2)
			goto wait_for_idle;
	}
	
	/* Check the last entry */
	if (entry == 0)
		entry = mach64_accel.num_entries - 1;
	else
		--entry;
	if (table[TABLE_SYS_ADDR(entry)] == system_addr)
		goto wait_for_idle;

	/* Check the second last entry */
	if (entry == 0)
		entry = mach64_accel.num_entries - 1;
	else
		--entry;
	if (table[TABLE_SYS_ADDR(entry)] == system_addr)
		goto wait_for_idle;
	
	return 0;

wait_for_idle:
	while((MACH64_CTRL_IN32(mach64_io, MACH64_GUI_STAT) & 0x00000001))
		KRN_DEBUG(4, "Waiting for idle");
	return 1;
}

/*
 * Execute an accel buffer knowing that the engine is currently running
 * and isn't on the last or the second last table entry
 */
static void mach64_exec_engine_running(mach64_chipset_io_t *mach64_io,
				       kgi_accel_buffer_t *buffer,
				       kgi_u32_t system_addr)
{
	/* The last buffer isn't last anymore */
	kgi_u32_t *table=(kgi_u32_t*)mach64_accel.table_aperture.virt;
	kgi_u_t entry = (mach64_accel.tail == 0) ? 
		mach64_accel.num_entries - 1 : mach64_accel.tail - 1;
       	table[TABLE_COMMAND(entry)] = table[TABLE_COMMAND(entry)] &
		~MACH64_162_END_OF_LIST_STATUS;
	
	KRN_ASSERT(mach64_accel.head != mach64_accel.tail);

	mach64_queue_buffer(buffer);
	mach64_free_finished_buffers(mach64_io, system_addr);
	
	KRN_DEBUG(4, "Engine is running (h %d, t %d)",
		  mach64_accel.head, mach64_accel.tail);
}

/*
 * Execute and accel buffer knowing that the engine is idle. Free up any
 * finished buffer, and start up the operation
 */
static void mach64_exec_engine_idle(mach64_chipset_io_t *mach64_io,
				    kgi_accel_buffer_t *buffer)
{
	/* The egine is idle */
	KRN_DEBUG(4, "Engine is idle h: %d t: %d",
		  mach64_accel.head, mach64_accel.tail);

	mach64_free_all_buffers();
	mach64_queue_buffer(buffer);
	
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_BUS_CNTL)&
			  ~MACH64_28_BUS_MASTER_DIS, MACH64_BUS_CNTL);
	MACH64_CTRL_OUT32(mach64_io, mach64_accel.table_aperture.bus,
			  MACH64_BM_GUI_TABLE);
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_SRC_CNTL)
			  | MACH64_6D_BUS_MASTER_EN
			  | MACH64_6D_BUS_MASTER_SYNC
			  | (3 << MACH64_6D_BUS_MASTER_OPShift),
			  MACH64_SRC_CNTL);
	MACH64_CTRL_OUT32(mach64_io, 0, MACH64_DST_HEIGHT_WIDTH);
}

static void mach64_chipset_accel_exec(kgi_accel_t *accel,
	kgi_accel_buffer_t *buffer)
{
	mach64_chipset_io_t *mach64_io = accel->meta_io;
	kgi_u32_t system_addr;

	KRN_ASSERT(KGI_AS_FILL == buffer->execution.state);
	KRN_DEBUG(4, "mach64_chipset_accel_exec");
	
	kgi_atomic_set(&mach64_accel_lock, 1);
	
	buffer->execution.state = KGI_AS_WAIT;

	mach64_check_buffer(accel, buffer);

	/* First check if the engine is running */
	if ((MACH64_CTRL_IN32(mach64_io, MACH64_GUI_STAT) & 0x00000001)) {
		int idle = 0;

		/* Now we need to find out the address of the currently
		   executing buffer. BM_SYSTEM_MEM_ADDR holds the address
		   of the data currently being bus mastered. However the
		   card bus masters not only the contents of buffers but
		   also the table data, so we need to keep on reading it
		   until we get an address that is part of a buffer. The
		   engine can go idle in the meantime so we need to check
		   for that too.
		*/
		do {
			system_addr =
			MACH64_CTRL_IN32(mach64_io, MACH64_BM_SYSTEM_MEM_ADDR);

			if (!(MACH64_CTRL_IN32(mach64_io, MACH64_GUI_STAT) &
			      0x00000001)){
				idle = 1;
				break;
			}
		} while((system_addr & ~(mach64_accel.table_aperture.size-1))==
			mach64_accel.table_aperture.bus);

		/* If the engine went idle, or it is executing the last or
		   the second last buffer let it go idle and start it up
		   again
		*/
		if(idle || mach64_engine_nearly_idle(mach64_io, system_addr))
			mach64_exec_engine_idle(mach64_io, buffer);
		else
			mach64_exec_engine_running(mach64_io, buffer,
						   system_addr);
		
	}
	else {

		mach64_exec_engine_idle(mach64_io, buffer);
	}

	buffer->execution.state = KGI_AS_EXEC;

	kgi_atomic_set(&mach64_accel_lock, 0);
}


/*
** Interrupt Handling
*/
kgi_error_t mach64_chipset_irq_handler(mach64_chipset_t *mach64, 
	mach64_chipset_io_t *mach64_io, irq_system_t *system)
{
	kgi_u32_t flags = MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL);
  
	KRN_ASSERT(mach64);
	KRN_ASSERT(mach64_io);

	/* For now we are very forgiving. Whatever interupt we recognize
	   we deal with, and clear them all at the end. */
	if(flags & MACH64_06_CRTC_VBLANK_INT){
		KRN_TRACE(0, mach64->interrupt.vblank++);
		KRN_DEBUG(4, "VBLANK interrupt");

		/* If the application uses up all the buffers, it goes to
		 * sleep on the next accelerator write. In that case there
		 * is plenty of work for the card queued up so that we can
		 * try clearing some of them only on the next vblank because
		 * the card will be busy for a while anyways
		 */
		if (!kgi_atomic_read(&mach64_accel_lock)){
			if (MACH64_CTRL_IN32(mach64_io, MACH64_GUI_STAT) & 0x00000001){
				/* The egine is running */
				/* FIXME: finish this up */
				/* mach64_free_finished_buffers(mach64_io); */
			}
			else {
				/* The egine is idle */
				mach64_free_all_buffers();
			}
		}
	}
	if(flags & MACH64_06_CRTC_VLINE_INT){
		KRN_TRACE(0, mach64->interrupt.vline++);
		KRN_DEBUG(4, "VLINE interrupt");
	}
	if(flags & MACH64_06_BUSMASTER_EOL_INT){
		KRN_TRACE(0, mach64->interrupt.dma++);
		KRN_DEBUG(4, "DMA complete interrupt");
	}
	if(flags & MACH64_06_GP_INT){
		KRN_TRACE(0, mach64->interrupt.gp++);
		KRN_DEBUG(4, "General Purpose I/O interrupt");
	}

	KRN_DEBUG(4, "Unknown interrupt 0x%.8x", flags);
	/* Clear all interupts */
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL),
			  MACH64_CRTC_INT_CNTL);

	return KGI_EOK;
}


/*
 * Pointer Control
 */
static void mach64_chipset_ptr_show(kgi_marker_t *ptr, kgi_u_t x, kgi_u_t y)
{
	mach64_chipset_io_t *mach64_io = (mach64_chipset_io_t *) ptr->meta_io;
	
	kgi_u32_t val = MACH64_CTRL_IN32(mach64_io, MACH64_GEN_TEST_CNTL);
	val |= MACH64_34_GEN_CUR_ENABLE;
	MACH64_CTRL_OUT32(mach64_io, val, MACH64_GEN_TEST_CNTL);
	
	val= ((x<<MACH64_1B_CUR_HORZ_POSNShift) & MACH64_1B_CUR_HORZ_POSNMask)|
	     ((y<<MACH64_1B_CUR_VERT_POSNShift) & MACH64_1B_CUR_VERT_POSNMask);
	MACH64_CTRL_OUT32(mach64_io, val, MACH64_CUR_HORZ_VERT_POSN);
}

static void mach64_chipset_ptr_hide(kgi_marker_t *ptr)
{
	mach64_chipset_io_t *mach64_io = (mach64_chipset_io_t *) ptr->meta_io;
	kgi_u32_t val = MACH64_CTRL_IN32(mach64_io, MACH64_GEN_TEST_CNTL);
	val &= ~MACH64_34_GEN_CUR_ENABLE;
	MACH64_CTRL_OUT32(mach64_io, val, MACH64_GEN_TEST_CNTL);
}

static void mach64_chipset_ptr_set_shape(kgi_marker_t *ptr, kgi_u_t shape,
	kgi_u_t hot_x, kgi_u_t hot_y, const void *data,
	const kgi_rgb_color_t *color)
{
	
}

static kgi_u_t mach64_chipset_memory_count(mach64_chipset_t *mach64,
					   mach64_chipset_io_t *mach64_io)
{
	kgi_u_t sizes1[] = { 512, 1024, 2*1024, 4*1024, 6*1024, 8*1024,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, 2*1024 };
	kgi_u_t sizes2[] = { 1*512, 2*512, 3*512, 4*512,
			     5*512, 6*512, 7*512, 8*512,
			     5*1024, 6*1024, 7*1024, 8*1024,
			     5*2048, 6*2048, 7*2048, 8*2048 };
	kgi_u32_t mem_size = MACH64_CTRL_IN32(mach64_io, MACH64_MEM_CNTL) &
		MACH64_2C_MEM_SIZEMask;

	/* Is this right ??? */
	if(mach64->chip < MACH64_CHIP_264GT)
		return sizes1[mem_size] * 1024;
	else
		return sizes2[mem_size] * 1024;
}


kgi_error_t mach64_chipset_init(mach64_chipset_t *mach64,
	mach64_chipset_io_t *mach64_io, const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = MACH64_PCIDEV(mach64_io);

	KRN_ASSERT(mach64);
	KRN_ASSERT(mach64_io);
	KRN_ASSERT(options);

	KRN_DEBUG(2, "initializing %s %s",
		mach64->chipset.vendor, mach64->chipset.model);

	PCICFG_SET_BASE32(mach64_io->mem.base_io, pcidev + PCI_BASE_ADDRESS_0);

	pcicfg_out16(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_IO,
		     pcidev + PCI_COMMAND);

	mach64_chipset_examine(mach64_io);

	

#warning	read & save initial configuration

	KRN_DEBUG(2, "chipset enabled");

	mach64->chipset.memory = options->chipset->memory
		? options->chipset->memory
		: mach64_chipset_memory_count(mach64, mach64_io);

	KRN_DEBUG(2, "Detected %d bytes of memory", mach64->chipset.memory);
	
	vga_text_chipset_init(&mach64->vga, &mach64_io->vga, options);
	
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_BUS_CNTL) |
			  MACH64_28_BUS_EXT_REG_EN, MACH64_BUS_CNTL);
	
	if (mach64->flags & MACH64_CF_IRQ_CLAIMED) {

		MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL);
		/* Clear all interrupts */
		MACH64_CTRL_OUT32(mach64_io,
			     MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL),
				  MACH64_CRTC_INT_CNTL);
		MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL);
		MACH64_CTRL_OUT32(mach64_io,
				  MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL) | MACH64_06_BUSMASTER_EOL_INT,
				  MACH64_CRTC_INT_CNTL);
		MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL);
		/* Enable BUSMASTER_EOL interrupt */
		MACH64_CTRL_OUT32(mach64_io,
				  MACH64_06_BUSMASTER_EOL_INT_EN |
				  MACH64_06_GP_INT_EN |
				  MACH64_06_CRTC_VBLANK_INT_EN |
				  MACH64_06_CRTC_VLINE_INT_EN,
				  MACH64_CRTC_INT_CNTL);
		
	}

	return KGI_EOK;
}

void mach64_chipset_done(mach64_chipset_t *mach64, 
	mach64_chipset_io_t *mach64_io,
	const kgim_options_t *options)
{
	pcicfg_vaddr_t pcidev = MACH64_PCIDEV(mach64_io);
  
	if (mach64->flags & MACH64_CF_IRQ_CLAIMED) {
		kgi_u32_t val;
		
		KRN_DEBUG(1, "Interrupts: vblank %d, vline %d, dma %d, gp %d",
			  mach64->interrupt.vblank, mach64->interrupt.vline,
			  mach64->interrupt.dma, mach64->interrupt.gp);
    
		/* Clear all interupts */
		val = MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL);
		MACH64_CTRL_OUT32(mach64_io, val, MACH64_CRTC_INT_CNTL);
    
		/* Disable all interrupts */
		val = MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_INT_CNTL);
		val &= ~MACH64_06_CRTC_VBLANK_INT_EN &
			~MACH64_06_CRTC_VLINE_INT_EN &
			~MACH64_06_CAPBUF0_INT_EN & ~MACH64_06_CAPBUF1_INT_EN &
			~MACH64_06_OVERLAY_EOF_INT_EN &
			~MACH64_06_ONESHOT_CAP_INT_EN &
			~MACH64_06_BUSMASTER_EOL_INT_EN & ~MACH64_06_GP_INT_EN;
		MACH64_CTRL_OUT32(mach64_io, val, MACH64_CRTC_INT_CNTL);
	}

#warning	restore initial chipset state here.
	
	vga_text_chipset_done(&mach64->vga, &mach64_io->vga, options);
	
	PCICFG_SET_BASE32(mach64->pci.BaseAddr0, pcidev + PCI_BASE_ADDRESS_0);
}

kgi_error_t mach64_chipset_mode_check(mach64_chipset_t *mach64,
	mach64_chipset_io_t *mach64_io,
	mach64_chipset_mode_t *mach64_mode, 
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_dot_port_mode_t *dpm = img->out;
	const kgim_monitor_mode_t *crt_mode = mach64_mode->mach64.kgim.crt;
	kgi_u_t bpf, bpc, bpp, width, lclk;
	kgi_mmio_region_t *r;
	kgi_accel_t *a;
	kgi_marker_t *p;
	kgi_u_t bpd;
	
	if (images != 1) {
		
		KRN_DEBUG(1, "%i images not supported.", images);
		return -KGI_ERRNO(CHIPSET, NOSUP);
	}

	/*	for text16 support we fall back to VGA mode
	**	for unsupported image flags, bail out.
	*/
	if ((img[0].flags & KGI_IF_TEXT16) || 
	    (img[0].fam & KGI_AM_TEXTURE_INDEX)) {
		
		return vga_text_chipset_mode_check(&mach64->vga,
						   &mach64_io->vga,
						   &mach64_mode->vga, cmd,
						   img, images);
	}
	if (img[0].flags &
	    (KGI_IF_TILE_X | KGI_IF_TILE_Y | KGI_IF_VIRTUAL | KGI_IF_STEREO)) {
		
		KRN_DEBUG(1, "image flags %.8x not supported", img[0].flags);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}

	/*	check if common attributes are supported.
	 */
	switch (img[0].cam) {
		
	case 0:
		break;
		
	case KGI_AM_STENCIL | KGI_AM_Z:
		if ((1 != img[0].bpca[0]) || (15 != img[0].bpca[1])) {
			
			KRN_DEBUG(1, "S%iZ%i local buffer not supported",
				  img[0].bpca[0], img[0].bpca[1]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		break;

	case KGI_AM_Z:
		if (16 != img[0].bpca[1]) {
			
			KRN_DEBUG(1,"Z%i local buffer not supported",
				  img[0].bpca[0]);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
	default:
		KRN_DEBUG(1, "common attributes %.8x not supported",
			  img[0].cam);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	/*	total bits per dot
	 */
	/* Bits per frame attributes */
	bpf = kgim_attr_bits(img[0].bpfa);
	/* Bits per common attributes */
	bpc = kgim_attr_bits(img[0].bpca);
	/* Bits per display attributes */
	bpd = kgim_attr_bits(dpm->bpda);
	/* Bits per pixel */
	bpp = bpc + bpf*img[0].frames;
	
	
	lclk = (cmd == KGI_TC_PROPOSE)
		? 0 : dpm->dclk * dpm->lclk.mul / dpm->lclk.div;
	
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
			
			img[0].virt.x = (8 * mach64->chipset.memory) /
				(img[0].virt.y * bpp);
			
			if (img[0].virt.x > mach64->chipset.maxdots.x) {
				
				img[0].virt.x = mach64->chipset.maxdots.x;
			}
		}
#warning	restrict img[0].virt.x to supported width here.

		if (0 == img[0].virt.y) {
			
			img[0].virt.y = (8 * mach64->chipset.memory) /
				(img[0].virt.x * bpp);
		}
		
		/*	Are we beyond the limits of the H/W?
		 */
		if ((img[0].size.x >= mach64->chipset.maxdots.x) || 
		    (img[0].virt.x >= mach64->chipset.maxdots.x)) {
			
			KRN_DEBUG(1, "%i (%i) horizontal pixels are too many",
				  img[0].size.x, img[0].virt.x);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}
		
		if ((img[0].size.y >= mach64->chipset.maxdots.y) ||
		    (img[0].virt.y >= mach64->chipset.maxdots.y)) {
			
			KRN_DEBUG(1, "%i (%i) vertical pixels are too many",
				  img[0].size.y, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, UNKNOWN);
		}
		
		if ((img[0].virt.x * img[0].virt.y * bpp) > 
		    (8 * mach64->chipset.memory)) {
			
			KRN_DEBUG(1, "not enough memory for (%ipf*%if + %ipc)@"
				  "%ix%i", bpf, img[0].frames, bpc,
				  img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET,NOMEM);
		}
		
		/*	set CRT visible fields
		 */
		dpm->dots.x = img[0].size.x;
		dpm->dots.y = img[0].size.y;
		
		if (img[0].size.y < 400) {
			
			dpm->dots.y += img[0].size.y;
		}
		return KGI_EOK;
		
	case KGI_TC_LOWER:
	case KGI_TC_RAISE:
		
		/*   FIXME: GX chipsets will use this
		 */
		dpm->lclk.mul = 1;
		dpm->lclk.div = 1;
		
		dpm->rclk.mul = 1;
		dpm->rclk.div = 1;
		
		if (cmd == KGI_TC_LOWER) {
			
			if (dpm->dclk < mach64->chipset.dclk.min) {
				
				KRN_DEBUG(1, "DCLK = %i Hz is too low",
					  dpm->dclk);
				return -KGI_ERRNO(CHIPSET, UNKNOWN);
			}
#warning		check/set LCLK maximum here
			/*
			  if (lclk > 50000000) {
			  
			  dpm->dclk = 50000000 * dpm->lclk.div /
			  dpm->lclk.mul;
			  }
			*/
			
		} else {
			
#warning		check/set LCLK maximum here
			/*
			  if (lclk > 50000000) {
			  
			  KRN_DEBUG(1, "LCLK = %i Hz is too high", lclk);
			  return -KGI_ERRNO(CHIPSET, UNKNOWN);
			  }
			*/
		}
		return KGI_EOK;
		
		
	case KGI_TC_CHECK:
		
#warning Do proper checking of hardware limits etc. here.
		
		width = (img[0].virt.x + 7) & ~7;
		
		if (width != img[0].virt.x) {
			
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		if ((img[0].size.x >= mach64->chipset.maxdots.x) ||
		    (img[0].size.y >= mach64->chipset.maxdots.y) || 
		    (img[0].virt.x >= mach64->chipset.maxdots.x) ||
		    ((img[0].virt.y * img[0].virt.x * bpp) >
		     (8 * mach64->chipset.memory))) {
			
			KRN_DEBUG(1, "resolution too high: %ix%i (%ix%i)",
				  img[0].size.x, img[0].size.y,
				  img[0].virt.x, img[0].virt.y);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
#warning	check LCLK and RCLK rations here!
		if (((dpm->lclk.mul != 1) && (dpm->lclk.div != 1)) ||
		    ((dpm->rclk.mul != dpm->lclk.mul) &&
		     (dpm->rclk.div != dpm->lclk.div))) {
			
			KRN_DEBUG(1, "invalid LCLK (%i:%i) or CLK (%i:%i)", 
				  dpm->lclk.mul, dpm->lclk.div,
				  dpm->rclk.mul, dpm->rclk.div);
			return -KGI_ERRNO(CHIPSET, INVAL);
		}
		
#warning	check maximum LCLK rate here.
		/*
		  if (lclk > 50000000) {
		  
		  KRN_DEBUG(1, "LCLK = %i Hz is too high\n", lclk);
		  return -KGI_ERRNO(CHIPSET, CLK_LIMIT);
		  }
		*/
		break;
		
		
	default:
		KRN_INTERNAL_ERROR;
		return -KGI_ERRNO(CHIPSET, UNKNOWN);
	}


	/*	Now everything is checked and should be sane.
	**	proceed to setup device dependent mode.
	*/
	/* Conservatively choose the largest number for FIFO_LWM */
	mach64_mode->mach64.gen_cntl = MACH64_07_CRTC_ENABLE |
		MACH64_07_CRTC_EXT_DISP_EN | 0xa<<MACH64_07_CRTC_FIFO_LWMShift;

	mach64_mode->mach64.dp_set_gui_engine =
		1 << MACH64_BF_SET_DRAWING_COMBOShift;
	
	switch (bpp) {
	case 4:
		mach64_mode->mach64.gen_cntl |= MACH64_07_CRTC_PIX_WIDTH4;
		break;
	case 8:
		mach64_mode->mach64.gen_cntl |= MACH64_07_CRTC_PIX_WIDTH8;
		mach64_mode->mach64.dp_set_gui_engine |=
			MACH64_BF_SET_DP_DST_PIX_WIDTH8;
		break;
	//case 15:
	//mach64_mode->mach64.gen_cntl |= MACH64_07_CRTC_PIX_WIDTH8;
	case 16:
		mach64_mode->mach64.gen_cntl |= MACH64_07_CRTC_PIX_WIDTH16;
		mach64_mode->mach64.dp_set_gui_engine |=
			MACH64_BF_SET_DP_DST_PIX_WIDTH16;
		break;
	case 24:
		mach64_mode->mach64.gen_cntl |= MACH64_07_CRTC_PIX_WIDTH24;
		/* Acceleration works weird in 24 bit mode */
		mach64_mode->mach64.dp_set_gui_engine |=
			MACH64_BF_SET_DP_DST_PIX_WIDTH8;
		break;
	case 32:
		mach64_mode->mach64.gen_cntl |= MACH64_07_CRTC_PIX_WIDTH32;
		mach64_mode->mach64.dp_set_gui_engine |=
			MACH64_BF_SET_DP_DST_PIX_WIDTH32;
		break;
	default:
		KRN_DEBUG(0, "Fatal: impossible bpp %d", bpp);
		return -KGI_ERRNO(CHIPSET, INVAL);
	}
	
	mach64_mode->mach64.h_total_disp =
		(((crt_mode->x.total)/8) << MACH64_00_CRTC_H_TOTALShift) |
		((crt_mode->x.width/8 - 1) << MACH64_00_CRTC_H_DISPShift);
	
	mach64_mode->mach64.h_sync_strt_wid =
		((crt_mode->x.syncstart / 8) & MACH64_01_CRTC_H_SYNC_STRTMask) |
		(((crt_mode->x.syncstart/8) & ~MACH64_01_CRTC_H_SYNC_STRTMask)?
		 MACH64_01_CRTC_H_SYNC_STRT_HI : 0) |
		(((crt_mode->x.syncend - crt_mode->x.syncstart) / 8) <<
		 MACH64_01_CRTC_H_SYNC_WIDShift) |
		(crt_mode->x.polarity ? MACH64_01_CRTC_H_SYNC_POL : 0);
	
	mach64_mode->mach64.v_total_disp =
		(crt_mode->y.total << MACH64_02_CRTC_V_TOTALShift) |
		((crt_mode->y.width - 1) << MACH64_02_CRTC_V_DISPShift);
	
	mach64_mode->mach64.v_sync_strt_wid =
		crt_mode->y.syncstart |
		((crt_mode->y.syncend - crt_mode->y.syncstart) <<
		 MACH64_03_CRTC_V_SYNC_WIDShift) |
		(crt_mode->y.polarity ? MACH64_03_CRTC_V_SYNC_POL : 0);
	
	mach64_mode->mach64.off_pitch =
		(img[0].size.x / 8) << MACH64_05_CRTC_PITCHShift;
	
	/*	initialize exported resources
	 */
	r = &mach64_mode->mach64.video_memory_aperture;
	r->meta = mach64;
	r->meta_io = mach64_io;
	r->type = KGI_RT_MMIO_FRAME_BUFFER;
	r->prot = KGI_PF_APP | KGI_PF_LIB | KGI_PF_DRV;
	r->name = "mach64 video memory aperture";
	r->access = 64 + 32 + 16 + 8;
	r->align  = 64 + 32 + 16 + 8;
	r->size   = r->win.size = mach64->chipset.memory;
	r->win.bus  = mach64_io->mem.base_bus;
	r->win.phys = mach64_io->mem.base_phys;
	r->win.virt = mach64_io->mem.base_virt;
	
	a = &mach64_mode->mach64.accelerator;
	a->meta = mach64;
	a->meta_io = mach64_io;
	a->type = KGI_RT_ACCELERATOR;
	a->prot = KGI_PF_LIB | KGI_PF_DRV; 
	a->name = "MACH64 graphics processor";
	/* a->flags |= KGI_AF_DMA_BUFFERS; */
	a->buffers = 3;
	a->buffer_size = 4096;
	a->context_size = sizeof(mach64_chipset_accel_context_t);
	a->execution.context = NULL;
	a->execution.queue = NULL;
	a->Init = mach64_chipset_accel_init;
	a->Done = mach64_chipset_accel_done;
	a->Exec = mach64_chipset_accel_exec;

	p = &mach64_mode->mach64.pointer_ctrl;
	p->meta = mach64;
	p->meta_io = mach64_io;
	p->type = KGI_RT_POINTER_CONTROL;
	p->prot = KGI_PF_DRV_RWS;
	p->name = "MACH64 pointer control";
	p->modes = 0;
	p->shapes = 1;
	p->size.x = 64;
	p->size.y = 64;
	p->SetMode = NULL;
	p->Select = NULL;
	p->SetShape = mach64_chipset_ptr_set_shape;
	p->Show = mach64_chipset_ptr_show;
	p->Hide = mach64_chipset_ptr_hide;
	p->Undo = NULL;
	
	return KGI_EOK;
}

kgi_resource_t *mach64_chipset_mode_resource(mach64_chipset_t *mach64, 
	mach64_chipset_mode_t *mach64_mode, 
	kgi_image_mode_t *img, kgi_u_t images, kgi_u_t index)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX)
		return vga_text_chipset_mode_resource(&mach64->vga,
						      &mach64_mode->vga,
						      img, images, index);
	
	switch (index) {
		
	case 0:
		return (kgi_resource_t *)&mach64_mode->mach64.video_memory_aperture;
	case 1:
		return (kgi_resource_t *)&mach64_mode->mach64.accelerator;
	case 2:
		return (kgi_resource_t *)&mach64_mode->mach64.accelerator;
	}
	return NULL;
}

kgi_resource_t *mach64_chipset_image_resource(mach64_chipset_t *mach64, 
	mach64_chipset_mode_t *mach64_mode, 
	kgi_image_mode_t *img, kgi_u_t image, kgi_u_t index)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX)
		return vga_text_chipset_image_resource(&mach64->vga,
						       &mach64_mode->vga,
						       img, image, index);
	
	return NULL;
}

void mach64_chipset_mode_prepare(mach64_chipset_t *mach64,
				 mach64_chipset_io_t *mach64_io,
				 mach64_chipset_mode_t *mach64_mode,
				 kgi_image_mode_t *img, kgi_u_t images)
{
	kgi_u32_t val;
	
	if (img->fam & KGI_AM_TEXTURE_INDEX) {
		
		/* Enable VGA */
		val = MACH64_CTRL_IN32(mach64_io, MACH64_CONFIG_STAT0);
		val |= MACH64_39_CFG_VGA_EN;
		MACH64_CTRL_OUT32(mach64_io, val, MACH64_CONFIG_STAT0);
		
		/* Disable extended CRTC */
		val = MACH64_CTRL_IN32(mach64_io, MACH64_CRTC_GEN_CNTL);
		val &= ~MACH64_07_CRTC_EXT_DISP_EN;
		MACH64_CTRL_OUT32(mach64_io, val, MACH64_CRTC_GEN_CNTL);
		
		mach64_io->flags |= MACH64_IF_VGA_DAC;
		vga_text_chipset_mode_prepare(&mach64->vga, &mach64_io->vga,
					      &mach64_mode->vga, img, images);
    
		KRN_DEBUG(2, "prepared for VGA-mode");
		return;
	}
	
	/* Disable VGA */
	val = MACH64_CTRL_IN32(mach64_io, MACH64_CONFIG_STAT0);
	val &= ~MACH64_39_CFG_VGA_EN;
	MACH64_CTRL_OUT32(mach64_io, val, MACH64_CONFIG_STAT0);
	
	mach64_io->flags &= ~MACH64_IF_VGA_DAC;
	KRN_DEBUG(2, "prepared for mach64 mode");
}

void mach64_chipset_mode_enter(mach64_chipset_t *mach64,
			       mach64_chipset_io_t *mach64_io,
			       mach64_chipset_mode_t *mach64_mode,
			       kgi_image_mode_t *img, kgi_u_t images)
{
	if (img->fam & KGI_AM_TEXTURE_INDEX) {
		
		vga_text_chipset_mode_enter(&mach64->vga, &mach64_io->vga,
					    &mach64_mode->vga, img, images);
		return;
	}

	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.gen_cntl,
			  MACH64_CRTC_GEN_CNTL);
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.h_total_disp,
			  MACH64_CRTC_H_TOTAL_DISP);
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.h_sync_strt_wid,
			  MACH64_CRTC_H_SYNC_STRT_WID);
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.v_total_disp,
			  MACH64_CRTC_V_TOTAL_DISP);
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.v_sync_strt_wid,
			  MACH64_CRTC_V_SYNC_STRT_WID);
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.off_pitch,
			  MACH64_CRTC_OFF_PITCH);
	
	MACH64_CTRL_OUT32(mach64_io, 0, MACH64_OVR_CLR);
	MACH64_CTRL_OUT32(mach64_io, 0, MACH64_OVR_WID_LEFT_RIGHT);
	MACH64_CTRL_OUT32(mach64_io, 0, MACH64_OVR_WID_TOP_BOTTOM);
	
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_DAC_CNTL) |
			  MACH64_31_DAC_8BIT_EN | MACH64_31_DAC_VGA_ADR_EN,
			  MACH64_DAC_CNTL);

	//MACH64_CTRL_OUT32(mach64_io, 0x8701058c, MACH64_DAC_CNTL);
	//MACH64_CTRL_OUT32(mach64_io, 0x680000f9, MACH64_BUS_CNTL);

	mach64_wait_for_fifo(mach64_io, 10);
	/* Reset engine */
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_GEN_TEST_CNTL) &
			  0xffffffff, MACH64_GEN_TEST_CNTL);

	mach64_wait_for_fifo(mach64_io, 10);
	/* Enable engine */
	MACH64_CTRL_OUT32(mach64_io,
			  MACH64_CTRL_IN32(mach64_io, MACH64_GEN_TEST_CNTL) |
			  MACH64_34_GEN_GUI_RESETB, MACH64_GEN_TEST_CNTL);

	/* Set up a default context */
	mach64_wait_for_fifo(mach64_io, 10);
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.dp_set_gui_engine,
			  MACH64_DP_SET_GUI_ENGINE);

	/* DP_SET_GUI_ENGINE is too restrictive for setting up pitch and
	   offset so we set it separately. DST_OFF_PITCH has the exact
	   same format as CRTC_OFF_PITCH. */
	MACH64_CTRL_OUT32(mach64_io, mach64_mode->mach64.off_pitch,
			  MACH64_DST_OFF_PITCH);


	MACH64_CTRL_OUT32(mach64_io, 0xfffffff, MACH64_DP_WRITE_MASK);


	/* DEBUG: Draw a rectangle */
	mach64_wait_for_fifo(mach64_io, 10);
	MACH64_CTRL_OUT32(mach64_io, 0xf0f0f0f0, MACH64_DP_FRGD_CLR);
	MACH64_CTRL_OUT32(mach64_io, 1 << MACH64_B6_DP_FRGD_SRCShift,
			  MACH64_DP_SRC);
	MACH64_CTRL_OUT32(mach64_io, 400, MACH64_DST_X);
	MACH64_CTRL_OUT32(mach64_io, 400, MACH64_DST_Y);
	MACH64_CTRL_OUT32(mach64_io, 900, MACH64_DST_HEIGHT);
	MACH64_CTRL_OUT32(mach64_io, 900, MACH64_DST_WIDTH);
	KRN_DEBUG(2, "drawn");

	mach64->mode = mach64_mode;
}

void mach64_chipset_mode_leave(mach64_chipset_t *mach64,
			       mach64_chipset_io_t *mach64_io,
			       mach64_chipset_mode_t *mach64_mode,
			       kgi_image_mode_t *img, kgi_u_t images)
{
#warning	sync with chipset here.

	mach64->mode = NULL;
}
