/*
 * Copyright 2020-2021,2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include <sys/mount.h>

#include "iamroot.h"

int umount2(const char *target, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;
	(void)target;
	(void)flags;

	siz = path_resolution2(AT_FDCWD, target, buf, sizeof(buf),
			     AT_SYMLINK_NOFOLLOW, PATH_RESOLUTION_NOWALKALONG);
	if (siz == -1)
		goto exit;

	__unset_path_resolution(buf);

exit:
	/* Not forwarding function */
	ret = 0;

	__debug("%s(target: '%s' -> '%s', flags: 0x%x) -> %i\n", __func__,
		target, buf, flags, ret);

	return ret;
}
