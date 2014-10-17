#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "msg.h"

int
main(int argc, char *argv[])
{
	struct msg *msg;

	msg_parser_init();

	if (argc != 2) {
		fprintf(stderr, "Usage: %s string\n", getprogname());
		return 1;
	}

	msg = msg_parse(argv[1]);
	if (msg == NULL)
		errx(1, "FAIL %s", argv[1]);

	msg_dump(msg);

	return 0;
}
