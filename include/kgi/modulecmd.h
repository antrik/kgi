/* ----------------------------------------------------------------------------
**	KGI module specific command requests
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
**	$Log: modulecmd.h,v $
**	Revision 1.1.1.1  2000/04/18 08:51:10  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:10  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _kgi_modulecmd_h
#define	_kgi_modulecmd_h

struct kgic_img_buffer_param_t
{
	kgi_u_t		index;		/* index of buffer		*/
};

struct kgic_img_buffer_result_t
{
	kgi_u_t		mmio;		/* mmio_region index to use	*/
	kgi_u_t		mmio_offset;	/* offset into region (bytes)	*/
	kgi_u_t		accel;		/* accelerator to use		*/

	kgi_scoord_t	size;		/* application area size	*/
	kgi_scoord_t	origin;		/* application area origin	*/
	kgi_u_t		stride;		/* application area stride	*/

	kgi_u_t	lpm[__KGI_MAX_NR_ATTRIBUTES];	/* left pixel masks	*/
	kgi_u_t	rpm[__KGI_MAX_NR_ATTRIBUTES];	/* right pixel masks	*/
	kgi_u_t	cpm[__KGI_MAX_NR_ATTRIBUTES];	/* common pixel masks	*/

};

#endif	/* #ifdef _kgi_modulecmd_h */
