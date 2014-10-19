#!/bin/sh

trace() {
	printf "[%s]\n" "$1"
	./tracegen.sh $2 $3 $4 $5 $6 $7 $8 > traces/$1
	make bench TRACE_FILE=traces/$1 | sort -rn
	echo
}

mkdir -p traces

trace typical 100 50 10 5 1000 30 2
trace dos 1000 5 1 3 1000 0 0
trace high-volume 10 20 10 40 1000 0 0
trace spammers 100 50 50 50 500 40 1
trace large-spammers 100 50 50 50 500 200 1
