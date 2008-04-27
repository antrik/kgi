Monitor(Hitachi, CM813U, Hitachi_CM813U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan 813 (CM813U)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1856, 1392)
	Size(406, 305)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 250000000)
	hFreq(0, 31000, 115000)
	vFreq(0, 50, 160)
End
#endif
