/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __pathperror(const char *path, const char *s)
{
	if (__ignored_errno(errno)) {
#ifdef __FreeBSD__
		__info("%s: %s: %s: %i\n", getrootdir(), path, s, errno);
#else
		__info("%s: %s: %s: %m\n", getrootdir(), path, s);
#endif
		return;
	}

#ifdef __FreeBSD__
	__note_or_fatal("%s: %s: %s: %i\n", getrootdir(), path, s, errno);
#else
	__note_or_fatal("%s: %s: %s: %m\n", getrootdir(), path, s);
#endif
}
