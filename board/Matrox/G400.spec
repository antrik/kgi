Board(Matrox, G400, Standard_SVGA)
#if Data
Begin
	SubsystemID(0x102B, 0x0525)
	Vendor("Matrox Graphics Inc")
	Model("G400")
	Driver(chipset,	Matrox/Gx00, mgag_chipset)
	Driver(ramdac,	Matrox/Gx00, mgag_ramdac)
	Driver(clock,	Matrox/Gx00, mgag_clock)
	Driver(monitor, crt/crt, crt_monitor)
End
#endif
