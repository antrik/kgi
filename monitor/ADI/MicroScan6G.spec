Monitor(ADI, MicroScan6G, ADI_MicroScan6G)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("ADI Corporation")
	Model("MicroScan 6G")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
	Size(404, 303)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 202500000)
	hFreq(0, 30000, 95000)
	vFreq(0, 50, 160)
End
#endif
