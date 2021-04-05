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

extern int __vfprintf(FILE *, const char *, va_list);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

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
	char *verbose;
	int ret;

	if (re)
		return;

	verbose = getenv("IAMROOT_VERBOSE");
	if (!verbose)
		verbose = "^chroot$";

	ret = regcomp(&regex, verbose, REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex, ret);
		return;
	}

	__fprintf(stderr, "IAMROOT_VERBOSE=%s\n", verbose);
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

static int verbose(const char *func)
{
	int ret = 0;

	if (!re)
		return 0;

	ret = regexec(re, func, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regcomp", re, ret);
		return 0;
	}

	return !ret;
}

__attribute__((visibility("hidden")))
int __vfverbosef(FILE *f, const char *func, const char *fmt, va_list ap) 
{
	if (!verbose(func))
		return 0;

	return __vfprintf(f, fmt, ap);
}

__attribute__((visibility("hidden")))
int __fverbosef(const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vfverbosef(stderr, func, fmt, ap);
	va_end(ap);
	return ret;
}
