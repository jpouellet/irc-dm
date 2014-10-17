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

#include "bev_util.h"

void
stdin_read_cb(struct bufferevent *bev, void *ctx)
{
	struct bufferevent *ressl = ctx;
	struct evbuffer *buf;
	char *line;
	size_t len;

	buf = bufferevent_get_input(bev);
	while ((line = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF)) != NULL) {
		len = strlen(line) + 1;
		line[len - 1] = '\n';
		bufferevent_write(ressl, line, len);
		free(line);
	}
}

void
ressl_read_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *buf;
	char *line;

	buf = bufferevent_get_input(bev);
	while ((line = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF)) != NULL) {
		printf("%s\n", line);
		free(line);
	}
}

void
stdin_event_cb(struct bufferevent *bev, short what, void *ctx)
{
	if (what & BEV_EVENT_EOF) {
		bufferevent_disable(bev, EV_READ);
		event_base_loopbreak(bufferevent_get_base(bev));
	}

	if (what & BEV_EVENT_ERROR) {
		bufferevent_disable(bev, EV_READ);
		warnx("bufferevent error for stdin");
		event_base_loopbreak(bufferevent_get_base(bev));
	}
}

void
ressl_event_cb(struct bufferevent *bev, short what, void *ctx)
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
	struct bufferevent *bev_stdin, *bev_ressl;
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

	bev_stdin = bufferevent_socket_new(base, STDIN_FILENO,
	    BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
	if (bev_stdin == NULL)
		errx(1, "bufferevent_socket_new for stdin failed");

	bev_ressl = bufferevent_ressl_new(base, ctx,
	    BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
	if (bev_ressl == NULL)
		errx(1, "bufferevent_ressl_new failed");

	bufferevent_setcb(bev_stdin, &stdin_read_cb, NULL, stdin_event_cb,
	    bev_ressl);
	bufferevent_setcb(bev_ressl, &ressl_read_cb, NULL, ressl_event_cb,
	    NULL);

	if (bufferevent_enable(bev_stdin, EV_READ) == -1)
		errx(1, "bufferevent_enable for stdin failed");

	if (bufferevent_enable(bev_ressl, EV_READ) == -1)
		errx(1, "bufferevent_enable for ressl failed");

	if (event_base_dispatch(base) == -1)
		errx(1, "event_base_dispatch failed");

	printf("done\n");
	return 0;
}
