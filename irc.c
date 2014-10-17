#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
#include "util.h"

static void irc_msg_dispatch(struct msg *);

int
irc_login(struct bufferevent *bev, const char *nick, const char *user,
  const char *real)
{
	char *str;
	int ret;

	if (!is_valid_nick(nick)) {
		warnx("invalid nick");
		return -1;
	}
	if (!is_valid_username(user)) {
		warnx("invalid username");
		return -1;
	}
	if (!is_valid_realname(real)) {
		warnx("invalid realname");
		return -1;
	}

	asprintf(&str, "NICK %s\r\nUSER %s 0 * :%s\r\n", nick, user, real);
	if (str == NULL)
		return -1;

	ret = bufferevent_write(bev, str, strlen(str));
	free(str);
	return (ret == 0 ? 0 : -1);
}

void
irc_bev_read_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *buf;
	struct msg *msg;
	char *line;

	buf = bufferevent_get_input(bev);
	while ((line = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF)) != NULL) {
		msg = msg_parse(line);
		if (msg != NULL) {
			irc_msg_dispatch(msg);
			msg_free(msg);
		}
		free(line);
	}
}

void
irc_bev_event_cb(struct bufferevent *bev, short what, void *ctx)
{
	if (what & BEV_EVENT_EOF) {
		bufferevent_disable(bev, EV_READ);
		event_base_loopbreak(bufferevent_get_base(bev));
	}

	if (what & BEV_EVENT_ERROR) {
		bufferevent_disable(bev, EV_READ);
		warnx("bufferevent error for ressl");
		event_base_loopbreak(bufferevent_get_base(bev));
	}
}

static void
irc_msg_dispatch(struct msg *msg)
{
	msg_dump(msg);
}

/*
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
char * msg_param(const struct msg *, off_t param);
*/
