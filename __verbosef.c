/*
 * Copyright 2021-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <regex.h>

#include "iamroot.h"

typedef struct {
	regex_t re;
#ifndef JIMREGEXP_H
	char jimpad[40];
#endif
} __regex_t;

__attribute__((visibility("hidden")))
int __getdebug()
{
	return strtol(_getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getdebug_fd()
{
	return strtol(_getenv("IAMROOT_DEBUG_FD") ?: "2", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getno_color()
{
	return strtol(_getenv("NO_COLOR") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getcolor()
{
	const int errno_save = errno;

	if (!isatty(__getdebug_fd()))
		return __set_errno(errno_save, 0);

	return __getno_color() == 0;
}

static regex_t *re_ignore;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		dprintf(DEBUG_FILENO, "%s\n", buf);
		return;
	}

	dprintf(DEBUG_FILENO, "%s: %s\n", s, buf);
}

__attribute__((constructor,visibility("hidden")))
void verbosef_init()
{
	static __regex_t regex_ignore;
	const char *ignore;
	int ret;

	ignore = _getenv("IAMROOT_DEBUG_IGNORE");
	if (!ignore)
		ignore = "^$";

	ret = regcomp(&regex_ignore.re, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_ignore.re, ret);
		return;
	}

	re_ignore = &regex_ignore.re;
}

__attribute__((destructor,visibility("hidden")))
void verbosef_fini()
{
	if (!re_ignore)
		return;

	regfree(re_ignore);
	re_ignore = NULL;
}

static int __ignore(const char *func)
{
	int ret = 0;

	if (!re_ignore)
		return 0;

	ret = regexec(re_ignore, func, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

static int __vdverbosef(int fd, int lvl, const char *func, const char *fmt,
			va_list ap)
{
	int debug;
	int color;
	int ret;

	if (lvl != 0 && __ignore(func))
		return 0;

	debug = __getdebug();
	if (debug < lvl || (!__inchroot() && debug < 5))
		return 0;

	color = __getcolor();
	if (color) {
		if (lvl == 0)
			dprintf(fd, "\033[31;1m");
		else
			dprintf(fd, "\033[32;1m");
	}

	ret = dprintf(fd, "%s: ", lvl == 0 ? "Warning" : "Debug");

	if (color)
		dprintf(fd, "\033[0m");

	ret += vdprintf(fd, fmt, ap);
	return ret;
}

__attribute__((visibility("hidden")))
int __verbosef(int lvl, const char *func, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = __vdverbosef(DEBUG_FILENO, lvl, func, fmt, ap);
	va_end(ap);
	return ret;
}
