Monitor(Hitachi, CM812U, Hitachi_CM812U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan 812 (CM812U)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
	Size(406, 305)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 230000000)
	hFreq(0, 31000, 107000)
	vFreq(0, 50, 160)
End
#endif
