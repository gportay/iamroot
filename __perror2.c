/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "iamroot.h"

void __perror2(const char *oldpath, const char *newpath, const char *s)
{
	if ((errno != EPERM) && (errno != EACCES))
		return;

	__warning("%s: %s: %s: %m\n", oldpath, newpath, s);
	if (__fatal())
		raise(SIGABRT);
}
