Board(IBM, VGA, Standard_VGA)
#if Data
Begin
	SubsystemID(0x0,0x0300)
	SubsystemID(0x0,0x0001)
	SubsystemID(0x15ad,0x0710)
	Vendor("Industrial Business Machines Inc.")
	Model("VGA")
	Driver(chipset,	IBM/VGA, vga_chipset)
	Driver(ramdac,	IBM/VGA, vga_ramdac)
	Driver(clock,	IBM/VGA, fixed_clock)
	Driver(monitor, crt/crt, monosync_monitor)
End
#endif
