/*
 * Copyright 2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <link.h>
#include <elf.h>
#include <regex.h>

#include "iamroot.h"

typedef struct {
	regex_t re;
#ifndef JIMREGEXP_H
	char jimpad[40];
#endif
} __regex_t;

extern int next_faccessat(int, const char *, int);
extern int next_open(const char *, int, mode_t);
extern void *next_dlopen(const char *, int);

__attribute__((visibility("hidden")))
const char *__basename(const char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1; /* trailing-slash */
}

static regex_t *re_ldso;

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
void dso_init()
{
	static __regex_t regex_ldso;
	const char *ldso = "^ld(|-[[:alnum:]._-]+)\\.so(|\\.[[:digit:]]+)$";
	int ret;

	if (re_ldso)
		return;

	ret = regcomp(&regex_ldso.re, ldso, REG_NOSUB|REG_EXTENDED);
	if (ret == -1) {
		__regex_perror("regcomp", &regex_ldso.re, ret);
		return;
	}

	re_ldso = &regex_ldso.re;
}

__attribute__((destructor,visibility("hidden")))
void dso_fini()
{
	if (!re_ldso)
		return;

	regfree(re_ldso);
	re_ldso = NULL;
}

__attribute__((visibility("hidden")))
int __is_ldso(const char *path)
{
	int ret = 0;

	if (!re_ldso)
		return 0;

	ret = regexec(re_ldso, path, 0, NULL, 0);
	if (ret == -1) {
		__regex_perror("regexec", re_ldso, ret);
		return 0;
	}

	return !ret;
}

static char *__path_strncat(char *dst, const char *src, size_t siz)
{
	if (*dst)
		_strncat(dst, ":", siz);

	return _strncat(dst, src, siz);
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

static int __is_in_path_callback(const char *path, void *user)
{
	const char *p = (const char *)user;

	return strneq(path, p, PATH_MAX);
}

static int __is_in_path(const char *pathname, const char *path)
{
	return __path_iterate(path, __is_in_path_callback, (void *)pathname);
}

static int __so_is_lib(const char *path)
{
	return __strncmp(__basename(path), "lib") == 0;
}

/*
 * Stolen and hacked from musl (ldso/dynlink.c)
 *
 * SPDX-FileCopyrightText: The musl Contributors
 *
 * SPDX-License-Identifier: MIT
 */
/* The original function name is fixup_rpath() in the musl sources */
static int __variable_has_dynamic_string_tokens(const char *value)
{
	const char *s, *t;
	int n;

	if (!strchr(value, '$'))
		return 0;

	n = 0;
	s = value;
	while ((t = strchr(s, '$'))) {
		if (__strneq(t, "$ORIGIN") && __strneq(t, "${ORIGIN}"))
			return 1;
		if (__strneq(t, "$LIB") && __strneq(t, "${LIB}"))
			return 1;
		if (__strneq(t, "$PLATFORM") && __strneq(t, "${PLATFORM}"))
			return 1;
		s = t + 1;
		n++;
	}

	return n;
}

static ssize_t __getneeded(const char *, char *, size_t);
static ssize_t __getrpath(const char *, char *, size_t);
static ssize_t __getrunpath(const char *, char *, size_t);
static int __getflags_1(const char *, uint32_t *);
static ssize_t __getdeflib(const char *, char *, size_t);

static int __ldso_preload_needed(const char *, const char *, char *, size_t);

struct __ldso_preload_needed_context {
	const char *library_path;
	char *buf;
	size_t bufsize;
};

static int __ldso_preload_needed_callback(const char *needed, void *user)
{
	struct __ldso_preload_needed_context *ctx =
				  (struct __ldso_preload_needed_context *)user;
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	/* FIXME: skipping */
	if (*needed == '/')
		return 0;

	siz = __path_access(needed, F_OK, ctx->library_path, buf, sizeof(buf));
	if (siz == -1)
		return -1;

	/* Ignore none-libraries (i.e. linux-vdso.so.1, ld.so-ish...) */
	if (!__so_is_lib(buf))
		return 0;

	/* The shared object is already in the buffer */
	ret = __is_in_path(buf, ctx->buf);
	if (ret == -1 || ret == 1)
		return 0;

	/* Add the needed shared objects to buf */
	return __ldso_preload_needed(buf, ctx->library_path, ctx->buf,
				     ctx->bufsize);
}

static int __ldso_preload_needed(const char *path, const char *library_path,
				 char *buf, size_t bufsize)
{
	struct __ldso_preload_needed_context ctx;
	char needed[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = __getneeded(path, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	/* Add the needed shared objects to path first */
	ctx.library_path = library_path;
	ctx.buf = buf;
	ctx.bufsize = bufsize;
	ret = __path_iterate(needed, __ldso_preload_needed_callback, &ctx);
	if (ret == -1)
		return -1;

	/* Add the shared object to path then */
	__path_strncat(buf, path, bufsize);

	return 0;
}

static const char *__getlibiamroot(Elf64_Ehdr *, const char *, int);
static const char *__getld_preload(Elf64_Ehdr *, const char *, int abi);
static const char *__getld_library_path(Elf64_Ehdr *, const char *, int);

/*
 * Note: The library libiamroot.so is **NOT** linked to any library, even the
 * libc. Therefore, it has no DT_NEEDED shared objects; the libc.so, libdl.so
 * and libpthread.so **HAVE TO** be preloaded manually.
 */
static int __ldso_preload_libiamroot_so(Elf64_Ehdr *ehdr, const char *ldso,
					int abi, char *buf, size_t bufsize)
{
	struct __ldso_preload_needed_context ctx;
	const char *library_path;
	const char *needed;
	const char *path;

	path = __getlibiamroot(ehdr, ldso, abi);
	if (!path)
		return -1;

	library_path = __getld_library_path(ehdr, ldso, abi);
	if (!library_path)
		return -1;

	needed = __getld_preload(ehdr, ldso, abi);
	if (!needed)
		return -1;

	/* Add the shared object to buffer first */
	__path_strncat(buf, path, bufsize);

	/* Add the needed shared objects to buffer then */
	ctx.library_path = library_path;
	ctx.buf = buf;
	ctx.bufsize = bufsize;
	return __path_iterate(needed, __ldso_preload_needed_callback, &ctx);
}

static int __ldso_preload_executable(const char *path,
				     const char *library_path,
				     char *buf,
				     size_t bufsize)
{
	struct __ldso_preload_needed_context ctx;
	char needed[PATH_MAX];
	ssize_t siz;

	siz = __getneeded(path, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	/* Add the needed shared objects to path only */
	ctx.library_path = library_path;
	ctx.buf = buf;
	ctx.bufsize = bufsize;
	return __path_iterate(needed, __ldso_preload_needed_callback, &ctx);
}

static int __dl_is_opened(const char *path)
{
	struct link_map *dso;
	void *handle;
	int err;

	handle = next_dlopen(NULL, RTLD_NOW);
	if (!handle)
		return -1;

	err = dlinfo(handle, RTLD_DI_LINKMAP, &dso);
	if (err == -1)
		return -1;

	while (dso) {
		if (streq(path, dso->l_name))
			return 1;

		dso = dso->l_next;
	}

	return 0;
}

static int __dlopen_needed_callback(const char *needed, void *user)
{
	char *library_path = (char *)user;
	char buf[PATH_MAX];
	void *handle;
	ssize_t siz;
	int ret;

	siz = __path_access(needed, F_OK, library_path, buf, sizeof(buf));
	if (siz == -1)
		return -1;

	/* Open the needed shared objects first */
	ret = __dlopen_needed(buf);
	if (ret == -1)
		return -1;

	/* Open the shared object then */
	handle = next_dlopen(buf, RTLD_LAZY);
	if (!handle)
		return -1;

	return 0;
}

__attribute__((visibility("hidden")))
int __dlopen_needed(const char *path)
{
	char library_path[PATH_MAX];
	char needed[PATH_MAX];
	ssize_t siz;
	int ret;

	/* The shared object is already opened */
	ret = __dl_is_opened(path);
	if (ret == -1 || ret == 1)
		return ret;

	siz = __dl_library_path(path, library_path, sizeof(library_path));
	if (siz == -1)
		return -1;

	siz = __getneeded(path, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	/* Open the needed shared objects */
	return __path_iterate(needed, __dlopen_needed_callback, library_path);
}

static int __ld_ldso_abi(const char *path, char ldso[NAME_MAX], int *abi)
{
	int ret;

	ret = sscanf(__basename(path), "ld-%" __xstr(NAME_MAX) "[^.].so.%i",
		     ldso, abi);
	if (ret < 2)
		return __set_errno(ENOTSUP, -1);

	return 0;
}

static int __ld_linux_version(const char *path, int *major, int *minor)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_FOLLOW);
	if (siz == -1)
		return -1;

	ret = sscanf(__basename(buf), "ld-%i.%i.so", major, minor);
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

static ssize_t __getelfheader(int fd, Elf64_Ehdr *ehdr)
{
	ssize_t siz;
	int err;

	if (fd < 0 || !ehdr)
		return __set_errno(EINVAL, -1);

	err = lseek(fd, 0, SEEK_SET);
	if (err)
		return -1;

	siz = read(fd, ehdr, sizeof(*ehdr));
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(*ehdr))
		return __set_errno(EIO, -1);

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, 4) != 0)
		return __set_errno(ENOEXEC, -1);

	/* It is a 32-bits ELF */
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS32)
		return sizeof(Elf32_Ehdr);
	/* It is a 64-bits ELF */
	else if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return sizeof(Elf64_Ehdr);

	/* It is an invalid ELF */
	return __set_errno(ENOEXEC, -1);
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

static int __dl_iterate_ehdr32(int fd, Elf32_Ehdr *ehdr, int d_tag,
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

	/* Look for the dynamic entry in the .dynamic segment */
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

		/* look for the dynamic entry */
		for (j = 0; j < phdr.p_filesz / sizeof(dyn[0]); j++) {
			char buf[BUFSIZ];
			size_t size;

			if ((int)dyn[j].d_tag != d_tag)
				continue;

			/*
			 * dymamic entry is an integer
			 */
			if (d_tag != DT_NEEDED && d_tag != DT_SONAME &&
			    d_tag != DT_RPATH && d_tag != DT_RUNPATH) {
				ret = callback(&dyn[j].d_un.d_val,
					       sizeof(dyn[j].d_un.d_val),
					       data);
				if (ret != 0)
					break;

				continue;
			}

			/*
			 * dynamic entry is a string
			 *
			 * copy the NULL-terminated string from the .strtab
			 * table
			 */
			str_off = strtab_off + dyn[j].d_un.d_ptr;
			str_siz = strtab_siz - dyn[j].d_un.d_ptr;
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

static int __dl_iterate_ehdr64(int fd, Elf64_Ehdr *ehdr, int d_tag,
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

	/* Look for the dynamic entry in the .dynamic segment */
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

		/* look for the dynamic entry */
		for (j = 0; j < phdr.p_filesz / sizeof(dyn[0]); j++) {
			char buf[BUFSIZ];
			size_t size;

			if ((int)dyn[j].d_tag != d_tag)
				continue;

			/*
			 * dymamic entry is an integer
			 */
			if (d_tag != DT_NEEDED && d_tag != DT_SONAME &&
			    d_tag != DT_RPATH && d_tag != DT_RUNPATH) {
				ret = callback(&dyn[j].d_un.d_val,
					       sizeof(dyn[j].d_un.d_val),
					       data);
				if (ret != 0)
					break;

				continue;
			}

			/*
			 * dynamic entry is a string
			 *
			 * copy the NULL-terminated string from the .strtab
			 * table
			 */
			str_off = strtab_off + dyn[j].d_un.d_ptr;
			str_siz = strtab_siz - dyn[j].d_un.d_ptr;
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

static int __dl_iterate_shared_object(int fd, int d_tag,
		     int (*callback)(const void *, size_t, void *), void *data)
{
	Elf64_Ehdr ehdr;
	ssize_t siz;
	int err;

	err = lseek(fd, 0, SEEK_SET);
	if (err)
		return -1;

	siz = read(fd, &ehdr, sizeof(ehdr));
	if (siz == -1)
		return -1;
	else if ((size_t)siz < sizeof(ehdr))
		return __set_errno(EIO, -1);

	/* Not an ELF */
	if (memcmp(ehdr.e_ident, ELFMAG, 4) != 0)
		return __set_errno(ENOEXEC, -1);

	/* Not a linked program or shared object */
	if ((ehdr.e_type != ET_EXEC) && (ehdr.e_type != ET_DYN))
		return __set_errno(ENOEXEC, -1);

	/* It is a 32-bits ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		return __dl_iterate_ehdr32(fd, (Elf32_Ehdr *)&ehdr, d_tag,
					   callback, data);
	/* It is a 64-bits ELF */
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		return __dl_iterate_ehdr64(fd, (Elf64_Ehdr *)&ehdr, d_tag,
					   callback, data);

	/* It is an invalid ELF */
	return __set_errno(ENOEXEC, -1);
}

static ssize_t __fgetinterp(int fd, char *buf, size_t bufsize)
{
	Elf64_Ehdr ehdr;
	ssize_t siz;

	/* Get the ELF header */
	siz = __getelfheader(fd, &ehdr);
	if (siz == -1)
		return -1;

	/* It is a 32-bits ELF */
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		return __getinterp32(fd, (Elf32_Ehdr *)&ehdr, buf, bufsize);
	/* It is a 64-bits ELF */
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		return __getinterp64(fd, (Elf64_Ehdr *)&ehdr, buf, bufsize);

	/* It is an invalid ELF */
	return __set_errno(ENOEXEC, -1);
}

static ssize_t __getinterp(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fgetinterp(fd, buf, bufsize);

	__close(fd);

	return ret;
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

static int is_64_bits(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a 64-bits ELF */
	return ehdr && (ehdr->e_ident[EI_CLASS] == ELFCLASS64);
}

static int is_x86(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is an x86 ELF */
	return ehdr && (ehdr->e_machine == EM_386);
}

static int is_x86_64(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is an x86-64 ELF */
	return ehdr && (ehdr->e_machine == EM_X86_64);
}

static int is_arm(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is an ARM ELF */
	return ehdr && (ehdr->e_machine == EM_ARM);
}

static int is_aarch64(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is an AArch64 ELF */
	return ehdr && (ehdr->e_machine == EM_AARCH64);
}

static int is_riscv(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a RISC-V ELF */
	return ehdr && (ehdr->e_machine == EM_RISCV);
}

static int is_linux(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ldso;
	(void)abi;

	/* It is a GNU/Linux ELF */
	return ehdr && (ehdr->e_ident[EI_OSABI] == ELFOSABI_GNU);
}

static int is_gnu_linux(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	/* LINUX_$ARCH_$ABI or it is a GNU/Linux ELF */
	return __strneq(ldso, "linux") || is_linux(ehdr, ldso, abi);
}

static int is_musl(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	(void)ehdr;
	(void)abi;

	/* MUSL_$ARCH_$ABI */
	return __strneq(ldso, "musl");
}

static int is_freebsd(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	/* It is a FreeBSD ELF or ELF_1 */
	return (ehdr && (ehdr->e_ident[EI_OSABI] == ELFOSABI_FREEBSD)) ||
	       (streq(ldso, "elf") && abi == 1);
}

static const char *__getlibiamroot(Elf64_Ehdr *ehdr, const char *ldso,
				   int abi)
{
	const int errno_save = errno;
	char buf[NAME_MAX];
	int n, err;
	char *ret;

	/*
	 * Use the library set by the environment variable
	 * IAMROOT_LIB_<LDSO>_<ABI> if set.
	 */
	n = _snprintf(buf, sizeof(buf), "IAMROOT_LIB_%s_%i", ldso, abi);
	if (n == -1)
		return NULL;
	__env_sanitize(buf, 1);

	ret = getenv(buf);
	if (ret)
		goto exit;

	/* The variable is unset, keep going... */

	/*
	 * Use the library set by the environment variable IAMROOT_LIB  if set.
	 */
	ret = getenv("IAMROOT_LIB");
	if (ret)
		goto exit;

	/*
	 * Neither IAMROOT_LIB_<LDSO>_<ABI> nor IAMROOT_LIB are set; try to
	 * guess automagically the library.
	 */

	/* IAMROOT_LIB_LINUX_$ARCH_$ABI */
	if (is_gnu_linux(ehdr, ldso, abi)) {
		/* It is an x86 ELF or IAMROOT_LIB_LINUX_2 */
		if (is_x86(ehdr, ldso, abi) ||
		    (streq(ldso, "linux") && abi == 2)) {
			ret = __xstr(PREFIX)"/lib/iamroot/i686/libiamroot-linux.so.2";
			goto access;
		}

		/* It is an x86-64 ELF or IAMROOT_LIB_LINUX_X86_64_2 */
		if (is_x86_64(ehdr, ldso, abi) ||
		    (streq(ldso, "linux-x86-64") && abi == 2)) {
			ret = __xstr(PREFIX)"/lib/iamroot/x86_64/libiamroot-linux-x86-64.so.2";
			goto access;
		}

		/* It is an ARM ELF or IAMROOT_LIB_LINUX_3 */
		if (is_arm(ehdr, ldso, abi) ||
		    (streq(ldso, "linux") && abi == 3)) {
			ret = __xstr(PREFIX)"/lib/iamroot/arm/libiamroot-linux.so.3";
			goto access;
		}

		/* It is an ARM ELF or IAMROOT_LIB_LINUX_ARMHF_3 */
		if (is_arm(ehdr, ldso, abi) ||
		    (streq(ldso, "linux-armhf") && abi == 3)) {
			ret = __xstr(PREFIX)"/lib/iamroot/armhf/libiamroot-linux-armhf.so.3";
			goto access;
		}

		/* It is an AArch64 ELF or IAMROOT_LIB_LINUX_AARCH64_1 */
		if (is_aarch64(ehdr, ldso, abi) ||
		    (streq(ldso, "linux-aarch64") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/aarch64/libiamroot-linux-aarch64.so.1";
			goto access;
		}

		/* It is an RISC-V lp64d ELF or IAMROOT_LIB_LINUX_RISCV64_LP64D_1 */
		if ((is_riscv(ehdr, ldso, abi) && is_64_bits(ehdr, ldso, abi)) ||
		    (streq(ldso, "linux-riscv64-lp64d") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/riscv64/libiamroot-linux-riscv64-lp64d.so.1";
			goto access;
		}
	}

	/* IAMROOT_LIB_MUSL_$ARCH_$ABI */
	if (is_musl(ehdr, ldso, abi)) {
		/* It is an x86 ELF or IAMROOT_LIB_MUSL_I386_1 */
		if (is_x86(ehdr, ldso, abi) ||
		    (streq(ldso, "musl-i386") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/i686/libiamroot-musl-i386.so.1";
			goto access;
		}

		/* It is an x86-64 ELF or IAMROOT_LIB_MUSL_X86_64_1 */
		if (is_x86_64(ehdr, ldso, abi) ||
		    (streq(ldso, "musl-x86_64") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/x86_64/libiamroot-musl-x86_64.so.1";
			goto access;
		}	

		/* It is an ARM ELF or IAMROOT_LIB_MUSL_ARMHF_1 */
		if (is_arm(ehdr, ldso, abi) ||
		    (streq(ldso, "musl-armhf") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/armhf/libiamroot-musl-armhf.so.1";
			goto access;
		}

		/* It is an AArch64 ELF or IAMROOT_LIB_MUSL_AARCH64_1 */
		if (is_aarch64(ehdr, ldso, abi) ||
		    (streq(ldso, "musl-aarch64") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/aarch64/libiamroot-musl-aarch64.so.1";
			goto access;
		}

		/* It is an RISC-V ELF or IAMROOT_LIB_MUSL_RISCV64_1 */
		if ((is_riscv(ehdr, ldso, abi) && is_64_bits(ehdr, ldso, abi)) ||
		    (streq(ldso, "musl-riscv64") && abi == 1)) {
			ret = __xstr(PREFIX)"/lib/iamroot/riscv64/libiamroot-musl-riscv64.so.1";
			goto access;
		}
	}

	/* It is a FreeBSD ELF or IAMROOT_LIB_ELF_1 */
	if (!is_freebsd(ehdr, ldso, abi)) {
		/* It is an x86-64 ELF */
		if (is_x86_64(ehdr, ldso, abi)) {
			ret = __xstr(PREFIX)"/local/lib/iamroot/amd64/libiamroot-elf.so.1";
			goto access;
		}
		
		/* It is an AArch64 ELF */
		if (is_aarch64(ehdr, ldso, abi)) {
			ret = __xstr(PREFIX)"/local/lib/iamroot/arm64/libiamroot-elf.so.1";
			goto access;
		}
	}

	/* It is something else */
	ret = __xstr(PREFIX)"/lib/iamroot/libiamroot.so";

access:
	err = next_faccessat(AT_FDCWD, ret, X_OK);
	if (err == -1)
		ret = __xstr(PREFIX)"/lib/iamroot/libiamroot.so";

exit:
	if (setenv("IAMROOT_LIB", ret, 1))
		return NULL;

	return __set_errno(errno_save, getenv("IAMROOT_LIB"));
}

static const char *__getld_preload(Elf64_Ehdr *ehdr, const char *ldso, int abi)
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

	/* LINUX_$ARCH_$ABI or it is a GNU/Linux ELF */
	if (is_gnu_linux(ehdr, ldso, abi))
		return "libc.so.6:libdl.so.2:libpthread.so.0";

	/* It is a FreeBSD ELF or ELF_1 */
	if (is_freebsd(ehdr, ldso, abi))
		return "libc.so.7:libdl.so.1";

	/* It is something else */
	return "";
}

static const char *__getld_library_path(Elf64_Ehdr *ehdr, const char *ldso,
					int abi)
{
	char buf[NAME_MAX];
	char *ret;

	/*
	 * The shared object is an executable file.
	 */
	if (ldso && *ldso && abi != -1) {
		int n;

		/*
		 * Use the default library path set by the environment variable
		 * IAMROOT_LIBRARY_PATH_<LDSO>_<ABI> if set.
		 */
		n = _snprintf(buf, sizeof(buf), "IAMROOT_LIBRARY_PATH_%s_%d",
			      ldso, abi);
		if (n == -1)
			return NULL;
		__env_sanitize(buf, 1);

		ret = getenv(buf);
		if (ret)
			return ret;

		/* The variable is unset, keep going... */
	}

	/*
	 * Use the default library path set by the environment variable
	 * IAMROOT_LIBRARY_PATH if set.
	 */
	ret = getenv("IAMROOT_LIBRARY_PATH");
	if (ret)
		return ret;

	/*
	 * Neither IAMROOT_LIBRARY_<LDSO>_<ABI> nor IAMROOT_LIBRARY_PATH are
	 * set; try to guess automagically the standard library path for the
	 * GNU/Linux systems.
	 *
	 * According to dl.so(8):
	 *
	 * On some 64-bit architectures, the default paths for 64-bit shared
	 * objects are /lib64, and then /usr/lib64.
	 */

	/* It is a 64-bits GNU/Linux */
	if (is_gnu_linux(ehdr, ldso, abi) && is_64_bits(ehdr, ldso, abi))
		return "/lib64:/usr/local/lib64:/usr/lib64";

	/* It is something else */
	return "/lib:/usr/local/lib:/usr/lib";
}

static ssize_t __ld_library_path(const char *, char *, size_t);

/*
 * Note: This resolves all the needed shared objects to preload in order to
 * prevent from loading the shared objects from the host system.
*/
static char *__setenv_ld_preload(Elf64_Ehdr *ehdr, const char *ldso, int abi,
				 const char *path)
{
	char library_path[PATH_MAX];
	char buf[PATH_MAX];
	ssize_t siz;
	int err;

	*buf = 0;
	err = __ldso_preload_libiamroot_so(ehdr, ldso, abi, buf, sizeof(buf));
	if (err == -1)
		return NULL;

	siz = __ld_library_path(path, library_path, sizeof(library_path));
	if (siz == -1)
		return NULL;

	err = __ldso_preload_executable(path, library_path, buf, sizeof(buf));
	if (err == -1)
		return NULL;

	err = setenv("ld_preload", buf, 1);
	if (err == -1)
		return NULL;

	return getenv("ld_preload");
}

static int __secure_execution_mode()
{
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;
	int ret;

	/*
	 * Secure-execution mode
	 *
	 * For security reasons, if the dynamic linker determines that a binary
	 * should be run in secure-execution mode, the effects of some
	 * environment variables are voided or modified, and furthermore those
	 * environment variables are stripped from the environment, so that the
	 * program does not even see the definitions. Some of these environment
	 * variables affect the operation of the dynamic linker itself, and are
	 * described below. Other environment variables treated in this way
	 * include: GCONV_PATH, GETCONF_DIR, HOSTALIASES, LOCALDOMAIN, LOCPATH,
	 * MALLOC_TRACE, NIS_PATH, NLSPATH, RESOLV_HOST_CONF, RES_OPTIONS,
	 * TMPDIR, and TZDIR.
	 *
	 * A binary is executed in secure-execution mode if the AT_SECURE entry
	 * in the auxiliary vector (see getauxval(3)) has a nonzero value. This
	 * entry may have a non‐zero value for various reasons, including:
	 *
	 * •  The process's real and effective user IDs differ, or the real and
	 * effective group IDs differ. This typically occurs as a result of
	 * executing a set-user-ID or set-group-ID program.
	 *
	 * •  A process with a non-root user ID executed a binary that
	 * conferred capabilities to the process.
	 *
	 * •  A nonzero value may have been set by a Linux Security Module.
	 */
	ret = getresuid(&ruid, &euid, &suid);
	if (ret == -1)
		return -1;

	ret = getresgid(&rgid, &egid, &sgid);
	if (ret == -1)
		return -1;

	return ruid != euid || rgid != egid;
}

/*
 * Note: This resolves all the library path of the executable file in order to
 * prevent from loading the shared objects from the host system.
*/
static ssize_t __ld_library_path(const char *path, char *buf, size_t bufsize)
{
	char *ld_library_path;
	int err, has_runpath;
	char tmp[PATH_MAX];
	uint32_t flags_1;

	*buf = 0;

	/*
	 * According to dl.so(8)
	 *
	 * (5)  In the default path /lib, and then /usr/lib. (On some 64-bit
	 * architectures, the default paths for 64-bit shared objects are
	 * /lib64, and then /usr/lib64.) If the binary was linked with the
	 * -z nodeflib linker option, this step is skipped.
	 */
	err = __getflags_1(path, &flags_1);
	if (!(flags_1 & DF_1_NODEFLIB)) {
		err = __getdeflib(path, tmp, sizeof(tmp));
		if (err == -1)
			return -1;

		__path_strncat(buf, tmp, bufsize);
	}

	/*
	 * (4)  From the cache file /etc/ld.so.cache, which contains a compiled
	 * list of candidate shared objects previously found in the augmented
	 * library path. If, however, the binary was linked with the -z
	 * nodeflib linker option, shared objects in the default paths are
	 * skipped. Shared objects installed in hardware capability directories
	 * are preferred to other shared objects.
	 */
	/* TODO: This is not applicable, at least for now. */

	/*
	 * (3)  Using the directories specified in the DT_RUNPATH dynamic
	 * section attribute of the binary if present. Such directories are
	 * searched only to find those objects required by DT_NEEDED (direct
	 * dependencies) entries and do not apply to those objects' children,
	 * which must themselves have their own DT_RUNPATH entries. This is
	 * unlike DT_RPATH, which is applied to searches for all children in
	 * the dependency tree.
	 */
	err = __getrunpath(path, tmp, bufsize);
	if (err == -1)
		return -1;

	has_runpath = err > 0;
	if (has_runpath)
		__path_strncat(buf, tmp, bufsize);

	/*
	 * (2)  Using the environment variable LD_LIBRARY_PATH, unless the
	 * executable is being run in secure-execution mode (see below), in
	 * which case this variable is ignored.
	 */
	ld_library_path = getenv("LD_LIBRARY_PATH");
	if (ld_library_path && !__secure_execution_mode())
		__path_strncat(buf, ld_library_path, bufsize);

	/*
	 * (1)  Using the directories specified in the DT_RPATH dynamic section
	 * attribute of the binary if present and DT_RUNPATH attribute does not
	 * exist. Use of DT_RPATH is deprecated.
	 */
	if (!has_runpath) {
		err = __getrpath(path, tmp, sizeof(tmp));
		if (err == -1)
			return -1;

		__path_strncat(buf, tmp, bufsize);
	}

	return strnlen(buf, bufsize);
}

static char *__setenv_ld_library_path(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int err;

	siz = __ld_library_path(path, buf, sizeof(buf));
	if (siz == -1)
		return NULL;

	err = __path_setenv(__getrootdir(), "ld_library_path", buf, 1);
	if (err == -1)
		return NULL;

	return getenv("ld_library_path");
}

static int __flags_callback(const void *data, size_t size, void *user)
{
	uint32_t *flags = (uint32_t *)data;
	uint32_t *f = (uint32_t *)user;
	(void)size;

	if (!data || !user)
		return __set_errno(EINVAL, -1);

	*f = *flags;

	return 0;
}

static int __fgetflags_1(int fd, uint32_t *flags)
{
	*flags = 0;
	return __dl_iterate_shared_object(fd, DT_FLAGS_1, __flags_callback,
					  flags);
}

static int __getflags_1(const char *path, uint32_t *flags)
{
	int fd, ret;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fgetflags_1(fd, flags);

	__close(fd);

	return ret;
}

static int __path_callback(const void *data, size_t size, void *user)
{
	const char *path = (const char *)data;
	char *p = (char *)user;
	(void)size;

	if (!data || !user)
		return __set_errno(EINVAL, -1);

	__path_strncat(p, path, PATH_MAX);

	return 0;
}

static ssize_t __fgetneeded(int fd, char *buf, size_t bufsize)
{
	int err;

	*buf = 0;
	err = __dl_iterate_shared_object(fd, DT_NEEDED, __path_callback, buf);
	if (err == -1)
		return -1;

	return strnlen(buf, bufsize);
}

static ssize_t __getneeded(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fgetneeded(fd, buf, bufsize);

	__close(fd);

	return ret;
}

static ssize_t __fgetrpath(int fd, char *buf, size_t bufsize)
{
	int err, n;

	*buf = 0;
	err = __dl_iterate_shared_object(fd, DT_RPATH, __path_callback, buf);
	if (err == -1)
		return -1;

	n = __variable_has_dynamic_string_tokens(buf);
	if (n)
		__warning("%s: RPATH has dynamic %i string token(s): %s\n",
			  __fpath(fd), n, buf);

	return strnlen(buf, bufsize);
}

static ssize_t __getrpath(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fgetrpath(fd, buf, bufsize);

	__close(fd);

	return ret;
}

static ssize_t __fgetrunpath(int fd, char *buf, size_t bufsize)
{
	int err, n;

	*buf = 0;
	err = __dl_iterate_shared_object(fd, DT_RUNPATH, __path_callback, buf);
	if (err == -1)
		return -1;

	n = __variable_has_dynamic_string_tokens(buf);
	if (n)
		__warning("%s: RUNPATH has dynamic %i string token(s): %s\n",
			  __fpath(fd), n, buf);

	return strnlen(buf, bufsize);
}

static ssize_t __getrunpath(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fgetrunpath(fd, buf, bufsize);

	__close(fd);

	return ret;
}

static int __has_needed_callback(const char *needed, void *user)
{
	const char *filename = (const char *)user;

	return __strleq(needed, filename);
}

static int __has_needed(int fd, const char *filename)
{
	char needed[PATH_MAX];
	ssize_t siz;

	siz = __fgetneeded(fd, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	/* Search for the needed shared object */
	return __path_iterate(needed, __has_needed_callback, (void *)filename);
}

static int __getld_linux_so_callback(const char *needed, void *user)
{
	char *interp = (char *)user;

	/* It is not the linux dynamic loader */
	if (__strneq(needed, "ld-linux") == 0)
		return 0;

	_strncpy(interp, needed, HASHBANG_MAX);

	return 1;
}

static ssize_t __getld_linux_so(int fd, char *buf, size_t bufsize)
{
	const char *libc_so_6 = "libc.so.6";
	char needed[PATH_MAX];
	char tmp[PATH_MAX];
	ssize_t siz;
	int err;

	/* Search for the linux C library (libc.so.6) first */
	err = __has_needed(fd, libc_so_6);
	if (err == -1)
		return -1;
	if (err == 0)
		return __set_errno(ENOENT, -1);

	siz = __fgetneeded(fd, needed, sizeof(needed));
	if (siz == -1)
		return -1;

	siz = __path_access(libc_so_6, F_OK, "/lib:/lib64", tmp, sizeof(tmp));
	if (siz == -1)
		return -1;

	/* Search for the linux dynamic loader (ld-linux-$ARCH.so.$ABI) then */
	err = __path_iterate(needed, __getld_linux_so_callback, buf);
	if (err == -1)
		return -1;
	if (err == 0)
		return __set_errno(ENOENT, -1);

	return strnlen(buf, bufsize);
}

static ssize_t __fgetdeflib(int fd, char *buf, size_t bufsize)
{
	const int errno_save = errno;
	char interp[HASHBANG_MAX];
	const char *library_path;
	char ldso[NAME_MAX];
	int err, abi = 0;
	Elf64_Ehdr ehdr;
	ssize_t siz;

	/*
	 * Detects the default library path using the ELF header:
	 *  - the class (32-bit or 64-bit architectures)
	 *  - the operating system and ABI (UNIX System V, GNU or Linux alias,
	 *    FreeBSD ABIs)
	 * and using the interpreter in the PT_INTERP segment if the shared
	 * object is an executable file, or from the needed libc, or from the
	 * executable file.
	 */
	siz = __getelfheader(fd, &ehdr);
	if (siz == -1)
		return -1;

	siz = __fgetinterp(fd, interp, sizeof(interp));
	/* It is an ELF library */
	if (siz == -1 && errno == ENOEXEC) {
		__info("%s: dynamic loader not found, trying libc.so.6...\n",
		       __fpath(fd));
		siz = __getld_linux_so(fd, interp, sizeof(interp));
		if (siz == -1 && errno != ENOENT)
			return -1;
	}
	/* It is an ELF library linked against libc */
	if (siz == -1 && errno == ENOENT) {
		const char *path;

		path = __execfn();
		__info("%s: dynamic loader not found, trying executable file...\n",
		       path);
		siz = __getinterp(path, interp, sizeof(interp));
		if (siz == -1 && errno != ENOENT)
			return -1;
	}
	if (siz < 1) {
		__warning("%s: dynamic loader not found!\n", __fpath(fd));
		*ldso = __set_errno(errno_save, 0);
		goto library_path;
	}

	err = __ld_ldso_abi(interp, ldso, &abi);
	if (err == -1)
		return -1;

library_path:
	library_path = __getld_library_path(&ehdr, ldso, abi);
	if (!library_path)
		return -1;

	strncpy(buf, library_path, bufsize);

	return strnlen(buf, bufsize);
}

static ssize_t __getdeflib(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	int fd;

	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	ret = __fgetdeflib(fd, buf, bufsize);

	__close(fd);

	return ret;
}

__attribute__((visibility("hidden")))
ssize_t __dl_library_path(const char *path, char *buf, size_t bufsize)
{
	const char *library_path;
	char tmp[PATH_MAX];
	uint32_t flags_1;
	int has_runpath;
	ssize_t ret;

	if (!buf)
		return __set_errno(EINVAL, -1);

	*buf = 0;
	/* FIXME: The executable file RPATH is looked up twice! */
	if (!path)
		path = __execfn();

	/*
	 * According to dlopen(3)
	 *
	 * •  (ELF only) If the calling object (i.e., the shared library or
	 * executable from which dlopen() is called) contains a DT_RPATH tag,
	 * and does not contain a DT_RUNPATH tag, then the directories listed
	 * in the DT_RPATH tag are searched.
	 */
	/* The man-page is incomplete! */
	ret = __getrunpath(path, tmp, sizeof(tmp));
	if (ret == -1)
		return -1;

	/*
	 * According to glibc (elf/dl-load.c)
	 *
	 * When the object has the RUNPATH information we don't use any RPATHs.
	 */
	has_runpath = ret > 0;
	if (!has_runpath) {
		char rpath[PATH_MAX];

		/*
		 * According to glibc (elf/dl-load.c)
		 *
		 * First try the DT_RPATH of the dependent object that caused
		 * NAME to be loaded. Then that object's dependent, and on up.
		 */
		ret = __getrpath(path, rpath, sizeof(rpath));
		if (ret == -1)
			return -1;

		if (ret != 0)
			__path_strncat(buf, rpath, bufsize);

		/*
		 * According to glibc (elf/dl-load.c)
		 *
		 * If dynamically linked, try the DT_RPATH of the executable
		 * itself.
		 *
		 * According to glibc (elf/get-dynamic-info.h)
		 *
		 * If both RUNPATH and RPATH are given, the latter is ignored.
		 */
		ret = __getrunpath(__execfn(), rpath, sizeof(rpath));
		if (ret == -1)
			return -1;

		if (ret <= 0) {
			ret = __getrpath(__execfn(), rpath, sizeof(rpath));
			if (ret == -1)
				return -1;

			if (ret != 0)
				__path_strncat(buf, rpath, bufsize);
		}
	}

	/*
	 * •  If, at the time that the program was started, the environment
	 * variable LD_LIBRARY_PATH was defined to contain a colon-separated
	 * list of directories, then these are searched. (As a security
	 * measure, this variable is ignored for set-user-ID and set-group-ID
	 * programs.)
	 */
	/*
	 * According to glibc (elf/dl-load.c)
	 *
	 * Try the LD_LIBRARY_PATH environment variable.
	 */
	library_path = getenv("LD_LIBRARY_PATH");
	if (library_path && !__secure_execution_mode())
		__path_strncat(buf, library_path, bufsize);

	/*
	 * •  (ELF only) If the calling object contains a DT_RUNPATH tag, then
	 * the directories listed in that tag are searched.
	 */
	/*
	 * According to glibc (elf/dl-load.c)
	 *
	 * Look at the RUNPATH information for this binary.
	 */
	if (has_runpath)
		__path_strncat(buf, tmp, bufsize);

	/*
	 * •  The cache file /etc/ld.so.cache (maintained by ldconfig(8)) is
	 * checked to see whether it contains an entry for filename.
	 */
	/*
	 * According to glibc (elf/dl-load.c)
	 *
	 * Check the list of libraries in the file /etc/ld.so.cache, for
	 * compatibility with Linux's ldconfig program.
	 */
	/* TODO: This is not applicable, at least for now. */

	/*
	 * •  The directories /lib and /usr/lib are searched (in that order).
	 */
	/*
	 * According to glibc (elf/dl-load.c)
	 *
	 * Finally, try the default path.
	 */
	ret = __getflags_1(path, &flags_1);
	if (!(flags_1 & DF_1_NODEFLIB)) {
		ret = __getdeflib(path, tmp, sizeof(tmp));
		if (ret == -1)
			return -1;

		__path_strncat(buf, tmp, bufsize);
	}

	/*
	 * If the object specified by filename has dependencies on other shared
	 * objects, then these are also automatically loaded by the dynamic
	 * linker using the same rules. (This process may occur recursively, if
	 * those objects in turn have dependencies, and so on.)
	 */
	/* TODO: This is to be implemented, at least in a near future. */

	return strnlen(buf, bufsize);
}

static char *__setenv_inhibit_rpath()
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

__attribute__((visibility("hidden")))
int __loader(const char *path, char * const argv[], char *interp,
	     size_t interpsiz, char *interparg[])
{
	int fd, abi = 0, ret = -1;
	char buf[HASHBANG_MAX];
	char ldso[NAME_MAX];
	Elf64_Ehdr ehdr;
	ssize_t siz;
	(void)argv;

	/*
	 * According to execve(2):
	 *
	 * If the executable is a dynamically linked ELF executable, the
	 * interpreter named in the PT_INTERP segment is used to load the
	 * needed shared objects. This interpreter is typically
	 * /lib/ld-linux.so.2 for binaries linked with glibc (see
	 * ld-linux.so(8)).
	 */
	/* Open the executable file... */
	fd = next_open(path, O_RDONLY | O_CLOEXEC, 0);
	if (fd == -1)
		return -1;

	/* ... get the ELF header... */
	siz = __getelfheader(fd, &ehdr);
	if (siz == -1)
		goto close;

	/*
	 * ... and get the dynamic loader stored in the .interp section of the
	 * ELF linked program...
	 */
	siz = __fgetinterp(fd, buf, sizeof(buf));
	if (siz < 1)
		goto close;

	/* ... and gets its LDSO-name and ABI number */
	ret = __ld_ldso_abi(buf, ldso, &abi);
	if (ret == -1)
		goto close;

	/*
	 * The interpreter has to preload its libiamroot.so library.
	 */
	if (__strneq(buf, "/lib/ld") || __strneq(buf, "/lib64/ld")) {
		char *argv0, *inhibit_rpath, *ld_library_path, *ld_preload;
		int has_argv0 = 1, has_preload = 1, has_inhibit_rpath = 0,
		    has_inhibit_cache = 0;
		int i, j, shift = 1;
		char * const *arg;

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
				goto close;

			has_argv0 = __ld_linux_has_argv0_option(buf);
			if (has_argv0 == -1)
				goto close;

			has_preload = __ld_linux_has_preload_option(buf);
			if (has_preload == -1)
				goto close;
		}

		inhibit_rpath = __setenv_inhibit_rpath();
		if (inhibit_rpath)
			__notice("%s: %s\n", __xstr(inhibit_rpath),
				 inhibit_rpath);

		ld_library_path = __setenv_ld_library_path(path);
		if (!ld_library_path)
			__warning("%s: is unset!\n", __xstr(ld_library_path));

		ld_preload = __setenv_ld_preload(&ehdr, ldso, abi, path);
		if (!ld_preload)
			__warning("%s: is unset!\n", __xstr(ld_preload));

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			return -1;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
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

		/*
		 * Add --preload and interpreter's libraries:
		 *  - libiamroot.so (from host)
		 *  - libc.so, libdl.so and libpthread.so (from chroot)
		 */
		if (has_preload && ld_preload) {
			interparg[i++] = "--preload";
			interparg[i++] = ld_preload;

			ret = unsetenv("LD_PRELOAD");
			if (ret == -1)
				goto close;
		} else if (ld_preload) {
			ret = setenv("LD_PRELOAD", ld_preload, 1);
			if (ret == -1)
				goto close;
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
			if (ret == -1)
				goto close;
		}

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = (char *)path;
		i += j;
		interparg[i] = NULL; /* ensure NULL-terminated */

		ret = i;
	} else {
		char *argv0, *ld_library_path, *ld_preload;
		int i, j, shift = 1;
		char * const *arg;

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			goto close;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
		 *   - the path to the binary (i.e. the full path in chroot,
		 *     *not* including chroot; first positional argument).
		 * Note: the binary's arguments are the original argv shifted
		 *       by one (i.e. without argv0; following arguments).
		 */
		argv0 = interparg[0];
		i = 0;
		for (arg = interparg; *arg; arg++)
			i++;
		for (j = i+shift; j > shift; j--)
			interparg[j] = interparg[j-shift];
		j = i;

		ret = setenv("argv0", argv0, 1);
		if (ret)
			goto close;

		ld_library_path = __setenv_ld_library_path(path);
		if (ld_library_path) {
			ret = setenv("LD_LIBRARY_PATH", ld_library_path, 1);
			if (ret)
				goto close;

			ret = unsetenv("ld_library_path");
			if (ret)
				goto close;
		}

		ld_preload = __setenv_ld_preload(&ehdr, ldso, abi, path);
		if (ld_preload) {
			ret = setenv("LD_PRELOAD", ld_preload, 1);
			if (ret)
				goto close;

			ret = unsetenv("ld_preload");
			if (ret)
				goto close;
		}

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = interp;

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = (char *)path;
		i += j;
		interparg[i] = NULL; /* ensure NULL-terminated */

		ret = i;
	}

close:
	__close(fd);

	return ret;
}
