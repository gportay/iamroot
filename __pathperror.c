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
	if ((errno != EPERM) && (errno != EACCES) && (errno != ENOSYS)) {
		__info("%s: %s: %s: %m\n", getrootdir(), path, s);
		return;
	}

	__note_or_fatal("%s: %s: %s: %m\n", getrootdir(), path, s);
}
