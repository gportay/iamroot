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

iamroot_root="${IAMROOT_ROOT:-}"

unset LD_PRELOAD
unset LD_LIBRARY_PATH
unset IAMROOT_ROOT

path="$1"
argv0="${argv0:-$path}"
case "${1##*/}" in
mount|umount|systemctl)
	shift
	log "Warning:" "Command is skipped:" "$argv0" "$@"
	;;
ldd)
	shift
	set -- "$iamroot_root/usr/bin/ldd" "$@"

	exec "$@"
	;;
ldconfig)
	if [ "${iamroot_root:-/}" != / ]
	then
		set -- "$@" -r "$iamroot_root"

		shift
		if ! ldconfig="$(command -v ldconfig)"
		then
			echo "$1: No such file" >&2
			exit 1
		fi
		set -- "$iamroot_root$ldconfig" "$@"
	fi

	# Fixes: $IAMROOT/usr/sbin/ldconfig: need absolute file name for configuration file when using -r
	sed -e 's,include ld.so.conf.d/\*.conf,include /etc/ld.so.conf.d/*.conf,' \
	    -i "$iamroot_root/etc/ld.so.conf"

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
		ln -sf /bin/bbsuid "$i"
	done
	;;
busybox)
	if [ "${iamroot_root:-/}" != / ]
	then
		shift
		set -- "$iamroot_root/bin/busybox" "$@"
	fi

	exec "$@"
	;;
*)
	echo "Warning:" "Command not handled:" "$@" >&2
	;;
esac
