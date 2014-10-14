.PHONY: clean

msg_regex.h: msg_regex_bnf.c
	cc -DJUST_PREPROCESS -E $< | tail -1 > $@

clean:
	rm -f msg_regex.h
