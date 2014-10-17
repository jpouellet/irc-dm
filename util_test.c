#include <err.h>
#include <stdio.h>

#include "util.h"

#define VALID(x) (x ? "valid" : "*INVALID*")

int
main()
{
	char *line = NULL;
	size_t linecap = 0;
	size_t linelen;

	if (util_init() == -1)
		errx(1, "util_init failed");

	while ((linelen = getline(&line, &linecap, stdin)) > 0) {
		line[linelen - 1] = '\0';
		printf("\tnick: %s\n\tuser: %s\n\treal: %s\n",
		    VALID(is_valid_nick(line)),
		    VALID(is_valid_username(line)),
		    VALID(is_valid_realname(line)));
	}

	return 0;
}
