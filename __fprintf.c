/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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

int __vfprintf(FILE *f, int lvl, const char *fmt, va_list ap)
{
	int debug;
	int ret;

	debug = __debug();
	if (debug < lvl || !inchroot())
		return 0;

	ret = fprintf(f, "%s: ", lvl == 0 ? "Warning" : "Debug");

	if (debug > 2)
		ret += fprintf(f, "%s: pid: %u: ", __libc(), getpid());

	ret += vfprintf(f, fmt, ap);
	return ret;
}

int __fprintf(FILE *f, int lvl, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vfprintf(f, lvl, fmt, ap);
	va_end(ap);
	return ret;
}
