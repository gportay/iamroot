/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>

#include <unistd.h>

#include "iamroot.h"

#ifdef __OpenBSD__
__attribute__((visibility("hidden")))
int next_closefrom(int fd)
{
	int (*sym)(int);
	int ret;

	sym = dlsym(RTLD_NEXT, "closefrom");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(fd);
	if (ret == -1)
		__fpathperror(fd, __func__);

	return ret;
}
#else
__attribute__((visibility("hidden")))
void next_closefrom(int fd)
{
	void (*sym)(int);

	sym = dlsym(RTLD_NEXT, "closefrom");
	if (!sym)
		return;

	sym(fd);
}
#endif

struct __closefrom {
	int fd;
	char buf[PATH_MAX];
};

static int __callback(const char *path, const char *filename, void *user)
{
	struct __closefrom *data = (struct __closefrom *)user;
	int fd;
	(void)path;

	if (!user)
		return __set_errno(EINVAL, -1);

	fd = __strtofd(filename, NULL);
	if (fd == -1)
		return -1;

	if (fd < data->fd)
		return 0;

	if (*data->buf)
		__strncat(data->buf, ", ");
	__strncat(data->buf, filename);

	__notice("%s: %i -> '%s'\n", "closefrom", fd, __fpath(fd));

	return 0;
}

#ifdef __OpenBSD__
int closefrom(int fd)
#else
void closefrom(int fd)
#endif
{
	struct __closefrom data = {
		.fd = fd,
		.buf = { 0 },
	};
	int err;

	err = __dir_iterate("/proc/self/fd", __callback, &data);
	if (err == -1)
		__strncpy(data.buf, "(error)");

	__debug("%s(fd: %i <-> '%s') fds: { %s }\n", __func__, fd, __fpath(fd),
		data.buf);

#ifdef __OpenBSD__
	return next_closefrom(fd);
#else
	next_closefrom(fd);
#endif
}
