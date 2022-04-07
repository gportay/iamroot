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
void __envperror(const char *name, const char *s)
{
	__notice("%s: %s: %m\n", name, s);
	if (__getfatal())
		raise(SIGABRT);
}
