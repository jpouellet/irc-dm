#include <sys/time.h>
#include <sys/resource.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "index.h"
#include "trace.h"

#define DEFAULT_SECS (3)

bool quiet = false;

#define MAGIC ((void *)0xdeadbeef)

int
do_round(struct index *idx, struct trace *t)
{
	const char *p;
	void *data;

	trace_foreach(t, p) {
		switch (*p) {
		case '+':
			assert(index_put(idx, p + 1, MAGIC) == 0);
			break;
		case ' ':
			assert(index_get(idx, p + 1, &data) == 0);
			assert(data == MAGIC);
			break;
		case '-':
			assert(index_del(idx, p + 1) == 0);
			break;
		default:
			errx(1, "invalid trace command character (%c)", *p);
		}
	}

	return 0;
}

volatile sig_atomic_t stop;

void
do_stop(int sig)
{
	stop = 1;
}

#define USEC(x) ((double)(x) / (double)1e6)

void
stats(const struct rusage *begin, const struct rusage *end, size_t rounds)
{
	time_t usr_s_delta, sys_s_delta, s_delta;
	suseconds_t usr_us_delta, sys_us_delta, us_delta;
	double usr_delta, sys_delta, delta, usr_rate, rate;

	usr_s_delta = end->ru_utime.tv_sec - begin->ru_utime.tv_sec;
	sys_s_delta = end->ru_stime.tv_sec - begin->ru_stime.tv_sec;
	s_delta = usr_s_delta + sys_s_delta;

	usr_us_delta = end->ru_utime.tv_usec - begin->ru_utime.tv_usec;
	sys_us_delta = end->ru_stime.tv_usec - begin->ru_stime.tv_usec;
	us_delta = usr_us_delta + sys_us_delta;

	usr_delta = (double)usr_s_delta + USEC(usr_us_delta);
	sys_delta = (double)sys_s_delta + USEC(sys_us_delta);
	delta = (double)s_delta + USEC(us_delta);

	usr_rate = (double)rounds / usr_delta;
	rate = (double)rounds / delta;

	printf("%ld\t%lf\t%lf\t%lf\t%lf\t%lf\t%s\n",
	    rounds, usr_delta, sys_delta, delta, usr_rate, rate,
	    getprogname());
}

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-q] [-t secs] trace\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct rusage begin, end;
	struct itimerval timer;
	struct trace *trace;
	struct index *idx;
	int ch, ret;
	time_t secs = DEFAULT_SECS;
	size_t round;

	while ((ch = getopt(argc, argv, "qt:")) != -1) {
		switch (ch) {
		case 'q':
			quiet = true;
			break;
		case 't':
			secs = atoi(optarg);
			if (secs <= 0)
				usage();
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	errno = 0;
	trace = trace_load(argv[0]);
	if (trace == NULL)
		err(1, "trace_load");

	idx = index_new();
	if (idx == NULL)
		errx(1, "index_new failed");

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_sec = secs;
	timer.it_value.tv_usec = 0;

	signal(SIGPROF, do_stop);

	round = 0;
	stop = 0;
	setitimer(ITIMER_PROF, &timer, NULL);
	getrusage(RUSAGE_SELF, &begin);
	do {
		ret = do_round(idx, trace);
		if (ret != 0)
			errx(1, "failed in round %zu", round);
		round++;
	} while (!stop);
	getrusage(RUSAGE_SELF, &end);

	stats(&begin, &end, round);

	return 0;
}
