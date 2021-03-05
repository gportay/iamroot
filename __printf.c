/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>

#include <stdio.h>

extern int __vfprintf(FILE *, const char *, va_list);

int __vprintf(const char *fmt, va_list ap)
{
	return vfprintf(stdout, fmt, ap);
}

int __printf(const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vprintf(fmt, ap);
	va_end(ap);
	return ret;
}
