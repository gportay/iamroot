#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

IAMROOT_LIB = $(CURDIR)/x86_64/libiamroot-elf.so.1
export IAMROOT_LIB

IAMROOT_LIB_ELF_1 = $(CURDIR)/x86_64/libiamroot-elf.so.1
export IAMROOT_LIB_ELF_1

IAMROOT_PATH_RESOLUTION_IGNORE = ^/(dev|var/run)/|^/etc/resolv.conf
export IAMROOT_PATH_RESOLUTION_IGNORE

IAMROOT_PATH = /sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin
export IAMROOT_PATH

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

-include local.mk

.PHONY: all
all: x86_64/libiamroot-elf.so.1

.PRECIOUS: x86_64/libiamroot-elf.so.1
x86_64/libiamroot-elf.so.1: $(wildcard *.c)

freebsd-13.1-chroot:
freebsd-%-chroot: x86_64/libiamroot-elf.so.1 | freebsd-%-rootfs
	bash iamroot-shell -c "chroot freebsd-$*-rootfs"

freebsd-13.1-rootfs:
freebsd-%-rootfs: | x86_64/libiamroot-elf.so.1 FreeBSD-%-RELEASE-base-amd64.txz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xf FreeBSD-$*-RELEASE-base-amd64.txz -C $@.tmp
	mv $@.tmp $@

FreeBSD-13.1-RELEASE-base-amd64.txz:
FreeBSD-%-RELEASE-base-amd64.txz:
	wget https://download.freebsd.org/releases/amd64/$*-RELEASE/base.txz -O $@

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf freebsd-*-rootfs/

%:
	$(MAKE) -f Makefile $@
