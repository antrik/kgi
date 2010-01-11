#include <fcntl.h>
#include <hurd/fd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "kgiUser.h"

void draw_crap(void *addr, size_t size, int width)
{
	const int offs = time(NULL);
	char *ptr;

	for(ptr = (char *)addr; ptr < (char *)addr + size; ++ptr) {
		*ptr = ((int)ptr + offs) % (width - 1) & 0xff;
	}
}

int main(int argc, char *argv[])
{
	kgi_image_mode_t mode;
	int kgi_fd, kgi_fd2;
	int err;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <kgi node>\n", argv[0]);
		exit(1);
	}


	/* first try (hopefully) impossible mode */

	kgi_fd = open(argv[1], O_RDWR);
	if (kgi_fd == -1)
		error(2, errno, "Opening kgi node \"%s\" failed", argv[1]);

	err = HURD_DPORT_USE(kgi_fd, kgi_set_images(port, 1));
	if (err)
		error(3, err, "kgi_set_images() failed");

	memset(&mode, 0, sizeof mode);
	mode.fam |= KGI_AM_COLORS;
	mode.bpfa[0] = 5;
	mode.bpfa[1] = 6;
	mode.bpfa[2] = 5;
	mode.bpfa[3] = 0;
	mode.frames = 1;
	mode.size.x = 23;
	mode.size.y = 42;

	err = HURD_DPORT_USE(kgi_fd, kgi_set_image_mode(port, 1, mode));
	if (err)
		error(3, err, "kgi_set_image_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_check_mode(port));
	if (err != EINVAL)
		error(3, err, "kgi_check_mode()");

	err = close(kgi_fd);
	if (err == -1)
		error(2, errno, "Closing kgi node failed");


	/* now for real... */

	kgi_fd = open(argv[1], O_RDWR);
	if (kgi_fd == -1)
		error(2, errno, "Opening kgi node \"%s\" failed", argv[1]);

	err = HURD_DPORT_USE(kgi_fd, kgi_set_images(port, 1));
	if (err)
		error(3, err, "kgi_set_images() failed");

	memset(&mode, 0, sizeof mode);
	mode.fam |= KGI_AM_COLORS;
	mode.bpfa[0] = 5;
	mode.bpfa[1] = 6;
	mode.bpfa[2] = 5;
	mode.bpfa[3] = 0;
	mode.frames = 1;
	mode.size.x = 640;
	mode.size.y = 480;

	err = HURD_DPORT_USE(kgi_fd, kgi_set_image_mode(port, 1, mode));
	if (err)
		error(3, err, "kgi_set_image_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_check_mode(port));
	if (err)
		error(3, err, "kgi_check_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_get_image_mode(port, 1, &mode));
	if (err)
		error(3, err, "kgi_get_image_mode() failed");
	printf("Checked mode: flags=%d virt.x=%d virt.y=%d size.x=%d size.y=%d frames=%d fam=%d bpfa[0]=%d bpfa[1]=%d bpfa[2]=%d bpfa[3]=%d)\n", mode.flags, mode.virt.x, mode.virt.y, mode.size.x, mode.size.y, mode.frames, mode.fam, mode.bpfa[0], mode.bpfa[1], mode.bpfa[2], mode.bpfa[3]);

	err = HURD_DPORT_USE(kgi_fd, kgi_unset_mode(port));
	if (err)
		error(3, err, "kgi_unset_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_set_images(port, 1));
	if (err)
		error(3, err, "kgi_set_images() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_set_image_mode(port, 1, mode));
	if (err)
		error(3, err, "kgi_set_image_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_check_mode(port));
	if (err)
		error(3, err, "kgi_check_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_set_mode(port));
	if (err)
		error(3, err, "kgi_set_mode() failed");

	err = HURD_DPORT_USE(kgi_fd, kgi_get_image_mode(port, 1, &mode));
	if (err)
		error(3, err, "kgi_get_image_mode() failed");

	{
		kgic_mapper_resource_info_result_t fb_res;
		void *fb;

		err = HURD_DPORT_USE(kgi_fd, kgi_get_fb_resource(port, &fb_res));
		if (err)
			error(3, err, "kgi_get_fb_resource() failed");
		printf("Resource info: image=%d resource=%d name=\"%.64s\" type=%x protection=%x info.mmio.access=%x info.mmio.align=%x info.mmio.size=%lx info.mmio.window=%lx\n", fb_res.image, fb_res.resource, fb_res.name, fb_res.type, fb_res.protection, fb_res.info.mmio.access, fb_res.info.mmio.align, fb_res.info.mmio.size, fb_res.info.mmio.window);

		err = HURD_DPORT_USE(kgi_fd, kgi_setup_mmap_fb(port));
		if (err)
			error(3, err, "kgi_setup_mmap_fb() failed");

		fb = mmap(NULL, fb_res.info.mmio.size, PROT_READ | PROT_WRITE, MAP_SHARED, kgi_fd, 0);
		if (fb == MAP_FAILED)
			error(3, errno, "mmap() of framebuffer failed");

		draw_crap(fb, fb_res.info.mmio.size, mode.virt.x);
	}


	kgi_fd2 = open(argv[1], O_RDWR);
	if (kgi_fd2 == -1)
		error(2, errno, "Opening kgi node \"%s\" failed", argv[1]);

	err = HURD_DPORT_USE(kgi_fd2, kgi_set_images(port, 1));
	if (err)
		error(3, err, "kgi_set_images() failed");

	memset(&mode, 0, sizeof mode);
	mode.fam |= KGI_AM_COLORS;
	mode.bpfa[0] = 5;
	mode.bpfa[1] = 6;
	mode.bpfa[2] = 5;
	mode.bpfa[3] = 0;
	mode.frames = 1;
	mode.size.x = 320;
	mode.size.y = 240;

	err = HURD_DPORT_USE(kgi_fd2, kgi_set_image_mode(port, 1, mode));
	if (err)
		error(3, err, "kgi_set_image_mode() failed");

	err = HURD_DPORT_USE(kgi_fd2, kgi_check_mode(port));
	if (err)
		error(3, err, "kgi_check_mode() failed");

	err = HURD_DPORT_USE(kgi_fd2, kgi_get_image_mode(port, 1, &mode));
	if (err)
		error(3, err, "kgi_get_image_mode() failed");
	printf("Checked mode: flags=%d virt.x=%d virt.y=%d size.x=%d size.y=%d frames=%d fam=%d bpfa[0]=%d bpfa[1]=%d bpfa[2]=%d bpfa[3]=%d)\n", mode.flags, mode.virt.x, mode.virt.y, mode.size.x, mode.size.y, mode.frames, mode.fam, mode.bpfa[0], mode.bpfa[1], mode.bpfa[2], mode.bpfa[3]);

	err = HURD_DPORT_USE(kgi_fd2, kgi_set_mode(port));
	if (err != EBUSY)
		error(3, err, "kgi_set_mode()");


	err = close(kgi_fd);
	if (err == -1)
		error(2, errno, "Closing kgi node failed");


	err = HURD_DPORT_USE(kgi_fd2, kgi_set_mode(port));
	if (err)
		error(3, err, "kgi_set_mode() failed");

	return 0;
}
