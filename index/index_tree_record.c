#ifndef T
#error no tree type specified: pass -DT(x)=_T(x,{RB,SPLAY})
#endif

#define T_(x,t) t ## _ ## x

#include "openbsd_sys/tree.h"

#include <stdlib.h>
#include <string.h>

#include "index.h"

struct record {
	T(ENTRY)(node) entry;
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

	(void)strcpy(record->key, key);
	return record;
}

int
record_cmp(const struct record *r1, const struct record *r2)
{
	return strcmp(r1->key, r2->key);
}

T(HEAD)(index, node);
T(PROTOTYPE)(index, node, entry, nodecmp)
T(GENERATE)(index, node, entry, nodecmp)

struct index *
index_new(void)
{
	struct index *new = NULL;

	new = malloc(sizeof(*new));
	if (new == NULL)
		return NULL;

	T(INIT)(new);
	return new;
}

int
index_put(struct index *idx, const char *key, void *val)
{
	struct record *new = NULL, *old;
	int rv = -1;

	new = record_new(key);
	if (new == NULL)
		return -1;

	new->val = val;

	old = T(INSERT)(index, idx, new);
	if (old != NULL) {
		free(new)
		return 1;
	}
	return 0;
}

int
index_get(struct index *idx, const char *key, void **val)
{
	struct node *res;

	/* XXX will this always be safe? or should i create an actual elm */
	res = T(FIND)(index, idx, key - KEY_OFFSET);
	if (res == NULL)
		return 1;

	if (val)
		*val = res->val;
	return 0;
}

int
index_del(struct index *idx, const char *key)
{
	struct node find, *removed;

	find.key = key;
	removed = T(REMOVE)(index, idx, &find);
	if (removed == NULL)
		return 1;

	free(removed);
	return 0;
}
