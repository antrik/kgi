Board(Matrox, 1x64sg, Standard_VGA)
#if Data
Begin
	SubsystemID(0x102B, 0x051A)
	Vendor("Matrox Graphics Inc")
	Model("1x64sg - Mystique")
	Driver(chipset,	Matrox/Gx00, mgag_chipset)
	Driver(ramdac,	Matrox/Gx00, mgag_ramdac)
	Driver(clock,	Matrox/Gx00, mgag_clock)
	Driver(monitor, crt/crt, crt_monitor)
End
#endif
