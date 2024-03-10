#
# Copyright 2023-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/pkg
export PREFIX

O ?= output
ARCH ?= $(shell uname -p 2>/dev/null)

IAMROOT_LIB ?= $(CURDIR)/$(ARCH)/libiamroot.elf_so
export IAMROOT_LIB

IAMROOT_LIB_X86_64_ELF ?= $(CURDIR)/$(ARCH)/libiamroot.elf_so
export IAMROOT_LIB_X86_64_ELF

IAMROOT_LIB_AARCH64_ELF ?= $(CURDIR)/$(ARCH)/libiamroot.elf_so
export IAMROOT_LIB_AARCH64_ELF

IAMROOT_PATH_RESOLUTION_IGNORE ?= ^/dev/|^/proc/
export IAMROOT_PATH_RESOLUTION_IGNORE

IAMROOT_EXEC ?= $(CURDIR)/exec.sh
export IAMROOT_EXEC

-include local.mk

.PHONY: all
all: libiamroot.so

.PRECIOUS: libiamroot.so
libiamroot.so: $(ARCH)/libiamroot.elf_so
	install -m755 $< $@

.PRECIOUS: $(ARCH)/libiamroot.elf_so
$(ARCH)/libiamroot.elf_so: $(O)-$(ARCH)/libiamroot.so
	install -d -m755 $(@D)
	install -m755 $< $@

.PRECIOUS: $(O)-%/libiamroot.so
$(O)-%/libiamroot.so: $(wildcard *.c) | $(O)-%
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: $(O)-%
$(O)-%:
	mkdir $@

netbsd-9.3-chroot: export PATH = /sbin:/usr/sbin:/bin:/usr/bin:/usr/pkg/sbin:/usr/pkg/bin:/usr/X11R7/bin:/usr/local/sbin:/usr/local/bin
netbsd-9.3-chroot: export SHELL = /bin/sh
netbsd-9.3-chroot: $(ARCH)/libiamroot.elf_so | netbsd-9.3-rootfs
	bash ido $(IDOFLAGS) chroot $@

ifeq ($(ARCH),x86_64)
netbsd-9.3-rootfs: | $(ARCH)/libiamroot.elf_so base.tar.xz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xzf base.tar.xz -C $@.tmp
	mv $@.tmp $@
else
netbsd-9.3-rootfs: | $(ARCH)/libiamroot.elf_so base.tgz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xzf base.tgz -C $@.tmp
	mv $@.tmp $@
endif

base.tar.xz:
	wget http://cdn.netbsd.org/pub/NetBSD/NetBSD-9.3/$(ARCH)/binary/sets/base.tar.xz -O $@

base.tgz:
	wget http://cdn.netbsd.org/pub/NetBSD/NetBSD-9.3/$(shell uname -m)-$(ARCH)/binary/sets/base.tgz -O $@

.PHONY: test ci ido ish
test ci ido ish: libiamroot.so
	$(MAKE) -f Makefile $@

.PHONY: cleanall
cleanall: clean
	rm -Rf netbsd-*-rootfs/

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf $(O)-*/
	rm -Rf $(ARCH)/

%:
	$(MAKE) -f Makefile $@
