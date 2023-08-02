/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef __NetBSD__
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "iamroot.h"

static int (*sym)(int, struct sockaddr *, socklen_t *, const sigset_t *, int);

__attribute__((visibility("hidden")))
int next_paccept(int socket, struct sockaddr *addr, socklen_t *addrlen,
		 const sigset_t *sigmask, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "paccept");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(socket, addr, addrlen, sigmask, flags);
}

int paccept(int socket, struct sockaddr *addr, socklen_t *addrlen,
	    const sigset_t *sigmask, int flags)

{
	struct sockaddr_un buf = { .sun_family = AF_UNIX, .sun_path = { 0 }};
	struct sockaddr_un *addrun = (struct sockaddr_un *)addr;
	socklen_t buflen;
	int ret = -1;
	ssize_t siz;

	/* Do not proceed to any hack if not an Unix socket */
	if (!addrun || addrun->sun_family != AF_UNIX || !*addrun->sun_path)
		return next_paccept(socket, addr, addrlen, sigmask, flags);

	siz = path_resolution(AT_FDCWD, addrun->sun_path, buf.sun_path,
			      sizeof(buf.sun_path), 0);
	if (siz == -1)
		goto exit;

	buflen = SUN_LEN(&buf);

	ret = next_paccept(socket, (struct sockaddr *)&buf, &buflen, sigmask, 
			   flags);

	if (ret == 0 && addrlen)
		*addrlen = buflen;

exit:
	__debug("%s(socket: %i, addr: { .sun_path: '%s' -> '%s', ... }, addrlen: %i -> %i, ..., flags: 0x%x) -> %i\n",
		__func__, socket, addrun->sun_path, buf.sun_path,
		addrlen ? *addrlen : 0, addrlen ? buflen : 0, flags, ret);

	return ret;
}
#endif
