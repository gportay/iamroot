#
## Copyright (C) 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

MAKEFLAGS += --no-print-directory

.PHONY: all
all: libiamroot-linux-x86-64.so
all: libiamroot-musl-x86_64.so

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

alpine-rootfs/bin/busybox: libiamroot-musl-x86_64.so.1

alpine-tests: libiamroot-musl-x86_64.so.1

tests: alpine-tests

ci: alpine-tests
ci: libiamroot-musl-x86_64.so.1

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf output-*/
	rm -f libiamroot*.so*

alpine-chroot: libiamroot-musl-x86_64.so

%:
	$(MAKE) -f Makefile $@
