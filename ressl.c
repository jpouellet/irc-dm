#include <sys/types.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <ressl.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bev_ressl.h"

void
read_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *buf;
	char *line;

	buf = bufferevent_get_input(bev);
	line = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF);
	if (line)
		printf("%s\n", line);
	else
		printf("(empty)\n");
	free(line);
}

void
event_cb(struct bufferevent *bev, short what, void *ctx)
{
	if (what & BEV_EVENT_CONNECTED)
		printf("connected\n");

	if (what & BEV_EVENT_EOF)
		errx(0, "EOF");

	if (what & BEV_EVENT_TIMEOUT)
		errx(1, "bufferevent timeout");

	if (what & BEV_EVENT_READING)
		errx(1, "bufferevent error while reading");
	if (what & BEV_EVENT_WRITING)
		errx(1, "bufferevent error while writing");
	if (what & BEV_EVENT_ERROR)
		errx(1, "bufferevent error");
}

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-s ca_file] host port\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct ressl_config *config;
	struct ressl *ctx;
	struct event_base *base;
	struct bufferevent *bufev;
	char *ca_file = NULL, *host, *port;
	int ch;

	while ((ch = getopt(argc, argv, "s:")) != -1) {
		switch (ch) {
		case 's':
			if (ca_file != NULL) {
				warnx("duplicate ca_file");
				usage();
			}
			ca_file = strdup(optarg);
			if (ca_file == NULL)
				err(1, "strdup ca_file");
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	host = argv[0];
	port = argv[1];

	if (ressl_init() == -1)
		errx(1, "ressl_init failed");

	config = ressl_config_new();
	if (config == NULL)
		errx(1, "ressl_config_new failed");

	if (ressl_config_set_ca_file(config, ca_file) == -1)
		errx(1, "ressl_config_set_ca_file failed");

	ctx = ressl_client();
	if (ctx == NULL)
		errx(1, "ressl_client failed");

	if (ressl_configure(ctx, config) == -1)
		errx(1, "ressl_configure: %s", ressl_error(ctx));

	if (ressl_connect(ctx, host, port) == -1)
		errx(1, "ressl_connect: %s", ressl_error(ctx));

	base = event_base_new();
	if (base == NULL)
		errx(1, "event_base_new failed");

	bufev = bufferevent_ressl_new(base, ctx, BEV_OPT_CLOSE_ON_FREE);
	if (bufev == NULL)
		errx(1, "bufferevent_ressl_new failed");

	bufferevent_setcb(bufev, &read_cb, NULL, event_cb, NULL);

	if (bufferevent_enable(bufev, EV_READ) == -1)
		errx(1, "bufferevent_enable failed");

	if (event_base_dispatch(base) == -1)
		errx(1, "event_base_dispatch failed");

	printf("done\n");
	return 0;
}
