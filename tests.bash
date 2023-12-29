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

run "Test without argument"
if ! ido
then
	ok
else
	ko
fi
echo

run "Test environment is not preserved"
if ! ( export FOO=foo && ido env | tee /dev/stderr | grep -q "^FOO=foo$" )
then
	ok
else
	ko
fi

run "Test option -C=101 do not closes file descriptors < 100"
if ( exec 100>&2 && ido -C=101 readlink /proc/self/fd/100 )
then
	ok
else
	ko
fi

run "Test option --close-from=100 closes file descriptor >= 100"
if ( exec 100>&2 && ! ido --close-from=100 readlink /proc/self/fd/100 )
then
	ok
else
	ko
fi
echo

run "Test option -D=tests changes working directory"
if ido -D=tests pwd | tee /dev/stderr | grep -q "^$PWD/tests$"
then
	ok
else
	ko
fi

run "Test option --chdir=/dev/null fails to change working directory"
if ! ido --chdir=/dev/null pwd
then
	ok
else
	ko
fi

run "Test option -E preserve user environment when running command"
if ( export FOO=foo && ido -E env | tee /dev/stderr | grep -q "^FOO=foo$" )
then
	ok
else
	ko
fi

run "Test option --preserve-env=FOO,BAR preserves specific environment variables FOO and BAR"
if ( export FOO=foo; export BAR=bar && ido --preserve-env=FOO,BAR env | tee /dev/stderr | grep -q "^FOO=foo$" &&
     export FOO=foo; export BAR=bar && ido --preserve-env=FOO,BAR env | tee /dev/stderr | grep -q "^BAR=bar$")
then
	ok
else
	ko
fi

run "Test options --preserve-env=FOO,BAZ --preserve-env=BAZ preserves specific environment variables FOO, BAR and BAZ"
if ( export FOO=foo; export BAR=bar; export BAZ=baz && ido --preserve-env=FOO,BAR --preserve-env=BAZ env | tee /dev/stderr | grep -q "^FOO=foo$" &&
     export FOO=foo; export BAR=bar; export BAZ=baz && ido --preserve-env=FOO,BAR --preserve-env=BAZ env | tee /dev/stderr | grep -q "^BAR=bar$" &&
     export FOO=foo; export BAR=bar; export BAZ=baz && ido --preserve-env=FOO,BAR --preserve-env=BAZ env | tee /dev/stderr | grep -q "^BAZ=baz$" )
then
	ok
else
	ko
fi

run "Test option -g=root runs command as group name root"
if ido -g=root id -gn | tee /dev/stderr | grep -q "^root$"
then
	ok
else
	ko
fi

run "Test option -g=\#0 runs command as group ID 0"
if ido -g=0 id -g | tee /dev/stderr | grep -q "^0$"
then
	ok
else
	ko
fi

run "Test option --group=\#${GROUPS[0]} runs command as group ID ${GROUPS[0]}"
if ido --group="${GROUPS[0]}" id -g | tee /dev/stderr | grep -q "^${GROUPS[0]}$"
then
	ok
else
	ko
fi

run "Test option -H sets HOME variable to root's home directory"
if ido -H env | tee /dev/stderr | grep -q "^HOME=/root$"
then
	ok
else
	ko
fi

run "Test option --set-home sets HOME variable to target user's home directory"
if ido -u="$USER" --set-home env | tee /dev/stderr | grep -q "^HOME=$HOME$"
then
	ok
else
	ko
fi

run "Test option -h displays help message and exit"
if ido -h | grep "^usage:"
then
	ok
else
	ko
fi
echo

run "Test option --help displays help message and exit"
if ido --help | grep "^usage:"
then
	ok
else
	ko
fi
echo

run "Test option -i runs login shell as root user"
if ( export SHELL=/bin/nologin && ido -i <<<whoami | tee /dev/stderr | grep -q -E "^root$" )
then
	ok
else
	ko
fi
echo

run "Test option --login runs login shell as root user with the specified command"
if ( export SHELL=/bin/nologin && ido --login whoami | tee /dev/stderr | grep -q -E "^root$" )
then
	ok
else
	ko
fi
echo

run "Test option -P preserves group vector ${GROUPS[*]}"
if ( export SHELL=/bin/bash && ido -P -s <<<'echo "GROUPS=${GROUPS[*]:1}"' | tee /dev/stderr | grep -q "^GROUPS=${GROUPS[*]}$" )
then
	ok
else
	ko
fi
echo

run "Test option --preserve-groups preserves group vector ${GROUPS[*]}"
if ( export SHELL=/bin/bash && ido --user="$USER" --preserve-groups --shell <<<'echo "GROUPS=${GROUPS[*]:1}"' | tee /dev/stderr | grep -q "GROUPS=${GROUPS[*]:1}" )
then
	ok
else
	ko
fi
echo

run "Test option -R=$PWD/rootfs changes root directory"
if ido -R="$PWD/rootfs" env | tee /dev/stderr | grep -q "^IAMROOT_ROOT=$PWD/rootfs$"
then
	ok
else
	ko
fi
echo

run "Test option --chroot=/dev/null fails to change root directory"
if ! ido --chroot=/dev/null env
then
	ok
else
	ko
fi
echo

run "Test option -s runs /bin/sh"
if ( export SHELL=/bin/sh; ido -s <<<env | tee /dev/stderr | grep -q '^SHELL=/bin/sh$' )
then
	ok
else
	ko
fi
echo

run "Test option --shell runs /bin/bash with the specified command"
if ( export SHELL=/bin/bash; ido --shell env | tee /dev/stderr | grep -q '^SHELL=/bin/bash$' )
then
	ok
else
	ko
fi
echo

run "Test option -T terminates command after the specfified time limit"
if ( ido -T=0.1 sleep 5; test "$?" -eq 124 )
then
	ok
else
	ko
fi
echo

run "Test option --command-timeout does not terminate command after the specfified time limit"
if ( ido --command-timeout=5 sleep 1; test "$?" -eq 0 )
then
	ok
else
	ko
fi
echo

run "Test option -u=root runs command as user name root"
if ido -u=root id -un | tee /dev/stderr | grep -q "^root$"
then
	ok
else
	ko
fi

run "Test option -u=\#0 runs command as user ID 0"
if ido -u=0 id -u | tee /dev/stderr | grep -q "^0$"
then
	ok
else
	ko
fi

run "Test option --user=$USER runs command as user name $USER"
if ido --user="$USER" id -un | tee /dev/stderr | grep -q "^$USER$"
then
	ok
else
	ko
fi

run "Test option --user=\#$UID runs command as user ID $UID"
if ido --user="#$UID" id -u | tee /dev/stderr | grep -q "^$UID$"
then
	ok
else
	ko
fi

run "Test option -V displays version information and exit"
if ido -V | tee /dev/stderr | grep -q -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "Test option --version displays version information and exit"
if ido --version | tee /dev/stderr | grep -q -E '^([0-9a-zA-Z]+)(\.[0-9a-zA-Z]+)*$'
then
	ok
else
	ko
fi
echo

run "Test both options -i -s"
if ! ido -i -s
then
	ok
else
	ko
fi
echo

run "Test both options -i -E"
if ! ido -i -E
then
	ok
else
	ko
fi
echo

run "Test environment sets LOGNAME to the login name of the target user"
if ido -s <<<'echo "LOGNAME=$LOGNAME"' | tee /dev/stderr | grep -q "^LOGNAME=root$"
then
	ok
else
	ko
fi
echo

run "Test environment sets IDO_COMMAND to the command run by ido"
if ( export SHELL=/bin/sh && ido -s <<<'echo "IDO_COMMAND=$IDO_COMMAND"' | tee /dev/stderr | grep -q "^IDO_COMMAND=/bin/sh$" )
then
	ok
else
	ko
fi
echo

run "Test environment sets IDO_COMMAND to the command run by ido"
if ( ido -i <<<'echo "IDO_COMMAND=$IDO_COMMAND"' | tee /dev/stderr | grep -q -E "^IDO_COMMAND=[A-Za-z0-9/]+ -l$" )
then
	ok
else
	ko
fi
echo

run "Test environment sets IDO_COMMAND to the command run by ido, including args"
if ( export SHELL=/bin/sh && ido -s env | tee /dev/stderr | grep -q "^IDO_COMMAND=/bin/sh -c 'env'$" )
then
	ok
else
	ko
fi
echo

run "Test environment sets IDO_GID to the group-ID of the user who invoked ido"
if ido -s <<<'echo "IDO_GID=$IDO_GID"' | tee /dev/stderr | grep -q "^IDO_GID=${GROUPS[0]}$"
then
	ok
else
	ko
fi
echo

run "Test environment sets IDO_UID to the user-ID of the user who invoked ido"
if ido -s <<<'echo "IDO_UID=$IDO_UID"' | tee /dev/stderr | grep -q "^IDO_UID=$UID$"
then
	ok
else
	ko
fi
echo

run "Test environment sets IDO_USER to the login name of the user who invoked ido"
if ido -s <<<'echo "IDO_USER=$IDO_USER"' | tee /dev/stderr | grep -q "^IDO_USER=$USER$"
then
	ok
else
	ko
fi
echo

run "Test environment sets USER to the same value as LOGNAME"
if ido -s <<<'echo "$USER=$LOGNAME"' | tee /dev/stderr | grep -q "^root=root$"
then
	ok
else
	ko
fi
echo
