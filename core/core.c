#include <assert.h>
#include <stdio.h>

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

int main(void)
{
	init_module();
	assert(display);

	printf("Init complete; press <return>.\n"); getchar();

	cleanup_module();
	assert(!display);
	return 0;
}
