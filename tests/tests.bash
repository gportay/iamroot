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
	echo -e "\e[1mRunning $test...\e[0m"
}

ok() {
	ok=$((ok+1))
	echo -e "\e[1m$test: \e[32m[OK]\e[0m"
}

ko() {
	ko=$((ko+1))
	echo -e "\e[1m$test: \e[31m[KO]\e[0m"
	reports+=("$test at line \e[1m$lineno \e[31mhas failed\e[0m!")
	if [[ $EXIT_ON_ERROR ]]
	then
		exit 1
	fi
}

fix() {
	fix=$((fix+1))
	echo -e "\e[1m$test: \e[34m[FIX]\e[0m"
	reports+=("$test at line \e[1m$lineno is \e[34mfixed\e[0m!")
}

bug() {
	bug=$((bug+1))
	echo -e "\e[1m$test: \e[33m[BUG]\e[0m"
	reports+=("$test at line \e[1m$lineno is \e[33mbugged\e[0m!")
}

result() {
	exitcode="$?"
	trap - 0

	echo -e "\e[1mTest report:\e[0m"
	for report in "${reports[@]}"
	do
		echo -e "$report" >&2
	done

	if [[ $ok ]]
	then
		echo -e "\e[1m\e[32m$ok test(s) succeed!\e[0m"
	fi

	if [[ $fix ]]
	then
		echo -e "\e[1m\e[34m$fix test(s) fixed!\e[0m" >&2
	fi

	if [[ $bug ]]
	then
		echo -e "\e[1mWarning: \e[33m$bug test(s) bug!\e[0m" >&2
	fi

	if [[ $ko ]]
	then
		echo -e "\e[1mError: \e[31m$ko test(s) failed!\e[0m" >&2
	fi

	if [[ $exitcode -ne 0 ]] && [[ $ko ]]
	then
		echo -e "\e[1;31mExited!\e[0m" >&2
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
export -n IAMROOT_FATAL
export -n IAMROOT_DEBUG
export -n IAMROOT_DEBUG_FD
export -n IAMROOT_DEBUG_IGNORE
export -n IAMROOT_EXEC_IGNORE
export -n IAMROOT_PATH_RESOLUTION_IGNORE

run "Test without argument opens interactive shell"
if ish <<<'echo "-=$-"' | grep "^-=hBs$"
then
	ok
else
	ko
fi
echo

run "Test option -c executes commands from a command line string"
if ish -c 'echo "-=$-"' | grep "^-=hBc$"
then
	ok
else
	ko
fi
echo

run "Test option -s executes commands from standard input"
if ish -s <<<'echo "-=$-"' | grep "^-=hBs$"
then
	ok
else
	ko
fi
echo

run "Test option -i specifies the shell is interactive"
if ish -i <<<'echo "-=$-"' 2>&1 | grep "^-=himBHs"
then
	ok
else
	ko
fi
echo

run "Test option --user root sets loging name of the user root"
if ish --user root -c "id -un" | grep "^root$"
then
	ok
else
	ko
fi

run "Test option --root rootfs sets the absolute path to the root directory to chroot in"
if ish --root "$PWD/rootfs" -c "env" | grep "^IAMROOT_ROOT=$PWD/rootfs$"
then
	ok
else
	ko
fi
echo

run "Test option --root /dev/null fails to set the absolute path to the root directory to chroot in"
if ! ish --root /dev/null -c "env"
then
	ok
else
	ko
fi
echo

run "Test environment is not preserved"
if ! ( export FOO=foo && ish -c "env" | grep "^FOO=foo$" )
then
	ok
else
	ko
fi

run "Test option --preserve-env FOO preserves environment variable"
if ( export FOO=foo && ish --preserve-env FOO -c "env" | grep "^FOO=foo$" )
then
	ok
else
	ko
fi

run "Test option --preserve-env FOO:BAZ preserves environment variables"
if ( export FOO=foo; export BAR=bar && ish --preserve-env FOO:BAR -c "env" | grep "^FOO=foo$" &&
     export FOO=foo; export BAR=bar && ish --preserve-env FOO:BAR -c "env" | grep "^BAR=bar$" )
then
	ok
else
	ko
fi

run "Test option --deflib /usr/local/lib:/lib:/usr/lib sets default library path to use in chroot"
if ish --deflib /usr/local/lib:/lib:/usr/lib -c "env" | grep "^IAMROOT_DEFLIB=/usr/local/lib:/lib:/usr/lib$"
then
	ok
else
	ko
fi

run "Test option --exec /dev/null sets the absolute path to the iamroot exec script to use"
if ish --exec /dev/null -c "env" | grep "^IAMROOT_EXEC=/dev/null$"
then
	ok
else
	ko
fi

run "Test option --library /dev/null sets the absolute path to the iamroot library to use"
if ish --library /dev/null -c "env" | grep "^IAMROOT_LIB=/dev/null$"
then
	ok
else
	ko
fi

run "Test option --shell /bin/sh sets the shell interpretor"
if ish --shell /bin/sh -c "env" | grep "^SHELL=/bin/sh$"
then
	ok
else
	ko
fi

run "Test option --shell /bin/bash sets the shell interpretor"
if ish --shell /bin/bash -c "env" | grep "^SHELL=/bin/bash$"
then
	ok
else
	ko
fi

run "Test option --shell /bin/nologin sets the shell interpretor"
if ! ish --shell /bin/nologin -c "env"
then
	ok
else
	ko
fi

run "Test option --no-color turns off color"
if ish --no-color -c "env" | grep "^NO_COLOR=1$"
then
	ok
else
	ko
fi

run "Test option --fatal sets abort on error"
if ish --fatal -c "env" | grep "^IAMROOT_FATAL=1$"
then
	ok
else
	ko
fi

run "Test option --debug turns of debug mode"
if ish --debug -c "env" | grep "^IAMROOT_DEBUG=1$"
then
	ok
else
	ko
fi

run "Test option --debug --debug turns of debug mode level 2"
if ish --debug --debug -c "env" | grep "^IAMROOT_DEBUG=2$"
then
	ok
else
	ko
fi

run "Test option --debug-fd 100 sets debug fd"
if ( exec 100>&2 && ish --debug-fd 100 -c "env" | grep "^IAMROOT_DEBUG_FD=100$" )
then
	ok
else
	ko
fi

run "Test option --debug-fd 100 sets and duplicates debug fd"
if ish --debug-fd 100 -c "env && readlink /proc/self/fd/100" | grep "^IAMROOT_DEBUG_FD=100$"
then
	ok
else
	ko
fi

run "Test option --debug-ignore ^(chroot|chdir|fchdir)$ sets regular expression of path to ignore for path resolution in chroot."
if ish --debug-ignore "^(chroot|chdir|fchdir)$" -c "env" | grep "^IAMROOT_DEBUG_IGNORE=^(chroot|chdir|fchdir)\\$\$"
then
	ok
else
	ko
fi

run "Test option --exec-ignore ^(ldd|ldconfig)$ sets regular expression of path to ignore for path resolution in chroot."
if ish --exec-ignore "^(ldd|ldconfig)$" -c "env" | grep "^IAMROOT_EXEC_IGNORE=^(ldd|ldconfig)\\$\$"
then
	ok
else
	ko
fi

run "Test option --path-resolution-ignore ^(/proc/|/sys/|/dev/)$ sets regular expression of path to ignore for path resolution in chroot."
if ish --path-resolution-ignore "^(/proc/|/sys/|/dev/)$" -c "env" | grep "^IAMROOT_PATH_RESOLUTION_IGNORE=^(/proc/|/sys/|/dev/)\\$\$"
then
	ok
else
	ko
fi

run "Test option --version prints version"
if ish --version | grep -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "Test option --help prints usage"
if ish --help | grep -o "^Usage:"
then
	ok
else
	ko
fi
echo

run "Test environment sets ISH_COMMAND to the command run by ish"
if ish <<<'echo "ISH_COMMAND=$ISH_COMMAND"' | grep -E "^ISH_COMMAND=/bin/bash$"
then
	ok
else
	ko
fi
echo

run "Test environment sets ISH_COMMAND to the command run by ish"
if ( SHELL=/bin/sh && ish <<<'echo "ISH_COMMAND=$ISH_COMMAND"' | grep "^ISH_COMMAND=/bin/sh$" )
then
	ok
else
	ko
fi
echo

run "Test environment sets ISH_COMMAND to the command run by ish"
if ish -c "env" | grep "^ISH_COMMAND=/bin/bash -c env$"
then
	ok
else
	ko
fi
echo

run "Test environment sets ISH_GID to the group-ID of the user who invoked ish"
if ish -s <<<'echo "ISH_GID=$ISH_GID"' | grep "^ISH_GID=${GROUPS[0]}$"
then
	ok
else
	ko
fi
echo

run "Test environment sets ISH_UID to the user-ID of the user who invoked ish"
if ish -s <<<'echo "ISH_UID=$ISH_UID"' | grep "^ISH_UID=$UID$"
then
	ok
else
	ko
fi
echo

run "Test environment sets ISH_USER to the login name of the user who invoked ish"
if ish -s <<<'echo "ISH_USER=$ISH_USER"' | grep "^ISH_USER=$USER$"
then
	ok
else
	ko
fi
echo
