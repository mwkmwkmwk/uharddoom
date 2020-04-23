#include "uharddoom.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>

__asm__(
	".section start, \"ax\"\n"
	"_start:\n"
	"lui	sp, 0xc0010\n"
	"lui	a0,%hi(_data_image_start)\n"
	"addi	a0,a0,%lo(_data_image_start)\n"
	"lui	a1,%hi(_data_start)\n"
	"addi	a1,a1,%lo(_data_start)\n"
	"lui	a2,%hi(_data_end)\n"
	"addi	a2,a2,%lo(_data_end)\n"
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
	SWRCMD[UHARDDOOM_SWRCMD_TYPE_DST_PITCH] = dst_pitch;
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

static void cmd_draw_line(uint32_t cmd_header) {
	FXCMD[UHARDDOOM_FXCMD_TYPE_FILL_COLOR] = UHARDDOOM_USER_DRAW_LINE_HEADER_EXTR_COLOR(cmd_header);
	/* XXX */
}

static void cmd_blit(uint32_t cmd_header) {
	/* XXX */
}

static void cmd_wipe(uint32_t cmd_header) {
	/* XXX */
}

static void cmd_draw_columns(uint32_t cmd_header) {
	/* XXX */
}

static void cmd_draw_fuzz(uint32_t cmd_header) {
	/* XXX */
}

static void cmd_draw_spans(uint32_t cmd_header) {
	/* XXX */
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
