Board(3Dlabs, Permedia2v)
#if Data
Begin
	SubsystemID(0x3D3D, 0x0100)
	Vendor("3Dlabs Incorporated")
	Model("Permedia2v Reference Board")
	Driver(chipset,	3Dlabs/PERMEDIA,  pgc_chipset)
	Driver(ramdac,	3Dlabs/PERMEDIA2v, pgc2v_ramdac)
	Driver(clock,	3Dlabs/PERMEDIA2v, pgc2v_clock)
End
#endif
