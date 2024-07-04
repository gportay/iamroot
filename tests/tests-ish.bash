#!/bin/bash
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

PATH="$PWD:$PATH"
trap result 0 SIGINT

export -n NO_COLOR
export -n ISHLVL
export -n ISH_RC_FILE
export -n ISH_PROFILE_FILE
export -n ISH_PRESERVE_ENV
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

run "ish: test without argument opens interactive shell"
if ish <<<'echo "-=$-"' | grep "^-=hBs$"
then
	ok
else
	ko
fi
echo

run "ish: test option -c executes commands from a command line string"
if ish -c 'echo "-=$-"' | grep "^-=hBc$"
then
	ok
else
	ko
fi
echo

run "ish: test option -s executes commands from standard input"
if ish -s <<<'echo "-=$-"' | grep "^-=hBs$"
then
	ok
else
	ko
fi
echo

run "ish: test option -i specifies the shell is interactive"
if ish -i <<<'echo "-=$-"' 2>&1 | grep "^-=himBHs"
then
	ok
else
	ko
fi
echo

run "ish: test option --root $PWD/rootfs sets the absolute path to the root directory to chroot in"
if ish --root "$PWD/rootfs" -c "env" | grep "^IAMROOT_ROOT=$PWD/rootfs$"
then
	ok
else
	ko
fi
echo

run "ish: test option --root /dev/null fails to set the absolute path to the root directory to chroot in"
if ! ish --root /dev/null -c "env"
then
	ok
else
	ko
fi
echo

run "ish: test option --multiarch uses multiarch library path in chroot"
if ish --multiarch -c "env" | grep "^IAMROOT_MULTIARCH=1$"
then
	ok
else
	ko
fi
echo

run "ish: test option --libdir set /lib:/usr/lib as default library path in chroot"
if ish --libdir -c "env" | grep "^IAMROOT_DEFLIB=/lib:/usr/lib$"
then
	ok
else
	ko
fi
echo

run "ish: test option --deflib /usr/local/lib:/lib:/usr/lib sets default library path to use in chroot"
if ish --deflib /usr/local/lib:/lib:/usr/lib -c "env" | grep "^IAMROOT_DEFLIB=/usr/local/lib:/lib:/usr/lib$"
then
	ok
else
	ko
fi
echo

run "ish: test option --exec /dev/null sets the absolute path to the iamroot exec script to use"
if ish --exec /dev/null -c "env" | grep "^IAMROOT_EXEC=/dev/null$"
then
	ok
else
	ko
fi
echo

run "ish: test option --library /dev/null sets the absolute path to the iamroot library to use"
if ish --library /dev/null -c "env" | grep "^IAMROOT_LIB=/dev/null$"
then
	ok
else
	ko
fi
echo

run "ish: test option --shell /bin/sh sets the shell interpretor"
if ish --shell /bin/sh -c "env" | grep "^SHELL=/bin/sh$"
then
	ok
else
	ko
fi
echo

run "ish: test option --shell /bin/bash sets the shell interpretor"
if ish --shell /bin/bash -c "env" | grep "^SHELL=/bin/bash$"
then
	ok
else
	ko
fi
echo

run "ish: test option --shell /bin/nologin sets the shell interpretor"
if ! ish --shell /bin/nologin -c "env"
then
	ok
else
	ko
fi
echo

run "ish: test option --no-color turns off color"
if ish --no-color -c "env" | grep "^NO_COLOR=1$"
then
	ok
else
	ko
fi
echo

run "ish: test option --debug turns of debug mode"
if ish --debug -c "env" | grep "^IAMROOT_DEBUG=1$"
then
	ok
else
	ko
fi
echo

run "ish: test option --debug --debug turns of debug mode level 2"
if ish --debug --debug -c "env" | grep "^IAMROOT_DEBUG=2$"
then
	ok
else
	ko
fi
echo

run "ish: test option --debug-fd 100 sets debug fd"
if ( exec 100>&2 && ish --debug-fd 100 -c "env" | grep "^IAMROOT_DEBUG_FD=100$" )
then
	ok
else
	ko
fi
echo

run "ish: test option --debug-fd 100 sets and duplicates debug fd"
if ish --debug-fd 100 -c "env && readlink /proc/self/fd/100" | grep "^IAMROOT_DEBUG_FD=100$"
then
	ok
else
	ko
fi
echo

run "ish: test option --debug-ignore ^(chroot|chdir|fchdir)$ sets regular expression of path to ignore for path resolution in chroot."
if ish --debug-ignore "^(chroot|chdir|fchdir)$" -c "env" | grep "^IAMROOT_DEBUG_IGNORE=^(chroot|chdir|fchdir)\\$\$"
then
	ok
else
	ko
fi
echo

run "ish: test option --exec-ignore ^(ldd|ldconfig)$ sets regular expression of path to ignore for path resolution in chroot."
if ish --exec-ignore "^(ldd|ldconfig)$" -c "env" | grep "^IAMROOT_EXEC_IGNORE=^(ldd|ldconfig)\\$\$"
then
	ok
else
	ko
fi
echo

run "ish: test option --path-resolution-ignore ^(/proc/|/sys/|/dev/)$ sets regular expression of path to ignore for path resolution in chroot."
if ish --path-resolution-ignore "^(/proc/|/sys/|/dev/)$" -c "env" | grep "^IAMROOT_PATH_RESOLUTION_IGNORE=^(/proc/|/sys/|/dev/)\\$\$"
then
	ok
else
	ko
fi
echo

run "ish: test option --version prints version"
if ish --version | grep -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "ish: test option --help prints usage"
if ish --help | grep -o "^Usage:"
then
	ok
else
	ko
fi
echo
