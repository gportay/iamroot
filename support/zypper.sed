#!/usr/bin/sed -f
#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

# The following XX recommended packages were automatically selected:
# The following XXX NEW packages are going to be installed:
# XXX new packages to install.
# Warning: XXX packages had to be excluded from file conflicts check because they are not yet downloaded.
/[[:digit:]]\+ \(recommended \|new \|NEW \|\)packages/ {
	s,[[:digit:]]\+,XXX,g
}

# Overall download size: XX.X MiB. Already cached: 0 B. After the operation, additional XXX.X  MiB will be used.
/Overall download size:/{
	s,[[:digit:] .]\{5\,6\}\s\([MK]i\|  \)B, XXX.X \1B,g
}

# Adding repository 'openSUSE-Tumbleweed-Oss' [......done]
# Retrieving repository 'openSUSE-Tumbleweed-Oss' metadata [..done]
# Building repository 'openSUSE-Tumbleweed-Oss' cache [....done]
# Checking for file conflicts: [.......................done]
# ( XX/XXX) Installing: filesystem-84.87-3.2.x86_64 [............done]
/[.*done.*]/ {
	s,\[\.\+done.*\],[done],
}

# Retrieving package filesystem-84.87-3.2.x86_64 (X/XXX),  X.X KiB (  XXX   B unpacked)
/^Retrieving package/{
	s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^+-]\+\|[[:alnum:]._~^+-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
	s,([[:digit:]]\+\/[[:digit:]]\+),(XXX/XXX),
	s,[[:digit:] .]\{5\,6\}\s\([MK]i\|  \)B, XXX.X \1B,g
}

# Retrieving: filesystem-84.87-3.2.x86_64.rpm [done]
/^Retrieving:/{
	s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^+-]\+\|[[:alnum:]._~^+-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
}

# ( XX/XXX) Installing: filesystem-84.87-3.2.x86_64 [............done]
/^([[:digit:] ]\+\/[[:digit:] ]\+) Installing:/{
	s,^([[:digit:] ]\+\/[[:digit:] ]\+),(XXX/XXX),
	s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^+-]\+\|[[:alnum:]._~^+-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
}

/^Additional rpm output/,/^$:/ {
	# /var/tmp/rpm-tmp.XXXXXX: line X: file: strerror
	/^\/var\/tmp\/rpm-tmp\./ {
		s,/rpm-tmp\.[[:alnum:]]\{6\,6\},/rpm-tmp.XXXXXX,
		s,line \([[:digit:]]\+\): ,line X: ,
	}

	# warning: opensuse-tumbleweed-rootfs/var/cache/zypp/packages/repo-oss/x86_64/filesystem-84.87-3.2.x86_64.rpm: Header V3 RSA/SHA256 Signature, key ID 3dbdc284: NOKEY
	/^warning:/{
		s,\([[:lower:][:upper:][:digit:]._+-]\+\)-\([[:digit:]]\+:[[:alnum:]._~^+-]\+\|[[:alnum:]._~^+-]\+\)-\([[:alnum:].%{?}-]\+\)\.\([[:lower:][:digit:]]\+\.[[:alnum:]_]\+\|[[:alnum:]_]\+\),\1,
	}
}
