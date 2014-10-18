#include <assert.h>
#include <stddef.h>

#include "index.h"

int
main()
{
	char *foo = "asdf";
	char *bar = NULL;
	struct index *i;

	assert((i = index_new()) != NULL);
//	assert(index_get(i, "key", NULL) == 1);
	assert(index_put(i, "key", foo) == 0);
	assert(index_get(i, "key", (void **)&bar) == 0);
	assert(bar == foo);
//	assert(index_put(i, "key", NULL) == 1);
//	assert(index_get(i, "key", (void **)&bar) == 0);
//	assert(bar == foo);
	assert(index_del(i, "key") == 0);
//	assert(index_get(i, "key", NULL) == 1);

	return 0;
}
