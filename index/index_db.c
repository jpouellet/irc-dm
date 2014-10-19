#ifndef MY_DB_TYPE
#error You must define MY_DB_TYPE (DB_HASH or DB_BTREE)
#endif

#include <sys/types.h>

#include <assert.h>
#include <db.h>
#include <fcntl.h>
#include <string.h>

#include "index.h"

#define FLAGS (O_CREAT | O_TRUNC | O_RDWR)

struct index *
index_new(void)
{
	return (struct index *)dbopen(NULL, FLAGS, 0, MY_DB_TYPE, NULL);
}

int
index_put(struct index *idx, const char *key, void *val)
{
	DBT dbt_key, dbt_val;
	DB *db = (DB *)idx;

	dbt_key.data = (void *)key;
	dbt_key.size = strlen(key);

	dbt_val.data = &val;
	dbt_val.size = sizeof(val);

	return db->put(db, &dbt_key, &dbt_val, R_NOOVERWRITE);
}

int
index_get(struct index *idx, const char *key, void **val)
{
	DBT dbt_key, dbt_val;
	DB *db = (DB *)idx;
	int ret;

	dbt_key.data = (void *)key;
	dbt_key.size = strlen(key);

	ret = db->get(db, &dbt_key, &dbt_val, 0);
	if (ret == 0) {
		assert(dbt_val.size == sizeof(val));
		if (val)
			*val = dbt_val.data;
	}
	return ret;
}

int
index_del(struct index *idx, const char *key)
{
	DBT dbt_key;
	DB *db = (DB *)idx;

	dbt_key.data = (void *)key;
	dbt_key.size = strlen(key);

	return db->del(db, &dbt_key, 0);
}
