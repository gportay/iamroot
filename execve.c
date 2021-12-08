/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
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

#include "iamroot.h"

/* See https://www.in-ulm.de/~mascheck/various/shebang/#results */
#define HASHBANG_MAX NAME_MAX
#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	  __typeof__ (b) _b = (b); \
	  _a > _b ? _a : _b; })
#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
	  __typeof__ (b) _b = (b); \
	  _a < _b ? _a : _b; })

extern int next_open(const char *, int, mode_t);
extern int next_stat(const char *, struct stat *);

static regex_t *re;

static void __regex_perror(const char *s, regex_t *regex, int err)
{
	char buf[128];
	regerror(err, regex, buf, sizeof(buf));
	if (!s) {
		dprintf(STDERR_FILENO, "%s\n", buf);
		return;
	}

	dprintf(STDERR_FILENO, "%s: %s\n", s, buf);
}

__attribute__((constructor))
void execve_init()
{
	static regex_t regex;
#ifndef JIMREGEXP_H
	__attribute__((unused)) static char jimpad[40];
#endif
	const char *ignore;
	int ret;

	if (re)
		return;

	ignore = getenv("IAMROOT_EXEC_IGNORE");
	if (!ignore)
		ignore = "ldd";

	ret = regcomp(&regex, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret) {
		__regex_perror("regcomp", &regex, ret);
		return;
	}

	__verbose("IAMROOT_EXEC_IGNORE=%s\n", ignore);
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
		__regex_perror("regexec", re, ret);
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
	if (s == -1) {
		goto close;
	} else if ((size_t)s < sizeof(hdr)) {
		errno = ENOEXEC;
		goto close;
	}

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
		if (s == -1) {
			goto close;
		} else if ((size_t)s < sizeof(hdr)) {
			errno = ENOEXEC;
			goto close;
		}

		off += sizeof(hdr);

		if (hdr.p_type != PT_INTERP)
			continue;

		if (bufsize < hdr.p_filesz) {
			errno = ENAMETOOLONG;
			goto close;
		}

		s = pread(fd, buf, hdr.p_filesz, hdr.p_offset);
		if (s == -1) {
			goto close;
		} else if ((size_t)s < hdr.p_filesz) {
			errno = ENOEXEC;
			goto close;
		}

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

static int exec_debug()
{
	return strtoul(getenv("IAMROOT_EXEC_DEBUG") ?: "0", NULL, 0);
}

#if !defined(NVERBOSE)
static void verbose_exec(const char *path, char * const argv[],
			 char * const envp[])
{
	int debug;

	debug = exec_debug();
	if (debug <= 0)
		debug = max(__debug() - 2, 0);

	if (debug == 1) {
		char * const *p;

		dprintf(STDERR_FILENO, "Debug: running");

		p = argv;
		while (*p)
			dprintf(STDERR_FILENO, " %s", *p++);
		dprintf(STDERR_FILENO, "\n");
	} else if (debug > 1) {
		char * const *p;

		dprintf(STDERR_FILENO, "Debug: %s: pid: %i: execve(path: '%s', argv: {",
			__libc(), getpid(), path);
		p = argv;
		while (*p)
			dprintf(STDERR_FILENO, " '%s',", *p++);
		dprintf(STDERR_FILENO, " NULL }, ");
		if (debug == 2) {
			dprintf(STDERR_FILENO, "...)\n");
		} else if (debug > 2) {
			dprintf(STDERR_FILENO, "envp:");
			p = envp;
			while (*p)
				dprintf(STDERR_FILENO, " %s", *p++);
			dprintf(STDERR_FILENO, " }\n");
		}
	}
}
#else
#define verbose_exec()
#endif

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	int (*sym)(const char *, char * const argv[], char * const envp[]);
	int ret;

	verbose_exec(path, argv, envp);

	sym = dlsym(RTLD_NEXT, "execve");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, argv, envp);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

static int interpexecve(const char *path, char * const interp[],
			char * const argv[], char * const envp[])
{
	char * const *arg;
	int argc;

	argc = 1;
	arg = interp;
	while (*arg++)
		argc++;
	arg = argv;
	while (*arg++)
		argc++;

	if ((argc > 0) && (argc < ARG_MAX)) {
		char *nargv[argc+1]; /* NULL */
		char **narg;

		narg = nargv;
		arg = interp;
		while (*arg)
			*narg++ = *arg++;
		arg = argv;
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL;

		return next_execve(path, nargv, envp);
	}

	errno = EINVAL;
	return -1;
}

static int __use_host_interp()
{
	return strtol(getenv("IAMROOT_USE_HOST_INTERP") ?: "0", NULL, 0);
}

static char *__libiamroot_musl_x86_64()
{
	char *ret;

	ret = getenv("IAMROOT_LIB_MUSL_X86_64");
	if (!ret)
		return "/usr/lib/iamroot/libiamroot-musl-x86_64.so.1";

	return ret;
}

static char *__libiamroot_linux_x86_64()
{
	char *ret;

	ret = getenv("IAMROOT_LIB_LINUX_X86_64");
	if (!ret)
		return "/usr/lib/iamroot/libiamroot-linux-x86-64.so.2";

	return ret;
}

static char *__ld_preload_linux_x86_64()
{
	char *ret;

	ret = getenv("IAMROOT_LD_PRELOAD_LINUX_X86_64");
	if (!ret)
		return "/usr/lib64/libc.so.6:/usr/lib64/libdl.so.2";

	return ret;
}

int execve(const char *path, char * const argv[], char * const envp[])
{
	char buf[PATH_MAX], hashbangbuf[PATH_MAX], loaderbuf[PATH_MAX];
	char hashbang[HASHBANG_MAX], loader[HASHBANG_MAX];
	char *real_path, *real_hashbang = NULL;
	char *interparg[9] = { NULL };
	char *interpargv0 = *argv;
	char *interppath = NULL;
	int i = 0, j, ret;
	char *ld_preload;
	ssize_t siz;
	size_t len;

	/* Run exec.sh script */
	if (ignore(path)) {
		__verbose("%s: Ignored\n", path);
		goto exec_sh;
	}

	/*
	 * Follows symlink as the subsequent calls to issuid(), getinterp() and
	 * gethashbang() use stat() and open() which are functions that follow
	 * symlinks.
	 *
	 * However, path_resolution() follows symlink by default; adding that
	 * AT flag and this comment emphasizes the need to follow symlinks and
	 * makes sure all the functions that use real_path in parameter resolve
	 * the exact same path.
	 */
	real_path = path_resolution(path, buf, sizeof(buf), AT_SYMLINK_FOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose_exec("%s(path: '%s' -> '%s', argv: { '%s', '%s', ... })\n",
		       __func__, path, real_path, argv[0], argv[1]);
	interppath = real_path; /* real program path as binary */
	interparg[i++] = (char *)path; /* original program path as argv0 */

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = issuid(real_path);
	if (ret == -1) {
		return -1;
	} else if (ret) {
		__verbose("%s: SUID\n", real_path);
		goto exec_sh;
	}

	/* Do not proceed to any hack if not in chroot */
	if (!inchroot())
		return next_execve(path, argv, envp);

	/* Get the interpeter directive stored after the hashbang */
	siz = gethashbang(real_path, hashbang, sizeof(hashbang));
	if (siz == -1) {
		/* Not an hashbang interpreter directive */
		if (errno == ENOEXEC)
			goto loader;

		perror("gethashbang");
		return -1;
	} else if (siz == 0) {
		goto loader;
	}

	/*
	 * Preserve original path in argv0 and set the interpreter and its
	 * optional argument (if any).
	 */
	real_hashbang = path_resolution(hashbang, hashbangbuf,
					sizeof(hashbangbuf),
					AT_SYMLINK_FOLLOW);
	if (!real_hashbang) {
		perror("path_resolution");
		return -1;
	}

	/* Reset argv0 */
	interparg[0] = hashbang; /* hashbang as argv0 */

	/* Add optional argument */
	len = strlen(hashbang);
	if (len < (size_t)siz)
		interparg[i++] = &hashbang[len+1];
	interparg[i++] = (char *)path; /* original program path as first
					* positional argument */
	interparg[i++] = NULL;

	interpargv0 = hashbang; /* hashbang as argv0 */
	interppath = real_hashbang; /* real hashbang as binary */
	real_path = real_hashbang; /* real hashbang path as binary */

	__verbose("%s: has hashbang: '%s' -> '%s' '%s'\n", path, hashbang,
		  real_hashbang, len < (size_t)siz ? &hashbang[len+1] : "");

loader:
	/*
	 * Run the dynamic linker directly
	 */
	if ((__strncmp(path, "/lib/ld") == 0) ||
	    (__strncmp(path, "/lib64/ld") == 0))
		return next_execve(real_path, argv, envp);

	/*
	 * Get the dynamic linker stored in the .interp section of the ELF
	 * linked program.
	 */
	siz = getinterp(real_path, loader, sizeof(loader));
	if (siz == -1) {
		/* Not an ELF linked program */
		if (errno == ENOEXEC) {
			__verbose("%s: Not an ELF linked program\n", real_path);
			goto exec_sh;
		}

		perror("getinterp");
		return -1;
	} else if (siz == 0) {
		__verbose("%s: No such .interp section\n", real_path);
		goto exec_sh;
	}

	__verbose("%s: interpreter: '%s'\n", real_path, loader);

	/*
	 * Run the interpreter from host file-system (i.e. do not use the
	 * interpreter from the chroot environment).
	 */
	if (__use_host_interp()) {
		__verbose("%s: use interpreter from host: '%s'\n", real_path,
			  loader);
		goto interp;
	}

	/*
	 * The interpreter has to preload its libiamroot.so library.
	 *
	 * TODO: Detect *real* change in interpreter. It is assumed for now the
	 * interpreter is /lib64/ld-linux-x86-64.so.2 (aka glibc's x86_64 ld).
	 */
	if (strcmp(loader, "/lib/ld-musl-x86_64.so.1") == 0) {
		real_path = path_resolution(loader, loaderbuf,
					    sizeof(loaderbuf),
					    AT_SYMLINK_FOLLOW);
		if (!real_path) {
			perror("path_resolution");
			return -1;
		}

		/*
		 * Shift enough interparg to prepend:
		 *   - the path to the interpreter (i.e. the full path in host,
		 *     including the chroot; argv0)
		 *   - the option --ld-preload and its argument (i.e. the path
		 *     in host to the interpreter's libiamroot.so to preload)
		 *   - the option --argv0 and its argument (i.e. the original
		 *     path in host to the binary).
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument)
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		for (j = 0; j < i; j++)
			interparg[j+5] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = real_path;

		/*
		 * Strip libiamroot.so from LD_PRELOAD
		 *
		 * TODO: Remove *real* libiamroot.so. It is assumed for now the
		 * library is at the first place.
		 */
		ld_preload = getenv("LD_PRELOAD");
		if (ld_preload) {
			char *n, *s = ld_preload;

			n = strchr(s, ':');
			if (n)
				n++;

			ld_preload = n;
			if (ld_preload && *ld_preload)
				setenv("LD_PRELOAD", ld_preload, 1);
			else
				unsetenv("LD_PRELOAD");
		}

		/* Add --preload and interpreter's library (host) */
		interparg[i++] = "--preload";
		interparg[i++] = __libiamroot_musl_x86_64();

		/* Add --argv0 and original argv0 */
		interparg[i++] = "--argv0";
		interparg[i++] = interpargv0;

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i++] = interppath;

		extern char **__environ;
		envp = __environ;
	} else if (strcmp(loader, "/lib64/ld-linux-x86-64.so.2") == 0) {
		real_path = path_resolution(loader, loaderbuf,
					    sizeof(loaderbuf),
					    AT_SYMLINK_FOLLOW);
		if (!real_path) {
			perror("path_resolution");
			return -1;
		}

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0)
		 *   - the option --ld-preload and its argument (i.e. the path
		 *     in host to the interpreter's libiamroot.so to preload)
		 *   - another option --ld-preload and its argument (i.e. the
		 *     path in chroot environment to the interpreter's libc.so
		 *     and libdl.so to preload)
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument)
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		for (j = 0; j < i; j++)
			interparg[j+5] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = real_path;

		/* Add --preload and interpreter's libraries */
		/* libiamroot.so (from host) */
		interparg[i++] = "--preload";
		interparg[i++] = __libiamroot_linux_x86_64();
		/* libc.so and libdl.so (from chroot) */
		ret = pathsetenv(getrootdir(), "LD_PRELOAD_LINUX_X86_64",
				 __ld_preload_linux_x86_64(), 1);
		if (ret) {
			perror("pathsetenv");
			return -1;
		}
		interparg[i++] = "--preload";
		interparg[i++] = getenv("LD_PRELOAD_LINUX_X86_64");

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i++] = interppath;

		extern char **__environ;
		envp = __environ;
	}

interp:
	goto execve;

exec_sh:
	real_path = strncpy(buf, getenv("SHELL") ?: "/bin/bash", sizeof(buf)-1);
	interparg[0] = (char *)path; /* original program path as argv0 */
	interparg[1] = getenv("IAMROOT_EXEC") ?: "/usr/lib/iamroot/exec.sh";
	interparg[2] = *argv; /* original argv0 as first positional argument */
	interparg[3] = NULL;

execve:
	return interpexecve(real_path, interparg, ++argv, envp);
}
