/**
 * Copyright © 2015  Mattias Andrée <maandree@member.fsf.org>
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
#include "client.h"



/**
 * The name of the process.
 */
extern char *argv0;



/**
 * Send a command to satd. Start satd if it is not running.
 * 
 * If `n` is 0 but `msg` is not `NULL`, `msg` is a
 * NUL-terminated string with the ID of the job to
 * remove for the queue. `msg` should otherwise,
 * unless it is `NULL` be the command the run, followed
 * by its environment.
 * 
 * @param   n    The length of the message, or a number
 *               number to send if `msg` is `NULL`.
 * @param   msg  The message to send.
 * @return       Zero on success.
 * 
 * @throws  0  Error at the daemon-side.
 */
int
send_command(size_t n, const char *restrict msg)
{
	return 0 /* TODO */
}

