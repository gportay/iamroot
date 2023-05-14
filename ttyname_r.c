/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_ttyname_r(int fd, char *buf, size_t bufsize)
{
	int (*sym)(int, char *, size_t);
	int ret;

	sym = dlsym(RTLD_NEXT, "ttyname_r");
	if (!sym)
		return __set_errno(ENOSYS, -1);

	ret = sym(fd, buf, bufsize);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}

int ttyname_r(int fd, char *buf, size_t bufsize)
{
	char *s;
	int ret;

	ret = next_ttyname_r(fd, buf, bufsize);
	if (ret == -1)
		goto exit;

	s = __striprootdir(buf);
	if (!s)
		ret = -1;

exit:
	__debug("%s(fd: %i <-> '%s', ...) -> %i\n", __func__, fd, __fpath(fd),
		ret);

	return ret;
}
