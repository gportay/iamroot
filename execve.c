/*
 * Copyright 2020-2023 Gaël PORTAY
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
#include <spawn.h>
#include <paths.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined __linux__ || defined __FreeBSD__
#include <sys/auxv.h>
#endif
#include <fcntl.h>
#include <elf.h>
#include <regex.h>

#include <unistd.h>

#include "iamroot.h"

typedef struct {
	regex_t re;
#ifndef JIMREGEXP_H
	char jimpad[40];
#endif
} __regex_t;

extern int next_open(const char *, int, mode_t);
extern int next_fstatat(int, const char *, struct stat *, int);
extern int next_posix_spawn(pid_t *, const char *,
			    const posix_spawn_file_actions_t *,
			    const posix_spawnattr_t *, char * const [],
			    char * const []);

#ifdef __linux__
static int __secure()
{
	return getauxval(AT_SECURE) != 0;
}

static const char *__execfn()
{
	return (const char *)getauxval(AT_EXECFN);
}
#endif

#ifdef __FreeBSD__
static const char *__execfn()
{
	static char buf[PATH_MAX];
	int ret;

	ret = elf_aux_info(AT_EXECPATH, buf, sizeof(buf));
	if (ret == -1)
		return NULL;

	return buf;
}
#endif

#ifdef __OpenBSD__
static const char *__execfn()
{
	return __set_errno(ENOSYS, NULL);
}
#endif

__attribute__((visibility("hidden")))
char *__basename(char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1; /* trailing-slash */
}

__attribute__((visibility("hidden")))
const char *__path()
{
	const char *ret;

	ret = getenv("IAMROOT_PATH");
	if (!ret)
		ret = _PATH_STDPATH;

	return ret;
}

__attribute__((visibility("hidden")))
const char *__library_path()
{
	const char *ret;

	ret = getenv("IAMROOT_LIBRARY_PATH");
	if (!ret)
		ret = _PATH_DEFLIBRARY_PATH;

	return ret;
}

__attribute__((visibility("hidden")))
int __path_prependenv(const char *name, const char *value, int overwrite)
{
	char *newval, *oldval;
	char buf[PATH_MAX];

	if (!name || !value)
		return __set_errno(EINVAL, -1);

	newval = __strncpy(buf, value);
	oldval = getenv(name);
	if (oldval && *oldval) {
		if (*buf)
			__strncat(buf, ":");
		__strncat(buf, oldval);
	}

	return setenv(name, newval, overwrite);
}

__attribute__((visibility("hidden")))
int __path_appendenv(const char *name, const char *value, int overwrite)
{
	char *newval, *oldval;
	char buf[PATH_MAX];

	if (!name || !value)
		return __set_errno(EINVAL, -1);

	oldval = getenv(name);
	if (!oldval)
		return setenv(name, value, overwrite);

	newval = __strncpy(buf, oldval);
	if (*value) {
		if (*buf)
			__strncat(buf, ":");
		__strncat(buf, value);
	}

	return setenv(name, newval, overwrite);
}

int __path_setenv(const char *root, const char *name, const char *value,
		  int overwrite)
{
	size_t rootlen, vallen, newlen;

	if (!name || !value)
		return __set_errno(EINVAL, -1);

	if (!root)
		goto setenv;

	newlen = 0;
	rootlen = __strlen(root);

	vallen = __strlen(value);
	if (vallen > 0) {
		char val[vallen+1]; /* NULL-terminated */
		char *token, *saveptr;

		newlen = vallen;
		newlen += rootlen;
		newlen++; /* ensure NULL-terminated */

		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token)
			while (strtok_r(NULL, ":", &saveptr))
				newlen += rootlen;
	}

	if (newlen > 0) {
		char val[vallen+1], new_value[newlen+1]; /* NULL-terminated */
		char *str, *token, *saveptr;

		str = new_value;
		__strncpy(val, value);
		token = strtok_r(val, ":", &saveptr);
		if (token && *token) {
			int n;

			n = _snprintf(str, newlen, "%s%s", root, token);
			if ((n == -1) || (newlen < (size_t)n))
				return __set_errno(EOVERFLOW, -1);
			str += n;
			newlen -= n;
			while ((token = strtok_r(NULL, ":", &saveptr))) {
				n = _snprintf(str, newlen, ":%s%s", root, token);
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

static int __librarypath_callback(const char *library, void *user)
{
	const char *library_path = (const char *)user;
	char buf[PATH_MAX];
	ssize_t siz;

	/* ignore dynamic loaders */
	if (streq(library, "ld.so") || __strneq(library, "ld-"))
		return 0;

	if (*library != '/') {
		siz = __path_access(library, F_OK, library_path, buf,
				    sizeof(buf));
		if (siz == -1)
			__warning("%s: library not found in library-path %s\n",
				  library, library_path);

		goto exit;
	}

	siz = path_resolution(AT_FDCWD, library, buf, sizeof(buf), 0);
	if (siz == -1)
		return -1;

exit:
	return __path_appendenv("ld_preload", buf, 1);
}

static int __strtok(const char *str, const char *delim,
		    int (*callback)(const char *, void *), void *user)
{
	size_t len;

	if (!str || !callback)
		return __set_errno(EINVAL, -1);

	len = __strlen(str);
	if (len > 0) {
		char buf[len+1]; /* NULL-terminated */
		char *token, *saveptr;

		__strncpy(buf, str);
		token = strtok_r(buf, delim, &saveptr);
		do {
			int ret;

			if (!token)
				break;

			ret = callback(token, user);
			if (ret != 0)
				return ret;
		} while ((token = strtok_r(NULL, delim, &saveptr)));
	}

	return 0;
}

__attribute__((visibility("hidden")))
int __path_iterate(const char *path, int (*callback)(const char *, void *),
		 void *user)
{
	return __strtok(path, ":", callback, user);
}

static regex_t *re_ignore;

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

__attribute__((constructor,visibility("hidden")))
void execve_init()
{
	static __regex_t regex_ignore;
	const char *ignore;
	int ret;

#ifdef __linux__
	if (__secure())
		__warning("%s: secure-execution mode\n", __execfn());
#endif

	if (re_ignore)
		return;

	ignore = getenv("IAMROOT_EXEC_IGNORE");
	if (!ignore)
		ignore = "ldd";

	ret = regcomp(&regex_ignore.re, ignore, REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_ignore.re, ret);
		return;
	}

	re_ignore = &regex_ignore.re;
}

__attribute__((destructor,visibility("hidden")))
void execve_fini()
{
	if (!re_ignore)
		return;

	regfree(re_ignore);
	re_ignore = NULL;
}

static int ignore(const char *path)
{
	int ret = 0;

	if (!re_ignore)
		return 0;

	ret = regexec(re_ignore, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ignore, ret);
		return 0;
	}

	return !ret;
}

__attribute__((visibility("hidden")))
int __exec_ignored(const char *path)
{
	return ignore(path);
}

static int __ld_linux_version(const char *path, int *major, int *minor)
{
	const char *basename;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_FOLLOW);
	if (siz == -1)
		return -1;

	basename = __basename(buf);
	ret = sscanf(basename, "ld-%i.%i.so", major, minor);
	if (ret < 2)
		return __set_errno(ENOTSUP, -1);

	return 0;
}

static int __ld_linux_has_inhibit_cache_option(const char *path)
{
	int ret, maj = 0, min = 0;

	ret = __ld_linux_version(path, &maj, &min);
	if ((ret == -1) && (errno != ENOTSUP))
		return -1;

	errno = 0;
	if (ret == -1)
		return 1;

	/* --inhibit-cache is supported since glibc 2.16 */
	return (maj > 2) || ((maj == 2) && (min >= 16));
}

static int __ld_linux_has_argv0_option(const char *path)
{
	int ret, maj = 0, min = 0;

	ret = __ld_linux_version(path, &maj, &min);
	if ((ret == -1) && (errno != ENOTSUP))
		return -1;

	errno = 0;
	if (ret == -1)
		return 1;

	/* --argv0 is supported since glibc 2.33 */
	return (maj > 2) || ((maj == 2) && (min >= 33));
}

static int __ld_linux_has_preload_option(const char *path)
{
	int ret, maj = 0, min = 0;

	ret = __ld_linux_version(path, &maj, &min);
	if ((ret == -1) && (errno != ENOTSUP))
		return -1;

	errno = 0;
	if (ret == -1)
		return 1;

	/* --preload is supported since glibc 2.30 */
	return (maj > 2) || ((maj == 2) && (min >= 30));
}

__attribute__((visibility("hidden")))
int __issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, 0);
	if (ret == -1)
		return -1;

	return (statbuf.st_mode & S_ISUID) != 0;
}

static ssize_t __getinterp32(int fd, Elf32_Ehdr *ehdr, char *buf,
			     size_t bufsize)
{
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Look for the .interp section */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Phdr phdr;

		ret = pread(fd, &phdr, sizeof(phdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(phdr)) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .interp section */
		if (phdr.p_type != PT_INTERP)
			continue;

		if (bufsize < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		/* copy the NULL-terminated string from the .interp section */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		errno = 0;
		goto exit;
	}

	ret = __set_errno(ENOEXEC, -1);

exit:
	return ret;
}

static ssize_t __getinterp64(int fd, Elf64_Ehdr *ehdr, char *buf,
			     size_t bufsize)
{
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Look for the .interp section */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr phdr;

		ret = pread(fd, &phdr, sizeof(phdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(phdr)) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .interp section */
		if (phdr.p_type != PT_INTERP)
			continue;

		if (bufsize < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		/* copy the NULL-terminated string from the .interp section */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		errno = 0;
		goto exit;
	}

	ret = __set_errno(ENOEXEC, -1);

exit:
	return ret;
}

static int __dl_iterate_ehdr32(int fd, Elf32_Ehdr *ehdr, int dt_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	const int errno_save = errno;
	size_t strtab_siz = 0;
	off_t strtab_off = 0;
	Elf32_Dyn dyn[1024];
	ssize_t siz = -1;
	int ret = -1;
	int i, num;
	off_t off;

	if (!callback)
		ret = __set_errno(EINVAL, -1);

	/* Look for the .shstrtab section */
	off = ehdr->e_shoff;
	num = ehdr->e_shnum;
	for (i = 0; i < num; i++) {
		Elf32_Shdr shdr;

		siz = pread(fd, &shdr, sizeof(shdr), off);
		if (siz == -1) {
			goto exit;
		} else if ((size_t)siz < sizeof(shdr)) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		off += sizeof(shdr);

		/* Not the .shstrtab section */
		if (shdr.sh_type != SHT_STRTAB)
			continue;

		strtab_siz = shdr.sh_size;
		strtab_off = shdr.sh_offset;
		break;
	}

	/* No .shstrtab section */
	if (!strtab_off)
		goto exit;

	/* Look for the string entry in the .dynamic segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Phdr phdr;
		unsigned int j;
		size_t str_siz;
		off_t str_off;

		siz = pread(fd, &phdr, sizeof(phdr), off);
		if (siz == -1) {
			goto exit;
		} else if ((size_t)siz < sizeof(phdr)) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .dynamic segment */
		if (phdr.p_type != PT_DYNAMIC)
			continue;

		if (sizeof(dyn) < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		/* copy the .dynamic segment */
		siz = pread(fd, dyn, phdr.p_filesz, phdr.p_offset);
		if (siz == -1) {
			goto exit;
		} else if ((size_t)siz < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		/* look for the string entry */
		for (j = 0; j < phdr.p_filesz / sizeof(dyn[0]); j++) {
			char buf[BUFSIZ];
			size_t size;

			if ((int)dyn[j].d_tag != dt_tag)
				continue;

			/*
			 * copy the NULL-terminated string from the .strtab
			 * table
			 */
			str_off = strtab_off + dyn[j].d_un.d_val;
			str_siz = strtab_siz - dyn[j].d_un.d_val;
			size = __min(str_siz, sizeof(buf));
			siz = pread(fd, buf, size, str_off);
			if (siz == -1) {
				goto exit;
			} else if ((size_t)siz < size) {
				ret = __set_errno(EIO, -1);
				goto exit;
			}

			ret = callback(buf, size, data);
			if (ret != 0)
				break;
		}
	}

	ret = 0;

exit:
	if (ret != -1)
		errno = errno_save;

	return ret;
}

static int __dl_iterate_ehdr64(int fd, Elf64_Ehdr *ehdr, int dt_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	const int errno_save = errno;
	size_t strtab_siz = 0;
	off_t strtab_off = 0;
	Elf64_Dyn dyn[1024];
	ssize_t siz = -1;
	int ret = -1;
	int i, num;
	off_t off;

	if (!callback)
		ret = __set_errno(EINVAL, -1);

	/* Look for the .shstrtab section */
	off = ehdr->e_shoff;
	num = ehdr->e_shnum;
	for (i = 0; i < num; i++) {
		Elf64_Shdr shdr;

		siz = pread(fd, &shdr, sizeof(shdr), off);
		if (siz == -1) {
			goto exit;
		} else if ((size_t)siz < sizeof(shdr)) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		off += sizeof(shdr);

		/* Not the .shstrtab section */
		if (shdr.sh_type != SHT_STRTAB)
			continue;

		strtab_siz = shdr.sh_size;
		strtab_off = shdr.sh_offset;
		break;
	}

	/* No .shstrtab section */
	if (!strtab_off)
		goto exit;

	/* Look for the string entry in the .dynamic segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr phdr;
		unsigned int j;
		size_t str_siz;
		off_t str_off;

		siz = pread(fd, &phdr, sizeof(phdr), off);
		if (siz == -1) {
			goto exit;
		} else if ((size_t)siz < sizeof(phdr)) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .dynamic segment */
		if (phdr.p_type != PT_DYNAMIC)
			continue;

		if (sizeof(dyn) < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		/* copy the .dynamic segment */
		siz = pread(fd, dyn, phdr.p_filesz, phdr.p_offset);
		if (siz == -1) {
			goto exit;
		} else if ((size_t)siz < phdr.p_filesz) {
			ret = __set_errno(EIO, -1);
			goto exit;
		}

		/* look for the string entry */
		for (j = 0; j < phdr.p_filesz / sizeof(dyn[0]); j++) {
			char buf[BUFSIZ];
			size_t size;

			if ((int)dyn[j].d_tag != dt_tag)
				continue;

			/*
			 * copy the NULL-terminated string from the .strtab
			 * table
			 */
			str_off = strtab_off + dyn[j].d_un.d_val;
			str_siz = strtab_siz - dyn[j].d_un.d_val;
			size = __min(str_siz, sizeof(buf));
			siz = pread(fd, buf, size, str_off);
			if (siz == -1) {
				goto exit;
			} else if ((size_t)siz < size) {
				ret = __set_errno(EIO, -1);
				goto exit;
			}

			ret = callback(buf, size, data);
			if (ret != 0)
				break;
		}
	}

	ret = 0;

exit:
	if (ret != -1)
		errno = errno_save;

	return ret;
}

static int __dl_iterate_shared_object(const char *path, int dt_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	int fd, ret = -1;
	Elf64_Ehdr ehdr;
	ssize_t siz;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	siz = read(fd, &ehdr, sizeof(ehdr));
	if (siz == -1) {
		goto close;
	} else if ((size_t)siz < sizeof(ehdr)) {
		ret = __set_errno(EIO, -1);
		goto close;
	}

	/* Not an ELF */
	if (memcmp(ehdr.e_ident, ELFMAG, 4) != 0) {
		ret = __set_errno(ENOEXEC, -1);
		goto close;
	}

	/* Not a linked program or shared object */
	if ((ehdr.e_type != ET_EXEC) && (ehdr.e_type != ET_DYN)) {
		__set_errno(ENOEXEC, -1);
		goto close;
	}

	/* It is a 32-bits ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		ret = __dl_iterate_ehdr32(fd, (Elf32_Ehdr *)&ehdr, dt_tag,
					  callback, data);
	/* It is a 64-bits ELF */
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		ret = __dl_iterate_ehdr64(fd, (Elf64_Ehdr *)&ehdr, dt_tag,
					  callback, data);
	/* It is an invalid ELF */
	else
		ret = __set_errno(ENOEXEC, -1);

close:
	__close(fd);

	return ret;
}

static ssize_t __getinterp(const char *path, char *buf, size_t bufsize)
{
	ssize_t siz, ret = -1;
	Elf64_Ehdr ehdr;
	int fd;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	siz = read(fd, &ehdr, sizeof(ehdr));
	if (siz == -1) {
		goto close;
	} else if ((size_t)siz < sizeof(ehdr)) {
		ret = __set_errno(EIO, -1);
		goto close;
	}

	/* Not an ELF */
	if (memcmp(ehdr.e_ident, ELFMAG, 4) != 0) {
		ret = __set_errno(ENOEXEC, -1);
		goto close;
	}

	/* Not a linked program or shared object */
	if ((ehdr.e_type != ET_EXEC) && (ehdr.e_type != ET_DYN)) {
		ret = __set_errno(ENOEXEC, -1);
		goto close;
	}

	/* It is a 32-bits ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		ret = __getinterp32(fd, (Elf32_Ehdr *)&ehdr, buf, bufsize);
	/* It is a 64-bits ELF */
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		ret = __getinterp64(fd, (Elf64_Ehdr *)&ehdr, buf, bufsize);
	/* It is an invalid ELF */
	else
		ret = __set_errno(ENOEXEC, -1);

close:
	__close(fd);

	return ret;
}

static ssize_t __gethashbang(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	char *d, *s;
	int fd;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	ret = read(fd, buf, bufsize-1); /* NULL-terminated */
	if (ret == -1) {
		goto close;
	}
	buf[ret] = 0; /* ensure NULL-terminated */

	/* Not an hashbang interpreter directive */
	if ((ret < 2) || (buf[0] != '#') || (buf[1] != '!')) {
		ret = __set_errno(ENOEXEC, -1);
		goto close;
	}

	s = buf+2;
	d = buf;

	/* skip leading blanks */
	while (isblank(*s))
		s++;
	/* copy interpreter */
	while (*s && *s != '\n' && !isblank(*s))
		*d++ = *s++;
	*d++ = 0;

	/* has an optional argument */
	if (isblank(*s)) {
		/* skip leading blanks */
		while (isblank(*s))
			s++;
		/* copy interpreter */
		while (*s && *s != '\n' && !isblank(*s))
			*d++ = *s++;
		*d++ = 0;
	}

	ret = d-buf-1;

close:
	__close(fd);

	return ret;
}

#if !defined(NVERBOSE)
__attribute__((visibility("hidden")))
void __verbose_exec(const char *path, char * const argv[], char * const envp[])
{
	int color, fd, debug;

	debug = __getdebug();
	if (debug == 0)
		return;

	fd = __getdebug_fd();
	if (fd < 0)
		return;

	color = __getcolor();
	if (color)
		dprintf(fd, "\033[32;1m");

	dprintf(fd, "Debug: ");

	if (color)
		dprintf(fd, "\033[0m");

	if (debug < 4) {
		char * const *p;

		dprintf(fd, "running");

		p = envp;
		while (*p)
			dprintf(fd, " \"%s\"", *p++);

		p = argv;
		while (*p)
			dprintf(fd, " \"%s\"", *p++);
		dprintf(fd, "\n");
	} else {
		char * const *p;

		dprintf(fd, "execve(path: '%s', argv: {", path);

		p = argv;
		while (*p)
			dprintf(fd, " '%s',", *p++);
		dprintf(fd, " NULL }, ");
		if (debug < 5) {
			dprintf(fd, "envp: %p)\n", envp);
		} else {
			dprintf(fd, "envp: %p {", envp);
			p = envp;
			while (*p)
				dprintf(fd, " %s", *p++);
			dprintf(fd, " }\n");
		}
	}
}
#endif

__attribute__((visibility("hidden")))
const char *__getexe()
{
	const char *exec;
	size_t len;
	char *root;

	exec = __execfn();
	if (!exec)
		return NULL;

	len = 0;
	root = __getroot();
	if (root)
		len = __strlen(root);

	if (__strlen(exec) < len)
		return NULL;

	return &exec[len];
}

static int (*sym)(const char *, char * const[], char * const[]);

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "execve");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, argv, envp);
}

static void __env_sanitize(char *name, int upper)
{
	char *c;

	c = name;
	while (*c) {
		if (*c == '-')
			*c = '_';
		else if (upper)
			*c = toupper(*c);
		else
			*c = tolower(*c);
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
	__env_sanitize(buf, 1);

	ret = getenv(buf);
	if (ret)
		goto exit;

#ifdef __linux__
	/* IAMROOT_LIB_LINUX_2 */
	if (streq(ldso, "linux") && abi == 2) {
		ret = __xstr(PREFIX)"/lib/iamroot/i686/libiamroot-linux.so.2";
		goto exit;
	}

	/* IAMROOT_LIB_LINUX_X86_64_2 */
	if (streq(ldso, "linux-x86-64") && abi == 2) {
		ret = __xstr(PREFIX)"/lib/iamroot/x86_64/libiamroot-linux-x86-64.so.2";
		goto exit;
	}

	/* IAMROOT_LIB_LINUX_3 */
	if (streq(ldso, "linux") && abi == 3) {
		ret = __xstr(PREFIX)"/lib/iamroot/arm/libiamroot-linux.so.3";
		goto exit;
	}

	/* IAMROOT_LIB_LINUX_ARMHF_3 */
	if (streq(ldso, "linux-armhf") && abi == 3) {
		ret = __xstr(PREFIX)"/lib/iamroot/armhf/libiamroot-linux-armhf.so.3";
		goto exit;
	}

	/* IAMROOT_LIB_LINUX_AARCH64_1 */
	if (streq(ldso, "linux-aarch64") && abi == 1) {
		ret = __xstr(PREFIX)"/lib/iamroot/aarch64/libiamroot-linux-aarch64.so.1";
		goto exit;
	}

	/* IAMROOT_LIB_MUSL_I386_1 */
	if (streq(ldso, "musl-i386") && abi == 1) {
		ret = __xstr(PREFIX)"/lib/iamroot/i686/libiamroot-musl-i386.so.1";
		goto exit;
	}

	/* IAMROOT_LIB_MUSL_X86_64_1 */
	if (streq(ldso, "musl-x86_64") && abi == 1) {
		ret = __xstr(PREFIX)"/lib/iamroot/x86_64/libiamroot-musl-x86_64.so.1";
		goto exit;
	}

	/* IAMROOT_LIB_MUSL_ARMHF_1 */
	if (streq(ldso, "musl-armhf") && abi == 1) {
		ret = __xstr(PREFIX)"/lib/iamroot/armhf/libiamroot-musl-armhf.so.1";
		goto exit;
	}

	/* IAMROOT_LIB_MUSL_AARCH64_1 */
	if (streq(ldso, "musl-aarch64") && abi == 1) {
		ret = __xstr(PREFIX)"/lib/iamroot/aarch64/libiamroot-musl-aarch64.so.1";
		goto exit;
	}
#endif

#ifdef __FreeBSD__
	/* IAMROOT_LIB_ELF_1 */
	if (streq(ldso, "elf") && abi == 1) {
#if defined(__x86_64__)
		ret = __xstr(PREFIX)"/lib/iamroot/amd64/libiamroot-elf.so.1";
#elif defined(__aarch64__)
		ret = __xstr(PREFIX)"/lib/iamroot/arm64/libiamroot-elf.so.1";
#else
		ret = __xstr(PREFIX)"/lib/iamroot/libiamroot-elf.so.1";
#endif
		goto exit;
	}
#endif

	ret = getenv("IAMROOT_LIB");
	if (!ret)
		ret = __xstr(PREFIX)"/lib/iamroot/libiamroot.so";

exit:
	if (setenv("IAMROOT_LIB", ret, 1))
		return NULL;

	return getenv("IAMROOT_LIB");
}

static char *__getld_preload(const char *ldso, int abi)
{
	char buf[NAME_MAX];
	char *ret;
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_LD_PRELOAD_%s_%i", ldso, abi);
	if (n == -1)
		return NULL;
	__env_sanitize(buf, 1);

	ret = getenv(buf);
	if (ret)
		return ret;

#ifdef __linux__
	/* IAMROOT_LD_PRELOAD_LINUX_$ARCH_$ABI */
	if (__strneq(ldso, "linux"))
		return "libc.so.6:libdl.so.2:libpthread.so.0";
#endif

#ifdef __FreeBSD__
	/* IAMROOT_LD_PRELOAD_ELF_1 */
	if (streq(ldso, "elf") && abi == 1)
		return "libc.so.7:libdl.so.1";
#endif

	return "";
}

static char *__getld_library_path(const char *ldso, int abi)
{
	char buf[NAME_MAX];
	char *ret;
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_LIBRARY_PATH_%s_%d", ldso,
		      abi);
	if (n == -1)
		return NULL;
	__env_sanitize(buf, 1);

	ret = getenv(buf);
	if (ret)
		return ret;

	ret = getenv("IAMROOT_LIBRARY_PATH");
	if (ret)
		return ret;

#ifdef __linux__
	/* IAMROOT_LIBRARY_PATH_LINUX_X86_64_2 */
	if (streq(ldso, "linux-x86-64") && abi == 2)
		return "/lib64:/usr/local/lib64:/usr/lib64";

	/* IAMROOT_LIBRARY_PATH_LINUX_AARCH64_1 */
	if (streq(ldso, "linux-aarch64") && abi == 1)
		return "/lib64:/usr/local/lib64:/usr/lib64";
#endif

	return "/lib:/usr/local/lib:/usr/lib";
}

static char *__ld_preload(const char *ldso, int abi)
{
	char path[PATH_MAX];
	char val[PATH_MAX];
	char *runpath;
	char *needed;
	char *rpath;
	int ret;

	__strncpy(path, __getld_library_path(ldso, abi));

	rpath = getenv("rpath");
	if (rpath) {
		if (*path)
			__strncat(path, ":");
		__strncat(path, rpath);
	}

	runpath = getenv("runpath");
	if (runpath) {
		if (*path)
			__strncat(path, ":");
		__strncat(path, runpath);
	}

	__strncpy(val, __getld_preload(ldso, abi));

	needed = getenv("needed");
	if (needed) {
		if (*val)
			__strncat(val, ":");
		__strncat(val, needed);
	}

	ret = unsetenv("ld_preload");
	if (ret)
		return NULL;

	ret = __path_iterate(val, __librarypath_callback, path);
	if (ret == -1)
		return NULL;

	__strncpy(val, __getlibiamroot(ldso, abi));
	ret = __path_prependenv("ld_preload", val, 1);
	if (ret == -1)
		return NULL;

	return getenv("ld_preload");
}

static char *__ld_library_path(const char *ldso, int abi)
{
	char val[PATH_MAX];
	char *runpath;
	char *rpath;
	int ret;

	__strncpy(val, __getld_library_path(ldso, abi));
	ret = __path_setenv(__getrootdir(), "ld_library_path", val, 1);
	if (ret == -1)
		return NULL;

	rpath = getenv("rpath");
	if (rpath) {
		__strncpy(val, rpath);
		ret = __path_setenv(__getrootdir(), "iamroot_rpath", val, 1);
		if (ret == -1)
			return NULL;

		__strncpy(val, getenv("iamroot_rpath"));
		ret = __path_prependenv("ld_library_path", val, 1);
		if (ret == -1)
			return NULL;
	}

	runpath = getenv("runpath");
	if (runpath) {
		__strncpy(val, runpath);
		ret = __path_setenv(__getrootdir(), "iamroot_runpath", val, 1);
		if (ret == -1)
			return NULL;

		__strncpy(val, getenv("iamroot_runpath"));
		ret = __path_prependenv("ld_library_path", val, 1);
		if (ret == -1)
			return NULL;
	}

	return getenv("ld_library_path");
}

static int __path_callback(const void *data, size_t size, void *user)
{
	const char *path = (const char *)data;
	char *needed = (char *)user;
	(void)size;

	if (!path || !user)
		return __set_errno(EINVAL, -1);

	if (*needed)
		_strncat(needed, ":", PATH_MAX);
	_strncat(needed, path, PATH_MAX);

	return 0;
}

__attribute__((visibility("hidden")))
char *__needed(const char *path)
{
	char buf[PATH_MAX];
	int ret;

	*buf = 0;
	ret = __dl_iterate_shared_object(path, DT_NEEDED, __path_callback,
					 buf);
	if (ret == -1)
		return NULL;

	ret = setenv("needed", buf, 1);
	if (ret)
		return NULL;

	return getenv("needed");
}

static char *__rpath(const char *path)
{
	char buf[PATH_MAX];
	int ret;

	*buf = 0;
	ret = __dl_iterate_shared_object(path, DT_RPATH, __path_callback, buf);
	if (ret == -1)
		return NULL;

	ret = setenv("rpath", buf, 1);
	if (ret)
		return NULL;

	return getenv("rpath");
}

static char *__runpath(const char *path)
{
	char buf[PATH_MAX];
	int ret;

	*buf = 0;
	ret = __dl_iterate_shared_object(path, DT_RUNPATH, __path_callback,
					 buf);
	if (ret == -1)
		return NULL;

	ret = setenv("runpath", buf, 1);
	if (ret)
		return NULL;

	return getenv("runpath");
}

static char *__inhibit_rpath()
{
	char *inhibit_rpath;
	char val[PATH_MAX];

	inhibit_rpath = getenv("IAMROOT_INHIBIT_RPATH");
	if (inhibit_rpath) {
		int ret;

		__strncpy(val, inhibit_rpath);
		ret = __path_setenv(__getrootdir(), "inhibit_rpath", val, 1);
		if (ret == -1)
			return NULL;
	}

	return getenv("inhibit_rpath");
}

static char *__getexec()
{
	char *ret;

	ret = getenv("IAMROOT_EXEC");
#ifdef __linux__
	if (!ret)
		return "/usr/lib/iamroot/exec.sh";
#else
	if (!ret)
		return "/usr/local/lib/iamroot/exec.sh";
#endif

	return ret;
}

__attribute__((visibility("hidden")))
int __execve(const char *path, char * const argv[], char * const envp[])
{
	const char *root;
	ssize_t len;

	root = __getrootdir();
	if (streq(root, "/"))
		goto exit;

	len = __strlen(root);
	if (strneq(path, root, len))
		path += len;

exit:
	return execve(path, argv, envp);
}

__attribute__((visibility("hidden")))
int __hashbang(const char *path, char * const argv[], char *interp,
	       size_t interpsiz, char *interparg[])
{
	char *xargv1;
	ssize_t siz;
	size_t len;
	int i = 0;
	(void)argv;

	/* Get the interpeter directive stored after the hashbang */
	siz = __gethashbang(path, interp, interpsiz);
	if (siz < 1)
		return siz;

	/* Reset argv0 */
	interparg[i++] = interp; /* hashbang as argv0 */

	/* Add extra argument */
	xargv1 = getenv("IAMROOT_EXEC_HASHBANG_ARGV1");
	if (xargv1)
		interparg[i++] = xargv1; /* extra argument as argv1 */
	/* Add optional argument */
	len = strnlen(interp, interpsiz);
	if (len < (size_t)siz && interp[len+1])
		interparg[i++] = &interp[len+1];
	interparg[i++] = (char *)path; /* FIXME: original program path as first
					* positional argument */
	interparg[i] = NULL; /* ensure NULL-terminated */

	return i;
}

__attribute__((visibility("hidden")))
int __loader(const char *path, char * const argv[], char *interp,
	     size_t interpsiz, char *interparg[])
{
	char buf[HASHBANG_MAX];
	ssize_t siz;
	(void)argv;

	/*
	 * Get the dynamic loader stored in the .interp section of the ELF
	 * linked program.
	 */
	siz = __getinterp(path, buf, sizeof(buf));
	if (siz < 1)
		return siz;

	/*
	 * The interpreter has to preload its libiamroot.so library.
	 */
	if (__strneq(buf, "/lib/ld") || __strneq(buf, "/lib64/ld")) {
		char *argv0, *xargv1, *needed, *rpath, *runpath,
		     *inhibit_rpath, *ld_library_path, *ld_preload;
		int has_argv0 = 1, has_preload = 1, has_inhibit_rpath = 0,
		    has_inhibit_cache = 0;
		int ret, i, j, shift = 1, abi = 0;
		const char *basename;
		char ldso[NAME_MAX];
		char * const *arg;

		basename = __basename(buf);
		ret = sscanf(basename, "ld-%" __xstr(NAME_MAX) "[^.].so.%i",
			     ldso, &abi);
		if (ret < 2)
			return __set_errno(ENOTSUP, -1);

		/*
		 * the glibc world supports --argv0 since 2.33, --preload since
		 * 2.30, --inhibit-cache since 2.16, and --inhibit-rpath since
		 * 2.0.94
		 */
		if (__strneq(ldso, "linux")) {
			has_inhibit_rpath = 1;

			has_inhibit_cache =
				      __ld_linux_has_inhibit_cache_option(buf);
			if (has_inhibit_cache == -1)
				return -1;

			has_argv0 = __ld_linux_has_argv0_option(buf);
			if (has_argv0 == -1)
				return -1;

			has_preload = __ld_linux_has_preload_option(buf);
			if (has_preload == -1)
				return -1;
		}

		needed = __needed(path);
		if (needed)
			__info("%s: NEEDED=%s\n", path, needed);

		rpath = __rpath(path);
		if (rpath)
			__info("%s: RPATH=%s\n", path, rpath);

		runpath = __runpath(path);
		if (runpath)
			__info("%s: RUNPATH=%s\n", path, runpath);

		inhibit_rpath = __inhibit_rpath();
		if (inhibit_rpath)
			__notice("%s: %s\n", "inhibit_rpath", inhibit_rpath);

		ld_library_path = __ld_library_path(ldso, abi);
		if (!ld_library_path)
			__warning("%s: is unset!\n", "ld_library_path");

		ld_preload = __ld_preload(ldso, abi);
		if (!ld_preload)
			__warning("%s: is unset!\n", "ld_preload");

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			return -1;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
		 *   - the optional extra argument as argv1.
		 *   - the option --ld-preload and its argument (i.e. the path
		 *     in host environment to the iamroot library and the path
		 *     in chroot environment to the interpreter's libc.so and
		 *     libdl.so to preload).
		 *   - the option --library-path and its argument (i.e. the
		 *     path in chroot environment to the libraries paths).
		 *   - the option --inhibit-rpath and its argument (i.e. the
		 *     path in host environment to the libbraries to inhibit).
		 *   - the option --inhibit-cache.
		 *   - the option --argv0 and its argument (i.e. the original
		 *     path in host to the binary).
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument).
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		argv0 = interparg[0];
		xargv1 = getenv("IAMROOT_EXEC_LD_ARGV1");
		if (xargv1)
			shift++;
		if (has_argv0)
			shift += 2;
		if (has_preload && ld_preload)
			shift += 2;
		if (ld_library_path)
			shift += 2;
		if (has_inhibit_rpath && inhibit_rpath)
			shift += 2;
		if (has_inhibit_cache)
			shift++;
		i = 0;
		for (arg = interparg; *arg; arg++)
			i++;
		for (j = i+shift; j > shift; j--)
			interparg[j] = interparg[j-shift];
		j = i;

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = interp;

		/* Add extra argument as argv1 */
		if (xargv1)
			interparg[i++] = xargv1;

		/*
		 * Add --preload and interpreter's libraries:
		 *  - libiamroot.so (from host)
		 *  - libc.so and libdl.so (from chroot)
		 */
		if (has_preload && ld_preload) {
			interparg[i++] = "--preload";
			interparg[i++] = ld_preload;

			ret = unsetenv("LD_PRELOAD");
			if (ret)
				return -1;
		} else if (ld_preload) {
			ret = setenv("LD_PRELOAD", ld_preload, 1);
			if (ret)
				return -1;
		}

		/* Add --library-path (chroot) */
		if (ld_library_path) {
			interparg[i++] = "--library-path";
			interparg[i++] = ld_library_path;
		}

		/* Add --inhibit-rpath (chroot) */
		if (has_inhibit_rpath && inhibit_rpath) {
			interparg[i++] = "--inhibit-rpath";
			interparg[i++] = inhibit_rpath;
		}

		/* Add --inhibit-cache */
		if (has_inhibit_cache)
			interparg[i++] = "--inhibit-cache";

		/* Add --argv0 and original argv0 */
		if (has_argv0) {
			interparg[i++] = "--argv0";
			interparg[i++] = argv0;
		} else {
			/*
			 * The dynamic loader does not support for the option
			 * --argv0; the value will be set by via the function
			 * __libc_start_main().
			 */
			ret = setenv("argv0", argv0, 1);
			if (ret)
				return -1;
		}

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = (char *)path;
		i += j;
		interparg[i] = NULL; /* ensure NULL-terminated */

		return i;
	} else {
		char *argv0, *needed, *rpath, *runpath, *ld_library_path,
		     *ld_preload;
		int ret, i, j, shift = 1, abi = 0;
		const char *basename;
		char ldso[NAME_MAX];
		char * const *arg;
		char *xargv1;

		basename = __basename(buf);
		ret = sscanf(basename, "ld-%" __xstr(NAME_MAX) "[^.].so.%i",
			     ldso, &abi);
		if (ret < 2)
			return __set_errno(ENOTSUP, -1);

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			return -1;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
		 *   - the optional extra argument as argv1.
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument).
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		argv0 = interparg[0];
		xargv1 = getenv("IAMROOT_EXEC_LD_ARGV1");
		if (xargv1)
			shift++;
		i = 0;
		for (arg = interparg; *arg; arg++)
			i++;
		for (j = i+shift; j > shift; j--)
			interparg[j] = interparg[j-shift];
		j = i;

		ret = setenv("argv0", argv0, 1);
		if (ret)
			return -1;

		needed = __needed(path);
		if (needed)
			__info("%s: NEEDED=%s\n", path, needed);

		rpath = __rpath(path);
		if (rpath)
			__info("%s: RPATH=%s\n", path, rpath);

		runpath = __runpath(path);
		if (runpath)
			__info("%s: RUNPATH=%s\n", path, runpath);

		ld_library_path = __ld_library_path(ldso, abi);
		if (ld_library_path) {
			ret = setenv("LD_LIBRARY_PATH", ld_library_path, 1);
			if (ret)
				return -1;

			ret = unsetenv("ld_library_path");
			if (ret)
				return -1;
		}

		ld_preload = __ld_preload(ldso, abi);
		if (ld_preload) {
			ret = setenv("LD_PRELOAD", ld_preload, 1);
			if (ret)
				return -1;

			ret = unsetenv("ld_preload");
			if (ret)
				return -1;
		}

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = interp;

		/* Add extra argument as argv1 */
		if (xargv1)
			interparg[i++] = xargv1;

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = (char *)path;
		i += j;
		interparg[i] = NULL; /* ensure NULL-terminated */

		return i;
	}

	__notice("%s: not handled\n", buf);
	return setenv("interp", buf, 1);
}

__attribute__((visibility("hidden")))
int __exec_sh(const char *path, char * const *argv, char **interparg,
	      char *buf, size_t bufsize)
{
	int ret, i;

	i = 0;
	interparg[i++] = _strncpy(buf, __getexec(), bufsize);
	interparg[i++] = (char *)path; /* original path as first positional
					* argument
					*/
	interparg[i] = NULL; /* ensure NULL-terminated */

	ret = setenv("argv0", *argv, 1);
	if (ret)
		return -1;

	ret = setenv("ld_preload", getenv("LD_PRELOAD") ?: "", 1);
	if (ret)
		return -1;

	ret = setenv("ld_library_path", getenv("LD_LIBRARY_PATH") ?: "", 1);
	if (ret)
		return -1;

	ret = unsetenv("LD_PRELOAD");
	if (ret)
		return -1;

	ret = unsetenv("LD_LIBRARY_PATH");
	if (ret)
		return -1;

	return i;
}

int execve(const char *path, char * const argv[], char * const envp[])
{
	char *interparg[15+1] = { NULL }; /*  0 ARGV0
					   *  1 /lib/ld.so
					   *  2 LD_LINUX_ARGV1
					   *  3 --preload
					   *  4 libiamroot.so:libc.so:libdl.so
					   *  5 --library-path
					   *  6 /usr/lib:/lib
					   *  7 --argv0
					   *  8 ARGV0
					   *  9 --inhibit-rpath
					   * 10 --inhibit-cache
					   * 11 /usr/lib/lib.so:/lib/lib.so
					   * 12 /bin/sh
					   * 13 HASHBANG_ARGV1
					   * 14 -x
					   * 15 script.sh
					   * 16 NULL-terminated
					   */
	char hashbang[HASHBANG_MAX];
	char hashbangbuf[PATH_MAX];
	char loaderbuf[PATH_MAX];
	char *program = NULL;
	char buf[PATH_MAX];
	char * const *arg;
	int argc, ret;
	ssize_t siz;

	/* Run exec.sh script */
	if (__exec_ignored(path))
		goto exec_sh;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return -1;

	__debug("%s(path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, path, buf, argv[0], argv[1], envp);

	interparg[0] = *argv; /* original argv0 as argv0 */
	program = buf;

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = __issuid(buf);
	if (ret == -1)
		return -1;
	else if (ret != 0)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!__inchroot()) {
		__execfd();
		__verbose_exec(path, argv, envp);
		return next_execve(path, argv, envp);
	}

	ret = __hashbang(program, argv, hashbang, sizeof(hashbang), interparg);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret < 1)
		goto loader;

	/* FIXME: __hashbang() should do the following; it must have original
	 * and resolved path. */
	interparg[ret-1] = (char *)path; /* original program path as first
					  * positional argument */

	/*
	 * Preserve original path in argv0 and set the interpreter and its
	 * optional argument (if any).
	 */
	siz = path_resolution(AT_FDCWD, hashbang, hashbangbuf,
			      sizeof(hashbangbuf), 0);
	if (siz == -1)
		return -1;

	program = hashbangbuf;

loader:
	/*
	 * Run the dynamic loader directly
	 */
	if (__strneq(path, "/usr/bin/ld.so") || __strneq(path, "/lib/ld") ||
	    __strneq(path, "/lib64/ld")) {
		__execfd();
		__verbose_exec(buf, argv, envp);
		return next_execve(buf, argv, envp);
	}

	ret = __loader(program, argv, loaderbuf, sizeof(loaderbuf), interparg);
	if ((ret == -1) && (errno != ENOEXEC))
		return -1;
	if (ret != -1)
		goto execve;

exec_sh:
	ret = __exec_sh(path, argv, interparg, buf, sizeof(buf));
	if (ret == -1)
		return -1;

execve:
	ret = setenv("IAMROOT_VERSION", __xstr(VERSION), 1);
	if (ret)
		return -1;

	argc = 1;
	arg = interparg;
	while (*arg++)
		argc++;
	arg = argv+1; /* skip original-argv0 */
	while (*arg++)
		argc++;

	if ((argc > 0) && (argc < ARG_MAX)) {
		char *nargv[argc+1]; /* NULL-terminated */
		char **narg;

		narg = nargv;
		arg = interparg;
		while (*arg)
			*narg++ = *arg++;
		arg = argv+1; /* skip original-argv0 */
		while (*arg)
			*narg++ = *arg++;
		*narg++ = NULL; /* ensure NULL-terminated */

		__execfd();
		__verbose_exec(*nargv, nargv, __environ);
		return next_execve(*nargv, nargv, __environ);
	}

	return __set_errno(EINVAL, -1);
}
