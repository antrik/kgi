Monitor(Hitachi, CM640U, Hitachi_CM640U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan 640 (CM640U-511)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1024, 768)
	Size(325, 245)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 110000000)
	hFreq(0, 31000, 69000)
	vFreq(0, 50, 130)
End
#endif
