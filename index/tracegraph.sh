#!/bin/sh

bar() {
	if [ $1 -gt 0 ]; then
		head -c $1 /dev/zero | tr -C x x
	fi
	echo
}

x=0
while read; do
	case $REPLY in
	+*) x=$(($x + 1));;
	-*) x=$(($x - 1));;
	esac
	bar $x
done
