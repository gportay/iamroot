#
# Copyright 2020-2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

VERSION = 6
PREFIX ?= /usr/local
OS ?= $(shell uname -o)

IAMROOT_LIB ?= $(CURDIR)/libiamroot.so
export IAMROOT_LIB

IAMROOT_LIB_LINUX_2 = $(IAMROOT_LIB)
export IAMROOT_LIB_LINUX_2

IAMROOT_LIB_LINUX_X86_64_2 = $(IAMROOT_LIB)
export IAMROOT_LIB_LINUX_X86_64_2

IAMROOT_LIB_LINUX_3 = $(IAMROOT_LIB)
export IAMROOT_LIB_LINUX_3

IAMROOT_LIB_LINUX_ARMHF_3 = $(IAMROOT_LIB)
export IAMROOT_LIB_LINUX_ARMHF_3

IAMROOT_LIB_LINUX_AARCH64_1 = $(IAMROOT_LIB)
export IAMROOT_LIB_LINUX_AARCH64_1

IAMROOT_LIB_MUSL_I386_1 = $(IAMROOT_LIB)
export IAMROOT_LIB_MUSL_I386_1

IAMROOT_LIB_MUSL_X86_64_1 = $(IAMROOT_LIB)
export IAMROOT_LIB_MUSL_X86_64_1

IAMROOT_LIB_MUSL_ARMHF_1 = $(IAMROOT_LIB)
export IAMROOT_LIB_MUSL_ARMHF_1

IAMROOT_LIB_MUSL_AARCH64_1 = $(IAMROOT_LIB)
export IAMROOT_LIB_MUSL_AARCH64_1

IAMROOT_LIB_ELF_1 = $(IAMROOT_LIB)
export IAMROOT_LIB_ELF_1

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

%.o: override CPPFLAGS += -D_GNU_SOURCE -DVERSION=$(VERSION)
%.o: override CFLAGS += -fPIC -Wall -Wextra
%.so: override LDFLAGS += -nodefaultlibs

ifdef COVERAGE
%.o: override CFLAGS += -fprofile-arcs -ftest-coverage
%.so: override LDFLAGS += -fprofile-arcs
%.so: override LDLIBS += -lgcov
endif

.PHONY: all
all: libiamroot.so

libiamroot.so: __abort.o
libiamroot.so: __envperror.o
libiamroot.so: __fpathperror.o
libiamroot.so: __fxstat.o
libiamroot.so: __fxstat64.o
libiamroot.so: __fxstatat.o
libiamroot.so: __fxstatat64.o
libiamroot.so: __libc_start_main.o
libiamroot.so: __lxstat.o
libiamroot.so: __lxstat64.o
libiamroot.so: __open_2.o
libiamroot.so: __openat_2.o
libiamroot.so: __opendir2.o
libiamroot.so: __opendirat.o
libiamroot.so: __pathdlperror.o
libiamroot.so: __pathperror.o
libiamroot.so: __pathperror2.o
libiamroot.so: __verbosef.o
libiamroot.so: __xmknod.o
libiamroot.so: __xmknodat.o
libiamroot.so: __xstat.o
libiamroot.so: __xstat64.o
libiamroot.so: access.o
libiamroot.so: acct.o
libiamroot.so: canonicalize_file_name.o
libiamroot.so: catopen.o
libiamroot.so: chdir.o
libiamroot.so: chflags.o
libiamroot.so: chflagsat.o
libiamroot.so: chmod.o
libiamroot.so: chown.o
libiamroot.so: chroot.o
libiamroot.so: creat.o
libiamroot.so: dl_iterate_phdr.o
libiamroot.so: dlmopen.o
libiamroot.so: dlopen.o
libiamroot.so: euidaccess.o
libiamroot.so: execl.o
libiamroot.so: execle.o
libiamroot.so: execlp.o
libiamroot.so: exect.o
libiamroot.so: execv.o
libiamroot.so: execvP.o
libiamroot.so: execve.o
libiamroot.so: execveat.o
libiamroot.so: execvp.o
libiamroot.so: execvpe.o
libiamroot.so: extattr_delete_fd.o
libiamroot.so: extattr_delete_file.o
libiamroot.so: extattr_delete_link.o
libiamroot.so: extattr_get_fd.o
libiamroot.so: extattr_get_file.o
libiamroot.so: extattr_get_link.o
libiamroot.so: extattr_list_fd.o
libiamroot.so: extattr_list_file.o
libiamroot.so: extattr_list_link.o
libiamroot.so: extattr_set_fd.o
libiamroot.so: extattr_set_file.o
libiamroot.so: extattr_set_link.o
libiamroot.so: faccessat.o
libiamroot.so: fanotify_mark.o
libiamroot.so: fchdir.o
libiamroot.so: fchmod.o
libiamroot.so: fchmodat.o
libiamroot.so: fchown.o
libiamroot.so: fchownat.o
libiamroot.so: fexecve.o
libiamroot.so: fgetxattr.o
libiamroot.so: flistxattr.o
libiamroot.so: fopen.o
libiamroot.so: fremovexattr.o
libiamroot.so: freopen.o
libiamroot.so: fsetxattr.o
libiamroot.so: fstat.o
libiamroot.so: fstat64.o
libiamroot.so: fstatat.o
libiamroot.so: fstatat64.o
libiamroot.so: ftok.o
libiamroot.so: ftw.o
libiamroot.so: futimesat.o
libiamroot.so: get_current_dir_name.o
libiamroot.so: getcwd.o
libiamroot.so: geteuid.o
libiamroot.so: getfh.o
libiamroot.so: getfhat.o
libiamroot.so: getuid.o
libiamroot.so: getwd.o
libiamroot.so: getxattr.o
libiamroot.so: group.o
libiamroot.so: inotify_add_watch.o
libiamroot.so: lchflags.o
libiamroot.so: lchmod.o
libiamroot.so: lchown.o
libiamroot.so: lgetfh.o
libiamroot.so: lgetxattr.o
libiamroot.so: link.o
libiamroot.so: linkat.o
libiamroot.so: listxattr.o
libiamroot.so: llistxattr.o
libiamroot.so: lremovexattr.o
libiamroot.so: lsetxattr.o
libiamroot.so: lstat.o
libiamroot.so: lstat64.o
libiamroot.so: lutimes.o
libiamroot.so: mkdir.o
libiamroot.so: mkdirat.o
libiamroot.so: mkdtemp.o
libiamroot.so: mkfifo.o
libiamroot.so: mkfifoat.o
libiamroot.so: mknod.o
libiamroot.so: mknodat.o
libiamroot.so: mkostemp.o
libiamroot.so: mkostemps.o
libiamroot.so: mkstemp.o
libiamroot.so: mkstemps.o
libiamroot.so: mktemp.o
libiamroot.so: mount.o
libiamroot.so: mq_open.o
libiamroot.so: name_to_handle_at.o
libiamroot.so: nftw.o
libiamroot.so: nftw64.o
libiamroot.so: nmount.o
libiamroot.so: open.o
libiamroot.so: openat.o
libiamroot.so: opendir.o
libiamroot.so: passwd.o
libiamroot.so: path_resolution.o
libiamroot.so: posix_spawn.o
libiamroot.so: posix_spawnp.o
libiamroot.so: readlink.o
libiamroot.so: readlinkat.o
libiamroot.so: realpath.o
libiamroot.so: remove.o
libiamroot.so: removexattr.o
libiamroot.so: rename.o
libiamroot.so: renameat.o
libiamroot.so: renameat2.o
libiamroot.so: rmdir.o
libiamroot.so: running_in_chroot.o
libiamroot.so: scandir.o
libiamroot.so: scandir64.o
libiamroot.so: scandir_b.o
libiamroot.so: scandirat.o
libiamroot.so: setegid.o
libiamroot.so: seteuid.o
libiamroot.so: setgid.o
libiamroot.so: setuid.o
libiamroot.so: setxattr.o
libiamroot.so: shadow.o
libiamroot.so: stat.o
libiamroot.so: stat64.o
libiamroot.so: statfs.o
libiamroot.so: statfs64.o
libiamroot.so: statvfs.o
libiamroot.so: statvfs64.o
libiamroot.so: statx.o
libiamroot.so: swapoff.o
libiamroot.so: swapon.o
libiamroot.so: symlink.o
libiamroot.so: symlinkat.o
libiamroot.so: tempnam.o
libiamroot.so: tmpnam.o
libiamroot.so: tmpnam_r.o
libiamroot.so: truncate.o
libiamroot.so: truncate64.o
libiamroot.so: umask.o
libiamroot.so: umount.o
libiamroot.so: umount2.o
libiamroot.so: unlink.o
libiamroot.so: unlinkat.o
libiamroot.so: unmount.o
libiamroot.so: unshare.o
libiamroot.so: utime.o
libiamroot.so: utimensat.o
libiamroot.so: utimes.o

.PHONY: doc
doc: iamroot-shell.1.gz iamroot.7.gz

.PHONY: install
install: install-exec install-doc install-bash-completion

.PHONY: install-exec
install-exec:
	install -d -m755 $(DESTDIR)$(PREFIX)/bin/ $(DESTDIR)$(PREFIX)/lib/iamroot/
	install -m755 libiamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	sed -e "s,\$$PWD,$(PREFIX)/lib/iamroot," iamroot-shell >$(DESTDIR)$(PREFIX)/bin/iamroot-shell
	chmod a+x $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	install -m755 libiamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	install -m755 exec.sh $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh

.PHONY: install-doc
install-doc:
	install -d -m755 $(DESTDIR)$(PREFIX)/share/man/man1/ $(DESTDIR)$(PREFIX)/share/man/man7/
	install -m644 iamroot-shell.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/iamroot-shell.1.gz
	install -m644 iamroot.7.gz $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz

.PHONY: install-bash-completion
install-bash-completion:
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion 2>/dev/null)}; \
	if [ -n "$$completionsdir" ]; then \
		install -m644 $(DESTDIR)$$completionsdir/; \
		install -m644 bash-completion $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/iamroot-shell.1.gz
	rm -f $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		rm -f $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

.PHONY: user-install
user-install: user-install-exec user-install-doc user-install-bash-completion

user-install-exec user-install-doc user-install-bash-completion user-uninstall:
user-%:
	$(MAKE) $* PREFIX=$$HOME/.local BASHCOMPLETIONSDIR=$$HOME/.local/share/bash-completion/completions

.PHONY: ci
ci: check test

.PHONY: check
check:
	shellcheck -e SC1090 -e SC3037 iamroot-shell exec.sh

.PHONY: test
test:
	$(MAKE) -C tests
	$(MAKE) -C tests $@

.PHONY: shell
shell: libiamroot.so
	bash iamroot-shell

.PHONY: clean
clean:
	rm -Rf libiamroot.so *.o *.i
	$(MAKE) -C tests $@

.PHONY: mrproper
mrproper: clean
	rm -f busybox-static
	rm -Rf busybox/

%-debug %-debug1:
	$(MAKE) $* IAMROOT_DEBUG=1

%-debug2:
	$(MAKE) $* IAMROOT_DEBUG=2

%-debug3:
	$(MAKE) $* IAMROOT_DEBUG=3

%-debug4:
	$(MAKE) $* IAMROOT_DEBUG=4

%-debug5:
	$(MAKE) $* IAMROOT_DEBUG=5

%-debug6:
	$(MAKE) $* IAMROOT_DEBUG=6

PREPROCESS.c = $(PREPROCESS.S)
%.i: %.c
	$(PREPROCESS.c) $(OUTPUT_OPTION) $<

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.1: %.1.adoc
	asciidoctor -b manpage -o $@ $<

%.7: %.7.adoc
	asciidoctor -b manpage -o $@ $<

%.gz: %
	gzip -c $< >$@
