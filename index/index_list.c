#include <sys/queue.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "index.h"

struct record {
	SLIST_ENTRY(record) next;
	void *val;
	char key[];
};

#define KEY_OFFSET (offsetof(struct record, key))

struct record *
record_new(const char *key)
{
	struct record *new;
	size_t len;

	len = strlen(key) + 1;
	new = malloc(KEY_OFFSET + len);
	if (new == NULL)
		return NULL;

	(void)strcpy(new->key, key);
	return new;
}

SLIST_HEAD(index, record);

struct index *
index_new(void)
{
	struct index *new;

	new = malloc(sizeof(*new));
	if (new == NULL)
		return NULL;

	SLIST_INIT(new);
	return new;
}

int
index_put(struct index *idx, const char *key, void *val)
{
	struct record *new = NULL;

	if (index_get(idx, key, NULL) == 0)
		return 1;

	new = record_new(key);
	if (new == NULL)
		return -1;

	new->val = val;

	SLIST_INSERT_HEAD(idx, new, next);
	return 0;
}

int
index_get(struct index *idx, const char *key, void **val)
{
	struct record *p;

	SLIST_FOREACH(p, idx, next) {
		if (strcmp(key, p->key) == 0) {
			if (val)
				*val = p->val;
			return 0;
		}
	}

	return 1;
}

int
index_del(struct index *idx, const char *key)
{
	struct record *last = NULL, *p;
	int found = 0;

	SLIST_FOREACH(p, idx, next) {
		if (strcmp(key, p->key) == 0) {
			found = 1;
			break;
		}
		last = p;
	}

	if (found) {
		if (last == NULL)
			SLIST_REMOVE_HEAD(idx, next);
		else
			SLIST_REMOVE_AFTER(last, next);
		free(p);
		return 0;
	}

	return 1;
}
