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
static int (*sym)(int);

__attribute__((visibility("hidden")))
int next_closefrom(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "closefrom");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(fd);
}
#else
static void (*sym)(int);

__attribute__((visibility("hidden")))
void next_closefrom(int fd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "closefrom");

	if (!sym)
		return;

	sym(fd);
}
#endif

#ifdef __linux__
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
#endif

#ifdef __OpenBSD__
int closefrom(int fd)
#else
void closefrom(int fd)
#endif
{
#ifdef __linux__
	struct __closefrom data = {
		.fd = fd,
		.buf = { 0 },
	};
	int err;
#endif
#ifdef __OpenBSD__
	int ret;
#endif

#ifdef __linux__
	err = __dir_iterate("/proc/self/fd", __callback, &data);
	if (err == -1)
		__strncpy(data.buf, "(error)");
#endif

#ifdef __OpenBSD__
	ret = next_closefrom(fd);

	__debug("%s(fd: %i <-> '%s') -> %i, fds: { %s }\n", __func__, fd,
		__fpath(fd), ret, data.buf);

	return ret;
#else
	next_closefrom(fd);

#ifdef __linux__
	__debug("%s(fd: %i <-> '%s') fds: { %s }\n", __func__, fd, __fpath(fd),
		data.buf);
#else
	__debug("%s(fd: %i <-> '%s')\n", __func__, fd, __fpath(fd));
#endif
#endif
}
