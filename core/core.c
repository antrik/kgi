#include <assert.h>
#include <errno.h>
#include <error.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "kgi/module.h"
#include "kgi/kgi.h"

extern int init_module(void);
extern void cleanup_module(void);

static kgi_display_t *display=NULL;

kgi_s_t kgi_register_display(kgi_display_t *dpy, kgi_u_t id)
{
	fprintf(stderr, "kgi_register_display(dpy=%p, id=%i)\n", dpy, id);
	display=dpy;
	return KGI_EOK;
}

void    kgi_unregister_display(kgi_display_t *dpy)
{
	fprintf(stderr, "kgi_unregister_display(dpy=%p)\n", dpy);
	display=NULL;
}

static kgi_mode_t *mode;

void setmode(void)
{
	mode=malloc(sizeof (kgi_mode_t));
	if(!mode)
		error_at_line(1, errno, __FILE__, __LINE__, "setmode()");
	memset(mode, 0, sizeof (kgi_mode_t));

	mode->revision=KGI_MODE_REVISION;

	mode->dev_mode=malloc(display->mode_size);
	if(!mode->dev_mode)
		error_at_line(1, errno, __FILE__, __LINE__, "setmode()");

	mode->images=1;

	mode->img[0].fam |= KGI_AM_COLORS;
	mode->img[0].bpfa[0] = 5;
	mode->img[0].bpfa[1] = 6;
	mode->img[0].bpfa[2] = 5;
	mode->img[0].bpfa[3] = 0;
	mode->img[0].frames = 1;
	mode->img[0].size.x = 640;
	mode->img[0].size.y = 480;
	mode->img[0].virt.x = 640;
	mode->img[0].virt.y = 480;

	(display->CheckMode)(display, KGI_TC_PROPOSE, mode->img, mode->images, mode->dev_mode, mode->resource, __KGI_MAX_NR_RESOURCES);

	(display->SetMode)(display, mode->img, mode->images, mode->dev_mode);
	(display->SetMode)(display, mode->img, mode->images, mode->dev_mode);    /* doesn't lock on first attempt... known problem, unknown cause */
}

void unsetmode(void)
{
	(display->UnsetMode)(display, mode->img, mode->images, mode->dev_mode);
}

#include "chipset/Matrox/Gx00-meta.h"

void draw_crap(void)
{
	const kgim_display_t *dpy = (kgim_display_t *)display;
	const mgag_chipset_io_t *mgag_io = (mgag_chipset_io_t *)dpy->subsystem[KGIM_SUBSYSTEM_chipset].meta_io;
	char *ptr;

	for(ptr = (char *)mgag_io->fb.base_virt; ptr < (char *)mgag_io->fb.base_virt + mgag_io->fb.size; ++ptr)
		*ptr = ((int)ptr) & 0xff;
}

int main(void)
{
	init_module();
	assert(display);

	printf("Init complete; press <return>.\n"); getchar();

	setmode();

	printf("setmode() complete; press <return>.\n"); getchar();

	draw_crap();

	printf("draw_crap() complete; press <return>.\n"); getchar();

	unsetmode();

	printf("unsetmode() complete; press <return>.\n"); getchar();

	cleanup_module();
	assert(!display);
	return 0;
}
