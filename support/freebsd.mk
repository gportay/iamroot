#
# Copyright 2022-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

O ?= output
ARCH ?= $(shell uname -m 2>/dev/null)

IAMROOT_LIB ?= $(CURDIR)/$(ARCH)/libiamroot-elf.so.1
export IAMROOT_LIB

IAMROOT_LIB_AMD64_ELF_1 ?= $(CURDIR)/$(ARCH)/libiamroot-elf.so.1
export IAMROOT_LIB_AMD64_ELF_1

IAMROOT_LIB_ARM64_ELF_1 ?= $(CURDIR)/$(ARCH)/libiamroot-elf.so.1
export IAMROOT_LIB_ARM64_ELF_1

IAMROOT_PATH_RESOLUTION_IGNORE ?= ^/dev/|^/etc/resolv.conf
export IAMROOT_PATH_RESOLUTION_IGNORE

IAMROOT_EXEC ?= $(CURDIR)/exec.sh
export IAMROOT_EXEC

-include local.mk

.PHONY: all
all: libiamroot.so

.PRECIOUS: libiamroot.so
libiamroot.so: $(ARCH)/libiamroot-elf.so.1
	install -m755 $< $@

.PRECIOUS: $(ARCH)/libiamroot-elf.so.1
$(ARCH)/libiamroot-elf.so.1: $(O)-$(ARCH)/libiamroot.so
	install -d -m755 $(@D)
	install -m755 $< $@

.PRECIOUS: $(O)-%/libiamroot.so
$(O)-%/libiamroot.so: $(wildcard *.c) | $(O)-%
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: $(O)-%
$(O)-%:
	mkdir $@

freebsd-13.1-chroot:
freebsd-%-chroot: export PATH = /sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin:/root/bin
freebsd-%-chroot: export SHELL = /bin/sh
freebsd-%-chroot: $(ARCH)/libiamroot-elf.so.1 | freebsd-%-rootfs
	bash ish -c "chroot freebsd-$*-rootfs"

freebsd-13.1-rootfs:
freebsd-%-rootfs: | $(ARCH)/libiamroot-elf.so.1 FreeBSD-%-RELEASE-base-$(ARCH).txz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xf FreeBSD-$*-RELEASE-base-$(ARCH).txz -C $@.tmp
	mv $@.tmp $@

FreeBSD-13.1-RELEASE-base-$(ARCH).txz:
FreeBSD-%-RELEASE-base-$(ARCH).txz:
	wget https://download.freebsd.org/releases/$(ARCH)/$*-RELEASE/base.txz -O $@

.PHONY: test ci ish
test ci ish: libiamroot.so
	$(MAKE) -f Makefile $@

.PHONY: cleanall
cleanall: clean
	rm -Rf freebsd-*-rootfs/

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf $(O)-*/
	rm -Rf $(ARCH)/

%:
	$(MAKE) -f Makefile $@
