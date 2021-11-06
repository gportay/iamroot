/*
 * Copyright 2020-2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
		fprintf(stderr, "%s\n", buf);
		return;
	}

	fprintf(stderr, "%s: %s\n", s, buf);
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

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	int (*sym)(const char *, char * const argv[], char * const envp[]);
	int debug;
	int ret;

	sym = dlsym(RTLD_NEXT, "execve");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	debug = exec_debug();
	if (debug <= 0)
		debug = max(__debug() - 2, 0);

	if (debug > 0) {
		char * const *p;

		fprintf(stderr, "Debug: %s: pid: %i: execve(path: '%s', argv: {",
			__libc(), getpid(), path);
		p = argv;
		while (*p)
			fprintf(stderr, " '%s',", *p++);
		fprintf(stderr, " NULL }, ");
		if (debug == 1) {
			fprintf(stderr, "...)\n");
		} else if (debug > 1) {
			fprintf(stderr, "envp:");
			p = envp;
			while (*p)
				fprintf(stderr, " %s", *p++);
			fprintf(stderr, " }\n");
		}
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

int execve(const char *path, char * const argv[], char * const envp[])
{
	char buf[PATH_MAX], hashbangbuf[PATH_MAX], loaderbuf[PATH_MAX];
	char hashbang[HASHBANG_MAX], loader[HASHBANG_MAX];
	char *real_path, *real_hashbang = NULL;
	char * const *interargv = argv;
	char *interparg[9] = { NULL };
	int i = 0, j, ret;
	char *ld_preload;
	ssize_t siz;
	size_t len;

	/* Run exec.sh script */
	if (ignore(path))
		goto exec_sh;

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

	__verbose("%s(path: '%s' -> '%s', argv: '%s'...)\n", __func__, path,
		  real_path, argv[0]);

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
		goto exec_sh;

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
	 * Preserve original path in argv0 and prepend the interpreter
	 * arguments.
	 *
	 * The busybox binary requires the real applet name in first argument:
	 *
	 *	$ path [arguments...]
	 *
	 * The next call to execve() resolves to:
	 *
	 * 	$ busybox applet [arguments...]
	 * 	          ^path   ^argv[1]...
	 *
	 * Where applet is path and arguments is argv[1]..argv[n].
	 *
	 * In case of the path is an bashbang and the interpreter is a symlink
	 * to busybox, the script name must be preserved for the subsequent
	 * call to execve() that will resolve hashbang:
	 *
	 *	$ script [arguments...]
	 *
	 * The next call to execve() resolves to:
	 *
	 * 	$ busybox interp [interparg] script [arguments...]
	 * 	          ^-------^hashbang  ^path   ^argv[1]...
	 *
	 * Where interp and interparg is hashbang, script is path and arguments
	 * is argv[1]..argv[n].
	 *
	 * Preserving original path in argv0 both keeps a track of the original
	 * path and satisfies the needs for busybox.
	 */
	real_hashbang = path_resolution(hashbang, hashbangbuf,
					sizeof(hashbangbuf),
					AT_SYMLINK_FOLLOW);
	if (!real_hashbang) {
		perror("path_resolution");
		return -1;
	}

	len = strlen(hashbang);
	/*
	 * Handle the busybox case by prepending the hashbang (i.e. the busybox
	 * applet) and its optional hashbang argument before the executable
	 * (path)
	 */
	if (strcmp(basename(real_hashbang), "busybox") == 0) {
		interparg[i++] = hashbang; /* busybox applet */
		interargv++; /* shift argv0 */

		/* Add optional argument */
		if (len < (size_t)siz)
			interparg[i++] = &hashbang[len+1];
		interparg[i++] = (char *)path;
		interparg[i++] = NULL;

		real_path = real_hashbang;
		goto loader;
	}

	interparg[i++] = (char *)path;
	/* Add optional argument */
	if (len < (size_t)siz)
		interparg[i++] = &hashbang[len+1];
	interparg[i++] = NULL;

	real_path = real_hashbang;

loader:
	/*
	 * Run the dynamic linker directly
	 */
	if (__strncmp(path, "/lib/ld") == 0)
		return next_execve(real_path, argv, envp);

	/*
	 * Get the dynamic linker stored in the .interp section of the ELF
	 * linked program.
	 */
	siz = getinterp(real_path, loader, sizeof(loader));
	if (siz == -1) {
		/* Not an ELF linked program */
		if (errno == ENOEXEC)
			goto exec_sh;

		perror("getinterp");
		return -1;
	} else if (siz == 0) {
		goto exec_sh;
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
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument)
		 */
		for (j = 0; j < i; j++)
			interparg[j+4] = interparg[j];

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
		interparg[i++] = getenv("IAMROOT_LIB_MUSL_X86_64") ?: "/usr/lib/iamroot/libiamroot-musl-x86_64.so.1";

		/* Add path to binary (in chroot, first positional argument) */
		if (real_hashbang)
			interparg[i++] = real_hashbang;
		else
			interparg[i++] = buf;

		extern char **__environ;
		envp = __environ;
	}

	return interpexecve(real_path, interparg, interargv, envp);

exec_sh:
	real_path = strncpy(buf, getenv("SHELL") ?: "/bin/bash", sizeof(buf)-1);
	interparg[0] = (char *)path;
	interparg[1] = getenv("IAMROOT_EXEC") ?: "/usr/lib/iamroot/exec.sh";
	interparg[2] = NULL;

	return interpexecve(real_path, interparg, argv, envp);
}
