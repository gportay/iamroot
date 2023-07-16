/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/auxv.h>

#include "iamroot.h"

static unsigned long (*sym)(unsigned long);

__attribute__((visibility("hidden")))
unsigned long next_getauxval(unsigned long type)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "getauxval");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(type);
}

unsigned long getauxval(unsigned long type)
{
	unsigned long ret;

	/*
	 * Secure-execution mode
	 *
	 * For security reasons, if the dynamic linker determines that a binary
	 * should be run in secure-execution mode, the effects of some
	 * environment variables are voided or modified, and furthermore those
	 * environment variables are stripped from the environment, so that the
	 * program does not even see the definitions. Some of these environment
	 * variables affect the operation of the dynamic linker itself, and are
	 * described below. Other environment variables treated in this way
	 * include: GCONV_PATH, GETCONF_DIR, HOSTALIASES, LOCALDOMAIN, LOCPATH,
	 * MALLOC_TRACE, NIS_PATH, NLSPATH, RESOLV_HOST_CONF, RES_OPTIONS,
	 * TMPDIR, and TZDIR.
	 *
	 * A binary is executed in secure-execution mode if the AT_SECURE entry
	 * in the auxiliary vector (see getauxval(3)) has a nonzero value. This
	 * entry may have a non‐zero value for various reasons, including:
	 *
	 * •  The process's real and effective user IDs differ, or the real and
	 * effective group IDs differ. This typically occurs as a result of
	 * executing a set-user-ID or set-group-ID program.
	 *
	 * •  A process with a non-root user ID executed a binary that
	 * conferred capabilities to the process.
	 *
	 * •  A nonzero value may have been set by a Linux Security Module.
	 */
	if (type == AT_SECURE) {
		uid_t ruid, euid, suid;
		gid_t rgid, egid, sgid;
		int err;

		err = getresuid(&ruid, &euid, &suid);
		if (err == -1)
			return -1;
	
		err = getresgid(&rgid, &egid, &sgid);
		if (err == -1)
			return -1;

		ret = ruid != euid || rgid != egid;
		goto exit;
	}

	ret = next_getauxval(type);

exit:
	__debug("%s(type: %lu) -> %lu\n", __func__, type, ret);

	return ret;
}
