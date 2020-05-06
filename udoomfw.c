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
	".text\n"
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

static volatile uint32_t *const FESEM = (void *)UHARDDOOM_FEMEM_FESEM;

static volatile uint32_t *const STAT_BUMP = (void *)UHARDDOOM_FEMEM_STAT_BUMP(0);

static uint32_t cmd_ptr;

static noreturn void error(int code, uint32_t data_a, uint32_t data_b) {
	*FE_ERROR_DATA_A = data_a;
	*FE_ERROR_DATA_B = data_b;
	*FE_ERROR_CODE = code;
	__builtin_unreachable();
}

static void srdcmd_src_ptr(uint32_t val) {
	SRDCMD[UHARDDOOM_SRDCMD_TYPE_SRC_PTR] = val;
}

static void srdcmd_src_pitch(uint32_t val) {
	SRDCMD[UHARDDOOM_SRDCMD_TYPE_SRC_PITCH] = val;
}

static void srdcmd_read_fx(uint32_t num) {
	SRDCMD[UHARDDOOM_SRDCMD_TYPE_READ] = UHARDDOOM_SRDCMD_DATA_READ(num, false);
}

static void srdcmd_read_col(uint32_t num) {
	SRDCMD[UHARDDOOM_SRDCMD_TYPE_READ] = UHARDDOOM_SRDCMD_DATA_READ(num, true);
}

static void srdsem(void) {
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_SRDSEM] = 0;
	SRDCMD[UHARDDOOM_SRDCMD_TYPE_SRDSEM] = 0;
}

static void spancmd_src_ptr(uint32_t val) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_SRC_PTR] = val;
}

static void spancmd_src_pitch(uint32_t val) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_SRC_PITCH] = val;
}

static void spancmd_uvmask(uint32_t ulog, uint32_t vlog) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_UVMASK] = UHARDDOOM_SPANCMD_DATA_UVMASK(ulog, vlog);
}

static void spancmd_ustart(uint32_t val) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_USTART] = val;
}

static void spancmd_ustep(uint32_t val) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_USTEP] = val;
}

static void spancmd_vstart(uint32_t val) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_VSTART] = val;
}

static void spancmd_vstep(uint32_t val) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_VSTEP] = val;
}

static void spancmd_draw(uint32_t num, uint32_t xoff) {
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_DRAW] = UHARDDOOM_SPANCMD_DATA_DRAW(num, xoff);
}

static void spansem(void) {
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_SPANSEM] = 0;
	SPANCMD[UHARDDOOM_SPANCMD_TYPE_SPANSEM] = 0;
}

static void colcmd_col_cmap_b_ptr(uint32_t val) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_CMAP_B_PTR] = val;
}

static void colcmd_col_src_ptr(uint32_t val) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SRC_PTR] = val;
}

static void colcmd_col_src_pitch(uint32_t val) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SRC_PITCH] = val;
}

static void colcmd_col_ustart(uint32_t val) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_USTART] = val;
}

static void colcmd_col_ustep(uint32_t val) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_USTEP] = val;
}

static void colcmd_col_enable(uint32_t x, bool cmap_b_en, uint32_t height) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SETUP] = UHARDDOOM_COLCMD_DATA_COL_SETUP(x, true, cmap_b_en, height);
}

static void colcmd_col_disable(uint32_t x) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_COL_SETUP] = UHARDDOOM_COLCMD_DATA_COL_SETUP(x, false, false, 0);
}

static void colcmd_load_cmap_a(void) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_LOAD_CMAP_A] = 0;
}

static void colcmd_draw(uint32_t num, bool cmap_a_en) {
	COLCMD[UHARDDOOM_COLCMD_TYPE_DRAW] = UHARDDOOM_COLCMD_DATA_DRAW(num, cmap_a_en);
}

static void colsem(void) {
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_COLSEM] = 0;
	COLCMD[UHARDDOOM_COLCMD_TYPE_COLSEM] = 0;
}

static void fxcmd_load_cmap(void) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_LOAD_CMAP] = 0;
}

static void fxcmd_load_block(void) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_LOAD_BLOCK] = 0;
}

static void fxcmd_load_fuzz(void) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_LOAD_FUZZ] = 0;
}

static void fxcmd_fill_color(uint32_t color) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_FILL_COLOR] = color;
}

static void fxcmd_col_enable(uint32_t x, uint32_t fuzzpos) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_COL_SETUP] = UHARDDOOM_FXCMD_DATA_COL_SETUP(x, fuzzpos, true);
}

static void fxcmd_col_disable(uint32_t x) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_COL_SETUP] = UHARDDOOM_FXCMD_DATA_COL_SETUP(x, 0, false);
}

static void fxcmd_skip(uint32_t sb, uint32_t se, bool sa) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_SKIP] = UHARDDOOM_FXCMD_DATA_SKIP(sb, se, sa);
}

static void fxcmd_draw_buf(uint32_t num) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(num, false, false, false, false);
}

static void fxcmd_draw_span(uint32_t num, bool cmap_en) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(num, cmap_en, false, false, true);
}

static void fxcmd_draw_srd(uint32_t num, bool cmap_en) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(num, cmap_en, false, true, false);
}

static void fxcmd_draw_fuzz(uint32_t num) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_DRAW] = UHARDDOOM_FXCMD_DATA_DRAW(num, false, true, true, false);
}

static void swrcmd_transmap_ptr(uint32_t val) {
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_TRANSMAP_PTR] = val;
}

static void swrcmd_dst_ptr(uint32_t val) {
	if (val & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PTR_UNALIGNED, cmd_ptr, val);
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PTR] = val;
}

static void swrcmd_dst_pitch(uint32_t val) {
	if (val & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_DST_PITCH_UNALIGNED, cmd_ptr, val);
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PITCH] = val;
}

static void swrcmd_draw_fx(uint32_t num, bool trans_en) {
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(num, false, trans_en);
}

static void swrcmd_draw_col(uint32_t num, bool trans_en) {
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DRAW] = UHARDDOOM_SWRCMD_DATA_DRAW(num, true, trans_en);
}

static void fesem(void) {
	SRDCMD[UHARDDOOM_SRDCMD_TYPE_FESEM] = 0;
	*FESEM;
}

static void assert(bool x) {
	if (!x)
		__builtin_trap();
}

static void cmd_fill_rect(uint32_t cmd_header) {
	STAT_BUMP[UHARDDOOM_STAT_FW_FILL_RECT] = 1;
	fxcmd_fill_color(UHARDDOOM_USER_FILL_RECT_HEADER_EXTR_COLOR(cmd_header));
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
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
	fxcmd_skip(x, skip_end, false);
	swrcmd_dst_pitch(UHARDDOOM_BLOCK_SIZE);
	while (h--) {
		STAT_BUMP[UHARDDOOM_STAT_FW_FILL_RECT_SPAN] = 1;
		fxcmd_draw_buf(blocks);
		swrcmd_dst_ptr(dst_ptr);
		swrcmd_draw_fx(blocks, false);
		dst_ptr += dst_pitch;
	}
}

static void draw_line_horiz_seg(uint32_t dst_ptr, uint32_t xb, uint32_t xe) {
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_LINE_HORIZ_SEG] = 1;
	uint32_t skip_begin = xb & UHARDDOOM_BLOCK_MASK;
	uint32_t skip_end = -xe & UHARDDOOM_BLOCK_MASK;
	uint32_t blocks = (skip_begin + skip_end + xe - xb) >> UHARDDOOM_BLOCK_SHIFT;
	fxcmd_skip(skip_begin, skip_end, false);
	fxcmd_draw_buf(blocks);
	swrcmd_dst_ptr(dst_ptr + (xb & ~UHARDDOOM_BLOCK_MASK));
	swrcmd_draw_fx(blocks, false);
}

static void draw_line_vert_seg(uint32_t dst_ptr, uint32_t x, uint32_t num) {
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_LINE_VERT_SEG] = 1;
	uint32_t xl = x & UHARDDOOM_BLOCK_MASK;
	fxcmd_skip(xl, UHARDDOOM_BLOCK_MASK - xl, true);
	fxcmd_draw_buf(num);
	swrcmd_dst_ptr(dst_ptr + (x & ~UHARDDOOM_BLOCK_MASK));
	swrcmd_draw_fx(num, false);
}

static void cmd_draw_line(uint32_t cmd_header) {
	fxcmd_fill_color(UHARDDOOM_USER_DRAW_LINE_HEADER_EXTR_COLOR(cmd_header));
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
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
		STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_LINE_HORIZ] = 1;
		swrcmd_dst_pitch(UHARDDOOM_BLOCK_SIZE);
		int32_t delta = 2 * (int32_t)dy - (int32_t)dx;
		uint32_t xlast = x1;
		for (uint32_t x = x1; x <= x2; x++) {
			if (delta > 0) {
				draw_line_horiz_seg(dst_ptr, xlast, x+1);
				dst_ptr += dst_pitch;
				xlast = x+1;
				delta -= 2 * dx;
			}
			delta += 2 * dy;
		}
		if (xlast != x2 + 1)
			draw_line_horiz_seg(dst_ptr, xlast, x2 + 1);
	} else {
		/* Mostly-vertical line.  */
		STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_LINE_VERT] = 1;
		swrcmd_dst_pitch(dst_pitch);
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
		if (num)
			draw_line_vert_seg(dst_ptr, x1, num);
	}
}

static void cmd_blit(uint32_t cmd_header) {
	uint32_t ulog = UHARDDOOM_USER_BLIT_HEADER_EXTR_ULOG(cmd_header);
	uint32_t vlog = UHARDDOOM_USER_BLIT_HEADER_EXTR_VLOG(cmd_header);
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
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
	swrcmd_dst_pitch(UHARDDOOM_BLOCK_SIZE);
	/* Decide on the path.  */
	if (src_w == dst_w && src_h == dst_h && (src_x & UHARDDOOM_BLOCK_MASK) == dst_x && ulog == 0x10 && vlog == 0x10 && !(src_ptr & UHARDDOOM_BLOCK_MASK) && !(src_pitch & UHARDDOOM_BLOCK_MASK)) {
		/* Simple case, no scaling, no intra-block shift — use SRD.  */
		STAT_BUMP[UHARDDOOM_STAT_FW_BLIT_SIMPLE] = 1;
		src_ptr += src_y * src_pitch;
		src_ptr += src_x & ~UHARDDOOM_BLOCK_MASK;
		src_x &= UHARDDOOM_BLOCK_MASK;
		/* Finish SWR work.  */
		srdsem();
		/* Prepare the skip.  */
		uint32_t skip_end = -(dst_w + dst_x) & UHARDDOOM_BLOCK_MASK;
		uint32_t blocks = (dst_w + dst_x + skip_end) >> UHARDDOOM_BLOCK_SHIFT;
		fxcmd_skip(dst_x, skip_end, false);
		srdcmd_src_pitch(UHARDDOOM_BLOCK_SIZE);
		while (dst_h--) {
			STAT_BUMP[UHARDDOOM_STAT_FW_BLIT_SIMPLE_SPAN] = 1;
			srdcmd_src_ptr(src_ptr);
			srdcmd_read_fx(blocks);
			fxcmd_draw_srd(blocks, false);
			swrcmd_dst_ptr(dst_ptr);
			swrcmd_draw_fx(blocks, false);
			src_ptr += src_pitch;
			dst_ptr += dst_pitch;
		}
	} else {
		/* Complex case, use SPAN.  */
		STAT_BUMP[UHARDDOOM_STAT_FW_BLIT_COMPLEX] = 1;
		spancmd_src_ptr(src_ptr);
		spancmd_src_pitch(src_pitch);
		spancmd_uvmask(ulog, vlog);
		uint32_t ustep = (src_w << 16) / dst_w;
		uint32_t ustart = (src_x << 16);
		uint32_t vstep = (src_h << 16) / dst_h;
		uint32_t vstart = (src_y << 16);
		spancmd_ustep(ustep);
		spancmd_vstep(0);
		/* Finish SWR work.  */
		spansem();
		/* Prepare the skip.  */
		uint32_t skip_end = -(dst_w + dst_x) & UHARDDOOM_BLOCK_MASK;
		uint32_t blocks = (dst_w + dst_x + skip_end) >> UHARDDOOM_BLOCK_SHIFT;
		fxcmd_skip(dst_x, skip_end, false);
		while (dst_h--) {
			STAT_BUMP[UHARDDOOM_STAT_FW_BLIT_COMPLEX_SPAN] = 1;
			spancmd_ustart(ustart);
			spancmd_vstart(vstart);
			spancmd_draw(dst_w, dst_x);
			fxcmd_draw_span(blocks, false);
			swrcmd_dst_ptr(dst_ptr);
			swrcmd_draw_fx(blocks, false);
			vstart += vstep;
			dst_ptr += dst_pitch;
		}
	}
}

#define HEAP_MAX 0x400

static uint32_t heap[HEAP_MAX];
static uint32_t heap_size;

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

#define WIPE_OP(op, x, y) ((y) << 12 | (op) << 8 | (x))

static void wipe_flush(uint32_t dst_ptr, uint32_t dst_pitch, uint32_t src_a_ptr, uint32_t src_a_pitch, uint32_t src_b_ptr, uint32_t src_b_pitch) {
	uint32_t active = 0;
	uint32_t ylast = 0;
	STAT_BUMP[UHARDDOOM_STAT_FW_WIPE_BATCH] = 1;
	while (!heap_empty()) {
		uint32_t opw = heap_get();
		uint32_t opy = opw >> 12;
		uint32_t op = opw >> 8 & 3;
		uint32_t opx = opw & 0x3f;
		if (opy != ylast && active) {
			uint32_t num = opy - ylast;
			colcmd_draw(num, false);
			swrcmd_dst_ptr(dst_ptr + ylast * dst_pitch);
			swrcmd_draw_col(num, false);
			STAT_BUMP[UHARDDOOM_STAT_FW_WIPE_SEG] = 1;
		}
		ylast = opy;
		switch (op) {
			case WIPE_OP_START_A:
				colcmd_col_src_ptr(src_a_ptr + opx);
				colcmd_col_src_pitch(src_a_pitch);
				colcmd_col_enable(opx, false, 0);
				active++;
				break;
			case WIPE_OP_START_B:
				colcmd_col_src_ptr(src_b_ptr + opx);
				colcmd_col_src_pitch(src_b_pitch);
				colcmd_col_enable(opx, false, 0);
				active++;
				break;
			case WIPE_OP_STOP_A:
			case WIPE_OP_STOP_B:
				colcmd_col_disable(opx);
				assert(active != 0);
				active--;
				break;
		}
	}
	assert(active == 0);
}

static void cmd_wipe(uint32_t cmd_header) {
	STAT_BUMP[UHARDDOOM_STAT_FW_WIPE] = 1;
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
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
	colcmd_col_ustart(0);
	colcmd_col_ustep(0x10000);
	swrcmd_dst_pitch(dst_pitch);
	colsem();
	while (w--) {
		STAT_BUMP[UHARDDOOM_STAT_FW_WIPE_COL] = 1;
		uint32_t yoff = *CMD_FETCH & UHARDDOOM_COORD_MASK;
		if (yoff > h)
			yoff = h;
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

#define DC_MEM_SIZE 0x200

static uint32_t dc_mem_idx = 0;
static uint32_t dc_mem_wr0[DC_MEM_SIZE];
static uint32_t dc_mem_tex_ptr[DC_MEM_SIZE];
static uint32_t dc_mem_ustart[DC_MEM_SIZE];
static uint32_t dc_mem_ustep[DC_MEM_SIZE];
static uint32_t dc_mem_cmap_b_ptr[DC_MEM_SIZE];

enum {
	/* Stop operations must be processed before start operations for given Y.  */
	DC_OP_STOP = 0,
	DC_OP_START = 1,
};

#define DC_OP(op, idx, y) ((y) << 12 | (op) << 10 | (idx))

static void draw_columns_flush(uint32_t dst_ptr, uint32_t dst_pitch, bool cmap_a_en, bool cmap_b_en, bool trans_en) {
	if (heap_empty())
		return;
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_COLUMNS_BATCH] = 1;
	dst_ptr += UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_X(dc_mem_wr0[0]) & ~UHARDDOOM_BLOCK_MASK;
	uint32_t active = 0;
	uint32_t ylast = 0;
	while (!heap_empty()) {
		uint32_t opw = heap_get();
		uint32_t opy = opw >> 12;
		uint32_t op = opw >> 10 & 1;
		uint32_t opi = opw & 0x1ff;
		if (opy != ylast && active) {
			uint32_t num = opy - ylast;
			colcmd_draw(num, cmap_a_en);
			swrcmd_dst_ptr(dst_ptr + ylast * dst_pitch);
			swrcmd_draw_col(num, trans_en);
			STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_COLUMNS_SEG] = 1;
		}
		ylast = opy;
		uint32_t x = UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_X(dc_mem_wr0[opi]) & UHARDDOOM_BLOCK_MASK;
		if (op == DC_OP_START) {
			uint32_t height = UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_SRC_HEIGHT(dc_mem_wr0[opi]);
			colcmd_col_src_ptr(dc_mem_tex_ptr[opi]);
			colcmd_col_ustart(dc_mem_ustart[opi]);
			colcmd_col_ustep(dc_mem_ustep[opi]);
			colcmd_col_cmap_b_ptr(dc_mem_cmap_b_ptr[opi]);
			colcmd_col_enable(x, cmap_b_en, height);
			active++;
		} else {
			colcmd_col_disable(x);
			assert(active != 0);
			active--;
		}
	}
	assert(active == 0);
	dc_mem_idx = 0;
}

static void cmd_draw_columns(uint32_t cmd_header) {
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_COLUMNS] = 1;
	bool cmap_a_en = UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_CMAP_A_EN(cmd_header);
	bool cmap_b_en = UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_CMAP_B_EN(cmd_header);
	bool trans_en = UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_TRANS_EN(cmd_header);
	uint32_t num = UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_NUM_COLS(cmd_header);
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
	swrcmd_dst_pitch(dst_pitch);
	if (cmap_a_en) {
		uint32_t cmap_a_ptr = *CMD_FETCH;
		if (cmap_a_ptr & UHARDDOOM_BLOCK_MASK)
			error(UHARDDOOM_FE_ERROR_CODE_COLORMAP_UNALIGNED, cmd_ptr, cmap_a_ptr);
		srdcmd_src_ptr(cmap_a_ptr);
		srdcmd_src_pitch(UHARDDOOM_BLOCK_SIZE);
		srdcmd_read_col(4);
		colcmd_load_cmap_a();
	}
	if (trans_en) {
		swrcmd_transmap_ptr(*CMD_FETCH);
	}
	uint32_t xlast = 0;
	uint32_t xl = 0, yl = 0;
	colcmd_col_src_pitch(1);
	while (num--) {
		STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_COLUMNS_COL] = 1;
		uint32_t wr0 = *CMD_FETCH;
		uint32_t x = UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_X(wr0);
		if ((x & ~UHARDDOOM_BLOCK_MASK) != xlast) {
			draw_columns_flush(dst_ptr, dst_pitch, cmap_a_en, cmap_b_en, trans_en);
			xlast = x & ~UHARDDOOM_BLOCK_MASK;
		}
		uint32_t wr1 = *CMD_FETCH;
		uint32_t y0 = UHARDDOOM_USER_DRAW_COLUMNS_WR1_EXTR_Y0(wr1);
		uint32_t y1 = UHARDDOOM_USER_DRAW_COLUMNS_WR1_EXTR_Y1(wr1);
		if (y0 > y1)
			error(UHARDDOOM_FE_ERROR_CODE_DRAW_COLUMNS_Y_REV, cmd_ptr, wr1);

		if (x < xl || (x == xl && y0 < yl))
			draw_columns_flush(dst_ptr, dst_pitch, cmap_a_en, cmap_b_en, trans_en);

		uint32_t idx = dc_mem_idx++;
		dc_mem_wr0[idx] = wr0;
		dc_mem_tex_ptr[idx] = *CMD_FETCH;
		dc_mem_ustart[idx] = *CMD_FETCH;
		dc_mem_ustep[idx] = *CMD_FETCH;
		if (cmap_b_en)
			dc_mem_cmap_b_ptr[idx] = *CMD_FETCH;

		heap_put(DC_OP(DC_OP_START, idx, y0));
		heap_put(DC_OP(DC_OP_STOP, idx, y1 + 1));

		xl = x;
		yl = y1 + 1;

		if (dc_mem_idx == DC_MEM_SIZE)
			draw_columns_flush(dst_ptr, dst_pitch, cmap_a_en, cmap_b_en, trans_en);
	}
	draw_columns_flush(dst_ptr, dst_pitch, cmap_a_en, cmap_b_en, trans_en);
}

static void draw_fuzz_flush(uint32_t dst_ptr, uint32_t dst_pitch, uint32_t fuzzstart, uint32_t fuzzend) {
	if (heap_empty())
		return;
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_FUZZ_BATCH] = 1;
	dst_ptr += UHARDDOOM_USER_DRAW_COLUMNS_WR0_EXTR_X(dc_mem_wr0[0]) & ~UHARDDOOM_BLOCK_MASK;
	uint32_t active = 0;
	uint32_t ylast = 0;
	srdsem();
	while (!heap_empty()) {
		uint32_t opw = heap_get();
		uint32_t opy = opw >> 12;
		uint32_t op = opw >> 10 & 1;
		uint32_t opi = opw & 0x1ff;
		if (opy != ylast && active) {
			uint32_t num = opy - ylast;
			srdcmd_read_fx(num);
			fxcmd_draw_fuzz(num);
			swrcmd_draw_fx(num, false);
			STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_FUZZ_SEG] = 1;
		}
		ylast = opy;
		uint32_t x = UHARDDOOM_USER_DRAW_FUZZ_WR0_EXTR_X(dc_mem_wr0[opi]) & UHARDDOOM_BLOCK_MASK;
		if (op == DC_OP_START) {
			if (!active) {
				if (opy == fuzzstart) {
					srdcmd_src_ptr(dst_ptr + opy * dst_pitch);
					srdcmd_read_fx(1);
					srdcmd_src_ptr(dst_ptr + opy * dst_pitch);
				} else {
					srdcmd_src_ptr(dst_ptr + (opy - 1) * dst_pitch);
					srdcmd_read_fx(1);
				}
				fxcmd_load_fuzz();
				swrcmd_dst_ptr(dst_ptr + opy * dst_pitch);
			}
			active++;
			uint32_t fuzzpos = UHARDDOOM_USER_DRAW_FUZZ_WR0_EXTR_FUZZPOS(dc_mem_wr0[opi]);
			fxcmd_col_enable(x, fuzzpos);
		} else {
			fxcmd_col_disable(x);
			assert(active != 0);
			active--;
			if (!active) {
				if (opy == fuzzend + 1)
					srdcmd_src_ptr(dst_ptr + fuzzend * dst_pitch);
				srdcmd_read_fx(1);
			}
		}
	}
	assert(active == 0);
	dc_mem_idx = 0;
}

static void cmd_draw_fuzz(uint32_t cmd_header) {
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_FUZZ] = 1;
	uint32_t num = UHARDDOOM_USER_DRAW_FUZZ_HEADER_EXTR_NUM_COLS(cmd_header);
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
	swrcmd_dst_pitch(dst_pitch);
	uint32_t w3 = *CMD_FETCH;
	uint32_t fuzzstart = UHARDDOOM_USER_DRAW_FUZZ_W3_EXTR_FUZZSTART(w3);
	uint32_t fuzzend = UHARDDOOM_USER_DRAW_FUZZ_W3_EXTR_FUZZEND(w3);
	uint32_t cmap_ptr = *CMD_FETCH;
	if (cmap_ptr & UHARDDOOM_BLOCK_MASK)
		error(UHARDDOOM_FE_ERROR_CODE_COLORMAP_UNALIGNED, cmd_ptr, cmap_ptr);
	srdcmd_src_ptr(cmap_ptr);
	srdcmd_src_pitch(UHARDDOOM_BLOCK_SIZE);
	srdcmd_read_fx(4);
	fxcmd_load_cmap();
	fxcmd_skip(0, 0, false);
	srdcmd_src_pitch(dst_pitch);

	uint32_t xlast = 0;
	uint32_t xl = 0, yl = 0;
	while (num--) {
		STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_FUZZ_COL] = 1;
		uint32_t wr0 = *CMD_FETCH;
		uint32_t x = UHARDDOOM_USER_DRAW_FUZZ_WR0_EXTR_X(wr0);
		if ((x & ~UHARDDOOM_BLOCK_MASK) != xlast) {
			draw_fuzz_flush(dst_ptr, dst_pitch, fuzzstart, fuzzend);
			xlast = x & ~UHARDDOOM_BLOCK_MASK;
		}
		uint32_t wr1 = *CMD_FETCH;
		uint32_t y0 = UHARDDOOM_USER_DRAW_FUZZ_WR1_EXTR_Y0(wr1);
		uint32_t y1 = UHARDDOOM_USER_DRAW_FUZZ_WR1_EXTR_Y1(wr1);
		if (y0 > y1)
			error(UHARDDOOM_FE_ERROR_CODE_DRAW_COLUMNS_Y_REV, cmd_ptr, wr1);

		if (x < xl || (x == xl && y0 < yl))
			draw_fuzz_flush(dst_ptr, dst_pitch, fuzzstart, fuzzend);

		uint32_t idx = dc_mem_idx++;
		dc_mem_wr0[idx] = wr0;

		heap_put(DC_OP(DC_OP_START, idx, y0));
		heap_put(DC_OP(DC_OP_STOP, idx, y1 + 1));

		xl = x;
		yl = y1 + 1;

		if (dc_mem_idx == DC_MEM_SIZE)
			draw_fuzz_flush(dst_ptr, dst_pitch, fuzzstart, fuzzend);
	}
	draw_fuzz_flush(dst_ptr, dst_pitch, fuzzstart, fuzzend);
}

static void cmd_draw_spans(uint32_t cmd_header) {
	STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_SPANS] = 1;
	bool cmap_en = UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_CMAP_EN(cmd_header);
	bool trans_en = UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_TRANS_EN(cmd_header);
	uint32_t ulog = UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_ULOG(cmd_header);
	uint32_t vlog = UHARDDOOM_USER_DRAW_SPANS_HEADER_EXTR_VLOG(cmd_header);
	spancmd_uvmask(ulog, vlog);
	uint32_t dst_ptr = *CMD_FETCH;
	uint32_t dst_pitch = *CMD_FETCH;
	uint32_t w3 = *CMD_FETCH;
	uint32_t y0 = UHARDDOOM_USER_DRAW_SPANS_W3_EXTR_Y0(w3);
	uint32_t y1 = UHARDDOOM_USER_DRAW_SPANS_W3_EXTR_Y1(w3);
	spancmd_src_ptr(*CMD_FETCH);
	spancmd_src_pitch(*CMD_FETCH);
	if (trans_en)
		swrcmd_transmap_ptr(*CMD_FETCH);
	dst_ptr += y0 * dst_pitch;
	uint32_t num;
	if (y1 > y0) {
		num = y1 - y0 + 1;
	} else {
		num = y0 - y1 + 1;
		dst_pitch = -dst_pitch;
	}
	swrcmd_dst_pitch(UHARDDOOM_BLOCK_SIZE);
	if (cmap_en)
		srdcmd_src_pitch(UHARDDOOM_BLOCK_SIZE);
	/* 1 is unaligned and thus invalid.  */
	uint32_t last_cmap_ptr = 1;
	while (num--) {
		STAT_BUMP[UHARDDOOM_STAT_FW_DRAW_SPANS_SPAN] = 1;
		uint32_t wr0 = *CMD_FETCH;
		uint32_t x0 = UHARDDOOM_USER_DRAW_SPANS_WR0_EXTR_X0(wr0);
		uint32_t x1 = UHARDDOOM_USER_DRAW_SPANS_WR0_EXTR_X1(wr0);
		if (x1 < x0)
			error(UHARDDOOM_FE_ERROR_CODE_DRAW_SPANS_X_REV, cmd_ptr, wr0);
		spancmd_ustart(*CMD_FETCH);
		spancmd_vstart(*CMD_FETCH);
		spancmd_ustep(*CMD_FETCH);
		spancmd_vstep(*CMD_FETCH);
		if (cmap_en) {
			uint32_t cmap_ptr = *CMD_FETCH;
			if (cmap_ptr != last_cmap_ptr) {
				if (cmap_ptr & UHARDDOOM_BLOCK_MASK)
					error(UHARDDOOM_FE_ERROR_CODE_COLORMAP_UNALIGNED, cmd_ptr, cmap_ptr);
				srdcmd_src_ptr(cmap_ptr);
				srdcmd_read_fx(4);
				fxcmd_load_cmap();
				last_cmap_ptr = cmap_ptr;
			}
		}
		uint32_t off = x0 & ~UHARDDOOM_BLOCK_MASK;
		x0 -= off;
		x1 -= off;
		uint32_t skip_end = ~x1 & UHARDDOOM_BLOCK_MASK;
		uint32_t blocks = (x1 + 1 + skip_end) >> UHARDDOOM_BLOCK_SHIFT;
		spancmd_draw(x1 - x0 + 1, x0);
		fxcmd_skip(x0, skip_end, false);
		fxcmd_draw_span(blocks, cmap_en);
		swrcmd_dst_ptr(dst_ptr + off);
		swrcmd_draw_fx(blocks, false);
		dst_ptr += dst_pitch;
	}
}

noreturn void main() {
	while (1) {
		*JOB_WAIT;
		STAT_BUMP[UHARDDOOM_STAT_FW_JOB] = 1;
		while (!*CMD_END) {
			cmd_ptr = *CMD_PTR;
			uint32_t cmd_header = *CMD_FETCH;
			STAT_BUMP[UHARDDOOM_STAT_FW_CMD] = 1;
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
		srdsem();
		fesem();
		*JOB_DONE = 0;
	}
}
