/* ----------------------------------------------------------------------------
**	KGI command definitions
** ----------------------------------------------------------------------------
**	Copyright (C)	1998-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**	MAINTAINER	Steffen_Seeger
**
**	$Log: cmd.h,v $
**	Revision 1.6  2002/10/10 21:18:28  redmondp
**	- clean up clut set command
**	- add new get clut info command
**	
**	Revision 1.5  2002/09/19 22:38:53  aldot
**	- use proper defines
**	
**	Revision 1.4  2002/06/14 23:52:10  redmondp
**	More changes to command interface for image resources
**	
**	Revision 1.3  2002/06/14 20:00:31  fspacek
**	mmap setup.
**	
**	Revision 1.2  2002/06/12 03:02:55  fspacek
**	Image resource commands. By golbez.
**	
**	Revision 1.2  2001/07/03 08:47:10  seeger_s
**	- text16 removed, (text is now done via image resoruces)
**	
**	Revision 1.1.1.1  2000/04/18 08:50:41  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _kgi_command_h
#define	_kgi_command_h

/* ----------------------------------------------------------------------------
**	null command data. No input, no output.
** ----------------------------------------------------------------------------
*/
typedef struct
{
} kgic_null_t;

#define	kgic_null_result_t	kgic_null_t
#define	kgic_null_request_t	kgic_null_t

/* ----------------------------------------------------------------------------
**	mapper commands
** ----------------------------------------------------------------------------
*/
typedef struct
{
	kgi_ascii_t	client[64];
	kgi_version_t	client_version;

} kgic_mapper_identify_request_t;

typedef struct
{
	kgi_ascii_t	mapper[64];
	kgi_version_t	mapper_version;
	kgi_u32_t	resources;

} kgic_mapper_identify_result_t;



typedef struct
{
	kgi_u32_t	images;

} kgic_mapper_set_images_request_t;

typedef struct
{
} kgic_mapper_set_images_result_t;



typedef struct
{
} kgic_mapper_get_images_request_t;

typedef struct
{
	kgi_u32_t	images;

} kgic_mapper_get_images_result_t;



typedef struct
{
	kgi_u32_t		image;
	kgi_image_mode_t	mode;

} kgic_mapper_set_image_mode_request_t;

typedef struct
{
} kgic_mapper_set_image_mode_result_t;



typedef struct
{
	kgi_u32_t	image;

} kgic_mapper_get_image_mode_request_t;

typedef struct
{
	kgi_image_mode_t	mode;

} kgic_mapper_get_image_mode_result_t;


#define __KGIC_RESOURCE_REQUEST_COMMON \
	kgi_u_t image, resource;

/* set image in kgic_mapper_resource_info_request_t to this get mode 
** resource info.  0 to __KGI_MAX_NR_IMAGE_RESOURCES will get per image
** resource info.
*/
#define	KGIC_MAPPER_NON_IMAGE_RESOURCE   -1

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON

} kgic_mapper_resource_info_request_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON
	kgi_ascii_t		name[64];
	kgi_resource_type_t	type;
	kgi_protection_flags_t	protection;

	union {
		struct {
			kgi_u32_t	access;
			kgi_u32_t	align;
			kgi_size_t	size;
			kgi_size_t	window;

		}		mmio;

		struct {
			kgi_u32_t	buffers;
			kgi_size_t	buffer_size;

		}		accel;

		struct {
			kgi_size_t		aperture_size;

		}	shmem;

	}			info;

} kgic_mapper_resource_info_result_t;

typedef struct {
	
	kgi_resource_type_t type;
	__KGIC_RESOURCE_REQUEST_COMMON
	
	union {
	
		struct {
		
			kgi_u_t buffers;
			kgi_u_t max_order;
			kgi_u_t min_order;
			kgi_u_t priority;
		} accel;
	} private;
	
} kgic_mapper_mmap_setup_request_t;

typedef struct {

} kgic_mapper_mmap_setup_result_t;


typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
} kgic_resource_request_t;

/* ----------------------------------------------------------------------------
**	{c,a,i,t}lut commands
** ----------------------------------------------------------------------------
*/

typedef kgic_null_result_t kgic_clut_get_info_request_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u_t tables;
	kgi_u_t entries;
	
} kgic_clut_get_info_result_t;

typedef struct 
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u_t	lut;

} kgic_clut_select_request_t;

typedef kgic_null_result_t kgic_clut_select_result_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;

	kgi_u_t	lut;
	kgi_u_t	idx;
	kgi_u_t cnt;
	kgi_attribute_mask_t am;
	kgi_u16_t *data;

} kgic_clut_set_request_t;

typedef kgic_null_result_t kgic_clut_set_result_t;

/* alut */
typedef kgic_clut_select_request_t 	kgic_alut_select_request_t;
typedef kgic_null_result_t 		kgic_alut_select_result_t;
typedef kgic_clut_set_request_t 	kgic_alut_set_request_t;
typedef kgic_null_result_t 		kgic_alut_set_result_t;

/* ilut */
typedef kgic_clut_select_request_t 	kgic_ilut_select_request_t;
typedef kgic_null_result_t 		kgic_ilut_select_result_t;
typedef kgic_clut_set_request_t 	kgic_ilut_set_request_t;
typedef kgic_null_result_t 		kgic_ilut_set_result_t;

/* tlut */
typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u16_t	lut;		/* image and lut number */

} kgic_tlut_select_request_t;

typedef kgic_null_result_t kgic_tlut_select_result_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u16_t	lut;		/* image and lut number	*/
	kgi_u16_t	idx, cnt;	/* first entry to set	*/
	kgi_u16_t	dx, dy;		/* bitmap size		*/
	kgi_size_t	size;		/* size of the data buffer */
	kgi_u8_t	*data;

} kgic_tlut_set_request_t;

typedef kgic_null_result_t kgic_tlut_set_result_t;

/* ----------------------------------------------------------------------------
**	marker commands
** ----------------------------------------------------------------------------
*/
typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
} kgic_marker_info_request_t;

typedef struct
{
	kgi_marker_mode_t 	modes;
	kgi_u8_t 		shapes;
	kgi_u8_coord_t 		size;
	
} kgic_marker_info_result_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_marker_mode_t mode;
	
} kgic_marker_set_mode_request_t;

typedef kgic_null_result_t kgic_marker_set_mode_result_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u_t shape;

} kgic_marker_select_request_t;

typedef kgic_null_result_t kgic_marker_select_result_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u_t shape;
	kgi_u_t hot_x, hot_y;
	void *data;
	kgi_rgb_color_t *color;
	
} kgic_marker_set_shape_request_t;

typedef kgic_null_result_t kgic_marker_set_shape_result_t;

typedef struct 
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u_t x,y;
	
} kgic_marker_show_request_t;

typedef kgic_null_result_t  kgic_marker_show_result_t;

/* ----------------------------------------------------------------------------
**	text16 commands
** ----------------------------------------------------------------------------
*/

typedef kgic_null_request_t kgic_text16_info_request_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_ucoord_t size;
	kgi_ucoord_t virt;
	kgi_ucoord_t cell;
	kgi_ucoord_t font;
		     
} kgic_text16_info_result_t;

typedef struct
{
	__KGIC_RESOURCE_REQUEST_COMMON;
	
	kgi_u_t offset;
	kgi_u16_t *text;
	kgi_u_t cnt;
	
} kgic_text16_put_text16_request_t;

typedef kgic_null_result_t kgic_text16_put_text16_result_t;

/* ----------------------------------------------------------------------------
**	display commands
** ----------------------------------------------------------------------------
*/


/* ----------------------------------------------------------------------------
**	KGI command encoding
** ----------------------------------------------------------------------------
*/
#define	KGIC(type, command, callback, code)				\
	KGIC_##type##_##command = 					\
		(KGIC_##type##_COMMAND) | 				\
		(sizeof(kgic_##callback##_request_t) << KGIC_SIZE_SHIFT) |\
		(sizeof(kgic_##callback##_request_t) ? KGIC_RD : 0) |	\
		(sizeof(kgic_##callback##_result_t)  ? KGIC_WR : 0) |	\
		(code)

typedef enum
{
	KGIC_RD			= 0x40000000,
	KGIC_WR			= 0x80000000,
	KGIC_SIZE_MASK		= 0x3FFF0000,
	KGIC_SIZE_SHIFT		= 16,

	KGIC_MAPPER_COMMAND	= 0x00000000,	/* ext. mapper commands	*/
		KGIC(MAPPER, IDENTIFY,		mapper_identify,	0x0001),
		KGIC(MAPPER, SET_IMAGES,	mapper_set_images,	0x0002),
		KGIC(MAPPER, GET_IMAGES,	mapper_get_images,	0x0003),
		KGIC(MAPPER, SET_IMAGE_MODE,	mapper_set_image_mode,	0x0004),
		KGIC(MAPPER, GET_IMAGE_MODE,	mapper_get_image_mode,	0x0005),
		KGIC(MAPPER, MODE_CHECK,	null,			0x0006),
		KGIC(MAPPER, MODE_SET,		null,			0x0007),
		KGIC(MAPPER, MODE_UNSET,	null,			0x0008),
		KGIC(MAPPER, MODE_DONE,		null,			0x0009),
		KGIC(MAPPER, RESOURCE_INFO,	mapper_resource_info,	0x000A),
		KGIC(MAPPER, MMAP_SETUP,	mapper_mmap_setup,	0x000B),

	KGIC_RESOURCE_COMMAND   = 0x00000100,   /* resource commands */
		/* commands for KGI_RT_{ILUT,ALUT}_CONTROL */
		KGIC(RESOURCE, CLUT_GET_INFO,	clut_get_info,		0x0001),
		KGIC(RESOURCE, CLUT_SELECT,	clut_select,		0x0002),
		KGIC(RESOURCE, CLUT_SET,	clut_set,		0x0003),
		/* commands for KGI_RT_TLUT_CONTROL */
		KGIC(RESOURCE, TLUT_SELECT,	tlut_select,		0x0020),
		KGIC(RESOURCE, TLUT_SET,	tlut_set,		0x0021),
		/* commands from KGI_RT_{CURSOR,POINTER}_CONTROL */
		KGIC(RESOURCE, MARKER_SET_MODE,	marker_set_mode,	0x0030),
		KGIC(RESOURCE, MARKER_SELECT,	marker_select,		0x0031),
		KGIC(RESOURCE, MARKER_SET_SHAPE,marker_set_shape,	0x0032),
		KGIC(RESOURCE, MARKER_SHOW,	marker_show,		0x0033),
		KGIC(RESOURCE, MARKER_HIDE,	null,			0x0034),
		KGIC(RESOURCE, MARKER_UNDO,	null,			0x0035),
		KGIC(RESOURCE, MARKER_INFO,	marker_info,		0x0036),
		/* commands for KGI_RT_TEXT16_CONTROL */
		KGIC(RESOURCE, TEXT16_PUT_TEXT16,text16_put_text16,	0x0040),
		KGIC(RESOURCE, TEXT16_INFO,	text16_info,		0x0041),

	KGIC_MODE_COMMAND	= 0x00008000,	/* mode commands	*/

	KGIC_DISPLAY_COMMAND	= 0x0000C000,	/* display commands	*/

	KGIC_TYPE_MASK		= 0x0000FF00,
	KGIC_NR_MASK		= 0x000000FF,
	KGIC_COMMAND_MASK	= KGIC_TYPE_MASK | KGIC_NR_MASK

} kgi_command_t;

#define	KGIC_COMMAND(cmd)	((cmd) & KGIC_COMMAND_MASK)
#define	KGIC_SIZE(cmd)		(((cmd) & KGIC_SIZE_MASK) >> KGIC_SIZE_SHIFT)
#define	KGIC_READ(cmd)		((cmd) & KGIC_RD)
#define	KGIC_WRITE(cmd)		((cmd) & KGIC_WR)

#endif	/* #ifndef _kgi_command_h */
