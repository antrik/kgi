/* ----------------------------------------------------------------------------
**	KII command definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: cmd.h,v $
**	Revision 1.1.1.1  2000/09/21 09:19:30  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1  2000/09/21 09:19:30  seeger_s
**	- added /dev/event mapper definitions
**	
**	
*/
#ifndef _kii_command_h
#define	_kii_command_h

/* ----------------------------------------------------------------------------
**	null command data. No input, no output.
** ----------------------------------------------------------------------------
*/
typedef struct
{
} kiic_null_t;

#define	kiic_null_result_t	kiic_null_t
#define	kiic_null_request_t	kiic_null_t

/* ----------------------------------------------------------------------------
**	mapper commands
** ----------------------------------------------------------------------------
*/
typedef struct
{
	kii_ascii_t	client[64];
	kii_version_t	client_version;

} kiic_mapper_identify_request_t;

typedef struct
{
	kii_ascii_t	mapper[64];
	kii_version_t	mapper_version;

} kiic_mapper_identify_result_t;


typedef struct
{
} kiic_mapper_get_keymap_info_request_t;

typedef struct
{
	kii_u_t	fn_buf_size;
	kii_u_t fn_str_size;

	kii_u_t	keymin, keymax;
	kii_u_t keymap_size;

	kii_u_t combine_size;

} kiic_mapper_get_keymap_info_result_t;


typedef struct
{
	kii_u_t	keymap, keymin, keymax;

} kiic_mapper_get_keymap_request_t, kiic_mapper_set_keymap_result_t;

typedef struct
{
	kii_u_t	keymap, keymin, keymax;
	kii_unicode_t	map[256];

} kiic_mapper_get_keymap_result_t, kiic_mapper_set_keymap_request_t;


/* ----------------------------------------------------------------------------
**	KII command encoding
** ----------------------------------------------------------------------------
*/
#define	KIIC(command, type, code)					\
	KIIC_##command =						\
		(sizeof(kiic_##type##_request_t) << KIIC_SIZE_SHIFT) |	\
		(sizeof(kiic_##type##_request_t) ? KIIC_RD : 0) |	\
		(sizeof(kiic_##type##_result_t)  ? KIIC_WR : 0) |	\
		(code)

typedef enum
{
	KIIC_RD			= 0x40000000,
	KIIC_WR			= 0x80000000,
	KIIC_SIZE_MASK		= 0x3FFF0000,
	KIIC_SIZE_SHIFT		= 16,

	KIIC_MAPPER_COMMAND	= 0x00000000,	/* ext. mapper commands	*/
		KIIC(MAPPER_IDENTIFY,		mapper_identify,	0x0001),
		KIIC(MAPPER_MAP_DEVICE,		null,			0x0002),
		KIIC(MAPPER_UNMAP_DEVICE,	null,			0x0003),
		KIIC(MAPPER_GET_KEYMAP_INFO,	mapper_get_keymap_info,	0x0004),
		KIIC(MAPPER_GET_KEYMAP,		mapper_get_keymap,	0x0005),

	KIIC_TYPE_MASK		= 0x0000FF00,
	KIIC_NR_MASK		= 0x000000FF,
	KIIC_COMMAND_MASK	= KIIC_TYPE_MASK | KIIC_NR_MASK

} kii_command_t;

#define	KIIC_COMMAND(cmd)	((cmd) & KIIC_COMMAND_MASK)
#define	KIIC_SIZE(cmd)		(((cmd) & KIIC_SIZE_MASK) >> KIIC_SIZE_SHIFT)
#define	KIIC_READ(cmd)		((cmd) & KIIC_RD)
#define	KIIC_WRITE(cmd)		((cmd) & KIIC_WR)

#endif	/* #ifndef _kii_command_h */
