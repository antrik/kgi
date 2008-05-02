
#ifndef _linux_kgii_h
#define	_linux_kgii_h

#ifdef __KERNEL__

/*	KGI/KII limits. 
**
** NOTE	All limits need to be less than 255.
*/
#define	CONFIG_KGII_MAX_NR_DISPLAYS	8
#define	CONFIG_KGII_MAX_NR_INPUTS	255
#define	CONFIG_KGII_MAX_NR_FOCUSES	4
#define	CONFIG_KGII_MAX_NR_CONSOLES	64
#define	CONFIG_KGII_MAX_NR_DEVICES	(2*CONFIG_KGII_MAX_NR_CONSOLES)

#define	INVALID_DISPLAY	((unsigned)-1)
#define	INVALID_INPUT	((unsigned)-1)
#define	INVALID_FOCUS	((unsigned)-1)
#define	INVALID_CONSOLE	((unsigned)-1)
#define	INVALID_DEVICE	((unsigned)-1)

#if !defined(CONFIG_KGI_CONSOLEBUFSIZE) || \
	(0 >= CONFIG_KGI_CONSOLEBUFSIZE) || (CONFIG_KGI_CONSOLEBUFSIZE > 15)
#undef CONFIG_KGI_CONSOLEBUFSIZE
#define CONFIG_KGI_CONSOLEBUFSIZE 4
#endif
#define	CONFIG_KGII_MAX_NR_DEFFONTS	4
#define	CONFIG_KGII_CONSOLEBUFSIZE	(CONFIG_KGI_CONSOLEBUFSIZE*PAGE_SIZE)

#if !defined(CONFIG_KGI_DISPLAYS) || ( 0 >= CONFIG_KGI_DISPLAYS) || \
	(CONFIG_KGI_DISPLAYS > CONFIG_KGII_MAX_NR_DISPLAYS)
#undef CONFIG_KGI_DISPLAYS
#define CONFIG_KGI_DISPLAYS 4
#endif

/*
**	public API
*/
extern void kgi_boot_init(void);

#ifdef	CONFIG_KGI_DPY_I386
extern int dpy_i386_init(int display, int max_display);
#endif
#ifdef	CONFIG_KGI_DPY_NULL
extern int dpy_null_init(int display, int max_display);
#endif

extern void kgi_init(void);
extern int focus_init(void);
extern void dev_console_init(void);
extern int dev_graphic_init(void);
extern int dev_event_init(void);

/*
**	public data, kgi.c
*/
extern unsigned char console_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
extern unsigned char graphic_map[CONFIG_KGII_MAX_NR_FOCUSES][CONFIG_KGII_MAX_NR_CONSOLES];
extern unsigned char focus_map[CONFIG_KGII_MAX_NR_DEVICES];
extern unsigned char display_map[CONFIG_KGII_MAX_NR_DEVICES];
extern unsigned int console_printk_console;

/*
**	The debugging framework
*/

#include <kgi/debug.h>

#ifdef	KRN_DEBUG_ANSI_CPP

	extern void __krn_ansi_error(const char *fmt, ...);
	extern void __krn_ansi_debug(int level, const char *fmt, ...);
	extern void __krn_ansi_notice(const char *fmt, ...);

#	define	__krn_error	__krn_ansi_error
#	define	__krn_debug	__krn_ansi_debug
#	define	__krn_notice	__krn_ansi_notice
#endif

#ifdef	KRN_DEBUG_GNU_CPP

	extern void __krn_gnu_error(const char *file, int line,
			const char *func, const char *fmt, ...);
	extern void __krn_gnu_debug(const char *file, int line,
			const char *func, int level, const char *fmt, ...);
	extern void __krn_gnu_notice(const char *fmt, ...);

#	define	__krn_error	__krn_gnu_error
#	define	__krn_debug	__krn_gnu_debug
#	define	__krn_notice	__krn_gnu_notice
#endif

#endif	/* #ifdef __KERNEL__	*/

#endif	/* #ifndef _linux_kgii_h	*/
