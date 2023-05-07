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
	(void)oldp;
	(void)newp;

	if (__ignored_errno(errno) || __ignored_function(s)) {
#ifdef __FreeBSD__
		__debug("%s: %s: %s: %i\n", oldp, newp, s,
		       errno);
#else
		__debug("%s: %s: %s: %m\n", oldp, newp, s);
#endif
		return;
	}

#ifdef __FreeBSD__
	__note_or_fatal("%s: %s: %s: %i\n", oldp, newp, s,
			errno);
#else
	__note_or_fatal("%s: %s: %s: %m\n", oldp, newp, s);
#endif
}
