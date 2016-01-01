/**
 * Copyright © 2015, 2016  Mattias Andrée <maandree@member.fsf.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef _DEFAULT_SOURCE
# define _DEFAULT_SOURCE
#endif
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/socket.h>



/**
 * The file descriptor for the socket.
 */
#define SOCK_FILENO  3

/**
 * The file descriptor for the state file.
 */
#define STATE_FILENO  4

/**
 * The file descriptor for the CLOCK_BOOTTIME timer.
 */
#define BOOT_FILENO  5

/**
 * The file descriptor for the CLOCK_REALTIME timer.
 */
#define REAL_FILENO  6

/**
 * The file descriptor for the lock file.
 */
#define LOCK_FILENO  7


/**
 * Command: queue a job.
 */
#define SAT_QUEUE  0

/**
 * Command: remove jobs.
 */
#define SAT_REMOVE  1

/**
 * Command: print job queue.
 */
#define SAT_PRINT  2

/**
 * Command: run jobs.
 */
#define SAT_RUN  3



#ifndef t
/**
 * Go to `fail` if a statement evaluates to non-zero.
 * 
 * @param  ...  The statement.
 */
# ifndef DEBUG
#  define t(...)  do { if (__VA_ARGS__) goto fail; } while (0)
# else
#  define t(...)  do { if ((__VA_ARGS__) ? (failed__ = #__VA_ARGS__) : 0) { (perror)(failed__); goto fail; } } while (0)
static const char *failed__ = NULL;
#  define perror(_)  ((void)(_))
# endif
#endif



/**
 * `dup2(OLD, NEW)` and, on success, `close(OLD)`.
 * 
 * @param   OLD:int  The file descriptor to duplicate and close.
 * @param   NEW:int  The new file descriptor.
 * @return           0 on success, -1 on error.
 */
#define DUP2_AND_CLOSE(OLD, NEW)  (dup2(OLD, NEW) == -1 ? -1 : (close(OLD), 0))



/**
 * Macro to put directly after the variable definitions in `main`.
 */
#define DAEMON_PROLOGUE  \
	int rc = 0;  \
	assert(argc == 3);  \
	t (reopen(STATE_FILENO, O_RDWR))  \

/**
 * Macro to put before the cleanup code in `main`.
 */
#define DAEMON_CLEANUP_START  \
done:  \
	(void) send_string(SOCK_FILENO, 127, NULL);  \
	shutdown(SOCK_FILENO, SHUT_WR);  \
	close(SOCK_FILENO);  \
	close(STATE_FILENO)

/**
 * Macro to put after the cleanup code in `main`.
 */
#define DAEMON_CLEANUP_END  \
	return rc;  \
fail:  \
	if (send_string(SOCK_FILENO, STDERR_FILENO, argv[0], ": ", strerror(errno), "\n", NULL))  \
		perror(argv[0]);  \
	rc = 1;  \
	goto done;  \
	(void) argc



/**
 * A queued job.
 */
struct job {
	/**
	 * The job number.
	 */
	size_t no;

	/**
	 * The number of “argv” elements in `payload`.
	 */
	int argc;

	/**
	 * The clock in which `ts` is measured.
	 */
	clockid_t clk;

	/**
	 * The time when the job shall be executed.
	 */
	struct timespec ts;

	/**
	 * The number of bytes in `payload`.
	 */
	size_t n;

	/**
	 * “argv” followed by “envp”.
	 */
	char payload[];
};



/**
 * Wrapper for `pread` that reads the required amount of data.
 * 
 * @param   fildes  See pread(3).
 * @param   buf     See pread(3).
 * @param   nbyte   See pread(3).
 * @param   offset  See pread(3).
 * @return          See pread(3), only short if the file is shorter.
 */
ssize_t preadn(int fildes, void *buf, size_t nbyte, size_t offset);

/**
 * Wrapper for `pwrite` that writes all specified data.
 * 
 * @param   fildes  See pwrite(3).
 * @param   buf     See pwrite(3).
 * @param   nbyte   See pwrite(3).
 * @param   offset  See pwrite(3).
 * @return          See pwrite(3).
 */
ssize_t pwriten(int fildes, const void *buf, size_t nbyte, size_t offset);

/**
 * Wrapper for `read` that reads all available data.
 * 
 * @param   fd   The file descriptor from which to to read.
 * @param   buf  Output parameter for the data.
 * @param   n    Output parameter for the number of read bytes.
 * @return       0 on success, -1 on error.
 * 
 * @throws  Any exception specified for read(3).
 * @throws  Any exception specified for realloc(3).
 */
int readall(int fd, char **buf, size_t *n);

/**
 * Unmarshal a `NULL`-terminated string array.
 * 
 * The elements are not actually copied, subpointers
 * to `buf` are stored in the returned list.
 * 
 * @param   buf  The marshalled array. Must end with a NUL byte.
 * @param   len  The length of `buf`.
 * @param   n    Output parameter for the number of elements. May be `NULL`
 * @return       The list, `NULL` on error.
 * 
 * @throws  Any exception specified for realloc(3).
 */
char **restore_array(char *buf, size_t len, size_t *n);

/**
 * Create `NULL`-terminate subcopy of an list,
 * 
 * @param   list  The list.
 * @param   n     The number of elements in the new sublist.
 * @return        The sublist, `NULL` on error.
 * 
 * @throws  Any exception specified for malloc(3).
 */
char **sublist(char *const *list, size_t n);

/**
 * Create a new open file descriptor for an already
 * existing file descriptor.
 * 
 * @param   fd     The file descriptor that shall be promoted
 *                 to a new open file descriptor.
 * @param   oflag  See open(3), `O_CREAT` is not allowed.
 * @return         0 on success, -1 on error.
 */
int reopen(int fd, int oflag);

/**
 * Send a string to a client.
 * 
 * @param   sockfd  The file descriptor of the socket.
 * @param   outfd   The file descriptor to which the client shall output the message.
 * @param   ...     `NULL`-terminated list of string to concatenate.
 * @return          0 on success, -1 on error.
 */
int send_string(int sockfd, int outfd, ...);

/**
 * Run a job or a hook.
 * 
 * @param   job   The job.
 * @param   hook  The hook, `NULL` to run the job.
 * @return        0 on success, -1 on error, 1 if the child failed.
 */
int run_job_or_hook(struct job *job, const char *hook);

/**
 * Removes (and optionally runs) a job.
 * 
 * @param   jobno   The job number, `NULL` for any job.
 * @param   runjob  Shall we run the job too? 2 if its time has expired (not forced).
 * @return          0 on success, -1 on error.
 * 
 * @throws  0  The job is not in the queue.
 */
int remove_job(const char *jobno, int runjob);

/**
 * Get a `NULL`-terminated list of all queued jobs.
 * 
 * @return  A `NULL`-terminated list of all queued jobs. `NULL` on error.
 */
struct job **get_jobs(void);



/**
 * This block of code allows us to compile with DEBUG=valgrind
 * or DEBUG=strace and have all exec:s be wrapped in
 * valgrind --leak-check=full --show-leak-kinds=all or
 * strace, respectively. Very useful for debugging. However,
 * children do not inherit strace, so between forking and
 * exec:ing we do not have strace.
 */
#if 1 && defined(DEBUG)
# if ((DEBUG == 2) || (DEBUG == 3))
#  ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
__attribute__((__used__))
#  endif
#  if DEBUG == 2
#   define DEBUGPROG  "strace"
#  else
#   define DEBUGPROG  "valgrind"
#  endif
#  define execl(path, _, ...) (execl)("/usr/bin/" DEBUGPROG, "/usr/bin/" DEBUGPROG, path, __VA_ARGS__)
#  define execve(...) execve_(__VA_ARGS__)
static int
execve_(const char *path, char *const argv[], char *const envp[])
{
	size_t n = 0;
	char **new_argv = NULL;
	int x = (DEBUG - 2) * 2, saved_errno;
	while (argv[n++]);
	t (!(new_argv = malloc((n + 1 + (size_t)x) * sizeof(char *))));
	new_argv[x] = "--show-leak-kinds=all";
	new_argv[1] = "--leak-check=full";
	new_argv[0] = "/usr/bin/" DEBUGPROG;
	new_argv[1 + x] = path;
	memcpy(new_argv + 2 + x, argv + 1, (n - 1) * sizeof(char *));
	(execve)(*new_argv, new_argv, envp);
fail:
	return saved_errno = errno, free(new_argv), errno = saved_errno, -1;
}
#  ifdef __GNUC__
#   pragma GCC diagnostic pop
#  endif
# endif
#endif

