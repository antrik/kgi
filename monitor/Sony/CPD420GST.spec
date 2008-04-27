Monitor(Sony, CPD420GST, Sony_CPD420GST)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("Multiscan CPD-420GST")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1600, 1200)
	Size(365, 274)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_VESA_DPMS | KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 150000000)
	hFreq(0, 30000, 96000)
	vFreq(0, 48, 120)
End
#endif
