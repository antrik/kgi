CFLAGS:=-Wall -O2 -D__KERNEL__ -D__MODULE__ \
    -include core/include/kgi/config.h -Icore/include -Iinclude -I. \
    -fno-strict-aliasing \
    -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE \
    $(CFLAGS)

kgi_h_includes=core/include/kgi/kgi.h core/include/kgi/system.h core/include/kgi/errno.h core/include/kgi/debug.h core/include/kgi/cmd.h core/include/kgi/compiler.h core/include/kgi/i386-types.h core/include/kgi/i386-compiler.h

MODS+=system/$(SYSTEM)/kgim-0.9-mod.o
OBJS+=system/$(SYSTEM)/kgim-0.9.o system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o

system/$(SYSTEM)/kgim-0.9-mod.o: system/$(SYSTEM)/kgim-0.9.o system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o

system/$(SYSTEM)/system.o system/$(SYSTEM)/hurd_video.o: system/$(SYSTEM)/hurd_video.h

MODS+=core/core-mod.o
OBJS+=core/core.o core/kgiServer.o
CLEANFILES+=core/kgiServer.h core/kgiUser.h core/kgiServer.c core/kgiUser.c

core/core-mod.o: core/core.o core/kgiServer.o

core/core.o: core/kgiServer.h $(kgi_h_includes) include/kgi/module.h include/kgi/modulelib.h

%Server.h %User.h %Server.c %User.c: %.defs %mutations.h
	mig -imacros $*mutations.h -sheader $*Server.h -header $*User.h -server $*Server.c -user $*User.c $<

OBJS+=rpctest.o
CLEANFILES+=rpctest

rpctest: core/rpctest.o core/kgiUser.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

core/rpctest.o: core/kgiUser.h $(kgi_h_includes)

core/kgiServer.o core/kgiUser.o: $(kgi_h_includes)

AWK?=awk

%-mod.o: %.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-board.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-mod.o: %-monitor.o
	$(LD) -r $(LDFLAGS) -o $@ $^
