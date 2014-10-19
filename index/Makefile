-include ../config.mk

CFLAGS+=-Wall -Wextra -std=c99 -pedantic
CFLAGS+=-Wno-unused-parameter -Wno-unused-function
CFLAGS+=-O2
CFLAGS+=-g

TREE_HDRS= ../openbsd_sys/tree.h
TREE_OBJS= index_splay.o index_rb.o
INDEX_OBJS=	index_db_btree.o index_db_hash.o \
		index_list.o index_list_front.o \
		index_ohash.o $(TREE_OBJS)
OBJS=	$(INDEX_OBJS) benchmark.o trace.o

TRACE_FILE=benchmark.trace

BENCHMARKS= $(patsubst index_%.o,benchmark_%,$(INDEX_OBJS))

index_splay.o: CPPFLAGS+=-D"T(x)=T_(x,SPLAY)"
index_rb.o: CPPFLAGS+=-D"T(x)=T_(x,RB)"
index_db_btree.o: CPPFLAGS+=-DMY_DB_TYPE=DB_BTREE
index_db_hash.o: CPPFLAGS+=-DMY_DB_TYPE=DB_HASH
benchmark_ohash: LDFLAGS+=-lutil
bench: SECS_EACH?=1

all: $(OBJS) $(BENCHMARKS)

objs: $(OBJS)

trace $(TRACE_FILE): tracegen.sh
	./tracegen.sh 100 50 10 5 500 30 1 > $(TRACE_FILE)

bench: $(BENCHMARKS) $(TRACE_FILE)
	@for benchmark in $(BENCHMARKS); do \
	  ./$$benchmark -q -t $(SECS_EACH) $(TRACE_FILE); \
	done

benchmark_%: index_%.o benchmark.o trace.o
	$(CC) $(LDFLAGS) -o $@ $^

index_db_btree.o index_db_hash.o: index_db.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TREE_OBJS): index_tree.c $(TREE_HDRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

index_*.c: index.h
	@touch $@

benchmark.c trace.c: trace.h
	@touch $@

clean:
	rm -f $(OBJS) $(BENCHMARKS) *.core
