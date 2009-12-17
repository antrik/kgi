#include <argp.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <hurd/trivfs.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "kgi/module.h"
#include "kgi/kgi.h"

extern int init_module(void);
extern void cleanup_module(void);

static kgi_display_t *display=NULL;

kgi_s_t kgi_register_display(kgi_display_t *dpy, kgi_u_t id)
{
	if(display)
		error(3, 0, "Trying to register more than one display? Sorry, can't cope with that...");

	fprintf(stderr, "kgi_register_display(dpy=%p, id=%i)\n", dpy, id);
	display=dpy;
	return KGI_EOK;
}

void    kgi_unregister_display(kgi_display_t *dpy)
{
	fprintf(stderr, "kgi_unregister_display(dpy=%p)\n", dpy);
	assert(dpy==display);    /* there should be only one, or otherwise we would have bailed on init... */
	display=NULL;
}

void setmode(void)
{
	kgi_mode_t *mode;

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

	display->mode = mode;
}

void unsetmode(void)
{
	kgi_mode_t *mode = display->mode;

	(display->UnsetMode)(display, mode->img, mode->images, mode->dev_mode);
}

kgi_mmio_region_t *get_fb(const kgi_mode_t *mode, int *resource)
{
	int res_i;

	assert(mode);

	for(res_i = 0; res_i < __KGI_MAX_NR_RESOURCES; ++res_i) {
		const kgi_resource_t *res = mode->resource[res_i];
		if (res && res->type == KGI_RT_MMIO_FRAME_BUFFER) {
			if (resource)
				*resource = res_i;
			return (kgi_mmio_region_t *)res;
		}
	}

	return NULL;    /* not found */
}

void draw_crap(void)
{
	const kgi_mmio_region_t *fb = get_fb(display->mode, NULL);
	char *ptr;

	const int offs = time(NULL);
	const int width = display->mode->img[0].virt.x;

	assert(fb);

	/* assuming the whole framebuffer is mapped in the aperture */
	for(ptr = (char *)fb->win.virt; ptr < (char *)fb->win.virt + fb->win.size; ++ptr)
		*ptr = ((int)ptr + offs) % (width - 1) & 0xff;
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
 * the protocol, without interference from others. However, only one can
 * actually set a mode at any given time. (Enforced by checking display->mode.)
 */
struct po_state {
	kgi_mode_t mode;
	enum {
		KGI_STATUS_NONE=0,
		KGI_STATUS_CHECKED,
		KGI_STATUS_SET
	} status;
	enum {
		KGI_MMAP_NONE=0,
		KGI_MMAP_FB
	} mmap_mode;
};

void unset_mode(struct po_state *state)
{
	kgi_mode_t *const mode = &state->mode;

	if (state->status == KGI_STATUS_SET) {
		(display->UnsetMode)(display, mode->img, mode->images, mode->dev_mode);
		display->mode = NULL;
	}

	free(mode->dev_mode);
	memset(mode, 0, sizeof (*mode));
}

error_t open_hook(struct trivfs_peropen *po)
{
	struct po_state *state;

	fprintf(stderr, "KGI open()\n");

	po->hook = state = malloc(sizeof *state);
	if(!state)
		return ENOMEM;
	memset(state, 0, sizeof *state);

	return 0;
}

void close_hook(struct trivfs_peropen *po)
{
	struct po_state *state = po->hook;

	fprintf(stderr, "KGI close()\n");

	unset_mode(state);
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

		if (display->mode)    /* a mode was set from other client/connection */
			return EBUSY;

		(display->SetMode)(display, mode->img, mode->images, mode->dev_mode);

		state->status = KGI_STATUS_SET;
		display->mode = mode;
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

		/* first things first... */
		if (state->status != KGI_STATUS_SET && state->status != KGI_STATUS_CHECKED)
			return EPROTO;

		unset_mode(state);
		state->status = KGI_STATUS_NONE;
	}

	return 0;
}

kern_return_t kgi_get_fb_resource(trivfs_protid_t io_object, kgic_mapper_resource_info_result_t *fb)
{
	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_READ))
		return EBADF;

	fprintf(stderr, "kgi_get_fb_resource()\n");

	{
		struct po_state *const state = io_object->po->hook;

		/* first things first... */
		if (state->status != KGI_STATUS_SET && state->status != KGI_STATUS_CHECKED)
			return EPROTO;

		{
			int res_index;
			const kgi_mmio_region_t *fb_res = get_fb(&state->mode, &res_index);

			if (!fb_res)
				return ENXIO;

			memset(fb, 0, sizeof (*fb));

			strncpy(fb->name, fb_res->name, sizeof (fb->name)); fb->name[sizeof (fb->name)-1] = 0;
			fb->resource = res_index;
			fb->image = -1;
			fb->type = fb_res->type;
			fb->protection = fb_res->prot;

			fb->info.mmio.access = fb_res->access;
			fb->info.mmio.align  = fb_res->align;
			fb->info.mmio.size   = fb_res->size;
			fb->info.mmio.window = fb_res->win.size;
		}
	}

	return 0;
}

kern_return_t kgi_setup_mmap_fb(trivfs_protid_t io_object)
{
	if (!io_object)
		return EOPNOTSUPP;
	if (!(io_object->po->openmodes & O_WRITE))
		return EBADF;

	fprintf(stderr, "kgi_setup_mmap_fb()\n");

	{
		struct po_state *const state = io_object->po->hook;

		state->mmap_mode = KGI_MMAP_FB;
	}
	
	return 0;
}

#include "system/GNU/hurd_video.h"

kern_return_t trivfs_S_io_map(
	trivfs_protid_t io_object,
	mach_port_t reply, mach_msg_type_name_t reply_type,
	memory_object_t *rd_obj, mach_msg_type_name_t *rd_type,
	memory_object_t *wr_obj, mach_msg_type_name_t *wr_type)
{
	if (!io_object)
		return EOPNOTSUPP;

	fprintf(stderr, "trivfs_S_io_map()\n");

	{
		const int rd = io_object->po->openmodes | O_READ;
		const int wr = io_object->po->openmodes | O_WRITE;
		const vm_prot_t prot = (rd ? VM_PROT_READ : 0) | (wr ? VM_PROT_WRITE : 0);

		struct po_state *const state = io_object->po->hook;

		if(!prot)
			return EBADF;

		/* first things first... */
		if (state->status != KGI_STATUS_SET)
			return EPROTO;

		switch (state->mmap_mode) {
			case KGI_MMAP_NONE:
				return EPROTO;

			case KGI_MMAP_FB: {
				const kgi_mmio_region_t *fb_res = get_fb(&state->mode, NULL);

				if(!fb_res)
					return ENXIO;

				{
					const memory_object_t fb_obj = get_fb_object(fb_res->win.phys, fb_res->win.size, prot);

					*rd_obj = rd ? fb_obj : MACH_PORT_NULL;
					*wr_obj = wr ? fb_obj : MACH_PORT_NULL;

					/* if the port is used twice, make sure we have a second reference */
					if (rd && wr)
						mach_port_mod_refs(mach_task_self(), fb_obj, MACH_PORT_RIGHT_SEND, 1);

					*rd_type = *wr_type = MACH_MSG_TYPE_MOVE_SEND;
				}
			}    /* case KGI_MMAP_FB */
		}
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
	if(!display)
		error(3, 0, "No suitable display found... I'm bored, bye.");

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
