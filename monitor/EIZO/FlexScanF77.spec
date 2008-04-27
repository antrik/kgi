Monitor(EIZO, FlexScanF77, EIZO_FlexScanF77)
#if Data
Begin
	Contributor("Dirk Lattermann <dlatt@datenrat.de>")
	Vendor("EIZO")
	Model("FlexScan F77")
	Flags(KGIM_MF_NORMAL | KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
	Size(390, 290)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_COMPOSITE |
		KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)
	Bandwidth(0, 230000000)
	hFreq(0, 30000, 95000)
	vFreq(0, 50, 160)
End
#endif
