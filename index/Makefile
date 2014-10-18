-include ../config.mk

CFLAGS+=-Wall -Wextra -std=c99 -pedantic -Wno-unused-parameter -Wno-unused-function

SPLAY_TREE_OBJS= index_splay.o index_splay_record.o
RB_TREE_OBJS= index_rb.o index_rb_record.o
TREE_OBJS= $(SPLAY_TREE_OBJS) $(RB_TREE_OBJS)
OBJS=	index_db_btree.o index_db_hash.o \
	index_list.o index_ohash.o \
	$(TREE_OBJS)

$(SPLAY_TREE_OBJS): CPPFLAGS+=-D"T(x)=T_(x,SPLAY)"
$(RB_TREE_OBJS): CPPFLAGS+=-D"T(x)=T_(x,RB)"
index_db_btree.o: CPPFLAGS+=-DMY_DB_TYPE=DB_BTREE
index_db_hash.o: CPPFLAGS+=-DMY_DB_TYPE=DB_HASH

all: $(OBJS)

index_db_btree.o index_db_hash.o: index_db.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TREE_OBJS): index_tree.c openbsd_sys/tree.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

index_*.c: index.h
	@touch $@

clean:
	rm -f $(OBJS) index_splay.c index_rb.c
