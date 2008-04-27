Monitor(MAXDATA, Belinea106020, MAXDATA_Belinea106020)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("MAXDATA")
	Model("Belinea 10 60 20")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
/* ???	19" monitor */
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)
	Bandwidth(0, 150000000)
	hFreq(0, 30000, 95000)
	vFreq(0, 50, 150)
End
#endif
