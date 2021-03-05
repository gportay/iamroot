/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>

#include <stdio.h>

int __vfprintf(FILE *f, const char *fmt, va_list ap)
{
	int debug;

	debug = strtoul(getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
	if (debug == 0)
		return 0;

	return vfprintf(f, fmt, ap);
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
