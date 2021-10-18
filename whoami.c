/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>

extern ssize_t next_geteuid();

uid_t whoami()
{
	return next_geteuid();
}
