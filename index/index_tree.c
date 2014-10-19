#ifndef T
#error no tree type specified: pass -DT(x)=_T(x,{RB,SPLAY})
#endif

#define T_(x,t) t ## _ ## x

#include "openbsd_sys/tree.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "index.h"

struct node {
	T(ENTRY)(node) entry;
	const char *key;
	void *val;
};

int
nodecmp(const struct node *e1, const struct node *e2)
{
	return strcmp(e1->key, e2->key);
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
	struct node *new = NULL, *old;
	char *keydup = NULL;
	int rv = -1;

	new = malloc(sizeof(*new));
	if (new == NULL)
		goto fail;

	keydup = strdup(key);
	if (keydup == NULL)
		goto fail;

	new->key = keydup;
	new->val = val;

	old = T(INSERT)(index, idx, new);
	if (old != NULL) {
		rv = 1;
		goto fail;
	}
	return 0;

fail:
	free(new);
	free(keydup);
	return rv;
}

int
index_get(struct index *idx, const char *key, void **val)
{
	struct node find, *res;

	find.key = key;
	res = T(FIND)(index, idx, &find);
	if (res == NULL)
		return 1;

	if (val)
		*val = res->val;
	return 0;
}

int
index_del(struct index *idx, const char *key)
{
	struct node search, *found, *removed;

	search.key = key;
	found = T(FIND)(index, idx, &search);
	if (found == NULL)
		return 1;

	removed = T(REMOVE)(index, idx, found);
	assert(removed == found);

	free((void *)removed->key);
	free(removed);
	return 0;
}
