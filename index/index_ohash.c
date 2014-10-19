#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <ohash.h>

#include "index.h"

#define OHASH_INITIAL_LOG_SIZE (7)

static void * hash_calloc(size_t, size_t, void *);
static void * hash_malloc(size_t, void *);
static void hash_free(void *, void *);

struct index {
	struct ohash h;
	bool read_only;
};

struct my_incomplete_type;

struct record {
	void *val;
	char key[];
};

static struct ohash_info info = {
	.key_offset = offsetof(struct record, key),
	.data = NULL,
	.calloc = &hash_calloc,
	.free = &hash_free,
	.alloc = &hash_malloc
};

static void *
hash_calloc(size_t nmemb, size_t size, void *data)
{
	struct index *idx = data;
	void *ptr;

	ptr = calloc(nmemb, size);
	if (ptr == NULL)
		idx->read_only = true;
	return ptr;
}

static void
hash_free(void *ptr, void *data)
{
	free(ptr);
}

static void *
hash_malloc(size_t size, void *data)
{
	return malloc(size);
}

struct index *
index_new(void)
{
	struct index *new;

	new = malloc(sizeof(*new));
	if (new == NULL)
		return NULL;

	new->read_only = false;
	info.data = new;
	ohash_init(&new->h, OHASH_INITIAL_LOG_SIZE, &info);
	return new;
}

int
index_put(struct index *idx, const char *key, void *val)
{
	struct record *rec;
	const char *end = NULL;
	unsigned int slot;

	if (idx->read_only)
		return -1;

	slot = ohash_qlookup(&idx->h, key);
	if (ohash_find(&idx->h, slot) != NULL)
		return 1;

	rec = ohash_create_entry(&info, key, &end);
	if (rec == NULL)
		return -1;

	rec->val = val;
	ohash_insert(&idx->h, slot, rec);
	return 0;
}

int
index_get(struct index *idx, const char *key, void **val)
{
	struct record *rec;
	unsigned int slot;

	slot = ohash_qlookup(&idx->h, key);
	rec = ohash_find(&idx->h, slot);
	if (rec == NULL)
		return 1;

	if (val)
		*val = rec->val;
	return 0;
}

int
index_del(struct index *idx, const char *key)
{
	struct record *rec;
	unsigned int slot;

	if (idx->read_only)
		return -1;

	slot = ohash_qlookup(&idx->h, key);
	rec = ohash_remove(&idx->h, slot);
	if (rec == NULL)
		return 1;

	free(rec);
	return 0;
}
