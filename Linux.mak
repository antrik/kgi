KERNEL_PATH?=/lib/modules/$(shell uname -r)/build

CFLAGS=-Wall -fomit-frame-pointer -O2 -D__KERNEL__ -D__MODULE__ -ffreestanding \
    -include ${KERNEL_PATH}/include/kgi/config.h -I${KERNEL_PATH}/include -Iinclude -I. \
    -fno-strict-aliasing

LDFLAGS=-melf_i386 -r --strip-debug -O2 

SYSTEM=Linux

MODS+=system/$(SYSTEM)/kgim-0.9.ko
OBJS+=system/$(SYSTEM)/kgim-0.9.o

AWK?=awk

%.ko: %.o
	$(LD) $(LDFLAGS) -o $@ $^

%.ko: %-board.o
	$(LD) $(LDFLAGS) -o $@ $^

%.ko: %-monitor.o
	$(LD) $(LDFLAGS) -o $@ $^
