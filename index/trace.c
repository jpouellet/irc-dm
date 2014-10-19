#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "trace.h"

int
__trace_iterate(const struct trace *t, const char **p)
{
	if (*p == NULL) {
		*p = (const char *)t;
		return 1;
	}

	*p = strchr(*p, '\0') + 1;
	if (*p == '\0')
		return 0;
	return 1;
}

struct trace *
trace_load(const char *path)
{
	struct stat st;
	char *buf = NULL;;
	char *s, *e;
	ssize_t len;
	size_t line;
	int fd = -1;

	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd == -1)
		goto fail;

	if (fstat(fd, &st) == -1)
		goto fail;

	buf = malloc(st.st_size + 2);
	if (buf == NULL)
		goto fail;

	len = read(fd, buf, st.st_size);
	if (len == -1)
		goto fail;

	close(fd);

	/*
	 * Transform to multiple consecutive null-terminated strings, split
	 * on each line. Empty lines are squeezed out. The sequence is
	 * terminated by an empty string (double NUL).
	 */
	line = 0;
	for (s = e = buf; e < buf + len; e++) {
		if (*e == '\r' || *e == '\0')
			continue;
		if (*e == '\n') {
			if (line == 0)
				continue;
			line = 0;
			*s++ = '\0';
			continue;
		}
		line++;
		*s++ = *e;
	}
	*s++ = '\0';
	*s = '\0';

	return (struct trace *)buf;

fail:
	if (fd != -1)
		close(fd);
	free(buf);
	return NULL;
}

void
trace_free(struct trace *t)
{
	free(t);
}
