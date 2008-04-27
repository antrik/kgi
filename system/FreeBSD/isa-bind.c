/* ----------------------------------------------------------------------------
**	KGI-0.9 isa binding driver
** ----------------------------------------------------------------------------
**	Copyright (c)	2003	Nicholas Souchu
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$FreeBSD$
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Nicholas_Souchu

#define KGI_DBG_LEVEL	4

#include <kgi/module.h>
#include <kgi/modulelib.h>
#include <kgi/modulecmd.h>

#include "system/FreeBSD/kgim_version.h"

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include "chipset/IBM/VGA.h"

#include <isa/isareg.h>
#include <isa/isavar.h>

#define DRIVER_NAME "kgisa"

static devclass_t	isavga_devclass;

typedef struct {
	int iorid;
	int memrid;
	struct resource *iores;
	struct resource *memres;
	mem_region_t text16fb;
} isavga_softc_t;

static isavga_softc_t *isavga_sc = NULL;

static void
isavga_identify(driver_t *driver, device_t parent)
{
	BUS_ADD_CHILD(parent, ISA_ORDER_SPECULATIVE, DRIVER_NAME, 0);
}

static int
isavga_probe(device_t dev)
{
	int error;

	/* No pnp support */
	if (isa_get_vendorid(dev))
		return (ENXIO);

	device_set_desc(dev, "KGI ISA VGA");

	//bus_set_resource(dev, SYS_RES_IOPORT, 0, adp.va_io_base, adp.va_io_size);
	error = bus_set_resource(dev, SYS_RES_MEMORY, 0,
				 VGA_TEXT_MEM_BASE, VGA_TEXT_MEM_SIZE / 2);

	return error;
}

static int
isavga_attach(device_t dev)
{
	isavga_softc_t *sc = device_get_softc(dev);

	//sc->iorid = 0;
	//sc->iores = bus_alloc_resource(dev, SYS_RES_IOPORT, &sc->iorid,
	//			       0, ~0, 0, RF_ACTIVE | RF_SHAREABLE);
	sc->memrid = 0;
	sc->memres = bus_alloc_resource(dev, SYS_RES_MEMORY, &sc->memrid,
					0, ~0, 0, RF_ACTIVE | RF_SHAREABLE);

	sc->text16fb.name      = "Matrox VGA text16 aperture";
	sc->text16fb.device    = PCICFG_NULL;
	sc->text16fb.base_virt = MEM_NULL;
	sc->text16fb.base_io   = VGA_TEXT_MEM_BASE;
	sc->text16fb.size      = VGA_TEXT_MEM_SIZE / 2;
	sc->text16fb.decode    = MEM_DECODE_ALL;
	sc->text16fb.rid       = sc->memrid;

	/* Having a unique isa device is what we want */
	isavga_sc = sc;

	return 0;
}

static int
isavga_detach(device_t dev)
{
	isavga_softc_t *sc = device_get_softc(dev);

	//bus_release_resource(dev, SYS_RES_IOPORT, sc->iorid, sc->iores);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->memrid, sc->memres);

	return 0;
}

static device_method_t isavga_methods[] = {
	DEVMETHOD(device_identify,	isavga_identify),
	DEVMETHOD(device_probe,		isavga_probe),
	DEVMETHOD(device_attach,	isavga_attach),

	DEVMETHOD(bus_print_child,	bus_generic_print_child),
	{ 0, 0 }
};

static driver_t isavga_driver = {
	DRIVER_NAME,
	isavga_methods,
	sizeof(isavga_softc_t),
};

DRIVER_MODULE(kgisa, isa, isavga_driver, isavga_devclass, 0, 0);
