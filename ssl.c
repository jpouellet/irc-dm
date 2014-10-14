#include <openssl/ssl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define REJECTED_SSL_PROTOS ( \
	    SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | \
	    SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 )

SSL_CTX *
get_ctx(const char *pkey, const char *cert_chain)
{
	SSL_CTX *ctx;

	SSL_load_error_strings()
	SSL_library_init();
	if (!RAND_poll())
		return NULL;

	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
		warnx("unable to get ssl context");
		return NULL;
	}

	SSL_CTX_set_options(ctx, REJECTED_SSL_PROTOS |
	    SSL_OP_TLSEXT_PADDING | SSL_OP_NO_COMPRESSION);

	return ctx;
}

void
shutdown_and_free(struct bufferevent *bev)
{
	SSL *ctx;

	ctx = bufferevent_openssl_get_ssl(bev);
	if (ctx != NULL)
		SSL_set_shutdown(ctx, SSL_RECEIVED_SHUTDOWN);
	bufferevent_free(bev);
}

void
read_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *buf;
	char *line;

	buf = bufferevent_get_input(bev);
	line = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF);
	if (line)
		printf("[%s]\n", line);
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
log_cb(int severity, const char *msg)
{
	warnx("%s", msg);
}

int
main(int argc, char *argv[])
{
	struct event_base *base;
	struct bufferevent *bufev;
	char *hostname;
	int port;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s host port\n", getprogname());
		return 1;
	}
	hostname = argv[1];
	port = atoi(argv[2]);

	event_set_log_callback(&log_cb);

	base = event_base_new();
	if (base == NULL)
		errx(1, "event_base_new failed");

	if (ssl) {
		bufev = bufferevent_openssl_socket_new(base, -1, ssl,
		    BUFFEREVENT_SSL_OPEN, BEV_OPT_CLOSE_ON_FREE);
	} else
		bufev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	if (bufev == NULL)
		errx(1, "bufferevent_socket_new failed");

	bufferevent_setcb(bufev, &read_cb, NULL, event_cb, NULL);

	if (bufferevent_socket_connect_hostname(bufev, NULL, AF_UNSPEC, hostname, port) == -1)
		errx(1, "bufferevent_socket_connect_hostname failed");

	if (bufferevent_enable(bufev, EV_READ) == -1)
		errx(1, "bufferevent_enable failed");

	if (event_base_dispatch(base) == -1)
		errx(1, "event_base_dispatch failed");

	printf("done\n");
	return 0;
}
