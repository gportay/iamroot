/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stddef.h>
#include <fcntl.h>

#include "path_resolutionat.h"

extern const char *fpath_resolutionat(int fd, const char *path, char *buf,
				     size_t bufsize, int flags);


const char *path_resolutionat(const char *path, char *buf, size_t bufsize,
			      int flags)
{
	return fpath_resolutionat(AT_FDCWD, path, buf, bufsize, flags);
}
