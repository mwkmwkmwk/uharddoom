#include "uharddoom.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>

__asm__(
	".section start, \"ax\"\n"
	"_start:\n"
	"lui	sp, 0xc0010\n"
	"lui	a0, %hi(_data_image_start)\n"
	"addi	a0, a0, %lo(_data_image_start)\n"
	"lui	a1, %hi(_data_start)\n"
	"addi	a1, a1, %lo(_data_start)\n"
	"lui	a2, %hi(_data_end)\n"
	"addi	a2, a2, %lo(_data_end)\n"
	"beq	a1, a2, .Linit_bss\n"
	".Linit_data_loop:\n"
	"lb	a3, (a0)\n"
	"sb	a3, (a1)\n"
	"addi	a0, a0, 1\n"
	"addi	a1, a1, 1\n"
	"bne	a1, a2, .Linit_data_loop\n"
	".Linit_bss:\n"
	"lui	a2,%hi(_bss_end)\n"
	"addi	a2,a2,%lo(_bss_end)\n"
	"beq	a1, a2, .Linit_bss_done\n"
	".Linit_bss_loop:\n"
	"sb	zero, (a1)\n"
	"addi	a1, a1, 1\n"
	"bne	a1, a2, .Linit_bss_loop\n"
	".Linit_bss_done:\n"
	"j	main\n"
);

static volatile uint32_t *const FE_ERROR_DATA_A = (void *)UHARDDOOM_FEMEM_FE_ERROR_DATA_A;
static volatile uint32_t *const FE_ERROR_DATA_B = (void *)UHARDDOOM_FEMEM_FE_ERROR_DATA_B;
static volatile uint32_t *const FE_ERROR_CODE = (void *)UHARDDOOM_FEMEM_FE_ERROR_CODE;

static volatile uint32_t *const JOB_WAIT = (void *)UHARDDOOM_FEMEM_JOB_WAIT;
static volatile uint32_t *const JOB_DONE = (void *)UHARDDOOM_FEMEM_JOB_DONE;

static volatile uint32_t *const CMD_PTR = (void *)UHARDDOOM_FEMEM_CMD_PTR;
static volatile uint32_t *const CMD_END = (void *)UHARDDOOM_FEMEM_CMD_END;
static volatile uint32_t *const CMD_FETCH = (void *)UHARDDOOM_FEMEM_CMD_FETCH;

static volatile uint32_t *const SRDCMD = (void *)UHARDDOOM_FEMEM_SRDCMD(0);
static volatile uint32_t *const SPANCMD = (void *)UHARDDOOM_FEMEM_SPANCMD(0);
static volatile uint32_t *const COLCMD = (void *)UHARDDOOM_FEMEM_COLCMD(0);
static volatile uint32_t *const FXCMD = (void *)UHARDDOOM_FEMEM_FXCMD(0);
static volatile uint32_t *const SWRCMD = (void *)UHARDDOOM_FEMEM_SWRCMD(0);

static uint32_t cmd_ptr;

static noreturn void error(int code, uint32_t data_a, uint32_t data_b) {
	*FE_ERROR_DATA_A = data_a;
	*FE_ERROR_DATA_B = data_b;
	*FE_ERROR_CODE = code;
	__builtin_unreachable();
}

static void cmd_fill_rect(uint32_t cmd_header) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_FILL_COLOR] = UHARDDOOM_USER_FILL_RECT_HEADER_EXTR_COLOR(cmd_header);
	uint32_t dst_ptr = *CMD_FETCH;
	if (dst_ptr & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PTR_UNALIGNED, cmd_ptr, dst_ptr);
	uint32_t dst_pitch = *CMD_FETCH;
	if (dst_pitch & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PITCH_UNALIGNED, cmd_ptr, dst_pitch);
	uint32_t w3 = *CMD_FETCH;
	uint32_t x = UHARDDOOM_USER_FILL_RECT_W3_EXTR_X(w3);
	uint32_t y = UHARDDOOM_USER_FILL_RECT_W3_EXTR_Y(w3);
	uint32_t w4 = *CMD_FETCH;
	uint32_t w = UHARDDOOM_USER_FILL_RECT_W4_EXTR_W(w4);
	uint32_t h = UHARDDOOM_USER_FILL_RECT_W4_EXTR_H(w4);
	dst_ptr += y * dst_pitch;
	dst_ptr += x & ~UHARDDOOM_BLOCK_MASK;
	x &= UHARDDOOM_BLOCK_MASK;
	uint32_t skip_end = -(w + x) & UHARDDOOM_BLOCK_MASK;
	uint32_t blocks = (w + x + skip_end) >> UHARDDOOM_BLOCK_SHIFT;
	FXCMD[UHARDDOOM_FXCMD_TYPE_SKIP] = UHARDDOOM_FXCMD_DATA_SKIP(x, skip_end);
	while (h--) {
		FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(blocks, false, false, false, false, false);
		SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = dst_ptr;
		SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(1, blocks, false, false, false);
		dst_ptr += dst_pitch;
	}
}

static void draw_line_horiz_seg(uint32_t dst_ptr, uint32_t xb, uint32_t xe) {
	uint32_t skip_begin = xb & UHARDDOOM_BLOCK_MASK;
	uint32_t skip_end = -xe & UHARDDOOM_BLOCK_MASK;
	uint32_t blocks = (skip_begin + skip_end + xe - xb) >> UHARDDOOM_BLOCK_SHIFT;
	FXCMD[UHARDDOOM_FXCMD_TYPE_SKIP] = UHARDDOOM_FXCMD_DATA_SKIP(skip_begin, skip_end);
	FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(blocks, false, false, false, false, false);
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = dst_ptr + (xb & ~UHARDDOOM_BLOCK_MASK);
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(1, blocks, false, false, false);
}

static void draw_line_vert_seg(uint32_t dst_ptr, uint32_t x, uint32_t num) {
	uint32_t xl = x & UHARDDOOM_BLOCK_MASK;
	FXCMD[UHARDDOOM_FXCMD_TYPE_SKIP] = UHARDDOOM_FXCMD_DATA_SKIP(xl, UHARDDOOM_BLOCK_MASK - xl);
	FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(1, false, false, false, false, false);
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = dst_ptr + (x & ~UHARDDOOM_BLOCK_MASK);
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(num, 1, false, false, false);
}

static void cmd_draw_line(uint32_t cmd_header) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_FILL_COLOR] = UHARDDOOM_USER_DRAW_LINE_HEADER_EXTR_COLOR(cmd_header);
	uint32_t dst_ptr = *CMD_FETCH;
	if (dst_ptr & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PTR_UNALIGNED, cmd_ptr, dst_ptr);
	uint32_t dst_pitch = *CMD_FETCH;
	if (dst_pitch & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PITCH_UNALIGNED, cmd_ptr, dst_pitch);
	uint32_t w3 = *CMD_FETCH;
	uint32_t x1 = UHARDDOOM_USER_DRAW_LINE_W3_EXTR_X(w3);
	uint32_t y1 = UHARDDOOM_USER_DRAW_LINE_W3_EXTR_Y(w3);
	uint32_t w4 = *CMD_FETCH;
	uint32_t x2 = UHARDDOOM_USER_DRAW_LINE_W4_EXTR_X(w4);
	uint32_t y2 = UHARDDOOM_USER_DRAW_LINE_W4_EXTR_Y(w4);
	/* Make sure x1/y1 is the leftmost endpoint.  */
	if (x1 > x2) {
		uint32_t tmp = x1;
		x1 = x2;
		x2 = tmp;
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	uint32_t dx = x2 - x1;
	/* Move dst_ptr vertically to the starting line.  */
	dst_ptr += dst_pitch * y1;
	uint32_t dy;
	if (y2 > y1) {
		/* Drawing line downwards.  */
		dy = y2 - y1;
	} else {
		/* Drawing line upwards.  Adjust the pitch.  */
		dy = y1 - y2;
		dst_pitch = -dst_pitch;
	}
	if (dx > dy) {
		/* Mostly-horizontal line.  */
		int32_t delta = 2 * (int32_t)dy - (int32_t)dx;
		uint32_t xlast = x1;
		for (uint32_t x = x1; x <= x2; x++) {
			if (delta > 0) {
				draw_line_horiz_seg(dst_ptr, xlast, x);
				dst_ptr += dst_pitch;
				xlast = x;
				delta -= 2 * dx;
			}
			delta += 2 * dy;
		}
		draw_line_horiz_seg(dst_ptr, xlast, x2 + 1);
	} else {
		/* Mostly-vertical line.  */
		SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PITCH] = dst_pitch;
		int32_t delta = 2 * (int32_t)dx - (int32_t)dy;
		uint32_t num = 0;
		uint32_t n = dy + 1;
		while (n--) {
			num++;
			if (delta > 0) {
				draw_line_vert_seg(dst_ptr, x1, num);
				dst_ptr += dst_pitch * num;
				num = 0;
				x1++;
				delta -= 2 * dy;
			}
			delta += 2 * dx;
		}
		draw_line_vert_seg(dst_ptr, x1, num);
	}
}

static void cmd_blit(uint32_t cmd_header) {
	uint32_t ulog = UHARDDOOM_USER_BLIT_HEADER_EXTR_ULOG(cmd_header);
	uint32_t vlog = UHARDDOOM_USER_BLIT_HEADER_EXTR_VLOG(cmd_header);
	uint32_t dst_ptr = *CMD_FETCH;
	if (dst_ptr & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PTR_UNALIGNED, cmd_ptr, dst_ptr);
	uint32_t dst_pitch = *CMD_FETCH;
	if (dst_pitch & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PITCH_UNALIGNED, cmd_ptr, dst_pitch);
	uint32_t w3 = *CMD_FETCH;
	uint32_t dst_x = UHARDDOOM_USER_BLIT_W3_EXTR_X(w3);
	uint32_t dst_y = UHARDDOOM_USER_BLIT_W3_EXTR_Y(w3);
	uint32_t w4 = *CMD_FETCH;
	uint32_t dst_w = UHARDDOOM_USER_BLIT_W4_EXTR_W(w4);
	uint32_t dst_h = UHARDDOOM_USER_BLIT_W4_EXTR_H(w4);
	uint32_t src_ptr = *CMD_FETCH;
	uint32_t src_pitch = *CMD_FETCH;
	uint32_t w7 = *CMD_FETCH;
	uint32_t src_x = UHARDDOOM_USER_BLIT_W7_EXTR_X(w7);
	uint32_t src_y = UHARDDOOM_USER_BLIT_W7_EXTR_Y(w7);
	uint32_t w8 = *CMD_FETCH;
	uint32_t src_w = UHARDDOOM_USER_BLIT_W8_EXTR_W(w8);
	uint32_t src_h = UHARDDOOM_USER_BLIT_W8_EXTR_H(w8);
	/* Adjust the pointers.  */
	dst_ptr += dst_y * dst_pitch;
	dst_ptr += dst_x & ~UHARDDOOM_BLOCK_MASK;
	dst_x &= UHARDDOOM_BLOCK_MASK;
	/* Decide on the path.  */
	if (src_w == dst_w && src_h == dst_h && (src_x & UHARDDOOM_BLOCK_MASK) == dst_x && ulog == 0x10 && vlog == 0x10 && !(src_ptr & UHARDDOOM_BLOCK_MASK) && !(src_pitch & UHARDDOOM_BLOCK_MASK)) {
		/* Simple case, no scaling, no intra-block shift — use SRD.  */
		src_ptr += src_y * src_pitch;
		src_ptr += src_x & ~UHARDDOOM_BLOCK_MASK;
		src_x &= UHARDDOOM_BLOCK_MASK;
		/* Finish SWR work.  */
		SWRCMD[UHARDDOOM_SWRCMD_TYPE_SRDLOCK] = 0;
		SRDCMD[UHARDDOOM_SRDCMD_TYPE_SRDLOCK] = 0;
		/* Prepare the skip.  */
		uint32_t skip_end = -(dst_w + dst_x) & UHARDDOOM_BLOCK_MASK;
		uint32_t blocks = (dst_w + dst_x + skip_end) >> UHARDDOOM_BLOCK_SHIFT;
		FXCMD[UHARDDOOM_FXCMD_TYPE_SKIP] = UHARDDOOM_FXCMD_DATA_SKIP(dst_x, skip_end);
		SRDCMD[UHARDDOOM_SRDCMD_TYPE_SRC_PITCH] = UHARDDOOM_BLOCK_SIZE;
		while (dst_h--) {
			SRDCMD[UHARDDOOM_SRDCMD_TYPE_SRC_PTR] = src_ptr;
			SRDCMD[UHARDDOOM_SRDCMD_TYPE_READ] = UHARDDOOM_SRDCMD_DATA_READ(blocks, false);
			FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(blocks, false, false, false, true, false);
			SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = dst_ptr;
			SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(1, blocks, false, false, false);
			src_ptr += src_pitch;
			dst_ptr += dst_pitch;
		}
	} else {
		/* Hard case, use SPAN.  */
		SPANCMD[UHARDDOOM_SPANCMD_TYPE_SRC_PTR] = src_ptr;
		SPANCMD[UHARDDOOM_SPANCMD_TYPE_SRC_PITCH] = src_pitch;
		SPANCMD[UHARDDOOM_SPANCMD_TYPE_UVMASK] = UHARDDOOM_SPANCMD_DATA_UVMASK(ulog, vlog);
		uint32_t ustep = (src_w << 16) / dst_w;
		uint32_t ustart = (src_x << 16) + 0x8000;
		uint32_t vstep = (src_h << 16) / dst_h;
		uint32_t vstart = (src_y << 16) + 0x8000;
		SPANCMD[UHARDDOOM_SPANCMD_TYPE_USTEP] = ustep;
		SPANCMD[UHARDDOOM_SPANCMD_TYPE_VSTEP] = 0;
		/* Finish SWR work.  */
		SWRCMD[UHARDDOOM_SWRCMD_TYPE_SPANLOCK] = 0;
		SPANCMD[UHARDDOOM_SPANCMD_TYPE_SPANLOCK] = 0;
		/* Prepare the skip.  */
		uint32_t skip_end = -(dst_w + dst_x) & UHARDDOOM_BLOCK_MASK;
		uint32_t blocks = (dst_w + dst_x + skip_end) >> UHARDDOOM_BLOCK_SHIFT;
		FXCMD[UHARDDOOM_FXCMD_TYPE_SKIP] = UHARDDOOM_FXCMD_DATA_SKIP(dst_x, skip_end);
		while (dst_h--) {
			SPANCMD[UHARDDOOM_SPANCMD_TYPE_USTART] = ustart;
			SPANCMD[UHARDDOOM_SPANCMD_TYPE_VSTART] = vstart;
			SPANCMD[UHARDDOOM_SPANCMD_TYPE_DRAW] = UHARDDOOM_SPANCMD_DATA_DRAW(dst_w, dst_x);
			FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(blocks, false, false, false, false, true);
			SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = dst_ptr;
			SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(1, blocks, false, false, false);
			vstart += vstep;
			dst_ptr += dst_pitch;
		}
	}
}

#define HEAP_MAX 0x400

uint32_t heap[HEAP_MAX];
uint32_t heap_size;

static inline uint32_t heap_left() {
	return HEAP_MAX - heap_size;
}
static inline bool heap_empty() {
	return heap_size == 0;
}

static inline uint32_t heap_pidx(uint32_t idx) {
	return (idx - 1) >> 1;
}

static inline uint32_t heap_cidx(uint32_t idx) {
	return (idx << 1) + 1;
}

static void heap_put(uint32_t word) {
	uint32_t idx = heap_size++;
	while (idx && word < heap[heap_pidx(idx)]) {
		heap[idx] = heap[heap_pidx(idx)];
		idx = heap_pidx(idx);
	}
	heap[idx] = word;
}

static uint32_t heap_get() {
	uint32_t res = heap[0];
	uint32_t word = heap[--heap_size];
	uint32_t idx = 0;
	while (heap_cidx(idx) < heap_size) {
		if (heap_cidx(idx) + 1 < heap_size) {
			/* Two children.  */
			uint32_t c0 = heap[heap_cidx(idx)];
			uint32_t c1 = heap[heap_cidx(idx) + 1];
			if (c0 < word && c0 < c1) {
				heap[idx] = c0;
				idx = heap_cidx(idx);
			} else if (c1 < word) {
				heap[idx] = c1;
				idx = heap_cidx(idx) + 1;
			} else {
				break;
			}
		} else {
			/* One child.  */
			if (heap[heap_cidx(idx)] < word) {
				heap[idx] = heap[heap_cidx(idx)];
				idx = heap_cidx(idx);
			} else {
				break;
			}
		}
	}
	heap[idx] = word;
	return res;
}

enum {
	WIPE_OP_START_A = 0,
	WIPE_OP_STOP_A = 1,
	WIPE_OP_START_B = 2,
	WIPE_OP_STOP_B = 3,
};

#define WIPE_OP(op, x, y) ((y) << 16 | (op) << 8 | (x))

static void wipe_flush(uint32_t dst_ptr, uint32_t dst_pitch, uint32_t src_a_ptr, uint32_t src_a_pitch, uint32_t src_b_ptr, uint32_t src_b_pitch) {
	uint32_t active = 0;
	uint32_t ylast = 0;
	while (!heap_empty()) {
		uint32_t opw = heap_get();
		uint32_t opy = opw >> 16;
		uint32_t op = opw >> 8 & 3;
		uint32_t opx = opw & 0x3f;
		if (opy != ylast && active) {
			uint32_t num = opy - ylast;
			COLCMD[UHARDDOOM_COLCMD_TYPE_DRAW] = UHARDDOOM_COLCMD_DATA_DRAW(num, false);
			SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = dst_ptr + opy * dst_pitch;
			SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(num, 1, true, true, false);
		}
		ylast = opy;
		switch (op) {
			case WIPE_OP_START_A:
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SRC_PTR] = src_a_ptr + opx;
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SRC_PITCH] = src_a_pitch;
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SETUP] = UHARDDOOM_COLCMD_DATA_COL_SETUP(opx, 0x10, true, false, true);
				active++;
				break;
			case WIPE_OP_START_B:
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SRC_PTR] = src_b_ptr + opx;
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SRC_PITCH] = src_b_pitch;
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SETUP] = UHARDDOOM_COLCMD_DATA_COL_SETUP(opx, 0x10, true, false, true);
				active++;
				break;
			case WIPE_OP_STOP_A:
			case WIPE_OP_STOP_B:
				COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SETUP] = UHARDDOOM_COLCMD_DATA_COL_SETUP(opx, 0x10, false, false, false);
				active--;
				break;
		}
	}
}

static void cmd_wipe(uint32_t cmd_header) {
	uint32_t dst_ptr = *CMD_FETCH;
	if (dst_ptr & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PTR_UNALIGNED, cmd_ptr, dst_ptr);
	uint32_t dst_pitch = *CMD_FETCH;
	if (dst_pitch & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PITCH_UNALIGNED, cmd_ptr, dst_pitch);
	uint32_t w3 = *CMD_FETCH;
	uint32_t x = UHARDDOOM_USER_WIPE_W3_EXTR_X(w3);
	uint32_t y = UHARDDOOM_USER_WIPE_W3_EXTR_Y(w3);
	uint32_t w4 = *CMD_FETCH;
	uint32_t w = UHARDDOOM_USER_WIPE_W4_EXTR_W(w4);
	uint32_t h = UHARDDOOM_USER_WIPE_W4_EXTR_H(w4);
	uint32_t src_a_ptr = *CMD_FETCH;
	uint32_t src_a_pitch = *CMD_FETCH;
	uint32_t src_b_ptr = *CMD_FETCH;
	uint32_t src_b_pitch = *CMD_FETCH;
	dst_ptr += y * dst_pitch;
	src_a_ptr += y * src_a_pitch;
	src_b_ptr += y * src_b_pitch;
	dst_ptr += (x & ~UHARDDOOM_BLOCK_MASK);
	src_a_ptr += (x & ~UHARDDOOM_BLOCK_MASK);
	src_b_ptr += (x & ~UHARDDOOM_BLOCK_MASK);
	x &= UHARDDOOM_BLOCK_MASK;
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_USTART] = 0;
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_USTEP] = 0x10000;
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PITCH] = dst_pitch;
	while (w--) {
		uint32_t yoff = *CMD_FETCH & UHARDDOOM_COORD_MASK;
		if (yoff) {
			heap_put(WIPE_OP(WIPE_OP_START_A, x, 0));
			heap_put(WIPE_OP(WIPE_OP_STOP_A, x, yoff));
		}
		if (yoff != h) {
			heap_put(WIPE_OP(WIPE_OP_START_B, x, yoff));
			heap_put(WIPE_OP(WIPE_OP_STOP_B, x, h));
		}
		x++;
		if (x == UHARDDOOM_BLOCK_SIZE) {
			wipe_flush(dst_ptr, dst_pitch, src_a_ptr, src_a_pitch, src_b_ptr, src_b_pitch);
			x = 0;
			dst_ptr += UHARDDOOM_BLOCK_SIZE;
			src_a_ptr += UHARDDOOM_BLOCK_SIZE;
			src_b_ptr += UHARDDOOM_BLOCK_SIZE;
		}
	}
	if (!heap_empty())
		wipe_flush(dst_ptr, dst_pitch, src_a_ptr, src_a_pitch, src_b_ptr, src_b_pitch);
}

static void cmd_draw_columns(uint32_t cmd_header) {
	/* XXX */
	__builtin_trap();
}

static void cmd_draw_fuzz(uint32_t cmd_header) {
	/* XXX */
	__builtin_trap();
}

static void cmd_draw_spans(uint32_t cmd_header) {
	/* XXX */
	__builtin_trap();
}

noreturn void main() {
	while (1) {
		*JOB_WAIT;
		while (!*CMD_END) {
			cmd_ptr = *CMD_PTR;
			uint32_t cmd_header = *CMD_FETCH;
			switch (UHARDDOOM_USER_CMD_HEADER_EXTR_TYPE(cmd_header)) {
				case UHARDDOOM_USER_CMD_TYPE_FILL_RECT:
					cmd_fill_rect(cmd_header);
					break;
				case UHARDDOOM_USER_CMD_TYPE_DRAW_LINE:
					cmd_draw_line(cmd_header);
					break;
				case UHARDDOOM_USER_CMD_TYPE_BLIT:
					cmd_blit(cmd_header);
					break;
				case UHARDDOOM_USER_CMD_TYPE_WIPE:
					cmd_wipe(cmd_header);
					break;
				case UHARDDOOM_USER_CMD_TYPE_DRAW_COLUMNS:
					cmd_draw_columns(cmd_header);
					break;
				case UHARDDOOM_USER_CMD_TYPE_DRAW_FUZZ:
					cmd_draw_fuzz(cmd_header);
					break;
				case UHARDDOOM_USER_CMD_TYPE_DRAW_SPANS:
					cmd_draw_spans(cmd_header);
					break;
				default:
					error(UHARDDOOM_FE_ERROR_CODE_UNK_USER_COMMAND, cmd_ptr, cmd_header);
			}
		}
		*JOB_DONE = 0;
	}
}
