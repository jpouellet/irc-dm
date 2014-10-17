#ifndef _IRC_H_
#define _IRC_H_

struct bufferevent;

int irc_login(struct bufferevent *, const char *, const char *, const char *);
void irc_bev_read_cb(struct bufferevent *, void *);
void irc_bev_event_cb(struct bufferevent *, short, void *);

#endif
