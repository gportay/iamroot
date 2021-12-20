#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all:

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

ifneq ($(shell command -v clang),)
all: libiamroot-linux-x86-64-clang.so

.PRECIOUS: libiamroot-linux-x86-64-clang.so
libiamroot-linux-x86-64-clang.so: libiamroot-linux-x86-64-clang.so.2
	ln -sf $< $@

libiamroot-linux-x86-64-clang.so.2:
libiamroot-linux-x86-64-clang.so.2: export CC = clang
endif

ifneq ($(shell command -v musl-gcc),)
all: libiamroot-musl-x86_64.so

rootfs: | libiamroot-musl-x86_64.so

.PRECIOUS: libiamroot-musl-x86_64.so
libiamroot-musl-x86_64.so: libiamroot-musl-x86_64.so.1
	ln -sf $< $@

libiamroot-musl-x86_64.so.1:
libiamroot-musl-x86_64.so.1: export CC = musl-gcc

install: install-exec-musl-x86_64

.PHONY: install-exec-musl-x86_64
install-exec-musl-x86_64:
	install -D -m 755 libiamroot-musl-x86_64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so.1
	ln -sf libiamroot-musl-x86_64.so.1 $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so

uninstall: uninstall-musl-x86_64

.PHONY: uninstall-musl-x86_64
uninstall-musl-x86_64:
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so.1
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/libiamroot-musl-x86_64.so

all test ci alpine-minirootfs alpine-rootfs alpine-3.14-rootfs alpine-3.14-chroot vmlinux-alpine-3.14 qemu-system-x86_64-alpine-3.14 alpine-edge-rootfs alpine-edge-chroot vmlinux-alpine-edge qemu-system-x86_64-alpine-edge: IAMROOT_LIB_MUSL_X86_64 = $(CURDIR)/libiamroot-musl-x86_64.so
all test ci alpine-minirootfs alpine-rootfs alpine-3.14-rootfs alpine-3.14-chroot vmlinux-alpine-3.14 qemu-system-x86_64-alpine-3.14 alpine-edge-rootfs alpine-edge-chroot vmlinux-alpine-edge qemu-system-x86_64-alpine-edge: libiamroot-musl-x86_64.so

alpine-test: IAMROOT_LIB_MUSL_X86_64 = $(CURDIR)/libiamroot-musl-x86_64.so
alpine-test: libiamroot-musl-x86_64.so | alpine-minirootfs
	$(MAKE) -f Makefile $@
	bash iamroot-shell -c "chroot alpine-minirootfs /lib/ld-musl-x86_64.so.1 --preload "$(IAMROOT_LIB_MUSL_X86_64)" bin/busybox"

endif

all: | libiamroot-linux-x86-64.so.2

%:
	$(MAKE) -f Makefile $@
