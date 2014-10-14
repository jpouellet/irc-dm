#ifndef _MSG_H_
#define _MSG_H_

struct msg {
	char *raw;
	char *prefix;
	char *server;
	char *nick;
	char *user;
	char *host;
	char *command;
	size_t n_params;
	char **params;
	char *trailing;
};

void msg_parser_init(void);
void msg_free(struct msg *);
struct msg * msg_parse(const char *);
void msg_dump(struct msg *);

#endif
