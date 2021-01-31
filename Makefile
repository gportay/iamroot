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

.PHONY: clean
clean:
	rm -f libiamroot.so *.o
	rm -Rf rootfs/

.PHONY: mrproper
mrproper: clean
	rm -R busybox

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
