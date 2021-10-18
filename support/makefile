#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all: libiamroot.so
all: libiamroot-linux-x86-64.so

.PRECIOUS: libiamroot.so
libiamroot.so: libiamroot-linux-x86-64.so
	ln -sf $< $@

.PRECIOUS: libiamroot-linux-x86-64.so
libiamroot-linux-x86-64.so: libiamroot-linux-x86-64.so.2
	ln -sf $< $@

.PRECIOUS: libiamroot-musl-x86_64.so
libiamroot-musl-x86_64.so: libiamroot-musl-x86_64.so.1
	ln -sf $< $@

libiamroot.so: $(wildcard *.c)

libiamroot-musl-x86_64.so.1: export CC = musl-gcc
libiamroot-musl-x86_64.so.1: export LDFLAGS += -nolibc

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

alpine-test: IAMROOT_LIB_MUSL_X86_64 = $(CURDIR)/libiamroot-musl-x86_64.so
alpine-test: libiamroot-musl-x86_64.so | alpine-minirootfs
	$(MAKE) -f Makefile $@
	bash iamroot-shell -c "chroot alpine-minirootfs /lib/ld-musl-x86_64.so.1 --preload "$(IAMROOT_LIB_MUSL_X86_64)" bin/busybox"

alpine-chroot: libiamroot-musl-x86_64.so

ifneq ($(shell command -v musl-gcc),)
all: libiamroot-musl-x86_64.so

alpine-rootfs/bin/busybox: libiamroot-musl-x86_64.so.1

alpine-test: libiamroot-musl-x86_64.so.1

test: alpine-test

ci: alpine-test
ci: libiamroot-musl-x86_64.so.1
endif

%:
	$(MAKE) -f Makefile $@
