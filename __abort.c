/*
 * Copyright 2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void __abort()
{
	raise(SIGABRT);
}
