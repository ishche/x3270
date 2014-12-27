/*
 * Copyright (c) 1995-2009, 2014 Paul Mattes.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the names of Paul Mattes nor the names of his contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL PAUL MATTES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *	macrosc.h
 *		Global declarations for macros.c.
 */

/* macro definition */
struct macro_def {
    char		*name;
    char		**parents;
    char		*action;
    struct macro_def	*next;
};
extern struct macro_def *macro_defs;
extern Boolean macro_output;

extern void abort_script(void);
extern action_t Abort_action;
extern action_t AnsiText_action;
extern action_t AsciiField_action;
extern action_t Ascii_action;
extern void cancel_if_idle_command(void);
extern action_t Bell_action;
extern action_t CloseScript_action;
extern action_t ContinueScript_action;
extern action_t EbcdicField_action;
extern action_t Ebcdic_action;
extern action_t Execute_action;
extern action_t Expect_action;
extern void login_macro(char *s);
extern void macros_init(void);
extern action_t Macro_action;
extern void macro_command(struct macro_def *m);
extern action_t PauseScript_action;
extern void peer_script_init(void);
extern void ps_set(char *s, Boolean is_hex);
extern action_t Printer_action;
extern void push_command(char *);
extern void push_idle(char *);
extern void push_keymap_action(char *);
extern void push_macro(char *, Boolean);
extern action_t Query_action;
extern action_t ReadBuffer_action;
extern action_t Script_action;
#if !defined(TCL3270) /*[*/
extern void sms_accumulate_time(struct timeval *, struct timeval *);
#else /*][*/
#define sms_accumulate_time(t0, t1)
#endif /*]*/
extern Boolean sms_active(void);
extern void sms_connect_wait(void);
extern void sms_continue(void);
extern void sms_error(const char *msg);
extern void sms_host_output(void);
extern void sms_info(const char *fmt, ...) printflike(1, 2);
extern void sms_init(void);
extern Boolean sms_in_macro(void);
extern Boolean sms_redirect(void);
extern void sms_store(unsigned char c);
extern action_t Snap_action;
#if defined(TCL3270) /*[*/
extern action_t Status_action;
#endif /*]*/
extern action_t Source_action;
extern action_t Wait_action;

typedef void *sms_cbh;
typedef void (*sms_data_cb)(sms_cbh handle, const char *buf, size_t len);
typedef void (*sms_done_cb)(sms_cbh handle, Boolean success,
	const char *status_buf, size_t status_len);
typedef struct {
    const char *shortname;
    enum iaction ia;
    sms_data_cb data;
    sms_done_cb done;
} sms_cb_t;
extern void push_cb(const char *buf, size_t len, const sms_cb_t *cb,
	sms_cbh handle);
