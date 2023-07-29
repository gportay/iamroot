/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include <dlfcn.h>

#include "iamroot.h"

static void *(*sym)(const char *, int);

__attribute__((visibility("hidden")))
void *next_dlopen(const char *path, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "dlopen");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, NULL);

	return sym(path, flags);
}

void *dlopen(const char *path, int flags)
{
	char buf[PATH_MAX];
	void *ret = NULL;
	ssize_t siz;
	int err;

	/*
	 * According to dlopen(3):
	 *
	 * If filename is NULL, then the returned handle is for the main
	 * program.
	 */
        /* Do not proceed to any hack if not in chroot */
	if (!path || !__inchroot())
		return next_dlopen(path, flags);

	/*
	 * If filename contains a slash ("/"), then it is interpreted as a
	 * (relative or absolute) pathname.
	 */
	if (strchr(path, '/')) {
		siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
		if (siz == -1)
			goto exit;
	/*
	 * Otherwise, the dynamic linker searches for the object as follows
	 * (see ld.so(8) for further details).
	 */
	} else {
		siz = __path_access(path, F_OK, __deflib(), buf, sizeof(buf));
		if (siz == -1)
			goto exit;
	}

	/*
	 * If the object specified by filename has dependencies on other shared
	 * objects, then these are also automatically loaded by the dynamic
	 * linker using the same rules. (This process may occur recursively, if
	 * those objects in turn have dependencies, and so on.)
	 */
	 /* Bypass the libdl.so loading of the DT_NEEDED shared objects. */
	err = __dlopen_needed(buf);
	if (err == -1)
		goto exit;

	ret = next_dlopen(buf, flags);

exit:
	__debug("%s(path: '%s' -> '%s', flags: 0x%x) -> %p\n", __func__, path,
		buf, flags, ret);

	return ret;
}
