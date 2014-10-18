#include <assert.h>
#include <err.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "index.h"

#define N_ROUNDS (100)

bool quiet = false;

int
__get(struct index *idx, const char *key, void *val)
{
	void *data;
	int rv;

	rv = index_get(idx, key, &data);

	if (val != NULL) {
		/* we expect something in particular */
		if (rv != 0)
			return -1;
		if (data != val)
			return -1;
	} else {
		/* we expect it missing */
		if (rv != 1)
			return -1;
	}
	return 0;
}
#define _get(k,v) (__get(idx,(k),(void *)(v)))
#define G(k,v) assert(_get((#k),(v)) == 0)

int
__put(struct index *idx, const char *key, void *val)
{
	int rv;

	rv = index_put(idx, key, val);

	if (val != NULL) {
		/* we expect it to have been inserted */
		if (rv != 0)
			return -1;
	} else {
		/* we expected something else there */
		if (rv != 1)
			return -1;
	}
	return 0;
}
#define _put(k,v) (__put(idx,(k),(void *)(v)))
#define P(k,v) assert(_put((#k),(v)) == 0)

int
__del(struct index *idx, const char *key, void *val)
{
	int rv;

	rv = index_del(idx, key);

	if (val != NULL) {
		/* we expected something there */
		if (rv != 0)
			return -1;
	} else {
		/* we expected it missing */
		if (rv != 1)
			return -1;
	}
	return 0;
}
#define _del(k,v) (__del(idx,(k),(void *)(v)))
#define D(k,v) assert(_del((#k),(v)) == 0)

#define X ((void *)(1))

int
do_round(struct index *idx)
{
	G(foo, NULL);
	P(foo, 1234);
	G(foo, 1234);
	D(foo, X);
	D(foo, NULL);

	return 0;
}

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-q] index.so ...\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct index *idx;
	int ch, i, ret;

	while ((ch = getopt(argc, argv, "q")) != -1) {
		switch (ch) {
		case 'q':
			quiet = true;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	/* TODO init timer */
	for (i = 0; i < N_ROUNDS; i++) {
		idx = index_new();
		if (idx == NULL)
			errx(1, "index_new failed in round %d", i);

		/* TODO start timer */
		ret = do_round(idx);
		/* TODO stop timer */
		if (ret != 0)
			errx(1, "failed in round %d", i);
	}
	/* TODO print timer */

	return 0;
}
