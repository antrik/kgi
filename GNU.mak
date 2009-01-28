CFLAGS:=-Wall -O2 -D__KERNEL__ -D__MODULE__ \
    -include core/include/kgi/config.h -Icore/include -Iinclude -I. \
    -fno-strict-aliasing \
    -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE \
    $(CFLAGS)

MODS+=system/$(SYSTEM)/kgim-0.9-mod.o
OBJS+=system/$(SYSTEM)/kgim-0.9.o system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o

system/$(SYSTEM)/kgim-0.9-mod.o: system/$(SYSTEM)/kgim-0.9.o system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o

system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o: system/$(SYSTEM)/hurd_video.h

MODS+=core/core-mod.o
OBJS+=core/core.o core/kgiServer.o
CLEANFILES+=core/kgiServer.h core/kgiUser.h core/kgiServer.c core/kgiUser.c

core/core-mod.o: core/core.o core/kgiServer.o

core/core.o: core/kgiServer.h

%Server.h %User.h %Server.c %User.c: %.defs %mutations.h
	mig -imacros $*mutations.h -sheader $*Server.h -header $*User.h -server $*Server.c -user $*User.c $<

OBJS+=rpctest.o
CLEANFILES+=rpctest

rpctest: core/rpctest.o core/kgiUser.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

core/rpctest.o: core/kgiUser.h

AWK?=awk

%-mod.o: %.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-board.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-monitor.o
	$(LD) -r $(LDFLAGS) -o $@ $^
