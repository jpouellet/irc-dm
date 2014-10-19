#ifndef _TRACE_H_
#define _TRACE_H_

struct trace;

int __trace_iterate(const struct trace *, const char **);
const struct trace * trace_load(const char *);
void trace_free(const struct trace *);

#define trace_foreach(t,p) for ((p) = NULL; __trace_iterate((t), &(p)); )

#endif
