/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fileno(FILE *stream)
{
	int (*sym)(FILE *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fileno");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(stream);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int fileno(FILE *stream)
{
	__debug("%s(stream: %p)\n", __func__, stream);

	return next_fileno(stream);
}
