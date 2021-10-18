/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <stdarg.h>

extern int __vfverbosef(FILE *, int, const char *, const char *, va_list);

__attribute__((visibility("hidden")))
int __verbosef(int lvl, const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vfverbosef(stderr, lvl, func, fmt, ap);
	va_end(ap);
	return ret;
}
