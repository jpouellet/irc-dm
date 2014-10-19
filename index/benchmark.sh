#!/bin/sh

trace() {
	printf "[%s]\n" "$1"
	./tracegen.sh $2 $3 $4 $5 > traces/$1
	make bench TRACE_FILE=traces/$1 | sort -rn
	echo
}

mkdir -p traces

trace typical 100 50 10 5
trace dos 1000 5 1 3
trace high-volume 10 20 10 40
