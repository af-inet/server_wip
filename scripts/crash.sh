#!/usr/bin/env bash
set -e
set -x

{
	sleep 1;
	siege -r 50 -c 10 -f scripts/urls.txt
	if [ $? == 0 ]; then
		pkill plexer
	fi
	sleep 10;
} &> siege.logs.txt &

lldb -s ./cmds.txt ./plexer
