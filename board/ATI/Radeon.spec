Board(ATI, Radeon, Standard_VANDA)
#if Data
Begin
	SubsystemID(0x1002, 0x5144) /* R100 */
	SubsystemID(0x1002, 0x5145)
	SubsystemID(0x1002, 0x5146)
	SubsystemID(0x1002, 0x5147)
	SubsystemID(0x1002, 0x5159) /* RV100 */
	SubsystemID(0x1002, 0x515A)
	SubsystemID(0x1002, 0x4C59) /* M6 */
	SubsystemID(0x1002, 0x4C5A)
	SubsystemID(0x1002, 0x5157) /* RV200 */
	SubsystemID(0x1002, 0x5158)
	SubsystemID(0x1002, 0x4C57) /* M7 */
	SubsystemID(0x1002, 0x4C58)
	Vendor("ATI")
	Model("Radeon")
	Driver(chipset,	ATI/RADEON, radeon_chipset)
	Driver(ramdac,	ATI/RADEON, radeon_ramdac)
	Driver(clock,	ATI/RADEON, radeon_clock)
	Driver(monitor, monosync/monosync, monosync_monitor)
End
#endif
