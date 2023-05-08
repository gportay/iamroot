#
# Copyright 2023 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

O ?= output
ARCH ?= $(shell uname -m 2>/dev/null)

IAMROOT_LIB = $(CURDIR)/$(ARCH)/libiamroot.so
export IAMROOT_LIB

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

openbsd-73-chroot: $(ARCH)/libiamroot.so | openbsd-73-rootfs
	bash iamroot-shell -c "chroot $@"

openbsd-73-rootfs: | $(ARCH)/libiamroot.so base-73.tgz
	rm -Rf $@
	mkdir -p $@.tmp
	tar xzf base-73.tgz -C $@.tmp
	mv $@.tmp $@

base-73.tgz:
	wget https://cdn.openbsd.org/pub/OpenBSD/7.3/amd64/base73.tgz -O $@

.PHONY: test ci
test ci: libiamroot.so
	$(MAKE) -f Makefile $@

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf openbsd-*-rootfs/
	rm -Rf $(O)-*/
	rm -Rf $(ARCH)/

%:
	$(MAKE) -f Makefile $@
