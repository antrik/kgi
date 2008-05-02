/* ----------------------------------------------------------------------------
**	/dev/graphic resource mapper definition
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
**	$Log: graphic.h,v $
**	Revision 1.3  2003/01/06 18:29:31  foske
**	Yet another cleanup for the devfs code. A real huge one this time :-)
**	
**	Revision 1.2  2002/06/14 20:00:31  fspacek
**	mmap setup.
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:50:42  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#ifndef _kgi_graphic_h
#define	_kgi_graphic_h

#define	GRAPH_MAJOR	60
#define	GRAPH_NAME	"graphic"
#define GRAPH_NAMELEN   7 /* this is strlen (GRAPH_NAME) */
typedef struct graph_file_s	graph_file_t;
typedef struct graph_device_s	graph_device_t;
typedef struct graph_mapping_s	graph_mapping_t;

#define	__GRAPH_RESOURCE_MAPPING					  \
	graph_file_t		*file;	/* the file this belongs to	*/\
	graph_device_t		*device;/* the device this belongs to	*/\
	graph_mapping_t		*next;	/* mappings of same file	*/\
	graph_mapping_t		*other;	/* mappings of same resource	*/\
	struct vm_area_struct	*vma;	/* virtual memory area		*/\
	unsigned long mstart, mend;	/* virtual region mapped	*/\
	kgi_resource_t	*resource;	/* the resource mapped		*/

struct graph_mapping_s
{
	__GRAPH_RESOURCE_MAPPING
};

/*	MMIO mappings
**
**	/dev/graphic provides a virtualized view to kgi_mmio_regions available
**	in a particular kgi_mode. Each kgi_mmio_region may be mapped several
**	times, possibly by several processes.
**	Per process, we keep a linked list of the various mappings (->next).
**	Additionally, all mappings of a particular kgi_mmio_region are kept
**	in a circular linked list (->other).
*/

typedef enum
{
	GRAPH_MM_INVALID = 0,
	GRAPH_MM_LINEAR_LINEAR,
	GRAPH_MM_LINEAR_PAGED,
	GRAPH_MM_PAGED_LINEAR,
	GRAPH_MM_PAGED_PAGED,

	GRAPH_MM_LAST

} graph_mmio_maptype_t;

typedef struct
{
	__GRAPH_RESOURCE_MAPPING

	graph_mmio_maptype_t	type;	/* mapping type			*/
	unsigned long 		offset;	/* offset for paged mappings	*/
	unsigned int		prot;	/* page protection flags	*/

} graph_mmio_mapping_t;

/*
**	Accelerator mappings
*/
typedef struct
{
	__GRAPH_RESOURCE_MAPPING

	unsigned long	buffer;		/* current mapped buffer offset	*/
	unsigned long	buf_size;	/* size of the buffers		*/
	unsigned long	buf_mask;	/* mask to wrap buffer		*/
	kgi_accel_buffer_t *buf;	/* list of buffers		*/
	unsigned int	buf_order;	/* ln2(buffer_size)-PAGE_SHIFT	*/

} graph_accel_mapping_t;


/*
**	A device represents the actual device.
*/
struct graph_device_s
{
	kgi_device_t	kgi;

	graph_mapping_t	*mappings[__KGI_MAX_NR_RESOURCES];

	void		*cmd_in_buffer, *cmd_out_buffer;
	struct semaphore cmd_mutex;
};


/*
**	A file is a particular virtual view of a device.
*/
typedef enum
{
	GRAPH_FF_CLIENT_IDENTIFIED	= 0x00000001,
	GRAPH_FF_SESSION_LEADER		= 0x00000002

} graph_file_flags_t;

struct graph_file_s
{
	unsigned long refcnt;
	graph_file_flags_t	flags;

	struct {
	
		kgic_mapper_mmap_setup_request_t request;
		
		const kgi_resource_t *resource;
		pid_t pid;
		gid_t gid;
	} mmap_setup;
		
	kgi_u_t		device_id;
	graph_device_t	*device;
	graph_mapping_t	*mappings;
};

#endif	/* #ifndef _kgi_graphic_h */
