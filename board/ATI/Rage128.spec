Board(ATI, Rage128)
#if Data
Begin
#	if 0
	SubsystemID(0x1002, 0x5041) /* RAGE128 PA */
	SubsystemID(0x1002, 0x5042) /* RAGE128 PB */
	SubsystemID(0x1002, 0x5043) /* RAGE128 PC */
	SubsystemID(0x1002, 0x5044) /* RAGE128 PD */
	SubsystemID(0x1002, 0x5045) /* RAGE128 PE */
	SubsystemID(0x1002, 0x5046) /* RAGE128 PF */
	SubsystemID(0x1002, 0x5047) /* RAGE128 PG */
	SubsystemID(0x1002, 0x5048) /* RAGE128 PH */
	SubsystemID(0x1002, 0x5049) /* RAGE128 PI */
	SubsystemID(0x1002, 0x504A) /* RAGE128 PJ */
	SubsystemID(0x1002, 0x504B) /* RAGE128 PK */
	SubsystemID(0x1002, 0x504C) /* RAGE128 PL */
	SubsystemID(0x1002, 0x504D) /* RAGE128 PM */
	SubsystemID(0x1002, 0x504E) /* RAGE128 PN */
	SubsystemID(0x1002, 0x504F) /* RAGE128 PO */
	SubsystemID(0x1002, 0x5050) /* RAGE128 PP */
	SubsystemID(0x1002, 0x5051) /* RAGE128 PQ */
	SubsystemID(0x1002, 0x5052) /* RAGE128 PR */
	SubsystemID(0x1002, 0x5053) /* RAGE128 PS */
	SubsystemID(0x1002, 0x5054) /* RAGE128 PT */
	SubsystemID(0x1002, 0x5055) /* RAGE128 PU */
	SubsystemID(0x1002, 0x5056) /* RAGE128 PV */
	SubsystemID(0x1002, 0x5057) /* RAGE128 PW */
	SubsystemID(0x1002, 0x5058) /* RAGE128 PX */
#	endif
	SubsystemID(0x1002, 0x5245) /* RAGE128 RE */
	SubsystemID(0x1002, 0x5246) /* RAGE128 RF */
	SubsystemID(0x1002, 0x5247) /* RAGE128 RG */
	SubsystemID(0x1002, 0x524B) /* RAGE128 RK */
	SubsystemID(0x1002, 0x524C) /* RAGE128 RL */
#	if 0
	SubsystemID(0x1002, 0x5345) /* RAGE128 SE */
	SubsystemID(0x1002, 0x5346) /* RAGE128 SF */
	SubsystemID(0x1002, 0x5347) /* RAGE128 SG */
	SubsystemID(0x1002, 0x5348) /* RAGE128 SH */
	SubsystemID(0x1002, 0x534B) /* RAGE128 SK */
	SubsystemID(0x1002, 0x534C) /* RAGE128 SL */
	SubsystemID(0x1002, 0x534D) /* RAGE128 SM */
	SubsystemID(0x1002, 0x534E) /* RAGE128 SN */

	SubsystemID(0x1002, 0x5446) /* RAGE128 TF */
	SubsystemID(0x1002, 0x544C) /* RAGE128 TL */
	SubsystemID(0x1002, 0x5452) /* RAGE128 TR */
	SubsystemID(0x1002, 0x5453) /* RAGE128 TS */
	SubsystemID(0x1002, 0x5454) /* RAGE128 TT */
	SubsystemID(0x1002, 0x5455) /* RAGE128 TU */
#	endif
	Vendor("ATI")
	Model("Rage 128")
	Driver(chipset,	ATI/R128, r128_chipset)
	Driver(ramdac,	ATI/R128, r128_ramdac)
	Driver(clock,	ATI/R128, r128_clock)
End
#endif
