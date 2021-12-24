#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all:

.PRECIOUS: libiamroot-%.so.1 libiamroot-%.so.2
libiamroot-%.so.1 libiamroot-%.so.2: output-%/libiamroot.so
	cp output-$*/libiamroot.so $@

.PRECIOUS: output-%/libiamroot.so
output-%/libiamroot.so: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: output-%
output-%:
	mkdir $@

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf output-*/
	rm -f libiamroot*.so*

ifneq ($(shell command -v clang),)
all: libiamroot-linux-x86-64-clang.so

.PRECIOUS: libiamroot-linux-x86-64-clang.so
libiamroot-linux-x86-64-clang.so: libiamroot-linux-x86-64-clang.so.2
	ln -sf $< $@

libiamroot-linux-x86-64-clang.so.2:
libiamroot-linux-x86-64-clang.so.2: export CC = clang
endif

ifneq ($(shell command -v musl-gcc),)
all: libiamroot-musl-x86_64.so

.PRECIOUS: libiamroot-musl-x86_64.so
libiamroot-musl-x86_64.so: libiamroot-musl-x86_64.so.1
	ln -sf $< $@

libiamroot-musl-x86_64.so.1:
libiamroot-musl-x86_64.so.1: export CC = musl-gcc

install: install-exec-musl-x86_64

.PHONY: install-exec-musl-x86_64
install-exec-musl-x86_64:
	install -D -m 755 libiamroot-musl-x86_64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so.1
	ln -sf libiamroot-musl-x86_64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so

uninstall: uninstall-musl-x86_64

.PHONY: uninstall-musl-x86_64
uninstall-musl-x86_64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so

all test ci rootfs alpine-minirootfs alpine-rootfs alpine-3.14-rootfs alpine-3.14-chroot vmlinux-alpine-3.14 qemu-system-x86_64-alpine-3.14 alpine-edge-rootfs alpine-edge-chroot vmlinux-alpine-edge qemu-system-x86_64-alpine-edge alpine-3.14.ext4 alpine-edge.ext4: IAMROOT_LIB_MUSL_X86_64 = $(CURDIR)/libiamroot-musl-x86_64.so
all test ci rootfs alpine-minirootfs alpine-rootfs alpine-3.14-rootfs alpine-3.14-chroot vmlinux-alpine-3.14 qemu-system-x86_64-alpine-3.14 alpine-edge-rootfs alpine-edge-chroot vmlinux-alpine-edge qemu-system-x86_64-alpine-edge alpine-3.14.ext4 alpine-edge.ext4: libiamroot-musl-x86_64.so

alpine-test: IAMROOT_LIB_MUSL_X86_64 = $(CURDIR)/libiamroot-musl-x86_64.so
alpine-test: libiamroot-musl-x86_64.so | alpine-minirootfs
	$(MAKE) -f Makefile $@
	bash iamroot-shell -c "chroot alpine-minirootfs /lib/ld-musl-x86_64.so.1 --preload "$(IAMROOT_LIB_MUSL_X86_64)" bin/busybox"
endif

ifneq ($(shell command -v aarch64-linux-gnu-gcc),)
all: libiamroot-linux-aarch64.so

rootfs: | libiamroot-linux-aarch64.so

.PRECIOUS: libiamroot-musl-aarch64.so
libiamroot-linux-aarch64.so: libiamroot-linux-aarch64.so.1
	ln -sf $< $@

libiamroot-linux-aarch64.so.1:
libiamroot-linux-aarch64.so.1: export CC = aarch64-linux-gnu-gcc

install: install-exec-linux-aarch64

.PHONY: install-exec-linux-aarch64
install-exec-linux-aarch64:
	install -D -m 755 libiamroot-linux-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so.1
	ln -sf libiamroot-linux-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so

uninstall: uninstall-exec-linux-aarch64

.PHONY: uninstall-exec-linux-aarch64
uninstall-exec-linux-aarch64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-linux-aarch64.so

all test ci rootfs aarch64-arch-rootfs aarch64-arch-chroot aarch64-fedora-33-rootfs aarch64-fedora-34-rootfs aarch64-fedora-35-rootfs aarch64-fedora-33-chroot aarch64-fedora-34-chroot aarch64-fedora-35-chroot: IAMROOT_LIB_LINUX_AARCH64 = $(CURDIR)/libiamroot-linux-aarch64.so
all test ci rootfs aarch64-arch-rootfs aarch64-arch-chroot aarch64-fedora-33-rootfs aarch64-fedora-34-rootfs aarch64-fedora-35-rootfs aarch64-fedora-33-chroot aarch64-fedora-34-chroot aarch64-fedora-35-chroot: libiamroot-linux-aarch64.so

.PHONY: aarch64-arch-chroot
aarch64-arch-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-arch-rootfs
aarch64-arch-chroot: | aarch64-arch-rootfs
	bash iamroot-shell -c "chroot aarch64-arch-rootfs"

.PHONY: aarch64-arch-rootfs
aarch64-arch-rootfs: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-arch-rootfs
aarch64-arch-rootfs: | aarch64-arch-rootfs/etc/machine-id

aarch64-arch-rootfs/etc/machine-id: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-arch-rootfs
aarch64-arch-rootfs/etc/machine-id: | libiamroot-linux-aarch64.so libiamroot.so
	mkdir -p aarch64-arch-rootfs
	bash iamroot-shell -c "pacstrap -C support/aarch64-pacman.conf aarch64-arch-rootfs"

aarch64-fedora-33-chroot:
aarch64-fedora-34-chroot:
aarch64-fedora-35-chroot:
aarch64-fedora-%-chroot: export IAMROOT_PATH = /usr/bin:/usr/sbin:/bin:/sbin
aarch64-fedora-%-chroot: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64:/usr/lib64/sssd:/usr/lib:/lib:/usr/lib/systemd
aarch64-fedora-%-chroot: | aarch64-fedora-%-rootfs
	bash iamroot-shell -c "chroot aarch64-fedora-$*-rootfs"

aarch64-fedora-33-rootfs: | aarch64-fedora-33-rootfs/etc/machine-id
aarch64-fedora-34-rootfs: | aarch64-fedora-34-rootfs/etc/machine-id
aarch64-fedora-35-rootfs: | aarch64-fedora-35-rootfs/etc/machine-id

aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_LD_LIBRARY_PATH = /usr/lib64:/lib64:/usr/lib64/sssd:/usr/lib:/lib:/usr/lib/systemd
aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_EXEC_IGNORE = ldd
aarch64-fedora-%-rootfs/etc/machine-id: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev|run/.+)/|$(CURDIR)/aarch64-fedora-$*-rootfs/var/log/dnf.rpm.log
aarch64-fedora-%-rootfs/etc/machine-id: | libiamroot-linux-aarch64.so libiamroot.so
	install -D -m644 fedora.repo aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
	bash iamroot-shell -c "dnf --forcearch aarch64 --releasever $* --assumeyes --installroot $(CURDIR)/aarch64-fedora-$*-rootfs group install minimal-environment"
	rm -f aarch64-fedora-$*-rootfs/etc/distro.repos.d/fedora.repo
endif

ifneq ($(shell command -v aarch64-linux-musl-gcc),)
all: libiamroot-musl-aarch64.so

.PRECIOUS: libiamroot-musl-aarch64.so
libiamroot-musl-aarch64.so: libiamroot-musl-aarch64.so.1
	ln -sf $< $@

libiamroot-musl-aarch64.so.1:
libiamroot-musl-aarch64.so.1: export CC = aarch64-linux-musl-gcc

install: install-exec-musl-aarch64

.PHONY: install-exec-musl-aarch64
install-exec-musl-aarch64:
	install -D -m 755 libiamroot-musl-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so.1
	ln -sf libiamroot-musl-aarch64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so

uninstall: uninstall-exec-musl-aarch64

.PHONY: uninstall-exec-musl-aarch64
uninstall-exec-musl-aarch64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-aarch64.so

all test ci rootfs aarch64-alpine-minirootfs aarch64-mini-chroot aarch64-alpine-3.14-rootfs aarch64-alpine-edge-rootfs aarch64-alpine-3.14-chroot aarch64-alpine-edge-chroot: libiamroot-musl-aarch64.so
all test ci rootfs aarch64-alpine-minirootfs aarch64-mini-chroot aarch64-alpine-3.14-rootfs aarch64-alpine-edge-rootfs aarch64-alpine-3.14-chroot aarch64-alpine-edge-chroot: IAMROOT_LIB_MUSL_AARCH64 = $(CURDIR)/libiamroot-linux-aarch64.so

aarch64-mini-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-mini-rootfs
aarch64-mini-chroot: | libiamroot-musl-aarch64.so libiamroot.so aarch64-alpine-minirootfs
	bash iamroot-shell -c "chroot aarch64-alpine-minirootfs /bin/sh"

.PHONY: aarch64-alpine-minirootfs
aarch64-alpine-minirootfs: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-minirootfs
aarch64-alpine-minirootfs: | aarch64-alpine-minirootfs/bin/busybox

aarch64-alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.13.0-aarch64.tar.gz
	mkdir -p aarch64-alpine-minirootfs
	tar xf alpine-minirootfs-3.13.0-aarch64.tar.gz -C aarch64-alpine-minirootfs

alpine-minirootfs-3.13.0-aarch64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/aarch64/alpine-minirootfs-3.13.0-aarch64.tar.gz

aarch64-alpine-3.14-chroot aarch64-alpine-edge-chroot:
aarch64-alpine-%-chroot: export QEMU_LD_PREFIX = $(CURDIR)/aarch64-alpine-$*-rootfs
aarch64-alpine-%-chroot: | aarch64-alpine-%-rootfs
	bash iamroot-shell -c "chroot aarch64-alpine-$*-rootfs"

aarch64-alpine-3.14-rootfs: | aarch64-alpine-3.14-rootfs/bin/busybox
aarch64-alpine-edge-rootfs: | aarch64-alpine-edge-rootfs/bin/busybox

aarch64-alpine-%-rootfs/bin/busybox: | libiamroot-musl-aarch64.so libiamroot.so
	bash iamroot-shell -c "APK_OPTS='--arch aarch64' alpine-make-rootfs aarch64-alpine-$*-rootfs --mirror-uri http://nl.alpinelinux.org/alpine --branch $*"
endif

all: | libiamroot-linux-x86-64.so.2

%-debug %-debug1:
	$(MAKE) $* IAMROOT_DEBUG=1

%-debug2:
	$(MAKE) $* IAMROOT_DEBUG=2

%-debug3:
	$(MAKE) $* IAMROOT_DEBUG=3

%:
	$(MAKE) -f Makefile $@
