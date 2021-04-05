/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdarg.h>

extern int __vfverbosef(FILE *, const char *, const char *, va_list);

__attribute__((visibility("hidden")))
int __verbosef(const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vfverbosef(stderr, func, fmt, ap);
	va_end(ap);
	return ret;
}
