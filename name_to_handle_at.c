/*
 * Copyright 2021-2022 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int next_name_to_handle_at(int dfd, const char *path,
			   struct file_handle *handle, int *mount_id,
			   int flags)
{
	int (*sym)(int, const char *, struct file_handle *, int *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "name_to_handle_at");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(dfd, path, handle, mount_id, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int name_to_handle_at(int dfd, const char *path, struct file_handle *handle,
		      int *mount_id, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ..., flags: 0x%x)\n", __func__, path,
		buf, flags);

	__remove_at_empty_path_if_needed(buf, flags);
	return next_name_to_handle_at(dfd, buf, handle, mount_id, flags);
}
#endif
