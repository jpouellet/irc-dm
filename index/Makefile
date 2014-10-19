-include ../config.mk

CFLAGS+=-Wall -Wextra -std=c99 -pedantic
CFLAGS+=-Wno-unused-parameter -Wno-unused-function
CFLAGS+=-g

TREE_HDRS= openbsd_sys/tree.h
SPLAY_TREE_OBJS= index_splay_detached.o index_splay_record.o
RB_TREE_OBJS= index_rb_detached.o index_rb_record.o
TREE_OBJS= $(SPLAY_TREE_OBJS) $(RB_TREE_OBJS)
INDEX_OBJS=	index_db_btree.o index_db_hash.o \
		index_list.o index_ohash.o \
		$(TREE_OBJS)
OBJS=	$(INDEX_OBJS) benchmark.o

BENCHMARKS= $(patsubst index_%.o,benchmark_%,$(INDEX_OBJS))

$(SPLAY_TREE_OBJS): CPPFLAGS+=-D"T(x)=T_(x,SPLAY)"
$(RB_TREE_OBJS): CPPFLAGS+=-D"T(x)=T_(x,RB)"
index_db_btree.o: CPPFLAGS+=-DMY_DB_TYPE=DB_BTREE
index_db_hash.o: CPPFLAGS+=-DMY_DB_TYPE=DB_HASH
benchmark_ohash: LDFLAGS+=-lutil

all: $(OBJS) $(BENCHMARKS)

objs: $(OBJS)

bench: $(BENCHMARKS)
	@for benchmark in $(BENCHMARKS); do ./$$benchmark; done

benchmark_%: index_%.o benchmark.o
	$(CC) $(LDFLAGS) -o $@ $^

index_db_btree.o index_db_hash.o: index_db.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

index_%_detached.o: index_tree.c $(TREE_HDRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

index_%_record.o: index_tree_record.c $(TREE_HDRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

index_*.c: index.h
	@touch $@

clean:
	rm -f $(OBJS) $(BENCHMARKS)
