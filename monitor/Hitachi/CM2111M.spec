Monitor(Hitachi, CM2111M, Hitachi_CM2111M)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi")
	Model("CM2111ME/MET")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1280)
	Size(395, 295)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_COMPOSITE | 
		KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 200000000)
	hFreq(0, 31000, 95000)
	vFreq(0, 50, 160)
End
#endif
