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
void __pathperror2(const char *oldpath, const char *newpath, const char *s)
{
	const char *oldp = *oldpath ? oldpath : "(empty)";
	const char *newp = *newpath ? newpath : "(empty)";

	if (__ignored_errno(errno) || __ignored_function(s)) {
#ifdef __FreeBSD__
		__debug("%s: %s: %s: %s: %i\n", __getrootdir(), oldp, newp, s,
		       errno);
#else
		__debug("%s: %s: %s: %s: %m\n", __getrootdir(), oldp, newp, s);
#endif
		return;
	}

#ifdef __FreeBSD__
	__note_or_fatal("%s: %s: %s: %s: %i\n", __getrootdir(), oldp, newp, s,
			errno);
#else
	__note_or_fatal("%s: %s: %s: %s: %m\n", __getrootdir(), oldp, newp, s);
#endif
}
