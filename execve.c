/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <byteswap.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...);
extern int next_open(const char *, int, mode_t);
extern int next_stat(const char *, struct stat *);

/* Stolen from musl (src/network/ntohl.c) */
static inline uint32_t ntohl(uint32_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_32(n) : n;
}

/* Stolen from musl (src/network/htonl.c) */
static inline uint32_t htonl(uint32_t n)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap_32(n) : n;
}

static inline int issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_stat(path, &statbuf);
	if (ret == -1)
		return -1;

	ret = (statbuf.st_mode & S_ISUID) != 0;
	if (ret != 0)
		fprintf(stderr, "Warning: %s: SUID set\n", path);

	return ret;
}

static inline int isstatic(const char *path)
{
	struct elf_header {
		uint32_t magic;
		unsigned char unused[12];
		uint16_t type;
		unsigned char unused2[46];
	} hdr;
	int ret = -1, fd;
	ssize_t s;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	s = read(fd, &hdr, sizeof(hdr));
	if (s == -1)
		goto close;
	else if ((size_t)s < sizeof(hdr))
		goto close;

	ret = (hdr.magic == htonl(0x7f454c46)) && (hdr.type == 2);
	if (ret != 0)
		fprintf(stderr, "Warning: %s: statically linked\n", path);

close:
	if (close(fd))
		perror("close");

	return ret;
}

static inline int canpreload(const char *path)
{
	int ret = -1;

	ret = issuid(path);
	if (ret == -1)
		return -1;
	else if (ret == 1)
		return 0;

	ret = isstatic(path);
	if (ret == -1)
		return -1;
	else if (ret == 1)
		return 0;

	return 1;
}

int next_execve(const char *path, char * const argv[], char * const envp[])
{
	int (*sym)(const char *, char * const argv[], char * const envp[]);

	sym = dlsym(RTLD_NEXT, "execve");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, argv, envp);
}

int execve(const char *path, char *const argv[], char * const envp[])
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	if (canpreload(real_path) == 0) {
		int force = strtoul(getenv("IAMROOT_FORCE") ?: "0", NULL, 0);
		fprintf(stderr, "%s: %s: Cannot support LD_PRELOAD\n",
				force == 0 ? "Error" : "Warning", real_path);
		if (force)
			_exit(0);

		errno = ENOEXEC;
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', argv: '%s'... , envp: @%p...)\n",
			  __func__, path, real_path, argv[0], envp[0]);

	return next_execve(real_path, argv, envp);
}
