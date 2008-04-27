Monitor(Sony, CPD300SFT5, Sony_CPD300SFT5)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("CPD-300SFT5")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1280, 1024)
	Size(388, 292)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_ON_GREEN | 
		KGIM_ST_SYNC_0714_0286 | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC)
/*???*/	Bandwidth(0, 200000000)
	hFreq(0, 30000, 86000)
	vFreq(0, 48, 150)
End
#endif
