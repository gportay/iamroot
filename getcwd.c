/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static char *(*sym)(char *, size_t);

hidden
char *next_getcwd(char *buf, size_t size)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "getcwd");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(buf, size);
}

char *getcwd(char *buf, size_t size)
{
	char *ret;

	ret = next_getcwd(buf, size);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(buf: %p, size: %zu) -> '%s'\n", __func__, buf, size, ret);

	return ret;
}

char *__getcwd_chk(char *buf, size_t size, size_t buflen)
{
	(void)buflen;

	__debug("%s(buf: %p, size: %zu, buflen: %zu)\n", __func__, buf, size,
		buflen);

	/* Forward to another function */
	return getcwd(buf, size);
}
