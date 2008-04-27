Board(ATI, Rage, Standard_SVGA)
#if Data
Begin
	SubsystemID(0x1002, 0x4354)
	SubsystemID(0x1002, 0x4754)
	SubsystemID(0x1002, 0x4755)
	SubsystemID(0x1002, 0x4756)
	SubsystemID(0x1002, 0x4757)
	SubsystemID(0x1002, 0x4758)
	SubsystemID(0x1002, 0x4C4D)
	Vendor("ATI")
	Model("Rage")
	Driver(chipset,	ATI/MACH64, mach64_chipset)
	Driver(ramdac,	ATI/MACH64, mach64_ramdac)
	Driver(clock,	ATI/MACH64, mach64_clock)
	Driver(monitor, crt/crt, crt_monitor)
End
#endif
