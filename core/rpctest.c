#include <fcntl.h>
#include <hurd/fd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kgiUser.h"

int main(int argc, char *argv[])
{
	kgi_image_mode_t mode;
	int kgi_fd;
	int err;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <kgi node>\n", argv[0]);
		exit(1);
	}

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

	err = HURD_DPORT_USE(kgi_fd, kgi_unset_mode(port));
	if (err)
		error(3, err, "kgi_unset_mode() failed");

	err = close(kgi_fd);
	if (err == -1)
		error(2, errno, "Closing kgi node failed");
	
	return 0;
}
