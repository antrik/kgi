Monitor(Sony, CPD200PST, Sony_CPD200PST)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("Multiscan CPD-200PST")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1280, 1024)
	Size(328, 242)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 140000000)
	hFreq(0, 30000, 92000)
	vFreq(0, 48, 160)
End
#endif
