#ifndef _INDEX_H_
#define _INDEX_H_

struct index;

struct index * index_new(void);
int index_put(struct index *, const char *, void *);
int index_get(struct index *, const char *, void **);
int index_del(struct index *, const char *);

#endif
