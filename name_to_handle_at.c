/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

static int (*sym)(int, const char *, struct file_handle *, int *, int);

__attribute__((visibility("hidden")))
int next_name_to_handle_at(int dfd, const char *path,
			   struct file_handle *handle, int *mount_id,
			   int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "name_to_handle_at");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(dfd, path, handle, mount_id, atflags);
}

int name_to_handle_at(int dfd, const char *path, struct file_handle *handle,
		      int *mount_id, int atflags)
{
	const int oldatflags = atflags;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	if (!(atflags & AT_SYMLINK_FOLLOW))
		atflags |= AT_SYMLINK_NOFOLLOW;

	siz = path_resolution(dfd, path, buf, sizeof(buf), atflags);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_name_to_handle_at(dfd, buf, handle, mount_id, oldatflags);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., atflags: 0x%x -> 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, oldatflags, atflags,
		ret);

	return ret;
}
#endif
