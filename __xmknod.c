/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "iamroot.h"

extern int __xmknodat(int, int, const char *, mode_t, dev_t *);

int __xmknod(int ver, const char *path, mode_t mode, dev_t *dev)
{
	__debug("%s(path: '%s', mode: 0%03o)\n", __func__, path, mode);

	return __xmknodat(ver, AT_FDCWD, path, mode, dev);
}
#endif
