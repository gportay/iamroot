/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "iamroot.h"

void __pathperror2(const char *oldpath, const char *newpath, const char *s)
{
	if ((errno != EPERM) && (errno != EACCES))
		return;

	__notice("%s: %s: %s: %m\n", oldpath, newpath, s);
	if (__getfatal())
		raise(SIGABRT);
}
