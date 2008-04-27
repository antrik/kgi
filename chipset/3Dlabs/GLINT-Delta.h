/* ----------------------------------------------------------------------------
**	3Dlabs GLINT Delta register definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1997-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: GLINT-Delta.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:15  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:15  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/

#ifndef	_chipset_3Dlabs_GLINT_Delta_h
#define	_chipset_3Dlabs_GLINT_Delta_h

/*	PCI Config region
**
**	please use definitions in pci.h
*/

/*
**	Control Registers (Base0)
*/
#define	DELTA_Base0_Size	0x20000

/*
**	Control Status
*/
#define	DELTA_ControlStatusBase	0x0000
#define	DELTA_ControlStatusSize	0x1000

#define	DELTA_CS_DResetStatus	0x0800
#	define	DELTA_CS800_Reset			0x80000000

#define	DELTA_CS_DIntEnable	0x0808
#	define	DELTA_CS808_DMAIntEnable		0x00000001
#	define	DELTA_CS808_ErrorIntEnable		0x00000008

#define	DELTA_CS_DIntFlags	0x0810
#	define	DELTA_CS810_DMAFlag			0x00000001
#	define	DELTA_CS810_ErrorFlag			0x00000008

#define	DELTA_CS_DErrorFlags	0x0838
#	define	DELTA_CS838_InFIFOError			0x00000001
#	define	DELTA_CS838_DMAError			0x00000004

#define	DELTA_CS_DTestRegister	0x0848
	/* *never* write! */

#define	DELTA_CS_DFIFODis	0x0868
#	define	DELTA_CS868_InFIFODisconnectEnable	0x00000001

/*
**	Local Buffer Registers
*/
#define	DELTA_LocalBufferBase	0x1000
#define	DELTA_LocalBufferSize	0x0800

/*
**	Frame Buffer Registers
*/
#define	DELTA_FrameBufferBase	0x1800
#define	DELTA_FrameBufferSize	0x0800

/*
**	GC FIFO Interface
*/
#define	DELTA_GCFIFOBase	0x2000
#define	DELTA_GCFIFOSize	0x1000

/*
**	Internal Video Registers
*/
#define	DELTA_InternalVideoBase	0x3000
#define	DELTA_InternalVideoSize	0x1000

/*
**	External Video Registers
*/
#define	DELTA_ExternalVideoBase	0x4000
#define	DELTA_ExternalVideoSize	0x4000

/*
**	GC Register Access
*/
#define	DELTA_GCRegisterBase	0x8000
#define	DELTA_GCRegisterSize	0x8000

#define	DELTA_GC_V0Fixed_s	0x0200
#define	DELTA_GC_V0Fixed_t	0x0201
#define	DELTA_GC_V0Fixed_q	0x0202
#define	DELTA_GC_V0Fixed_Ks	0x0203
#define	DELTA_GC_V0Fixed_Kd	0x0204
#define	DELTA_GC_V0Fixed_red	0x0205
#define	DELTA_GC_V0Fixed_green	0x0206
#define	DELTA_GC_V0Fixed_blue	0x0207
#define	DELTA_GC_V0Fixed_alpha	0x0208
#define	DELTA_GC_V0Fixed_fog	0x0209
#define	DELTA_GC_V0Fixed_x	0x020A
#define	DELTA_GC_V0Fixed_y	0x020B
#define	DELTA_GC_V0Fixed_z	0x020C

#define	DELTA_GC_V1Fixed_s	0x0210
#define	DELTA_GC_V1Fixed_t	0x0211
#define	DELTA_GC_V1Fixed_q	0x0212
#define	DELTA_GC_V1Fixed_Ks	0x0213
#define	DELTA_GC_V1Fixed_Kd	0x0214
#define	DELTA_GC_V1Fixed_red	0x0215
#define	DELTA_GC_V1Fixed_green	0x0216
#define	DELTA_GC_V1Fixed_blue	0x0217
#define	DELTA_GC_V1Fixed_alpha	0x0218
#define	DELTA_GC_V1Fixed_fog	0x0219
#define	DELTA_GC_V1Fixed_x	0x021A
#define	DELTA_GC_V1Fixed_y	0x021B
#define	DELTA_GC_V1Fixed_z	0x021C

#define	DELTA_GC_V2Fixed_s	0x0220
#define	DELTA_GC_V2Fixed_t	0x0221
#define	DELTA_GC_V2Fixed_q	0x0222
#define	DELTA_GC_V2Fixed_Ks	0x0223
#define	DELTA_GC_V2Fixed_Kd	0x0224
#define	DELTA_GC_V2Fixed_red	0x0225
#define	DELTA_GC_V2Fixed_green	0x0226
#define	DELTA_GC_V2Fixed_blue	0x0227
#define	DELTA_GC_V2Fixed_alpha	0x0228
#define	DELTA_GC_V2Fixed_fog	0x0229
#define	DELTA_GC_V2Fixed_x	0x022A
#define	DELTA_GC_V2Fixed_y	0x022B
#define	DELTA_GC_V2Fixed_z	0x022C

#define	DELTA_GC_V0Float_s	0x0230
#define	DELTA_GC_V0Float_t	0x0231
#define	DELTA_GC_V0Float_q	0x0232
#define	DELTA_GC_V0Float_Ks	0x0233
#define	DELTA_GC_V0Float_Kd	0x0234
#define	DELTA_GC_V0Float_red	0x0235
#define	DELTA_GC_V0Float_green	0x0236
#define	DELTA_GC_V0Float_blue	0x0237
#define	DELTA_GC_V0Float_alpha	0x0223
#define	DELTA_GC_V0Float_fog	0x0239
#define	DELTA_GC_V0Float_x	0x023A
#define	DELTA_GC_V0Float_y	0x023B
#define	DELTA_GC_V0Float_z	0x023C

#define	DELTA_GC_V1Float_s	0x0240
#define	DELTA_GC_V1Float_t	0x0241
#define	DELTA_GC_V1Float_q	0x0242
#define	DELTA_GC_V1Float_Ks	0x0243
#define	DELTA_GC_V1Float_Kd	0x0244
#define	DELTA_GC_V1Float_red	0x0245
#define	DELTA_GC_V1Float_green	0x0246
#define	DELTA_GC_V1Float_blue	0x0247
#define	DELTA_GC_V1Float_alpha	0x0243
#define	DELTA_GC_V1Float_fog	0x0249
#define	DELTA_GC_V1Float_x	0x024A
#define	DELTA_GC_V1Float_y	0x024B
#define	DELTA_GC_V1Float_z	0x024C

#define	DELTA_GC_V2Float_s	0x0250
#define	DELTA_GC_V2Float_t	0x0251
#define	DELTA_GC_V2Float_q	0x0252
#define	DELTA_GC_V2Float_Ks	0x0253
#define	DELTA_GC_V2Float_Kd	0x0254
#define	DELTA_GC_V2Float_red	0x0255
#define	DELTA_GC_V2Float_green	0x0256
#define	DELTA_GC_V2Float_blue	0x0257
#define	DELTA_GC_V2Float_alpha	0x0223
#define	DELTA_GC_V2Float_fog	0x0259
#define	DELTA_GC_V2Float_x	0x025A
#define	DELTA_GC_V2Float_y	0x025B
#define	DELTA_GC_V2Float_z	0x025C

#define	DELTA_GC_DeltaMode	0x0260
#define	DELTA_GC_DrawTriangle	0x0261
#define	DELTA_GC_RepeatTriangle	0x0262
#define	DELTA_GC_DrawLine01	0x0263
#define	DELTA_GC_DrawLine10	0x0264
#define	DELTA_GC_RepeatLine	0x0265
#define	DELTA_GC_BroadcastMask	0x026F

#endif	/* #ifdef _chipset_3Dlabs_GLINT_Delta_h */
