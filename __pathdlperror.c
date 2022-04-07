/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <dlfcn.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __pathdlperror(const char *path, const char *s)
{
	__notice("%s: %s: %s\n", path, s, dlerror());
	if (__getfatal())
		raise(SIGABRT);
}
