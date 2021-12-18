#!/bin/sh
#
# Copyright 2021 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

log() {
	if [ "${IAMROOT_DEBUG:-0}" -lt 1 ]
	then
		return
	fi

	echo "$@" >&2
}

path="$1"
argv0="${argv0:-$path}"
case "${1##*/}" in
mount|umount|systemctl)
	shift
	log "Warning:" "Command is skipped:" "$argv0" "$@"
	;;
ldd)
	shift
	set -- "$IAMROOT_ROOT$path" "$@"

	exec "$@"
	;;
ldconfig)
	shift
	set -- "$IAMROOT_ROOT$path" "$@"

	if [ "${IAMROOT_ROOT:-/}" != / ]
	then
		set -- "$@" -r "$IAMROOT_ROOT"
	fi

	# Fixes: $IAMROOT/usr/sbin/ldconfig: need absolute file name for configuration file when using -r
	sed -e 's,include ld.so.conf.d/\*.conf,include /etc/ld.so.conf.d/*.conf,' \
	    -i "$IAMROOT_ROOT/etc/ld.so.conf"

	exec "$@"
	;;
passwd|su)
	shift
	echo "Error:" "Command not handled:" "$argv0" "$@" >&2
	exit 1
	;;
bbsuid)
	for i in /bin/mount /bin/umount /bin/su /usr/bin/crontab /usr/bin/passwd /usr/bin/traceroute /usr/bin/traceroute6 /usr/bin/vlock
	do
		ln -sf /bin/bbsuid "$IAMROOT_ROOT$i"
	done
	;;
busybox)
	if [ "${IAMROOT_ROOT:-/}" != / ]
	then
		shift
		set -- "$IAMROOT_ROOT$path" "$@"
	fi

	exec "$@"
	;;
*)
	echo "Warning:" "Command not handled:" "$@" >&2
	;;
esac
