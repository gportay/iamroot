/*
 * Copyright 2021-2022 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int __getdebug()
{
	return strtol(getenv("IAMROOT_DEBUG") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getdebug_fd()
{
	return strtol(getenv("IAMROOT_DEBUG_FD") ?: "2", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getno_color()
{
	return strtol(getenv("NO_COLOR") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int __getcolor()
{
	int save_errno;
	
	save_errno = errno;
	if (!isatty(__getdebug_fd())) {
		errno = save_errno;
		return 0;
	}

	return __getno_color() == 0;
}

static regex_t *re;

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

__attribute__((optimize("O0")))
__attribute__((constructor,visibility("hidden")))
void verbosef_init()
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

	__info("IAMROOT_DEBUG_IGNORE=%s\n", ignore);
	re = &regex;
}

__attribute__((destructor,visibility("hidden")))
void verbosef_fini()
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

static int __vdverbosef(int fd, int lvl, const char *func, const char *fmt,
			va_list ap)
{
	int debug;
	int color;
	int ret;

	if (lvl != 0 && ignore(func))
		return 0;

	debug = __getdebug();
	if (debug < lvl || (!inchroot() && debug < 5))
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

	if (debug > 2)
		ret += dprintf(fd, "%s: %s: pid: %u: ", __libc(), __arch(),
			       getpid());

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
