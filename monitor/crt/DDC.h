/* ----------------------------------------------------------------------------
**	DDC monitor meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2001	 	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: DDC.h,v $
**	Revision 1.4  2003/03/15 20:40:02  redmondp
**	-define MAINTAINER and KGIM_MONITOR_DRIVER for all c files
**	
**	Revision 1.3  2003/02/11 23:02:47  aldot
**	- remove redundant include
**	  test syncmail for nsouch
**	
**	Revision 1.2  2003/02/06 09:37:09  foske
**	Dropped VESA standard for faster DDC1 detection. New module option added
**	monitor_ddc: 0 = no ddc, 1 = ddc2 + fast ddc1, 2 = ddc2 + slow ddc1
**	
**	Revision 1.1  2003/02/02 22:49:34  cegger
**	fix warnings about implicit function calls
**	
**	
**	
*/

#ifndef _monitor_crt_DDC_h
#define _monitor_crt_DDC_h

kgi_error_t DDC1_Read_Configuration (crt_monitor_t *crt, 
	crt_monitor_io_t *crt_io, const kgim_options_t *options);

kgi_error_t DDC2_Read_Configuration (crt_monitor_t *crt, 
	crt_monitor_io_t *crt_io, const kgim_options_t *options);

#endif
