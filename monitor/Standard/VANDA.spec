Monitor(Standard, VANDA)
#if Data
Begin
	Contributor("Nicholas Souchu <nsouch@freebsd.org>")
	Vendor("Standard")
	Model("VANDA")
	Flags(KGIM_MF_NORMAL)
	MaxRes(640, 480)
	Size(240, 180)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL)
	Bandwidth(0, 32000000)
	hFreq(0, 30000, 38000)
	vFreq(0, 50, 70)
End
#endif
