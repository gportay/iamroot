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

extern int next_open(const char *, int, mode_t);
extern int next_fstatat(int, const char *, struct stat *, int);

static const char *__basename(const char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1;
}

int pathprependenv(const char *name, const char *value, int overwrite)
{
	char *newval, *oldval;
	char buf[PATH_MAX];

	if (!name || !value) {
		errno = EINVAL;
		return -1;
	}

	newval = __strncpy(buf, value);
	oldval = getenv(name);
	if (oldval && *oldval) {
		__strncat(buf, ":");
		__strncat(buf, oldval);
	}

	return setenv(name, newval, overwrite);
}

int pathsetenv(const char *root, const char *name, const char *value,
	       int overwrite)
{
	size_t rootlen, vallen, newlen;

	if (!name || !value) {
		errno = EINVAL;
		return -1;
	}

	if (!root)
		goto setenv;

	newlen = 0;
	rootlen = strlen(root);

	vallen = strlen(value);
	if (vallen > 0) {
		char *token, *saveptr, val[vallen+1];

		newlen = vallen;
		newlen += rootlen;
		newlen++; /* NUL */

		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token)
			while (strtok_r(NULL, ":", &saveptr))
				newlen += rootlen;
	}

	if (newlen > 0) {
		char *str, *token, *saveptr, val[vallen+1], new_value[newlen+1];

		str = new_value;
		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token) {
			int n;

			n = snprintf(str, newlen, "%s%s", root, token);
			if ((n == -1) || (newlen < (size_t)n)) {
				errno = EOVERFLOW;
				return -1;
			}
			str += n;
			newlen -= n;
			while ((token = strtok_r(NULL, ":", &saveptr))) {
				n = snprintf(str, newlen, ":%s%s", root, token);
				if ((n == -1) || (newlen < (size_t)n)) {
					errno = EOVERFLOW;
					return -1;
				}
				str += n;
				newlen -= n;
			}
		}

		return setenv(name, new_value, overwrite);
	}

setenv:
	return setenv(name, value, overwrite);
}


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

	__info("IAMROOT_EXEC_IGNORE=%s\n", ignore);
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

static int __ld_linux_version(const char *path, int *major, int *minor)
{
	const char *basename;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = readlink(path, buf, sizeof(buf));
	if (siz == -1) {
		__pathperror(path, "readlink");
		return -1;
	}
	buf[siz] = 0;

	basename = __basename(buf);
	ret = sscanf(basename, "ld-%i.%i.so", major, minor);
	if (ret < 2) {
		errno = ENOTSUP;
		return -1;
	}

	return 0;
}

static int __ld_linux_has_argv0_option(const char *path)
{
	int ret, maj = 0, min = 0;
	struct stat statbuf;

	ret = lstat(path, &statbuf);
	if (ret == -1)
		return -1;

	/* not a symlink since glibc 2.34; --argv0 is supported since 2.33 */
	if (!S_ISLNK(statbuf.st_mode))
		return 1;

	ret = __ld_linux_version(path, &maj, &min);
	if (ret == -1)
		return -1;

	/* --argv0 is supported since glibc 2.33 */
	return (maj > 2) || ((maj == 2) && (min >= 33));
}

static int issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, 0);
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
		__fpathperror(fd, "close");

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
	buf[siz] = 0; /* ensure NULL terminated */

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
		__fpathperror(fd, "close");

	return siz;
}

#if !defined(NVERBOSE)
static void verbose_exec(const char *path, char * const argv[],
			 char * const envp[])
{
	int debug;

	debug = __getdebug();
	if (debug == 0)
		return;

	if (debug == 1) {
		char *ld_library_path;
		char *ld_preload;
		char * const *p;
		char *root;

		dprintf(STDERR_FILENO, "Debug: running");

		root = __getroot();
		if (root)
			dprintf(STDERR_FILENO, " IAMROOT_ROOT=%s", root);

		ld_preload = getenv("LD_PRELOAD");
		if (ld_preload)
			dprintf(STDERR_FILENO, " LD_PRELOAD=%s", ld_preload);

		ld_library_path = getenv("LD_LIBRARY_PATH");
		if (ld_library_path)
			dprintf(STDERR_FILENO, " LD_LIBRARY_PATH=%s",
				ld_library_path);

		p = argv;
		while (*p)
			dprintf(STDERR_FILENO, " %s", *p++);
		dprintf(STDERR_FILENO, "\n");
	} else {
		char * const *p;

		dprintf(STDERR_FILENO, "Debug: %s: %s: pid: %i: execve(path: '%s', argv: {",
			__libc(), __arch(), getpid(), path);
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
#define verbose_exec(path, argv, envp)
#endif

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	int (*sym)(const char *, char * const argv[], char * const envp[]);
	int ret;

	verbose_exec(path, argv, envp);

	sym = dlsym(RTLD_NEXT, "execve");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, argv, envp);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

static void __sanitize(char *name)
{
	char *c;

	c = name;
	while (*c) {
		if (*c == '-')
			*c = '_';
		else
			*c = toupper(*c);
		c++;
	}
}

static char *__getlibiamroot(const char *ldso, int abi)
{
	char buf[NAME_MAX];
	char *ret;
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_LIB_%s_%i", ldso, abi);
	if (n == -1)
		return NULL;
	__sanitize(buf);

	ret = getenv(buf);
	if (!ret)
#if defined(__GLIBC__)
#if defined(__x86_64__)
		return "/usr/lib/iamroot/libiamroot-linux-x86-64.so.2";
#elif defined(__aarch64__)
		return "/usr/lib/iamroot/libiamroot-linux-aarch64.so.1";
#else
		return NULL;
#endif
#else /* assuming musl */
#if defined(__x86_64__)
		return "/usr/lib/iamroot/libiamroot-musl-x86_64.so.1";
#elif defined(__aarch64__)
		return "/usr/lib/iamroot/libiamroot-musl-aarch64.so.1";
#else
		return NULL;
#endif
#endif

	return ret;
}

static char *__getld_preload(const char *ldso, int abi)
{
	char buf[NAME_MAX];
	char *ret;
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_LD_PRELOAD_%s_%d", ldso, abi);
	if (n == -1)
		return NULL;
	__sanitize(buf);

	ret = getenv(buf);
	if (!ret)
#if defined(__GLIBC__)
#if defined(__LP64__)
		return "/usr/lib64/libc.so.6:/usr/lib64/libdl.so.2";
#else
		return NULL;
#endif
#else /* assuming musl */
#if defined(__LP64__)
		return "";
#else
		return NULL;
#endif
#endif

	return ret;
}

static char *__ld_preload(const char *ldso, int abi)
{
	char buf[NAME_MAX];
	int n, ret;

	n = _snprintf(buf, sizeof(buf), "LD_PRELOAD_%s", ldso);
	if (n == -1)
		return NULL;
	__sanitize(buf);

	ret = pathsetenv(getrootdir(), buf, __getld_preload(ldso, abi), 1);
	if (ret) {
		perror("pathsetenv");
		return NULL;
	}

	ret = pathprependenv(buf, __getlibiamroot(ldso, abi), 1);
	if (ret) {
		perror("pathprependenv");
		return NULL;
	}

	return getenv(buf);
}

static int __getuse_host_interp()
{
	return strtol(getenv("IAMROOT_USE_HOST_INTERP") ?: "0", NULL, 0);
}

static char *__ld_library_path()
{
	int ret;

	ret = pathsetenv(getrootdir(), "LD_LIBRARY_PATH_INTERP",
			 getenv("IAMROOT_LD_LIBRARY_PATH") ?: "/usr/lib:/lib",
			 1);
	if (ret) {
		perror("pathprependenv");
		return NULL;
	}

	return getenv("LD_LIBRARY_PATH_INTERP");
}

static char *__getexec()
{
	char *ret;

	ret = getenv("IAMROOT_EXEC");
	if (!ret)
		return "/usr/lib/iamroot/exec.sh";

	return ret;
}

int execve(const char *path, char * const argv[], char * const envp[])
{
	char buf[PATH_MAX], hashbangbuf[PATH_MAX], loaderbuf[PATH_MAX];
	char hashbang[HASHBANG_MAX], loader[HASHBANG_MAX];
	char *real_path, *real_hashbang = NULL;
	char *interparg[10+1] = { NULL }; /*  0 ARGV0
					   *  1 /lib/ld.so
					   *  2 --preload
					   *  3 libiamroot.so:libc.so:libdl.so
					   *  4 --library-path
					   *  3 /usr/lib:/lib
					   *  6 --argv0
					   *  7 ARGV0
					   *  8 /bin/sh
					   *  9 -x
					   * 10 script.sh
					   * 11 NULL
					   */
	char *interpargv0 = *argv;
	char *interppath = NULL;
	int i = 0, j, argc, ret;
	char * const *arg;
	ssize_t siz;
	size_t len;
	char *exec;

	/* Run exec.sh script */
	if (ignore(path)) {
		__notice("%s: Ignored\n", path);
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
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', argv: { '%s', '%s', ... })\n",
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
		__notice("%s: SUID\n", real_path);
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

		__pathperror(real_path, "gethashbang");
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
		__pathperror(hashbang, "path_resolution");
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

	__notice("%s: has hashbang: '%s' -> '%s' '%s'\n", path, hashbang,
		 real_hashbang, len < (size_t)siz ? &hashbang[len+1] : "");

loader:
	/*
	 * Run the dynamic linker directly
	 */
	if ((__strncmp(path, "/usr/bin/ld.so") == 0) ||
	    (__strncmp(path, "/lib/ld") == 0) ||
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
			__notice("%s: Not an ELF linked program\n", real_path);
			goto exec_sh;
		}

		__pathperror(real_path, "getinterp");
		return -1;
	} else if (siz == 0) {
		__notice("%s: No such .interp section\n", real_path);
		goto exec_sh;
	}

	__notice("%s: interpreter: '%s'\n", real_path, loader);

	/*
	 * Run the interpreter from host file-system (i.e. do not use the
	 * interpreter from the chroot environment).
	 */
	if (__getuse_host_interp()) {
		__notice("%s: use interpreter from host: '%s'\n", real_path,
			 loader);
		goto interp;
	}

	/*
	 * The interpreter has to preload its libiamroot.so library.
	 */
	if ((__strncmp(loader, "/lib/ld") == 0) ||
	    (__strncmp(loader, "/lib64/ld") == 0)) {
		char *ld_preload, *ld_library_path;
		int has_argv0 = 1, shift = 1;
		const char *basename;
		char ldso[NAME_MAX];
		int abi = 0;

		basename = __basename(loader);
		ret = sscanf(basename, "ld-%[^.].so.%d", ldso, &abi);
		if (ret < 2) {
			__pathperror(basename, "sscanf");
			errno = ENOTSUP;
			return -1;
		}

		/* the glibc world supports --argv0 since 2.33 */
		if (__strncmp(ldso, "linux") == 0) {
			has_argv0 = __ld_linux_has_argv0_option(loader);
			if (has_argv0 == -1) {
				__pathperror(loader,
					     "__ld_linux_has_argv0_option");
				return -1;
			}
		}

		ld_preload = __ld_preload(ldso, abi);
		if (!ld_preload)
			__warning("%s: is unset!\n", "LD_PRELOAD");

		ld_library_path = __ld_library_path();
		if (!ld_library_path)
			__warning("%s: is unset!", "LD_LIBRARY_PATH");

		real_path = path_resolution(loader, loaderbuf,
					    sizeof(loaderbuf),
					    AT_SYMLINK_FOLLOW);
		if (!real_path) {
			__pathperror(loader, "path_resolution");
			return -1;
		}

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0)
		 *   - the option --ld-preload and its argument (i.e. the path
		 *     in host environment to the interpreter's libiamroot.so,
		 *     and the path in chroot environment to the interpreter's
		 *     libc.so and libdl.so)
		 *   - the option --library-path and its argument (i.e. the
		 *     path in chroot environment to the libraries)
		 *   - the option --argv0 and its argument (i.e. the original
		 *     path in host to the binary).
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument)
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		if (has_argv0)
			shift += 2;
		if (ld_preload)
			shift += 2;
		if (ld_library_path)
			shift += 2;
		for (j = 0; j < i; j++)
			interparg[j+shift] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = real_path;

		/*
		 * Add --preload and interpreter's libraries:
		 *  - libiamroot.so (from host)
		 *  - libc.so and libdl.so (from chroot)
		 */
		if (ld_preload) {
			interparg[i++] = "--preload";
			interparg[i++] = ld_preload;
		}

		/* Add --library-path (chroot) */
		if (ld_library_path) {
			interparg[i++] = "--library-path";
			interparg[i++] = ld_library_path;
		}

		/* Add --argv0 and original argv0 */
		if (has_argv0) {
			interparg[i++] = "--argv0";
			interparg[i++] = interpargv0;
		}

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i++] = interppath;

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
	}

interp:
	goto execve;

exec_sh:
	exec = __getexec();
	real_path = __strncpy(buf, exec);
	i = 0;
	interparg[i++] = exec;
	interparg[i++] = (char *)path; /* original path as first positional
					* argument
					*/
	interparg[i++] = NULL;

	ret = setenv("argv0", *argv, 1);
	if (ret) {
		__envperror("argv0", "setenv");
		return -1;
	}

	ret = setenv("ld_preload", getenv("LD_PRELOAD") ?: "", 1);
	if (ret) {
		__envperror("ld_preload", "setenv");
		return -1;
	}

	ret = setenv("ld_library_path", getenv("LD_LIBRARY_PATH") ?: "", 1);
	if (ret) {
		__envperror("ld_library_path", "setenv");
		return -1;
	}

	ret = unsetenv("LD_PRELOAD");
	if (ret) {
		__envperror("LD_PRELOAD", "unsetenv");
		return -1;
	}

	ret = unsetenv("LD_LIBRARY_PATH");
	if (ret) {
		__envperror("LD_LIBRARY_PATH", "unsetenv");
		return -1;
	}

execve:
	/*
	 * envp is updated by the change in the environments (i.e. by calling
	 * setenv() abd unsetenv()).
	 */ 
	if (envp != environ)
		envp = environ;

	argc = 1;
	arg = interparg;
	while (*arg++)
		argc++;
	arg = argv+1;
	while (*arg++)
		argc++;

	if ((argc > 0) && (argc < ARG_MAX)) {
		char *nargv[argc+1]; /* NULL */
		char **narg;

		narg = nargv;
		arg = interparg;
		while (*arg)
			*narg++ = *arg++;
		arg = argv+1;
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL;

		return next_execve(real_path, nargv, envp);
	}

	errno = EINVAL;
	return -1;
}
