/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <linux/limits.h>

#include <unistd.h>

int execle(const char *path, const char *arg, ...)
{
	va_list ap;
	int argc;

	if (getenv("IAMROOT_DEBUG"))
		fprintf(stderr, "%s(path: '%s', arg: '%s'...)\n",
				__func__, path, arg);

	argc = 1;
	va_start(ap, arg);
	while (va_arg(ap, const char *))
		argc++;
	va_end(ap);

	if (argc < ARG_MAX) {
		char **envp, *argv[argc + 1];
		int i;

		argv[0] = (char *)arg;
		va_start(ap, arg);
		for (i = 1; i <= argc; i++)
			argv[i] = va_arg(ap, char *);
		envp = va_arg(ap, char **);
		va_end(ap);

		return execve(path, argv, envp);
	}

	errno = EINVAL;
	return -1;
}
