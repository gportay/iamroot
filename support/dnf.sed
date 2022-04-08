#!/usr/bin/sed -f
#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

1,/^Dependencies resolved\./ {
	# Fedora 35 - x86_64                               XX MB/s |  XX MB     XX:XX
	/^.\+\s-\s\S\+/s,^\(.\+\s-\s\S\+\)\s\+\S\+\s.B/s\s|\s\+\S\{1\,3\}\s.B\s\+\S\{2\,2\}:\S\{2\,2\}\s\+$,\1 XXX xB/s | XXX xB XX:XX,
	# Last metadata expiration check: H:mm:ss ago on Day XX Mon YYYY HH:mm:ss xM TZ.
	/^Last metadata expiration check: /s,:\s.*,: HH:mm:ss on Day XX Mon YYYY HH:mm:ss xM TZ.,
}

/^Dependencies resolved\./,/^Downloading Packages:/ {
	# ======================================================================================
	#  Package                         Arch    Version                         Repo     Size
	# ======================================================================================
	/^\s\+Package\(\s\+\S\+\)\{4,4\}/s,\s\+, ,g
	/^=\+/s,=\+,===============================,

	#  acl                             x86_64  2.3.1-2.fc35                    fedora   XX k
	/^\(\s\+\S\+\)\{6,6\}/s,^\(\s\+\S\+\).*,\1,

	# Install  XXX Packages
	# Total download size: XXX M
	# Installed size: XXX M
	/[[:digit:]]\+\s\(Packages\|M\)$/s,[[:digit:]]\+\s\(Packages\|M\),XXX \1,

	# Installing Environment Groups:
	#  Minimal Install
	# Installing Groups:
	#  Core
	# (trailing spaces)
	s,\s\+$,,
}

/^Downloading Packages:/,/^Running transaction check/{
	# (XXX/XXX): acl-2.3.1-2.fc35.x86_64.rpm          X.X MB/s |  XX kB     XX:XX
	/^(\S\+\/\S\+): /d

	# --------------------------------------------------------------------------------
	/^-\+/s,-\+,----------------------------------,

	# Total                                           XXX MB/s | XXX MB     XX:XX
	/^Total/s,\s\+\S\+\s.B/s\s|\s\+\S\{1\,3\}\s.B\s\+\S\{2\,2\}:\S\{2\,2\}\s\+$, XXX xB/s | XXX xB XX:XX,
}

/^Running transaction/,/^Installed:/ {
	#   Running scriptlet: filesystem-3.14-7.fc35.x86_64                      XXX/XXX
	#   Installing       : filesystem-3.14-7.fc35.x86_64                      XXX/XXX
	#   Verifying        : filesystem-3.14-7.fc35.x86_64                      XXX/XXX
	/^\s\+\(\S\+\s*\)\{1,2\}: / {
		s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^-]\+\|[[:alnum:]._~^-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
		s,\s\+[[:digit:]]\+\/[[:digit:]]\+\s\+$, XXX/XXX,
	}

	# /var/tmp/rpm-tmp.XXXXXX: line X: file: command not found
	/^\/var\/tmp\/rpm-tmp\./ {
		s,/rpm-tmp\.[[:alnum:]]\{6\,6\},/rpm-tmp.XXXXXX,
		s,line \([[:digit:]]\+\): ,line X: ,
	}
}

/^Installed:/,$ {
	#   filesystem-3.14-7.fc35.x86_64
	/^\s\s/ {
		s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^-]\+\|[[:alnum:]._~^-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
		s,\s\+$,,
	}
}
