/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <byteswap.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <regex.h>

#include <unistd.h>

#include "path_resolution.h"

/* See https://www.in-ulm.de/~mascheck/various/shebang/#results */
#define HASHBANG_MAX NAME_MAX

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern const char *getrootdir();
extern int next_open(const char *, int, mode_t);
extern int next_stat(const char *, struct stat *);

static regex_t *re;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		fprintf(stderr, "%s\n", buf);
		return;
	}

	fprintf(stderr, "%s: %s\n", s, buf);
}

__attribute__((constructor))
void execve_init()
{
	static regex_t regex;
	char *ignore;
	int ret;

	if (re)
		return;

	ignore = getenv("IAMROOT_EXEC_IGNORE");
	if (!ignore)
		ignore = "^(/usr/bin/|/bin/|)systemd-sysusers$|ldd";

	ret = regcomp(&regex, ignore, REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex, ret);
		return;
	}

	__fprintf(stderr, "IAMROOT_EXEC_IGNORE=%s\n", ignore);
	re = &regex;
}

__attribute__((destructor))
void execve_fini()
{
	if (!re)
		return;

	regfree(re);
	re = NULL;
}

static int ignore(const char *path)
{
	int ret = 0;

	if (!re)
		return 0;

	ret = regexec(re, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regcomp", re, ret);
		return 0;
	}

	return !ret;
}

static int issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_stat(path, &statbuf);
	if (ret == -1)
		return -1;

	return (statbuf.st_mode & S_ISUID) != 0;
}

static ssize_t getinterp(const char *path, char *buf, size_t bufsize)
{
	ssize_t s, ret = -1;
	int fd, i, num;
	Elf64_Ehdr hdr;
	off_t off;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	s = read(fd, &hdr, sizeof(hdr));
	if (s == -1)
		goto close;
	else if ((size_t)s < sizeof(hdr))
		goto close;

	/* Not an ELF */
	if (memcmp(hdr.e_ident, ELFMAG, 4) != 0) {
		errno = ENOEXEC;
		goto close;
	}

	/* TODO: Support class ELF32 */
	if (hdr.e_ident[EI_CLASS] != ELFCLASS64) {
		errno = ENOEXEC;
		goto close;
	}

	ret = 0;

	/* Not a linked program or shared object */
	if ((hdr.e_type != ET_EXEC) && (hdr.e_type != ET_DYN)) {
		errno = ENOEXEC;
		goto close;
	}

	/* Look for the .interp section */
	off = hdr.e_phoff;
	num = hdr.e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr hdr;

		s = pread(fd, &hdr, sizeof(hdr), off);
		if (s == -1)
			goto close;
		else if ((size_t)s < sizeof(hdr))
			goto close;

		off += sizeof(hdr);

		if (hdr.p_type != PT_INTERP)
			continue;

		if (bufsize < hdr.p_filesz) {
			errno = ENAMETOOLONG;
			goto close;
		}

		s = pread(fd, buf, hdr.p_filesz, hdr.p_offset);
		if (s == -1)
			goto close;
		else if ((size_t)s < hdr.p_filesz)
			goto close;

		ret = s;
		goto close;
	}

	errno = ENOEXEC;
	ret = -1;

close:
	if (close(fd))
		perror("close");

	return ret;
}

static ssize_t gethashbang(const char *path, char *buf, size_t bufsize)
{
	ssize_t siz;
	char *d, *s;
	int fd;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	siz = read(fd, buf, bufsize-1);
	if (siz == -1)
		goto close;

	buf[siz] = 0;

	/* Not an hashbang interpreter directive */
	if ((siz < 2) || (buf[0] != '#') || (buf[1] != '!')) {
		errno = ENOEXEC;
		siz = -1;
		goto close;
	}

	s = buf+2;
	d = buf;

	while (isblank(*s))
		s++;
	while (*s && *s != '\n' && !isblank(*s))
		*d++ = *s++;
	*d++ = 0;

	if (isblank(*s)) {
		while (isblank(*s))
			s++;
		while (*s && *s != '\n' && !isblank(*s))
			*d++ = *s++;
		*d++ = 0;
	}

	siz = d-buf-1;

close:
	if (close(fd))
		perror("close");

	return siz;
}

static int force()
{
	return strtoul(getenv("IAMROOT_FORCE") ?: "0", NULL, 0);
}

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	int (*sym)(const char *, char * const argv[], char * const envp[]);

	sym = dlsym(RTLD_NEXT, "execve");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, argv, envp);
}

int execve(const char *path, char *const argv[], char * const envp[])
{
	char *interparg, interp[HASHBANG_MAX];
	int argc = -1, ret, forced;
	char *real_path, *exec;
	char buf[PATH_MAX];
	char * const *arg;
	ssize_t siz;

	if (ignore(path))
		goto exec;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = issuid(real_path);
	if (ret == -1)
		return -1;
	else if (ret)
		goto exec;

	/*
	 * Get the dynamic linker stored in the .interp section of the ELF
	 * linked program.
	 */
	siz = getinterp(real_path, interp, sizeof(interp));
	if (siz == -1) {
		/* Not an ELF linked program */
		if (errno == ENOEXEC)
			goto hashbang;

		perror("getinterp");
		return -1;
	} else if (siz == 0) {
		goto force;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', argv: '%s'... , envp: @%p...)\n",
			  __func__, path, real_path, argv[0], envp[0]);

	return next_execve(real_path, argv, envp);

hashbang:
	/* Do not proceed to interpreter hack if not in chroot */
	if (strcmp(getrootdir(), "/") == 0)
		return next_execve(path, argv, envp);

	/* Get the interpeter directive stored after the hashbang */
	siz = gethashbang(real_path, interp, sizeof(interp));
	if (siz == -1) {
		/* Not an hashbang interpreter directive */
		if (errno == ENOEXEC)
			goto exec;

		perror("gethashbang");
		return -1;
	} else if (siz > 0) {
		size_t len;

		argc = 1;
		arg = argv;
		while (*arg++)
			argc++;

		len = strlen(interp);
		interparg = (len < (size_t)siz) ? &interp[len+1] : NULL;
		if (interparg)
			argc++;

		goto interp;
	}

exec:
	exec = getenv("IAMROOT_EXEC") ?: "/usr/lib/iamroot/exec.sh";
	if (!*exec)
		goto force;

	strcpy(interp, getenv("SHELL") ?: "/bin/bash");
	interparg = exec;

	argc = 2;
	arg = argv;
	while (*arg++)
		argc++;

interp:
	if ((argc != -1) && (argc < ARG_MAX)) {
		char *nargv[argc + 1];
		char **narg;

		/* Prepend the interpreter and its optional argument */
		narg = nargv;
		*narg++ = interp;
		if (interparg)
			*narg++ = interparg;

		arg = argv;
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL;

		return execve(interp, nargv, environ);
	}

force:
	forced = force();
	fprintf(stderr, "%s: %s: Cannot support LD_PRELOAD\n",
			forced == 0 ? "Error" : "Warning", real_path);
	if (forced)
		_exit(0);

	errno = ENOEXEC;
	return -1;
}
