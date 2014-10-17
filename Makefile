-include config.mk

CFLAGS+=-Wall -Wextra -std=c89 -pedantic
CFLAGS+=-Wno-unused-parameter -Wno-long-long

.PHONY: clean regex regex-c

event event_cat: LDFLAGS+=-levent_core
ssl ressl bev_ressl.o: LDFLAGS+=-levent_core -levent_openssl -lssl -lcrypto
ressl: LDFLAGS+=-lressl
msg: CFLAGS+=-DTEST

all: ressl event event_cat msg not_line

ressl: bev_ressl.o

msg.c: msg_regex.h
	@touch msg.c

regex:
	@$(CC) msg_regex_bnf.c -o msg_regex_bnf
	@./msg_regex_bnf
	@rm -f msg_regex_bnf

regex-c:
	@$(CC) msg_regex_bnf.c -DESCAPE -o msg_regex_bnf
	@./msg_regex_bnf
	@rm -f msg_regex_bnf

msg_regex.h: msg_regex_bnf.c
	$(CPP) -DJUST_PREPROCESS $< | tail -1 > $@

clean:
	rm -f msg_regex.h msg event event_cat ssl ressl bev_ressl.o not_line
	rm -rf *.dSYM
