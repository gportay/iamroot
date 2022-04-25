/*
 * Copyright 2020-2022 Gaël PORTAY
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
#include <limits.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/auxv.h>
#include <fcntl.h>
#include <elf.h>
#include <regex.h>

#include <unistd.h>

#include "iamroot.h"

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

__attribute__((visibility("hidden")))
char *__basename(char *path)
{
	char *s = strrchr(path, '/');
	if (!s)
		return path;

	return s+1;
}

__attribute__((visibility("hidden")))
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
	rootlen = __strlen(root);

	vallen = __strlen(value);
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

			n = _snprintf(str, newlen, "%s%s", root, token);
			if ((n == -1) || (newlen < (size_t)n)) {
				errno = EOVERFLOW;
				return -1;
			}
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

__attribute__((optimize("O0")))
__attribute__((constructor,visibility("hidden")))
void execve_init()
{
	static regex_t regex;
#ifndef JIMREGEXP_H
	__attribute__((unused)) static char jimpad[40];
#endif
	const char *ignore;
	int ret;

#ifdef __linux__
	if (__secure())
		__warning("%s: secure-execution mode\n", __execfn());
#endif

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

__attribute__((destructor,visibility("hidden")))
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

__attribute__((visibility("hidden")))
int exec_ignored(const char *path)
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
	if (ret < 2) {
		errno = ENOTSUP;
		return -1;
	}

	return 0;
}

__attribute__((visibility("hidden")))
int __ld_linux_has_inhibit_cache_option(const char *path)
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

__attribute__((visibility("hidden")))
int __ld_linux_has_argv0_option(const char *path)
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

__attribute__((visibility("hidden")))
int __ld_linux_has_preload_option(const char *path)
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
int issuid(const char *path)
{
	struct stat statbuf;
	int ret = -1;

	ret = next_fstatat(AT_FDCWD, path, &statbuf, 0);
	if (ret == -1)
		return -1;

	return (statbuf.st_mode & S_ISUID) != 0;
}

static ssize_t getdynamicentry32(int fd, Elf32_Ehdr *ehdr, int dt_tag,
				 char *buf, size_t bufsize)
{
	size_t strtab_siz = 0;
	off_t strtab_off = 0;
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, 4) != 0) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Not a linked program or shared object */
	if ((ehdr->e_type != ET_EXEC) && (ehdr->e_type != ET_DYN)) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Look for the .shstrtab section */
	off = ehdr->e_shoff;
	num = ehdr->e_shnum;
	for (i = 0; i < num; i++) {
		Elf32_Shdr shdr;

		ret = pread(fd, &shdr, sizeof(shdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(shdr)) {
			errno = EIO;
			ret = -1;
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
	if (!strtab_off) {
		ret = -1;
		goto exit;
	}

	/* Look for the string entry in the .dynamic segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Dyn *dyn = (Elf32_Dyn *)buf;
		off_t str_off, val = 0;
		size_t str_siz = 0;
		unsigned int j;
		Elf32_Phdr phdr;

		ret = pread(fd, &phdr, sizeof(phdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(phdr)) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .dynamic segment */
		if (phdr.p_type != PT_DYNAMIC)
			continue;

		if (bufsize < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		/* copy the .dynamic segment */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		/* look for the string entry */
		for (j = 0; j < phdr.p_filesz / sizeof(*dyn); j++) {
			if (dyn[j].d_tag != dt_tag)
				continue;

			val = dyn[j].d_un.d_val;
			break;
		}

		/* No string entry */
		if (j == phdr.p_filesz / sizeof(*dyn))
			continue;

		/* copy the NULL-terminated string from the .strtab table */
		str_off = strtab_off + val;
		str_siz = strtab_siz - val;
		ret = pread(fd, buf, __min(str_siz, bufsize), str_off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < __min(str_siz, bufsize)) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		ret = strnlen(buf, bufsize);
		errno = 0;
		goto exit;
	}

	errno = ENOENT;
	ret = -1;

exit:
	return ret;
}

static ssize_t getdynamicentry64(int fd, Elf64_Ehdr *ehdr, int dt_tag,
				 char *buf, size_t bufsize)
{
	size_t strtab_siz = 0;
	off_t strtab_off = 0;
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, 4) != 0) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Not a linked program or shared object */
	if ((ehdr->e_type != ET_EXEC) && (ehdr->e_type != ET_DYN)) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Look for the .shstrtab section */
	off = ehdr->e_shoff;
	num = ehdr->e_shnum;
	for (i = 0; i < num; i++) {
		Elf64_Shdr shdr;

		ret = pread(fd, &shdr, sizeof(shdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(shdr)) {
			errno = EIO;
			ret = -1;
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
	if (!strtab_off) {
		ret = -1;
		goto exit;
	}

	/* Look for the string entry in the .dynamic segment */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Dyn *dyn = (Elf64_Dyn *)buf;
		off_t str_off, val = 0;
		size_t str_siz = 0;
		unsigned int j;
		Elf64_Phdr phdr;

		ret = pread(fd, &phdr, sizeof(phdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(phdr)) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .dynamic segment */
		if (phdr.p_type != PT_DYNAMIC)
			continue;

		if (bufsize < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		/* copy the .dynamic segment */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		/* look for the string entry */
		for (j = 0; j < phdr.p_filesz / sizeof(*dyn); j++) {
			if (dyn[j].d_tag != dt_tag)
				continue;

			val = dyn[j].d_un.d_val;
			break;
		}

		/* No string entry */
		if (j == phdr.p_filesz / sizeof(*dyn))
			continue;

		/* copy the NULL-terminated string from the .strtab table */
		str_off = strtab_off + val;
		str_siz = strtab_siz - val;
		ret = pread(fd, buf, __min(str_siz, bufsize), str_off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < __min(str_siz, bufsize)) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		ret = strnlen(buf, bufsize);
		errno = 0;
		goto exit;
	}

	errno = ENOENT;
	ret = -1;

exit:
	return ret;
}

static ssize_t getdynamicentry(const char *path, int dt_tag, char *buf,
			       size_t bufsize)
{
	ssize_t ret = -1;
	Elf64_Ehdr ehdr;
	int fd;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	ret = read(fd, &ehdr, sizeof(ehdr));
	if (ret == -1) {
		goto close;
	} else if ((size_t)ret < sizeof(ehdr)) {
		errno = EIO;
		ret = -1;
		goto close;
	}

	errno = ENOEXEC;
	ret = -1;
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		ret = getdynamicentry32(fd, (Elf32_Ehdr *)&ehdr, dt_tag, buf,
					bufsize);
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		ret = getdynamicentry64(fd, (Elf64_Ehdr *)&ehdr, dt_tag, buf,
					bufsize);

close:
	__close(fd);

	return ret;
}

static ssize_t getinterp32(int fd, Elf32_Ehdr *ehdr, char *buf, size_t bufsize)
{
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, 4) != 0) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Not a linked program or shared object */
	if ((ehdr->e_type != ET_EXEC) && (ehdr->e_type != ET_DYN)) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Look for the .interp section */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf32_Phdr phdr;

		ret = pread(fd, &phdr, sizeof(phdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(phdr)) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .interp section */
		if (phdr.p_type != PT_INTERP)
			continue;

		if (bufsize < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		/* copy the NULL-terminated string from the .interp section */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		errno = 0;
		goto exit;
	}

	errno = ENOEXEC;
	ret = -1;

exit:
	return ret;
}

static ssize_t getinterp64(int fd, Elf64_Ehdr *ehdr, char *buf, size_t bufsize)
{
	ssize_t ret = -1;
	int i, num;
	off_t off;

	/* Not an ELF */
	if (memcmp(ehdr->e_ident, ELFMAG, 4) != 0) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Not a linked program or shared object */
	if ((ehdr->e_type != ET_EXEC) && (ehdr->e_type != ET_DYN)) {
		errno = ENOEXEC;
		goto exit;
	}

	/* Look for the .interp section */
	off = ehdr->e_phoff;
	num = ehdr->e_phnum;
	for (i = 0; i < num; i++) {
		Elf64_Phdr phdr;

		ret = pread(fd, &phdr, sizeof(phdr), off);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < sizeof(phdr)) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		off += sizeof(phdr);

		/* Not the .interp section */
		if (phdr.p_type != PT_INTERP)
			continue;

		if (bufsize < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		/* copy the NULL-terminated string from the .interp section */
		ret = pread(fd, buf, phdr.p_filesz, phdr.p_offset);
		if (ret == -1) {
			goto exit;
		} else if ((size_t)ret < phdr.p_filesz) {
			errno = EIO;
			ret = -1;
			goto exit;
		}

		errno = 0;
		goto exit;
	}

	errno = ENOEXEC;
	ret = -1;

exit:
	return ret;
}

__attribute__((visibility("hidden")))
ssize_t getinterp(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret = -1;
	Elf64_Ehdr ehdr;
	int fd;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	ret = read(fd, &ehdr, sizeof(ehdr));
	if (ret == -1) {
		goto close;
	} else if ((size_t)ret < sizeof(ehdr)) {
		errno = EIO;
		ret = -1;
		goto close;
	}

	errno = ENOEXEC;
	ret = -1;
	if (ehdr.e_ident[EI_CLASS] == ELFCLASS32)
		ret = getinterp32(fd, (Elf32_Ehdr *)&ehdr, buf, bufsize);
	else if (ehdr.e_ident[EI_CLASS] == ELFCLASS64)
		ret = getinterp64(fd, (Elf64_Ehdr *)&ehdr, buf, bufsize);

close:
	__close(fd);

	return ret;
}

__attribute__((visibility("hidden")))
ssize_t gethashbang(const char *path, char *buf, size_t bufsize)
{
	ssize_t ret;
	char *d, *s;
	int fd;

	fd = next_open(path, O_RDONLY, 0);
	if (fd == -1)
		return -1;

	ret = read(fd, buf, bufsize-1);
	if (ret == -1) {
		goto close;
	}
	buf[ret] = 0; /* ensure NULL terminated */

	/* Not an hashbang interpreter directive */
	if ((ret < 2) || (buf[0] != '#') || (buf[1] != '!')) {
		errno = ENOEXEC;
		ret = -1;
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
void verbose_exec(const char *path, char * const argv[], char * const envp[])
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
		char *ld_library_path;
		char *ld_preload;
		char * const *p;
		char *argv0;
		char *root;

		dprintf(fd, "running");

		root = __getroot();
		if (root)
			dprintf(fd, " IAMROOT_ROOT=%s", root);

		ld_preload = getenv("LD_PRELOAD");
		if (ld_preload)
			dprintf(fd, " LD_PRELOAD=%s", ld_preload);

		ld_library_path = getenv("LD_LIBRARY_PATH");
		if (ld_library_path)
			dprintf(fd, " LD_LIBRARY_PATH=%s", ld_library_path);

		argv0 = getenv("argv0");
		if (argv0)
			dprintf(fd, " argv0=%s", argv0);

		p = argv;
		while (*p)
			dprintf(fd, " %s", *p++);
		dprintf(fd, "\n");
	} else {
		char * const *p;

		dprintf(fd, "%s: %s: pid: %i: execve(path: '%s', argv: {",
			__libc(), __arch(), getpid(), path);
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
#else
#define verbose_exec(path, argv, envp)
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

__attribute__((visibility("hidden")))
int next_execve(const char *path, char * const argv[], char * const envp[])
{
	int (*sym)(const char *, char * const[], char * const[]);
	int ret;

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

static void __sanitize(char *name, int upper)
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
	__sanitize(buf, 1);

	ret = getenv(buf);
	if (!ret) {
		if (__strncmp(buf, "IAMROOT_LIB_LINUX_2") == 0)
			return "/usr/lib/iamroot/i686/libiamroot-linux.so.2";
		else if (__strncmp(buf, "IAMROOT_LIB_LINUX_X86_64_2") == 0)
			return "/usr/lib/iamroot/x86_64/libiamroot-linux-x86-64.so.2";
		else if (__strncmp(buf, "IAMROOT_LIB_LINUX_ARMHF_3") == 0)
			return "/usr/lib/iamroot/armhf/libiamroot-linux-armhf.so.3";
		else if (__strncmp(buf, "IAMROOT_LIB_LINUX_AARCH64_1") == 0)
			return "/usr/lib/iamroot/aarch64/libiamroot-linux-aarch64.so.1";
		else if (__strncmp(buf, "IAMROOT_LIB_MUSL_X86_64_1") == 0)
			return "/usr/lib/iamroot/x86_64/libiamroot-musl-x86_64.so.1";
		else if (__strncmp(buf, "IAMROOT_LIB_MUSL_AARCH64_1") == 0)
			return "/usr/lib/iamroot/aarch64/libiamroot-musl-aarch64.so.1";
		else
			return "/usr/lib/iamroot/libiamroot.so";
	}

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
	__sanitize(buf, 1);

	ret = getenv(buf);
	if (!ret) {
		if (__strncmp(buf, "IAMROOT_LD_PRELOAD_LINUX_2") == 0)
			return "/usr/lib/libc.so.6:/usr/lib/libdl.so.2";
		else if (__strncmp(buf, "IAMROOT_LD_PRELOAD_LINUX_X86_64_2") == 0)
			return "/usr/lib64/libc.so.6:/usr/lib64/libdl.so.2";
		else if (__strncmp(buf, "IAMROOT_LD_PRELOAD_MUSL_X86_64_1") == 0)
			return "";
		else
			return "/usr/lib:/lib";
	}

	return ret;
}

static char *__getld_library_path(const char *ldso, int abi)
{
	char buf[NAME_MAX];
	char *ret;
	int n;

	n = _snprintf(buf, sizeof(buf), "IAMROOT_LD_LIBRARY_PATH_%s_%d", ldso,
		      abi);
	if (n == -1)
		return NULL;
	__sanitize(buf, 1);

	ret = getenv(buf);
	if (!ret)
		ret = getenv("IAMROOT_LD_LIBRARY_PATH");
	if (!ret)
		return "/usr/lib:/lib";

	return ret;
}

__attribute__((visibility("hidden")))
char *__ld_preload(const char *ldso, int abi)
{
	char val[PATH_MAX];
	char buf[NAME_MAX];
	int n, ret;

	n = _snprintf(buf, sizeof(buf), "ld_preload_%s", ldso);
	if (n == -1)
		return NULL;
	__sanitize(buf, 0);

	__strncpy(val, __getld_preload(ldso, abi));

	ret = pathsetenv(getrootdir(), buf, val, 1);
	if (ret)
		return NULL;

	ret = pathprependenv(buf, __getlibiamroot(ldso, abi), 1);
	if (ret)
		return NULL;

	return getenv(buf);
}

__attribute__((visibility("hidden")))
char *__ld_library_path(const char *ldso, int abi)
{
	char val[PATH_MAX];
	char buf[NAME_MAX];
	char *runpath;
	char *rpath;
	int n, ret;

	n = _snprintf(buf, sizeof(buf), "ld_library_path_%s", ldso);
	if (n == -1)
		return NULL;
	__sanitize(buf, 0);

	__strncpy(val, __getld_library_path(ldso, abi));

	ret = pathsetenv(getrootdir(), buf, val, 1);
	if (ret)
		return NULL;

	rpath = getenv("rpath");
	if (rpath) {
		__strncpy(val, rpath);

		ret = pathsetenv(getrootdir(), "iamroot_rpath", rpath, 1);
		if (ret)
			return NULL;

		ret = pathprependenv(buf, getenv("iamroot_rpath"), 1);
		if (ret)
			return NULL;
	}

	runpath = getenv("runpath");
	if (runpath) {
		__strncpy(val, runpath);

		ret = pathsetenv(getrootdir(), "iamroot_runpath", runpath, 1);
		if (ret)
			return NULL;

		ret = pathprependenv(buf, getenv("iamroot_runpath"), 1);
		if (ret)
			return NULL;
	}

	return getenv(buf);
}

__attribute__((visibility("hidden")))
char *__rpath(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = getdynamicentry(path, DT_RPATH, buf, sizeof(buf));
	if (siz == -1)
		return NULL;

	ret = setenv("rpath", buf, 1);
	if (ret)
		return NULL;

	return getenv("rpath");
}

__attribute__((visibility("hidden")))
char *__runpath(const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = getdynamicentry(path, DT_RUNPATH, buf, sizeof(buf));
	if (siz == -1)
		return NULL;

	ret = setenv("runpath", buf, 1);
	if (ret)
		return NULL;

	return getenv("runpath");
}

__attribute__((visibility("hidden")))
char *__inhibit_rpath()
{
	char *val;
	int ret;

	val = getenv("IAMROOT_INHIBIT_RPATH");
	if (!val)
		return NULL;

	ret = pathsetenv(getrootdir(), "inhibit_rpath", val, 1);
	if (ret)
		return NULL;

	return getenv("inhibit_rpath");
}

__attribute__((visibility("hidden")))
char *__getexec()
{
	char *ret;

	ret = getenv("IAMROOT_EXEC");
	if (!ret)
		return "/usr/lib/iamroot/exec.sh";

	return ret;
}

__attribute__((visibility("hidden")))
int __execve(const char *path, char * const argv[], char * const envp[])
{
	const char *root;
	ssize_t len;

	root = getrootdir();
	if (strcmp(root, "/") == 0)
		goto exit;

	len = __strlen(root);
	if (strncmp(path, root, len) == 0)
		path += len;

exit:
	return execve(path, argv, envp);
}

int execve(const char *path, char * const argv[], char * const envp[])
{
	char buf[PATH_MAX], hashbangbuf[PATH_MAX], loaderbuf[PATH_MAX];
	char hashbang[HASHBANG_MAX], loader[HASHBANG_MAX];
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
					   * 16 NULL
					   */
	ssize_t hashbangsiz, siz;
	char *program = NULL;
	int i, j, argc, ret;
	char * const *arg;
	char *xargv1;
	size_t len;

	/* Run exec.sh script */
	if (exec_ignored(path))
		goto exec_sh;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', argv: { '%s', '%s', ... }, envp: %p)\n",
		__func__, path, buf, argv[0], argv[1], envp);
	i = 0;
	interparg[i++] = *argv; /* original argv0 as argv0 */
	program = buf;

	/*
	 * In secure-execution mode, preload pathnames containing slashes are
	 * ignored. Furthermore, shared objects are preloaded only from the
	 * standard search directories and only if they have set-user-ID mode
	 * bit enabled (which is not typical).
	 */
	ret = issuid(buf);
	if (ret == -1)
		return -1;
	else if (ret)
		goto exec_sh;

	/* Do not proceed to any hack if not in chroot */
	if (!inchroot())
		return next_execve(path, argv, envp);

	/* Get the interpeter directive stored after the hashbang */
	hashbangsiz = gethashbang(program, hashbang, sizeof(hashbang));
	if (hashbangsiz == -1) {
		/* Not an hashbang interpreter directive */
		if (errno == ENOEXEC)
			goto loader;

		return -1;
	} else if (hashbangsiz == 0) {
		goto loader;
	}

	/*
	 * Preserve original path in argv0 and set the interpreter and its
	 * optional argument (if any).
	 */
	siz = path_resolution(AT_FDCWD, hashbang, hashbangbuf,
			      sizeof(hashbangbuf), 0);
	if (siz == -1)
		return -1;

	/* Reset argv0 */
	interparg[0] = hashbang; /* hashbang as argv0 */

	/* Add extra argument */
	xargv1 = getenv("IAMROOT_EXEC_HASHBANG_ARGV1");
	if (xargv1)
		interparg[i++] = xargv1; /* extra argument as argv1 */
	/* Add optional argument */
	len = __strnlen(hashbang);
	if (len < (size_t)hashbangsiz && hashbang[len+1])
		interparg[i++] = &hashbang[len+1];
	interparg[i++] = (char *)path; /* original program path as first
					* positional argument */
	interparg[i] = NULL; /* ensure NULL terminated */
	program = hashbangbuf;

loader:
	/*
	 * Run the dynamic linker directly
	 */
	if ((__strncmp(path, "/usr/bin/ld.so") == 0) ||
	    (__strncmp(path, "/lib/ld") == 0) ||
	    (__strncmp(path, "/lib64/ld") == 0)) {
		verbose_exec(buf, argv, envp);
		return next_execve(buf, argv, envp);
	}

	/*
	 * Get the dynamic linker stored in the .interp section of the ELF
	 * linked program.
	 */
	siz = getinterp(program, loader, sizeof(loader));
	if (siz == -1) {
		/* Not an ELF linked program */
		if (errno == ENOEXEC)
			goto exec_sh;

		return -1;
	} else if (siz == 0) {
		goto exec_sh;
	}

	/*
	 * The interpreter has to preload its libiamroot.so library.
	 */
	if ((__strncmp(loader, "/lib/ld") == 0) ||
	    (__strncmp(loader, "/lib64/ld") == 0)) {
		char *argv0, *rpath, *runpath, *ld_preload, *ld_library_path,
		     *inhibit_rpath;
		int has_argv0 = 1, has_preload = 1, has_inhibit_rpath = 0,
		    has_inhibit_cache = 0;
		int shift = 1, abi = 0;
		const char *basename;
		char ldso[NAME_MAX];

		basename = __basename(loader);
		ret = sscanf(basename, "ld-%[^.].so.%d", ldso, &abi);
		if (ret < 2) {
			errno = ENOTSUP;
			return -1;
		}

		/*
		 * the glibc world supports --argv0 since 2.33, --preload since
		 * 2.30, --inhibit-cache since 2.16, and --inhibit-rpath since
		 * 2.0.94
		 */
		if (__strncmp(ldso, "linux") == 0) {
			has_inhibit_rpath = 1;

			has_inhibit_cache =
				   __ld_linux_has_inhibit_cache_option(loader);
			if (has_inhibit_cache == -1)
				return -1;

			has_argv0 = __ld_linux_has_argv0_option(loader);
			if (has_argv0 == -1)
				return -1;

			has_preload = __ld_linux_has_preload_option(loader);
			if (has_preload == -1)
				return -1;
		}

		rpath = __rpath(buf);
		if (rpath)
			__info("%s: RPATH=%s\n", buf, rpath);

		runpath = __runpath(buf);
		if (runpath)
			__info("%s: RUNPATH=%s\n", buf, runpath);

		ld_preload = __ld_preload(ldso, abi);
		if (!ld_preload)
			__warning("%s: is unset!\n", "ld_preload");

		ld_library_path = __ld_library_path(ldso, abi);
		if (!ld_library_path)
			__warning("%s: is unset!\n", "ld_library_path");

		inhibit_rpath = __inhibit_rpath();
		if (!inhibit_rpath)
			__notice("%s: is unset!\n", "inhibit_rpath");

		siz = path_resolution(AT_FDCWD, loader, loaderbuf,
				      sizeof(loaderbuf), 0);
		if (siz == -1)
			return -1;

		/*
		 * Shift enough room in interparg to prepend:
		 *   - the path to the interpreter (i.e. the absolute path in
		 *     host, including the chroot; argv0).
		 *   - the optional extra argument as argv1.
		 *   - the option --ld-preload and its argument (i.e. the path
		 *     in host environment to the interpreter's libiamroot.so,
		 *     and the path in chroot environment to the interpreter's
		 *     libc.so and libdl.so).
		 *   - the option --library-path and its argument (i.e. the
		 *     path in chroot environment to the libraries).
		 *   - the option --inhibit-rpath and its argument (i.e. the
		 *     path in host environment to the libbraries).
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
		for (j = 0; j < i; j++)
			interparg[j+shift] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = loaderbuf;

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
			if (ret) {
				__envperror("LD_PRELOAD", "unsetenv");
				return -1;
			}
		} else {
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
		interparg[i] = program;
		i += j;
		interparg[i] = NULL; /* ensure NULL terminated */

		goto execve;
	} else {
		int shift = 1;

		siz = path_resolution(AT_FDCWD, loader, loaderbuf,
				      sizeof(loaderbuf), 0);
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
		xargv1 = getenv("IAMROOT_EXEC_LD_ARGV1");
		if (xargv1)
			shift++;
		for (j = 0; j < i; j++)
			interparg[j+shift] = interparg[j];

		/* Add path to interpreter (host, argv0) */
		i = 0;
		interparg[i++] = loaderbuf;

		/* Add extra argument as argv1 */
		if (xargv1)
			interparg[i++] = xargv1;

		/* Add path to binary (in chroot, first positional argument) */
		interparg[i] = program;
		i += j;
		interparg[i] = NULL; /* ensure NULL terminated */

		goto execve;
	}

exec_sh:
	__strncpy(buf, __getexec());
	i = 0;
	interparg[i++] = buf;
	interparg[i++] = (char *)path; /* original path as first positional
					* argument
					*/
	interparg[i] = NULL; /* ensure NULL terminated */

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

execve:
	ret = setenv("IAMROOT_VERSION", __xstr(VERSION), 1);
	if (ret)
		return -1;

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

		verbose_exec(*nargv, nargv, __environ);
		return next_execve(*nargv, nargv, __environ);
	}

	errno = EINVAL;
	return -1;
}
