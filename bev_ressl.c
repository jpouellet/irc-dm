/*
 * Ugly hack to reach into libressl and pull out an openssl ctx and socket fd.
 */

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <ressl.h>
#include "libressl/ressl_internal.h"
#include "bev_ressl.h"

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
