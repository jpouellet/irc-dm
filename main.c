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
#include "irc.h"
#include "msg.h"
#include "util.h"

void
usage(void)
{
	fprintf(stderr, "Usage: %s [-s ca_file] [-u username] [-r realname] "
	    "host port nick\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct ressl_config *config;
	struct ressl *ctx;
	struct event_base *base;
	struct bufferevent *bev;
	char *ca_file = NULL, *username = NULL, *realname = NULL;
	char *host, *port, *nick;
	int ch;

	while ((ch = getopt(argc, argv, "s:u:r:")) != -1) {
		switch (ch) {
		case 's':
			ca_file = optarg;
			break;
		case 'u':
			username = optarg;
			break;
		case 'r':
			realname = optarg;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 3)
		usage();

	host = argv[0];
	port = argv[1];
	nick = argv[2];

	if (username == NULL)
		username = nick;
	if (realname == NULL)
		realname = nick;

	if (util_init() == -1)
		errx(1, "util_init failed");

	if (msg_parser_init() == -1)
		errx(1, "msg_parser_init failed");

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

	bev = bufferevent_ressl_new(base, ctx,
	    BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
	if (bev == NULL)
		errx(1, "bufferevent_ressl_new failed");

	bufferevent_setcb(bev, &irc_bev_read_cb, NULL, irc_bev_event_cb,
	    NULL);

	if (bufferevent_enable(bev, EV_READ) == -1)
		errx(1, "bufferevent_enable for ressl failed");

	if (irc_login(bev, nick, username, realname) == -1)
		errx(1, "irc_login failed");

	if (event_base_dispatch(base) == -1)
		errx(1, "event_base_dispatch failed");

	printf("done\n");
	return 0;
}
