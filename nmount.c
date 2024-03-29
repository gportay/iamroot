/*
 * Copyright 2022-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#if defined __FreeBSD__ || defined __OpenBSD__
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/uio.h>

#include "iamroot.h"

int nmount(struct iovec *iov, u_int niov, int flags)
{
	int ret;
	(void)iov;
	(void)niov;
	(void)flags;

	/* Not forwarding function */
	ret = 0;

	__debug("%s(..., flags: 0x%x) -> %i\n", __func__, flags, ret);

	return ret;
}
#endif
