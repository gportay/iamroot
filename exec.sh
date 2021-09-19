#!/bin/bash
#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: GPL-2.1
#

log() {
	if [[ "${IAMROOT_DEBUG:-0}" -lt 1 ]]
	then
		return
	fi

	echo "$@" >&2
}

case "${1##*/}" in
mount|umount)
	log "Warning:" "Command is skipped:" "$@"
	;;
ldd)
	unset LD_PRELOAD
	unset LD_LIBRARY_PATH
	unset IAMROOT_ROOT

	shift
	set -- "$IAMROOT_ROOT/usr/bin/ldd" "$@"

	exec "$@"
	;;
ldconfig)
	if [[ "${IAMROOT_ROOT:-/}" != / ]]
	then
		set -- "$@" -r "$IAMROOT_ROOT"

		shift
		set -- "$IAMROOT_ROOT/usr/sbin/ldconfig" "$@"
	fi

	unset LD_PRELOAD
	unset IAMROOT_ROOT

	exec "$@"
	;;
passwd|su)
	echo "Error:" "Command not handled:" "$@" >&2
	exit 1
	;;
busybox)
	if [[ "${IAMROOT_ROOT:-/}" != / ]]
	then
		set -- "$@"

		shift
		set -- "$IAMROOT_ROOT/bin/busybox" "$@"
	fi

	unset LD_PRELOAD
	unset IAMROOT_ROOT

	exec "$@"
	;;
*)
	echo "Warning:" "Command not handled:" "$@" >&2
	;;
esac
