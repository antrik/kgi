Monitor(MAXDATA, Belinea101510, MAXDATA_Belinea101510)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("MAXDATA")
	Model("Belinea 10 15 10")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1024, 768)
/* ???	15" monitor */
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_TFT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC | KGIM_ST_SYNC_0700_0300)
	Bandwidth(0, 80000000)
	hFreq(0, 30000, 61000)
	vFreq(0, 50, 77)
End
#endif
