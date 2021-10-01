/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int __debug()
{
	return strtol(getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
}

int __vfprintf(FILE *f, const char *fmt, va_list ap)
{
#ifdef __GLIBC__
	const char *libc = "glibc";
#else
	const char *libc = "libc";
#endif
	int debug;
	int ret;

	debug = strtoul(getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
	if (debug < 1 || !inchroot())
		return 0;

	ret = fprintf(stderr, "Debug: ");

	if (debug > 2)
		ret += fprintf(stderr, "%s: pid: %u: ", libc, getpid());

	ret += vfprintf(f, fmt, ap);
	return ret;
}

int __fprintf(FILE *f, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vfprintf(f, fmt, ap);
	va_end(ap);
	return ret;
}
