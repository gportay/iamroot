#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

ARCH ?= $(shell uname -m 2>/dev/null)

IAMROOT_LIB = $(CURDIR)/$(ARCH)/libiamroot-elf.so.1
export IAMROOT_LIB

IAMROOT_LIB_ELF_1 = $(CURDIR)/$(ARCH)/libiamroot-elf.so.1
export IAMROOT_LIB_ELF_1

IAMROOT_PATH_RESOLUTION_IGNORE = ^/dev/|^/etc/resolv.conf
export IAMROOT_PATH_RESOLUTION_IGNORE

IAMROOT_PATH = /sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin
export IAMROOT_PATH

IAMROOT_EXEC = $(CURDIR)/exec.sh
export IAMROOT_EXEC

-include local.mk

.PHONY: all
all: libiamroot.so

.PRECIOUS: libiamroot.so
libiamroot.so: $(ARCH)/libiamroot-elf.so.1
	install -m755 $< $@

.PRECIOUS: $(ARCH)/libiamroot-elf.so.1
$(ARCH)/libiamroot-elf.so.1: output-$(ARCH)/libiamroot.so
	install -d -m755 $(@D)
	install -m755 $< $@

.PRECIOUS: output-%/libiamroot.so
output-%/libiamroot.so: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: output-%
output-%:
	mkdir $@

freebsd-13.1-chroot:
freebsd-%-chroot: $(ARCH)/libiamroot-elf.so.1 | freebsd-%-rootfs
	bash iamroot-shell -c "chroot freebsd-$*-rootfs"

freebsd-13.1-rootfs:
freebsd-%-rootfs: | $(ARCH)/libiamroot-elf.so.1 FreeBSD-%-RELEASE-base-$(ARCH).txz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xf FreeBSD-$*-RELEASE-base-$(ARCH).txz -C $@.tmp
	mv $@.tmp $@

FreeBSD-13.1-RELEASE-base-$(ARCH).txz:
FreeBSD-%-RELEASE-base-$(ARCH).txz:
	wget https://download.freebsd.org/releases/$(ARCH)/$*-RELEASE/base.txz -O $@

.PHONY: test
test: libiamroot.so
test:
	$(MAKE) -f Makefile $@

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf freebsd-*-rootfs/
	rm -Rf output-*/
	rm -Rf $(ARCH)/

%:
	$(MAKE) -f Makefile $@
