Board(Creative_Labs, Graphics_Blaster_Riva_TNT2_Ultra, Standard_VANDA)
#if Data
Begin
	SubsystemID(0x10de, 0x0028)
	Vendor("Creative Labs, Inc.")
	Model("Graphics Blaster Riva TNT2 Ultra")
	Driver(chipset,	nVidia/TNT2, tnt2_chipset)
	Driver(ramdac,	nVidia/TNT2, tnt2_ramdac)
	Driver(clock,	nVidia/TNT2, tnt2_clock)
	Driver(monitor, monosync/monosync, monosync_monitor)
End
#endif
