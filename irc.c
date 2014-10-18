#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bev_util.h"
#include "msg.h"
#include "util.h"

static int irc_msg_dispatch(struct msg *, struct bufferevent *);
void cmd_privmsg(struct msg *, struct bufferevent *);
void cmd_ping(struct msg *, struct bufferevent *);

struct irc_command {
	const char *name;
	void (*callback)(struct msg *, struct bufferevent *);
} dispatch_table[] = {
	{"PRIVMSG", cmd_privmsg},
	{"PING", cmd_ping}
};
#define N_COMMANDS (sizeof(dispatch_table) / sizeof(dispatch_table[0]))

void
cmd_privmsg(struct msg *msg, struct bufferevent *bev)
{
	if (msg->nick == NULL || msg->trailing == NULL)
		return;

	printf("%s: %s\n", msg->nick, msg->trailing);
	bufferevent_printf(bev, "PRIVMSG %s :You said: %s\n",
	    msg->nick, msg->trailing);
}

void
cmd_ping(struct msg *msg, struct bufferevent *bev)
{
	char *cookie;

	cookie = msg->trailing;
	if (cookie == NULL)
		cookie = "";

	printf("PONGing (%s)\n", cookie);
	bufferevent_printf(bev, "PONG :%s\n", cookie);
}

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
			irc_msg_dispatch(msg, bev);
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

static int
irc_msg_dispatch(struct msg *msg, struct bufferevent *bev)
{
	struct irc_command *cmd;
	size_t i;

	msg_dump(msg);

	for (i = 0; i < N_COMMANDS; i++) {
		cmd = &dispatch_table[i];
		if (strcasecmp(msg->command, cmd->name) == 0) {
			cmd->callback(msg, bev);
			return 0;
		}
	}
	return -1;
}
