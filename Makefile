TOOLCHAIN=/opt/riscv64-unknown-elf-gcc-8.2.0-2019.05.3-x86_64-linux-ubuntu14/bin/riscv64-unknown-elf-

build/bootloader: bootloader.c _start.s link.ld
	$(TOOLCHAIN)gcc -O0 -march=rv32i -mabi=ilp32 -static -mcmodel=medany -fvisibility=hidden -fno-builtin -nostdlib -nostartfiles -o $@ -T link.ld _start.s $<

dasm: build/bootloader
	$(TOOLCHAIN)objdump -d $<

GDB_UPLOAD_ARGS ?= --batch
GDB_UPLOAD_CMDS += -ex "set remotetimeout 240"
GDB_UPLOAD_CMDS += -ex "target extended-remote localhost:3333"
GDB_UPLOAD_CMDS += -ex "monitor reset halt"
GDB_UPLOAD_CMDS += -ex "monitor flash protect 0 64 last off"
GDB_UPLOAD_CMDS += -ex "load"

upload: build/bootloader
	$(TOOLCHAIN)gdb $< $(GDB_UPLOAD_ARGS) $(GDB_UPLOAD_CMDS) -ex "monitor resume" -ex "quit"

debug: build/bootloader
	$(TOOLCHAIN)gdb $< -tui $(GDB_UPLOAD_CMDS)

clean:
	rm build/*
