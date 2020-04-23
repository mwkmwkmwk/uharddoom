#include "uharddoom.h"

#include <stdint.h>
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

static uint32_t cmd_ptr;

static void cmd_fill_rect(uint32_t cmd_header) {
	/* XXX */
}

static void cmd_draw_line(uint32_t cmd_header) {
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
					*FE_ERROR_DATA_A = cmd_ptr;
					*FE_ERROR_DATA_B = cmd_header;
					*FE_ERROR_CODE = UHARDDOOM_FE_ERROR_CODE_UNK_USER_COMMAND;
			}
		}
		*JOB_DONE = 0;
	}
}
