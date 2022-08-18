#!/bin/sed -f
#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

1,$ {
	# I: Retrieving base-files
	# I: Validating base-files 12.2
	/^I:\s\(Retrieving\|Validating\)\s/ {
		s,\([[:lower:][:digit:]+.-]\+\)\s\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)$,\1,
	}

	# http://deb.debian.org/debian/dists/unstable/main/binary-amd64/by-hash/SHA256/646f59e179f69a5d9742b7aca0a1907d3b19c9156084daa464ed41d215f98ee8:
	/^http.*:$/{
		d
	}

	# YYYY-MM-DD hh:mm:ss ERROR 404: Not Found.
	# YYYY-MM-DD hh:mm:ss URL:http://deb.debian.org/debian/pool/main/b/base-files/base-files_12.2_am
	/^[[:digit:]]\{4,4\}-[[:digit:]]\{2,2\}-[[:digit:]]\{2,2\}\s[[:digit:]]\{2,2\}:[[:digit:]]\{2,2\}:[[:digit:]]\{2,2\}\s\(URL\|ERROR\)/{
		d
	}

	# Unpacking base-files (12.2) ...
	# Unpacking base-files (12.2) over (12.2) ...
	# Setting up base-files (12.2) ...
	# Processing triggers for libc-bin (2.33-7) ...
	/^\(Unpacking\|Setting up\|Processing triggers for\)/{
		s,(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\))\sover\s,,
		s,\([[:lower:][:digit:]+.-]\+\)\s(\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\))\s\.\.\.,\1 ...,
	}

	# Preparing to unpack .../base-files_12.2_amd64.deb ...
	/^Preparing to unpack/{
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb\s\.\.\.,\1 ...,
	}

	# dpkg: regarding .../archives/dpkg_1.21.1_amd64.deb containing dpkg, pre-dependency problem:
	#  dpkg pre-depends on libc6 (>= 2.15)
	#   libc6 is not installed.
	/^dpkg: /,/^$/{
		s,\([[:lower:][:digit:]+.-]\+\)_\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)_\([[:lower:][:digit:]-]\+\)\.deb,\1,
		s,\([[:lower:][:digit:]+.-]\+\)\s(>=\s\([[:digit:]]\+:[[:alnum:].+~%-]\+\|[[:alnum:].+~%-]\+\)),\1,
	}

	# Local time is now:      Day Mon dd hh:mm:ss TZ YYYY.
	# Universal Time is now:  Day Mon dd hh:mm:ss TZ YYYY.
        /^\(Local time\|Universal Time\) is now: /{
		s,\(:\s\+\).*$,\1Day Mon dd hh:mm:ss TZ YYYY.,
	}

	# (Reading database ... XXXX files and directories currently installed.)
	/^(Reading database \.\.\./{
		s,[[:digit:]]\+,XXX,
	}

	# dpkg: warning: parsing file '/var/lib/dpkg/status' near line X package 'dpkg':
	s,line \([[:digit:]]\+\) ,line X: ,
}

# Creating group 'bin' with GID 1.
# Creating user 'bin' (n/a) with UID 1 and GID 1.
/^Creating \(user\|group\) '.*'/{
	s,UID [[:digit:]]\+,UID XXX,
	s,GID [[:digit:]]\+,GID XXX,
}
