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
#define CG_ALL (0)
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
	free(msg->raw);
	free(msg->prefix);
	free(msg->server);
	free(msg->nick);
	free(msg->user);
	free(msg->host);
	free(msg->command);
	if (msg->params)
		free(msg->params[0]);
	free(msg->params);
	free(msg->trailing);
	free(msg);
}

static int
extract(const char *str, const regmatch_t *match, char **store)
{
	char *copy;

	if (match->rm_so < 0 || match->rm_eo < 0) {
		assert(match->rm_so == -1);
		assert(match->rm_eo == -1);
		*store = NULL;
		return 0;
	}
	assert(match->rm_so <= match->rm_eo);

	copy = strndup(&str[match->rm_so], match->rm_eo - match->rm_so);
	if (copy == NULL) {
		warn("malloc failed");
		return -1;
	}

	*store = copy;
	return 0;
}

struct msg *
msg_parse(const char *line)
{
	struct msg *msg;
	char *params = NULL, *p;
	size_t i, n;

	if (regexec(&preg, line, PAT_NPMATCH, pmatch, 0) != 0) {
		warnx("invalid line: %s", line);
		return NULL;
	}

	msg = malloc(sizeof(struct msg));
	if (msg == NULL) {
		warn("malloc failed");
		return NULL;
	}
	memset(msg, 0, sizeof(struct msg));

	if (extract(line, &pmatch[CG_ALL], &msg->raw) ||
	    extract(line, &pmatch[CG_PREFIX], &msg->prefix) ||
	    extract(line, &pmatch[CG_SERVER], &msg->server) ||
	    extract(line, &pmatch[CG_NICK], &msg->nick) ||
	    extract(line, &pmatch[CG_USER], &msg->user) ||
	    extract(line, &pmatch[CG_HOST], &msg->host) ||
	    extract(line, &pmatch[CG_COMMAND], &msg->command) ||
	    extract(line, &pmatch[CG_TRAILING], &msg->trailing))
		goto cleanup;

	if (extract(line, &pmatch[CG_COMMAND], &params))
		goto cleanup;

	if (params != NULL) {
		n = 0;
		for (p = params; *p != '\0'; p++)
			if (*p == ' ')
				n++;
		msg->params = calloc(n, sizeof(char *));
		if (msg->params == NULL)
			goto cleanup;

		i = 0;
		msg->params[i++] = params;
		for (p = params; *p != '\0'; p++) {
			if (*p == ' ') {
				*p = '\0';
				assert(i < n);
				msg->params[i++] = p + 1;
			}
		}
		msg->n_params = n;
		params = NULL;
	}

	return msg;

cleanup:
	msg_free(msg);
	free(params);
	return NULL;
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
	size_t i;

#define dump(sym) printf("\t" #sym "\t[%s]\n", msg->sym ? msg->sym : "(empty)")
	printf("[%s]\n", msg->raw);
	dump(prefix);
	dump(server);
	dump(nick);
	dump(user);
	dump(host);
	dump(command);
	for (i = 0; i < msg->n_params; i++)
		printf("\tparam %zu\t[%s]\n", i, msg->params[i]);
	dump(trailing);
}

#ifdef TEST
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
#endif
