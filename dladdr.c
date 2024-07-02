/*
 * Copyright 2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <sys/types.h>

#include <dlfcn.h>

#include "iamroot.h"

static int (*sym)(const void *, Dl_info *);

hidden int next_dladdr(const void *addr, Dl_info *info)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dladdr");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(addr, info);
}

int dladdr(const void *addr, Dl_info *info)
{
	int ret;

	ret = next_dladdr(addr, info);
	if (ret == 0)
		goto exit;

	if (info->dli_fname)
		info->dli_fname = __skiprootdir(info->dli_fname);

exit:
	__debug("%s(addr: %p, info: { .dli_fname: '%s' }) -> %i\n", __func__,
		addr, info->dli_fname ?: "nuxx", ret);

	return ret;
}
