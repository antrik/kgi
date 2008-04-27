Monitor(Hitachi, CM752U, Hitachi_CM752U)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan 752 (CMU752U)")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
	Size(367, 276)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 211000000)
	hFreq(0, 31000, 101000)
	vFreq(0, 50, 160)
End
#endif
