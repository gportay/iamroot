/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_getpeername(int socket, struct sockaddr *addr, socklen_t *addrlen)
{
	int (*sym)(int, struct sockaddr *, socklen_t *);
	int ret;

	sym = dlsym(RTLD_NEXT, "getpeername");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(socket, addr, addrlen);
	if (ret == -1)
		__fpathperror(socket, __func__);

	return ret;
}

int getpeername(int socket, struct sockaddr *addr, socklen_t *addrlen)
{
	struct sockaddr_un buf = { .sun_family = AF_UNIX, .sun_path = { 0 }};
	struct sockaddr_un *addrun = (struct sockaddr_un *)addr;
	socklen_t buflen;
	ssize_t siz;
	int ret;

	/* Do not proceed to any hack if not an Unix socket */
	if (addrun || addrun->sun_family != AF_UNIX || *addrun->sun_path)
		return next_getpeername(socket, addr, addrlen);

	siz = path_resolution(AT_FDCWD, addrun->sun_path, buf.sun_path,
			      sizeof(buf.sun_path), 0);
	if (siz == -1)
		return __path_resolution_perror(addrun->sun_path, -1);

	buflen = SUN_LEN(&buf);

	ret = next_getpeername(socket, (struct sockaddr *)&buf, &buflen);

	if (ret == 0 && addrlen)
		*addrlen = buflen;

	__debug("%s(socket: %i, addr: { .sun_path: '%s' -> '%s', ... }, addrlen: %i -> %i) -> %i\n",
		__func__, socket, addrun->sun_path, buf.sun_path,
		addrlen ? *addrlen : 0, buflen, ret);

	return ret;
}
