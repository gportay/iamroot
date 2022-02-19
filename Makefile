#
# Copyright 2020-2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

VERSION = 3
PREFIX ?= /usr/local

%.o: override CFLAGS += -fPIC -Wall -Wextra -Werror
%.o: override CFLAGS += -D_GNU_SOURCE -DVERSION=$(VERSION)
%.so: override LDFLAGS += -nolibc

.PHONY: all
all: x86_64/libiamroot-linux-x86-64.so.2

x86_64/libiamroot-linux-x86-64.so.2: libiamroot.so
	install -D -m755 $< $@

libiamroot.so: __envperror.o
libiamroot.so: __fpathperror.o
libiamroot.so: __fstat.o
libiamroot.so: __fstat64.o
libiamroot.so: __fxstat.o
libiamroot.so: __fxstat64.o
libiamroot.so: __fxstatat.o
libiamroot.so: __fxstatat64.o
libiamroot.so: __libc_start_main.o
libiamroot.so: __lxstat.o
libiamroot.so: __lxstat64.o
libiamroot.so: __nss_files_fopen.o
libiamroot.so: __open.o
libiamroot.so: __open64.o
libiamroot.so: __open64_2.o
libiamroot.so: __open_2.o
libiamroot.so: __openat64_2.o
libiamroot.so: __openat_2.o
libiamroot.so: __opendir.o
libiamroot.so: __opendirat.o
libiamroot.so: __pathdlperror.o
libiamroot.so: __pathperror.o
libiamroot.so: __pathperror2.o
libiamroot.so: __statfs.o
libiamroot.so: __statfs64.o
libiamroot.so: __verbosef.o
libiamroot.so: __xmknod.o
libiamroot.so: __xmknodat.o
libiamroot.so: __xstat.o
libiamroot.so: __xstat64.o
libiamroot.so: access.o
libiamroot.so: canonicalize_file_name.o
libiamroot.so: chdir.o
libiamroot.so: chmod.o
libiamroot.so: chown.o
libiamroot.so: chroot.o
libiamroot.so: creat.o
libiamroot.so: creat64.o
libiamroot.so: dlmopen.o
libiamroot.so: dlopen.o
libiamroot.so: eaccess.o
libiamroot.so: euidaccess.o
libiamroot.so: execl.o
libiamroot.so: execle.o
libiamroot.so: execlp.o
libiamroot.so: execv.o
libiamroot.so: execve.o
libiamroot.so: execveat.o
libiamroot.so: execvp.o
libiamroot.so: execvpe.o
libiamroot.so: faccessat.o
libiamroot.so: fchdir.o
libiamroot.so: fchmod.o
libiamroot.so: fchmodat.o
libiamroot.so: fchown.o
libiamroot.so: fchownat.o
libiamroot.so: fexecve.o
libiamroot.so: fgetxattr.o
libiamroot.so: fopen.o
libiamroot.so: fopen64.o
libiamroot.so: freopen.o
libiamroot.so: freopen64.o
libiamroot.so: fsetxattr.o
libiamroot.so: fstat.o
libiamroot.so: fstat64.o
libiamroot.so: fstatat.o
libiamroot.so: fstatat64.o
libiamroot.so: futimesat.o
libiamroot.so: get_current_dir_name.o
libiamroot.so: getcwd.o
libiamroot.so: geteuid.o
libiamroot.so: getuid.o
libiamroot.so: getwd.o
libiamroot.so: getxattr.o
libiamroot.so: group.o
libiamroot.so: lchmod.o
libiamroot.so: lchown.o
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
libiamroot.so: mkostemp64.o
libiamroot.so: mkostemps.o
libiamroot.so: mkostemps64.o
libiamroot.so: mkstemp.o
libiamroot.so: mkstemp64.o
libiamroot.so: mkstemps.o
libiamroot.so: mkstemps64.o
libiamroot.so: mktemp.o
libiamroot.so: mount.o
libiamroot.so: name_to_handle_at.o
libiamroot.so: open.o
libiamroot.so: open64.o
libiamroot.so: openat.o
libiamroot.so: openat64.o
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
libiamroot.so: symlink.o
libiamroot.so: symlinkat.o
libiamroot.so: tempnam.o
libiamroot.so: tmpnam.o
libiamroot.so: tmpnam_r.o
libiamroot.so: truncate.o
libiamroot.so: umask.o
libiamroot.so: umount.o
libiamroot.so: umount2.o
libiamroot.so: unlink.o
libiamroot.so: unlinkat.o
libiamroot.so: unshare.o
libiamroot.so: utime.o
libiamroot.so: utimensat.o
libiamroot.so: utimes.o
libiamroot.so: whereami.o
libiamroot.so: whoami.o

.PHONY: doc
doc: iamroot-shell.1.gz iamroot.7.gz

.PHONY: install
install: install-exec install-doc install-bash-completion

.PHONY: install-exec
install-exec:
	install -D -m 755 iamroot-shell $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	sed -e "s,\$$PWD,$(PREFIX)/lib/iamroot," -i $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	install -D -m 755 x86_64/libiamroot-linux-x86-64.so.2 $(DESTDIR)$(PREFIX)/lib/iamroot/x86_64/libiamroot-linux-x86-64.so.2
	ln -sf x86_64/libiamroot-linux-x86-64.so.2 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	install -D -m 755 exec.sh $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh

.PHONY: install-doc
install-doc:
	install -D -m 644 iamroot-shell.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/iamroot-shell.1.gz
	install -D -m 644 iamroot.7.gz $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz

.PHONY: install-bash-completion
install-bash-completion:
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		install -D -m 644 bash-completion $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/x86_64/libiamroot-linux-x86-64.so.2
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
	shellcheck -e SC3037 iamroot-shell exec.sh

.PHONY: test
test: | x86_64/libiamroot-linux-x86-64.so.2
	$(MAKE) -C tests
	$(MAKE) -C tests $@ LD_PRELOAD=$(CURDIR)/x86_64/libiamroot-linux-x86-64.so.2:libdl.so.2 IAMROOT_LIB=$(CURDIR)/x86_64/libiamroot-linux-x86-64.so.2:libdl.so.2

.PHONY: shell
shell: x86_64/libiamroot-linux-x86-64.so.2
	bash iamroot-shell

.PHONY: clean
clean:
	rm -Rf x86_64/ libiamroot.so *.o *.i
	$(MAKE) -C tests $@

.PHONY: mrproper
mrproper: clean
	rm -f busybox-static
	rm -Rf busybox/

PREPROCESS.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -E
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
