Monitor(HP, 98789A, HP_98789A)
#if Data
Begin
	Contributor("Brian S. Julin <bri@tull.umassp.edu>")
	Vendor("Hewlett-Packard")
	Model("98789A")
	MaxRes(1280, 1024)
	Size(380, 280)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_ON_GREEN)
	/* 108181000 +- 5% tolerance */
	Bandwidth(10277195, 11359005)
	/* 63.34KHz, 60Hz +- 5% tolerance */
	hFreq(0, 60173, 66507)
	vFreq(0, 57.000, 63.000)
End
#endif

