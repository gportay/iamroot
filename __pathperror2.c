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
	const char *oldp = oldpath && *oldpath ? oldpath : "(empty)";
	const char *newp = newpath && *newpath ? newpath : "(empty)";
	(void)oldp;
	(void)newp;
	(void)s;

	if (__ignored_errno(errno)) {
#if defined __FreeBSD__ || defined __OpenBSD__
		__debug("%s: %s: %s: %i\n", oldp, newp, s,
		       errno);
#else
		__debug("%s: %s: %s: %m\n", oldp, newp, s);
#endif
		return;
	}

#if defined __FreeBSD__ || defined __OpenBSD__
	__note_or_fatal("%s: %s: %s: %i\n", oldp, newp, s,
			errno);
#else
	__note_or_fatal("%s: %s: %s: %m\n", oldp, newp, s);
#endif
}
