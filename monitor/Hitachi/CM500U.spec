Monitor(Hitachi, CM500U, Hitachi_CM500U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan Pro 500 (CM500U)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1280, 1024)
	Size(280, 210)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 85000000)
	hFreq(0, 30000, 69000)
	vFreq(0, 50, 100)
End
#endif
