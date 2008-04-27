Monitor(EIZO, FlexScanF55, EIZO_FlexScanF55)
#if Data
Begin
	Contributor("Christoph Egger <Christoph_Egger@t-online.de>")
	Vendor("EIZO")
	Model("FlexScan F55")
	Flags(KGIM_MF_NORMAL)
	MaxRes(1280, 1024)
	Size(328, 242)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_COMPOSITE |
                KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_VESA_DPMS |
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)
	Bandwidth(0, 100000000)	
	hFreq(0, 27000, 70000)
	vFreq(0, 50, 120)
End
#endif
