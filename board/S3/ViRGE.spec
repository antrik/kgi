Board(S3, ViRGE)
#if Data
Begin
	Vendor("S3")
	Model("ViRGE")
	Driver(chipset,	S3/ViRGE, virge_chipset)
	Driver(ramdac,	S3/ViRGE, virge_ramdac)
	Driver(clock,	S3/ViRGE, virge_clock)
End
#endif
