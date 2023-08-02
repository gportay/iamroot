/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static char *(*sym)(char *);

__attribute__((visibility("hidden")))
char *next_getwd(char *buf)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "getwd");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(buf);
}

char *getwd(char *buf)
{
	char *ret;

	ret = next_getwd(buf);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(buf: %p) -> '%s'\n", __func__, buf, ret);

	return ret;
}

char *__getwd_chk(char *buf, size_t buflen)
{
	(void)buflen;

	__debug("%s(buf: %p, buflen: %zu)\n", __func__, buf, buflen);

	/* Forward to another function */
	return getwd(buf);
}
