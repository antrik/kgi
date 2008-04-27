Monitor(MAXDATA, Belinea104010, MAXDATA_Belinea104010)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("MAXDATA")
	Model("Belinea 10 40 10")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1024, 768)
/* ???	14" monitor */
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)
	Bandwidth(0, 65000000)
	hFreq(0, 30000, 54000)
	vFreq(0, 50, 120)
End
#endif
