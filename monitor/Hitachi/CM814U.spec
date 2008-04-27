Monitor(Hitachi, CM814U, Hitachi_CM814U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan 814 (CM814U)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1856, 1392)
	Size(406, 305)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 270000000)
	hFreq(0, 31000, 125000)
	vFreq(0, 50, 160)
End
#endif
