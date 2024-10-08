/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <errno.h>
#include <sys/types.h>

#include <dlfcn.h>

#include "iamroot.h"

static int (*sym)(const void *, Dl_info *, void **, int);

hidden int next_dladdr1(const void *addr, Dl_info *info, void **extra_info,
			int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dladdr1");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(addr, info, extra_info, flags);
}

int dladdr1(const void *addr, Dl_info *info, void **extra_info, int flags)
{
	int ret;

	ret = next_dladdr1(addr, info, extra_info, flags);
	if (ret == 0)
		goto exit;

	if (info->dli_fname)
		info->dli_fname = __skiprootdir(info->dli_fname);

exit:
	__debug("%s(addr: %p, info: { .dli_fname: '%s' }, ...) -> %i\n",
		__func__, addr, info->dli_fname, ret);

	return ret;
}
#endif
