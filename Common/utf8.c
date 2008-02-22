/*
 * Copyright 2007 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x3270, c3270, wc3270, s3270, tcl3270, pr3287 and wpr3287 are distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the file LICENSE for more details.
 */

/*
 *	utf8.c
 *		3270 Terminal Emulator
 *		UTF-8 conversions
 */

#include "globals.h"

#include "popupsc.h"
#include "utf8c.h"

char *locale_codeset = CN;

static int utf8_ix = -1;
static Boolean is_utf8 = False;
#if defined(X3270_DBCS) /*[*/
static Boolean is_gb18030 = False;
#endif /*]*/

/* Various types of 8-bit character sets we understand. */
enum UTF_IX {
    	/* Real translations from 8-bit encodings to UTF-8. */
	U_ISO8859_1,
	U_ISO8859_2,
	U_ISO8859_7,
	U_ISO8859_8,
	U_ISO8859_9,
	U_ISO8859_11,
	U_ISO8859_15,
	U_KOI8_R,

#if defined(X3270_DBCS) /*[*/
	/* Multi-byte expansions of non-UTF-8 encodings. */
	PSEUDO_GB18030,
#endif /*]*/

	U_MAX
};

/*
 * Names of the above (real) character sets.  These are the names used in
 * displayCharset resources.
 */
static char *dcs[] = {		/* same order as enum UTF_IX and utf8_tab[] */
	"iso8859-1",
	"iso8859-2",
	"iso8859-7",
	"iso8859-8",
	"iso8859-9",
	"iso8859-11",
	"iso8859-15",
	"koi8-r",
	CN
};

/*
 * UTF-8 translation tables.
 * These are used to convert code points 0xa0 through 0xff to UTF-8.
 *
 * Note that this could be done much more generally with the iconv library
 * or the ICU library, but there is no guarantee that either is available or
 * properly configured.
 */
#define HI_SIZE 96
static char *utf8_tab[U_MAX][96] = {
/* iso8859-1 */ {
     "\xc2\xa0",     "\xc2\xa1",     "\xc2\xa2",     "\xc2\xa3",
     "\xc2\xa4",     "\xc2\xa5",     "\xc2\xa6",     "\xc2\xa7",
     "\xc2\xa8",     "\xc2\xa9",     "\xc2\xaa",     "\xc2\xab",
     "\xc2\xac",     "\xc2\xad",     "\xc2\xae",     "\xc2\xaf",
     "\xc2\xb0",     "\xc2\xb1",     "\xc2\xb2",     "\xc2\xb3",
     "\xc2\xb4",     "\xc2\xb5",     "\xc2\xb6",     "\xc2\xb7",
     "\xc2\xb8",     "\xc2\xb9",     "\xc2\xba",     "\xc2\xbb",
     "\xc2\xbc",     "\xc2\xbd",     "\xc2\xbe",     "\xc2\xbf",
     "\xc3\x80",     "\xc3\x81",     "\xc3\x82",     "\xc3\x83",
     "\xc3\x84",     "\xc3\x85",     "\xc3\x86",     "\xc3\x87",
     "\xc3\x88",     "\xc3\x89",     "\xc3\x8a",     "\xc3\x8b",
     "\xc3\x8c",     "\xc3\x8d",     "\xc3\x8e",     "\xc3\x8f",
     "\xc3\x90",     "\xc3\x91",     "\xc3\x92",     "\xc3\x93",
     "\xc3\x94",     "\xc3\x95",     "\xc3\x96",     "\xc3\x97",
     "\xc3\x98",     "\xc3\x99",     "\xc3\x9a",     "\xc3\x9b",
     "\xc3\x9c",     "\xc3\x9d",     "\xc3\x9e",     "\xc3\x9f",
     "\xc3\xa0",     "\xc3\xa1",     "\xc3\xa2",     "\xc3\xa3",
     "\xc3\xa4",     "\xc3\xa5",     "\xc3\xa6",     "\xc3\xa7",
     "\xc3\xa8",     "\xc3\xa9",     "\xc3\xaa",     "\xc3\xab",
     "\xc3\xac",     "\xc3\xad",     "\xc3\xae",     "\xc3\xaf",
     "\xc3\xb0",     "\xc3\xb1",     "\xc3\xb2",     "\xc3\xb3",
     "\xc3\xb4",     "\xc3\xb5",     "\xc3\xb6",     "\xc3\xb7",
     "\xc3\xb8",     "\xc3\xb9",     "\xc3\xba",     "\xc3\xbb",
     "\xc3\xbc",     "\xc3\xbd",     "\xc3\xbe",     "\xc3\xbf"
},
/* iso8859-2 */ {
     "\xc2\xa0",     "\xc4\x84",     "\xcb\x98",     "\xc5\x81",
     "\xc2\xa4",     "\xc4\xbd",     "\xc5\x9a",     "\xc2\xa7",
     "\xc2\xa8",     "\xc5\xa0",     "\xc5\x9e",     "\xc5\xa4",
     "\xc5\xb9",     "\xc2\xad",     "\xc5\xbd",     "\xc5\xbb",
     "\xc2\xb0",     "\xc4\x85",     "\xcb\x9b",     "\xc5\x82",
     "\xc2\xb4",     "\xc4\xbe",     "\xc5\x9b",     "\xcb\x87",
     "\xc2\xb8",     "\xc5\xa1",     "\xc5\x9f",     "\xc5\xa5",
     "\xc5\xba",     "\xcb\x9d",     "\xc5\xbe",     "\xc5\xbc",
     "\xc5\x94",     "\xc3\x81",     "\xc3\x82",     "\xc4\x82",
     "\xc3\x84",     "\xc4\xb9",     "\xc4\x86",     "\xc3\x87",
     "\xc4\x8c",     "\xc3\x89",     "\xc4\x98",     "\xc3\x8b",
     "\xc4\x9a",     "\xc3\x8d",     "\xc3\x8e",     "\xc4\x8e",
     "\xc4\x90",     "\xc5\x83",     "\xc5\x87",     "\xc3\x93",
     "\xc3\x94",     "\xc5\x90",     "\xc3\x96",     "\xc3\x97",
     "\xc5\x98",     "\xc5\xae",     "\xc3\x9a",     "\xc5\xb0",
     "\xc3\x9c",     "\xc3\x9d",     "\xc5\xa2",     "\xc3\x9f",
     "\xc5\x95",     "\xc3\xa1",     "\xc3\xa2",     "\xc4\x83",
     "\xc3\xa4",     "\xc4\xba",     "\xc4\x87",     "\xc3\xa7",
     "\xc4\x8d",     "\xc3\xa9",     "\xc4\x99",     "\xc3\xab",
     "\xc4\x9b",     "\xc3\xad",     "\xc3\xae",     "\xc4\x8f",
     "\xc4\x91",     "\xc5\x84",     "\xc5\x88",     "\xc3\xb3",
     "\xc3\xb4",     "\xc5\x91",     "\xc3\xb6",     "\xc3\xb7",
     "\xc5\x99",     "\xc5\xaf",     "\xc3\xba",     "\xc5\xb1",
     "\xc3\xbc",     "\xc3\xbd",     "\xc5\xa3",     "\xcb\x99"
},
/* iso8859-7 */ {
     "\xc2\xa0", "\xe2\x80\x98", "\xe2\x80\x99",     "\xc2\xa3",
 "\xe2\x82\xac", "\xe2\x82\xaf",     "\xc2\xa6",     "\xc2\xa7",
     "\xc2\xa8",     "\xc2\xa9",     "\xcd\xba",     "\xc2\xab",
     "\xc2\xac",     "\xc2\xad",            " ", "\xe2\x80\x95",
     "\xc2\xb0",     "\xc2\xb1",     "\xc2\xb2",     "\xc2\xb3",
     "\xce\x84",     "\xce\x85",     "\xce\x86",     "\xc2\xb7",
     "\xce\x88",     "\xce\x89",     "\xce\x8a",     "\xc2\xbb",
     "\xce\x8c",     "\xc2\xbd",     "\xce\x8e",     "\xce\x8f",
     "\xce\x90",     "\xce\x91",     "\xce\x92",     "\xce\x93",
     "\xce\x94",     "\xce\x95",     "\xce\x96",     "\xce\x97",
     "\xce\x98",     "\xce\x99",     "\xce\x9a",     "\xce\x9b",
     "\xce\x9c",     "\xce\x9d",     "\xce\x9e",     "\xce\x9f",
     "\xce\xa0",     "\xce\xa1",            " ",     "\xce\xa3",
     "\xce\xa4",     "\xce\xa5",     "\xce\xa6",     "\xce\xa7",
     "\xce\xa8",     "\xce\xa9",     "\xce\xaa",     "\xce\xab",
     "\xce\xac",     "\xce\xad",     "\xce\xae",     "\xce\xaf",
     "\xce\xb0",     "\xce\xb1",     "\xce\xb2",     "\xce\xb3",
     "\xce\xb4",     "\xce\xb5",     "\xce\xb6",     "\xce\xb7",
     "\xce\xb8",     "\xce\xb9",     "\xce\xba",     "\xce\xbb",
     "\xce\xbc",     "\xce\xbd",     "\xce\xbe",     "\xce\xbf",
     "\xcf\x80",     "\xcf\x81",     "\xcf\x82",     "\xcf\x83",
     "\xcf\x84",     "\xcf\x85",     "\xcf\x86",     "\xcf\x87",
     "\xcf\x88",     "\xcf\x89",     "\xcf\x8a",     "\xcf\x8b",
     "\xcf\x8c",     "\xcf\x8d",     "\xcf\x8e",            " "
},
/* iso8859-8 */ {
     "\xc2\xa0",            " ",     "\xc2\xa2",     "\xc2\xa3",
     "\xc2\xa4",     "\xc2\xa5",     "\xc2\xa6",     "\xc2\xa7",
     "\xc2\xa8",     "\xc2\xa9",     "\xc3\x97",     "\xc2\xab",
     "\xc2\xac",     "\xc2\xad",     "\xc2\xae",     "\xc2\xaf",
     "\xc2\xb0",     "\xc2\xb1",     "\xc2\xb2",     "\xc2\xb3",
     "\xc2\xb4",     "\xc2\xb5",     "\xc2\xb6",     "\xc2\xb7",
     "\xc2\xb8",     "\xc2\xb9",     "\xc3\xb7",     "\xc2\xbb",
     "\xc2\xbc",     "\xc2\xbd",     "\xc2\xbe",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ",            " ",
            " ",            " ",            " ", "\xe2\x80\x97",
     "\xd7\x90",     "\xd7\x91",     "\xd7\x92",     "\xd7\x93",
     "\xd7\x94",     "\xd7\x95",     "\xd7\x96",     "\xd7\x97",
     "\xd7\x98",     "\xd7\x99",     "\xd7\x9a",     "\xd7\x9b",
     "\xd7\x9c",     "\xd7\x9d",     "\xd7\x9e",     "\xd7\x9f",
     "\xd7\xa0",     "\xd7\xa1",     "\xd7\xa2",     "\xd7\xa3",
     "\xd7\xa4",     "\xd7\xa5",     "\xd7\xa6",     "\xd7\xa7",
     "\xd7\xa8",     "\xd7\xa9",     "\xd7\xaa",            " ",
            " ", "\xe2\x80\x8e", "\xe2\x80\x8f",            " "
},
/* iso8859-9 */ {
     "\xc2\xa0",     "\xc2\xa1",     "\xc2\xa2",     "\xc2\xa3",
     "\xc2\xa4",     "\xc2\xa5",     "\xc2\xa6",     "\xc2\xa7",
     "\xc2\xa8",     "\xc2\xa9",     "\xc2\xaa",     "\xc2\xab",
     "\xc2\xac",     "\xc2\xad",     "\xc2\xae",     "\xc2\xaf",
     "\xc2\xb0",     "\xc2\xb1",     "\xc2\xb2",     "\xc2\xb3",
     "\xc2\xb4",     "\xc2\xb5",     "\xc2\xb6",     "\xc2\xb7",
     "\xc2\xb8",     "\xc2\xb9",     "\xc2\xba",     "\xc2\xbb",
     "\xc2\xbc",     "\xc2\xbd",     "\xc2\xbe",     "\xc2\xbf",
     "\xc3\x80",     "\xc3\x81",     "\xc3\x82",     "\xc3\x83",
     "\xc3\x84",     "\xc3\x85",     "\xc3\x86",     "\xc3\x87",
     "\xc3\x88",     "\xc3\x89",     "\xc3\x8a",     "\xc3\x8b",
     "\xc3\x8c",     "\xc3\x8d",     "\xc3\x8e",     "\xc3\x8f",
     "\xc4\x9e",     "\xc3\x91",     "\xc3\x92",     "\xc3\x93",
     "\xc3\x94",     "\xc3\x95",     "\xc3\x96",     "\xc3\x97",
     "\xc3\x98",     "\xc3\x99",     "\xc3\x9a",     "\xc3\x9b",
     "\xc3\x9c",     "\xc4\xb0",     "\xc5\x9e",     "\xc3\x9f",
     "\xc3\xa0",     "\xc3\xa1",     "\xc3\xa2",     "\xc3\xa3",
     "\xc3\xa4",     "\xc3\xa5",     "\xc3\xa6",     "\xc3\xa7",
     "\xc3\xa8",     "\xc3\xa9",     "\xc3\xaa",     "\xc3\xab",
     "\xc3\xac",     "\xc3\xad",     "\xc3\xae",     "\xc3\xaf",
     "\xc4\x9f",     "\xc3\xb1",     "\xc3\xb2",     "\xc3\xb3",
     "\xc3\xb4",     "\xc3\xb5",     "\xc3\xb6",     "\xc3\xb7",
     "\xc3\xb8",     "\xc3\xb9",     "\xc3\xba",     "\xc3\xbb",
     "\xc3\xbc",     "\xc4\xb1",     "\xc5\x9f",     "\xc3\xbf"
},
/* iso8859-11 */ {
     "\xc2\xa0", "\xe0\xb8\x81", "\xe0\xb8\x82", "\xe0\xb8\x83",
 "\xe0\xb8\x84", "\xe0\xb8\x85", "\xe0\xb8\x86", "\xe0\xb8\x87",
 "\xe0\xb8\x88", "\xe0\xb8\x89", "\xe0\xb8\x8a", "\xe0\xb8\x8b",
 "\xe0\xb8\x8c", "\xe0\xb8\x8d", "\xe0\xb8\x8e", "\xe0\xb8\x8f",
 "\xe0\xb8\x90", "\xe0\xb8\x91", "\xe0\xb8\x92", "\xe0\xb8\x93",
 "\xe0\xb8\x94", "\xe0\xb8\x95", "\xe0\xb8\x96", "\xe0\xb8\x97",
 "\xe0\xb8\x98", "\xe0\xb8\x99", "\xe0\xb8\x9a", "\xe0\xb8\x9b",
 "\xe0\xb8\x9c", "\xe0\xb8\x9d", "\xe0\xb8\x9e", "\xe0\xb8\x9f",
 "\xe0\xb8\xa0", "\xe0\xb8\xa1", "\xe0\xb8\xa2", "\xe0\xb8\xa3",
 "\xe0\xb8\xa4", "\xe0\xb8\xa5", "\xe0\xb8\xa6", "\xe0\xb8\xa7",
 "\xe0\xb8\xa8", "\xe0\xb8\xa9", "\xe0\xb8\xaa", "\xe0\xb8\xab",
 "\xe0\xb8\xac", "\xe0\xb8\xad", "\xe0\xb8\xae", "\xe0\xb8\xaf",
 "\xe0\xb8\xb0", "\xe0\xb8\xb1", "\xe0\xb8\xb2", "\xe0\xb8\xb3",
 "\xe0\xb8\xb4", "\xe0\xb8\xb5", "\xe0\xb8\xb6", "\xe0\xb8\xb7",
 "\xe0\xb8\xb8", "\xe0\xb8\xb9", "\xe0\xb8\xba",            " ",
            " ",            " ",            " ", "\xe0\xb8\xbf",
 "\xe0\xb9\x80", "\xe0\xb9\x81", "\xe0\xb9\x82", "\xe0\xb9\x83",
 "\xe0\xb9\x84", "\xe0\xb9\x85", "\xe0\xb9\x86", "\xe0\xb9\x87",
 "\xe0\xb9\x88", "\xe0\xb9\x89", "\xe0\xb9\x8a", "\xe0\xb9\x8b",
 "\xe0\xb9\x8c", "\xe0\xb9\x8d", "\xe0\xb9\x8e", "\xe0\xb9\x8f",
 "\xe0\xb9\x90", "\xe0\xb9\x91", "\xe0\xb9\x92", "\xe0\xb9\x93",
 "\xe0\xb9\x94", "\xe0\xb9\x95", "\xe0\xb9\x96", "\xe0\xb9\x97",
 "\xe0\xb9\x98", "\xe0\xb9\x99", "\xe0\xb9\x9a", "\xe0\xb9\x9b",
            " ",            " ",            " ",            " "
},
/* iso8859-15 */ {
     "\xc2\xa0",     "\xc2\xa1",     "\xc2\xa2",     "\xc2\xa3",
 "\xe2\x82\xac",     "\xc2\xa5",     "\xc5\xa0",     "\xc2\xa7",
     "\xc5\xa1",     "\xc2\xa9",     "\xc2\xaa",     "\xc2\xab",
     "\xc2\xac",     "\xc2\xad",     "\xc2\xae",     "\xc2\xaf",
     "\xc2\xb0",     "\xc2\xb1",     "\xc2\xb2",     "\xc2\xb3",
     "\xc5\xbd",     "\xc2\xb5",     "\xc2\xb6",     "\xc2\xb7",
     "\xc5\xbe",     "\xc2\xb9",     "\xc2\xba",     "\xc2\xbb",
     "\xc5\x92",     "\xc5\x93",     "\xc5\xb8",     "\xc2\xbf",
     "\xc3\x80",     "\xc3\x81",     "\xc3\x82",     "\xc3\x83",
     "\xc3\x84",     "\xc3\x85",     "\xc3\x86",     "\xc3\x87",
     "\xc3\x88",     "\xc3\x89",     "\xc3\x8a",     "\xc3\x8b",
     "\xc3\x8c",     "\xc3\x8d",     "\xc3\x8e",     "\xc3\x8f",
     "\xc3\x90",     "\xc3\x91",     "\xc3\x92",     "\xc3\x93",
     "\xc3\x94",     "\xc3\x95",     "\xc3\x96",     "\xc3\x97",
     "\xc3\x98",     "\xc3\x99",     "\xc3\x9a",     "\xc3\x9b",
     "\xc3\x9c",     "\xc3\x9d",     "\xc3\x9e",     "\xc3\x9f",
     "\xc3\xa0",     "\xc3\xa1",     "\xc3\xa2",     "\xc3\xa3",
     "\xc3\xa4",     "\xc3\xa5",     "\xc3\xa6",     "\xc3\xa7",
     "\xc3\xa8",     "\xc3\xa9",     "\xc3\xaa",     "\xc3\xab",
     "\xc3\xac",     "\xc3\xad",     "\xc3\xae",     "\xc3\xaf",
     "\xc3\xb0",     "\xc3\xb1",     "\xc3\xb2",     "\xc3\xb3",
     "\xc3\xb4",     "\xc3\xb5",     "\xc3\xb6",     "\xc3\xb7",
     "\xc3\xb8",     "\xc3\xb9",     "\xc3\xba",     "\xc3\xbb",
     "\xc3\xbc",     "\xc3\xbd",     "\xc3\xbe",     "\xc3\xbf"
},
/* koi8-r */ {
 "\xe2\x95\x90", "\xe2\x95\x91", "\xe2\x95\x92",     "\xd1\x91",
 "\xe2\x95\x93", "\xe2\x95\x94", "\xe2\x95\x95", "\xe2\x95\x96",
 "\xe2\x95\x97", "\xe2\x95\x98", "\xe2\x95\x99", "\xe2\x95\x9a",
 "\xe2\x95\x9b", "\xe2\x95\x9c", "\xe2\x95\x9d", "\xe2\x95\x9e",
 "\xe2\x95\x9f", "\xe2\x95\xa0", "\xe2\x95\xa1",     "\xd0\x81",
 "\xe2\x95\xa2", "\xe2\x95\xa3", "\xe2\x95\xa4", "\xe2\x95\xa5",
 "\xe2\x95\xa6", "\xe2\x95\xa7", "\xe2\x95\xa8", "\xe2\x95\xa9",
 "\xe2\x95\xaa", "\xe2\x95\xab", "\xe2\x95\xac",     "\xc2\xa9",
     "\xd1\x8e",     "\xd0\xb0",     "\xd0\xb1",     "\xd1\x86",
     "\xd0\xb4",     "\xd0\xb5",     "\xd1\x84",     "\xd0\xb3",
     "\xd1\x85",     "\xd0\xb8",     "\xd0\xb9",     "\xd0\xba",
     "\xd0\xbb",     "\xd0\xbc",     "\xd0\xbd",     "\xd0\xbe",
     "\xd0\xbf",     "\xd1\x8f",     "\xd1\x80",     "\xd1\x81",
     "\xd1\x82",     "\xd1\x83",     "\xd0\xb6",     "\xd0\xb2",
     "\xd1\x8c",     "\xd1\x8b",     "\xd0\xb7",     "\xd1\x88",
     "\xd1\x8d",     "\xd1\x89",     "\xd1\x87",     "\xd1\x8a",
     "\xd0\xae",     "\xd0\x90",     "\xd0\x91",     "\xd0\xa6",
     "\xd0\x94",     "\xd0\x95",     "\xd0\xa4",     "\xd0\x93",
     "\xd0\xa5",     "\xd0\x98",     "\xd0\x99",     "\xd0\x9a",
     "\xd0\x9b",     "\xd0\x9c",     "\xd0\x9d",     "\xd0\x9e",
     "\xd0\x9f",     "\xd0\xaf",     "\xd0\xa0",     "\xd0\xa1",
     "\xd0\xa2",     "\xd0\xa3",     "\xd0\x96",     "\xd0\x92",
     "\xd0\xac",     "\xd0\xab",     "\xd0\x97",     "\xd0\xa8",
     "\xd0\xad",     "\xd0\xa9",     "\xd0\xa7",     "\xd0\xaa"
},
#if defined(X3270_DBCS) /*[*/
/* pseudo-gb18030 - not actually a UTF-8 expansion */ {
 "\x81\x30\x84\x32", "\x81\x30\x84\x33", "\x81\x30\x84\x34", "\x81\x30\x84\x35",
         "\xa1\xe8", "\x81\x30\x84\x36", "\x81\x30\x84\x37",         "\xa1\xec",
         "\xa1\xa7", "\x81\x30\x84\x38", "\x81\x30\x84\x39", "\x81\x30\x85\x30",
 "\x81\x30\x85\x31", "\x81\x30\x85\x32", "\x81\x30\x85\x33", "\x81\x30\x85\x34",
         "\xa1\xe3",         "\xa1\xc0", "\x81\x30\x85\x35", "\x81\x30\x85\x36",
 "\x81\x30\x85\x37", "\x81\x30\x85\x38", "\x81\x30\x85\x39",         "\xa1\xa4",
 "\x81\x30\x86\x30", "\x81\x30\x86\x31", "\x81\x30\x86\x32", "\x81\x30\x86\x33",
 "\x81\x30\x86\x34", "\x81\x30\x86\x35", "\x81\x30\x86\x36", "\x81\x30\x86\x37",
 "\x81\x30\x86\x38", "\x81\x30\x86\x39", "\x81\x30\x87\x30", "\x81\x30\x87\x31",
 "\x81\x30\x87\x32", "\x81\x30\x87\x33", "\x81\x30\x87\x34", "\x81\x30\x87\x35",
 "\x81\x30\x87\x36", "\x81\x30\x87\x37", "\x81\x30\x87\x38", "\x81\x30\x87\x39",
 "\x81\x30\x88\x30", "\x81\x30\x88\x31", "\x81\x30\x88\x32", "\x81\x30\x88\x33",
 "\x81\x30\x88\x34", "\x81\x30\x88\x35", "\x81\x30\x88\x36", "\x81\x30\x88\x37",
 "\x81\x30\x88\x38", "\x81\x30\x88\x39", "\x81\x30\x89\x30",         "\xa1\xc1",
 "\x81\x30\x89\x31", "\x81\x30\x89\x32", "\x81\x30\x89\x33", "\x81\x30\x89\x34",
 "\x81\x30\x89\x35", "\x81\x30\x89\x36", "\x81\x30\x89\x37", "\x81\x30\x89\x38",
         "\xa8\xa4",         "\xa8\xa2", "\x81\x30\x89\x39", "\x81\x30\x8a\x30",
 "\x81\x30\x8a\x31", "\x81\x30\x8a\x32", "\x81\x30\x8a\x33", "\x81\x30\x8a\x34",
         "\xa8\xa8",         "\xa8\xa6",         "\xa8\xba", "\x81\x30\x8a\x35",
         "\xa8\xac",         "\xa8\xaa", "\x81\x30\x8a\x36", "\x81\x30\x8a\x37",
 "\x81\x30\x8a\x38", "\x81\x30\x8a\x39",         "\xa8\xb0",         "\xa8\xae",
 "\x81\x30\x8b\x30", "\x81\x30\x8b\x31", "\x81\x30\x8b\x32",         "\xa1\xc2",
 "\x81\x30\x8b\x33",         "\xa8\xb4",         "\xa8\xb2", "\x81\x30\x8b\x34",
         "\xa8\xb9", "\x81\x30\x8b\x35", "\x81\x30\x8b\x36", "\x81\x30\x8b\x37"
}
#endif /*]*/
};

/*
 * Save the codeset from the locale, and set globals based on known values.
 */
void
set_codeset(char *codeset_name)
{
#if defined(X3270_DBCS) /*[*/
    	is_gb18030 = !strcasecmp(codeset_name, "gb18030");
#endif /*]*/

#if !defined(TCL3270) /*[*/
    	is_utf8 = (!strcasecmp(codeset_name, "utf-8") ||
	           !strcasecmp(codeset_name, "utf8") ||
		   !strcasecmp(codeset_name, "utf_8"));
#else /*][*/
	/*
	 * tcl3270 is always in UTF-8 mode, because it needs to
	 * supply UTF-8 strings to libtcl.
	 */
	is_utf8 = 1;
#endif /*]*/

	Replace(locale_codeset, NewString(codeset_name));
}

/*
 * Set globals based on an x3270 character set list.
 *
 * This defines the 'implied' 8-bit character set for code points 0xa0..0xff,
 * used by utf8_expand() below.
 *
 * set_codeset, above, must be called _before_ this function.
 */
Boolean
utf8_set_display_charsets(char *cslist, char *csname)
{
    	char *dup;
    	char *ptr;
	char *tok;
	int i;

	utf8_ix = -1;

#if defined(X3270_DBCS) /*[*/
	if (strchr(cslist, '+') != CN)
	    	dbcs = True;
#endif /*]*/

    	if (!is_utf8) {
#if defined(X3270_DBCS) /*[*/
	    	if (is_gb18030) {
		    	/*
			 * A convenient lie.
			 * If the locale is GB18030, use the UTF-8
			 * expansion mechanism to translate 0xa0 through
			 * 0xff, using a table entry that expands to GB18030
			 * multi-byte sequences rather than UTF-8.
			 *
			 * Note that there appears to be a bug in ncursesw
			 * (or something it depends on) that turns these
			 * characters into garbage.  Hopefully this bug will
			 * be fixed and this code will start working properly.
			 *
			 * As a workarond, everything works properly in a
			 * Chinese UTF-8 locale.
			 */
		    	utf8_ix = PSEUDO_GB18030;
		}
#endif /*]*/
	    	return True;
	}

#if defined(X3270_DBCS) /*[*/
	/* For DBCS, map 0xa0..0xff as Latin-1. */
	if (dbcs) {
	    	utf8_ix = U_ISO8859_1;
		return True;
	}
#endif /*]*/

	/* Skip 3270cg sets. */
    	ptr = dup = NewString(cslist);
	while ((tok = strtok(ptr, ",")) != CN) {
		ptr = NULL;
	    	if (!strncasecmp(tok, "3270cg", 6))
		    	continue;
		break;
	}
	if (tok == CN) {
	    	popup_an_error("Invalid displayCharset specification");
		Free(dup);
		return False;
	}

	/* Look up the charset. */
	for (i = 0; dcs[i] != CN; i++) {
	    	if (!strcasecmp(dcs[i], tok))
		    	break;
	}
	if (dcs[i] == CN) {
	    	popup_an_error("Unknown displayCharset specification '%s'",
			csname);
		Free(dup);
		return False;
	}
	utf8_ix = i;
	Free(dup);
	return True;
}

/* Expand an 8-bit character in the 'implied' character set. */
char *
utf8_expand(unsigned char c)
{
    	static char nox[2];

	if (c & 0x80) {
	    	if (utf8_ix >= 0) {
			if (c >= 0xa0)
				return utf8_tab[utf8_ix][c - 0xa0];
			else
				return " ";
#if defined(X3270_DBCS) /*[*/
		} else if (dbcs) {
		    	/*
			 * GB18030 it treated as a special case of UTF-8,
			 * above.  GB2312 does not support these characters.
			 * Other DBCS encodings will need to be added as they
			 * are better understood.
			 */
		    	return " ";
#endif /*]*/
		}
	}
	nox[0] = c;
	nox[1] = '\0';
	return nox;
}

/*
 * Look up a multi-byte UTF-8 string and return its value in the 'implied'
 * character set.
 * Returns 0 if the lookup fails.
 */
unsigned char
utf8_lookup(char *mbs, enum ulfail *fail, int *consumed)
{
    	int i;
	int mblen = strlen(mbs);

    	if (utf8_ix < 0) {
	    	if (fail != NULL)
		    	*fail = ULFAIL_NOUTF8;
	    	return 0;
	}
	for (i = 0; i < HI_SIZE; i++) {
	    	int tlen = strlen(utf8_tab[utf8_ix][i]);

		/* Check for dummy " " entry. */
		if (tlen == 1)
		    	continue;

		/* Check for incomplete string. */
		if (mblen < tlen) {
			if (fail != NULL)
				*fail = ULFAIL_INCOMPLETE;
			return 0;
		}
	    	if (!strncmp(mbs, utf8_tab[utf8_ix][i], tlen)) {
		    	if (consumed != NULL)
			    	*consumed = tlen;
		    	return 0xa0 + i;
		}
	}
	if (fail != NULL)
	    	*fail = ULFAIL_INVALID;
	return 0;
}
