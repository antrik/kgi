Monitor(Sony, CPD1730, Sony_CPD1730)
#if Data
Begin
	Contributor("Steffen Seeger <seeger@physik.tu-chemnitz.de>")
	Vendor("Sony Corporation")
	Model("CPD-1730")
	Flags(KGIM_MF_POWERSAVE)
	MaxRes(1024, 768)
	Size(328, 242)
	Type(KGIM_MT_ANALOG | KGIM_MT_RGB | KGIM_MT_CRT)
	Sync(KGIM_ST_SYNC_NORMAL | KGIM_ST_SYNC_ON_GREEN | 
		KGIM_ST_SYNC_0714_0286 | KGIM_ST_SYNC_VESA_DPMS | 
		KGIM_ST_SYNC_MULTISYNC)
	Bandwidth(0, 135000000)
	hFreq(0, 28000, 58000)
	vFreq(0, 55, 110)
/*
**	Whitepoint(PhosphorP22)
*/
End
#endif
