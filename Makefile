all: udoomfw.elf udoomfw.bin

udoomfw.elf: udoomfw.c udoomfw.lds
	riscv64-linux-gnu-gcc -O3 -ffreestanding -nostdlib udoomfw.c -o udoomfw.elf -march=rv32im -mabi=ilp32 -Wl,-T,udoomfw.lds,--build-id=none

udoomfw.bin: udoomfw.elf
	riscv64-linux-gnu-objcopy udoomfw.elf -O binary udoomfw.bin
