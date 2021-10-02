#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

MAKEFLAGS += --no-print-directory

.PHONY: all
all: libiamroot-linux-x86-64.so

libiamroot-linux-x86-64.so: libiamroot-linux-x86-64.so.2
	ln -sf $< $@

libiamroot-musl-x86_64.so: libiamroot-musl-x86_64.so.1
	ln -sf $< $@

libiamroot.so: $(wildcard *.c)

libiamroot-musl-x86_64.so.1: export CC = musl-gcc
libiamroot-musl-x86_64.so.1: export LDFLAGS += -nolibc

.PRECIOUS: libiamroot-%.so
libiamroot-%.so.1 libiamroot-%.so.2: $(wildcard *.c) | output-%
	$(MAKE) -f $(CURDIR)/Makefile -C output-$* libiamroot.so VPATH=$(CURDIR)
	cp output-$*/libiamroot.so $@

.PRECIOUS: output-%
output-%:
	mkdir $@

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf output-*/
	rm -f libiamroot*.so*

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
