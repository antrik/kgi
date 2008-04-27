Monitor(Hitachi, CMDT3140, Hitachi_CMDT3140)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi Displays")
	Model("SuperScan LC140 (CMDT3140)")
	Flags(KGIM_MF_POWERSAVE | KGIM_MF_PROPSIZE)
	MaxRes(1024, 768)
	Size(285, 214)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_TFT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 80000000)
	hFreq(0, 31000, 60000)
	vFreq(0, 56, 75)
End
#endif
