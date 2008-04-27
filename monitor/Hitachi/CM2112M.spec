Monitor(Hitachi, CM2112M, Hitachi_CM2112M)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Hitachi")
	Model("CM2112ME/MET")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1280)
	Size(395, 295)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_COMPOSITE |
		KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_0700_0300 |
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 220000000)
	hFreq(0, 31000, 107000)
	vFreq(0, 50, 160)
End
#endif
