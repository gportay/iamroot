#
# Copyright 2020-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

VERSION = 20
PREFIX ?= /usr/local
COVERAGE ?= 0
OS ?= $(shell uname -o 2>/dev/null || uname -s 2>/dev/null)

IAMROOT_LIB ?= $(CURDIR)/libiamroot.so
export IAMROOT_LIB

IAMROOT_LIB_I686_LINUX_2 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_I686_LINUX_2

IAMROOT_LIB_X86_64_LINUX_X86_64_2 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_X86_64_LINUX_X86_64_2

IAMROOT_LIB_ARM_LINUX_3 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_ARM_LINUX_3

IAMROOT_LIB_ARMHF_LINUX_ARMHF_3 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_ARMHF_LINUX_ARMHF_3

IAMROOT_LIB_AARCH64_LINUX_AARCH64_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_AARCH64_LINUX_AARCH64_1

IAMROOT_LIB_RISCV64_LINUX_RISCV64_LP64D_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_RISCV64_LINUX_RISCV64_LP64D_1

IAMROOT_LIB_I686_MUSL_I386_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_I686_MUSL_I386_1

IAMROOT_LIB_X86_64_MUSL_X86_64_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_X86_64_MUSL_X86_64_1

IAMROOT_LIB_ARM_MUSL_ARM_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_ARM_MUSL_ARM_1

IAMROOT_LIB_ARMHF_MUSL_ARMHF_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_ARMHF_MUSL_ARMHF_1

IAMROOT_LIB_AARCH64_MUSL_AARCH64_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_AARCH64_MUSL_AARCH64_1

IAMROOT_LIB_RISCV64_MUSL_RISCV64_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_RISCV64_MUSL_RISCV64_1

IAMROOT_LIB_X86_64_ELF_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_X86_64_ELF_1

IAMROOT_LIB_AARCH64_ELF_1 ?= $(IAMROOT_LIB)
export IAMROOT_LIB_AARCH64_ELF_1

IAMROOT_EXEC ?= $(CURDIR)/exec.sh
export IAMROOT_EXEC

NVERBOSE ?= 1

ifneq ($(NVERBOSE),0)
CFLAGS += -DNVERBOSE
endif

%.o: override CPPFLAGS += -D_GNU_SOURCE -DVERSION=$(VERSION) -DPREFIX=$(PREFIX)
%.o: override CFLAGS += -fPIC -Wall -Wextra
ifeq ($(OS),GNU/Linux)
lib%.so: override LDFLAGS += -nodefaultlibs
endif
ifeq ($(OS),FreeBSD)
lib%.so: override LDFLAGS += -ldl
endif

ifneq ($(COVERAGE),0)
%.o: override CFLAGS += -fprofile-arcs -ftest-coverage
%.so: override LDFLAGS += -fprofile-arcs
%.so: override LDLIBS += -lgcov
endif

.PHONY: all
all: ld-iamroot.so
all: libiamroot.so

ld-iamroot.so: __fxstat.o
ld-iamroot.so: __fxstatat.o
ld-iamroot.so: dlopen.o
ld-iamroot.so: dso.o
ld-iamroot.so: env.o
ld-iamroot.so: execve.o
ld-iamroot.so: faccessat.o
ld-iamroot.so: fgetxattr.o
ld-iamroot.so: fstat.o
ld-iamroot.so: fstatat.o
ld-iamroot.so: getcwd.o
ld-iamroot.so: iamroot.o
ld-iamroot.so: interpreter-script.o
ld-iamroot.so: ld.o
ld-iamroot.so: ldso-cache.o
ld-iamroot.so: lgetxattr.o
ld-iamroot.so: lremovexattr.o
ld-iamroot.so: lsetxattr.o
ld-iamroot.so: open.o
ld-iamroot.so: path_resolution.o
ld-iamroot.so: readlinkat.o
ld-iamroot.so: realpath.o
ld-iamroot.so: scandir.o

libiamroot.so: __fstat64_time64.o
libiamroot.so: __fstatat64_time64.o
libiamroot.so: __fxstat.o
libiamroot.so: __fxstat64.o
libiamroot.so: __fxstatat.o
libiamroot.so: __fxstatat64.o
libiamroot.so: __libc_start_main.o
libiamroot.so: __lstat64_time64.o
libiamroot.so: __lxstat.o
libiamroot.so: __lxstat64.o
libiamroot.so: __open_2.o
libiamroot.so: __openat_2.o
libiamroot.so: __opendir2.o
libiamroot.so: __stat64_time64.o
libiamroot.so: __xmknod.o
libiamroot.so: __xmknodat.o
libiamroot.so: __xstat.o
libiamroot.so: __xstat64.o
libiamroot.so: accept.o
libiamroot.so: accept4.o
libiamroot.so: access.o
libiamroot.so: acct.o
libiamroot.so: bind.o
libiamroot.so: canonicalize_file_name.o
libiamroot.so: catopen.o
libiamroot.so: chdir.o
libiamroot.so: chflags.o
libiamroot.so: chflagsat.o
libiamroot.so: chmod.o
libiamroot.so: chown.o
libiamroot.so: chroot.o
libiamroot.so: close.o
libiamroot.so: close_range.o
libiamroot.so: closefrom.o
libiamroot.so: compat.o
libiamroot.so: connect.o
libiamroot.so: creat.o
libiamroot.so: ctermid.o
libiamroot.so: dl_iterate_phdr.o
libiamroot.so: dlmopen.o
libiamroot.so: dlopen.o
libiamroot.so: dso.o
libiamroot.so: dup.o
libiamroot.so: dup2.o
libiamroot.so: dup3.o
libiamroot.so: env.o
libiamroot.so: euidaccess.o
libiamroot.so: execl.o
libiamroot.so: execle.o
libiamroot.so: execlp.o
libiamroot.so: exect.o
libiamroot.so: execv.o
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
libiamroot.so: fclose.o
libiamroot.so: fdopen.o
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
libiamroot.so: fts.o
libiamroot.so: fts64.o
libiamroot.so: ftw.o
libiamroot.so: futimesat.o
libiamroot.so: get_current_dir_name.o
libiamroot.so: getcwd.o
libiamroot.so: getegid.o
libiamroot.so: geteuid.o
libiamroot.so: getfh.o
libiamroot.so: getfhat.o
libiamroot.so: getgid.o
libiamroot.so: getgroups.o
libiamroot.so: getpeername.o
libiamroot.so: getresgid.o
libiamroot.so: getresuid.o
libiamroot.so: getsockname.o
libiamroot.so: getuid.o
libiamroot.so: getwd.o
libiamroot.so: getxattr.o
libiamroot.so: group.o
libiamroot.so: iamroot.o
libiamroot.so: initgroups.o
libiamroot.so: inotify_add_watch.o
libiamroot.so: interpreter-script.o
libiamroot.so: kill.o
libiamroot.so: lchflags.o
libiamroot.so: lchmod.o
libiamroot.so: lchown.o
libiamroot.so: ldso-cache.o
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
libiamroot.so: mkostempsat.o
libiamroot.so: mkstemp.o
libiamroot.so: mkstemps.o
libiamroot.so: mktemp.o
libiamroot.so: mount.o
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
libiamroot.so: ptsname.o
libiamroot.so: ptsname_r.o
libiamroot.so: readlink.o
libiamroot.so: readlinkat.o
libiamroot.so: realpath.o
libiamroot.so: remove.o
libiamroot.so: removexattr.o
libiamroot.so: rename.o
libiamroot.so: renameat.o
libiamroot.so: renameat2.o
libiamroot.so: rmdir.o
libiamroot.so: scandir.o
libiamroot.so: scandir64.o
libiamroot.so: scandir_b.o
libiamroot.so: scandirat.o
libiamroot.so: setegid.o
libiamroot.so: seteuid.o
libiamroot.so: setfsgid.o
libiamroot.so: setfsuid.o
libiamroot.so: setgid.o
libiamroot.so: setgroups.o
libiamroot.so: setregid.o
libiamroot.so: setresgid.o
libiamroot.so: setresuid.o
libiamroot.so: setreuid.o
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
libiamroot.so: truncate.o
libiamroot.so: truncate64.o
libiamroot.so: ttyname.o
libiamroot.so: ttyname_r.o
libiamroot.so: umask.o
libiamroot.so: umount.o
libiamroot.so: umount2.o
libiamroot.so: unlink.o
libiamroot.so: unlinkat.o
libiamroot.so: unmount.o
libiamroot.so: unshare.o
libiamroot.so: updwtmp.o
libiamroot.so: updwtmpx.o
libiamroot.so: utime.o
libiamroot.so: utimensat.o
libiamroot.so: utimes.o
libiamroot.so: utmpname.o
libiamroot.so: utmpxname.o

.PHONY: doc
doc: ido.1.gz ish.1.gz iamroot.7.gz ld-iamroot.so.8.gz

.PHONY: install
install: install-exec install-doc install-bash-completion

.PHONY: install-exec
install-exec: install-bin install-lib

.PHONY: install-bin
install-bin:
	install -d -m755 $(DESTDIR)$(PREFIX)/bin/
	sed -e "s,/usr/local,$(PREFIX)," ido >$(DESTDIR)$(PREFIX)/bin/ido
	chmod a+x $(DESTDIR)$(PREFIX)/bin/ido
	sed -e "s,/usr/local,$(PREFIX)," ish >$(DESTDIR)$(PREFIX)/bin/ish
	chmod a+x $(DESTDIR)$(PREFIX)/bin/ish
	install -d -m755 $(DESTDIR)$(PREFIX)/lib/iamroot/
	install -m755 exec.sh $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh
	install -m755 ld-iamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/ld-iamroot.so
	ln -sf $(PREFIX)/lib/iamroot/ld-iamroot.so $(DESTDIR)$(PREFIX)/bin/ld-iamroot.so

.PHONY: install-lib
install-lib:
	install -d -m755 $(DESTDIR)$(PREFIX)/lib/iamroot/
	install -m755 libiamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so

.PHONY: install-doc
install-doc:
	install -d -m755 $(DESTDIR)$(PREFIX)/share/man/man1/ $(DESTDIR)$(PREFIX)/share/man/man7/
	install -m644 ido.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/ido.1.gz
	install -m644 ish.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/ish.1.gz
	install -m644 iamroot.7.gz $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz

.PHONY: install-bash-completion
install-bash-completion:
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion 2>/dev/null)}; \
	if [ -n "$$completionsdir" ]; then \
		install -d -m755 $(DESTDIR)$$completionsdir/; \
		install -m644 completions/ido $(DESTDIR)$$completionsdir/ido; \
		install -m644 completions/ish $(DESTDIR)$$completionsdir/ish; \
	fi

.PHONY: install-support
install-support:
	install -D -m644 support/aarch64-archlinuxarm-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinuxarm/pacman.conf-aarch64
	install -D -m644 support/armv7h-archlinuxarm-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinuxarm/pacman.conf-armv7h
	install -D -m644 support/ceres $(DESTDIR)$(PREFIX)/share/iamroot/debootstrap/ceres
	install -D -m644 support/fedora-archive.repo $(DESTDIR)$(PREFIX)/share/iamroot/fedora/fedora-archive.repo
	install -D -m644 support/fedora.repo $(DESTDIR)$(PREFIX)/share/iamroot/fedora/fedora.repo
	install -D -m644 support/i686-archlinux32-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinux32/pacman.conf-i686
	install -D -m644 support/opensuse-leap-repo-oss.repo $(DESTDIR)$(PREFIX)/share/iamroot/opensuse/leap-repo-oss.repo
	install -D -m644 support/opensuse-tumbleweed-repo-oss.repo $(DESTDIR)$(PREFIX)/share/iamroot/opensuse/tumbleweed-repo-oss.repo
	install -D -m644 support/powerpc64-archlinuxpower-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archpower/pacman.conf-powerpc64
	install -D -m644 support/powerpc64le-archlinuxpower-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archpower/pacman.conf-powerpc64le
	install -D -m644 support/riscv64-archlinuxriscv-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinuxrisc-v/pacman.conf-riscv64
	install -D -m644 support/toolchain_archlinuxpower_powerpc64_defconfig $(DESTDIR)$(PREFIX)/share/iamroot/buildroot/toolchain_archlinuxpower_powerpc64_defconfig
	install -D -m644 support/toolchain_debian_mips64el_defconfig $(DESTDIR)$(PREFIX)/share/iamroot/buildroot/toolchain_debian_mips64el_defconfig
	install -D -m644 support/x86_64-archlinux-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinux/pacman.conf
	install -D -m644 support/x86_64-manjaro-stable-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/manjaro/pacman.conf-stable
	install -D -m644 support/x86_64-manjaro-unstable-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/manjaro/pacman.conf-unstable
	install -D -m644 support/x86_64-xbps.conf $(DESTDIR)$(PREFIX)/share/iamroot/voidlinux/xbps.conf

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/ish
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/exec.sh
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/ido.1.gz
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/ish.1.gz
	rm -f $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		rm -f $(DESTDIR)$$completionsdir/ido; \
		rm -f $(DESTDIR)$$completionsdir/ish; \
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
	shellcheck -e SC1090 -e SC3037 ido ish exec.sh

.PHONY: test
test: libiamroot.so
	$(MAKE) -C tests
	$(MAKE) -C tests $@ PATH=$(CURDIR):$$PATH

.PHONY: multiarch-ido
multiarch-ido: IDOFLAGS += --multiarch
multiarch-ido: ido

.PHONY: multiarch-ish
multiarch-ish: ISHFLAGS += --multiarch
multiarch-ish: ish

.PHONY: ido
ido: libiamroot.so
	bash ido $(IDOFLAGS) -i

.PHONY: ish
ish: libiamroot.so
	bash ish $(ISHFLAGS)

.PHONY: sh bash zsh
sh bash zsh csh ksh: PATH := $(CURDIR):$(PATH)
sh bash zsh csh ksh:
	$@

.PHONY: shell
shell: PATH := $(CURDIR):$(PATH)
shell:
	$(SHELL)

.PHONY: cleanall
cleanall: clean

.PHONY: clean
clean:
	rm -f ld-iamroot.so libiamroot.so* fuzzer *.o *.i *.1.gz *.7.gz *.8.gz
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

.PHONY: define-list
define-list:
	$(CPP) -dM - </dev/null

PREPROCESS.c = $(PREPROCESS.S)
%.i: %.c
	$(PREPROCESS.c) $(OUTPUT_OPTION) $<

lib%.so: override LDFLAGS += -shared
lib%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@.tmp
ifeq ($(OS),GNU/Linux)
	patchelf --add-needed libc.so.6 --add-needed libdl.so.2 --add-needed libpthread.so.0 $@.tmp
endif
	mv $@.tmp $@

ld%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: fuzzing
fuzzing: PATH := $(CURDIR):$(PATH)
fuzzing: fuzzer | corpus
	fuzzer $(FUZZERFLAGS) -only_ascii=1 -max_len=4096 corpus

corpus:
	mkdir $@

fuzzer: CC = clang -g -fsanitize=address,undefined,fuzzer -fno-omit-frame-pointer
fuzzer: CXX = $(CC)
fuzzer: __fxstat.o
fuzzer: __fxstatat.o
fuzzer: env.o
fuzzer: fgetxattr.o
fuzzer: fstat.o
fuzzer: fstatat.o
fuzzer: getcwd.o
fuzzer: iamroot.o
fuzzer: lgetxattr.o
fuzzer: path_resolution.o
fuzzer: readlinkat.o
fuzzer: realpath.o
fuzzer: scandir.o
fuzzer: fuzzer.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: compile_commands.json
compile_commands.json:
	@( echo '['; \
	$(foreach c,$(wildcard *.c),\
		echo '  {'; \
		echo '    "directory": "$(CURDIR)",'; \
		echo '    "command": "$(COMPILE.c) $(CFLAGS) $(CPPFAGLS) $(TARGET_ARCH) -o $(c:.c=.o) $(c)",'; \
		echo '    "file": "$(c)",'; \
		echo '    "output": "$(c:.c=.o)"'; \
		echo '  },'; \
	) \
	echo '  {'; \
	echo '    "directory": "$(CURDIR)",'; \
	echo '    "command": "true",'; \
	echo '    "file": "iamroot.h",'; \
	echo '    "output": "libiamroot.so"'; \
	echo '  }'; \
	echo ']' ) >$@

%.1: %.1.adoc
	asciidoctor -b manpage -o $@ $<

%.7: %.7.adoc
	asciidoctor -b manpage -o $@ $<

%.8: %.8.adoc
	asciidoctor -b manpage -o $@ $<

%.gz: %
	gzip -c $< >$@
