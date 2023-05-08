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
	(void)iov;
	(void)niov;
	(void)flags;

	__debug("%s(...)\n", __func__);

	return 0;
}
#endif
