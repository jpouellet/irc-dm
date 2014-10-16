#ifndef _BEV_RESSL_H_
#define _BEV_RESSL_H_

struct bufferevent;
struct event_base;
struct ressl;

struct bufferevent *
bufferevent_ressl_new(struct event_base *, struct ressl *, int);

void bufferevent_ressl_shutdown_and_free(struct bufferevent *);

#endif
