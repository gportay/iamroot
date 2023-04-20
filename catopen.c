/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <nl_types.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
nl_catd next_catopen(const char *path, int flag)
{
	nl_catd (*sym)(const char *, int);
	nl_catd ret;

	sym = dlsym(RTLD_NEXT, "catopen");
	if (!sym)
		return __dl_set_errno(ENOSYS, (nl_catd)-1);

	ret = sym(path, flag);
	if (ret == (nl_catd)-1)
		__pathperror(path, __func__);

	return ret;
}

nl_catd __catopen(const char *path, int flag)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return (nl_catd)-1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return next_catopen(buf, flag);
}

static struct __libc {
	char secure;
} libc;

#define do_catopen(p) __catopen(p, oflag)

/*
 * Stolen and hacked from musl (/src/locale/catopen.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <langinfo.h>
#include <locale.h>

nl_catd catopen(const char *name, int oflag)
{
	nl_catd catd;

	if (strchr(name, '/')) return do_catopen(name);

	char buf[PATH_MAX];
	size_t i;
	const char *path, *lang, *p, *z;
	if (libc.secure || !(path = getenv("NLSPATH"))) {
		errno = ENOENT;
		return (nl_catd)-1;
	}
	lang = oflag ? nl_langinfo(_NL_LOCALE_NAME(LC_MESSAGES)) : getenv("LANG");
	if (!lang) lang = "";
	for (p=path; *p; p=z) {
		i = 0;
		z = __strchrnul(p, ':');
		for (; p<z; p++) {
			const char *v;
			size_t l;
			if (*p!='%') v=p, l=1;
			else switch (*++p) {
			case 'N': v=name; l=strlen(v); break;
			case 'L': v=lang; l=strlen(v); break;
			case 'l': v=lang; l=strcspn(v,"_.@"); break;
			case 't':
				v=__strchrnul(lang,'_');
				if (*v) v++;
				l=strcspn(v,".@");
				break;
			case 'c': v="UTF-8"; l=5; break;
			case '%': v="%"; l=1; break;
			default: v=0;
			}
			if (!v || l >= sizeof buf - i) {
				break;
			}
			memcpy(buf+i, v, l);
			i += l;
		}
		if (!*z && (p<z || !i)) break;
		if (p<z) continue;
		if (*z) z++;
		buf[i] = 0;
		/* Leading : or :: in NLSPATH is same as %N */
		catd = do_catopen(i ? buf : name);
		if (catd != (nl_catd)-1) return catd;
	}
	errno = ENOENT;
	return (nl_catd)-1;
}
