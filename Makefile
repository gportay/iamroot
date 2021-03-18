#
# Copyright 2020-2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

PREFIX ?= /usr/local

override CFLAGS += -fPIC -Wall -Wextra -Werror
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

libiamroot.so: __fxstatat.o
libiamroot.so: __lxstat.o
libiamroot.so: __statfs.o
libiamroot.so: __xmknod.o
libiamroot.so: __xmknodat.o
libiamroot.so: __xstat.o
libiamroot.so: __fprintf.o
libiamroot.so: __printf.o
libiamroot.so: access.o
libiamroot.so: canonicalize_file_name.o
libiamroot.so: chdir.o
libiamroot.so: chmod.o
libiamroot.so: chown.o
libiamroot.so: chroot.o
libiamroot.so: creat.o
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
libiamroot.so: fpath_resolutionat.o
libiamroot.so: freopen.o
libiamroot.so: fstatat.o
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
libiamroot.so: lutimes.o
libiamroot.so: mkdir.o
libiamroot.so: mkdirat.o
libiamroot.so: mkfifo.o
libiamroot.so: mkfifoat.o
libiamroot.so: mknod.o
libiamroot.so: mknodat.o
libiamroot.so: mkostemp.o
libiamroot.so: mkostemps.o
libiamroot.so: mkstemp.o
libiamroot.so: mkstemps.o
libiamroot.so: mount.o
libiamroot.so: open.o
libiamroot.so: openat.o
libiamroot.so: opendir.o
libiamroot.so: path_resolution.o
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
libiamroot.so: scandirat.o
libiamroot.so: setxattr.o
libiamroot.so: stat.o
libiamroot.so: statfs.o
libiamroot.so: statvfs.o
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

.PHONY: install
install:
	install -D -m 755 iamroot-shell $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	sed -e "s,/usr/local/,$(PREFIX)/," -i $(DESTDIR)$(PREFIX)/bin/iamroot-shell
	install -D -m 755 libiamroot.so $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot.so
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
run-qemu: export LD_PRELOAD = $(CURDIR)/libiamroot.so
run-qemu: export EUID = 0
run-qemu: export IAMROOT_FORCE = 1
run-qemu: export IAMROOT_EXEC = $(CURDIR)/exec.sh
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
static-tests: export LD_PRELOAD = $(CURDIR)/libiamroot.so
static-tests: libiamroot.so | static-rootfs
	whoami | tee /dev/stderr | grep -q "^root\$$"
	IAMROOT_GETEUID=$$EUID whoami | tee /dev/stderr | grep -q "^$$USER\$$"

.PHONY: shell-tests
shell-tests: export PATH := $(CURDIR):$(PATH)
shell-tests: export IAMROOT_LIB = $(CURDIR)/libiamroot.so
shell-tests: export IAMROOT_EXEC = $(CURDIR)/exec.sh
shell-tests: libiamroot.so | static-rootfs
	iamroot-shell -c "whoami" | tee /dev/stderr | grep -q "^root\$$"
	iamroot-shell -c "stat --print '%u:%g\n' ." | tee /dev/stderr | grep -q "^0:0$$"
	iamroot-shell -c "echo \$$IAMROOTLVL" | tee /dev/stderr | grep -q "^[0-9]\+$$"

.PHONY: alpine-tests
alpine-tests: export LD_PRELOAD = $(CURDIR)/libiamroot.so
alpine-tests: export IAMROOT_EXEC = $(CURDIR)/exec.sh
alpine-tests: libiamroot.so | alpine-minirootfs
	chroot alpine-minirootfs pwd | tee /dev/stderr | grep -q "^/\$$"
	chroot alpine-minirootfs cat /etc/os-release | tee /dev/stderr | grep 'NAME="Alpine Linux"'

.PHONY: shell
shell: export PATH := $(CURDIR):$(PATH)
shell: export IAMROOT_LIB = $(CURDIR)/libiamroot.so
shell: export IAMROOT_EXEC = $(CURDIR)/exec.sh
shell: libiamroot.so
	iamroot-shell

.PHONY: chroot
chroot: export LD_PRELOAD = $(CURDIR)/libiamroot.so
chroot: export IAMROOT_EXEC = $(CURDIR)/exec.sh
chroot: export PATH := $(CURDIR):/bin:/sbin
chroot: libiamroot.so | static-rootfs
	chroot static-rootfs /bin/sh

.PHONY: alpine-chroot
alpine-chroot: export LD_PRELOAD = $(CURDIR)/libiamroot.so
alpine-chroot: export IAMROOT_EXEC = $(CURDIR)/exec.sh
alpine-chroot: export PATH := $(CURDIR):/bin:/sbin
alpine-chroot: libiamroot.so | alpine-minirootfs
	chroot alpine-minirootfs /bin/sh

.PHONY: arch-chroot
arch-chroot: | arch-rootfs
	bash iamroot-shell --library $(CURDIR)/libiamroot.so --exec $(CURDIR)/exec.sh -c "chroot arch-rootfs"

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
arch-rootfs: | arch-rootfs/usr/bin/pacman

arch-rootfs/boot/vmlinuz-linux: export LD_PRELOAD = $(CURDIR)/libiamroot.so
arch-rootfs/boot/vmlinuz-linux: export IAMROOT_EXEC = $(CURDIR)/exec.sh
arch-rootfs/boot/vmlinuz-linux: export EUID = 0
arch-rootfs/boot/vmlinuz-linux: export IAMROOT_FORCE = 1
arch-rootfs/boot/vmlinuz-linux: libiamroot.so | arch-rootfs/usr/bin/pacman
	pacman -r arch-rootfs --noconfirm -S linux

arch-rootfs/usr/bin/pacman: export LD_PRELOAD = $(CURDIR)/libiamroot.so
arch-rootfs/usr/bin/pacman: export IAMROOT_EXEC = $(CURDIR)/exec.sh
arch-rootfs/usr/bin/pacman: export EUID = 0
arch-rootfs/usr/bin/pacman: export IAMROOT_FORCE = 1
arch-rootfs/usr/bin/pacman: libiamroot.so
	mkdir -p arch-rootfs
	pacstrap arch-rootfs

.PHONY: clean
clean:
	rm -f libiamroot.so *.o
	rm -Rf static-rootfs/ alpine-minirootfs/ arch-rootfs/
	$(MAKE) -C tests $@

.PHONY: mrproper
mrproper: clean
	rm -f busybox alpine-minirootfs-3.13.0-x86_64.tar.gz

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
