/*
 * Ugly hack to reach into libressl and pull out an openssl ctx and socket fd.
 */

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>

#include <ressl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libressl/ressl_internal.h"
#include "bev_util.h"

struct bufferevent *
bufferevent_ressl_new(
    struct event_base *base,
    struct ressl *ctx,
    int options)
{
	if (ctx == NULL || ctx->socket == -1 || ctx->ssl_conn == NULL)
		return NULL;

	return bufferevent_openssl_socket_new(
	    base, ctx->socket, ctx->ssl_conn,
	    BUFFEREVENT_SSL_OPEN, options);
}

/*
 * Hack to do clean shutdowns cause BEV_OPT_CLOSE_ON_FREE doesn't.
 * XXX Should be removed when bufferevent_shutdown() is finished.
 * http://www.wangafu.net/~nickm/libevent-book/Ref6a_advanced_bufferevents.html
 */
void
bufferevent_ressl_shutdown_and_free(struct bufferevent *bev)
{
	SSL *ctx;

	ctx = bufferevent_openssl_get_ssl(bev);
	if (ctx != NULL)
		SSL_set_shutdown(ctx, SSL_RECEIVED_SHUTDOWN);
	bufferevent_free(bev);
}

int
bufferevent_printf(struct bufferevent *bev, const char *fmt, ...)
{
	va_list ap;
	char *str = NULL;
	int ret;

	va_start(ap, fmt);
	vasprintf(&str, fmt, ap);
	va_end(ap);

	if (str == NULL)
		return -1;

	ret = bufferevent_write(bev, str, strlen(str));
	free(str);
	return (ret == 0 ? 0 : -1);
}
