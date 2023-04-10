/*
 * Copyright 2021-2023 Gaël PORTAY
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

int execlp(const char *file, const char *arg, ...)
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

		__debug("%s(file: '%s', arg: '%s', ...)\n", __func__, file,
			arg);

#ifdef __linux__
		return execvpe(file, argv, __environ);
#else
		return execvp(file, argv);
#endif
	}

	return __set_errno(EINVAL, -1);
}
