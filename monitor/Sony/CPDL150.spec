Monitor(Sony, CPDL150, Sony_CPDL150)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("TFT Display L150")
	Flags(KGIM_MF_POWERSAVE | KGIM_MF_PROPSIZE)
	MaxRes(1024, 768)
	Size(304, 228)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_TFT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 135000000)
	hFreq(0, 30000, 70000)
	vFreq(0, 50, 85)
End
#endif
