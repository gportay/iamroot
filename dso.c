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

#include "iamroot.h"

extern int next_faccessat(int, const char *, int);
extern int next_open(const char *, int, mode_t);
extern void *next_dlopen(const char *, int);

static const char *__basename(const char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1; /* trailing-slash */
}

static char *__path_strncat(char *dst, const char *src, size_t siz)
{
	if (*dst)
		_strncat(dst, ":", siz);

	return _strncat(dst, src, siz);
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
	if (oldval && *oldval)
		__path_strncat(buf, oldval, sizeof(buf));

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
	if (*value)
		__path_strncat(buf, value, sizeof(buf));

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

static ssize_t __getinterp(int fd, char *buf, size_t bufsize)
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

	/*
	 * According do dl.so(8):
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

static char *__ld_preload(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	char path[PATH_MAX];
	char val[PATH_MAX];
	char *runpath;
	char *needed;
	char *rpath;
	int ret;
	(void)ehdr;

	__strncpy(path, __getld_library_path(ehdr, ldso, abi));

	rpath = getenv("rpath");
	if (rpath)
		__path_strncat(path, rpath, sizeof(path));

	runpath = getenv("runpath");
	if (runpath)
		__path_strncat(path, runpath, sizeof(path));

	__strncpy(val, __getld_preload(ehdr, ldso, abi));

	needed = getenv("needed");
	if (needed)
		__path_strncat(val, needed, sizeof(val));

	ret = unsetenv("ld_preload");
	if (ret)
		return NULL;

	ret = __path_iterate(val, __librarypath_callback, path);
	if (ret == -1)
		return NULL;

	__strncpy(val, __getlibiamroot(ehdr, ldso, abi));
	ret = __path_prependenv("ld_preload", val, 1);
	if (ret == -1)
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

static char *__ld_library_path(Elf64_Ehdr *ehdr, const char *ldso, int abi)
{
	char *ld_library_path;
	char val[PATH_MAX];
	int flags_1, ret;
	char *runpath;
	char *rpath;

	/*
	 * According to dl.so(8)
	 *
	 * (5)  In the default path /lib, and then /usr/lib. (On some 64-bit
	 * architectures, the default paths for 64-bit shared objects are
	 * /lib64, and then /usr/lib64.) If the binary was linked with the
	 * -z nodeflib linker option, this step is skipped.
	 */
	flags_1 = strtol(getenv("flags_1") ?: "0", NULL, 0);
	if (!(flags_1 & DF_1_NODEFLIB)) {
		__strncpy(val, __getld_library_path(ehdr, ldso, abi));
		ret = __path_setenv(__getrootdir(), "ld_library_path", val, 1);
		if (ret == -1)
			return NULL;
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

	/*
	 * (2)  Using the environment variable LD_LIBRARY_PATH, unless the
	 * executable is being run in secure-execution mode (see below), in
	 * which case this variable is ignored.
	 */
	ld_library_path = getenv("LD_LIBRARY_PATH");
	if (ld_library_path && !__secure_execution_mode()) {
		__strncpy(val, ld_library_path);
		ret = __path_setenv(__getrootdir(), "iamroot_ld_library_path",
				    val, 1);
		if (ret == -1)
			return NULL;

		__strncpy(val, getenv("iamroot_ld_library_path"));
		ret = __path_prependenv("ld_library_path", val, 1);
		if (ret == -1)
			return NULL;
	}

	/*
	 * (1)  Using the directories specified in the DT_RPATH dynamic section
	 * attribute of the binary if present and DT_RUNPATH attribute does not
	 * exist. Use of DT_RPATH is deprecated.
	 */
	rpath = getenv("rpath");
	if (rpath && (!runpath || (runpath && !*runpath))) {
		__strncpy(val, rpath);
		ret = __path_setenv(__getrootdir(), "iamroot_rpath", val, 1);
		if (ret == -1)
			return NULL;

		__strncpy(val, getenv("iamroot_rpath"));
		ret = __path_prependenv("ld_library_path", val, 1);
		if (ret == -1)
			return NULL;
	}

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

static int __flags_1(int fd)
{
	char buf[BUFSIZ];
	uint32_t flags;
	int ret;

	ret = __dl_iterate_shared_object(fd, DT_FLAGS_1, __flags_callback,
					 &flags);
	if (ret == -1)
		return -1;

	ret = _snprintf(buf, sizeof(buf), "0x%x", flags);
	if (ret == -1)
		return -1;

	ret = setenv("flags_1", buf, 1);
	if (ret == -1)
		return -1;

	return flags;
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

__attribute__((visibility("hidden")))
ssize_t __getlibrary_path(const char *path, char *buf, size_t bufsize)
{
	const char *library_path;
	char tmp[PATH_MAX];
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
	if (library_path)
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
	/* TODO: This is to be reworked, at least in a close future. */
	library_path = __library_path();
	if (library_path)
		__path_strncat(buf, library_path, bufsize);

	/*
	 * If the object specified by filename has dependencies on other shared
	 * objects, then these are also automatically loaded by the dynamic
	 * linker using the same rules. (This process may occur recursively, if
	 * those objects in turn have dependencies, and so on.)
	 */
	/* TODO: This is to be implemented, at least in a near future. */

	return strnlen(buf, bufsize);
}

static char *__needed(int fd)
{
	char buf[PATH_MAX];
	ssize_t err;
	int ret;

	err = __fgetneeded(fd, buf, sizeof(buf));
	if (err == -1)
		return NULL;

	ret = setenv("needed", buf, 1);
	if (ret)
		return NULL;

	return getenv("needed");
}

static char *__rpath(int fd)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int n, ret;

	siz = __fgetrpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return NULL;

	n = __variable_has_dynamic_string_tokens(buf);
	if (n)
		__warning("%s: RPATH has dynamic %i string token(s): %s\n",
			  __fpath(fd), n, buf);

	ret = setenv("rpath", buf, 1);
	if (ret)
		return NULL;

	return getenv("rpath");
}

static char *__runpath(int fd)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int n, ret;

	siz = __fgetrunpath(fd, buf, sizeof(buf));
	if (siz == -1)
		return NULL;

	n = __variable_has_dynamic_string_tokens(buf);
	if (n)
		__warning("%s: RUNPATH has dynamic %i string token(s): %s\n",
			  __fpath(fd), n, buf);

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
	 * Open the shared object...
	 */
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
	siz = __getinterp(fd, buf, sizeof(buf));
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
		char *argv0, *xargv1, *needed, *rpath, *runpath,
		     *inhibit_rpath, *ld_library_path, *ld_preload;
		int has_argv0 = 1, has_preload = 1, has_inhibit_rpath = 0,
		    has_inhibit_cache = 0;
		int i, j, shift = 1;
		char * const *arg;
		int flags_1;

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

		flags_1 = __flags_1(fd);
		if (flags_1 & DF_1_NODEFLIB)
			__info("%s: FLAGS_1=0x%08x (DF_1_NODEFLIB)\n", path,
			       flags_1);

		needed = __needed(fd);
		if (needed)
			__info("%s: NEEDED=%s\n", path, needed);

		rpath = __rpath(fd);
		if (rpath)
			__info("%s: RPATH=%s\n", path, rpath);

		runpath = __runpath(fd);
		if (runpath)
			__info("%s: RUNPATH=%s\n", path, runpath);

		inhibit_rpath = __inhibit_rpath();
		if (inhibit_rpath)
			__notice("%s: %s\n", __xstr(inhibit_rpath),
				 inhibit_rpath);

		ld_library_path = __ld_library_path(&ehdr, ldso, abi);
		if (!ld_library_path)
			__warning("%s: is unset!\n", __xstr(ld_library_path));

		ld_preload = __ld_preload(&ehdr, ldso, abi);
		if (!ld_preload)
			__warning("%s: is unset!\n", __xstr(ld_preload));

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
		char *argv0, *xargv1, *needed, *rpath, *runpath,
		     *ld_library_path, *ld_preload;
		int i, j, shift = 1;
		char * const *arg;
		int flags_1;

		siz = path_resolution(AT_FDCWD, buf, interp, interpsiz, 0);
		if (siz == -1)
			goto close;

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
			goto close;

		flags_1 = __flags_1(fd);
		if (flags_1 & DF_1_NODEFLIB)
			__info("%s: FLAGS_1=0x%08x (DF_1_NODEFLIB)\n", path,
			       flags_1);

		needed = __needed(fd);
		if (needed)
			__info("%s: NEEDED=%s\n", path, needed);

		rpath = __rpath(fd);
		if (rpath)
			__info("%s: RPATH=%s\n", path, rpath);

		runpath = __runpath(fd);
		if (runpath)
			__info("%s: RUNPATH=%s\n", path, runpath);

		ld_library_path = __ld_library_path(&ehdr, ldso, abi);
		if (ld_library_path) {
			ret = setenv("LD_LIBRARY_PATH", ld_library_path, 1);
			if (ret)
				goto close;

			ret = unsetenv("ld_library_path");
			if (ret)
				goto close;
		}

		ld_preload = __ld_preload(&ehdr, ldso, abi);
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

		/* Add extra argument as argv1 */
		if (xargv1)
			interparg[i++] = xargv1;

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
