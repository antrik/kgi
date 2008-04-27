/* ---------------------------------------------------------------------------
**	KGI kernel driver module environment.
** ---------------------------------------------------------------------------
**	Copyright (C)	1995-2000	Steffen Seeger 
**	Copyright (C)	1995-1997	Andreas Beck	
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ---------------------------------------------------------------------------
**
** 	$Log: kgim-0.9.c,v $
** 	Revision 1.7  2003/01/09 10:47:34  foske
** 	Fixed initialisation of meta memory. Now null pointer checks will actually
** 	work.
** 	
** 	Revision 1.6  2002/10/15 00:52:22  aldot
** 	- include cleanup
** 	
** 	Revision 1.5  2002/09/23 13:38:03  aldot
** 	- add kgim_strncpy
** 	
** 	Revision 1.4  2002/08/23 20:06:39  aldot
** 	- emit ERRORs instead of DEBUGs on failure paths
** 	
** 	Revision 1.3  2002/07/28 18:03:04  aldot
** 	- back out old args
** 	
** 	Revision 1.2  2002/07/27 00:15:20  aldot
** 	- mark console msgs explicitely
** 	- emit ERRORs instead of DEBUGs on failure paths; other minor cosmetics;
** 	
** 	Revision 1.1.1.1  2001/07/18 14:57:10  aldot
** 	import of kgi-0.9 20020321
** 	
** 	Revision 1.3  2001/07/03 08:58:26  seeger_s
** 	- image resource export implemented
** 	
** 	Revision 1.2  2000/09/21 09:25:04  seeger_s
** 	- E() -> KGI_ERRNO()
** 	
** 	Revision 1.1.1.1  2000/04/18 08:51:03  seeger_s
** 	- initial import of pre-SourceForge tree
** 	
*/

#define	MODULE

#ifndef DEBUG_LEVEL
#define	DEBUG_LEVEL	1
#endif

#include "kgi/config.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>

#include <kgi/module.h>
#define __KGI_NEED_MALLOC
#include <kgi/alloc.h>

MODULE_LICENSE("GPL and additional rights");

kgi_u_t kgim_attr_bits(const kgi_u8_t *bpa)
{
	kgi_u_t bits = 0;

	KRN_DEBUG(2, "kgim_attr_bits()");

	if (bpa) {

		while (*bpa) {

			bits += *(bpa++);
		}
	}

	return bits;
}

/*
**	Debugging output functions
*/
#define	__KGIM_BUF_SIZE	1024

#if (HOST_OS == HOST_OS_Linux)
#	define	__KGIM_HAS_STDARGS
#	define	__KGIM_HAS_LIBC
#	if defined(__KERNEL__) || defined(__MODULE__)
#		define	PRINT	printk
#	else
#		define	PRINT	printf
#	endif
#	include <stdarg.h>
#endif

void kgim_ansi_debug(int level, const char *fmt, ...)
{
#ifdef __KGIM_HAS_STDARGS
	char buf[__KGIM_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT("<%i>%s\n", level, buf);
#else
#	error implement kgim_ansi_debug()!
#endif
}

void kgim_ansi_error(const char *fmt, ...)
{
#ifdef	__KGIM_HAS_STDARGS
	char buf[__KGIM_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT("%s\n", buf);
#else
#	error implement kgim_ansi_error()!
#endif
}

void kgim_gnu_debug(const char *file, int line, const char *func, int level,
	const char *fmt, ...)
{
#ifdef	__KGIM_HAS_STDARGS
	char buf[__KGIM_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT("<%i>%s:%s:%i:D:%s\n", level, file, func, line, buf);
#else
#	error implement kgim_gnu_debug()!
#endif
}

void kgim_gnu_error(const char *file, int line, const char *func, 
	const char *fmt, ...)
{
#ifdef __KGIM_HAS_STDARGS
	char buf[__KGIM_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT(KERN_ERR "%s:%s:%i:E:%s\n", file, func, line, buf);
#else
#	error implement kgim_gnu_error()!
#endif
}

void kgim_notice(const char *fmt, ...)
{
#ifdef __KGIM_HAS_STDARGS
	char buf[__KGIM_BUF_SIZE];
	va_list args;

	va_start(args, fmt);
		vsprintf(buf, fmt, args);
	va_end(args);
	PRINT(KERN_NOTICE "%s\n", buf);
#else
#	error implement kgim_ansi_notice()!
#endif
}

/*
**	memset(), memcpy(), memcmp() and strcpy()
*/
void kgim_memset(void *p, kgi_u8_t val, kgi_size_t size)
{
#if (HOST_OS == HOST_OS_Linux)
	memset(p, val, size);
#else
#	error implement kgim_memset()!
#endif
}

void kgim_memcpy(void *dst, const void *src, kgi_size_t size)
{
#if (HOST_OS == HOST_OS_Linux)
	memcpy(dst, src, size);
#else
#	error implement kgim_memcpy()!
#endif
}

kgi_s_t kgim_memcmp(const void *s1, const void *s2, kgi_size_t size)
{
#if (HOST_OS == HOST_OS_Linux)
	return memcmp(s1, s2, size);
#else
#	error implement kgim_memcmp()!
#endif
}

kgi_s_t kgim_strcmp(const kgi_u8_t *s1, const kgi_u8_t *s2)
{
#if (HOST_OS == HOST_OS_Linux)
	return strcmp(s1, s2);
#else
#	error implement kgim_strcmp()!
#endif
}

kgi_u8_t *kgim_strcpy(kgi_u8_t *dst, const kgi_u8_t *src)
{
#if (HOST_OS == HOST_OS_Linux)
	return strcpy(dst, src);
#else
#	error implement kgim_strcpy()!
#endif
}

kgi_u8_t *kgim_strncpy(kgi_u8_t *dst, const kgi_u8_t *src, kgi_size_t size)
{
#if (HOST_OS == HOST_OS_Linux)
	return strncpy(dst, src, size);
#else
#	error implement kgim_strncpy()!
#endif
}


/*
**	kgim_display_t initialization/deinitalization
*/
#define	KGIM_ALIGN(x)	(((x) + 7) & ~7)

static kgi_error_t kgim_display_command(
	kgi_display_t *kgi_dpy,
	kgi_u_t cmd, void *in, void **out, kgi_size_t *out_size)
{
/*	kgim_display_t *kgim = (kgim_display_t *) kgi_dpy;
**	kgim_display_mode_t *kgim_mode = (kgim_display_mode_t *) dev_mode;
*/
	KRN_ERROR("kgim_display_command(cmd = %d)", cmd);

	return -KGI_ERRNO(DRIVER, INVAL);
}


static kgi_error_t kgim_display_check_mode(kgi_display_t *kgi_dpy,
	kgi_timing_command_t cmd, kgi_image_mode_t *img, kgi_u_t images,
	void *dev_mode, const kgi_resource_t **r, kgi_u_t rsize)
{
	kgi_s_t cnt;
	kgim_display_t *dpy = (kgim_display_t *) kgi_dpy;
	kgim_display_mode_t *dpy_mode = dev_mode;

	kgim_monitor_mode_t	*monitor_mode;
	kgim_ramdac_mode_t	*dac_mode;
	kgim_chipset_mode_t	*chipset_mode;
	kgim_clock_mode_t	*clock_mode;

	KRN_DEBUG(2, "kgim_display_check_mode()");

	if (1 != images) {

		KRN_ERROR("%i images not yet supported", images);
		return -KGI_ERRNO(DRIVER, NOSUP);
	}

#warning for KGI_TC_CHECK too??
	
	if (KGI_TC_PROPOSE == cmd) {

		kgi_u_t i;

		KRN_DEBUG(2, "KGI_TC_PROPOSE:");

		kgim_memset(dpy_mode, 0, dpy->kgi.mode_size);

		dev_mode = ((kgi_u8_t *) dev_mode) +
				KGIM_ALIGN(sizeof(kgim_display_mode_t));

		for (i = 0; i < __KGIM_MAX_NR_SUBSYSTEMS; i++) {

			const kgim_meta_t *meta = dpy->subsystem[i].meta_lang;

			if (meta && meta->mode_size) {

				dpy_mode->subsystem_mode[i] = dev_mode;
				dev_mode = ((kgi_u8_t *) dev_mode) +
					KGIM_ALIGN(meta->mode_size);

				KRN_DEBUG(2, "subsystem_mode %i @ %p", i,
				dpy_mode->subsystem_mode[i]);
			}
		}

		if (! img[0].fam) {

			if (img[0].flags & KGI_IF_TEXT16) {

				KRN_DEBUG(2, "Proposing default textmode");

				img[0].fam = KGI_AM_TEXT;
				kgim_strcpy(img[0].bpfa, (kgi_u8_t[]){4,4,8,0});

			} else {

				KRN_DEBUG(2, "Proposing default graphic mode");

				img[0].fam = KGI_AM_COLOR_INDEX;
				kgim_strcpy(img[0].bpfa, (kgi_u8_t[]){8,0});
			}
		}
	}

	chipset_mode	= dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_chipset];
	dac_mode	= dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_ramdac];
	clock_mode	= dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_clock];
	monitor_mode	= dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_monitor];

#warning for KGI_TC_CHECK too??
	
	if (KGI_TC_PROPOSE == cmd) {

		KRN_DEBUG(2, "KGI_TC_PROPOSE(2):");

		chipset_mode->crt	= monitor_mode;
		dac_mode->crt		= monitor_mode;

		img[0].out = &(monitor_mode->in);
		KRN_DEBUG(2, "monitor_mode @ %p, dpm @ %p", monitor_mode,
			&(monitor_mode->in));
	}

	KRN_DEBUG(3, "Getting ready to check mode against all subsystems....");

	cnt = 10;
	while (cnt-- && (KGI_TC_READY != cmd)) {
		const kgim_meta_t *meta;

		kgi_error_t error;

#define	SUBSYSTEM_CHECK_MODE(sys)					\
		meta = dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_lang;	\
		error = (meta && meta->ModeCheck)			\
			? meta->ModeCheck(				\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_data,\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_io,\
				dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_##sys],\
				cmd, img, images)			\
			: KGI_EOK;					\
		KRN_DEBUG(2, "system %i, io %p, mode %p, cmd %i, return %.8x", \
			KGIM_SUBSYSTEM_##sys, 				\
			dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_io,	\
			dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_##sys],	\
			cmd, error);

		KRN_DEBUG(4, "Checking against clock...");
		SUBSYSTEM_CHECK_MODE(clock)
		if (error) {

			KRN_ERROR("Clock check failed! (%.8x)", error);
			return error;
		}

		KRN_DEBUG(4, "Checking against ramdac...");
		SUBSYSTEM_CHECK_MODE(ramdac)
		if (error) {

			KRN_ERROR("Ramdac check failed! (%.8x)", error);
			return error;
		}

		KRN_DEBUG(4, "Checking against chipset...");
		SUBSYSTEM_CHECK_MODE(chipset)
		if (error) {

			KRN_ERROR("Chipset check failed! (%.8x)", error);
			return error;
		}

		KRN_DEBUG(4, "Checking against clock(2)...");
		SUBSYSTEM_CHECK_MODE(clock)
		if (error) {

			KRN_ERROR("Clock check 2 failed! (%.8x)", error);
			return error;
		}

		KRN_DEBUG(4, "Checking against monitor...");
		SUBSYSTEM_CHECK_MODE(monitor)

		switch (error) 
		{
		case KGI_TC_LOWER:
			KRN_DEBUG(2, "KGI_TC_LOWER:"); cmd = error; continue;
		case KGI_TC_RAISE:
			KRN_DEBUG(2, "KGI_TC_RAISE:"); cmd = error; continue;
		case KGI_TC_CHECK:
			KRN_DEBUG(2, "KGI_TC_CHECK:"); cmd = error; continue;
		case KGI_TC_READY:
			KRN_DEBUG(2, "KGI_TC_READY:"); cmd = error; continue;
		default:
			KRN_ERROR("Unknown command %.8x from monitor", error);
			return error;
		}

#undef	SUBSYSTEM_CHECK_MODE
	}

	if (cnt < 0) {

		KRN_ERROR("Exceeded 10 tries for a good mode, bailing out....");
		return -EINVAL;

	} else {

		kgi_u_t index, subsys_index;
		kgim_subsystem_type_t subsys;

		KRN_DEBUG(4, "Setting up global resources and metadata "
			"for %i subsystems", KGIM_LAST_SUBSYSTEM);
		index = 0;
		subsys = 0;
		subsys_index = 0;
		while (subsys < KGIM_LAST_SUBSYSTEM) {

			const kgim_meta_t *meta;
			void *meta_data, *meta_mode;
			kgi_resource_t *resource;

			if (rsize <= index) {

				KRN_ERROR("rsize (%i) <= index (%i)",
					rsize, index);
				return -ENOMEM;
			}

			meta = dpy->subsystem[subsys].meta_lang;
			meta_data = dpy->subsystem[subsys].meta_data;
			meta_mode = dpy_mode->subsystem_mode[subsys];

			resource = (meta && meta->ModeResource)
				? meta->ModeResource(meta_data, meta_mode,
					img, images, subsys_index)
				: NULL;

			if (resource) {

				KRN_DEBUG(2, "resource %i: %s io %p meta %p",
					index, resource->name,
					resource->meta_io, resource->meta);
				r[index++] = resource;
				subsys_index++;

			} else {

				subsys++;
				subsys_index = 0;
			}
			continue;
		}

		KRN_DEBUG(4, "Setting up image resources and metadata "
			"for %i subsystems", KGIM_LAST_SUBSYSTEM);
		index = 0;
		subsys = 0;
		subsys_index = 0;
		while (subsys < KGIM_LAST_SUBSYSTEM) {

			const kgim_meta_t *meta;
			void *meta_data, *meta_mode;
			kgi_resource_t *resource;

			if (__KGI_MAX_NR_IMAGE_RESOURCES <= index) {

				KRN_ERROR("%i <= index",
					__KGI_MAX_NR_IMAGE_RESOURCES);
				return -ENOMEM;
			}

			meta = dpy->subsystem[subsys].meta_lang;
			meta_data = dpy->subsystem[subsys].meta_data;
			meta_mode = dpy_mode->subsystem_mode[subsys];

			resource = (meta && meta->ImageResource)
				? meta->ImageResource(meta_data, meta_mode,
					img, 0, subsys_index)
				: NULL;

			if (resource) {

				KRN_DEBUG(2, "img resource %i: %s io %p meta %p",
					index, resource->name,
					resource->meta_io, resource->meta);
				img[0].resource[index++] = resource;
				subsys_index++;

			} else {

				subsys++;
				subsys_index = 0;
			}
			continue;
		}

		return KGI_EOK;
	}
}

static void kgim_display_set_mode(kgi_display_t *kgi_dpy,
	kgi_image_mode_t *img, kgi_u_t images, void *dev_mode)
{
	kgim_display_t *dpy = (kgim_display_t *) kgi_dpy;
	kgim_display_mode_t *dpy_mode = dev_mode;
	const kgim_meta_t *meta;
	
	KRN_DEBUG(2, "kgim_display_set_mode()");

#define	SUBSYSTEM_PREPARE_MODE(sys)					\
		meta = dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_lang;	\
		if (meta && meta->ModePrepare) {			\
									\
			KRN_DEBUG(3, "preparing " #sys);		\
			meta->ModePrepare(				\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_data,\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_io,\
				dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_##sys],\
				img, images);				\
		}

		SUBSYSTEM_PREPARE_MODE(monitor)
		SUBSYSTEM_PREPARE_MODE(ramdac)
		SUBSYSTEM_PREPARE_MODE(clock)
		SUBSYSTEM_PREPARE_MODE(chipset)

#undef	SUBSYSTEM_PREPARE_MODE

#define	SUBSYSTEM_ENTER_MODE(sys)					\
		meta = dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_lang;	\
		if (meta && meta->ModeEnter) {				\
									\
			meta->ModeEnter(				\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_data,\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_io,\
				dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_##sys],\
				img, images);				\
		}

		SUBSYSTEM_ENTER_MODE(monitor)
		SUBSYSTEM_ENTER_MODE(ramdac)
		SUBSYSTEM_ENTER_MODE(clock)
		SUBSYSTEM_ENTER_MODE(chipset)

#undef	SUBSYSTEM_ENTER_MODE
}

static void kgim_display_unset_mode(kgi_display_t *kgi_dpy,
	kgi_image_mode_t *img, kgi_u_t images, void *dev_mode)
{
	kgim_display_t *dpy = (kgim_display_t *) kgi_dpy;
	kgim_display_mode_t *dpy_mode = dev_mode;
	const kgim_meta_t *meta;

	KRN_DEBUG(2, "kgim_display_unset_mode()");

#define	SUBSYSTEM_LEAVE_MODE(sys)					\
		meta = dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_lang;	\
		if (meta && meta->ModeLeave) {				\
			meta->ModeLeave(				\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_data,\
				dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_io,\
				dpy_mode->subsystem_mode[KGIM_SUBSYSTEM_##sys],\
				img, images);				\
		}

		SUBSYSTEM_LEAVE_MODE(monitor)
		SUBSYSTEM_LEAVE_MODE(ramdac)
		SUBSYSTEM_LEAVE_MODE(clock)
		SUBSYSTEM_LEAVE_MODE(chipset)

#undef	SUBSYSTEM_LEAVE_MODE
}


static kgi_error_t kgim_display_subsystem_init(kgim_display_t *dpy, 
	kgim_subsystem_type_t system)
{
	const kgim_meta_t *meta = dpy->subsystem[system].meta_lang;
	void *meta_data, *meta_io;
	kgi_error_t error;

	KRN_DEBUG(2, "kgim_display_subsystem_init()");

	KRN_ASSERT(dpy);

	if (NULL == meta) {

		KRN_ERROR("No meta-language for subsystem %i", system);
		return -EINVAL;
	}

	KRN_DEBUG(2, "Initializing subsystem %i", system);

	meta_data = meta->data_size ?
		kgim_alloc(meta->data_size, GFP_KERNEL) : NULL;
	meta_io   = meta->io_size   ?
		kgim_alloc(meta->io_size, GFP_KERNEL)   : NULL;

	if ((meta->data_size && (NULL == meta_data)) ||
		(meta->io_size && (NULL == meta_io))) {

		KRN_ERROR("Failed to allocate meta data and I/O context");
		kgim_free(meta_io);
		meta_io = NULL;
		kgim_free(meta_data);
		meta_data = NULL;

		return -ENOMEM;
	}

	/* Clear the area. KGI assumes a pointer is NULL or valid. If the 
	 * meta blocks aren't filled with zeroes, an invalid pointer can have
	 * any value.
	 */
	if (meta->data_size) kgim_memset(meta_data, 0, meta->data_size);
	if (meta->io_size)   kgim_memset(meta_io, 0, meta->io_size);
	
	if ((0 == meta->io_size) && (KGIM_SUBSYSTEM_chipset != system)) {

		KRN_ASSERT(NULL == meta_io);

		KRN_DEBUG(3, "meta->io_size == 0; Assigning chipset's meta_io");

		meta_io = dpy->subsystem[KGIM_SUBSYSTEM_chipset].meta_io;
	}

	KRN_DEBUG(2, "meta->InitModule = %p", meta->InitModule);

	if (meta->InitModule) { 

		error = meta->InitModule(meta_data, meta_io, &(dpy->options));

		if (error) {

			KRN_ERROR("Failed to initialize meta data (%.8x)",
				error);

			if (meta->io_size) {

				kgim_free(meta_io);
				meta_io = NULL;
			}

			kgim_free(meta_data);
			meta_data = NULL;

			return error;
		}

		KRN_DEBUG(3, "Meta data initialized");

	} else {

		KRN_DEBUG(2, "No meta->InitModule() function for this device");
	}

	KRN_DEBUG(2, "meta->Init = %p", meta->Init);
	KRN_DEBUG(2, "meta_data[%i]=%p meta_io[%i]=%p",
		meta->data_size, meta_data, meta->io_size, meta_io);

	if (meta->Init) {
		
		error = meta->Init(meta_data, meta_io, &(dpy->options));

		if (error) {

			KRN_ERROR("Failed to initialize device (%.8x)", error);

			if (meta->DoneModule) {

				meta->DoneModule(meta_data, meta_io,
					&(dpy->options));
			}

			if (meta->io_size) {

				kgim_free(meta_io);
				meta_io = NULL;
			}

			kgim_free(meta_data);
			meta_data = NULL;

			return error;
		}

		KRN_DEBUG(2, "Device initialized");

	} else {

		KRN_DEBUG(2, "No meta->Init() function for this device");
	}

	dpy->subsystem[system].meta_data = meta_data;
	dpy->subsystem[system].meta_io   = meta_io;
	dpy->kgi.mode_size += KGIM_ALIGN(meta->mode_size);

	KRN_DEBUG(2, "Subsystem %i initialized", system);

	return KGI_EOK;
}

static void kgim_display_subsystem_done(kgim_display_t *dpy,
	kgim_subsystem_type_t system)
{
	const kgim_meta_t *meta = dpy->subsystem[system].meta_lang;
	void *meta_data   = dpy->subsystem[system].meta_data;
	void *meta_io     = dpy->subsystem[system].meta_io;

	KRN_DEBUG(2, "kgim_display_subsystem_done()");

	if (meta->Done) {

		meta->Done(meta_data, meta_io, &(dpy->options));
	}

	if (meta->DoneModule) {

		meta->DoneModule(meta_data, meta_io, &(dpy->options));
	}

	if (meta->io_size) {

		kgim_free(meta_io);
		meta_io = NULL;
		dpy->subsystem[system].meta_io = NULL;
	}
	kgim_free(meta_data);
	meta_data = NULL;
	dpy->subsystem[system].meta_data = NULL;
	dpy->kgi.mode_size -= KGIM_ALIGN(meta->mode_size);
}

kgi_error_t kgim_display_init(kgim_display_t *dpy)
{
	kgi_error_t error;

	KRN_DEBUG(2, "kgim_display_init()");

	dpy->kgi.mode_size = KGIM_ALIGN(sizeof(kgim_display_mode_t));

	KRN_DEBUG(4, "Initializing chipset subsystem....");
	if ((error = kgim_display_subsystem_init(dpy, KGIM_SUBSYSTEM_chipset))){
		KRN_ERROR("Failed (%.8x) to initialize chipset", error);
		goto chipset;
	}
	KRN_DEBUG(4, "Chipset subsystem initialized");

	KRN_DEBUG(4, "Initializing ramdac subsystem....");
	if ((error = kgim_display_subsystem_init(dpy, KGIM_SUBSYSTEM_ramdac))) {

		KRN_ERROR("Failed (%.8x) to initialize ramdac", error);
		goto ramdac;
	}
	KRN_DEBUG(4, "Ramdac subsystem initialized");

	KRN_DEBUG(4, "Initializing clock subsystem....");
	if ((error = kgim_display_subsystem_init(dpy, KGIM_SUBSYSTEM_clock))) {

		KRN_ERROR("Failed (%.8x) to initialize clock", error);
		goto clock;
	}
	KRN_DEBUG(4, "Clock subsystem initialized");

	KRN_DEBUG(4, "Initializing monitor subsystem....");
	if ((error = kgim_display_subsystem_init(dpy, KGIM_SUBSYSTEM_monitor))){

		KRN_ERROR("Failed (%.8x) to initialize monitor", error);
		goto monitor;
	}
	KRN_DEBUG(4, "Monitor subsystem initialized");

	dpy->kgi.revision	= KGI_DISPLAY_REVISION;
/* no KGIM_INVALID{_DISPLAY,} in module.h*/
	dpy->kgi.id		= -1;
	dpy->kgi.Command	= kgim_display_command;
	dpy->kgi.CheckMode	= kgim_display_check_mode;
	dpy->kgi.SetMode	= kgim_display_set_mode;
	dpy->kgi.UnsetMode	= kgim_display_unset_mode;

	if ((error = kgi_register_display(&(dpy->kgi),
		dpy->options.misc->display))) {

		KRN_ERROR("Failed (%d) to register display.", error);
		goto display;
	}

	KRN_DEBUG(1, "%s %s driver loaded as display %i (%p)",
		dpy->kgi.vendor, dpy->kgi.model, dpy->kgi.id, dpy);
	return KGI_EOK;

	/*	If there was some error during startup, we still have to
	**	shut down properly.
	*/
display:
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_monitor);
monitor:
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_clock);
clock:
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_ramdac);
ramdac:
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_chipset);
chipset:
	return error;
}

void kgim_display_done(kgim_display_t *dpy)
{
	KRN_DEBUG(2, "kgim_display_done()");

	kgi_unregister_display(&dpy->kgi);

	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_monitor);
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_clock);
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_ramdac);
	kgim_display_subsystem_done(dpy, KGIM_SUBSYSTEM_chipset);
}
