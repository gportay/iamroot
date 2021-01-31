#
# Copyright 2020-2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

override CFLAGS += -fPIC -Wall -Wextra -Werror

.PHONY: all
all: libiamroot.so

libiamroot.so: chroot.o
libiamroot.so: execve.o
libiamroot.so: geteuid.o
libiamroot.so: path_resolution.o
libiamroot.so: override LDLIBS += -ldl

.PHONY: tests
tests: SHELL = /bin/bash
tests: export LD_PRELOAD += $(CURDIR)/libiamroot.so
tests: libiamroot.so | rootfs
	whoami | tee /dev/stderr | grep -q root
	IAMROOT_GETEUID=$$EUID whoami | tee /dev/stderr | grep -q $$USER
	chroot rootfs pwd | tee /dev/stderr | grep -q /

.PHONY: shell-tests
shell-tests: export LD_LIBRARY_PATH := $(CURDIR)
shell-tests: export PATH := $(CURDIR):$(PATH)
shell-tests: libiamroot.so | rootfs
	iamroot-shell -c "whoami" | tee /dev/stderr | grep -q root
	iamroot-shell -c "chroot rootfs pwd" | tee /dev/stderr | grep -q /

.PHONY: alpine-tests
alpine-tests: export LD_PRELOAD += $(CURDIR)/libiamroot.so
alpine-tests: libiamroot.so | alpine-minirootfs
	chroot alpine-minirootfs pwd | tee /dev/stderr | grep -q /
	chroot alpine-minirootfs /bin/sh | tee /dev/stderr | grep -q /

.PHONY: rootfs
rootfs: rootfs/usr/bin/sh
rootfs: rootfs/bin
rootfs: rootfs/root

rootfs/usr/bin/sh: PATH := $(CURDIR):$(PATH)
rootfs/usr/bin/sh: | busybox rootfs/usr/bin
	busybox --install $(@D)

busybox:
	wget https://busybox.net/downloads/binaries/1.31.0-defconfig-multiarch-musl/busybox-x86_64
	chmod +x busybox-x86_64
	mv busybox-x86_64 $@

rootfs/bin: | rootfs/usr/bin
	ln -sf usr/bin $@

rootfs/usr/bin rootfs/root:
	mkdir -p $@

.PHONY: alpine-minirootfs
alpine-minirootfs: | alpine-minirootfs/bin/busybox

alpine-minirootfs/bin/busybox: | alpine-minirootfs-3.13.0-x86_64.tar.gz
	mkdir -p alpine-minirootfs
	tar xf alpine-minirootfs-3.13.0-x86_64.tar.gz -C alpine-minirootfs

alpine-minirootfs-3.13.0-x86_64.tar.gz:
	wget http://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64/alpine-minirootfs-3.13.0-x86_64.tar.gz

.PHONY: clean
clean:
	rm -f libiamroot.so *.o
	rm -Rf rootfs/ alpine-minirootfs/

.PHONY: mrproper
mrproper: clean
	rm -f busybox alpine-minirootfs-3.13.0-x86_64.tar.gz

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
