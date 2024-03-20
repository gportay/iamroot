/*
 * Copyright 2023-2024 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <signal.h>

#include "iamroot.h"

static int (*sym)(pid_t, int);

hidden int next_kill(pid_t pid, int sig)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "kill");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(pid, sig);
}

int kill(pid_t pid, int sig)
{
	int ret;

	ret = next_kill(pid, sig);

	/* Force ignoring EPERM error */
	if ((ret == -1) && (errno == EPERM)) {
		__warning("%s: ignore non killed pid %u\n", strsignal(sig),
			  pid);
		ret = __set_errno(0, 0);
	}

	__debug("%s(pid: %u, sig: %i) -> %i\n", __func__, pid, sig, ret);

	return ret;
}
