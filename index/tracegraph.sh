#!/bin/sh

[ -n "$1" ] && v=$1 || v=0

bar() {
	if [ $1 -gt 0 ]; then
		head -c $1 /dev/zero | tr -C x x
	fi
	echo
}

last_dir=
dir=

dir_change() {
	[ -n "$last_dir" -a -n "$dir" -a X"$last_dir" != X"$dir" ]
}

x=0
max=0
while read; do
	case $REPLY in
	+*)
		dir=up
		x=$(($x + 1))
		[ $x -gt $max ] && max=$x
		( dir_change || [ $v -ge 1 ] ) && bar $x
		;;
	-*)
		dir=down
		x=$(($x - 1))
		( dir_change || [ $v -ge 1 ] ) && bar $x
		;;
	*)
		[ $v -ge 2 ] && bar $x
		;;
	esac
	last_dir=$dir
done

echo max: $max
