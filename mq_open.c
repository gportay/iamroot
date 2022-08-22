/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
mqd_t next_mq_open(const char *path, int oflags, mode_t mode,
		   struct mq_attr *attr)
{
	mqd_t (*sym)(const char *, int, ...);
	mqd_t ret;

	sym = dlsym(RTLD_NEXT, "mq_open");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return (mqd_t)-1;
	}

	ret = sym(path, oflags, mode, attr);
	if (ret == (mqd_t)-1)
		__pathperror(path, __func__);

	return ret;
}

mqd_t mq_open(const char *path, int oflags, ...)
{
	struct mq_attr *attr = NULL;
	char buf[PATH_MAX];
	mode_t mode = 0;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return (mqd_t)-1;
	}

	if (oflags & O_CREAT) {
		va_list ap;
		va_start(ap, oflags);
		mode = va_arg(ap, mode_t);
		attr = va_arg(ap, struct mq_attr *);
		va_end(ap);
	}

	__debug("%s(path: '%s' -> '%s', oflags: 0%o, mode: 0%03o, ...)\n",
		__func__, path, buf, oflags, mode);

	if (oflags & O_CREAT)
		__warn_if_insuffisant_user_mode(buf, mode);

	return next_mq_open(buf, oflags, mode, attr);
}
