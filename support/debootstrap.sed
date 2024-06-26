#!/bin/sed -f
#
# Copyright 2022-2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

1,$ {
	# I: Valid Release signature (key id XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX)
	/^I: Valid Release signature\s/ {
		/key\s/s,[0-9A-F]\{40\,40\},XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,
	}

	# I: Retrieving base-files
	# I: Validating base-files 12.2
	/^I:\s\(Retrieving\|Validating\)\s/ {
		s,\([[:lower:][:digit:]+.-]\+\)\s\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)$,\1,
	}

	# I: Unpacking libc6:amd64...
	# I: Configuring libc6:amd64...
	/^I:\s\(Unpacking\|Configuring\)\s/ {
		s,\([[:lower:][:digit:]+.-]\+\):\([[:lower:][:digit:]]\+\)\.\.\.,\1...,
	}

	# http://deb.debian.org/debian/dists/unstable/main/binary-amd64/by-hash/SHA256/646f59e179f69a5d9742b7aca0a1907d3b19c9156084daa464ed41d215f98ee8:
	/^http.*:$/ {
		d
	}

	# YYYY-MM-DD hh:mm:ss ERROR 404: Not Found.
	# YYYY-MM-DD hh:mm:ss URL:http://deb.debian.org/debian/pool/main/b/base-files/base-files_12.2_am
	/^[[:digit:]]\{4,4\}-[[:digit:]]\{2,2\}-[[:digit:]]\{2,2\}\s[[:digit:]]\{2,2\}:[[:digit:]]\{2,2\}:[[:digit:]]\{2,2\}\s\(URL\|ERROR\)/ {
		d
	}

	# Unpacking base-files (12.2) ...
	# Unpacking base-files (12.2) over (12.2) ...
	# Setting up base-files (12.2) ...
	# Unpacking libc6:amd64 (2.34-3) ...
	# Selecting previously unselected package libc6:amd64.
	# Processing triggers for libc-bin (2.33-7) ...
	/^\(Unpacking\|Selecting\|Setting up\|Processing triggers for\)/ {
		s,(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\))\sover\s,,
		s,\([[:lower:][:digit:]+.-]\+\)\s(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\))\s\.\.\.,\1 ...,
		s,\([[:lower:][:digit:]+.-]\+\):\([[:lower:][:digit:]]\+\),\1,
	}

	# Preparing to unpack .../base-files_12.2_amd64.deb ...
	/^Preparing to unpack/ {
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb\s\.\.\.,\1.deb ...,
	}

	# dpkg: regarding .../archives/dpkg_1.21.1_amd64.deb containing dpkg, pre-dependency problem:
	#  dpkg pre-depends on libc6 (>= 2.15)
	#   libc6 is not installed.
	# dpkg: libc6:amd64: dependency problems, but configuring anyway as you requested:
	#  libc6:amd64 depends on libgcc-s1; however:
	#   Package libgcc-s1 is not installed.
	/^dpkg: /,/^$/ {
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb,\1.deb,
		s,\([[:lower:][:digit:]+.-]\+\)\s(>\?=\s\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)),\1,
		s,\([[:lower:][:digit:]+.-]\+\):\([[:lower:][:digit:]]\+\),\1,
	}

	# Local time is now:      Day Mon dd hh:mm:ss TZ YYYY.
	# Universal Time is now:  Day Mon dd hh:mm:ss TZ YYYY.
        /^\(Local time\|Universal Time\) is now: / {
		s,\(:\s\+\).*$,\1Day Mon dd hh:mm:ss TZ YYYY.,
	}

	# (Reading database ... XXXX files and directories currently installed.)
	/^(Reading database \.\.\./ {
		s,[[:digit:]]\+,XXX,g
	}

	# dpkg: warning: parsing file '/var/lib/dpkg/status' near line X package 'dpkg':
	s,line \([[:digit:]]\+\) ,line X: ,

	# /var/lib/dpkg/info/libc6:amd64.postinst: 1: which: not found
	/^\/var\/lib\/dpkg\/info\/.*.postinst: /{
		s,\([[:lower:][:digit:]+.-]\+\):\([[:lower:][:digit:]]\+\),\1,
	}

	# amd64: ok
	/^[[:lower:][:digit:]]\+: ok$/d

	# Creating SSH2 RSA key; this may take some time ...
	# 2048 SHA256:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX root@XXX (RSA)
	# Creating SSH2 ECDSA key; this may take some time ...
	# 256 SHA256:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX root@XXX (ECDSA)
	# Creating SSH2 ED25519 key; this may take some time ...
	# 256 SHA256:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX root@XXX (ED25519)
	/^\(2048\|256\) SHA256/ {
		s,SHA256:.\{43\,43\} \(.\+\)@.\+ (\(.\+\),SHA256:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX \1@XXX (\2),
	}

        /^\(Warning\|Debug\): /,/^$/ {
		# rm -f /tmp/tmp.XXXXXXXXXX
		s,/tmp\.[[:alnum:]]\{10\,10\},/tmp.XXXXXXXXXX,g

		# Day Mon dd hh:mm:ss TZ YYYY
		s,[[:alpha:]]\{3\,3\}[[:blank:]][[:alpha:]]\{3\,3\}[[:blank:]][[:digit:]]\{2\,2\}[[:blank:]][[:digit:]]\{2\,2\}:[[:digit:]]\{2\,2\}:[[:digit:]]\{2\,2\}[[:blank:]][[:alnum:]]\+[[:blank:]][[:digit:]]\{4\,4\},Day Mon dd hh:mm:ss TZ YYYY,g

		# (...) DPKG_RUNNING_VERSION=X (...)
		s,DPKG_RUNNING_VERSION=[[:alnum:].]\+,DPKG_RUNNING_VERSION=X,g
	}
}

# Creating group 'bin' with GID XXX.
# Creating user 'bin' (n/a) with UID XXX and GID XXX.
/^Creating \(user\|group\) '.*'/ {
	s,UID [[:digit:]]\+,UID XXX,
	s,GID [[:digit:]]\+,GID XXX,
}

# Adding group `crontab' (GID XXX) ...
/^Adding group `.\+'/ {
	s,GID [[:digit:]]\+,GID XXX,
}

/^gpgv:\s/ {
	# gpgv: Signature made Day Mon dd hh:mm:ss YYYY TZ
	# gpgv:                using RSA key XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	# gpgv:                using RSA key XXXXXXXXXXXXXXXX
	s,\(Signature made\s\+\).*$,\1Day Mon dd hh:mm:ss YYYY TZ,
	/key\s/s,[0-9A-F]\{40\,40\},XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,
	/key\s/s,[0-9A-F]\{16\,16\},XXXXXXXXXXXXXXXX,
}
