/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __envperror(const char *name, const char *s)
{
	const char *n = *name ? name : "(empty)";

#ifdef __FreeBSD__
	__note_or_fatal("%s: %s: %i\n", n, s, errno);
#else
	__note_or_fatal("%s: %s: %m\n", n, s);
#endif
}
