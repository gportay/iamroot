/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_name_to_handle_at(int fd, const char *path, struct file_handle *handle,
			   int *mount_id, int flags)
{
	int (*sym)(int, const char *, struct file_handle *, int *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "name_to_handle_at");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, handle, mount_id, flags);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int name_to_handle_at(int fd, const char *path, struct file_handle *handle,
		      int *mount_id, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return next_name_to_handle_at(fd, real_path, handle, mount_id, flags);
}
