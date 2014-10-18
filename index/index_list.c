#include <sys/queue.h>

#include <stdlib.h>
#include <string.h>

#include "index.h"

struct node {
	SLIST_ENTRY(node) next;
	const char *key;
	void *val;
};

SLIST_HEAD(index, node);

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
	struct node *new = NULL;
	char *keydup = NULL;

	if (index_get(idx, key, NULL) == 0)
		return 1;

	keydup = strdup(key);
	if (keydup == NULL)
		goto fail;

	new = malloc(sizeof(*new));
	if (new == NULL)
		goto fail;

	new->key = keydup;
	new->val = val;

	SLIST_INSERT_HEAD(idx, new, next);
	return 0;

fail:
	free(keydup);
	free(new);
	return -1;
}

int
index_get(struct index *idx, const char *key, void **val)
{
	struct node *p;

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
	struct node *last = NULL, *p;
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
		free((void *)p->key);
		free(p);
		return 0;
	}

	return 1;
}
