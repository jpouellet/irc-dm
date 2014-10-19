#!/bin/sh

if [ $# -ne 4 ]; then
	echo "Usage: $0 n_ppl max_msgs msg_int ppl_int" 1>&2
	printf "\tnumber of unique people\n" 1>&2
	printf "\tmax number of messages per person\n" 1>&2
	printf "\tmax message interval per person\n" 1>&2
	printf "\tmax new person interval\n" 1>&2
	exit 1
fi

n_ppl=$1;	shift
max_msgs=$1;	shift
msg_int=$1;	shift
ppl_int=$1;	shift

name_max=12
name_min=6 # long enough that dupes are improbable

pick_name() {
	head -c $name_max /dev/random | base64 | sed 's/[+/]//g' \
	| head -c $((($RANDOM % ($name_max - $name_min)) + $name_min))
}

update_time() {
	echo $(($1 + ($RANDOM % $msg_int)))
}

generate() {
	last_base=0
	i=0
	while [ $i -lt $n_ppl ]; do
		i=$(($i+1))

		base=$(($RANDOM % $ppl_int + $last_base))
		last_base=$base

		name=$(pick_name)

		printf "%s\t+%s\n" $base $name
		now=$(update_time $base)

		n_msgs=$(($RANDOM % $max_msgs))
		j=0
		while [ $j -lt $n_msgs ]; do
			j=$(($j+1))

			printf "%s\t %s\n" $now $name
			now=$(update_time $now)
		done

		printf "%s\t-%s\n" $now $name
	done
}

generate | sort -n | cut -f 2-
