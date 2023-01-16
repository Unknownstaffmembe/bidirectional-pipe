#ifndef BDPIPE_INCLUDED_H
#define BDPIPE_INCLUDED_H
#include <unistd.h>
#include <fcntl.h>

/* bidirectional pipe */
struct bdpipe {
	int in; /* file descriptor which will be used to write to stdin of
		   child process */
	int out; /* file descriptor which will be used to read from stdout of
		    child process */
	pid_t pid; /* child process id */
};

extern int __bdopen(struct bdpipe *bd, char *cmd, char **argv);
extern ssize_t bread(struct bdpipe *bd, char buff[], size_t nb);
extern ssize_t bdwrite(struct bdpipe *bd, char *buff, size_t nb);
extern void bdclose(struct bdpipe *bd);

#define bdopen(bd, cmd, ...) ({ \
	char *argv[] = {cmd, ##__VA_ARGS__, NULL}; \
	__bdopen(bd, cmd, argv); })

/* bdopen2 shall open a bidirectional pipe with nonblocking file descriptors */
#define bdopen2(bd, cmd, ...) ({ \
		int ret = bdopen(bd, cmd, ##__VA_ARGS__); \
		if (ret == 0) { \
			fcntl((bd)->in, F_SETFL, O_NONBLOCK); \
			fcntl((bd)->out, F_SETFL, O_NONBLOCK); \
		} \
		ret;})

#endif
