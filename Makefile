#
# Copyright 2020 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

override CFLAGS += -fPIC

.PHONY: all
all: libiamroot.so

libiamroot.so: geteuid.o

.PHONY: tests
tests: export LD_PRELOAD += $(CURDIR)/libiamroot.so
tests: libiamroot.so
	whoami | tee /dev/stderr | grep -q root

.PHONY: shell-tests
shell-tests: export LD_LIBRARY_PATH := $(CURDIR)
shell-tests: export PATH := $(CURDIR):$(PATH)
shell-tests: libiamroot.so
	iamroot-shell -c "whoami" | tee /dev/stderr | grep -q root

.PHONY: clean
clean:
	rm -f libiamroot.so *.o

%.so: override LDFLAGS += -shared
%.so:
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
