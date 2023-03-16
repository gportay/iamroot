/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __pathdlperror(const char *path, const char *s)
{
	const char *p = *path ? path : "(empty)";

	__note_or_fatal("%s: %s: %s\n", p, s, dlerror());
}
