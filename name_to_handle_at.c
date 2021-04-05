/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_name_to_handle_at(int fd, const char *path, struct file_handle *handle,
			   int *mount_id, int flags)
{
	int (*sym)(int, const char *, struct file_handle *, int *, int);

	sym = dlsym(RTLD_NEXT, "name_to_handle_at");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, handle, mount_id, flags);
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

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return next_name_to_handle_at(fd, real_path, handle, mount_id, flags);
}
