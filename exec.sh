#!/bin/sh
#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

log() {
	if [[ "${IAMROOT_DEBUG:-0}" -lt 1 ]]
	then
		return
	fi

	echo "$@" >&2
}

case "${1##*/}" in
mount|umount|systemctl)
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
		if ! ldconfig="$(command -v ldconfig)"
		then
			echo "$1: No such file" >&2
			exit 1
		fi
		set -- "$IAMROOT_ROOT$ldconfig" "$@"
	fi

	iamroot_root="${IAMROOT_ROOT:-}"

	unset LD_PRELOAD
	unset LD_LIBRARY_PATH
	unset IAMROOT_ROOT

	# Fixes: $IAMROOT/usr/sbin/ldconfig: need absolute file name for configuration file when using -r
	IAMROOT_DEBUG= \
	IAMROOT_EXEC_DEBUG= \
	sed -e 's,include ld.so.conf.d/\*.conf,include /etc/ld.so.conf.d/*.conf,' \
	    -i "$iamroot_root/etc/ld.so.conf"
	unset iamroot_root

	exec "$@"
	;;
passwd|su)
	echo "Error:" "Command not handled:" "$@" >&2
	exit 1
	;;
bbsuid)
	iamroot_root="${IAMROOT_ROOT:-}"

	unset LD_PRELOAD
	unset LD_LIBRARY_PATH
	unset IAMROOT_ROOT

	for i in /bin/mount /bin/umount /bin/su /usr/bin/crontab /usr/bin/passwd /usr/bin/traceroute /usr/bin/traceroute6 /usr/bin/vlock
	do
		ln -sf /bin/bbsuid "$iamroot$i"
	done
	unset iamroot_root
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
