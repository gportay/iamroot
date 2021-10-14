/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <regex.h>

#include "iamroot.h"

extern int __vfprintf(FILE *, int, const char *, va_list);
extern int __fprintf(FILE *, int, const char *, ...) __attribute__ ((format(printf,3,4)));

static regex_t *re;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		fprintf(stderr, "%s\n", buf);
		return;
	}

	fprintf(stderr, "%s: %s\n", s, buf);
}

__attribute__((constructor))
void fverbosef_init()
{
	static regex_t regex;
#ifndef JIMREGEXP_H
	__attribute__((unused)) static char jimpad[40];
#endif
	const char *ignore;
	int ret;

	if (re)
		return;

	ignore = getenv("IAMROOT_DEBUG_IGNORE");
	if (!ignore)
		ignore = "^$";

	ret = regcomp(&regex, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex, ret);
		return;
	}

	__verbose("IAMROOT_DEBUG_IGNORE=%s\n", ignore);
	re = &regex;
}

__attribute__((destructor))
void fverbosef_fini()
{
	if (!re)
		return;

	regfree(re);
	re = NULL;
}

static int ignore(const char *func)
{
	int ret = 0;

	if (!re)
		return 0;

	ret = regexec(re, func, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re, ret);
		return 0;
	}

	return !ret;
}

__attribute__((visibility("hidden")))
int __vfverbosef(FILE *f, int lvl, const char *func, const char *fmt,
		 va_list ap)
{
	if (ignore(func))
		return 0;

	return __vfprintf(f, lvl, fmt, ap);
}

__attribute__((visibility("hidden")))
int __fverbosef(int lvl, const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vfverbosef(stderr, lvl, func, fmt, ap);
	va_end(ap);
	return ret;
}
