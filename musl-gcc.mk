#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

CC = musl-gcc
CFLAGS += -DARG_MAX=$(shell getconf ARG_MAX)
LDFLAGS += -nolibc

include Makefile
