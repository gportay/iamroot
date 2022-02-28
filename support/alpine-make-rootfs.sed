#!/usr/bin/sed -f
#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

/\x1b\[1;36m> Installing system/,/^\x1b\[1:36m> / {
	# [1;36m> Installing system [0m
	s,\x1b\[[[:digit:];]*m,,g

	# fetch http://nl.alpinelinux.org/alpine/edge/main/x86_64/APKINDEX.tar.gz
	/^fetch /d

	# (1/6) Installing musl (1.2.2-r7)
	/^([[:digit:]]\+\/[[:digit:]]\+)/ {
		s,^([[:digit:]]\+\/[[:digit:]]\+),(X/X),
		s,\([[:lower:][:digit:]_-]\+\) (\([[:alnum:]._+-]\+\)-\(r[[:digit:]]\+\))$,\1,
	}

	# Executing busybox-1.35.0-r2.post-install
	s,\([[:lower:][:digit:]_-]\+\)-\([[:alnum:]._+-]\+\)-\(r[[:digit:]]\+\),\1,

	# OK: X MiB in X packages
	/^OK:/ {
                s,[[:digit:].]\+,X,g
                s,.iB,XiB,
        }
}
