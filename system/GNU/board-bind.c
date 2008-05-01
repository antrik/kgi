/* ----------------------------------------------------------------------------
**	KGI-0.9 module board binding driver
** ----------------------------------------------------------------------------
**	Copyright (c)	1999-2000	Steffen Seeger
**
**	This file is distributed under the terms and conditions of the 
**	MIT/X public license. Please see the file COPYRIGHT.MIT included
**	with this software for details of these terms and conditions.
**
** ----------------------------------------------------------------------------
**
**	$Log: board-bind.c,v $
**	Revision 1.8  2004/02/15 22:05:38  aldot
**	- add optional debugging to monitor_options parsing
**	
**	Revision 1.7  2003/03/13 03:17:40  fspacek
**	- remove sscanf dependence
**	
**	Revision 1.6  2003/02/06 09:49:46  foske
**	No comment... sigh...
**	
**	Revision 1.5  2003/02/06 09:37:10  foske
**	Dropped VESA standard for faster DDC1 detection. New module option added
**	monitor_ddc: 0 = no ddc, 1 = ddc2 + fast ddc1, 2 = ddc2 + slow ddc1
**	
**	Revision 1.4  2002/12/09 18:26:13  fspacek
**	- parsing of monitor options
**	
**	Revision 1.3  2002/10/15 00:50:02  aldot
**	- include cleanup
**	
**	Revision 1.2  2002/06/25 02:47:44  aldot
**	- temporary workaround for monitor driver
**	    build in currently configured monitor; merely ment for ease of Users.
**	    FIXME: allow -out module to be replaced
**	
**	Revision 1.1.1.1  2000/04/18 08:51:02  aldot
**	import of kgi-0.9 20020321
**	
**	Revision 1.1.1.1  2000/04/18 08:51:02  seeger_s
**	- initial import of pre-SourceForge tree
**	
*/
#include <kgi/maintainers.h>
#define	MAINTAINER	Steffen_Seeger

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif
#define	MODULE

#include <linux/config.h>
#include <linux/module.h>

MODULE_LICENSE("GPL and additional rights");

#include <kgi/module.h>

/* ----------------------------------------------------------------------------
**	module options
** ----------------------------------------------------------------------------
*/

/*	misc options
*/
signed int display = -1;	MODULE_PARM(display,  "0-255i");
unsigned long law_base = 0;	MODULE_PARM(law_base, "1-4294967295i");

/*	PCI options
*/
signed int  pcibus = -1;	MODULE_PARM(pcibus, "0-255i");
signed int  pcidev = -1;	MODULE_PARM(pcidev, "0-255i");
signed int  pcifn  = -1;	MODULE_PARM(pcifn,  "0-255i");
unsigned long pcibase0 = 0;	MODULE_PARM(pcibase0, "1-4294967295i");
unsigned long pcibase1 = 0;	MODULE_PARM(pcibase1, "1-4294967295i");
unsigned long pcibase2 = 0;	MODULE_PARM(pcibase2, "1-4294967295i");
unsigned long pcibase3 = 0;	MODULE_PARM(pcibase3, "1-4294967295i");
unsigned long pcibase4 = 0;	MODULE_PARM(pcibase4, "1-4294967295i");
unsigned long pcibase5 = 0;	MODULE_PARM(pcibase5, "1-4294967295i");
unsigned long pcibase6 = 0;	MODULE_PARM(pcibase6, "1-4294967295i");
unsigned long pcibase7 = 0;	MODULE_PARM(pcibase7, "1-4294967295i");

/*	DAC options
*/
unsigned int dac_lclk_min =  0;	MODULE_PARM(dac_lclk_min, "0-4294967295i");
unsigned int dac_lclk_max = -1;	MODULE_PARM(dac_lclk_max, "0-4294967295i");
unsigned int dac_dclk_min =  0;	MODULE_PARM(dac_dclk_min, "0-4294967295i");
unsigned int dac_dclk_max = -1;	MODULE_PARM(dac_dclk_max, "0-4294967295i");

/*	Clock options
*/
unsigned int fref =  0;		MODULE_PARM(fref, "0-4294967295i");
unsigned int fvco_min = 0;	MODULE_PARM(fvco_min, "0-4294967295i");
unsigned int fvco_max = 0;	MODULE_PARM(fvco_max, "0-4294967295i");

/*	chipset options
*/
char *chipset = NULL;		MODULE_PARM(chipset, "s");
unsigned int chipset_ram = 0;	MODULE_PARM(chipset_ram, "0-4294967295i");

/*	monitor options
*/
char *monitor_timings = NULL;	MODULE_PARM(timings, "s");
char *monitor_vfreq = NULL;	MODULE_PARM(monitor_vfreq, "s");
char *monitor_hfreq = NULL;	MODULE_PARM(monitor_hfreq, "s");
char *monitor_dclk = NULL;	MODULE_PARM(monitor_dclk, "s");
unsigned int monitor_ddc = 1;	MODULE_PARM(monitor_ddc, "0-2i");
char *sync_type = NULL;		MODULE_PARM(sync_type, "s");
char *monitor_type = NULL;	MODULE_PARM(monitor_type, "s");
unsigned int monitor_width = 0; MODULE_PARM(monitor_width, "0-4294967295i");
unsigned int monitor_height = 0;MODULE_PARM(monitor_height, "0-4294967295i");

/*	Initialize kgim_display_t.options from the module-parameters passed.
**	This is common to all boards but needs to be local to each module.
*/
static inline kgi_u_t kgim_options_parse_uint(char **input, char end)
{
	kgi_u_t result = 0;

	while(**input >= '0' && **input <= '9'){

		result *= 10;
		result += **input - '0';
		++(*input);
	}

	if (**input != '\0') {

		if (**input != end) {

			KRN_ERROR("Malformed argument");
		}

		++(*input);
	}

	return result;
}

static inline kgi_s_t kgim_options_parse_int(char **input, char end)
{
	if (**input == '-') {

		++(*input);
		return -kgim_options_parse_uint(input, end);
	}
	else {

		return kgim_options_parse_uint(input, end);
	}
}


static inline void kgim_options_parse_ranges(char *input,
	kgi_urange_t *ranges, kgi_u_t *count)
{
	while (input && *input && *count < KGIM_OPTIONS_MAX_RANGES) {

		ranges[*count].min = kgim_options_parse_uint(&input, '-');
		ranges[*count].max = kgim_options_parse_uint(&input, ';');

		(*count)++;
	}
}


static inline void kgim_options_init(kgim_display_t *dpy,
	const kgi_u32_t *subsystemID, kgim_monitor_init_module_fn *monitor_init)
{
	kgi_u_t i;
	struct {
		const kgi_ascii_t *name;
		kgim_sync_type_t value;
	} sync_map[] = {
		{ "separate",		KGIM_ST_SYNC_SEPARATE		},
		{ "on_red",		KGIM_ST_SYNC_ON_RED		},
		{ "on_green",		KGIM_ST_SYNC_ON_GREEN		},
		{ "on_blue",		KGIM_ST_SYNC_ON_BLUE		},
		{ "serrate",		KGIM_ST_SYNC_SERRATE		},
		{ "composite",		KGIM_ST_SYNC_COMPOSITE		},
		{ "normal",		KGIM_ST_SYNC_NORMAL		},
		{ "pedestral",		KGIM_ST_SYNC_PEDESTRAL		},
		{ "0700_0300",		KGIM_ST_SYNC_0700_0300		},
		{ "0714_0286",		KGIM_ST_SYNC_0714_0286		},
		{ "0100_0400",		KGIM_ST_SYNC_0100_0400		},
		{ "unknown_level",	KGIM_ST_SYNC_UNKNOWNLEVEL	},
		{ "signal_level",	KGIM_ST_SYNC_SIGNALLEVEL	},
		{ "standby",		KGIM_ST_SYNC_STANDBY		},
		{ "suspend",		KGIM_ST_SYNC_SUSPEND		},
		{ "off",		KGIM_ST_SYNC_OFF		},
		{ "ssync_right",	KGIM_ST_SYNC_SSYNC_RIGHT	},
		{ "ssync_left",		KGIM_ST_SYNC_SSYNC_LEFT		},
		{ "gtf",		KGIM_ST_SYNC_MULTISYNC		}
	};
	struct {
		const kgi_ascii_t *name;
		kgim_monitor_type_t value;
	} type_map[] = {
		{ "digital",		KGIM_MT_DIGITAL		},
		{ "analog",		KGIM_MT_ANALOG		},
		{ "mono",		KGIM_MT_MONO		},
		{ "rgb",		KGIM_MT_RGB		},
		{ "yuv",		KGIM_MT_YUV		},
		{ "crt",		KGIM_MT_CRT		},
		{ "lcd",		KGIM_MT_LCD		},
		{ "tft",		KGIM_MT_TFT		},
		{ "dstn",		KGIM_MT_DSTN		},
		{ "plasma",		KGIM_MT_PLASMA		},
	};
	KRN_DEBUG(2, "kgim_options_init()");

#if defined(DEBUG_LEVEL) && (DEBUG_LEVEL > 1)
#define PRINT_TIMINGS(X,Y) \
	dpy->options_monitor.X = Y; \
	KRN_DEBUG(DEBUG_LEVEL, "%s = %s",#X, dpy->options_monitor.X)
#else
#define PRINT_TIMINGS(X,Y) \
	dpy->options_monitor.X = Y
#endif

	/*
	**	enter options
	*/
	dpy->options_misc.display  = display;
	dpy->options_misc.law_base = law_base;
	dpy->options.misc = &(dpy->options_misc);

	if (((pcibus != -1) || (pcidev != -1)) && (pcifn == -1)) {

		pcifn = 0;
	}
	dpy->options_pci.dev = PCICFG_VADDR(pcibus, pcidev, pcifn);
	if (PCICFG_NULL == dpy->options_pci.dev) {

		KRN_DEBUG(2, "probing subsystem");
		if (pcicfg_find_subsystem(&dpy->options_pci.dev, subsystemID)) {

			KRN_DEBUG(1, "subsystem autodetect failed.");
			dpy->options_pci.dev = PCICFG_NULL;
		}
	}
	dpy->options_pci.base0 = pcibase0;
	dpy->options_pci.base1 = pcibase1;
	dpy->options_pci.base2 = pcibase2;
	dpy->options_pci.base3 = pcibase3;
	dpy->options_pci.base4 = pcibase4;
	dpy->options_pci.base5 = pcibase5;
	dpy->options_pci.base6 = pcibase6;
	dpy->options_pci.base7 = pcibase7;
	dpy->options.pci = &(dpy->options_pci);

	dpy->options_chipset.chipset = chipset;
	dpy->options_chipset.memory = chipset_ram KB;
	dpy->options.chipset = &(dpy->options_chipset);

	dpy->options_ramdac.lclk_min = dac_lclk_min;
	dpy->options_ramdac.lclk_max = dac_lclk_max;
	dpy->options_ramdac.dclk_min = dac_dclk_min;
	dpy->options_ramdac.dclk_max = dac_dclk_max;
	dpy->options.ramdac = &(dpy->options_ramdac);

	dpy->options_clock.fref = fref;
	dpy->options_clock.fvco_min = fvco_min;
	dpy->options_clock.fvco_max = fvco_max;
	dpy->options.clock = &(dpy->options_clock);

	/* Preinitialize options from .spec file of the monitor */
	monitor_init(NULL, &(dpy->options_monitor));

	dpy->options_monitor.timings = 0;

	while (monitor_timings && *monitor_timings &&
		dpy->options_monitor.timings < KGIM_OPTIONS_MAX_TIMINGS) {

		kgi_u_t t = dpy->options_monitor.timings;

		PRINT_TIMINGS(x[t].width ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(x[t].blankstart ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(x[t].syncstart ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(x[t].syncend ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(x[t].blankend ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(x[t].total ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(x[t].polarity ,
			kgim_options_parse_int(&monitor_timings, '/'));

		PRINT_TIMINGS(y[t].width ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(y[t].blankstart ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(y[t].syncstart ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(y[t].syncend ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(y[t].blankend ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(y[t].total ,
			kgim_options_parse_uint(&monitor_timings, ','));
		PRINT_TIMINGS(y[t].polarity ,
			kgim_options_parse_int(&monitor_timings, ';'));

		dpy->options_monitor.timings++;
	}

	dpy->options_monitor.num_hfreq = 0;
	dpy->options_monitor.num_vfreq = 0;
	dpy->options_monitor.num_dclk = 0;

	kgim_options_parse_ranges(monitor_hfreq,
		dpy->options_monitor.hfreq, &dpy->options_monitor.num_hfreq);
	kgim_options_parse_ranges(monitor_vfreq,
		dpy->options_monitor.vfreq, &dpy->options_monitor.num_vfreq);
	kgim_options_parse_ranges(monitor_dclk,
		dpy->options_monitor.dclk, &dpy->options_monitor.num_dclk);

	while (sync_type && *sync_type) {

		char *end = strchr(sync_type, ',');

		if (end == NULL) {

			end = sync_type + strlen(sync_type);
		}

		for (i = 0; i < sizeof(sync_map)/sizeof(sync_map[0]); ++i) {

			if (strncmp(sync_type, sync_map[i].name,
				end - sync_type) == 0) {

				dpy->options_monitor.sync |= sync_map[i].value;
			}
		}

		sync_type = end;
	}

	while (monitor_type && *monitor_type) {

		char *end = strchr(monitor_type, ',');

		if (end == NULL) {

			end = monitor_type + strlen(monitor_type);
		}

		for (i = 0; i < sizeof(type_map)/sizeof(type_map[0]); ++i) {

			if (strncmp(monitor_type, type_map[i].name,
				end - monitor_type) == 0) {

				dpy->options_monitor.type |= type_map[i].value;
			}
		}

		monitor_type = end;
	}

	dpy->options_monitor.ddc = monitor_ddc;
	dpy->options_monitor.size.x = monitor_width;
	dpy->options_monitor.size.y = monitor_height;
	dpy->options_monitor.init_module = monitor_init;

	dpy->options.monitor = &(dpy->options_monitor);
#undef PRINT_TIMINGS
}

/* ----------------------------------------------------------------------------
**	board- and module-specific functions and declarations.
** ----------------------------------------------------------------------------
**	These are derived from the spec file misusing CPP. Consider it
**	'advanced voodoo' if you like.
*/
#define	Board(vendor, model, monitor)	\
									\
	extern kgi_error_t monitor_init_module_##monitor		\
	     (kgim_monitor_t *monitor, const kgim_options_monitor_t *options); \
									\
	static kgim_display_t kgim_##vendor##_##model##_display;	\
									\
	static void kgim_##vendor##_##model##_inc_refcount(kgi_display_t *dpy) \
	{								\
		MOD_INC_USE_COUNT;					\
	}								\
									\
	static void kgim_##vendor##_##model##_dec_refcount(kgi_display_t *dpy) \
	{								\
		MOD_DEC_USE_COUNT;					\
	}								\
									\
	static const kgi_u32_t kgim_##vendor##_##model##_subsystemID[] = \
	{

#define	Begin

#define	SubsystemID(vendor, device)					\
		PCICFG_SIGNATURE(vendor, device),			\

#define	Vendor(vendor_name)						\
		PCICFG_SIGNATURE(0x0000, 0x0000)			\
	};

#define	Model(model_name)

#define	Driver(subsystem, driver, meta)	\
	extern kgim_meta_t	meta##_meta;

#define	End

#ifndef	BOARD_SPEC
#	define	BOARD_SPEC	"board-bind.spec"
#endif

#define	Data	1
#include BOARD_SPEC

#undef	Board
#undef	Data
#undef	Begin
#undef	SubsystemID
#undef	Vendor
#undef	Model
#undef	Driver
#undef	End

#define	Board(vendor,model,monitor)					\
	static kgi_error_t board_init(kgim_display_t *dpy,		\
		const kgi_u32_t *subsystemID)				\
	{								\
		kgim_options_init(dpy, subsystemID,			\
				  monitor_init_module_##monitor);	\
									\
		return kgim_display_init(dpy);				\
	}

#define Data 0
#include BOARD_SPEC

#undef	Board
#undef	Data
#undef	Begin
#undef	SubsystemID
#undef	Vendor
#undef	Model
#undef	Driver
#undef	End

/*	Now derive the cleanup_module() and init_module() functions.
**	Techniques used are as above.
*/
#define	Board(vendor, model, monitor)					\
	void cleanup_module(void)					\
	{								\
		kgim_display_t *dpy = &kgim_##vendor##_##model##_display; \
		kgim_display_done(dpy);					\
		KRN_DEBUG(1, "closed "#vendor" "#model" display");	\
	}								\
									\
	int init_module(void)						\
	{								\
		kgim_display_t *dpy = &kgim_##vendor##_##model##_display; \
		const kgi_u32_t *subsystemID =				\
			 kgim_##vendor##_##model##_subsystemID;		\
		KRN_DEBUG(1, "initializing "#vendor" "#model" display");\
		kgim_memset(dpy, 0, sizeof(*dpy));			\
		dpy->kgi.IncRefcount = kgim_##vendor##_##model##_inc_refcount;\
		dpy->kgi.DecRefcount = kgim_##vendor##_##model##_dec_refcount;

#define	Begin

#define	SubsystemID(vendor, device)

#define	Vendor(vendor_name)						\
		kgim_strcpy(dpy->kgi.vendor, vendor_name);

#define	Model(model_name)						\
		kgim_strcpy(dpy->kgi.model, model_name);

#define	Driver(sys, driver, meta)					\
		dpy->subsystem[KGIM_SUBSYSTEM_##sys].meta_lang = &meta##_meta;

#define End								\
		return board_init(dpy, subsystemID);			\
	}

#define Data	1
#include BOARD_SPEC
