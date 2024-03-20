/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

static char *(*sym)();

hidden char *next_get_current_dir_name()
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "get_current_dir_name");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym();
}

char *get_current_dir_name()
{
	char *ret;

	ret = next_get_current_dir_name();
	if (!ret)
		goto exit;

	ret = __striprootdir(ret);

exit:
	__debug("%s() -> '%s'\n", __func__, ret);

	return ret;
}
#endif
