#
# Copyright 2020-2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

VERSION = 1
PREFIX ?= /usr/local

override CFLAGS += -fPIC -Wall -Wextra -Werror
override CFLAGS += -D_GNU_SOURCE
override CFLAGS += -DARG_MAX=$(shell getconf ARG_MAX)

QEMU ?= qemu-system-x86_64
QEMU += -enable-kvm -m 4G -machine q35 -smp 4 -cpu host
ifndef NO_SPICE
QEMU += -vga virtio -display egl-headless,gl=on
QEMU += -spice port=5924,disable-ticketing -device virtio-serial-pci -device virtserialport,chardev=spicechannel0,name=com.redhat.spice.0 -chardev spicevmc,id=spicechannel0,name=vdagent
endif
QEMU += -serial mon:stdio

.PHONY: all
all: libiamroot.so

libiamroot.so: __fstat.o
libiamroot.so: __fstat64.o
libiamroot.so: __fxstat.o
libiamroot.so: __fxstat64.o
libiamroot.so: __fxstatat.o
libiamroot.so: __fxstatat64.o
libiamroot.so: __lxstat.o
libiamroot.so: __lxstat64.o
libiamroot.so: __open.o
libiamroot.so: __open64.o
libiamroot.so: __open64_2.o
libiamroot.so: __open_2.o
libiamroot.so: __openat64_2.o
libiamroot.so: __openat_2.o
libiamroot.so: __opendir.o
libiamroot.so: __opendirat.o
libiamroot.so: __statfs.o
libiamroot.so: __statfs64.o
libiamroot.so: __xmknod.o
libiamroot.so: __xmknodat.o
libiamroot.so: __xstat.o
libiamroot.so: __xstat64.o
libiamroot.so: __fprintf.o
libiamroot.so: __printf.o
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
libiamroot.so: execvp.o
libiamroot.so: execvpe.o
libiamroot.so: faccessat.o
libiamroot.so: fchdir.o
libiamroot.so: fchmodat.o
libiamroot.so: fchown.o
libiamroot.so: fchownat.o
libiamroot.so: fexecve.o
libiamroot.so: fopen.o
libiamroot.so: fopen64.o
libiamroot.so: fpath_resolutionat.o
libiamroot.so: freopen.o
libiamroot.so: freopen64.o
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
libiamroot.so: opendir64.o
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
libiamroot.so: setxattr.o
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
libiamroot.so: override LDLIBS += -ldl

.PHONY: doc
doc: iamroot-shell.1.gz iamroot.7.gz

.PHONY: install
install:
	install -D -m 755 iamroot-shell $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	sed -e "s,\$$PWD,$(PREFIX)/lib/iamroot," -i $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	install -D -m 755 libiamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	install -D -m 644 iamroot-shell.1.gz $(DESTDIR)$(PREFIX)/share/man/man1/iamroot-shell.1.gz
	install -D -m 644 iamroot.7.gz $(DESTDIR)$(PREFIX)/share/man/man7/iamroot.7.gz
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		install -D -m 644 bash-completion $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
	completionsdir=$${BASHCOMPLETIONSDIR:-$$(pkg-config --define-variable=prefix=$(PREFIX) \
	                             --variable=completionsdir \
	                             bash-completion)}; \
	if [ -n "$$completionsdir" ]; then \
		rm -f $(DESTDIR)$$completionsdir/iamroot-shell; \
	fi

user-install user-uninstall:
user-%:
	$(MAKE) $* PREFIX=$$HOME/.local BASHCOMPLETIONSDIR=$$HOME/.local/share/bash-completion/completions

.PHONY: ci
ci: check
	$(MAKE) clean tests
	$(SHELL) make-musl-gcc.sh clean all

.PHONY: run-qemu
run-qemu: override QEMUFLAGS += -append "console=ttyS0 root=host0 rootfstype=9p rootflags=trans=virtio debug"
run-qemu: override QEMUFLAGS += -kernel arch-rootfs/boot/vmlinuz-linux
run-qemu: override QEMUFLAGS += -initrd arch-rootfs/boot/initramfs-linux-fallback.img
run-qemu: override QEMUFLAGS += -virtfs local,path=$(CURDIR)/arch-rootfs,mount_tag=host0,security_model=passthrough,id=host0
run-qemu: | arch-rootfs/boot/vmlinuz-linux
	$(QEMU) $(QEMUFLAGS)

.PHONY: check
check:
	shellcheck iamroot-shell

.PHONY: tests
tests: alpine-tests
tests: shell-tests
tests: static-tests
tests: | libiamroot.so alpine-minirootfs
	$(MAKE) -C tests
	$(MAKE) -C tests $@ LD_PRELOAD=$(CURDIR)/libiamroot.so IAMROOT_LIB=$(CURDIR)/libiamroot.so ALPINE_MINIROOTFS=$(CURDIR)/alpine-minirootfs

.PHONY: static-tests
static-tests: SHELL = /bin/bash
static-tests: libiamroot.so | static-rootfs
	bash iamroot-shell -c "whoami | tee /dev/stderr | grep -q \"^root\$$\""
	bash iamroot-shell -c "IAMROOT_GETEUID=$$EUID whoami | tee /dev/stderr | grep -q \"^$$USER\$$\""

.PHONY: shell-tests
shell-tests: libiamroot.so | static-rootfs
	bash iamroot-shell -c "whoami" | tee /dev/stderr | grep -q "^root\$$"
	bash iamroot-shell -c "stat --print '%u:%g\n' ." | tee /dev/stderr | grep -q "^0:0$$"
	bash iamroot-shell -c "echo \$$IAMROOTLVL" | tee /dev/stderr | grep -q "^[0-9]\+$$"

.PHONY: alpine-tests
alpine-tests: libiamroot.so | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs pwd" | tee /dev/stderr | grep -q "^/\$$"
	bash iamroot-shell -c "chroot alpine-minirootfs cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	bash iamroot-shell --path /bin:/usr/bin:/sbin:/usr/sbin -c "chroot alpine-minirootfs chroot . cat /etc/os-release" | tee /dev/stderr | grep 'NAME="Alpine Linux"'

.PHONY: shell
shell: libiamroot.so
	bash iamroot-shell

.PHONY: chroot
chroot: libiamroot.so | static-rootfs
	bash iamroot-shell -c "chroot static-rootfs/bin/sh"

.PHONY: mini-chroot
mini-chroot: libiamroot.so | alpine-minirootfs
	bash iamroot-shell -c "chroot alpine-minirootfs /bin/sh"

.PHONY: arch-chroot
arch-chroot: | arch-rootfs
	bash iamroot-shell -c "chroot arch-rootfs"

.PHONY: static-rootfs
static-rootfs: static-rootfs/usr/bin/sh
static-rootfs: static-rootfs/bin
static-rootfs: static-rootfs/root

static-rootfs/usr/bin/sh: PATH := $(CURDIR):$(PATH)
static-rootfs/usr/bin/sh: | busybox static-rootfs/usr/bin
	busybox --install $(@D)

busybox:
	wget https://busybox.net/downloads/binaries/1.31.0-defconfig-multiarch-musl/busybox-x86_64
	chmod +x busybox-x86_64
	mv busybox-x86_64 $@

static-rootfs/bin: | static-rootfs/usr/bin
	ln -sf usr/bin $@

static-rootfs/usr/bin static-rootfs/root:
	mkdir -p $@

.PHONY: alpine-minirootfs
alpine-minirootfs: | alpine-minirootfs/bin/busybox

alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.13.0-x86_64.tar.gz
	mkdir -p alpine-minirootfs
	tar xf alpine-minirootfs-3.13.0-x86_64.tar.gz -C alpine-minirootfs

alpine-minirootfs-3.13.0-x86_64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64/alpine-minirootfs-3.13.0-x86_64.tar.gz

.PHONY: arch-rootfs
arch-rootfs: | arch-rootfs/etc/machine-id

arch-rootfs/boot/vmlinuz-linux: libiamroot.so | arch-rootfs/etc/machine-id
	bash iamroot-shell -c "pacman -r arch-rootfs --noconfirm -S linux"

arch-rootfs/etc/machine-id: libiamroot.so
	mkdir -p arch-rootfs
	bash iamroot-shell -c "pacstrap arch-rootfs"

.PHONY: clean
clean:
	rm -f libiamroot.so *.o
	chmod -f +w arch-rootfs/etc/ca-certificates/extracted/cadir || true
	rm -Rf static-rootfs/ alpine-minirootfs/ arch-rootfs/
	$(MAKE) -C tests $@

.PHONY: mrproper
mrproper: clean
	rm -f busybox alpine-minirootfs-3.13.0-x86_64.tar.gz

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.1: %.1.adoc
	asciidoctor -b manpage -o $@ $<

%.7: %.7.adoc
	asciidoctor -b manpage -o $@ $<

%.gz: %
	gzip -c $< >$@
