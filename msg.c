#include <assert.h>
#include <err.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PAT "^:$"
#define PAT_NSUB (3)
#define PAT_NPMATCH (PAT_NSUB + 1)

struct msg {
	char *line;
};

regex_t preg;
regmatch_t pmatch[PAT_NPMATCH];

void msg_parser_init(void);
void msg_free(struct msg *);
struct msg * msg_parse(const char *);
void msg_dump(struct msg *);

void
msg_parser_init(void)
{
	assert(regcomp(&preg, PAT, REG_EXTENDED) == 0);
printf("nsub %zu\n", preg.re_nsub);
	assert(preg.re_nsub == PAT_NSUB);
}

void
msg_free(struct msg *msg)
{
	free(msg->line);
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

	msg->line = strdup(line);
	for (i = 0; i < PAT_NPMATCH; i++)
		printf("[%lld:%lld] %s\n",
		    pmatch[i].rm_so, pmatch[i].rm_eo,
		    strndup(&line[pmatch[i].rm_so],
		        pmatch[i].rm_eo - pmatch[i].rm_so));

	return msg;
}

void
msg_dump(struct msg *msg)
{
	printf("[nsub %p | line=%s]\n", msg, msg->line);
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
