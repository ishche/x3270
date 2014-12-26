/*
 * Copyright (c) 1996-2009, 2013 Paul Mattes.
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
 *     * Neither the name of Paul Mattes nor his contributors may be used
 *       to endorse or promote products derived from this software without
 *       specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL PAUL MATTES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *	keymapc.h
 *		Global declarations for keymap.c.
 */

#define PA_KEYMAP_TRACE		PA_PFX "KeymapTrace"
#define PA_END			PA_PFX "End"

extern char *current_keymap;
extern Boolean keymap_changed;
extern char *keymap_trace;
extern struct trans_list *temp_keymaps;

extern void do_keymap_display(Widget w, XtPointer userdata, XtPointer calldata);
extern void keymap_init(const char *km, Boolean interactive);
extern XtTranslations lookup_tt(const char *name, char *table);
extern void PA_End_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
extern void PA_KeymapTrace_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
extern int temporary_keymap(const char *k);
