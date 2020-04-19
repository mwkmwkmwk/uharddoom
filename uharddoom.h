#ifndef UHARDDOOM_H
#define UHARDDOOM_H

/* Section 1: PCI ids. */

#define UHARDDOOM_VENDOR_ID			0x0666
#define UHARDDOOM_DEVICE_ID			0x1995

/* Section 2: MMIO registers.  */

/* Section 2.1: main control area.  */

/* Enables active units of the device.  TLB and CACHE is passive and doesn't
 * have an enable (disable the client instead).  FIFOs also don't have enables
 * -- disable the source and/or destination unit instead.  */
#define UHARDDOOM_ENABLE				0x0000
#define UHARDDOOM_ENABLE_BATCH				0x00000001
#define UHARDDOOM_ENABLE_JOB				0x00000002
#define UHARDDOOM_ENABLE_CMD				0x00000004
#define UHARDDOOM_ENABLE_FE				0x00000008
#define UHARDDOOM_ENABLE_COL				0x00000010
#define UHARDDOOM_ENABLE_SPAN				0x00000020
#define UHARDDOOM_ENABLE_FX				0x00000040
#define UHARDDOOM_ENABLE_SWR				0x00000080
#define UHARDDOOM_ENABLE_ALL				0x000000ff
/* Status of device units -- 1 means they have work to do.  */
#define UHARDDOOM_STATUS				0x0004
#define UHARDDOOM_STATUS_BATCH				0x00000001
#define UHARDDOOM_STATUS_JOB				0x00000002
#define UHARDDOOM_STATUS_CMD				0x00000004
#define UHARDDOOM_STATUS_FE				0x00000008
#define UHARDDOOM_STATUS_COL				0x00000010
#define UHARDDOOM_STATUS_SPAN				0x00000020
#define UHARDDOOM_STATUS_FX				0x00000040
#define UHARDDOOM_STATUS_SWR				0x00000080
#define UHARDDOOM_STATUS_FIFO_COLCMD			0x00100000
#define UHARDDOOM_STATUS_FIFO_SPANCMD			0x00200000
#define UHARDDOOM_STATUS_FIFO_FXCMD			0x00400000
#define UHARDDOOM_STATUS_FIFO_SWRCMD			0x00800000
#define UHARDDOOM_STATUS_FIFO_FELOCK			0x01000000
#define UHARDDOOM_STATUS_FIFO_COLLOCK			0x02000000
#define UHARDDOOM_STATUS_FIFO_SPANLOCK			0x04000000
#define UHARDDOOM_STATUS_FIFO_SPANOUT			0x08000000
#define UHARDDOOM_STATUS_FIFO_COLOUT			0x10000000
#define UHARDDOOM_STATUS_FIFO_FXOUT			0x20000000
/* The reset register.  Punching 1 will clear all pending work and/or
 * cached data.  */
#define UHARDDOOM_RESET					0x0004
#define UHARDDOOM_RESET_BATCH				0x00000001
#define UHARDDOOM_RESET_JOB				0x00000002
#define UHARDDOOM_RESET_CMD				0x00000004
#define UHARDDOOM_RESET_FE				0x00000008
#define UHARDDOOM_RESET_COL				0x00000010
#define UHARDDOOM_RESET_SPAN				0x00000020
#define UHARDDOOM_RESET_FX				0x00000040
#define UHARDDOOM_RESET_SWR				0x00000080
#define UHARDDOOM_RESET_TLB_BATCH			0x00000100
#define UHARDDOOM_RESET_TLB_CMD				0x00000200
#define UHARDDOOM_RESET_TLB_SWR_DST			0x00000400
#define UHARDDOOM_RESET_TLB_COL_CMAP_A			0x00000800
#define UHARDDOOM_RESET_TLB_COL_CMAP_B			0x00001000
#define UHARDDOOM_RESET_TLB_COL_SRC			0x00002000
#define UHARDDOOM_RESET_TLB_SPAN_SRC			0x00004000
#define UHARDDOOM_RESET_TLB_SWR_TRANSMAP		0x00008000
#define UHARDDOOM_RESET_TLB_USER_ALL			0x0000fe00
#define UHARDDOOM_RESET_CACHE_COL_CMAP_B		0x00010000
#define UHARDDOOM_RESET_CACHE_COL_SRC			0x00020000
#define UHARDDOOM_RESET_CACHE_SPAN_SRC			0x00040000
#define UHARDDOOM_RESET_CACHE_SWR_TRANSMAP		0x00080000
#define UHARDDOOM_RESET_FIFO_COLCMD			0x00100000
#define UHARDDOOM_RESET_FIFO_SPANCMD			0x00200000
#define UHARDDOOM_RESET_FIFO_FXCMD			0x00400000
#define UHARDDOOM_RESET_FIFO_SWRCMD			0x00800000
#define UHARDDOOM_RESET_FIFO_FELOCK			0x01000000
#define UHARDDOOM_RESET_FIFO_COLLOCK			0x02000000
#define UHARDDOOM_RESET_FIFO_SPANLOCK			0x04000000
#define UHARDDOOM_RESET_FIFO_SPANOUT			0x08000000
#define UHARDDOOM_RESET_FIFO_COLOUT			0x10000000
#define UHARDDOOM_RESET_FIFO_FXOUT			0x20000000
#define UHARDDOOM_RESET_STATS				0x80000000
#define UHARDDOOM_RESET_ALL				0xbfffffff
/* Interrupt status.  */
#define UHARDDOOM_INTR					0x0008
#define UHARDDOOM_INTR_BATCH_WAIT			0x00000001
#define UHARDDOOM_INTR_JOB_DONE				0x00000002
#define UHARDDOOM_INTR_FE_ERROR				0x00000010
#define UHARDDOOM_INTR_CMD_ERROR			0x00000020
#define UHARDDOOM_INTR_PAGE_FAULT(i)			(0x00000100 << (i))
#define UHARDDOOM_INTR_PAGE_FAULT_BATCH			0x00000100
#define UHARDDOOM_INTR_PAGE_FAULT_CMD			0x00000200
#define UHARDDOOM_INTR_PAGE_FAULT_SWR_DST		0x00000400
#define UHARDDOOM_INTR_PAGE_FAULT_COL_CMAP_A		0x00000800
#define UHARDDOOM_INTR_PAGE_FAULT_COL_CMAP_B		0x00001000
#define UHARDDOOM_INTR_PAGE_FAULT_COL_SRC		0x00002000
#define UHARDDOOM_INTR_PAGE_FAULT_SPAN_SRC		0x00004000
#define UHARDDOOM_INTR_PAGE_FAULT_SWR_TRANSMAP		0x00008000
#define UHARDDOOM_INTR_MASK				0x0000ff33
/* And enable (same bitfields).  */
#define UHARDDOOM_INTR_ENABLE				0x000c

/* Section 2.2: BATCH -- batch processor.  Fetches (user pd, cmd ptr,
 * cmd size) tuples from kernel pd and pokes JOB.
 *
 * The pseudocode for this unit is:
 *
 * while True:
 *     # Whenever we have an available batch to read and JOB is not busy...
 *     if not STATUS.JOB and BATCH_GET != BATCH_PUT:
 *         # Read a 16-byte batch structure from the kernel PD
 *         cur_batch = mem_read(BATCH_PD, BATCH_GET, 0x10)
 *         # And send it to run
 *         JOB_PD = cur_batch[0:4]
 *         JOB_CMD_PTR = cur_batch[4:8]
 *         JOB_CMD_SIZE = cur_batch[8:12]
 *         # bytes 12:16 ignored
 *         JOB.trigger_from_batch()
 *         # Wait until JOB is finished
 *         JOB.wait()
 *         # Increment BATCH_GET, possibly wrapping
 *         BATCH_GET += 0x10
 *         if BATCH_GET == BATCH_WRAP:
 *             BATCH_GET = 0
 *         # If we reached the interrupt value, trigger interrupt.
 *         if BATCH_GET == BATCH_WAIT:
 *             INTR |= INTR_BATCH_WAIT
 * */

/* The kernel PD, used for fetching batch jobs.  Goes straight to
 * TLB_KERNEL_PD.  */
#define UHARDDOOM_BATCH_PDP				0x0020
/* If BATCH_GET would be equal to this value after incrementing, set it to 0
 * instead.  */
#define UHARDDOOM_BATCH_WRAP				0x0024
/* The current kernel vaddr that BATCH is reading batches from.  Must be
 * 0x10-byte aligned.  */
#define UHARDDOOM_BATCH_GET				0x0028
/* Current end pointer -- when BATCH_GET is equal to BATCH_PUT, BATCH will
 * halt until more data is available.  */
#define UHARDDOOM_BATCH_PUT				0x002c
/* Interrupt pointer -- when BATCH_GET is incremented and reaches this value,
 * the BATCH_WAIT interrupt will be triggered.  */
#define UHARDDOOM_BATCH_WAIT				0x0030
#define UHARDDOOM_BATCH_PTR_MASK			0xfffffff0

/* Section 2.3: JOB -- the main job controller.  Collects a single job, sends
 * the PD to TLB, pokes CMD and FE, waits until the whole job is finished,
 * notifies BATCH of completion, raises JOB_DONE interrupt.
 *
 * Functions:
 *
 * 1. Can be triggered by BATCH or manually.  When this happens, the PD is
 *    sent to TLB_USER_PD, the CMD pointer and size is sent to PD, and
 *    the ACTIVE bit is set in state.
 * 2. When JOB_WAIT is read by FE, the read will block until the ACTIVE bit
 *    is set.
 * 3. When JOB_DONE is written by FE, the job considered to be done -- the
 *    JOB_DONE interrupt is triggered, ACTIVE bit is cleared, and BATCH
 *    is notified of the JOB completion (if FROM_BATCH flag was set).
 */

/* The user PD for this job.  Goes straight to TLB_USER_PD.  */
#define UHARDDOOM_JOB_PDP				0x0040
/* Pointer (in user PD) to commands for this job.  Goes straight to
 * CMD_CMD_PTR.  */
#define UHARDDOOM_JOB_CMD_PTR				0x0044
/* Size (in bytes) of command buffer for this job.  Goes straight to
 * CMD_CMD_SIZE.  */
#define UHARDDOOM_JOB_CMD_SIZE				0x0048
/* Write any value to this register to manually trigger a new job.  */
#define UHARDDOOM_JOB_TRIGGER				0x004c
/* A state register.  */
#define UHARDDOOM_JOB_STATE				0x0050
/* Set if a job is active.  Cleared by FE when it decides we're done.  */
#define UHARDDOOM_JOB_STATE_ACTIVE			0x00000001
/* Set if the current job comes from BATCH (and not manual trigger).  If set,
 * will notify BATCH that it should bump the BATCH_GET pointer when we're
 * done with the current job.  */
#define UHARDDOOM_JOB_STATE_FROM_BATCH			0x00000002
#define UHARDDOOM_JOB_STATE_MASK			0x00000003

/* Section 2.4: CMD -- the command fetcher.  Reads a stream of commands from
 * the user PD, buffers them, and delivers them to FE.  Set up by RUN.
 *
 * Functions:
 *
 * 1. When a new job is triggered in JOB, both CMD_FE_PTR and CMD_READ_PTR
 *    are set to JOB_CMD_PTR, and CMD_READ_SIZE is set to JOB_CMD_SIZE.
 * 2. Whenever there is space in the buffer, CMD reads from CMD_READ_PTR,
 *    increments it, decrements CMD_READ_SIZE accordingly, and stores
 *    the commands in the buffer.
 * 3. At any time, FE can read the CMD_END register to know if there are
 *    any commands left in the job -- if CMD_READ_SIZE is zero and the buffer
 *    is empty, CMD will return 1 (otherwise, it will return 0).
 * 4. FE can read the CMD_FETCH register to get the next command from the buffer.
 *    If there is something in the buffer, the command is removed from
 *    the buffer and returned to FE.  If the buffer is empty, but
 *    CMD_READ_SIZE is non-0, the read is blocked until the next command is
 *    read.  If the buffer is empty and CMD_READ_SIZE is 0, the CMD_ERROR
 *    interrupt is raised.
 */

/* The current FE command pointer (ie. the address of the next command to be
 * delivered to FE).  */
#define UHARDDOOM_CMD_FE_PTR				0x0060
/* The current command read pointer (ie. the address of the next command to
 * be buffered).  */
#define UHARDDOOM_CMD_READ_PTR				0x0064
/* How many bytes are still left in the command buffer after CMD_READ_PTR.  */
#define UHARDDOOM_CMD_READ_SIZE				0x0068
#define UHARDDOOM_CMD_FIFO_GET				0x0070
#define UHARDDOOM_CMD_FIFO_PUT				0x0074
#define UHARDDOOM_CMD_FIFO_WINDOW			0x0078
#define UHARDDOOM_CMD_FIFO_SIZE				0x00000400
#define UHARDDOOM_CMD_PTR_MASK				0xfffffffc
#define UHARDDOOM_CMD_SIZE_MASK				0xfffffffc

/* Section 2.5: FE -- the front end.  Runs firmware that reads the user
 * commands, and converts them to the proper low-level commands understood by
 * COL/SPAN/FX/SWR.  */

/* The FE RAM window.  Reading/writing the WINDOW register reads/writes
 * a word of FE RAM at address ADDR, then auto-increments ADDR by 4.  */
#define UHARDDOOM_FE_RAM_ADDR				0x0100
#define UHARDDOOM_FE_RAM_ADDR_MASK			0x0000fffc
#define UHARDDOOM_FE_RAM_WINDOW				0x0104
/* The current program counter.  */
#define UHARDDOOM_FE_PC					0x0108
#define UHARDDOOM_FE_PC_MASK				0xfffffffc
/* The current execution state.  */
#define UHARDDOOM_FE_STATE				0x010c
/* The core is actively executing instructions.  */
#define UHARDDOOM_FE_STATE_STATE_RUNNING		0x00000000
/* The core is halted due to an error.  */
#define UHARDDOOM_FE_STATE_STATE_ERROR			0x00000001
/* The core is blocked on RUN_WAIT read.  */
#define UHARDDOOM_FE_STATE_STATE_RUN_WAIT		0x00000002
/* The core is blocked on CMD_FETCH read.  */
#define UHARDDOOM_FE_STATE_STATE_CMD_FETCH		0x00000003
/* The core is blocked on a FIFO write.  */
#define UHARDDOOM_FE_STATE_STATE_COLCMD			0x00000004
#define UHARDDOOM_FE_STATE_STATE_SPANCMD		0x00000005
#define UHARDDOOM_FE_STATE_STATE_FXCMD			0x00000006
#define UHARDDOOM_FE_STATE_STATE_SWRCMD			0x00000007
/* The core is blocked on FELOCK read.  */
#define UHARDDOOM_FE_STATE_STATE_FELOCK			0x00000008
#define UHARDDOOM_FE_STATE_STATE_MASK			0x0000000f
/* The pending command code, when core is blocked on a FIFO write.  */
#define UHARDDOOM_FE_STATE_CMD_MASK			0x000000f0
/* The destination register, when core is blocked on a register read.  */
#define UHARDDOOM_FE_STATE_DST_MASK			0x00001f00
#define UHARDDOOM_FE_STATE_MASK				0x00001fff
/* The pending write data, when core is blocked on a FIFO write.  */
#define UHARDDOOM_FE_WRITE_DATA				0x0110
/* The FE error code (set when FE_ERROR interrupt is triggered).  */
#define UHARDDOOM_FE_ERROR_CODE				0x0114
/* XXX add more error codes here */
/* The FE core encountered an illegal instruction.  A is address, B is
 * the instruction opcode.  */
#define UHARDDOOM_FE_ERROR_CODE_ILLEGAL_INSTRUCTION	0x00000080
/* The FE core encountered an instruction fetch bus error.  A is the
 * faulting address.  */
#define UHARDDOOM_FE_ERROR_CODE_BUS_ERROR_EXEC		0x00000081
/* The FE core encountered a read bus error.  A is the faulting
 * address.  */
#define UHARDDOOM_FE_ERROR_CODE_BUS_ERROR_READ		0x00000082
/* The FE core encountered a write bus error.  A is the faulting
 * address, B is the written data.  */
#define UHARDDOOM_FE_ERROR_CODE_BUS_ERROR_WRITE		0x00000083
#define UHARDDOOM_FE_ERROR_CODE_MASK			0x000000ff
/* The FE error data (set when FE_ERROR interrupt is triggered).  */
#define UHARDDOOM_FE_ERROR_DATA_A			0x0118
#define UHARDDOOM_FE_ERROR_DATA_B			0x011c
/* The registers.  */
#define UHARDDOOM_FE_REG(i)				(0x0180 + (i) * 4)
#define UHARDDOOM_FE_REG_NUM				0x20

/* Section 2.6: FIFOs.  */

/* XXX */

/* Section 2.7: TLB.  */

/* The TLB client indices.  */
#define UHARDDOOM_TLB_CLIENT_BATCH			0
#define UHARDDOOM_TLB_CLIENT_CMD			1
#define UHARDDOOM_TLB_CLIENT_SWR_DST			2
#define UHARDDOOM_TLB_CLIENT_COL_CMAP_A			3
#define UHARDDOOM_TLB_CLIENT_COL_CMAP_B			4
#define UHARDDOOM_TLB_CLIENT_COL_SRC			5
#define UHARDDOOM_TLB_CLIENT_SPAN_SRC			6
#define UHARDDOOM_TLB_CLIENT_SWR_TRANSMAP		7
/* The master PD pointers.  */
#define UHARDDOOM_TLB_KERNEL_PDP			0x0400
#define UHARDDOOM_TLB_USER_PDP				0x0404
/* The PDE cache pool (two entries).  */
#define UHARDDOOM_TLB_POOL_PDE_TAG(i)			(0x0410 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PDE_VALUE(i)			(0x0414 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PDE_SIZE			2
/* The per-client singular PTE TLBs.  */
#define UHARDDOOM_TLB_CLIENT_PTE_TAG(i)			(0x0440 + (i) * 8)
#define UHARDDOOM_TLB_CLIENT_PTE_VALUE(i)		(0x0444 + (i) * 8)
/* The PTE cache pool.  */
#define UHARDDOOM_TLB_POOL_PTE_TAG(i)			(0x0500 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PTE_VALUE(i)			(0x0504 + (i) * 8)
#define UHARDDOOM_TLB_POOL_PTE_SIZE			32
/* The tag fields.  */
#define UHARDDOOM_TLB_TAG_VALID				0x00000001
/* Set if the TLB entry relates to kernel PD.  */
#define UHARDDOOM_TLB_TAG_KERNEL			0x00000002
#define UHARDDOOM_TLB_TAG_MASK_PD			0xffc00003
#define UHARDDOOM_TLB_TAG_MASK_PT			0xfffff003

/* Section 2.8: STATS.  */

/* XXX */

/* Section 2.9: SPAN.  */

/* XXX */

/* Section 2.10: FX.  */

/* XXX */

/* Section 2.11: SWR.  */

/* XXX */

/* Section 2.12: COL.  */

/* XXX */

/* Section 2.13: CACHEs.  */

/* XXX */


/* Section 3: Page tables.  */

/* Page directory pointer.  */
#define UHARDDOOM_PDP_MASK				0x0fffffff
#define UHARDDOOM_PDP_SHIFT				12
/* Page directory entry.  */
#define UHARDDOOM_PDE_PRESENT				0x00000001
#define UHARDDOOM_PDE_PA_MASK				0xfffffff0
#define UHARDDOOM_PDE_PA_SHIFT				8
#define UHARDDOOM_PDE_MASK				0xfffffff1
/* Page table entry.  */
#define UHARDDOOM_PTE_PRESENT				0x00000001
#define UHARDDOOM_PTE_WRITABLE				0x00000002
#define UHARDDOOM_PTE_PA_MASK				0xfffffff0
#define UHARDDOOM_PTE_PA_SHIFT				8
#define UHARDDOOM_PTE_MASK				0xfffffff3
/* Splits the VA into PDI + PTI + OFF */
#define UHARDDOOM_VA_PDI(va)				((va) >> 20 & 0x3ff)
#define UHARDDOOM_VA_PTI(va)				((va) >> 12 & 0x3ff)
#define UHARDDOOM_VA_OFF(va)				((va) & 0xfff)
/* The parts of VA that are used for tagging PDE and PTE caches.  */
#define UHARDDOOM_VA_PDE_TAG_MASK			0xffc00000
#define UHARDDOOM_VA_PTE_TAG_MASK			0xfffff000


/* Section 4: The driver commands.  */

/* XXX */


/* Section 5: Misc definitions.  */

/* The block size used for drawing etc (64 pixels).  */
#define UHARDDOOM_BLOCK_SIZE				0x40
/* XXX */


/* Section 6: FE core internal memory map.  */

/* Aliases of the FE_ERROR_DATA_* registers.  */
#define UHARDDOOM_FEMEM_FE_ERROR_DATA_A			0x00000100
#define UHARDDOOM_FEMEM_FE_ERROR_DATA_B			0x00000104
/* When written, triggers FE_ERROR and halts the core.  */
#define UHARDDOOM_FEMEM_FE_ERROR_CODE			0x00000108
/* Read to wait until a job is active.  */
#define UHARDDOOM_FEMEM_JOB_WAIT			0x00000200
/* Write to signal job done.  */
#define UHARDDOOM_FEMEM_JOB_DONE			0x00000204
/* Read only alias of CMD_FE_PTR.  */
#define UHARDDOOM_FEMEM_CMD_PTR				0x00000300
/* If 1, means no more commands in the job.  */
#define UHARDDOOM_FEMEM_CMD_END				0x00000304
/* Read to get the next command word.  */
#define UHARDDOOM_FEMEM_CMD_FETCH			0x00000308
/* Write to submit a command to a FIFO.  */
#define UHARDDOOM_FEMEM_COLCMD(t)			(0x00000400 + (t) * 4)
#define UHARDDOOM_FEMEM_SPANCMD(t)			(0x00000500 + (t) * 4)
#define UHARDDOOM_FEMEM_FXCMD(t)			(0x00000600 + (t) * 4)
#define UHARDDOOM_FEMEM_SWRCMD(t)			(0x00000700 + (t) * 4)
/* Write to bump a STATS counter.  */
#define UHARDDOOM_FEMEM_STATS_BUMP(t)			(0x00000800 + (t) * 4)
/* The code/data RAM.  */
#define UHARDDOOM_FEMEM_RAM_BASE			0x80000000
#define UHARDDOOM_FEMEM_RAM_SIZE			0x10000


/* Section 7: Internal commands.  */

/* Section 7.1: COLCMD -- COL unit internal commands.  */

/* XXX */

/* Section 7.2: SPANCMD -- SPAN unit internal commands.  */

/* XXX */

/* Section 7.3: FXCMD -- FX unit internal commands.  */

/* XXX */

/* Section 7.4: SWRCMD -- SWR unit internal commands.  */

/* XXX */


#endif
