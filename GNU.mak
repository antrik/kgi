CFLAGS:=-Wall -O2 -D__KERNEL__ -D__MODULE__ \
    -include core/include/kgi/config.h -Icore/include -Iinclude -I. \
    -fno-strict-aliasing \
    $(CFLAGS)

MODS+=system/$(SYSTEM)/kgim-0.9-mod.o
OBJS+=system/$(SYSTEM)/kgim-0.9.o

AWK?=awk

%-mod.o: %.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-board.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-monitor.o
	$(LD) -r $(LDFLAGS) -o $@ $^
