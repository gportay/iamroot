#!/usr/bin/sed -f
#
# Copyright 2022 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

1,$ {
	# I: Retrieving base-files
	# I: Validating base-files 12.2
	/^I:\s\(Retrieving\|Validating\)\s/ {
		s,\([[:lower:][:digit:]+.-]\+\)\s\([[:digit:]]\+:[[:alnum:].+~-]\+\|[[:alnum:].+~-]\+\)$,\1,
	}
}
