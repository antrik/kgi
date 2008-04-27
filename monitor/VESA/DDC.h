/** ---------------------------------------------------------------------------
**	Copyright (C)	2001	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**	MAINTAINER	Jos_Hulzink
**
*/
#ifndef _monitor_VESA_DDC_h
#define	_monitor_VESA_DDC_h
#include "kgi/kgi.h" 

struct ddc1_io_t{
	void (*init) (void*);
	void (*set_scl) (void*, kgi_u8_t);
	kgi_u16_t (*get_sda) (void*);
	void (*done) (void*);
	void * io;
};

#endif	/* #ifdef _monitor_VESA_DDC_h */
