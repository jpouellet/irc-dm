#!/bin/sh

if [ $# -ne 7 ]; then
	echo "Usage: $0 n_ppl max_msgs msg_int ppl_int" 1>&2
	printf "\tnumber of unique people\n" 1>&2
	printf "\tmax number of messages per person\n" 1>&2
	printf "\tmax message interval per person\n" 1>&2
	printf "\tmax new person interval\n" 1>&2
	printf "\tspam chance\n" 1>&2
	printf "\tmessages per spamming run\n" 1>&2
	printf "\tspam message interval\n" 1>&2
	exit 1
fi

n_ppl=$1;	shift
max_msgs=$1;	shift
msg_int=$1;	shift
ppl_int=$1;	shift
spam_chance=$1;	shift
spam_msgs=$1;	shift
spam_int=$1;	shift

name_max=12
name_min=6 # long enough that dupes are improbable

head_c() {
	[ -n "$2" ] && f="if=$2" || f=
	dd bs=$1 count=1 $f 2>/dev/null
}

pick_name() {
	head_c $name_max /dev/random | base64 | sed 's/[+/]//g' \
	| head_c $((($RANDOM % ($name_max - $name_min)) + $name_min))
}

now=

update_time() {
	now=$(($now + ($RANDOM % $1)))
}

spam_check() {
	if [ $(($RANDOM % $spam_chance)) -eq 0 ]; then
		i=0
		while [ $i -lt $spam_msgs ]; do
			i=$(($i+1))
			printf "%s\t %s\n" $now $1
			update_time $spam_int
		done
	fi
}

generate() {
	last_base=0
	i=0
	while [ $i -lt $n_ppl ]; do
		i=$(($i+1))

		now=$(($RANDOM % $ppl_int + $last_base))
		last_base=$now

		name=$(pick_name)

		printf "%s\t+%s\n" $now $name
		update_time $msg_int

		n_msgs=$(($RANDOM % $max_msgs))
		j=0
		while [ $j -lt $n_msgs ]; do
			j=$(($j+1))

			spam_check $name

			printf "%s\t %s\n" $now $name
			update_time $msg_int
		done

		printf "%s\t-%s\n" $now $name
	done
}

generate | sort -sn | cut -f 2-
