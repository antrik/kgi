KERNEL_PATH?=/src/kgi-include/

CFLAGS:=-Wall -O2 -D__KERNEL__ \
    -include ${KERNEL_PATH}/include/kgi/config.h -I${KERNEL_PATH}/include -Iinclude -I. \
    -fno-strict-aliasing \
    $(CFLAGS)

SYSTEM=GNU

MODS+=system/$(SYSTEM)/kgim-0.9.o

AWK?=awk
