/*
 * Copyright (c) 1993-2013, Paul Mattes.
 * Copyright (c) 1990, Jeff Sparkes.
 * Copyright (c) 1989, Georgia Tech Research Corporation (GTRC), Atlanta,
 *  GA 30332.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the names of Paul Mattes, Jeff Sparkes, GTRC nor their
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES, JEFF SPARKES AND GTRC "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL PAUL MATTES, JEFF SPARKES OR
 * GTRC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *	xio.c
 *		Low-level I/O setup functions and exit code.
 */

#include "globals.h"

#include "actionsc.h"
#include "hostc.h"
#include "telnetc.h"
#include "togglesc.h"
#include "utilc.h"
#include "xioc.h"

/* Statics. */
static ioid_t ns_read_id;
static ioid_t ns_exception_id;
static Boolean reading = False;
static Boolean excepting = False;

/*
 * Called to set up input on a new network connection.
 */
void
x_add_input(int net_sock)
{
	ns_exception_id = AddExcept(net_sock, net_exception);
	excepting = True;
	ns_read_id = AddInput(net_sock, net_input);
	reading = True;
}

/*
 * Called when an exception is received to disable further exceptions.
 */
void
x_except_off(void)
{
	if (excepting) {
		RemoveInput(ns_exception_id);
		excepting = False;
	}
}

/*
 * Called when exception processing is complete to re-enable exceptions.
 * This includes removing and restoring reading, so the exceptions are always
 * processed first.
 */
void
x_except_on(int net_sock)
{
	if (excepting)
		return;
	if (reading)
		RemoveInput(ns_read_id);
	ns_exception_id = AddExcept(net_sock, net_exception);
	excepting = True;
	if (reading)
		ns_read_id = AddInput(net_sock, net_input);
}

/*
 * Called to disable input on a closing network connection.
 */
void
x_remove_input(void)
{
	if (reading) {
		RemoveInput(ns_read_id);
		reading = False;
	}
	if (excepting) {
		RemoveInput(ns_exception_id);
		excepting = False;
	}
}

/*
 * Application exit, with cleanup.
 */
void
x3270_exit(int n)
{
	static Boolean already_exiting = 0;

	/* Handle unintentional recursion. */
	if (already_exiting)
		return;

	already_exiting = True;

	/* Flush any pending output (mostly for Windows). */
	fflush(stdout);
	fflush(stderr);

	/* Turn off toggle-related activity. */
	shutdown_toggles();

	/* Shut down the socket gracefully. */
	host_disconnect(False);

	/* Tell anyone else who's interested. */
	st_changed(ST_EXITING, True);

	if (n) {
		char buf[2];
		char *r;

		printf("\n[Press <Enter>] ");
		fflush(stdout);
		r = fgets(buf, sizeof(buf), stdin);
		r = r; /* keep gcc happy */

	}

#if !defined(_WIN32) /*[*/
	exit(n);
#else /*][*/
	/*
	 * On Windows, call ExitProcess() instead of the POSIXish exit().
	 * Apparently calling exit() in a ConsoleCtrlHandler is a bad thing on
	 * XP, and causes a hang.
	 */
	ExitProcess(n);
#endif /*]*/
}

Boolean
Quit_eaction(ia_t ia, unsigned argc, const char **argv)
{
    eaction_debug("Quit", ia, argc, argv);
    if (ia != IA_KEYMAP || !CONNECTED) {
	x3270_exit(0);
    }
    return False;
}
