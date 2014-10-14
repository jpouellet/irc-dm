#include <assert.h>
#include <err.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "msg.h"

/* Big regex built up in readable parts. */
static const char msg_regex[] =
#include "msg_regex.h"
;

/* Don't forget to update the capture groups! */
#define PAT_NSUB (18)

/* Capture groups. */
#define CG_PREFIX (2)
#define CG_SERVER (3)
#define CG_NICK (5)
#define CG_USER (8)
#define CG_HOST (9)
#define CG_COMMAND (15)
#define CG_PARAMS (16)
#define CG_TRAILING (18)

#define PAT_NPMATCH (PAT_NSUB + 1)

static regex_t preg;
static regmatch_t pmatch[PAT_NPMATCH];

void
msg_parser_init(void)
{
	assert(regcomp(&preg, msg_regex, REG_EXTENDED) == 0);
	assert(preg.re_nsub == PAT_NSUB);
}

void
msg_free(struct msg *msg)
{
	size_t i;

	free(msg->raw);
	free(msg->prefix);
	free(msg->server);
	free(msg->nick);
	free(msg->user);
	free(msg->host);
	free(msg->command);
	for (i = 0; i < msg->n_params; i++)
		free(msg->params[i]);
	free(msg->trailing);
	free(msg);
}

struct msg *
msg_parse(const char *line)
{
	struct msg *msg;
	int i;

	msg = malloc(sizeof(struct msg));
	if (msg == NULL) {
		warn("malloc failed");
		return NULL;
	}

	if (regexec(&preg, line, PAT_NPMATCH, pmatch, 0) != 0) {
		warnx("invalid line: %s", line);
		return NULL;
	}

	msg->raw = strdup(line);
	for (i = 0; i < PAT_NPMATCH; i++)
		printf("%2d [%lld:%lld] %s\n",
		    i, pmatch[i].rm_so, pmatch[i].rm_eo,
		    strndup(&line[pmatch[i].rm_so],
		        pmatch[i].rm_eo - pmatch[i].rm_so));

	return msg;
}

char *
msg_param(const struct msg *msg, off_t param)
{
	if (param < 0 || (size_t)param >= msg->n_params)
		return NULL;
	return msg->params[param];
}

void
msg_dump(struct msg *msg)
{
	printf("[nsub %p | line=%s]\n", msg, msg->raw);
}

int
main(int argc, char *argv[])
{
	struct msg *msg;

	msg_parser_init();

	if (argc != 2) {
		fprintf(stderr, "Usage: %s string\n", argv[0]);
		return 1;
	}

	msg = msg_parse(argv[1]);
	if (msg == NULL)
		errx(1, "no msg");

	msg_dump(msg);

	return 0;
}
