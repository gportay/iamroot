/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <unistd.h>

#include "iamroot.h"

int execl(const char *path, const char *arg, ...)
{
	va_list ap;
	int argc;

	argc = 1;
	va_start(ap, arg);
	while (va_arg(ap, const char *))
		argc++;
	va_end(ap);

	if (argc < ARG_MAX) {
		char *argv[argc+1]; /* NULL-terminated */
		int i;

		argv[0] = (char *)arg;
		va_start(ap, arg);
		for (i = 1; i <= argc; i++)
			argv[i] = va_arg(ap, char *);
		va_end(ap);

		__debug("%s(path: '%s', arg: '%s', ...)\n", __func__, path,
			arg);

		/* Forward to another function */
		return execve(path, argv, __environ);
	}

	return __set_errno(E2BIG, -1);
}
