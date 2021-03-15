/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stddef.h>
#include <fcntl.h>

#include "path_resolution.h"

extern char *path_resolutionat(const char *path, char *buf, size_t bufsize,
			       int flags);

char *path_resolution(const char *path, char *buf, size_t bufsize, int flags)
{
	return path_resolutionat(path, buf, bufsize, flags);
}
