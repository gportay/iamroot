#
# Copyright 2021-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

PREFIX ?= /usr/local
O ?= output
CCMACH ?= $(shell $(CC) -dumpmachine 2>/dev/null)
LIBC ?= $(if $(findstring musl,$(CCMACH)),musl,linux)
ARCH ?= $(shell uname -m 2>/dev/null)
KVER ?= $(shell uname -r 2>/dev/null)
VMLINUX_KVER ?= $(shell vmlinux --version 2>/dev/null)
KVM ?= $(shell test -c /dev/kvm 2>/dev/null && echo 1)
CLANG ?= 0
COVERAGE ?= 0

NVERBOSE ?= 0
export NVERBOSE

IAMROOT_ORIGIN ?= $(CURDIR)
export IAMROOT_ORIGIN

ifeq ($(ARCH),x86_64)
vpath := $(O)-$(ARCH)-$(LIBC)-x86-64
else
vpath := $(O)-$(ARCH)-$(LIBC)-$(ARCH)
endif

-include local.mk

MAKEFLAGS += --no-print-directory

.PHONY: all
all: ld-iamroot.so
all: libiamroot.so

define libs
$(strip libiamroot.so \
	$(if $(findstring :$(2):,:arm: :armel:                     ),$(if $(findstring :$(1):,:musl:),arm/libiamroot-musl-arm.so.1                ,arm/libiamroot-linux.so.3                  ), \
	$(if $(findstring :$(2):,:armhf: :armv7: :armv7hl: :armv7h:),$(if $(findstring :$(1):,:musl:),armhf/libiamroot-musl-armhf.so.1            ,armhf/libiamroot-linux-armhf.so.3          ), \
	$(if $(findstring :$(2):,:aarch64: :arm64:                 ),$(if $(findstring :$(1):,:musl:),aarch64/libiamroot-musl-aarch64.so.1        ,aarch64/libiamroot-linux-aarch64.so.1      ), \
	$(if $(findstring :$(2):,:aarch64_be:                      ),$(if $(findstring :$(1):,:musl:),aarch64_be/libiamroot-musl-aarch64_be.so.1  ,aarch64_be/libiamroot-linux-aarch64_be.so.1), \
	$(if $(findstring :$(2):,:mipsle: :mipsel:                 ),$(if $(findstring :$(1):,:musl:),mipsle/libiamroot-musl-mipsel.so.1          ,mipsle/libiamroot.so.1                     ), \
	$(if $(findstring :$(2):,:mips64le: :mips64el:             ),$(if $(findstring :$(1):,:musl:),mips64le/libiamroot-musl-mips64el.so.1      ,mips64le/libiamroot.so.1                   ), \
	$(if $(findstring :$(2):,:powerpc: :ppc:                   ),$(if $(findstring :$(1):,:musl:),powerpc/libiamroot-musl-powerpc.so.1        ,powerpc/libiamroot.so.1                    ), \
	$(if $(findstring :$(2):,:powerpc64: :ppc64:               ),$(if $(findstring :$(1):,:musl:),powerpc64/libiamroot-musl-powerpc64.so.1    ,powerpc64/libiamroot.so.2                  ), \
	$(if $(findstring :$(2):,:powerpc64le: :ppc64le: :ppc64el: ),$(if $(findstring :$(1):,:musl:),powerpc64le/libiamroot-musl-powerpc64le.so.1,powerpc64le/libiamroot.so.2                ), \
	$(if $(findstring :$(2):,:riscv64:                         ),$(if $(findstring :$(1):,:musl:),riscv64/libiamroot-musl-riscv64.so.1        ,riscv64/libiamroot-linux-riscv64-lp64d.so.1), \
	$(if $(findstring :$(2):,:s390x:                           ),$(if $(findstring :$(1):,:musl:),s390x/libiamroot-musl-s390x.so.1            ,s390x/libiamroot.so.1                      ), \
	$(if $(findstring :$(2):,:x86_64: :amd64:                  ),$(if $(findstring :$(1):,:musl:),x86_64/libiamroot-musl-x86_64.so.1          ,x86_64/libiamroot-linux-x86-64.so.2        ), \
	$(if $(findstring :$(2):,:x86: :i386: :i686: :pmmx:        ),$(if $(findstring :$(1):,:musl:),i686/libiamroot-musl-i386.so.1              ,i686/libiamroot-linux.so.2                 ), \
	$(error $(1)-$(2): No such library)))))))))))))) \
)
endef

define libiamroot_so_abi =
all: $(1)/libiamroot.so.$(2)

.PRECIOUS: $(1)/libiamroot.so.$(2)
$(1)/libiamroot.so.$(2): $(O)-$(1)/libiamroot.so
	install -D -m755 $$< $$@

$(O)-$(1)/libiamroot.so:

install: install-exec-$(1).$(2)

.PHONY: install-exec-$(1).$(2)
install-exec-$(1).$(2):
	install -D -m755 $(1)/libiamroot.so.$(2) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot.so.$(2)
	ln -sf libiamroot.so.$(2) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot.so

uninstall: uninstall-$(1).$(2)

.PHONY: uninstall-$(1).$(2)
uninstall-$(1).$(2):
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot.so.$(2)
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot.so

clean: clean-$(1).$(2)

.PHONY: clean-$(1).$(2)
clean-$(1).$(2):
	rm -Rf $(O)-$(1)/
	rm -Rf $(1)/
endef

define libiamroot_ldso_so_abi =
all: $(1)/libiamroot-$(2).so.$(3)

.PRECIOUS: $(1)/libiamroot-$(2).so.$(3)
$(1)/libiamroot-$(2).so.$(3): $(O)-$(1)-$(2)/libiamroot.so
	install -D -m755 $$< $$@

$(O)-$(1)-$(2)/libiamroot.so:

install: install-exec-$(1)-$(2).$(3)

.PHONY: install-exec-$(1)-$(2).$(3)
install-exec-$(1)-$(2).$(3):
	install -D -m755 $(1)/libiamroot-$(2).so.$(3) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so.$(3)
	ln -sf libiamroot-$(2).so.$(3) $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so

uninstall: uninstall-$(1)-$(2).$(3)

.PHONY: uninstall-$(1)-$(2).$(3)
uninstall-$(1)-$(2).$(3):
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so.$(3)
	rm -f $(DESTDIR)$(PREFIX)/lib/iamroot/$(1)/libiamroot-$(2).so

clean: clean-$(1)-$(2).$(3)

.PHONY: clean-$(1)-$(2).$(3)
clean-$(1)-$(2).$(3):
	rm -Rf $(O)-$(1)-$(2)/
	rm -Rf $(1)/
endef

define log
.PRECIOUS: support/$(2).txt
support/$(2).txt: $(2).log
	support/$(1).sed -e 's,$(CURDIR),\$$$$ROOT,g' $$< >$$@.tmp
	mv $$@.tmp $$@

.PRECIOUS: $(2).log
$(2).log: SHELL = /bin/bash -o pipefail
$(2).log:
	$(MAKE) --silent $(2) 2>&1 | tee $$@.tmp
	mv $$@.tmp $$@
endef

define chroot_shell
.PHONY: $(1)-$(2)-chroot
$(1)-$(2)-chroot: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-chroot: | $(1)-$(2)-rootfs
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs $(3)

.PHONY: $(1)-$(2)-shell
$(1)-$(2)-shell: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-shell: libiamroot.so
	@echo $(4)
	ish
endef

define pacstrap-rootfs
.PRECIOUS: $(1)-$(2)-rootfs/bin/sh
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export EUID = 0

$(eval $(call chroot_shell,$(1),$(2),/bin/bash,pacstrap -GMC support/$(1)-$(2)-pacman.conf $(1)-$(2)-rootfs $(3)))

$(1)-$(2)-rootfs: | $(1)-$(2)-rootfs/bin/sh
$(1)-$(2)-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-rootfs/bin/sh: | ld-iamroot.so $(call libs,linux,$(1))
	ido $$(IDOFLAGS) mkdir -p $(1)-$(2)-rootfs
	ido $$(IDOFLAGS) pacstrap -GMC support/$(1)-$(2)-pacman.conf $(1)-$(2)-rootfs $(3)

$(eval $(call log,pacstrap,$(1)-$(2)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2))) \
	$(eval $(call pacstrap-postrootfs,$(1),$(2))) \
)
endef

define debootstrap-rootfs
.PRECIOUS: $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: IDOFLAGS += --multiarch
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^/dev/(null|zero|full|random|urandom|tty|console|pts|shm|ptmx)|^$(CURDIR)/.*\.gcda
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/debian
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= $(3)
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export DEBOOTSTRAPFLAGS ?=

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/sh,debootstrap --keep-debootstrap-dir $$(DEBOOTSTRAPFLAGS) $(3) $(1)-$(2)-$(3)-rootfs $$(DEBOOTSTRAP_MIRROR) $$(DEBOOTSTRAP_SCRIPT)))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-$(3)-rootfs/bin/sh: | ld-iamroot.so $(call libs,linux,$(1))
	ido $$(IDOFLAGS) mkdir -p $(1)-$(2)-$(3)-rootfs
	ido $$(IDOFLAGS) debootstrap --keep-debootstrap-dir --arch=$(1) $$(DEBOOTSTRAPFLAGS) $(3) $(1)-$(2)-$(3)-rootfs $$(DEBOOTSTRAP_MIRROR) $$(DEBOOTSTRAP_SCRIPT)
	ido $$(IDOFLAGS) cat $(1)-$(2)-$(3)-rootfs/debootstrap/debootstrap.log
	ido $$(IDOFLAGS) rm -Rf $(1)-$(2)-$(3)-rootfs/debootstrap/

$(eval $(call log,debootstrap,$(1)-$(2)-$(3)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call debootstrap-postrootfs,$(1),$(2)-$(3))) \
)
endef

define mmdebstrap-rootfs
.PRECIOUS: $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: IDOFLAGS += --multiarch
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^/dev/(null|zero|full|random|urandom|tty|console|pts|shm|ptmx)|^$(CURDIR)/.*\.gcda
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?=

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/sh,mmdebstrap --verbose --architectures=$(1) $$(MMDEBSTRAPFLAGS) $(1)-$(2)-$(3)-rootfs <support/$(2)-$(3)-sources.list))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-$(3)-rootfs/bin/sh: | ld-iamroot.so $(call libs,linux,$(1))
	ido $$(IDOFLAGS) mmdebstrap --verbose --architectures=$(1) $$(MMDEBSTRAPFLAGS) $(3) $(1)-$(2)-$(3)-rootfs - <support/$(2)-$(3)-sources.list

$(eval $(call log,mmdebstrap,$(1)-$(2)-$(3)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call debootstrap-postrootfs,$(1),$(2)-$(3))) \
)
endef

define dnf-rootfs
.PRECIOUS: $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora.repo

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/bash,dnf --forcearch $(1) --releasever $(3) --assumeyes --installroot $(CURDIR)/$(1)-$(2)-$(3)-rootfs group install minimal-environment))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-$(3)-rootfs/bin/sh: | ld-iamroot.so $(call libs,linux,$(1))
	ido $$(IDOFLAGS) install -D -m644 $$(FEDORA_REPO) $(1)-$(2)-$(3)-rootfs/etc/distro.repos.d/fedora.repo
	ido $$(IDOFLAGS) dnf --forcearch $(1) --releasever $(3) --assumeyes --installroot $(CURDIR)/$(1)-$(2)-$(3)-rootfs group install "$(4)"
	ido $$(IDOFLAGS) rm -f $(1)-$(2)-$(3)-rootfs/etc/distro.repos.d/fedora.repo

$(eval $(call log,dnf,$(1)-$(2)-$(3)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call dnf-postrootfs,$(1),$(2)-$(3))) \
)
endef

define zypper-rootfs
.PRECIOUS: $(1)-$(2)-rootfs/bin/sh
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|/usr/bin/chkstat
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys)/|^$(CURDIR)/.*\.gcda

$(if $(findstring 0,$$(COVERAGE)),, \
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_LIB_X86_64_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1:$(CURDIR)/gcompat/libgcompat.so.0
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_LIB_I686_MUSL_I386_1 = $(CURDIR)/i686/libiamroot-musl-i386.so.1:$(CURDIR)/gcompat-i386/libgcompat.so.0
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_WARNING_IGNORE = $(CURDIR)/gcompat/libgcompat.so.0
)

$(eval $(call chroot_shell,$(1),$(2),/bin/sh,zypper --root $(CURDIR)/$(1)-$(2)-rootfs --non-interactive --no-gpg-checks install patterns-base-minimal_base zypper systemd))

$(1)-$(2)-rootfs: | $(1)-$(2)-rootfs/bin/sh
$(1)-$(2)-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-rootfs/bin/sh: | ld-iamroot.so $(call libs,linux,$(1))
	ido $$(IDOFLAGS) zypper --root $(CURDIR)/$(1)-$(2)-rootfs addrepo --no-gpgcheck support/$(2)-repo-oss.repo
	ido $$(IDOFLAGS) zypper --root $(CURDIR)/$(1)-$(2)-rootfs --non-interactive --no-gpg-checks install patterns-base-minimal_base zypper systemd

$(eval $(call log,zypper,$(1)-$(2)-rootfs))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2))) \
	$(eval $(call zypper-postrootfs,$(1),$(2))) \
)
endef

define xbps-install-rootfs
.PRECIOUS: $(1)-$(2)-rootfs/bin/sh
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda

$(if $(findstring 0,$$(COVERAGE)),, \
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_LIB_X86_64_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1:$(CURDIR)/gcompat/libgcompat.so.0
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_LIB_I686_MUSL_I386_1 = $(CURDIR)/i686/libiamroot-musl-i386.so.1:$(CURDIR)/gcompat-i386/libgcompat.so.0
$(1)-$(2)-chroot $(1)-$(2)-shell $(1)-$(2)-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_WARNING_IGNORE = $(CURDIR)/gcompat/libgcompat.so.0
)

$(eval $(call chroot_shell,$(1),$(2),/bin/bash,xbps-install -S -r $(1)-$(2)-rootfs -R http://repo-default.voidlinux.org/current base-system))

$(1)-$(2)-rootfs: | $(1)-$(2)-rootfs/bin/sh
$(1)-$(2)-rootfs/bin/sh: export XBPS_ARCH=$(1)
$(1)-$(2)-rootfs/bin/sh: IDOFLAGS += --preserve-env=XBPS_ARCH
$(1)-$(2)-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-rootfs/bin/sh: | ld-iamroot.so $(call libs,linux,$(1))
	ido $$(IDOFLAGS) install -D -t $(1)-$(2)-rootfs/var/db/xbps/keys/ /var/db/xbps/keys/*
	ido $$(IDOFLAGS) xbps-install -S -y -r $(1)-$(2)-rootfs -R http://repo-default.voidlinux.org/current base-system

$(eval $(call log,xbps-install,$(1)-$(2)-rootfs))
endef

define xbps-install-musl-rootfs
.PRECIOUS: $(1)-$(2)-musl-rootfs/bin/sh
$(1)-$(2)-musl-chroot $(1)-$(2)-musl-shell $(1)-$(2)-musl-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_IGNORE = ^/(proc|sys|dev)/|^$(CURDIR)/.*\.gcda

$(if $(findstring 0,$$(COVERAGE)),, \
$(1)-$(2)-musl-chroot $(1)-$(2)-musl-shell $(1)-$(2)-musl-rootfs/bin/sh: export IAMROOT_LIB_X86_64_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1:$(CURDIR)/gcompat/libgcompat.so.0
$(1)-$(2)-musl-chroot $(1)-$(2)-musl-shell $(1)-$(2)-musl-rootfs/bin/sh: export IAMROOT_LIB_I686_MUSL_I386_1 = $(CURDIR)/i686/libiamroot-musl-i386.so.1:$(CURDIR)/gcompat-i386/libgcompat.so.0
$(1)-$(2)-musl-chroot $(1)-$(2)-musl-shell $(1)-$(2)-musl-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_WARNING_IGNORE = $(CURDIR)/gcompat/libgcompat.so.0
)

$(eval $(call chroot_shell,$(1),$(2)-musl,/bin/bash,xbps-install -S -r $(1)-$(2)-musl-rootfs -R http://repo-default.voidlinux.org/current/musl base-system))

$(1)-$(2)-musl-rootfs: | $(1)-$(2)-musl-rootfs/bin/sh
$(1)-$(2)-musl-rootfs/bin/sh: export XBPS_ARCH=$(1)-musl
$(1)-$(2)-musl-rootfs/bin/sh: IDOFLAGS += --preserve-env=XBPS_ARCH
$(1)-$(2)-musl-rootfs/bin/sh: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-musl-rootfs/bin/sh: | ld-iamroot.so $(call libs,musl,$(1))
	ido $$(IDOFLAGS) install -D -t $(1)-$(2)-musl-rootfs/var/db/xbps/keys/ /var/db/xbps/keys/*
	ido $$(IDOFLAGS) xbps-install -S -y -r $(1)-$(2)-musl-rootfs -R http://repo-default.voidlinux.org/current/musl base-system

$(eval $(call log,xbps-install,$(1)-$(2)-musl-rootfs))
endef

define alpine-make-rootfs-generic-rootfs
.PRECIOUS: $(1)-$(2)-$(3)-rootfs$(4)
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs$(4): export APK_OPTS += --arch $(1) --no-progress
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs$(4): IDOFLAGS += --preserve-env=APK_OPTS
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs$(4): export ALPINE_MAKE_ROOTFSFLAGS ?= --packages apk-tools --packages openrc --mirror-uri http://mirrors.edge.kernel.org/alpine

$(if $(findstring 0,$$(COVERAGE)),, \
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs$(4): export IAMROOT_LIB_X86_64_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1:$(CURDIR)/gcompat/libgcompat.so.0
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs$(4): export IAMROOT_LIB_I686_MUSL_I386_1 = $(CURDIR)/i686/libiamroot-musl-i386.so.1:$(CURDIR)/gcompat-i386/libgcompat.so.0
$(1)-$(2)-$(3)-chroot $(1)-$(2)-$(3)-shell $(1)-$(2)-$(3)-rootfs$(4): export IAMROOT_PATH_RESOLUTION_WARNING_IGNORE = $(CURDIR)/gcompat/libgcompat.so.0
)

$(eval $(call chroot_shell,$(1),$(2)-$(3),/bin/sh,alpine-make-rootfs $(1)-$(2)-$(3)-rootfs --keys-dir /usr/share/apk/keys/$(1) --branch $(3) $$(ALPINE_MAKE_ROOTFSFLAGS)))

$(1)-$(2)-$(3)-rootfs: | $(1)-$(2)-$(3)-rootfs$(4)
$(1)-$(2)-$(3)-rootfs$(4): PATH := $(CURDIR):$(PATH)
$(1)-$(2)-$(3)-rootfs$(4): | ld-iamroot.so $(call libs,musl,$(1))
	ido $$(IDOFLAGS) alpine-make-rootfs $(1)-$(2)-$(3)-rootfs --keys-dir /usr/share/apk/keys/$(1) --branch $(3) $$(ALPINE_MAKE_ROOTFSFLAGS)

$(eval $(call log,alpine-make-rootfs,$(1)-$(2)-$(3)-rootfs))
endef

define alpine-make-rootfs-alpine-rootfs
$(eval $(call alpine-make-rootfs-generic-rootfs,$(1),$(2),$(3),/bin/busybox))

$(if $(findstring x86_64,$(1)), \
	$(eval $(call run,$(1),$(2)-$(3))) \
	$(eval $(call alpinelinux-postrootfs,$(1),$(2)-$(3))) \
)
endef

define alpine-make-rootfs-adelie-rootfs
.PRECIOUS: $(1)-$(2)-$(3)-rootfs/bin/sh
$(1)-$(2)-$(3)-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --packages adelie-core --repositories-file support/adelie-repositories
$(eval $(call alpine-make-rootfs-generic-rootfs,$(1),$(2),$(3),/bin/sh))
$(1)-$(2)-$(3)-rootfs/bin/sh: export APK_OPTS += --allow-untrusted
$(1)-$(2)-$(3)-rootfs/bin/sh: $(1)-$(2)-$(3)-rootfs/etc/alpine-release

.INTERMEDIATE: $(1)-$(2)-$(3)-rootfs/etc/alpine-release
$(1)-$(2)-$(3)-rootfs/etc/alpine-release:
	install -D -m644 /dev/null $$@
endef

define run
ifneq ($(KVM),0)
.PHONY: qemu-system-$(1)-$(2)
qemu-system-$(1)-$(2): override CMDLINE += panic=5
qemu-system-$(1)-$(2): override CMDLINE += console=ttyS0
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -enable-kvm -cpu host
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -m 4G -machine q35 -smp 4
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -nographic -serial mon:stdio
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -kernel /boot/vmlinuz-linux
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -initrd initrd-rootfs.cpio
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -drive file=$(1)-$(2).ext4,if=virtio
qemu-system-$(1)-$(2): override QEMUSYSTEMFLAGS += -append "$$(CMDLINE)"
qemu-system-$(1)-$(2): | $(1)-$(2).ext4 initrd-rootfs.cpio
	qemu-system-x86_64 $$(QEMUSYSTEMFLAGS)
endif

ifneq ($(VMLINUX_KVER),)
.PHONY: vmlinux-$(2)
vmlinux-$(2): override VMLINUXFLAGS+=panic=5
vmlinux-$(2): override VMLINUXFLAGS+=console=tty0 con0=fd:0,fd:1 con=none
vmlinux-$(2): override VMLINUXFLAGS+=mem=256M
vmlinux-$(2): override VMLINUXFLAGS+=rw
vmlinux-$(2): override VMLINUXFLAGS+=ubd0=$(1)-$(2).ext4
vmlinux-$(2): override VMLINUXFLAGS+=$$(CMDLINE)
vmlinux-$(2): | $(1)-$(2).ext4
	settings=$$$$(stty -g); \
	if ! vmlinux $$(VMLINUXFLAGS); then \
		echo stty "$$$$settings"; \
		false; \
	fi; \
	stty "$$$$settings"
endif

.PRECIOUS: $(1)-$(2).ext4
ifneq ($(KVER),)
MODULESDIRS_$(1)-$(2) += $(1)-$(2)-rootfs/usr/lib/modules/$(KVER)
endif
ifneq ($(VMLINUX_KVER),)
MODULESDIRS_$(1)-$(2) += $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER)
endif
$(1)-$(2).ext4: PATH := $(CURDIR):$(PATH)
$(1)-$(2).ext4: | x86_64/libiamroot-linux-x86-64.so.2 $(1)-$(2)-rootfs $$(MODULESDIRS_$(1)-$(2))
	$(MAKE) $(1)-$(2)-postrootfs
	rm -f $$@.tmp
	fallocate --length 2G $$@.tmp
	ido $$(IDOFLAGS) mkfs.ext4 -d $(1)-$(2)-rootfs $$@.tmp
	mv $$@.tmp $$@

.PHONY: $(1)-$(2)-postrootfs
$(1)-$(2)-postrootfs:

.PRECIOUS: $(1)-$(2)-rootfs/usr/lib/modules/$(KVER) $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER)
$(1)-$(2)-rootfs/usr/lib/modules/$(KVER) $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER): PATH := $(CURDIR):$(PATH)
$(1)-$(2)-rootfs/usr/lib/modules/$(KVER) $(1)-$(2)-rootfs/usr/lib/modules/$(VMLINUX_KVER): | x86_64/libiamroot-linux-x86-64.so.2 $(1)-$(2)-rootfs
	rm -Rf $$@.tmp $$@
	ido $$(IDOFLAGS) mkdir -p $$(@D)
	ido $$(IDOFLAGS) rsync -a /usr/lib/modules/$$(@F)/. $$@.tmp/.
	mv $$@.tmp $$@

.PHONY: chroot-$(1)-$(2)
chroot-$(1)-$(2):
	$(MAKE) mount-$(1)-$(2)
	-sudo chroot mnt $(SHELL)
	$(MAKE) umount-$(1)-$(2)

.PHONY: mount-$(1)-$(2)
mount-$(1)-$(2): | $(1)-$(2).ext4 mnt
	sudo mount -oloop $(1)-$(2).ext4 mnt

.PHONY: umount-$(1)-$(2)
umount-$(1)-$(2): | mnt
	sudo umount mnt
endef

define pacstrap-postrootfs
$(1)-$(2)-postrootfs: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	mkdir -p $(1)-$(2)-rootfs/var/lib/systemd/linger
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service
endef

define debootstrap-postrootfs
$(1)-$(2)-postrootfs: IDOFLAGS += --multiarch
$(1)-$(2)-postrootfs: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	if test -e $(1)-$(2)-rootfs/lib/systemd/systemd; then \
		rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service; \
		ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service; \
		rm -f $(1)-$(2)-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service; \
		ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl disable sshd.service; \
	else \
		sed -e '/^1:/i0:2345:respawn:/sbin/getty --noclear 38400 tty0' \
		    -e '/^[1-9]:/s,^,#,' \
		    -e '/^#T0:/s,^#,,g' \
		    -i $(1)-$(2)-rootfs/etc/inittab; \
	fi
endef

define dnf-postrootfs
$(1)-$(2)-postrootfs: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	touch $(1)-$(2)-rootfs/etc/systemd/zram-generator.conf
	mkdir -p $(1)-$(2)-rootfs/var/lib/systemd/linger
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/multi-user.target.wants/sshd.service
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl disable sshd.service
endef

define zypper-postrootfs
$(1)-$(2)-postrootfs: PATH := $(CURDIR):$(PATH)
$(1)-$(2)-postrootfs: | x86_64/libiamroot-linux-x86-64.so.2
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root:\*:/s,^root:\*:,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs pam-config -a --nullok
	mkdir -p $(1)-$(2)-rootfs/var/lib/systemd/linger
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@tty0.service
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl enable getty@tty0.service
	rm -f $(1)-$(2)-rootfs/etc/systemd/system/getty.target.wants/getty@ttyS0.service
	ido $$(IDOFLAGS) chroot $(1)-$(2)-rootfs systemctl enable getty@ttyS0.service
endef

define alpinelinux-postrootfs
$(1)-$(2)-postrootfs:
	sed -e '/^root:x:/s,^root:x:,root::,' \
	    -i $(1)-$(2)-rootfs/etc/passwd
	sed -e '/^root::/s,^root::,root:x:,' \
	    -i $(1)-$(2)-rootfs/etc/shadow
	sed -e '/^UNKNOWN$$:/d' \
	    -e '/^tty0$$:/d' \
	    -e '1iUNKNOWN' \
	    -e '1itty0' \
	    -i $(1)-$(2)-rootfs/etc/securetty
	sed -e '/^tty1:/itty0::respawn:/sbin/getty 38400 tty0' \
	    -e '/^tty[1-9]:/s,^,#,' \
	    -e '/^#ttyS0:/s,^#,,g' \
	    -i $(1)-$(2)-rootfs/etc/inittab
	chmod +r $(1)-$(2)-rootfs/bin/bbsuid
endef

define void-rootfs
$(eval $(call chroot_shell,$(1),void,/bin/bash,tar xf void-$(1)-ROOTFS-$(2).tar.xz -C $(1)-void-rootfs))
$(1)-void-chroot $(1)-void-shell: | x86_64/libiamroot-linux-x86-64.so.2

$(1)-void-rootfs: | x86_64/libiamroot-linux-x86-64.so.2 void-$(1)-ROOTFS-$(2).tar.xz
	mkdir -p $(1)-void-rootfs
	tar xf void-$(1)-ROOTFS-$(2).tar.xz -C $(1)-void-rootfs

void-$(1)-ROOTFS-$(2).tar.xz:
	wget http://repo-default.voidlinux.org/live/current/void-$(1)-ROOTFS-$(2).tar.xz
endef

define void-musl-rootfs
$(eval $(call chroot_shell,$(1),void-musl,/bin/bash,tar xf void-$(1)-musl-ROOTFS-$(2).tar.xz -C $(1)-void-musl-rootfs))
$(1)-void-musl-chroot $(1)-void-musl-shell: | x86_64/libiamroot-linux-x86-64.so.2 x86_64/libiamroot-musl-x86_64.so.1

$(1)-void-musl-rootfs: | x86_64/libiamroot-linux-x86-64.so.2 x86_64/libiamroot-musl-x86_64.so.1 void-$(1)-musl-ROOTFS-$(2).tar.xz
	mkdir -p $(1)-void-musl-rootfs
	tar xf void-$(1)-musl-ROOTFS-$(2).tar.xz -C $(1)-void-musl-rootfs

void-$(1)-musl-ROOTFS-$(2).tar.xz:
	wget http://repo-default.voidlinux.org/live/current/void-$(1)-musl-ROOTFS-$(2).tar.xz
endef

define adelie-mini-rootfs
$(eval $(call chroot_shell,$(1),adelie-mini,/bin/sh,tar xf adelie-rootfs-mini-$(1)-1.0-beta5-$(2).tar.txz -C $(1)-adelie-mini-rootfs))
$(1)-adelie-mini-chroot $(1)-adelie-mini-shell: | ld-iamroot.so $(call libs,musl,$(1))

$(1)-adelie-mini-rootfs: | $(1)-adelie-mini-rootfs/bin/sh
$(1)-adelie-mini-rootfs/bin/sh: | ld-iamroot.so $(call libs,musl,$(1)) adelie-rootfs-mini-$(1)-1.0-beta5-$(2).txz
	mkdir -p $(1)-adelie-mini-rootfs
	tar xf adelie-rootfs-mini-$(1)-1.0-beta5-$(2).txz -C $(1)-adelie-mini-rootfs

adelie-rootfs-mini-$(1)-1.0-beta5-$(2).txz:
	wget https://distfiles.adelielinux.org/adelie/1.0-beta5/iso/adelie-rootfs-mini-$(1)-1.0-beta5-$(2).txz
endef

define alpine-mini-rootfs
$(eval $(call chroot_shell,$(1),alpine-mini,/bin/sh,tar xf alpine-minirootfs-$(2).0-$(1).tar.gz -C $(1)-alpine-mini-rootfs))
$(1)-alpine-mini-chroot $(1)-alpine-mini-shell: | ld-iamroot.so $(call libs,musl,$(1))

$(if $(findstring 0,$$(COVERAGE)),, \
$(1)-alpine-mini-chroot $(1)-alpine-mini-shell $(1)-alpine-mini-rootfs/bin/sh: export IAMROOT_LIB_X86_64_MUSL_X86_64_1 = $(CURDIR)/x86_64/libiamroot-musl-x86_64.so.1:$(CURDIR)/gcompat/libgcompat.so.0
$(1)-alpine-mini-chroot $(1)-alpine-mini-shell $(1)-alpine-mini-rootfs/bin/sh: export IAMROOT_LIB_I686_MUSL_I386_1 = $(CURDIR)/i686/libiamroot-musl-i386.so.1:$(CURDIR)/gcompat-i386/libgcompat.so.0
$(1)-alpine-mini-chroot $(1)-alpine-mini-shell $(1)-alpine-mini-rootfs/bin/sh: export IAMROOT_PATH_RESOLUTION_WARNING_IGNORE = $(CURDIR)/gcompat/libgcompat.so.0
)

$(1)-alpine-mini-rootfs: | $(1)-alpine-mini-rootfs/bin/sh
$(1)-alpine-mini-rootfs/bin/sh: | ld-iamroot.so $(call libs,musl,$(1)) alpine-minirootfs-$(2).0-$(1).tar.gz
	mkdir -p $(1)-alpine-mini-rootfs
	tar xf alpine-minirootfs-$(2).0-$(1).tar.gz -C $(1)-alpine-mini-rootfs

alpine-minirootfs-$(2).0-$(1).tar.gz:
	wget http://mirrors.edge.kernel.org/alpine/v$(2)/releases/$(1)/alpine-minirootfs-$(2).0-$(1).tar.gz

$(1)-alpine-mini-rootfs/usr/bin/%: support/% | $(1)-alpine-mini-rootfs
	cp $$< $$@
endef

export CC
export CFLAGS

ld-iamroot.so: $(O)-$(ARCH)/ld-iamroot.so
	install -D -m755 $< $@

$(ARCH)/ld-iamroot.so: $(O)-$(ARCH)/libiamroot.so
	install -D -m755 $< $@

$(O)-$(ARCH)/ld-iamroot.so: $(wildcard *.c) | $(O)-$(ARCH)
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$(ARCH) ld-iamroot.so VPATH=$(CURDIR)

ifeq ($(ARCH),x86_64)
ifeq ($(LIBC),musl)
libiamroot.so: x86_64/libiamroot-musl-x86_64.so.1
	install -D -m755 $< $@

$(eval $(call libiamroot_ldso_so_abi,x86_64,musl-x86_64,1))
else
libiamroot.so: x86_64/libiamroot-linux-x86-64.so.2
	install -D -m755 $< $@

$(eval $(call libiamroot_ldso_so_abi,x86_64,linux-x86-64,2))

$(O)-i686-linux/libiamroot.so: override CC += -m32
$(O)-i686-linux/libiamroot.so: override CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_ldso_so_abi,i686,linux,2))

ifneq ($(shell command -v arm-buildroot-linux-gnueabi-gcc 2>/dev/null),)
$(O)-arm-linux/libiamroot.so: override CC = arm-buildroot-linux-gnueabi-gcc
$(eval $(call libiamroot_ldso_so_abi,arm,linux,3))
endif

ifneq ($(shell command -v arm-buildroot-linux-gnueabihf-gcc 2>/dev/null),)
$(O)-armhf-linux-armhf/libiamroot.so: override CC = arm-buildroot-linux-gnueabihf-gcc
$(eval $(call libiamroot_ldso_so_abi,armhf,linux-armhf,3))
endif

ifneq ($(shell command -v aarch64-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-aarch64-linux-aarch64/libiamroot.so: override CC = aarch64-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_ldso_so_abi,aarch64,linux-aarch64,1))
endif

ifneq ($(shell command -v aarch64_be-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-aarch64_be-linux-aarch64_be/libiamroot.so: override CC = aarch64_be-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_ldso_so_abi,aarch64_be,linux-aarch64_be,1))
endif

ifneq ($(shell command -v i386-musl-gcc 2>/dev/null),)
$(O)-i686-musl-i386/libiamroot.so: override CC = i386-musl-gcc
$(O)-i686-musl-i386/libiamroot.so: override CFLAGS += -fno-stack-protector
$(eval $(call libiamroot_ldso_so_abi,i686,musl-i386,1))
endif

ifneq ($(shell command -v musl-gcc 2>/dev/null),)
$(O)-x86_64-musl-x86_64/libiamroot.so: override CC = musl-gcc
$(eval $(call libiamroot_ldso_so_abi,x86_64,musl-x86_64,1))
endif

ifneq ($(shell command -v arm-buildroot-linux-musleabi-gcc 2>/dev/null),)
$(O)-arm-musl-arm/libiamroot.so: override CC = arm-buildroot-linux-musleabi-gcc
$(eval $(call libiamroot_ldso_so_abi,arm,musl-arm,1))
endif

ifneq ($(shell command -v arm-buildroot-linux-musleabihf-gcc 2>/dev/null),)
$(O)-armhf-musl-armhf/libiamroot.so: override CC = arm-buildroot-linux-musleabihf-gcc
$(eval $(call libiamroot_ldso_so_abi,armhf,musl-armhf,1))
endif

ifneq ($(shell command -v aarch64-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-aarch64-musl-aarch64/libiamroot.so: override CC = aarch64-buildroot-linux-musl-gcc
$(eval $(call libiamroot_ldso_so_abi,aarch64,musl-aarch64,1))
endif

ifneq ($(shell command -v aarch64_be-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-aarch64_be-musl-aarch64_be/libiamroot.so: override CC = aarch64_be-buildroot-linux-musl-gcc
$(eval $(call libiamroot_ldso_so_abi,aarch64_be,musl-aarch64_be,1))
endif

ifneq ($(shell command -v mipsel-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-mipsle/libiamroot.so: override CC = mipsel-buildroot-linux-gnu-gcc -march=mips32r2
$(eval $(call libiamroot_so_abi,mipsle,1))
endif

ifneq ($(shell command -v mipsel-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-mipsle-musl-mipsel/libiamroot.so: override CC = mipsel-buildroot-linux-musl-gcc -march=mips32r2
$(eval $(call libiamroot_ldso_so_abi,mipsle,musl-mipsel,1))
endif

ifneq ($(shell command -v mips64el-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-mips64le/libiamroot.so: override CC = mips64el-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_so_abi,mips64le,1))
endif

ifneq ($(shell command -v powerpc-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-powerpc/libiamroot.so: override CC = powerpc-buildroot-linux-gnu-gcc -fno-stack-protector
$(eval $(call libiamroot_so_abi,powerpc,1))
endif

ifneq ($(shell command -v powerpc-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-powerpc-musl-powerpc/libiamroot.so: override CC = powerpc-buildroot-linux-musl-gcc -fno-stack-protector
$(eval $(call libiamroot_ldso_so_abi,powerpc,musl-powerpc,1))
endif

ifneq ($(shell command -v powerpc64-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-powerpc64/libiamroot.so: override CC = powerpc64-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_so_abi,powerpc64,2))
endif

ifneq ($(shell command -v powerpc64le-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-powerpc64le/libiamroot.so: override CC = powerpc64le-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_so_abi,powerpc64le,2))
endif

ifneq ($(shell command -v powerpc64-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-powerpc64-musl-powerpc64/libiamroot.so: override CC = powerpc64-buildroot-linux-musl-gcc
$(eval $(call libiamroot_ldso_so_abi,powerpc64,musl-powerpc64,1))
endif

ifneq ($(shell command -v powerpc64le-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-powerpc64le-musl-powerpc64le/libiamroot.so: override CC = powerpc64le-buildroot-linux-musl-gcc
$(eval $(call libiamroot_ldso_so_abi,powerpc64le,musl-powerpc64le,1))
endif

ifneq ($(shell command -v riscv64-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-riscv64-linux-riscv64-lp64d/libiamroot.so: override CC = riscv64-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_ldso_so_abi,riscv64,linux-riscv64-lp64d,1))
endif

ifneq ($(shell command -v riscv64-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-riscv64-musl-riscv64/libiamroot.so: override CC = riscv64-buildroot-linux-musl-gcc
$(eval $(call libiamroot_ldso_so_abi,riscv64,musl-riscv64,1))
endif

ifneq ($(shell command -v s390x-buildroot-linux-gnu-gcc 2>/dev/null),)
$(O)-s390x/libiamroot.so: override CC = s390x-buildroot-linux-gnu-gcc
$(eval $(call libiamroot_so_abi,s390x,1))
endif

ifneq ($(shell command -v s390x-buildroot-linux-musl-gcc 2>/dev/null),)
$(O)-s390x-musl-s390x/libiamroot.so: override CC = s390x-buildroot-linux-musl-gcc
$(eval $(call libiamroot_ldso_so_abi,s390x,musl-s390x,1))
endif
endif

ifneq ($(CLANG),0)
ifneq ($(shell command -v clang 2>/dev/null),)
$(O)-clang-linux-x86-64/libiamroot.so: override CC = clang
$(eval $(call libiamroot_ldso_so_abi,clang,linux-x86-64,2))
endif
endif
endif

ifeq ($(ARCH),aarch64)
libiamroot.so: aarch64/libiamroot-linux-aarch64.so.1
	install -D -m755 $< $@

$(eval $(call libiamroot_ldso_so_abi,aarch64,linux-aarch64,1))
endif

.PRECIOUS: $(O)-%/ld-iamroot.so
$(O)-%/ld-iamroot.so: $(wildcard *.c) | $(O)-%
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$* ld-iamroot.so VPATH=$(CURDIR)

.PRECIOUS: $(O)-%/libiamroot.so
$(O)-%/libiamroot.so: $(wildcard *.c) | $(O)-%
	$(MAKE) -f $(CURDIR)/Makefile -C $(O)-$* libiamroot.so VPATH=$(CURDIR)

.PRECIOUS: $(O)-%
$(O)-%:
	mkdir -p $@

.PHONY: rootfs
rootfs: i686-rootfs

.PHONY: extra-rootfs
extra-rootfs:

.PHONY: stable-rootfs
stable-rootfs:

.PHONY: unstable-rootfs
unstable-rootfs:

.PHONY: broken-rootfs
broken-rootfs:

.PHONY: fixme-rootfs
fixme-rootfs:

.PHONY: i686-rootfs
i686-rootfs:

.PHONY: aarch64-rootfs
aarch64-rootfs:

.PHONY: arm-rootfs
arm-rootfs:

.PHONY: riscv64-rootfs
riscv64-rootfs:

.PHONY: mips-rootfs
mips-rootfs:

.PHONY: mips64-rootfs
mips64-rootfs:

.PHONY: powerpc-rootfs
powerpc-rootfs:

.PHONY: powerpc64-rootfs
powerpc64-rootfs:

.PHONY: ido multiarch-ido ish multiarch-ish
ido multiarch-ido ish multiarch-ish: ld-iamroot.so libiamroot.so
	$(MAKE) -f Makefile $@ VPATH=$(vpath)

.PHONY: test test-library test-libiamroot.so test-frontends test-ld-iamroot.so test-ido test-ish ci
test test-library test-libiamroot.so test-frontends test-ld-iamroot.so test-ido test-ish ci: ld-iamroot.so libiamroot.so
	$(MAKE) -f Makefile $@ VPATH=$(vpath)

.PHONY: coverage
coverage: gcov/index.html

.PHONY: gcov/index.html
gcov/index.html:
	mkdir -p $(@D)
	gcovr --gcov-ignore-parse-errors --html-details --html-title iamroot -s -v -o $@ $(O)-x86_64-linux-x86-64/ tests/

.PHONY: cobertura.xml
cobertura.xml:
	gcovr --gcov-ignore-parse-errors --cobertura -s -v -o $@ $(O)-x86_64-linux-x86-64/ tests/

.PHONY: codacy
codacy: cobertura.xml
	bash <(curl -Ls https://coverage.codacy.com/get.sh)

.PHONY: install
install:
	$(MAKE) -f Makefile $@

.PHONY: uninstall
uninstall:
	$(MAKE) -f Makefile $@

.PHONY: install-support
install-support: install-support-i686
install-support: install-support-x86_64
install-support: install-support-aarch64
install-support: install-support-aarch64_be
install-support: install-support-arm
install-support: install-support-armhf
install-support: install-support-riscv64
install-support: install-support-powerpc
install-support: install-support-powerpc64
install-support: install-support-powerpc64le
install-support: install-support-mips64le
install-support: install-support-mipsle
install-support: install-support-s390x

.PHONY: install-support-i686
install-support-i686:

.PHONY: install-support-x86_64
install-support-x86_64:

.PHONY: install-support-aarch64
install-support-aarch64:

.PHONY: install-support-aarch64_be
install-support-aarch64_be:

.PHONY: install-support-arm
install-support-arm:

.PHONY: install-support-armhf
install-support-armhf:

.PHONY: install-support-riscv64
install-support-riscv64:

.PHONY: install-support-powerpc
install-support-powerpc:

.PHONY: install-support-powerpc64
install-support-powerpc64:

.PHONY: install-support-powerpc64le
install-support-powerpc64le:

.PHONY: install-support-mips64le
install-support-mips64le:

.PHONY: install-support-mipsle
install-support-mipsle:

.PHONY: install-support-s390x
install-support-s390x:

.PHONY: cleanall
cleanall: clean
	rm -f cobertura.xml
	rm -f *.gcda *.gcno
	rm -f *.ext4 *.cpio
	rm -f *-rootfs.log*
	rm -Rf *-rootfs/

.PHONY: clean
clean:
	$(MAKE) -f Makefile $@
	rm -Rf $(O)-$(ARCH)/

ifeq ($(ARCH),x86_64)
ifneq ($(shell command -v pacstrap 2>/dev/null),)
.PHONY: archlinux-test
archlinux-test: | x86_64-archlinux-rootfs/usr/bin/shebang.sh
archlinux-test: | x86_64-archlinux-rootfs/usr/bin/shebang-arg.sh
archlinux-test: $(subst $(CURDIR)/,,$(IAMROOT_LIB)) | x86_64-archlinux-rootfs
	ido $(IDOFLAGS) chroot x86_64-archlinux-rootfs shebang.sh one two three
	ido $(IDOFLAGS) chroot x86_64-archlinux-rootfs shebang-arg.sh one two three

x86_64-archlinux-rootfs/usr/bin/%: support/% | x86_64-archlinux-rootfs
	cp $< $@

rootfs: x86_64-archlinux-rootfs

$(eval $(call pacstrap-rootfs,x86_64,archlinux,base))

install-support-i686:
	install -D -m644 support/i686-archlinux32-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinux32/pacman.conf-i686

i686-rootfs: i686-archlinux32-rootfs

$(eval $(call pacstrap-rootfs,i686,archlinux32,base))

extra-rootfs: x86_64-manjaro-stable-rootfs
stable-rootfs: x86_64-manjaro-stable-rootfs
unstable-rootfs: x86_64-manjaro-unstable-rootfs

$(eval $(call pacstrap-rootfs,x86_64,manjaro-stable,base))
$(eval $(call pacstrap-rootfs,x86_64,manjaro-unstable,base))

install-support-x86_64: install-support-x86_64-pacman

.PHONY: install-support-x86_64-pacman
install-support-x86_64-pacman:
	install -D -m644 support/x86_64-archlinux-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinux/pacman.conf
	install -D -m644 support/x86_64-manjaro-stable-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/manjaro/pacman.conf-stable
	install -D -m644 support/x86_64-manjaro-unstable-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/manjaro/pacman.conf-unstable
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
rootfs: amd64-debian-rootfs

.PHONY: amd64-debian-rootfs
amd64-debian-rootfs: amd64-debian-stretch-rootfs
amd64-debian-rootfs: amd64-debian-buster-rootfs
amd64-debian-rootfs: amd64-debian-bullseye-rootfs
amd64-debian-rootfs: amd64-debian-bookworm-rootfs
amd64-debian-rootfs: amd64-debian-trixie-rootfs
amd64-debian-rootfs: amd64-debian-sid-rootfs

stable-rootfs: amd64-debian-bookworm-rootfs
unstable-rootfs: amd64-debian-sid-rootfs

amd64-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,amd64,debian,stretch))
$(eval $(call debootstrap-rootfs,amd64,debian,buster))
$(eval $(call debootstrap-rootfs,amd64,debian,bullseye))
$(eval $(call debootstrap-rootfs,amd64,debian,bookworm))
$(eval $(call debootstrap-rootfs,amd64,debian,trixie))
$(eval $(call debootstrap-rootfs,amd64,debian,sid))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
amd64-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
amd64-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
amd64-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh

i686-rootfs: i386-debian-rootfs

.PHONY: i386-debian-rootfs
i386-debian-rootfs: i386-debian-stretch-rootfs
i386-debian-rootfs: i386-debian-buster-rootfs
i386-debian-rootfs: i386-debian-bullseye-rootfs
i386-debian-rootfs: i386-debian-bookworm-rootfs
i386-debian-rootfs: i386-debian-trixie-rootfs
i386-debian-rootfs: i386-debian-sid-rootfs

i386-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,i386,debian,stretch))
$(eval $(call debootstrap-rootfs,i386,debian,buster))
$(eval $(call debootstrap-rootfs,i386,debian,bullseye))
$(eval $(call debootstrap-rootfs,i386,debian,bookworm))
$(eval $(call debootstrap-rootfs,i386,debian,trixie))
$(eval $(call debootstrap-rootfs,i386,debian,sid))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
i386-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
i386-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
i386-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh

.PHONY: amd64-ubuntu-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-trusty-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-xenial-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-bionic-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-focal-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-groovy-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-hirsute-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-impish-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-jammy-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-kinetic-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-lunar-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-mantic-rootfs
amd64-ubuntu-rootfs: amd64-ubuntu-noble-rootfs

stable-rootfs: amd64-ubuntu-noble-rootfs

amd64-ubuntu-trusty-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/ubuntu
amd64-ubuntu-xenial-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/ubuntu
amd64-ubuntu-bionic-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/ubuntu
amd64-ubuntu-focal-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/ubuntu
amd64-ubuntu-groovy-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://old-releases.ubuntu.com/ubuntu
amd64-ubuntu-hirsute-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://old-releases.ubuntu.com/ubuntu
amd64-ubuntu-impish-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://old-releases.ubuntu.com/ubuntu
amd64-ubuntu-jammy-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/ubuntu
amd64-ubuntu-kinetic-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://old-releases.ubuntu.com/ubuntu
amd64-ubuntu-lunar-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://old-releases.ubuntu.com/ubuntu
amd64-ubuntu-mantic-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://old-releases.ubuntu.com/ubuntu
amd64-ubuntu-noble-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://mirrors.edge.kernel.org/ubuntu
$(eval $(call debootstrap-rootfs,amd64,ubuntu,trusty))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,xenial))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,bionic))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,focal))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,groovy))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,hirsute))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,impish))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,jammy))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,kinetic))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,lunar))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,mantic))
$(eval $(call debootstrap-rootfs,amd64,ubuntu,noble))
# Setting up makedev ...
# mknod: 'null-': Permission denied
# makedev null c 1 3 root root 0666: failed
# mknod: 'zero-': Permission denied
# makedev zero c 1 5 root root 0666: failed
# mknod: 'full-': Permission denied
# makedev full c 1 7 root root 0666: failed
# mknod: 'random-': Permission denied
# makedev random c 1 8 root root 0666: failed
# mknod: 'urandom-': Permission denied
# makedev urandom c 1 9 root root 0666: failed
# mknod: 'tty-': Permission denied
# makedev tty c 5 0 root tty 0666: failed
# mknod: 'tty0-': Permission denied
# makedev tty0 c 4 0 root tty 0600: failed
# mknod: 'console-': Permission denied
# makedev console c 5 1 root tty 0600: failed
# chmod: cannot access '/dev/tty[0-9]*': No such file or directory
# dpkg: error processing package makedev (--configure):
#  subprocess installed post-installation script returned error exit status 1
# Setting up initscripts ...
# rm: cannot remove '/dev/shm': Permission denied
# dpkg: error processing package initscripts (--configure):
#  subprocess installed post-installation script returned error exit status 1
amd64-ubuntu-trusty-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|/var/lib/dpkg/info/(initscripts|makedev).postinst
# Setting up makedev ...
# mknod: null-: Permission denied
# makedev null c 1 3 root root 0666: failed
# mknod: zero-: Permission denied
# makedev zero c 1 5 root root 0666: failed
# mknod: full-: Permission denied
# makedev full c 1 7 root root 0666: failed
# mknod: random-: Permission denied
# makedev random c 1 8 root root 0666: failed
# mknod: urandom-: Permission denied
# makedev urandom c 1 9 root root 0666: failed
# mknod: tty-: Permission denied
# makedev tty c 5 0 root tty 0666: failed
# mknod: tty0-: Permission denied
# makedev tty0 c 4 0 root tty 0600: failed
# mknod: console-: Permission denied
# makedev console c 5 1 root tty 0600: failed
# chmod: cannot access '/dev/tty[0-9]*': No such file or directory
# dpkg: error processing package makedev (--configure):
#  subprocess installed post-installation script returned error exit status 1
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--configure):
#  subprocess installed post-installation script returned error exit status 1
amd64-ubuntu-xenial-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn|/var/lib/dpkg/info/makedev.postinst
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Network Management systemd-network' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
#  installed systemd package post-installation script subprocess returned error exit status 1
amd64-ubuntu-bionic-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn

extra-rootfs: amd64-devuan-rootfs

.PHONY: amd64-devuan-rootfs
amd64-devuan-rootfs: amd64-devuan-beowulf-rootfs
amd64-devuan-rootfs: amd64-devuan-chimaera-rootfs
amd64-devuan-rootfs: amd64-devuan-daedalus-rootfs

stable-rootfs: amd64-devuan-daedalus-rootfs

amd64-devuan-jessie-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.devuan.org/merged/
amd64-devuan-jessie-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= support/ceres
amd64-devuan-ascii-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.devuan.org/merged/
amd64-devuan-ascii-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= support/ceres
amd64-devuan-beowulf-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://deb.devuan.org/merged/
amd64-devuan-beowulf-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= support/ceres
amd64-devuan-chimaera-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://deb.devuan.org/merged/
amd64-devuan-chimaera-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= support/ceres
amd64-devuan-daedalus-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://deb.devuan.org/merged/
amd64-devuan-daedalus-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= support/ceres
amd64-devuan-excalibur-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://deb.devuan.org/merged/
amd64-devuan-excalibur-rootfs/bin/sh: export DEBOOTSTRAP_SCRIPT ?= support/ceres
amd64-devuan-excalibur-rootfs/bin/sh: export DEBOOTSTRAPFLAGS ?= --keyring support/devuan-keyring-excalibur-archive.gpg
$(eval $(call debootstrap-rootfs,amd64,devuan,jessie))
$(eval $(call debootstrap-rootfs,amd64,devuan,ascii))
$(eval $(call debootstrap-rootfs,amd64,devuan,beowulf))
$(eval $(call debootstrap-rootfs,amd64,devuan,chimaera))
$(eval $(call debootstrap-rootfs,amd64,devuan,daedalus))
$(eval $(call debootstrap-rootfs,amd64,devuan,excalibur))

legacy-support: support/amd64-devuan-jessie-rootfs.txt
legacy-support: support/amd64-devuan-ascii-rootfs.txt
legacy-support: support/amd64-devuan-beowulf-rootfs.txt
legacy-support: support/amd64-devuan-chimaera-rootfs.txt
legacy-log: amd64-devuan-jessie-rootfs.log
legacy-log: amd64-devuan-ascii-rootfs.log
legacy-log: amd64-devuan-beowulf-rootfs.log
legacy-log: amd64-devuan-chimaera-rootfs.log

stable-support: support/amd64-devuan-daedalus-rootfs.txt
stable-log: amd64-devuan-daedalus-rootfs.log

unstable-support: support/amd64-devuan-excalibur-rootfs.txt
unstable-log: amd64-devuan-excalibur-rootfs.log

install-support-x86_64: install-support-x86_64-debootstrap

.PHONY: install-support-x86_64-debootstrap
install-support-x86_64-debootstrap:
	install -D -m644 support/ceres $(DESTDIR)$(PREFIX)/share/iamroot/debootstrap/ceres
endif

ifneq ($(shell command -v mmdebstrap 2>/dev/null),)
amd64-mobian-bookworm-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?= --hook-dir=/usr/share/mmdebstrap/hooks/maybe-merged-usr --variant=apt --include=mobian-base
amd64-mobian-trixie-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?= --hook-dir=/usr/share/mmdebstrap/hooks/maybe-merged-usr --variant=apt --include=mobian-base
$(eval $(call mmdebstrap-rootfs,amd64,mobian,bookworm))
$(eval $(call mmdebstrap-rootfs,amd64,mobian,trixie))

extra-rootfs: amd64-mobian-rootfs

.PHONY: amd64-mobian-rootfs
amd64-mobian-rootfs: amd64-mobian-bookworm-rootfs
amd64-mobian-rootfs: amd64-mobian-trixie-rootfs

extra-support: support/amd64-mobian-bookworm-rootfs.txt
extra-log: amd64-mobian-bookworm-rootfs.log

unstable-support: support/amd64-mobian-trixie-rootfs.txt
unstable-log: amd64-mobian-trixie-rootfs.log

install-support-x86_64: install-support-x86_64-debootstrap

.PHONY: install-support-x86_64-mmdebstrap
install-support-x86_64-mmdebstrap:
	install -D -m644 support/mobian-bookworm-sources.list $(DESTDIR)$(PREFIX)/share/iamroot/mmdebstrap/
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
rootfs: x86_64-fedora-rootfs

.PHONY: x86_64-fedora-rootfs
fedora-rootfs: x86_64-fedora-20-rootfs
fedora-rootfs: x86_64-fedora-30-rootfs
fedora-rootfs: x86_64-fedora-31-rootfs
fedora-rootfs: x86_64-fedora-32-rootfs
fedora-rootfs: x86_64-fedora-33-rootfs
fedora-rootfs: x86_64-fedora-34-rootfs
fedora-rootfs: x86_64-fedora-35-rootfs
fedora-rootfs: x86_64-fedora-36-rootfs
fedora-rootfs: x86_64-fedora-37-rootfs
fedora-rootfs: x86_64-fedora-38-rootfs
fedora-rootfs: x86_64-fedora-39-rootfs
fedora-rootfs: x86_64-fedora-40-rootfs
fedora-rootfs: x86_64-fedora-rawhide-rootfs

stable-rootfs: x86_64-fedora-40-rootfs
unstable-rootfs: x86_64-fedora-rawhide-rootfs

x86_64-fedora-20-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-30-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-31-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-32-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-33-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-34-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-35-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-36-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-37-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-38-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
x86_64-fedora-rawhide-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-rawhide.repo
$(eval $(call dnf-rootfs,x86_64,fedora,20,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,30,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,31,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,32,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,33,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,34,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,35,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,36,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,37,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,38,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,39,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,40,Minimal Install))
$(eval $(call dnf-rootfs,x86_64,fedora,rawhide,Fedora Custom Operating System))

install-support-x86_64: install-support-x86_64-fedora

.PHONY: install-support-x86_64-fedora
install-support-x86_64-fedora:
	install -D -m644 support/fedora-archive.repo $(DESTDIR)$(PREFIX)/share/iamroot/fedora/fedora-archive.repo
	install -D -m644 support/fedora.repo $(DESTDIR)$(PREFIX)/share/iamroot/fedora/fedora.repo
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
extra-rootfs: opensuse-rootfs

.PHONY: opensuse-rootfs
opensuse-rootfs: x86_64-opensuse-tumbleweed-rootfs

stable-rootfs: x86_64-opensuse-tumbleweed-rootfs

$(eval $(call zypper-rootfs,x86_64,opensuse-leap))
$(eval $(call zypper-rootfs,x86_64,opensuse-tumbleweed))
x86_64-opensuse-leap-chroot x86_64-opensuse-leap-shell x86_64-opensuse-leap-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|/usr/bin/chkstat

install-support-x86_64: install-support-x86_64-zypper

.PHONY: install-support-x86_64-zypper
install-support-x86_64-zypper:
	install -D -m644 support/opensuse-leap-repo-oss.repo $(DESTDIR)$(PREFIX)/share/iamroot/opensuse/leap-repo-oss.repo
	install -D -m644 support/opensuse-tumbleweed-repo-oss.repo $(DESTDIR)$(PREFIX)/share/iamroot/opensuse/tumbleweed-repo-oss.repo

qemu-system-x86_64-opensuse-leap: override CMDLINE += rw init=/usr/lib/systemd/systemd
endif

ifneq ($(shell command -v xbps-install 2>/dev/null),)
$(eval $(call xbps-install-rootfs,x86_64,voidlinux))
$(eval $(call xbps-install-musl-rootfs,x86_64,voidlinux))

install-support-x86_64: install-support-x86_64-xbps

.PHONY: install-support-x86_64-xbps
install-support-x86_64-xbps:
	install -D -m644 support/x86_64-xbps.conf $(DESTDIR)$(PREFIX)/share/iamroot/voidlinux/xbps.conf
endif

$(eval $(call void-rootfs,x86_64,20240314))

$(eval $(call void-rootfs,i686,20240314))

ifneq ($(shell command -v musl-gcc 2>/dev/null)$(if musl,$(LIBC),YES,),)
.PHONY: alpine-test
alpine-test: | x86_64-alpine-mini-rootfs/usr/bin/shebang.sh
alpine-test: | x86_64-alpine-mini-rootfs/usr/bin/shebang-arg.sh
alpine-test: | x86_64-alpine-mini-rootfs/usr/bin/shebang-busybox.sh
alpine-test: ld-iamroot.so $(call libs,musl,x86_64) | x86_64-alpine-mini-rootfs
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs pwd                          | tee /dev/stderr | grep -q "^/\$$"
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs cat /etc/os-release          | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs chroot . cat /etc/os-release | tee /dev/stderr | grep 'NAME="Alpine Linux"'
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs /bin/busybox
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs shebang.sh one two three
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs shebang-arg.sh one two three
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs shebang-busybox.sh one two three
	ido $(IDOFLAGS) chroot x86_64-alpine-mini-rootfs /lib/ld-musl-x86_64.so.1 --preload "$$PWD/x86_64/libiamroot-musl-x86_64.so.1" bin/busybox

rootfs: alpinelinux-rootfs

$(eval $(call void-musl-rootfs,x86_64,20240314))

$(eval $(call adelie-mini-rootfs,x86_64,20240426))

$(eval $(call adelie-mini-rootfs,pmmx,20240426))

$(eval $(call alpine-mini-rootfs,x86_64,3.20))

$(eval $(call alpine-mini-rootfs,x86,3.20))

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
.PHONY: alpinelinux-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.14-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.15-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.16-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.17-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.18-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.19-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-3.20-rootfs
alpinelinux-rootfs: x86_64-alpinelinux-edge-rootfs

stable-rootfs: x86_64-alpinelinux-3.19-rootfs
stable-rootfs: x86_64-alpinelinux-3.20-rootfs
unstable-rootfs: x86_64-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86_64,alpinelinux,edge))

chroot-alpine-%: PATH = /usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin
chroot-alpine-%: SHELL = /bin/sh

.PRECIOUS: gcompat/ld-%
gcompat/ld-%: LOADER_NAME=ld-$*
gcompat/ld-%: | gcompat/Makefile
	$(MAKE) -C gcompat $(@F) LOADER_NAME=ld-$* CC=musl-gcc

.PRECIOUS: gcompat/libgcompat.so.0
gcompat/libgcompat.so.0: | gcompat/Makefile
	$(MAKE) -C gcompat libgcompat.so.0 CC=musl-gcc LDFLAGS+=-nostdlib

.PRECIOUS: gcompat/Makefile
gcompat/Makefile:
	git clone https://git.adelielinux.org/adelie/gcompat.git

.PRECIOUS: gcompat-i386/ld-%
gcompat-i386/ld-%: LOADER_NAME=ld-$*
gcompat-i386/ld-%: | gcompat-i386/Makefile
	$(MAKE) -C gcompat-i386 $(@F) LOADER_NAME=ld-$* CC=i386-musl-gcc

.PRECIOUS: gcompat-i386/libgcompat.so.0
gcompat-i386/libgcompat.so.0: | gcompat-i386/Makefile
	$(MAKE) -C gcompat-i386 libgcompat.so.0 CC=i386-musl-gcc LDFLAGS+=-nostdlib CFLAGS+=-fno-stack-protector

.PRECIOUS: gcompat-i386/Makefile
gcompat-i386/Makefile: | gcompat/Makefile
	git clone gcompat gcompat-i386

clean: clean-gcompat

.PHONY: clean-gcompat
clean-gcompat:
	-$(MAKE) -C gcompat clean
	rm -f gcompat/ld-*.so*
	-$(MAKE) -C gcompat-i386 clean
	rm -f gcompat-i386/ld-*.so*

ifneq ($(COVERAGE),0)
x86_64/libiamroot-musl-x86_64.so.1: | gcompat/libgcompat.so.0

i686/libiamroot-musl-i386.so.1: | gcompat-i386/libgcompat.so.0
endif

$(eval $(call alpine-make-rootfs-adelie-rootfs,x86_64,adelielinux,current))
x86_64-adelielinux-current-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --no-default-pkgs --packages adelie-core --repositories-file support/adelielinux-repositories --mirror-uri https://distfiles.adelielinux.org/adelie

ifneq ($(shell command -v i386-musl-gcc 2>/dev/null),)
i686-rootfs: x86-alpinelinux-rootfs

.PHONY: x86-alpinelinux-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.14-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.15-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.16-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.17-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.18-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.19-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-3.20-rootfs
x86-alpinelinux-rootfs: x86-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,x86,alpinelinux,edge))

$(eval $(call alpine-make-rootfs-adelie-rootfs,pmmx,adelielinux,current))
pmmx-adelielinux-current-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --no-default-pkgs --packages adelie-core --repositories-file support/adelielinux-repositories --mirror-uri https://distfiles.adelielinux.org/adelie
endif
endif
endif

ifneq ($(shell command -v pacstrap 2>/dev/null),)
ifneq ($(shell command -v aarch64-buildroot-linux-gnu-gcc 2>/dev/null),)
aarch64-rootfs: aarch64-archlinuxarm-rootfs

.PHONY: aarch64-archlinuxarm-rootfs
aarch64-archlinuxarm-rootfs: aarch64-archlinuxarm-rootfs/bin/sh

$(eval $(call pacstrap-rootfs,aarch64,archlinuxarm,base))
aarch64-archlinuxarm-chroot aarch64-archlinuxarm-shell aarch64-archlinuxarm-rootfs/bin/sh: IDOFLAGS += --libdir

install-support-aarch64: install-support-aarch64-pacman

.PHONY: install-support-aarch64-pacman
install-support-aarch64-pacman:
	install -D -m644 support/aarch64-archlinuxarm-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinuxarm/pacman.conf-aarch64
endif

ifneq ($(shell command -v arm-buildroot-linux-gnueabihf-gcc 2>/dev/null),)
arm-rootfs: armv7h-archlinuxarm-rootfs

.PHONY: armv7h-archlinuxarm-rootfs
armv7h-archlinuxarm-rootfs: armv7h-archlinuxarm-rootfs/bin/sh

$(eval $(call pacstrap-rootfs,armv7h,archlinuxarm,base))

install-support-armhf: install-support-armv7h-pacman

.PHONY: install-support-armv7h-pacman
install-support-armv7h-pacman:
	install -D -m644 support/armv7h-archlinuxarm-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinuxarm/pacman.conf-armv7h
endif

ifneq ($(shell command -v riscv64-buildroot-linux-gnu-gcc 2>/dev/null),)
riscv64-rootfs: riscv64-archlinuxriscv-rootfs

.PHONY: riscv64-archlinuxriscv-rootfs
riscv64-archlinuxriscv-rootfs: riscv64-archlinuxriscv-rootfs/bin/sh

$(eval $(call pacstrap-rootfs,riscv64,archlinuxriscv,base))
riscv64-archlinuxriscv-chroot riscv64-archlinuxriscv-shell riscv64-archlinuxriscv-rootfs/bin/sh: IDOFLAGS += --libdir

install-support-riscv64: install-support-riscv64-pacman

.PHONY: install-support-riscv64-pacman
install-support-riscv64-pacman:
	install -D -m644 support/riscv64-archlinuxriscv-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archlinuxrisc-v/pacman.conf-riscv64
endif

ifneq ($(shell command -v powerpc-buildroot-linux-gnu-gcc 2>/dev/null),)
powerpc-rootfs: powerpc-archlinuxpower-rootfs

.PHONY: powerpc-archlinuxpower-rootfs
powerpc-archlinuxpower-rootfs: powerpc-archlinuxpower-rootfs/bin/sh

$(eval $(call pacstrap-rootfs,powerpc,archlinuxpower,base))

install-support-powerpc: install-support-powerpc-pacman

.PHONY: install-support-powerpc-pacman
install-support-powerpc-pacman:
	install -D -m644 support/powerpc-archlinuxpower-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archpower/pacman.conf-powerpc
endif

ifneq ($(shell command -v powerpc64-buildroot-linux-gnu-gcc 2>/dev/null),)
powerpc64-rootfs: powerpc64-archlinuxpower-rootfs

.PHONY: powerpc64-archlinuxpower-rootfs
powerpc64-archlinuxpower-rootfs: powerpc64-archlinuxpower-rootfs/bin/sh

$(eval $(call pacstrap-rootfs,powerpc64,archlinuxpower,base))
powerpc64-archlinuxpower-chroot powerpc64-archlinuxpower-shell powerpc64-archlinuxpower-rootfs/bin/sh: export IDOFLAGS += --libdir

install-support-powerpc64: install-support-powerpc64-pacman

.PHONY: install-support-powerpc64-pacman
install-support-powerpc64-pacman:
	install -D -m644 support/powerpc64-archlinuxpower-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archpower/pacman.conf-powerpc64
	install -D -m644 support/toolchain_archlinuxpower_powerpc64_defconfig $(DESTDIR)$(PREFIX)/share/iamroot/buildroot/toolchain_archlinuxpower_powerpc64_defconfig
endif

ifneq ($(shell command -v powerpc64le-buildroot-linux-gnu-gcc 2>/dev/null),)
powerpc64-rootfs: powerpc64le-archlinuxpower-rootfs

.PHONY: powerpc64le-archlinuxpower-rootfs
powerpc64le-archlinuxpower-rootfs: powerpc64le-archlinuxpower-rootfs/bin/sh

$(eval $(call pacstrap-rootfs,powerpc64le,archlinuxpower,base))
powerpc64le-archlinuxpower-chroot powerpc64le-archlinuxpower-shell powerpc64le-archlinuxpower-rootfs/bin/sh: IDOFLAGS += --libdir

install-support-powerpc64le: install-support-powerpc64le-pacman

.PHONY: install-support-powerpc64le-pacman
install-support-powerpc64le-pacman:
	install -D -m644 support/powerpc64le-archlinuxpower-pacman.conf $(DESTDIR)$(PREFIX)/share/iamroot/archpower/pacman.conf-powerpc64le
endif
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
ifneq ($(shell command -v arm-buildroot-linux-gnueabi-gcc 2>/dev/null),)
arm-rootfs: armel-debian-rootfs

.PHONY: armel-debian-rootfs
armel-debian-rootfs: armel-debian-buster-rootfs
armel-debian-rootfs: armel-debian-bullseye-rootfs
armel-debian-rootfs: armel-debian-bookworm-rootfs
armel-debian-rootfs: armel-debian-trixie-rootfs
armel-debian-rootfs: armel-debian-sid-rootfs

armel-debian-buster-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,armel,debian,buster))
$(eval $(call debootstrap-rootfs,armel,debian,bullseye))
$(eval $(call debootstrap-rootfs,armel,debian,bookworm))
$(eval $(call debootstrap-rootfs,armel,debian,trixie))
$(eval $(call debootstrap-rootfs,armel,debian,sid))
armel-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif

ifneq ($(shell command -v arm-buildroot-linux-gnueabihf-gcc 2>/dev/null),)
arm-rootfs: armhf-debian-rootfs

.PHONY: armhf-debian-rootfs
armhf-debian-rootfs: armhf-debian-buster-rootfs
armhf-debian-rootfs: armhf-debian-bullseye-rootfs
armhf-debian-rootfs: armhf-debian-bookworm-rootfs
armhf-debian-rootfs: armhf-debian-trixie-rootfs
armhf-debian-rootfs: armhf-debian-sid-rootfs

$(eval $(call debootstrap-rootfs,armhf,debian,buster))
$(eval $(call debootstrap-rootfs,armhf,debian,bullseye))
$(eval $(call debootstrap-rootfs,armhf,debian,bookworm))
$(eval $(call debootstrap-rootfs,armhf,debian,trixie))
$(eval $(call debootstrap-rootfs,armhf,debian,sid))
armhf-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif

ifneq ($(shell command -v aarch64-buildroot-linux-gnu-gcc 2>/dev/null),)
arm-rootfs: arm64-debian-rootfs

.PHONY: arm64-debian-rootfs
arm64-debian-rootfs: arm64-debian-stretch-rootfs
arm64-debian-rootfs: arm64-debian-buster-rootfs
arm64-debian-rootfs: arm64-debian-bullseye-rootfs
arm64-debian-rootfs: arm64-debian-bookworm-rootfs
arm64-debian-rootfs: arm64-debian-trixie-rootfs
arm64-debian-rootfs: arm64-debian-sid-rootfs

arm64-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,arm64,debian,stretch))
$(eval $(call debootstrap-rootfs,arm64,debian,buster))
$(eval $(call debootstrap-rootfs,arm64,debian,bullseye))
$(eval $(call debootstrap-rootfs,arm64,debian,bookworm))
$(eval $(call debootstrap-rootfs,arm64,debian,trixie))
$(eval $(call debootstrap-rootfs,arm64,debian,sid))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
arm64-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
arm64-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
arm64-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif

ifneq ($(shell command -v riscv64-buildroot-linux-gnu-gcc 2>/dev/null),)
riscv64-rootfs: riscv64-debian-rootfs

.PHONY: riscv64-debian-rootfs
riscv64-debian-rootfs: riscv64-debian-buster-rootfs
riscv64-debian-rootfs: riscv64-debian-bullseye-rootfs
riscv64-debian-rootfs: riscv64-debian-bookworm-rootfs
riscv64-debian-rootfs: riscv64-debian-trixie-rootfs
riscv64-debian-rootfs: riscv64-debian-sid-rootfs

$(eval $(call debootstrap-rootfs,riscv64,debian,buster))
$(eval $(call debootstrap-rootfs,riscv64,debian,bullseye))
$(eval $(call debootstrap-rootfs,riscv64,debian,bookworm))
$(eval $(call debootstrap-rootfs,riscv64,debian,trixie))
$(eval $(call debootstrap-rootfs,riscv64,debian,sid))
riscv64-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif

ifneq ($(shell command -v mipsel-buildroot-linux-gnu-gcc 2>/dev/null),)
mips-rootfs: mipsel-debian-rootfs

.PHONY: mipsel-debian-rootfs
mipsel-debian-rootfs: mipsel-debian-stretch-rootfs
mipsel-debian-rootfs: mipsel-debian-buster-rootfs
mipsel-debian-rootfs: mipsel-debian-bullseye-rootfs
mipsel-debian-rootfs: mipsel-debian-bookworm-rootfs

mipsel-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,mipsel,debian,stretch))
$(eval $(call debootstrap-rootfs,mipsel,debian,buster))
$(eval $(call debootstrap-rootfs,mipsel,debian,bullseye))
$(eval $(call debootstrap-rootfs,mipsel,debian,bookworm))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
mipsel-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
mipsel-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
mipsel-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif

ifneq ($(shell command -v mips64el-buildroot-linux-gnu-gcc 2>/dev/null),)
mips64-rootfs: mips64el-debian-rootfs

.PHONY: mips64el-debian-rootfs
mips64el-debian-rootfs: mips64el-debian-stretch-rootfs
mips64el-debian-rootfs: mips64el-debian-buster-rootfs
mips64el-debian-rootfs: mips64el-debian-bullseye-rootfs
mips64el-debian-rootfs: mips64el-debian-bookworm-rootfs
mips64el-debian-rootfs: mips64el-debian-trixie-rootfs
mips64el-debian-rootfs: mips64el-debian-sid-rootfs

mips64el-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,mips64el,debian,stretch))
$(eval $(call debootstrap-rootfs,mips64el,debian,buster))
$(eval $(call debootstrap-rootfs,mips64el,debian,bullseye))
$(eval $(call debootstrap-rootfs,mips64el,debian,bookworm))
$(eval $(call debootstrap-rootfs,mips64el,debian,trixie))
$(eval $(call debootstrap-rootfs,mips64el,debian,sid))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
mips64el-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
mips64el-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
mips64el-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh

install-support-mips64le: install-support-mips64le-defconfig

.PHONY: install-support-mips64le-defconfig
install-support-mips64le-defconfig:
	install -D -m644 support/toolchain_debian_mips64el_defconfig $(DESTDIR)$(PREFIX)/share/iamroot/buildroot/toolchain_debian_mips64el_defconfig
endif

ifneq ($(shell command -v powerpc-buildroot-linux-gnu-gcc 2>/dev/null),)
powerpc-rootfs: powerpc-debian-rootfs

.PHONY: powerpc-debian-rootfs
powerpc-debian-rootfs: powerpc-debian-jessie-rootfs

powerpc-debian-jessie-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian/
$(eval $(call debootstrap-rootfs,powerpc,debian,jessie))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
powerpc-debian-jessie-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
powerpc-debian-jessie-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --no-check-gpg --include ssh
endif

ifneq ($(shell command -v powerpc64le-buildroot-linux-gnu-gcc 2>/dev/null),)
powerpc64-rootfs: ppc64el-debian-rootfs

.PHONY: ppc64el-debian-rootfs
ppc64el-debian-rootfs: ppc64el-debian-stretch-rootfs
ppc64el-debian-rootfs: ppc64el-debian-buster-rootfs
ppc64el-debian-rootfs: ppc64el-debian-bullseye-rootfs
ppc64el-debian-rootfs: ppc64el-debian-bookworm-rootfs
ppc64el-debian-rootfs: ppc64el-debian-trixie-rootfs
ppc64el-debian-rootfs: ppc64el-debian-sid-rootfs

ppc64el-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
ppc64el-debian-buster-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,ppc64el,debian,stretch))
$(eval $(call debootstrap-rootfs,ppc64el,debian,buster))
$(eval $(call debootstrap-rootfs,ppc64el,debian,bullseye))
$(eval $(call debootstrap-rootfs,ppc64el,debian,bookworm))
$(eval $(call debootstrap-rootfs,ppc64el,debian,trixie))
$(eval $(call debootstrap-rootfs,ppc64el,debian,sid))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
ppc64el-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
ppc64el-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
ppc64el-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif

ifneq ($(shell command -v s390x-buildroot-linux-gnu-gcc 2>/dev/null),)
s390x-rootfs: s390x-debian-rootfs

.PHONY: s390x-debian-rootfs
s390x-debian-rootfs: s390x-debian-stretch-rootfs
s390x-debian-rootfs: s390x-debian-buster-rootfs
s390x-debian-rootfs: s390x-debian-bullseye-rootfs
s390x-debian-rootfs: s390x-debian-bookworm-rootfs
s390x-debian-rootfs: s390x-debian-trixie-rootfs
s390x-debian-rootfs: s390x-debian-sid-rootfs

s390x-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
s390x-debian-buster-rootfs/bin/sh: export DEBOOTSTRAP_MIRROR ?= http://archive.debian.org/debian
$(eval $(call debootstrap-rootfs,s390x,debian,stretch))
$(eval $(call debootstrap-rootfs,s390x,debian,buster))
$(eval $(call debootstrap-rootfs,s390x,debian,bullseye))
$(eval $(call debootstrap-rootfs,s390x,debian,bookworm))
$(eval $(call debootstrap-rootfs,s390x,debian,trixie))
$(eval $(call debootstrap-rootfs,s390x,debian,sid))
# Setting up systemd ...
# chfn: PAM: System error
# adduser: `/usr/bin/chfn -f systemd Time Synchronization systemd-timesync' returned error code 1. Exiting.
# dpkg: error processing package systemd (--install):
# subprocess installed post-installation script returned error exit status 1
# Processing triggers for libc-bin ...
# Errors were encountered while processing:
#  systemd
s390x-debian-stretch-rootfs/bin/sh: export IAMROOT_EXEC_IGNORE = mountpoint|chfn
s390x-debian-stretch-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
s390x-debian-buster-rootfs/bin/sh: export DEBOOTSTRAPFLAGS += --include ssh
endif
endif

ifneq ($(shell command -v mmdebstrap 2>/dev/null),)
ifneq ($(shell command -v arm-buildroot-linux-gnueabihf-gcc 2>/dev/null),)
arm-rootfs: armhf-mobian-rootfs

.PHONY: armhf-mobian-rootfs
armhf-mobian-rootfs: armhf-mobian-bookworm-rootfs
armhf-mobian-rootfs: armhf-mobian-trixie-rootfs

armhf-mobian-bookworm-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?= --hook-dir=/usr/share/mmdebstrap/hooks/maybe-merged-usr
armhf-mobian-trixie-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?= --hook-dir=/usr/share/mmdebstrap/hooks/maybe-merged-usr
$(eval $(call mmdebstrap-rootfs,armhf,mobian,bookworm))
$(eval $(call mmdebstrap-rootfs,armhf,mobian,trixie))
endif

ifneq ($(shell command -v aarch64-buildroot-linux-gnu-gcc 2>/dev/null),)
arm-rootfs: arm64-mobian-rootfs

.PHONY: arm64-mobian-rootfs
arm64-mobian-rootfs: arm64-mobian-bookworm-rootfs
arm64-mobian-rootfs: arm64-mobian-trixie-rootfs

arm64-mobian-bookworm-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?= --hook-dir=/usr/share/mmdebstrap/hooks/maybe-merged-usr
arm64-mobian-trixie-rootfs/bin/sh: export MMDEBSTRAPFLAGS ?= --hook-dir=/usr/share/mmdebstrap/hooks/maybe-merged-usr
$(eval $(call mmdebstrap-rootfs,arm64,mobian,bookworm))
$(eval $(call mmdebstrap-rootfs,arm64,mobian,trixie))
endif
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
ifneq ($(shell command -v aarch64-buildroot-linux-gnu-gcc 2>/dev/null),)
aarch64-rootfs: aarch64-fedora-rootfs

.PHONY: aarch64-fedora-rootfs
aarch64-fedora-rootfs: aarch64-fedora-33-rootfs
aarch64-fedora-rootfs: aarch64-fedora-34-rootfs
aarch64-fedora-rootfs: aarch64-fedora-35-rootfs
aarch64-fedora-rootfs: aarch64-fedora-36-rootfs
aarch64-fedora-rootfs: aarch64-fedora-37-rootfs
aarch64-fedora-rootfs: aarch64-fedora-38-rootfs
aarch64-fedora-rootfs: aarch64-fedora-39-rootfs
aarch64-fedora-rootfs: aarch64-fedora-40-rootfs
aarch64-fedora-rootfs: aarch64-fedora-rawhide-rootfs

aarch64-fedora-33-rootfs: aarch64-fedora-33-rootfs/bin/sh
aarch64-fedora-34-rootfs: aarch64-fedora-34-rootfs/bin/sh
aarch64-fedora-35-rootfs: aarch64-fedora-35-rootfs/bin/sh
aarch64-fedora-36-rootfs: aarch64-fedora-36-rootfs/bin/sh
aarch64-fedora-37-rootfs: aarch64-fedora-37-rootfs/bin/sh
aarch64-fedora-38-rootfs: aarch64-fedora-38-rootfs/bin/sh
aarch64-fedora-39-rootfs: aarch64-fedora-39-rootfs/bin/sh
aarch64-fedora-40-rootfs: aarch64-fedora-40-rootfs/bin/sh
aarch64-fedora-rawhide-rootfs: aarch64-fedora-rawhide-rootfs/bin/sh

aarch64-fedora-33-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-34-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-35-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-36-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-37-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
aarch64-fedora-38-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
$(eval $(call dnf-rootfs,aarch64,fedora,33,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,34,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,35,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,36,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,37,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,38,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,39,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,40,Minimal Install))
$(eval $(call dnf-rootfs,aarch64,fedora,rawhide,Fedora Custom Operating System))
endif

ifneq ($(shell command -v arm-buildroot-linux-gnueabihf-gcc 2>/dev/null),)
arm-rootfs: armv7hl-fedora-rootfs

.PHONY: armv7hl-fedora-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-33-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-34-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-35-rootfs
armv7hl-fedora-rootfs: armv7hl-fedora-36-rootfs

armv7hl-fedora-33-rootfs: armv7hl-fedora-33-rootfs/bin/sh
armv7hl-fedora-34-rootfs: armv7hl-fedora-34-rootfs/bin/sh
armv7hl-fedora-35-rootfs: armv7hl-fedora-35-rootfs/bin/sh
armv7hl-fedora-36-rootfs: armv7hl-fedora-36-rootfs/bin/sh

armv7hl-fedora-33-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
armv7hl-fedora-34-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
armv7hl-fedora-35-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
armv7hl-fedora-36-rootfs/bin/sh: export FEDORA_REPO ?= support/fedora-archive.repo
$(eval $(call dnf-rootfs,armv7hl,fedora,33,Minimal Install))
$(eval $(call dnf-rootfs,armv7hl,fedora,34,Minimal Install))
$(eval $(call dnf-rootfs,armv7hl,fedora,35,Minimal Install))
$(eval $(call dnf-rootfs,armv7hl,fedora,36,Minimal Install))
endif
endif

ifneq ($(shell command -v xbps-install 2>/dev/null),)
ifneq ($(shell command -v aarch64-buildroot-linux-musl-gcc 2>/dev/null),)
$(eval $(call void-rootfs,aarch64,20240314))
endif

ifneq ($(shell command -v arm-buildroot-linux-gnueabihf-gcc 2>/dev/null),)
$(eval $(call void-rootfs,armv6l,20240314))
$(eval $(call void-rootfs,armv7l,20240314))
endif
endif

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
ifneq ($(shell command -v aarch64-buildroot-linux-musl-gcc 2>/dev/null),)
$(eval $(call adelie-mini-rootfs,aarch64,20240426))

aarch64-rootfs: aarch64-alpinelinux-rootfs

.PHONY: aarch64-alpinelinux-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.14-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.15-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.16-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.17-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.18-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.19-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-3.20-rootfs
aarch64-alpinelinux-rootfs: aarch64-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,aarch64,alpinelinux,edge))

$(eval $(call alpine-mini-rootfs,aarch64,3.20))

aarch64-rootfs: aarch64-adelielinux-rootfs

.PHONY: aarch64-adelielinux-rootfs
aarch64-adelielinux-rootfs: aarch64-adelielinux-current-rootfs

$(eval $(call alpine-make-rootfs-adelie-rootfs,aarch64,adelielinux,current))
aarch64-adelielinux-current-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --no-default-pkgs --packages adelie-core --repositories-file support/adelielinux-repositories --mirror-uri https://distfiles.adelielinux.org/adelie
endif

ifneq ($(shell command -v arm-buildroot-linux-musleabihf-gcc 2>/dev/null),)
$(eval $(call adelie-mini-rootfs,armv7,20240426))

arm-rootfs: armhf-alpinelinux-rootfs

.PHONY: armhf-alpinelinux-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.14-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.15-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.16-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.17-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.18-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.19-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-3.20-rootfs
armhf-alpinelinux-rootfs: armhf-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armhf,alpinelinux,edge))

$(eval $(call alpine-mini-rootfs,armhf,3.20))

.PHONY: armv7-alpinelinux-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.14-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.15-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.16-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.17-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.18-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.19-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-3.20-rootfs
armv7-alpinelinux-rootfs: armv7-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,armv7,alpinelinux,edge))

$(eval $(call alpine-mini-rootfs,armv7,3.20))

arm-rootfs: armv7-adelielinux-rootfs

.PHONY: armv7-adelielinux-rootfs
armv7-adelielinux-rootfs: armv7-adelielinux-current-rootfs

$(eval $(call alpine-make-rootfs-adelie-rootfs,armv7,adelielinux,current))
armv7-adelielinux-current-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --no-default-pkgs --packages adelie-core --repositories-file support/adelielinux-repositories --mirror-uri https://distfiles.adelielinux.org/adelie
endif

ifneq ($(shell command -v powerpc-buildroot-linux-musl-gcc 2>/dev/null),)
$(eval $(call adelie-mini-rootfs,ppc,20240426))

powerpc-rootfs: ppc-adelielinux-rootfs

.PHONY: ppc-adelielinux-rootfs
ppc-adelielinux-rootfs: ppc-adelielinux-current-rootfs

$(eval $(call alpine-make-rootfs-adelie-rootfs,ppc,adelielinux,current))
ppc-adelielinux-current-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --no-default-pkgs --packages adelie-core --repositories-file support/adelielinux-repositories --mirror-uri https://distfiles.adelielinux.org/adelie
endif

ifneq ($(shell command -v powerpc64le-buildroot-linux-musl-gcc 2>/dev/null),)
$(eval $(call adelie-mini-rootfs,ppc64,20240426))

powerpc64-rootfs: ppc64le-alpinelinux-rootfs

.PHONY: ppc64le-alpinelinux-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.14-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.15-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.16-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.17-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.18-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.19-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-3.20-rootfs
ppc64le-alpinelinux-rootfs: ppc64le-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,ppc64le,alpinelinux,edge))

$(eval $(call alpine-mini-rootfs,ppc64le,3.20))

powerpc64-rootfs: ppc64-adelielinux-rootfs

.PHONY: ppc64-adelielinux-rootfs
ppc64-adelielinux-rootfs: ppc64-adelielinux-current-rootfs

$(eval $(call alpine-make-rootfs-adelie-rootfs,ppc64,adelielinux,current))
ppc64-adelielinux-current-rootfs/bin/sh: export ALPINE_MAKE_ROOTFSFLAGS = --no-default-pkgs --packages adelie-core --repositories-file support/adelielinux-repositories --mirror-uri https://distfiles.adelielinux.org/adelie
endif

ifneq ($(shell command -v riscv64-buildroot-linux-musl-gcc 2>/dev/null),)
riscv64-rootfs: riscv64-alpinelinux-rootfs

.PHONY: riscv64-alpinelinux-rootfs
riscv64-alpinelinux-rootfs: riscv64-alpinelinux-3.20-rootfs
riscv64-alpinelinux-rootfs: riscv64-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,riscv64,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,riscv64,alpinelinux,edge))

$(eval $(call alpine-mini-rootfs,riscv64,3.20))
endif

ifneq ($(shell command -v s390x-buildroot-linux-musl-gcc 2>/dev/null),)
s390x-rootfs: s390x-alpinelinux-rootfs

.PHONY: s390x-alpinelinux-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.14-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.15-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.16-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.17-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.18-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.19-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-3.20-rootfs
s390x-alpinelinux-rootfs: s390x-alpinelinux-edge-rootfs

$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.14))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.15))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.16))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.17))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.18))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.19))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,3.20))
$(eval $(call alpine-make-rootfs-alpine-rootfs,s390x,alpinelinux,edge))

$(eval $(call alpine-mini-rootfs,s390x,3.20))
endif
endif

ifneq ($(KVER),)
.PRECIOUS: %-rootfs/lib/modules/$(KVER) %-rootfs/usr/lib/modules/$(KVER)
%-rootfs/lib/modules/$(KVER): | %-rootfs
	rm -Rf $@.tmp $@
	mkdir -p $@.tmp
	rsync -a --include '*/' --include '*.ko*' --exclude '*' /usr/lib/modules/$(KVER)/. $@.tmp/.
	find $@.tmp -name "*.zst" -exec unzstd -q --rm {} \;
	mv $@.tmp $@
endif

initrd-rootfs.cpio: initrd-rootfs/init
initrd-rootfs.cpio: initrd-rootfs/bin/sh
initrd-rootfs.cpio: initrd-rootfs/etc/passwd
initrd-rootfs.cpio: initrd-rootfs/etc/group
initrd-rootfs.cpio: initrd-rootfs/etc/mdev.conf
ifneq ($(KVER),)
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)/modules.dep
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)/modules.alias
initrd-rootfs.cpio: initrd-rootfs/lib/modules/$(KVER)/modules.symbols
endif

initrd-rootfs/init: tinird.sh | initrd-rootfs
	cp $< $@

initrd-rootfs/bin/sh: | initrd-rootfs/bin/busybox initrd-rootfs/bin
	ln -sf busybox $@

initrd-rootfs/bin/busybox: busybox-static | initrd-rootfs/bin
	cp $< $@

initrd-rootfs/etc/passwd: | initrd-rootfs/etc
	echo "root::0:0:root:/root:/bin/sh" >$@

initrd-rootfs/etc/group: | initrd-rootfs/etc
	echo "root:x:0:root" >$@

initrd-rootfs/etc/mdev.conf: | initrd-rootfs/etc
	echo '$$MODALIAS=.* root:root 660 @busybox modprobe "$$MODALIAS"' >$@

initrd-rootfs initrd-rootfs/bin initrd-rootfs/etc:
	mkdir -p $@

ifneq ($(KVER),)
initrd-rootfs/lib/modules/$(KVER)/modules.%: initrd-rootfs/bin/busybox | initrd-rootfs/lib/modules/$(KVER)
	initrd-rootfs/bin/busybox depmod -b initrd-rootfs $(KVER) $(F@)
endif

%.cpio:
	cd $* && find . | cpio -H newc -o -R root:root >$(CURDIR)/$@

.PHONY: static-chroot
static-chroot: x86_64/libiamroot-linux-x86-64.so.2 | static-rootfs
	ido $(IDOFLAGS) chroot static-rootfs /bin/sh

.PHONY: static-rootfs
static-rootfs: static-rootfs/usr/bin/sh
static-rootfs: static-rootfs/bin
static-rootfs: static-rootfs/root

static-rootfs/usr/bin/sh: PATH := $(CURDIR):$(PATH)
static-rootfs/usr/bin/sh: | busybox-static static-rootfs/usr/bin
	busybox-static --install $(@D)

static-rootfs/bin: | static-rootfs/usr/bin
	ln -sf usr/bin $@

static-rootfs/usr/bin static-rootfs/root:
	mkdir -p $@

busybox-static: busybox/busybox
	cp $< $@

.SILENT: busybox/busybox
busybox/busybox: busybox/.config
	$(MAKE) -C busybox CONFIG_STATIC=y CC=musl-gcc LD=musl-gcc

.SILENT: busybox/.config
busybox/.config: busybox/Makefile
	echo CONFIG_MODPROBE_SMALL=n >$@
	yes "" | $(MAKE) -C busybox oldconfig

.SILENT: busybox/Makefile
busybox/Makefile:
	wget -qO- https://www.busybox.net | \
	sed -n '/<li><b>.* -- BusyBox .* (stable)<\/b>/,/<\/li>/{/<p>/s,.*<a.*href="\(.*\)">BusyBox \(.*\)</a>.*,wget -qO- \1 | tar xvj \&\& ln -sf busybox-\2 busybox,p}' | \
	head -n 1 | \
	$(SHELL)

busybox_menuconfig:
busybox_%:
	$(MAKE) -C busybox $* CONFIG_STATIC=y CC=musl-gcc LD=musl-gcc

mnt:
	mkdir -p $@

.PHONY: support
support: all

.PHONY: legacy-support
legacy-support: all

.PHONY: stable-support
stable-support: all

.PHONY: unstable-support
unstable-support: all

.PHONY: extra-support
extra-support: all

.PHONY: broken-support
broken-support: all

.PHONY: fixme-support
fixme-support: all

.PHONY: log
log: all

.PHONY: stable-log
stable-log: all

.PHONY: unstable-log
unstable-log: all

.PHONY: extra-log
extra-log: all

.PHONY: broken-log
broken-log: all

.PHONY: fixme-log
fixme-log: all

ifneq ($(shell command -v pacstrap 2>/dev/null),)
support: archlinux-support

.PHONY: archlinux-support
archlinux-support: support/x86_64-archlinux-rootfs.txt
archlinux-support: support/i686-archlinux32-rootfs.txt

extra-support: manjaro-support

.PHONY: manjaro-support
manjaro-support: support/x86_64-manjaro-stable-rootfs.txt
manjaro-support: support/x86_64-manjaro-unstable-rootfs.txt

log: archlinux-log

.PHONY: archlinux-log
archlinux-log: x86_64-archlinux-rootfs.log
archlinux-log: i686-archlinux32-rootfs.log

extra-log: manjaro-log

.PHONY: manjaro-log
manjaro-log: x86_64-manjaro-stable-rootfs.log
manjaro-log: x86_64-manjaro-unstable-rootfs.log

stable-support: support/x86_64-archlinux-rootfs.txt
stable-support: support/i686-archlinux32-rootfs.txt
stable-support: support/x86_64-manjaro-stable-rootfs.txt
stable-log: x86_64-archlinux-rootfs.log
stable-log: i686-archlinux32-rootfs.log
stable-log: x86_64-manjaro-stable-rootfs.log

unstable-support: support/x86_64-manjaro-unstable-rootfs.txt
unstable-log: x86_64-manjaro-unstable-rootfs.log
endif

ifneq ($(shell command -v debootstrap 2>/dev/null),)
support: debian-support

.PHONY: debian-support
debian-support: support/amd64-debian-buster-rootfs.txt
debian-support: support/amd64-debian-bullseye-rootfs.txt
debian-support: support/amd64-debian-bookworm-rootfs.txt
debian-support: support/amd64-debian-trixie-rootfs.txt
debian-support: support/amd64-debian-sid-rootfs.txt

log: debian-log

.PHONY: debian-log
debian-log: amd64-debian-buster-rootfs.log
debian-log: amd64-debian-bullseye-rootfs.log
debian-log: amd64-debian-bookworm-rootfs.log
debian-log: amd64-debian-trixie-rootfs.log
debian-log: amd64-debian-sid-rootfs.log

support: ubuntu-support

.PHONY: ubuntu-support
ubuntu-support: support/amd64-ubuntu-trusty-rootfs.txt
ubuntu-support: support/amd64-ubuntu-xenial-rootfs.txt
ubuntu-support: support/amd64-ubuntu-bionic-rootfs.txt
ubuntu-support: support/amd64-ubuntu-focal-rootfs.txt
ubuntu-support: support/amd64-ubuntu-groovy-rootfs.txt
ubuntu-support: support/amd64-ubuntu-hirsute-rootfs.txt
ubuntu-support: support/amd64-ubuntu-impish-rootfs.txt
ubuntu-support: support/amd64-ubuntu-jammy-rootfs.txt
ubuntu-support: support/amd64-ubuntu-kinetic-rootfs.txt
ubuntu-support: support/amd64-ubuntu-lunar-rootfs.txt
ubuntu-support: support/amd64-ubuntu-mantic-rootfs.txt
ubuntu-support: support/amd64-ubuntu-noble-rootfs.txt

log: ubuntu-log

.PHONY: ubuntu-log
ubuntu-log: amd64-ubuntu-trusty-rootfs.log
ubuntu-log: amd64-ubuntu-xenial-rootfs.log
ubuntu-log: amd64-ubuntu-bionic-rootfs.log
ubuntu-log: amd64-ubuntu-focal-rootfs.log
ubuntu-log: amd64-ubuntu-groovy-rootfs.log
ubuntu-log: amd64-ubuntu-hirsute-rootfs.log
ubuntu-log: amd64-ubuntu-impish-rootfs.log
ubuntu-log: amd64-ubuntu-jammy-rootfs.log
ubuntu-log: amd64-ubuntu-kinetic-rootfs.log
ubuntu-log: amd64-ubuntu-lunar-rootfs.log
ubuntu-log: amd64-ubuntu-mantic-rootfs.log
ubuntu-log: amd64-ubuntu-noble-rootfs.log

stable-support: support/amd64-ubuntu-trusty-rootfs.txt
stable-support: support/amd64-ubuntu-xenial-rootfs.txt
stable-support: support/amd64-ubuntu-bionic-rootfs.txt
stable-support: support/amd64-ubuntu-focal-rootfs.txt
stable-support: support/amd64-ubuntu-jammy-rootfs.txt
stable-support: support/amd64-ubuntu-noble-rootfs.txt

stable-log: amd64-ubuntu-trusty-rootfs.log
stable-log: amd64-ubuntu-xenial-rootfs.log
stable-log: amd64-ubuntu-bionic-rootfs.log
stable-log: amd64-ubuntu-focal-rootfs.log
stable-log: amd64-ubuntu-jammy-rootfs.log
stable-log: amd64-ubuntu-noble-rootfs.log

legacy-support: support/amd64-ubuntu-groovy-rootfs.txt
legacy-support: support/amd64-ubuntu-hirsute-rootfs.txt
legacy-support: support/amd64-ubuntu-impish-rootfs.txt
legacy-support: support/amd64-ubuntu-kinetic-rootfs.txt
legacy-support: support/amd64-ubuntu-lunar-rootfs.txt
legacy-support: support/amd64-ubuntu-mantic-rootfs.txt
legacy-log: amd64-ubuntu-groovy-rootfs.log
legacy-log: amd64-ubuntu-hirsute-rootfs.log
legacy-log: amd64-ubuntu-impish-rootfs.log
legacy-log: amd64-ubuntu-kinetic-rootfs.log
legacy-log: amd64-ubuntu-lunar-rootfs.log
legacy-log: amd64-ubuntu-mantic-rootfs.log

extra-support: devuan-support

.PHONY: devuan-support
devuan-support: support/amd64-devuan-jessie-rootfs.txt
devuan-support: support/amd64-devuan-ascii-rootfs.txt
devuan-support: support/amd64-devuan-beowulf-rootfs.txt
devuan-support: support/amd64-devuan-chimaera-rootfs.txt
devuan-support: support/amd64-devuan-daedalus-rootfs.txt
devuan-support: support/amd64-devuan-excalibur-rootfs.txt

log: devuan-log

.PHONY: devuan-log
devuan-log: amd64-devuan-jessie-rootfs.log
devuan-log: amd64-devuan-ascii-rootfs.log
devuan-log: amd64-devuan-beowulf-rootfs.log
devuan-log: amd64-devuan-chimaera-rootfs.log
devuan-log: amd64-devuan-daedalus-rootfs.log
devuan-log: amd64-devuan-excalibur-rootfs.log

legacy-support: support/amd64-debian-stretch-rootfs.txt
legacy-support: support/amd64-debian-buster-rootfs.txt
legacy-support: support/amd64-debian-bullseye-rootfs.txt
legacy-log: amd64-debian-stretch-rootfs.log
legacy-log: amd64-debian-buster-rootfs.log
legacy-log: amd64-debian-bullseye-rootfs.log

oldstable-support: support/amd64-debian-stretch-rootfs.txt
oldstable-support: support/amd64-debian-buster-rootfs.txt
oldstable-support: support/amd64-debian-bullseye-rootfs.txt

stable-support: support/amd64-debian-bookworm-rootfs.txt
stable-log: amd64-debian-bookworm-rootfs.log

unstable-support: support/amd64-debian-sid-rootfs.txt
unstable-log: amd64-debian-sid-rootfs.log
endif

ifneq ($(shell command -v mmdebstrap 2>/dev/null),)
extra-support: mobian-support

.PHONY: mobian-support
mobian-support: support/amd64-mobian-bookworm-rootfs.txt
mobian-support: support/amd64-mobian-trixie-rootfs.txt

log: mobian-log

.PHONY: mobian-log
mobian-log: amd64-mobian-bookworm-rootfs.log
mobian-log: amd64-mobian-trixie-rootfs.log
endif

ifneq ($(shell command -v dnf 2>/dev/null),)
support: fedora-support

.PHONY: fedora-support
fedora-support: support/x86_64-fedora-20-rootfs.txt
fedora-support: support/x86_64-fedora-30-rootfs.txt
fedora-support: support/x86_64-fedora-31-rootfs.txt
fedora-support: support/x86_64-fedora-32-rootfs.txt
fedora-support: support/x86_64-fedora-33-rootfs.txt
fedora-support: support/x86_64-fedora-34-rootfs.txt
fedora-support: support/x86_64-fedora-35-rootfs.txt
fedora-support: support/x86_64-fedora-36-rootfs.txt
fedora-support: support/x86_64-fedora-37-rootfs.txt
fedora-support: support/x86_64-fedora-38-rootfs.txt
fedora-support: support/x86_64-fedora-39-rootfs.txt
fedora-support: support/x86_64-fedora-40-rootfs.txt
fedora-support: support/x86_64-fedora-rawhide-rootfs.txt

log: fedora-log

.PHONY: fedora-log
fedora-log: x86_64-fedora-20-rootfs.log
fedora-log: x86_64-fedora-30-rootfs.log
fedora-log: x86_64-fedora-31-rootfs.log
fedora-log: x86_64-fedora-32-rootfs.log
fedora-log: x86_64-fedora-33-rootfs.log
fedora-log: x86_64-fedora-34-rootfs.log
fedora-log: x86_64-fedora-35-rootfs.log
fedora-log: x86_64-fedora-36-rootfs.log
fedora-log: x86_64-fedora-37-rootfs.log
fedora-log: x86_64-fedora-38-rootfs.log
fedora-log: x86_64-fedora-39-rootfs.log
fedora-log: x86_64-fedora-40-rootfs.log
fedora-log: x86_64-fedora-rawhide-rootfs.log
endif

ifneq ($(shell command -v zypper 2>/dev/null),)
extra-support: opensuse-support

.PHONY: opensuse-support
broken-support: support/x86_64-opensuse-leap-rootfs.txt
opensuse-support: support/x86_64-opensuse-tumbleweed-rootfs.txt

extra-log: opensuse-log

.PHONY: opensuse-log
broken-log: x86_64-opensuse-leap-rootfs.log
opensuse-log: x86_64-opensuse-tumbleweed-rootfs.log
endif

ifneq ($(shell command -v xbps-install 2>/dev/null),)
extra-support: support/x86_64-voidlinux-rootfs.txt
extra-support: support/x86_64-voidlinux-musl-rootfs.txt

.PHONY: voidlinux-support
voidlinux-support: support/x86_64-voidlinux-rootfs.txt
voidlinux-support: support/x86_64-voidlinux-musl-rootfs.txt

extra-log: voidlinux-log

.PHONY: voidlinux-log
voidlinux-log: x86_64-voidlinux-rootfs.log
voidlinux-log: x86_64-voidlinux-musl-rootfs.log
endif

ifneq ($(shell command -v alpine-make-rootfs 2>/dev/null),)
support: alpinelinux-support

.PHONY: alpinelinux-support
alpinelinux-support: support/x86_64-alpinelinux-3.14-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-3.15-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-3.16-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-3.17-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-3.18-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-3.19-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-3.20-rootfs.txt
alpinelinux-support: support/x86_64-alpinelinux-edge-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.14-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.15-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.16-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.17-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.18-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.19-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-3.20-rootfs.txt
alpinelinux-support: support/x86-alpinelinux-edge-rootfs.txt

log: alpinelinux-log

.PHONY: alpinelinux-log
alpinelinux-log: x86_64-alpinelinux-3.14-rootfs.log
alpinelinux-log: x86_64-alpinelinux-3.15-rootfs.log
alpinelinux-log: x86_64-alpinelinux-3.16-rootfs.log
alpinelinux-log: x86_64-alpinelinux-3.17-rootfs.log
alpinelinux-log: x86_64-alpinelinux-3.18-rootfs.log
alpinelinux-log: x86_64-alpinelinux-3.19-rootfs.log
alpinelinux-log: x86_64-alpinelinux-3.20-rootfs.log
alpinelinux-log: x86_64-alpinelinux-edge-rootfs.log
alpinelinux-log: x86-alpinelinux-3.14-rootfs.log
alpinelinux-log: x86-alpinelinux-3.15-rootfs.log
alpinelinux-log: x86-alpinelinux-3.16-rootfs.log
alpinelinux-log: x86-alpinelinux-3.17-rootfs.log
alpinelinux-log: x86-alpinelinux-3.18-rootfs.log
alpinelinux-log: x86-alpinelinux-3.19-rootfs.log
alpinelinux-log: x86-alpinelinux-3.20-rootfs.log
alpinelinux-log: x86-alpinelinux-edge-rootfs.log

legacy-support: support/x86_64-alpinelinux-3.14-rootfs.txt
legacy-support: support/x86_64-alpinelinux-3.15-rootfs.txt
legacy-support: support/x86_64-alpinelinux-3.16-rootfs.txt
legacy-support: support/x86-alpinelinux-3.14-rootfs.txt
legacy-support: support/x86-alpinelinux-3.15-rootfs.txt
legacy-support: support/x86-alpinelinux-3.16-rootfs.txt
legacy-log: x86_64-alpinelinux-3.14-rootfs.log
legacy-log: x86_64-alpinelinux-3.15-rootfs.log
legacy-log: x86_64-alpinelinux-3.16-rootfs.log
legacy-log: x86-alpinelinux-3.14-rootfs.log
legacy-log: x86-alpinelinux-3.15-rootfs.log
legacy-log: x86-alpinelinux-3.16-rootfs.log

stable-support: support/x86_64-alpinelinux-3.17-rootfs.txt
stable-support: support/x86_64-alpinelinux-3.18-rootfs.txt
stable-support: support/x86_64-alpinelinux-3.19-rootfs.txt
stable-support: support/x86_64-alpinelinux-3.20-rootfs.txt
stable-support: support/x86-alpinelinux-3.17-rootfs.txt
stable-support: support/x86-alpinelinux-3.18-rootfs.txt
stable-support: support/x86-alpinelinux-3.19-rootfs.txt
stable-support: support/x86-alpinelinux-3.20-rootfs.txt
stable-log: x86_64-alpinelinux-3.17-rootfs.log
stable-log: x86_64-alpinelinux-3.18-rootfs.log
stable-log: x86_64-alpinelinux-3.19-rootfs.log
stable-log: x86_64-alpinelinux-3.20-rootfs.log
stable-log: x86-alpinelinux-3.17-rootfs.log
stable-log: x86-alpinelinux-3.18-rootfs.log
stable-log: x86-alpinelinux-3.19-rootfs.log
stable-log: x86-alpinelinux-3.20-rootfs.log

unstable-support: support/x86_64-alpinelinux-edge-rootfs.txt
unstable-support: support/x86-alpinelinux-edge-rootfs.txt
unstable-log: x86_64-alpinelinux-edge-rootfs.log
unstable-log: x86-alpinelinux-edge-rootfs.log

support: adelielinux-support

.PHONY: adelielinux-support
adelielinux-support: support/x86_64-adelielinux-current-rootfs.txt
adelielinux-support: support/pmmx-adelielinux-current-rootfs.txt

log: adelielinux-log

.PHONY: adelielinux-log
adelielinux-log: x86_64-adelielinux-current-rootfs.log
adelielinux-log: pmmx-adelielinux-current-rootfs.log

unstable-support: support/x86_64-adelielinux-current-rootfs.txt
unstable-support: support/pmmx-adelielinux-current-rootfs.txt
unstable-log: x86_64-adelielinux-current-rootfs.log
unstable-log: pmmx-adelielinux-current-rootfs.log
endif
endif

%:
	$(MAKE) -f Makefile $@
