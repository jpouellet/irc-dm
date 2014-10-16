#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
read_cb(struct bufferevent *bev_in, void *ctx)
{
	struct evbuffer *buf;
	struct bufferevent *bev_out = ctx;
	char *line;
	size_t len;

	buf = bufferevent_get_input(bev_in);
	while ((line = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF)) != NULL) {
		len = strlen(line);
		line[len] = '\n';
		bufferevent_write(bev_out, line, len + 1);
		free(line);
	}
}

int
main(int argc, char *argv[])
{
	struct event_base *base;
	struct bufferevent *bev_in, *bev_out;

	base = event_base_new();
	if (base == NULL)
		errx(1, "event_base_new failed");

	bev_in = bufferevent_socket_new(base, STDIN_FILENO, BEV_OPT_CLOSE_ON_FREE);
	bev_out = bufferevent_socket_new(base, STDOUT_FILENO, BEV_OPT_CLOSE_ON_FREE);
	if (bev_in == NULL || bev_out == NULL)
		errx(1, "bufferevent_socket_new failed");

	bufferevent_setcb(bev_in, &read_cb, NULL, NULL, bev_out);

	if (bufferevent_enable(bev_in, EV_READ) == -1)
		errx(1, "bufferevent_enable failed");

	if (event_base_dispatch(base) == -1)
		errx(1, "event_base_dispatch failed");

	return 0;
}
