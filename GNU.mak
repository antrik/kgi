CFLAGS:=-Wall -O2 -D__KERNEL__ -D__MODULE__ \
    -include core/include/kgi/config.h -Icore/include -Iinclude -I. \
    -fno-strict-aliasing \
    $(CFLAGS)

MODS+=system/$(SYSTEM)/kgim-0.9-mod.o
OBJS+=system/$(SYSTEM)/kgim-0.9.o system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o

system/$(SYSTEM)/kgim-0.9-mod.o: system/$(SYSTEM)/kgim-0.9.o system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o

system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o: system/$(SYSTEM)/hurd_video.h

MODS+=core/core-mod.o
OBJS+=core/core.o

AWK?=awk

%-mod.o: %.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-board.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-monitor.o
	$(LD) -r $(LDFLAGS) -o $@ $^
