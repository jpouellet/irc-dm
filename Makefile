include config.mk

CFLAGS+=-Wall -Wextra -std=c89 -pedantic
CFLAGS+=-Wno-unused-parameter -Wno-long-long

.PHONY: clean regex regex-c

event: LDFLAGS+=-levent
ssl: LDFLAGS+=-levent -lssl -levent_openssl
ressl bev_ressl.o: LDFLAGS+=-levent -levent_openssl -lssl -lcrypto
msg: CFLAGS+=-DTEST

all: ressl event msg

ressl: bev_ressl.o ../libressl/libressl.a

bev_ressl.o:

event:

msg:

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
	rm -f msg_regex.h msg event ssl ressl bev_ressl.o
