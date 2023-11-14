/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <stdlib.h>

#include "iamroot.h"

static int (*sym)(int, char *, size_t);

__attribute__((visibility("hidden")))
int next_ptsname_r(int fd, char *buf, size_t bufsiz)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "ptsname_r");

	if (!sym)
		return __set_errno(ENOSYS, -1);

	return sym(fd, buf, bufsiz);
}

int ptsname_r(int fd, char *buf, size_t bufsiz)
{
	char *s;
	int ret;

	ret = next_ptsname_r(fd, buf, bufsiz);
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

int __ptsname_r_chk(int fd, char *buf, size_t bufsiz, size_t buflen)
{
	(void)buflen;

	__debug("%s(fd: %i, buf: %p, bufsiz: %zu, buflen: %zu)\n", __func__,
		fd, buf, bufsiz, buflen);

	/* Forward to another function */
	return ptsname_r(fd, buf, bufsiz);
}
