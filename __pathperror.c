/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __pathperror(const char *path, const char *s)
{
	const char *p = path && *path ? path : "(empty)";
	(void)p;
	(void)s;

	if (__ignored_errno(errno) || __ignored_function(s)) {
#if defined __FreeBSD__ || defined __OpenBSD__
		__debug("%s: %s: %i\n", p, s, errno);
#else
		__debug("%s: %s: %m\n", p, s);
#endif
		return;
	}

#if defined __FreeBSD__ || defined __OpenBSD__
	__note_or_fatal("%s: %s: %i\n", p, s, errno);
#else
	__note_or_fatal("%s: %s: %m\n", p, s);
#endif
}
