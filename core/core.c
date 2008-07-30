#include <stdio.h>

#include "kgi/kgi.h"

extern int init_module(void);
extern void cleanup_module(void);

kgi_s_t kgi_register_display(kgi_display_t *dpy, kgi_u_t id)
{
	fprintf(stderr, "kgi_register_display(dpy=%p, id=%i)\n", dpy, id);
	return KGI_EOK;
}

void    kgi_unregister_display(kgi_display_t *dpy)
{
	fprintf(stderr, "kgi_unregister_display(dpy=%p)\n", dpy);
}

int main(void)
{
	init_module();

	cleanup_module();
	return 0;
}
