Monitor(Sony, GDMW900, Sony_GDMW900)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("Multiscan GDM-W900")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1920, 1200)
	Size(482, 304)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_0700_0300 | 
		KGIM_ST_SYNC_ON_GREEN | KGIM_ST_SYNC_VESA_DPMS |
		KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 250000000)
	hFreq(0, 30000, 96000)
	vFreq(0, 50, 160)
End
#endif
