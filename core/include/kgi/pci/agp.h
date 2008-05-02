/* ----------------------------------------------------------------------------
**	AGP standard rev 2.0 extensions to the PCI configuration
** ----------------------------------------------------------------------------
**	Copyright (C)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: agp.h,v $
**	Revision 1.1.1.1  2000/04/18 08:50:43  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:43  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef	_pci_agp_h
#define	_pci_agp_h

/*
**	The AGP standard defines these extensions to the PCI Config space
*/
#define	PCI_STATUS_AGP_LIST	0x00000010
#define	PCI_CFG08_AGP_LIST	0x00000010

#define	PCI_CAPABILITIES_POINTER	0x34
#	define	PCI_CFG34_CAP_PTR_MASK	0x000000FF

#define	PCI_CAP_ID		0x00
#	define	PCI_CAP00_ID_MASK		0x000000FF
#	define	PCI_CAP00_ID_AGP		0x00000002
#	define	PCI_CAP00_NEXT_PTR_MASK		0x0000FF00
#	define	PCI_CAP00_NEXT_PTR_SHIFT	8
#	define	PCI_CAP00_MINOR_MASK		0x000F0000
#	define	PCI_CAP00_MINOR_SHIFT		16
#	define	PCI_CAP00_MAJOR_MASK		0x00F00000
#	define	PCI_CAP00_MAJOR_SHIFT		20
#	define	PCI_CAP00_RESERVED		0xFF000000

/*
**	The AGP standard registers
*/
#define	PCI_AGP_STATUS		0x04
#	define	PCI_AGP04_DATA_RATE_MASK	0x00000007
#	define	PCI_AGP04_DATA_RATE_1X		0x00000001
#	define	PCI_AGP04_DATA_RATE_2X		0x00000002
#	define	PCI_AGP04_DATA_RATE_4X		0x00000004
#	define	PCI_AGP04_FW			0x00000010
#	define	PCI_AGP04_4G			0x00000020
#	define	PCI_AGP04_SBA			0x00000100
#	define	PCI_AGP04_RQ_DEPTH_MASK		0xFF000000
#	define	PCI_AGP04_RQ_DEPTH_SHIFT	24
#	define	PCI_AGP04_RESERVED		0x00FFFEC8

#define	PCI_AGP_COMMAND		0x08
#	define	PCI_AGP08_DATA_RATE_MASK	0x00000007
#	define	PCI_AGP08_DATA_RATE_1X		0x00000001
#	define	PCI_AGP08_DATA_RATE_2X		0x00000002
#	define	PCI_AGP08_DATA_RATE_4X		0x00000004
#	define	PCI_AGP08_FW_ENABLE		0x00000010
#	define	PCI_AGP08_4G			0x00000020
#	define	PCI_AGP08_AGP_ENABLE		0x00000100
#	define	PCI_AGP08_SBA_ENABLE		0x00000200
#	define	PCI_AGP08_RQ_DEPTH_MASK		0xFF000000
#	define	PCI_AGP08_RQ_DEPTH_SHIFT	24
#	define	PCI_AGP08_RESERVED		0x00FFFCC8

#endif /* #ifndef _pci_agp_h */
