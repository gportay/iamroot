#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

CC = musl-gcc
LDFLAGS += -nolibc

.PHONY: all
all: musl-all

.PHONY: clean
clean: musl-clean

.PHONY: musl-all
musl-all: libiamroot-musl.so

libiamroot-musl.so: libiamroot.so
	cp $< $@

.PHONY: musl-clean
musl-clean:
	rm -f libiamroot-musl.so

include Makefile
