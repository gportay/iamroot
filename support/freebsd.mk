#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

IAMROOT_LIB = $(CURDIR)/amd64/libiamroot-elf.so.1
export IAMROOT_LIB

IAMROOT_LIB_ELF_1 = $(CURDIR)/amd64/libiamroot-elf.so.1
export IAMROOT_LIB_ELF_1

IAMROOT_PATH_RESOLUTION_IGNORE = ^/dev/|^/etc/resolv.conf
export IAMROOT_PATH_RESOLUTION_IGNORE

IAMROOT_PATH = /sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin
export IAMROOT_PATH

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

-include local.mk

.PHONY: all
all: amd64/libiamroot-elf.so.1

.PRECIOUS: amd64/libiamroot-elf.so.1
amd64/libiamroot-elf.so.1: output-amd64/libiamroot.so
	install -d -m755 $(@D)
	install -m755 $< $@

.PRECIOUS: output-%/libiamroot.so
output-%/libiamroot.so: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: output-%
output-%:
	mkdir $@

freebsd-13.1-chroot:
freebsd-%-chroot: amd64/libiamroot-elf.so.1 | freebsd-%-rootfs
	bash iamroot-shell -c "chroot freebsd-$*-rootfs"

freebsd-13.1-rootfs:
freebsd-%-rootfs: | amd64/libiamroot-elf.so.1 FreeBSD-%-RELEASE-base-amd64.txz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xf FreeBSD-$*-RELEASE-base-amd64.txz -C $@.tmp
	mv $@.tmp $@

FreeBSD-13.1-RELEASE-base-amd64.txz:
FreeBSD-%-RELEASE-base-amd64.txz:
	wget https://download.freebsd.org/releases/amd64/$*-RELEASE/base.txz -O $@

.PHONY: test
test: export IAMROOT_FATAL ?= 0
test: amd64/libiamroot-elf.so.1
test:
	$(MAKE) -f Makefile $@

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf freebsd-*-rootfs/
	rm -Rf output-*/
	rm -Rf amd64/

%:
	$(MAKE) -f Makefile $@
