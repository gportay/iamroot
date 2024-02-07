/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <climits>
#include <fcntl.h>

extern "C" {

static ssize_t __path_resolution(int dfd, const uint8_t *path, size_t pathsiz,
				 char *buf, size_t bufsiz, int atflags)
{
	ssize_t path_resolution(int, const char *, char *, size_t, int);
	char tmp[PATH_MAX+1]; /* NULL-terminated */
	size_t len;

	len = pathsiz;
	if (!path || len == 0 || len >= sizeof(tmp)) {
		errno = EINVAL;
		return -1;
	}

	memcpy(tmp, path, len);
	tmp[len] = 0; /* ensure NULL-terminated */
	return path_resolution(dfd, tmp, buf, bufsiz, atflags);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t datasiz)
{
	char buf[PATH_MAX];

	__path_resolution(AT_FDCWD, data, datasiz, buf, sizeof(buf), 0);

	return 0;
}

}
