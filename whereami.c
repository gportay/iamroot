/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <unistd.h>

extern ssize_t next_readlink(const char *, char *, size_t);

ssize_t whereami(char *buf, ssize_t bufsiz)
{
	return next_readlink("/proc/self/cwd", buf, bufsiz);
}
