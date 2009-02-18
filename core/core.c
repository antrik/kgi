#include <argp.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <hurd/trivfs.h>
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

/* translator stuff */

int trivfs_fstype = FSTYPE_DEV;    /* or maybe _MISC?... */
int trivfs_fsid = 0;    /* let libtrivfs set it */

int trivfs_support_read = 0;
int trivfs_support_write = 0;
int trivfs_support_exec = 0;

int trivfs_allow_open = O_READ | O_WRITE;    /* we don't support the standard read()/write() operations, but our custom RPCs require read/write permissions */

void trivfs_modify_stat(struct trivfs_protid *cred, io_statbuf_t *st)
{
}

/* someone asks us to die */
error_t trivfs_goaway(struct trivfs_control *cntl, int flags)
{
	const int protid_users = ports_count_class(cntl->protid_class);
	const int force = flags & FSYS_GOAWAY_FORCE;
	const int nosync = flags & FSYS_GOAWAY_NOSYNC;

	/* does our live have a meaning yet? */
	if(protid_users && !force) {
		/* yeah, have to toil a bit longer -> blow it off */
		ports_enable_class(cntl->protid_class);
		return EBUSY;
	}
	/* nope, that was it */
	if(!nosync)    /* we use a liberal interpretation of NOSYNC here */
		cleanup_module();    /* "...as though it never existed..." */
	exit(0);    /* "...now I will just say goodbye" */
}

static struct argp kgi_argp = {
	.options = NULL,
	.parser = NULL,
	.args_doc = NULL,
	.doc = "A server providing graphics hardware access"
};

#include "kgiServer.h"

/*
 * All the state is managed per client connection. Thus each client can follow
 * the protocol, without interference from others. However, there is no
 * protection against multiple clients setting a mode -- the latest will simply
 * overwrite the earlier. Perhaps we should make open() exclusive in the first
 * place?
 *
 * Also, if the client closes the connection without unsetting the mode first,
 * the mode remains active, but we loose all reference to the state. This seems
 * wrong. Probably we should either unset the mode on client exit, or make
 * these things global after all...
 */
struct po_state {
	kgi_mode_t mode;
	enum {
		KGI_STATUS_NONE=0,
		KGI_STATUS_CHECKED,
		KGI_STATUS_SET
	} status;
};

error_t open_hook(struct trivfs_peropen *po)
{
	struct po_state *state;

	po->hook = state = malloc(sizeof *state);
	if(!state)
		return ENOMEM;
	memset(state, 0, sizeof *state);

	return 0;
}

void close_hook(struct trivfs_peropen *po)
{
	struct po_state *state = po->hook;

	free(state->mode.dev_mode);
	free(state);
}

error_t (*trivfs_peropen_create_hook) (struct trivfs_peropen *) = open_hook;
void (*trivfs_peropen_destroy_hook) (struct trivfs_peropen *) = close_hook;

kern_return_t kgi_set_images(trivfs_protid_t io_object, int images)
{
	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_WRITE))
		return EBADF;

	fprintf(stderr, "kgi_set_images(%d)\n", images);

	{
		struct po_state *const state = io_object->po->hook;

		/* first things first... */
		if (state->status != KGI_STATUS_NONE)
			return EPROTO;

		/* we do not actually support multiple images -- hail the glorious overengineering! */
		if (images != 1)
			return EINVAL;

		state->mode.images = images;
	}

	return 0;
}

kern_return_t kgi_set_image_mode(trivfs_protid_t io_object, int image, kgi_image_mode_t mode)
{
	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_WRITE))
		return EBADF;

	fprintf(stderr, "kgi_set_image_mode(%d, {flags=%d virt.x=%d virt.y=%d size.x=%d size.y=%d frames=%d fam=%d bpfa[0]=%d bpfa[1]=%d bpfa[2]=%d bpfa[3]=%d})\n", image, mode.flags, mode.virt.x, mode.virt.y, mode.size.x, mode.size.y, mode.frames, mode.fam, mode.bpfa[0], mode.bpfa[1], mode.bpfa[2], mode.bpfa[3]);

	{
		struct po_state *const state = io_object->po->hook;

		/* first things first... */
		if (state->status != KGI_STATUS_NONE || !state->mode.images)
			return EPROTO;

		memcpy(&state->mode.img[0], &mode, sizeof mode);
	}

	return 0;
}

kern_return_t kgi_get_image_mode(trivfs_protid_t io_object, int image, kgi_image_mode_t *mode)
{
	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_READ))
		return EBADF;

	fprintf(stderr, "kgi_get_image_mode(%d)\n", image);

	{
		struct po_state *const state = io_object->po->hook;

		/* first things first... */
		if (!state->mode.images)
			return EPROTO;

		memcpy(mode, &state->mode.img[0], sizeof (*mode));
	}

	return 0;
}

kern_return_t kgi_check_mode(trivfs_protid_t io_object)
{
	assert(display);
	assert(display->CheckMode);

	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_WRITE))
		return EBADF;

	fprintf(stderr, "kgi_check_mode()\n");

	{
		struct po_state *const state = io_object->po->hook;
		kgi_mode_t *const mode = &state->mode;

		/* first things first... */
		if (state->status != KGI_STATUS_NONE || !state->mode.images)
			return EPROTO;

		mode->dev_mode = malloc(display->mode_size);
		if(!mode->dev_mode)
			error_at_line(1, errno, __FILE__, __LINE__, "setmode()");

		(display->CheckMode)(display, KGI_TC_PROPOSE, mode->img, mode->images, mode->dev_mode, mode->resource, __KGI_MAX_NR_RESOURCES);

		state->status = KGI_STATUS_CHECKED;
	}

	return 0;
}

kern_return_t kgi_set_mode(trivfs_protid_t io_object)
{
	assert(display);
	assert(display->SetMode);

	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_WRITE))
		return EBADF;

	fprintf(stderr, "kgi_set_mode()\n");

	{
		struct po_state *const state = io_object->po->hook;
		kgi_mode_t *const mode = &state->mode;

		/* first things first... */
		if (state->status != KGI_STATUS_CHECKED)
			return EPROTO;

		(display->SetMode)(display, mode->img, mode->images, mode->dev_mode);
		(display->SetMode)(display, mode->img, mode->images, mode->dev_mode);    /* doesn't lock on first attempt... known problem, unknown cause */

		state->status = KGI_STATUS_SET;
	}

	return 0;
}

kern_return_t kgi_unset_mode(trivfs_protid_t io_object)
{
	assert(display);
	assert(display->UnsetMode);

	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_WRITE))
		return EBADF;

	fprintf(stderr, "kgi_unset_mode()\n");

	{
		struct po_state *const state = io_object->po->hook;
		kgi_mode_t *const mode = &state->mode;

		/* first things first... */
		if (state->status != KGI_STATUS_SET && state->status != KGI_STATUS_CHECKED)
			return EPROTO;

		if (state->status == KGI_STATUS_SET)
			(display->UnsetMode)(display, mode->img, mode->images, mode->dev_mode);

		free(mode->dev_mode);
		memset(mode, 0, sizeof (*mode));
		state->status = KGI_STATUS_NONE;
	}

	return 0;
}

static int kgi_demuxer (mach_msg_header_t *inp, mach_msg_header_t *outp)
{
	extern int kgi_server (mach_msg_header_t *inp, mach_msg_header_t *outp);
	return kgi_server(inp, outp) || trivfs_demuxer(inp, outp);
}

int main(int argc, char *argv[])
{
	mach_port_t bootstrap;
	struct trivfs_control *control;
	error_t err;

	argp_parse(&kgi_argp, argc, argv, 0, NULL, NULL);

	init_module();
	assert(display);

	task_get_bootstrap_port(mach_task_self(), &bootstrap);
	if(bootstrap == MACH_PORT_NULL) {    /* not started as translator */
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

	err = trivfs_startup(bootstrap, 0, NULL, NULL, NULL, NULL, &control);
	mach_port_deallocate(mach_task_self(), bootstrap);
	if(err)
		error(2, err, "trivfs_startup");
	
	ports_manage_port_operations_one_thread(control->pi.bucket, kgi_demuxer, 0);
	return 0;    /* not reached */
}
