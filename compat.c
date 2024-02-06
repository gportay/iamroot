/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __linux__
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <dlfcn.h>

#include "iamroot.h"

long int __isoc23_strtol(const char *nptr, char **endptr, int base)
{
	static long int (*sym)(const char *, char **, int);

	if (!sym)
		sym = dlsym(RTLD_NEXT, __func__);

	if (!sym)
		sym = dlsym(RTLD_NEXT, "strtol");

	if (!sym)
		return __set_errno(ENOSYS, -1L);

	return sym(nptr, endptr, base);
}

unsigned long int __isoc23_strtoul(const char *nptr, char **endptr, int base)
{
	static unsigned long int (*sym)(const char *, char **, int);

	if (!sym)
		sym = dlsym(RTLD_NEXT, __func__);

	if (!sym)
		sym = dlsym(RTLD_NEXT, "strtoul");

	if (!sym)
		return __set_errno(ENOSYS, -1UL);

	return sym(nptr, endptr, base);
}

int __isoc23_sscanf(const char *s, const char *format, ...)
{
	static int (*sym)(const char *, const char *, va_list);
	va_list ap;
	int ret;

	if (!sym)
		sym = dlsym(RTLD_NEXT, "__isoc23_vsscanf");

	if (!sym)
		sym = dlsym(RTLD_NEXT, "vsscanf");

	if (!sym)
		return __set_errno(ENOSYS, -1);

	va_start(ap, format);
	ret = sym(s, format, ap);
	va_end(ap);

	return ret;
}
#endif
