CFLAGS=-Wall -Wextra -std=c89 -pedantic -Wno-unused-parameter

.PHONY: clean regex regex-c

event: LDFLAGS=-levent
ssl: LDFLAGS=-levent -lssl # -levent_openssl
msg: CFLAGS+=-DTEST

all: ssl event msg

ssl:

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
	rm -f msg_regex.h msg event ssl
