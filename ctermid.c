/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_ctermid(char *s)
{
	char *(*sym)(char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "ctermid");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(s);
	if (!ret)
		__pathperror(s, __func__);

	return ret;
}

char *ctermid(char *s)
{
	char *ret;

	ret = next_ctermid(s);
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s(s: '%s') -> '%s'\n", __func__, s, ret);

	return ret;
}
