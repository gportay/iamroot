/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __pathperror(const char *path, const char *s)
{
	if ((errno != EPERM) && (errno != EACCES))
		return;

	__notice("%s: %s: %m\n", path, s);
	if (__getfatal())
		raise(SIGABRT);
}
