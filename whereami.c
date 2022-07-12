/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <fcntl.h>

extern ssize_t next_readlinkat(int, const char *, char *, size_t);

ssize_t whereami(char *buf, ssize_t bufsiz)
{
	return next_readlinkat(AT_FDCWD, "/proc/self/cwd", buf, bufsiz);
}
