CFLAGS=-Wall -Wextra -std=c89 -pedantic

.PHONY: clean regex regex-c

msg: msg.c

msg.c: msg_regex.h
	@touch msg.c

msg_regex.h: msg_regex_bnf.c
	cc -DJUST_PREPROCESS -E $< | tail -1 > $@

clean:
	rm -f msg_regex.h msg
