/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_fclose(FILE *stream)
{
	int (*sym)(FILE *);
	int ret;

	sym = dlsym(RTLD_NEXT, "fclose");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(stream);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

int fclose(FILE *stream)
{
	const int fd = fileno(stream);
	int ret;
	(void)fd;

	ret = next_fclose(stream);
	__notice("%s: %i -> '%s'\n", __func__, fd, __fpath(fd));

	__debug("%s(...) -> %i\n", __func__, ret);

	return ret;
}
