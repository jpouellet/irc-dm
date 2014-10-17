#include <assert.h>
#include <regex.h>
#include <string.h>

#include "util.h"

static const char nick_regex[] = "^"
#include "regex/nickname.h"
"$";

static regex_t preg;

int
util_init(void)
{
	if (regcomp(&preg, nick_regex, REG_EXTENDED | REG_NOSUB) != 0)
		return -1;

	return 0;
}

int
is_valid_nick(const char *nick)
{
	return (regexec(&preg, nick, 0, NULL, 0) == 0);
}

int
is_valid_username(const char *user)
{
	const char *p;

	if (*user == ':' || *user == '\0')
		return 0;

	for (p = user; *p != '\0'; p++) {
		switch (*p) {
		case ' ': case '@': case '\r': case '\n':
			return 0;
		default:
			continue;
		}
	}

	return 1;
}

int
is_valid_realname(const char *real)
{
	if (*real == '\0')
		return 0;

	if (strchr(real, '\r') != NULL || strchr(real, '\n') != NULL)
		return 0;

	return 1;
}
