.PHONY: all everything

all: 1x64sg-VANDA

1x64sg-VANDA: board/Matrox/1x64sg-mod.o monitor/Standard/VANDA-mod.o system/$(SYSTEM)/kgim-0.9-mod.o
	$(CC) $(CFLAGS) $(LDFLAGS) -lpci -o $@ $^

clean:
	rm -f ${OBJS} ${MODS} ${CLEANFILES}

distclean: clean
	rm -f Makefile

%.o: %-bind.o %-meta.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%.o: %-fixed.o clock/fixed-meta.o
	$(LD) -r $(LDFLAGS) -o $@ $^

%-monitor.o:
	$(LD) -r $(LDFLAGS) -o $@ $^

clock/%-fixed.o: clock/%.spec
	$(CC) $(CFLAGS) -c -o $@ -DFIXED_CLOCK_SPEC=\"$<\" clock/fixed-bind.c

board/%-board.o: board/%.spec
	$(CC) $(CFLAGS) -c -o $@ -DBOARD_SPEC=\"$<\" system/${SYSTEM}/board-bind.c

monitor/%-monosync.o: monitor/%.spec
	$(CC) $(CFLAGS) -c -o $@ -DMONITOR_SPEC=\"$<\" system/${SYSTEM}/monitor-monosync.c

monitor/%-crt.o: monitor/%.spec
	$(CC) $(CFLAGS) -c -o $@ -DMONITOR_SPEC=\"$<\" system/${SYSTEM}/monitor-crt.c

system/$(SYSTEM)/%.o: system/$(SYSTEM)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

monitor/crt/VESA-timings.c: monitor/crt/VESA.t
	$(AWK) -v timingset=VESA -f monitor/crt/vesa2c.awk $^ > $@

monitor/crt/VGA-timings.c: monitor/crt/VGA.t
	$(AWK) -v timingset=VGA -f monitor/crt/vga2c.awk $^ > $@

monitor/monosync/SVGA-timings.c: monitor/monosync/SVGA.t
	$(AWK) -v timingset=SVGA -f monitor/monosync/t2c.awk $^ > $@

monitor/monosync/VGA-timings.c: monitor/monosync/VGA.t
	$(AWK) -v timingset=VGA -f monitor/monosync/t2c.awk $^ > $@
