-include config.mk

CFLAGS+=-Wall -Wextra -std=c89 -pedantic
CFLAGS+=-Wno-unused-parameter -Wno-long-long

.PHONY: clean regex regex-c

event event_cat: LDFLAGS+=-levent_core
irc-dm ssl ressl bev_util.o: LDFLAGS+=-levent_core -levent_openssl -lssl -lcrypto
irc-dm ressl: LDFLAGS+=-lressl
msg_test: CPPFLAGS+=-DTEST
regex regex-c: TERMINAL?=message

all: irc-dm ressl event event_cat msg_test util_test not_line

irc-dm: main.o irc.o msg.o util.o bev_util.o
	$(CC) $(LDFLAGS) -o $@ $^

ressl: bev_util.o

msg.c: regex/message.h
	@touch msg.c

util.c: regex/nickname.h
	@touch util.c

msg_test: msg.o

util_test: util.o

regex:
	@$(CC) -DTERMINAL=$(TERMINAL) regex_bnf.c -o regex_bnf
	@./regex_bnf
	@rm -f regex_bnf

regex-c:
	@$(CC) -DTERMINAL=$(TERMINAL) regex_bnf.c -DESCAPE -o regex_bnf
	@./regex_bnf
	@rm -f regex_bnf

regex/message.h: regex_bnf.c
	@mkdir -p regex
	$(CPP) -DJUST_PREPROCESS $< | tail -1 > $@

regex/nickname.h: regex_bnf.c
	@mkdir -p regex
	$(CPP) -DJUST_PREPROCESS -DTERMINAL=nickname $< | tail -1 > $@

clean:
	rm -f msg msg_test util_test event event_cat ssl ressl not_line \
	    irc-dm *.o
	rm -rf regex/ *.dSYM
