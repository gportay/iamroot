/*
 * Copyright 2020-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

int chroot(const char *path)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	/* Not forwarding function */
	ret = __setrootdir(buf);

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
