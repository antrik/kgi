/* ----------------------------------------------------------------------------
**	I2C monitor meta language definition
** ----------------------------------------------------------------------------
**	Copyright (C)	2003	 	Jos Hulzink
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: I2C.h,v $
**	Revision 1.1  2003/02/09 11:15:33  foske
**	Initial commit of DDC2 testing code.
**	
*/

#ifndef _MONITOR_CRT_I2C_H
#define _MONITOR_CRT_I2C_H

/* BusInit: Sets up all pointers in i2c_io to build a complete i2c driver */
extern kgi_error_t i2c_BusInit (kgim_i2c_io_t *i2c_io);

/* ProbeAddress: Sends the address to the bus and checks for the slaves ack */
extern kgi_error_t i2c_ProbeAddress (kgim_i2c_io_t *i2c_io, kgi_u_t addr);

/* DevInit: Sets up the Device Record. Needed for normal reads and writes. */
extern kgi_error_t i2c_DevInit (I2CDevRec * d);
#endif
