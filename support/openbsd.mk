#
# Copyright 2023-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

O ?= output
ARCH ?= $(shell uname -p 2>/dev/null)

IAMROOT_PATH_RESOLUTION_IGNORE ?= ^/dev/|^/etc/resolv.conf
export IAMROOT_PATH_RESOLUTION_IGNORE

IAMROOT_ORIGIN ?= $(CURDIR)
export IAMROOT_ORIGIN

-include local.mk

.PHONY: all
all: ld-iamroot.so
all: libiamroot.so

.PRECIOUS: libiamroot.so
libiamroot.so: $(ARCH)/libiamroot.so
	install -m755 $< $@

.PRECIOUS: $(ARCH)/libiamroot.so
$(ARCH)/libiamroot.so: $(O)-$(ARCH)/libiamroot.so
	install -d -m755 $(@D)
	install -m755 $< $@

.PRECIOUS: $(O)-%/libiamroot.so
$(O)-%/libiamroot.so: $(wildcard *.c) | $(O)-%
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: $(O)-%
$(O)-%:
	mkdir $@

openbsd-7.3-chroot: export IDO_SECURE_PATH = /sbin:/usr/sbin:/bin:/usr/bin:/usr/X11R6/bin:/usr/local/sbin:/usr/local/bin
openbsd-7.3-chroot: export SHELL = /bin/sh
openbsd-7.3-chroot: PATH := $(CURDIR):$(PATH)
openbsd-7.3-chroot: ld-iamroot.so libiamroot.so $(ARCH)/libiamroot.so | openbsd-7.3-rootfs
	ido $(IDOFLAGS) chroot openbsd-7.3-rootfs

openbsd-7.3-rootfs: | ld-iamroot.so libiamroot.so $(ARCH)/libiamroot.so base.tgz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xzf base.tgz -C $@.tmp
	mv $@.tmp $@

base.tgz:
	wget https://cdn.openbsd.org/pub/OpenBSD/7.3/$(shell uname -m)/base73.tgz -O $@

.PHONY: test ci ido ish
test ci ido ish: libiamroot.so
	$(MAKE) -f Makefile $@

.PHONY: cleanall
cleanall: clean
	rm -Rf openbsd-*-rootfs/

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf $(O)-*/
	rm -Rf $(ARCH)/

%:
	$(MAKE) -f Makefile $@
