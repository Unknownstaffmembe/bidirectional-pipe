#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "bdpipe.h"

/* __bdopen shall open a bidirectional pipe, stdin on the child process can be
 * written to and stdout of the child process can be read from
 *
 * bd = pointer to bidirectional pipe struct
 * cmd = NULL termined string that is a command
 *
 * returns 0 on success
 * returns -1 on faliure
 */
int __bdopen(struct bdpipe *bd, char *cmd, char **argv)
{
	int pid;
	int in[2];
	int out[2];
	int ret;

	ret = pipe(in);
	if (ret == -1)
		return -1;
	ret = pipe(out);
	if (ret == -1) {
		close(in[0]);
		close(in[1]);
		return -1;
	}

	pid = fork();
	if (pid == 0) {
		dup2(in[0], 0);
		dup2(out[1], 1);
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		execvp(cmd, argv);
		_exit(-1); /* execvp doesn't return if it's successful,
			      if it fails, we'll call _exit */
	} else {
		if (pid == -1) return -1;
		bd->in = in[1];
		bd->out = out[0];
		bd->pid = pid;
		close(in[0]);
		close(out[1]);
	}

	return 0;
}

/* bdread shall read from a bidirectional pipe and return the number of bytes
 * read, on faliure, bdread shall return -1
 *
 * bd = pointer to bd struct
 * buff = buffer to read to
 * nb = size of buffer
 *
 * returns bytes read on success
 * returns -1 on faliure
 */
ssize_t bdread(struct bdpipe *bd, char buff[], size_t nb)
{
	ssize_t ret;

	ret = read(bd->out, buff, nb);
	if (ret == -1) return -1;

	return ret;
}

/* bdwrite shall write to a bidirectional pipe and return the number of bytes
 * written, on faliure, bdread shall return -1
 *
 * bd = pointer to bd struct
 * buff = buffer to read from
 * nb = number of bytes to read from buffer
 *
 * returns bytes written on success
 * returns -1 on faliure
 */
ssize_t bdwrite(struct bdpipe *bd, char *buff, size_t nb)
{
	ssize_t nb2; /* number of bytes that can be written (for when write()
			doesn't somehow write everything to a pipe) */
	ssize_t ret;

	nb2 = nb;
	while ((ret = write(bd->in, buff, nb2)) > 0) {
		nb2 -= ret;
		if (nb2 == 0) break;
		buff += ret;
	}
	if (ret == -1) return -1;

	return nb - nb2;
}

/* bdclose shall close an open bidirectional pipe
 *
 * bd = pointer to bd struct
 */
void bdclose(struct bdpipe *bd)
{
	kill(bd->pid, SIGKILL);
	close(bd->in);
	close(bd->out);
}
