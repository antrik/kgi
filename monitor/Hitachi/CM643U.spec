Monitor(Hitachi, CM643U, Hitachi_CM643U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan 643 (CM642U-511)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
	Size(325, 245)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 150000000)
	hFreq(0, 31000, 95000)
	vFreq(0, 50, 130)
End
#endif
