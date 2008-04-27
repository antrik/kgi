KERNEL_PATH?=	/usr/src/sys

KMODDIR?=	/boot/kernel
KMODOWN?=	root
KMODGRP?=	wheel
KMODMODE?=	555

SYSTEM=FreeBSD
ARCH=i386

OBJS+=machine @ device_if.h bus_if.h pci_if.h
MODS+=system/${SYSTEM}/kgim-0.9.ko

CLEANFILES=

CFLAGS=	${COPTS} -D_KERNEL -DKLD_MODULE

CFLAGS+= -Wall -finline-limit=15000 -fno-common -g -mno-align-long-strings \
	 -mpreferred-stack-boundary=2 -ffreestanding -std=c99 -Wstrict-prototypes \
	 -Wmissing-prototypes -O -pipe -mcpu=pentiumpro

CFLAGS+= -I. -I${KERNEL_PATH} -Iinclude -I${KERNEL_PATH}/dev

CFLAGS+= ${DEBUG_FLAGS}

LDFLAGS+= -r -d -warn-common

AWK?=awk
INSTALL?=install

machine:
	ln -s ${KERNEL_PATH}/${ARCH}/include machine

@:
	ln -s ${KERNEL_PATH} @

device_if.h: @/kern/device_if.m
	${AWK} -f @/tools/makeobjops.awk $^ -h
bus_if.h: @/kern/bus_if.m
	${AWK} -f @/tools/makeobjops.awk $^ -h
pci_if.h: @/dev/pci/pci_if.m
	${AWK} -f @/tools/makeobjops.awk $^ -h

%.ko: %.o
	$(LD) -Bshareable -d -warn-common -o $@ $^

%.ko: %-board.o
	$(LD) -Bshareable -d -warn-common -o $@ $^

%.ko: %-monitor.o
	$(LD) -Bshareable -d -warn-common -o $@ $^

install: everything
	$(INSTALL) -o ${KMODOWN} -g ${KMODGRP} -m ${KMODMODE} ${MODS} ${KMODDIR}
