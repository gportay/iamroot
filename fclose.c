/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <stdio.h>

#include "iamroot.h"

static int (*sym)(FILE *);

hidden
int next_fclose(FILE *stream)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "fclose");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(stream);
}

int fclose(FILE *stream)
{
	const int fd = fileno(stream);
	int ret;
	(void)fd;

	ret = next_fclose(stream);
	__info("%s: %i -> '%s'\n", __func__, fd, __fpath(fd));

	__debug("%s(...) -> %i\n", __func__, ret);

	return ret;
}
