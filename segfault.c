#include <sys/types.h>
#include <sys/socket.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
send_line(struct bufferevent *to, char *line)
{
	size_t len;

	len = strlen(line) + 1;
	line[len - 1] = '\n';
	bufferevent_write(to, line, len);
}

void
read_cb(struct bufferevent *bev, void *ctx)
{
	struct evbuffer *buf;
	struct bufferevent *other = ctx;
	char *s;

	buf = bufferevent_get_input(bev);
	while ((s = evbuffer_readln(buf, NULL, EVBUFFER_EOL_CRLF)) != NULL) {
		if (other == NULL)
			printf("%s\n", s);
		else
			send_line(other, s);
		free(s);
	}
}

void
event_cb(struct bufferevent *bev, short what, void *ctx)
{
	printf("%s event: %hd\n", ctx ? "stdin" : "ssl", what);
}

int
do_connect(const char *host, const char *port)
{
	struct addrinfo hints, *res, *res0;
	int sock = -1, ret;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(host, port, &hints, &res0)) != 0)
		errx(1, "%s", gai_strerror(ret));
	for (res = res0; res; res = res->ai_next) {
		sock = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (sock == -1)
			err(1, "socket");
		if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
			close(sock);
			sock = -1;
			continue;
		}
		break;  /* Connected. */
	}
	freeaddrinfo(res0);

	return sock;
}

#if 0
int
ressl_connect_socket(struct ressl *ctx, int socket, const char *hostname)
{
	union { struct in_addr ip4; struct in6_addr ip6; } addrbuf;
	X509 *cert = NULL;
	int ret;

	if ((ctx->flags & RESSL_CLIENT) == 0) {
		ressl_set_error(ctx, "not a client context");
		goto err;
	}

	ctx->socket = socket;

	if ((ctx->ssl_ctx = SSL_CTX_new(SSLv23_client_method())) == NULL) {
		ressl_set_error(ctx, "ssl context failure");
		goto err;
	}

	if (ressl_configure_ssl(ctx) != 0)
		goto err;

	if (ctx->config->verify_host) {
		if (hostname == NULL) {
			ressl_set_error(ctx, "server name not specified");
			goto err;
		}
	}

	if (ctx->config->verify_cert) {
		SSL_CTX_set_verify(ctx->ssl_ctx, SSL_VERIFY_PEER, NULL);

		if (SSL_CTX_load_verify_locations(ctx->ssl_ctx,
		    ctx->config->ca_file, ctx->config->ca_path) != 1) {
			ressl_set_error(ctx, "ssl verify setup failure");
			goto err;
		}
		if (ctx->config->verify_depth >= 0)
			SSL_CTX_set_verify_depth(ctx->ssl_ctx,
			    ctx->config->verify_depth);
	}

	if ((ctx->ssl_conn = SSL_new(ctx->ssl_ctx)) == NULL) {
		ressl_set_error(ctx, "ssl connection failure");
		goto err;
	}
	if (SSL_set_fd(ctx->ssl_conn, ctx->socket) != 1) {
		ressl_set_error(ctx, "ssl file descriptor failure");
		goto err;
	}

	/*
	 * RFC4366 (SNI): Literal IPv4 and IPv6 addresses are not
	 * permitted in "HostName".
	 */
	if (hostname != NULL &&
	    inet_pton(AF_INET, hostname, &addrbuf) != 1 &&
	    inet_pton(AF_INET6, hostname, &addrbuf) != 1) {
		if (SSL_set_tlsext_host_name(ctx->ssl_conn, hostname) == 0) {
			ressl_set_error(ctx, "SNI host name failed");
			goto err;
		}
	}

	if ((ret = SSL_connect(ctx->ssl_conn)) != 1) {
		ressl_set_error(ctx, "SSL connect failed: %i",
		    SSL_get_error(ctx->ssl_conn, ret));
		goto err;
	}

	if (ctx->config->verify_host) {
		cert = SSL_get_peer_certificate(ctx->ssl_conn);
		if (cert == NULL) {
			ressl_set_error(ctx, "no server certificate");
			goto err;
		}
		if (ressl_check_hostname(cert, hostname) != 0) {
			ressl_set_error(ctx, "host `%s' not present in"
			    " server certificate", hostname);
			goto err;
		}
	}

	return (0);

err:
	X509_free(cert);

	return (-1);
}
#endif

int
main(int argc, char *argv[])
{
	SSL_CTX *ctx;
	SSL *ssl;
	int sock, ret;
	struct event_base *base;
	struct bufferevent *bev;
	struct bufferevent *bev_stdin;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s host port\n", getprogname());
		return 1;
	}

	SSL_load_error_strings();
	SSL_library_init();

	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL)
		errx(1, "SSL_CTX_new");

	ssl = SSL_new(ctx);
	if (ssl == NULL)
		errx(1, "SSL_new");

	sock = do_connect(argv[1], argv[2]);
	if (sock == -1)
		err(1, "connect");

	if (SSL_set_fd(ssl, sock) == -1)
		err(1, "SSL_set_fd");

	if ((ret = SSL_connect(ssl)) != 1)
		errx(1, "SSL_connect: %d", SSL_get_error(ssl, ret));

	base = event_base_new();
	if (base == NULL)
		errx(1, "event_base_new");

	bev = bufferevent_openssl_socket_new(base, sock, ssl,
	    BUFFEREVENT_SSL_OPEN, 0);
	if (bev == NULL)
		errx(1, "bufferevent_openssl_socket_new");

	bev_stdin = bufferevent_socket_new(base, STDIN_FILENO, 0);
	if (bev_stdin == NULL)
		errx(1, "bufferevent_socket_new for stdin");

	bufferevent_setcb(bev, &read_cb, NULL, &event_cb, NULL);
	bufferevent_setcb(bev_stdin, &read_cb, NULL, &event_cb, bev);

	if (bufferevent_enable(bev, EV_READ) == -1)
		errx(1, "bufferevent_enable ssl");

	if (bufferevent_enable(bev_stdin, EV_READ) == -1)
		errx(1, "bufferevent_enable stdin");

	if (event_base_dispatch(base) == -1)
		errx(1, "event_base_dispatch");

	return 0;
}
