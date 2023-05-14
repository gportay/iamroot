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

__attribute__((visibility("hidden")))
int next_close_range(unsigned int first, unsigned int last, int flags)
{
	int (*sym)(unsigned int, unsigned int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "close_range");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(first, last, flags);
	if (ret == -1)
		__pathperror(NULL, __func__);

	return ret;
}

#ifdef __linux__
struct __close_range {
	int first;
	int last;
	char buf[PATH_MAX];
};

static int __callback(const char *path, const char *filename, void *user)
{
	struct __close_range *data = (struct __close_range *)user;
	int fd;
	(void)path;

	if (!user)
		return __set_errno(EINVAL, -1);

	fd = __strtofd(filename, NULL);
	if (fd == -1)
		return -1;

	if (fd < data->first || fd > data->last)
		return 0;

	if (*data->buf)
		__strncat(data->buf, ", ");
	__strncat(data->buf, filename);

	__notice("%s: %i -> '%s'\n", "close_range", fd, __fpath(fd));

	return 0;
}
#endif

int close_range(unsigned int first, unsigned int last, int flags)
{
#ifdef __linux__
	struct __close_range data = {
		.first = first,
		.last = last,
		.buf = { 0 },
	};
	int err, ret;

	err = __dir_iterate("/proc/self/fd", __callback, &data);
	if (err == -1)
		__strncpy(data.buf, "(error)");

#else
	int ret;

#endif
	ret = next_close_range(first, last, flags);

#ifdef __linux__
	__debug("%s(first: %u <-> '%s', last: %u <-> '%s', flags: 0x%x) -> %i, fds: { %s }\n",
		__func__, first, __fpath(first), last, __fpath2(last), flags,
		ret, data.buf);
#else
	__debug("%s(first: %u <-> '%s', last: %u <-> '%s', flags: 0x%x) -> %i\n",
		__func__, first, __fpath(first), last, __fpath2(last), flags,
		ret);
#endif

	return ret;
}
