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

msg.c: regex/message.h
	@touch msg.c

regex:
	@$(CC) regex_bnf.c -o regex_bnf
	@./regex_bnf
	@rm -f regex_bnf

regex-c:
	@$(CC) regex_bnf.c -DESCAPE -o regex_bnf
	@./regex_bnf
	@rm -f regex_bnf

regex/message.h: regex_bnf.c
	@mkdir -p regex
	$(CPP) -DJUST_PREPROCESS $< | tail -1 > $@

regex/nickname.h: regex_bnf.c
	@mkdir -p regex
	$(CPP) -DJUST_PREPROCESS -DTERMINAL=nickname $< | tail -1 > $@

clean:
	rm -f msg event event_cat ssl ressl bev_ressl.o not_line
	rm -rf regex/ *.dSYM
