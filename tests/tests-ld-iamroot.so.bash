#!/usr/bin/env bash
#
# Copyright 2024 Gaël PORTAY
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#

set -e
set -o pipefail

run() {
	lineno="${BASH_LINENO[0]}"
	test="$*"
	if [ -t 1 ]
	then
		echo -e "\e[1mRunning $test...\e[0m"
	else
		echo "Running $test..."
	fi
}

ok() {
	ok=$((ok+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[32m[OK]\e[0m"
	else
		echo "$test: [OK]"
	fi
}

ko() {
	ko=$((ko+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[31m[KO]\e[0m"
	else
		echo "$test: [KO]"
	fi
	if [ -t 2 ]
	then
		reports+=("$test at line \e[1m$lineno \e[31mhas failed\e[0m!")
	else
		reports+=("$test at line $lineno has failed!")
	fi
	if [[ $EXIT_ON_ERROR ]]
	then
		exit 1
	fi
}

fix() {
	fix=$((fix+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[34m[FIX]\e[0m"
	else
		echo "$test: [FIX]"
	fi
	if [ -t 2 ]
	then
		reports+=("$test at line \e[1m$lineno is \e[34mfixed\e[0m!")
	else
		reports+=("$test at line $lineno is fixed!")
	fi
}

bug() {
	bug=$((bug+1))
	if [ -t 1 ]
	then
		echo -e "\e[1m$test: \e[33m[BUG]\e[0m"
	else
		echo "$test: [BUG]"
	fi
	if [ -t 2 ]
	then
		reports+=("$test at line \e[1m$lineno is \e[33mbugged\e[0m!")
	else
		reports+=("$test at line $lineno is bugged!")
	fi
}

result() {
	exitcode="$?"
	trap - 0

	if [ -t 2 ]
	then
		echo -e "\e[1mTest report:\e[0m"
	else
		echo "Test report:"
	fi
	for report in "${reports[@]}"
	do
		if [ -t 2 ]
		then
			echo -e "$report" >&2
		else
			echo "$report" >&2
		fi
	done

	if [[ $ok ]]
	then
		if [ -t 1 ]
		then
			echo -e "\e[1m\e[32m$ok test(s) succeed!\e[0m"
		else
			echo "$ok test(s) succeed!"
		fi
	fi

	if [[ $fix ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1m\e[34m$fix test(s) fixed!\e[0m" >&2
		else
			echo "$fix test(s) fixed!" >&2
		fi
	fi

	if [[ $bug ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1mWarning: \e[33m$bug test(s) bug!\e[0m" >&2
		else
			echo "Warning: $bug test(s) bug!" >&2
		fi
	fi

	if [[ $ko ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1mError: \e[31m$ko test(s) failed!\e[0m" >&2
		else
			echo "Error: $ko test(s) failed!" >&2
		fi
	fi

	if [[ $exitcode -ne 0 ]] && [[ $ko ]]
	then
		if [ -t 2 ]
		then
			echo -e "\e[1;31mExited!\e[0m" >&2
		else
			echo "Exited!" >&2
		fi
	elif [[ $exitcode -eq 0 ]] && [[ $ko ]]
	then
		exit 1
	fi

	exit "$exitcode"
}

PATH="$IAMROOT_ORIGIN:$PWD:$PATH"
trap result 0 SIGINT

export -n NO_COLOR
export -n ISHLVL
export -n ISH_RC_FILE
export -n ISH_PROFILE_FILE
export -n IAMROOT_USER
export -n IAMROOT_ROOT
export -n IAMROOT_DEFLIB
export -n IAMROOT_EXEC
# Ignore: export -n IAMROOT_LIB
export -n IAMROOT_DEBUG
export -n IAMROOT_DEBUG_FD
export -n IAMROOT_DEBUG_IGNORE
export -n IAMROOT_EXEC_IGNORE
export -n IAMROOT_PATH_RESOLUTION_IGNORE

run "ld-iamroot.so: test without argument"
if ! ld-iamroot.so
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test with program argument"
if ld-iamroot.so /usr/bin/true
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test with program argument exiting with error fails"
if ! ld-iamroot.so /usr/bin/false
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -- with program argument and with program options"
if ld-iamroot.so -- /bin/sh -c "true"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test with program argument and with program options fails"
if ! ld-iamroot.so /bin/sh -c "true"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -A -sh sets argv[0]"
if ld-iamroot.so -A "-sh" -- /bin/sh -c 'echo "$0"' | grep "^-sh$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --argv0 nologin sets argv[0]"
if ld-iamroot.so --argv0 nologin -- /bin/sh -c 'echo "$0"' | grep "^nologin$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -P $PWD/libiamnotroot.so preloads the object $PWD/libiamnotroot.so"
if ld-iamroot.so -P "$PWD/libiamnotroot.so" env | grep "^LD_PRELOAD=$PWD/libiamnotroot.so"
then
	fix
else
	bug
fi
echo

run "ld-iamroot.so: test option --preload /dev/null preloads the object /dev/null"
if ld-iamroot.so --preload /dev/null env | grep "^LD_PRELOAD=/dev/null"
then
	fix
else
	bug
fi
echo

run "ld-iamroot.so: test option -L $IAMROOT_ORIGIN uses $IAMROOT_ORIGIN instead of LD_LIBRARY_PATH environment variable setting"
if LD_LIBRARY_PATH=/dev/null ld-iamroot.so -L "$IAMROOT_ORIGIN" env | grep "^LD_LIBRARY_PATH=$IAMROOT_ORIGIN$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --library-path $PWD/rootfs/lib:$PWD/rootfs/usr/lib uses $PWD/rootfs/lib:$PWD/rootfs/usr/lib instead of LD_LIBRARY_PATH environment variable setting"
if LD_LIBRARY_PATH=/dev/null ld-iamroot.so --library-path "$PWD/rootfs/lib:$PWD/rootfs/usr/lib" env | grep "^LD_LIBRARY_PATH=$PWD/rootfs/lib:$PWD/rootfs/usr/lib$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -M sets multiarch library path in chroot"
if ld-iamroot.so -M env | grep "^IAMROOT_MULTIARCH=1$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --multiarch sets multiarch library path in chroot"
if ld-iamroot.so --multiarch env | grep "^IAMROOT_MULTIARCH=1$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -R $PWD/rootfs changes root directory"
if ld-iamroot.so -R "$PWD/rootfs" env | grep "^IAMROOT_ROOT=$PWD/rootfs$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --chroot /dev/null fails to change root directory"
if ! ld-iamroot.so --root /dev/null env
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -C /tmp sets current working directory"
if ld-iamroot.so -C /tmp pwd | grep "^/tmp$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --cwd /dev/null fails to set current working directory"
if ! ld-iamroot.so --cwd /dev/null pwd
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -D turns of debug mode"
if ld-iamroot.so --debug env | grep "^IAMROOT_DEBUG=1$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --debug --debug turns of debug mode level 2"
if ld-iamroot.so --debug --debug env | grep "^IAMROOT_DEBUG=2$"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -h displays help message and exit"
if ld-iamroot.so -h | tee /dev/stderr | grep -q "^Usage:"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --help displays help message and exit"
if ld-iamroot.so --help | tee /dev/stderr | grep -q "^Usage:"
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option -V displays version information and exit"
if ld-iamroot.so -V | tee /dev/stderr | grep -q -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "ld-iamroot.so: test option --version displays version information and exit"
if ld-iamroot.so --version | tee /dev/stderr | grep -q -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo
