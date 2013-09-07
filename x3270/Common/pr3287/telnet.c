/*
 * Copyright (c) 1993-2013, Paul Mattes.
 * Copyright (c) 1990, Jeff Sparkes.
 * Copyright (c) 1989, Georgia Tech Research Corporation (GTRC), Atlanta, GA
 *  30332.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the names of Paul Mattes, Jeff Sparkes, GTRC nor the names of
 *       their contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY PAUL MATTES, JEFF SPARKES AND GTRC "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PAUL MATTES, JEFF SPARKES OR GTRC BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *	telnet.c
 *		This module initializes and manages a telnet socket to
 *		the given IBM host.
 */


#include "globals.h"
#if defined(_WIN32) /*[*/
#include <winsock2.h>
#include <ws2tcpip.h>
#undef AF_INET6
#else /*][*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#endif /*]*/
#define TELCMDS 1
#define TELOPTS 1
#include "arpa_telnet.h"
#include <errno.h>
#include <fcntl.h>
#if !defined(_WIN32) /*[*/
#include <netdb.h>
#endif /*]*/
#include <stdarg.h>
#include <string.h>
#if !defined(_MSC_VER) /*[*/
#include <sys/time.h>
#endif /*]*/
#if !defined(_MSC_VER) /*[*/
#include <unistd.h>
#endif /*]*/
#include <stdlib.h>
#include <time.h>
#if defined(HAVE_LIBSSL) /*[*/ 
#if defined(_WIN32) /*[*/
#include "ssl_dll.h"
#endif /*]*/
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#endif /*]*/
#include "tn3270e.h"

#include "pr3287.h"

#include "ctlrc.h"
#include "resolverc.h"
#include "trace_dsc.h"
#include "telnetc.h"

#if !defined(TELOPT_STARTTLS) /*[*/
#define TELOPT_STARTTLS        46
#endif /*]*/
#define TLS_FOLLOWS    1

typedef union {
	struct sockaddr sa;
	struct sockaddr_in sin;
#if defined(AF_INET6) /*[*/
	struct sockaddr_in6 sin6;
#endif /*]*/
} sockaddr_46_t;

extern FILE *tracef;

#if defined(HAVE_LIBSSL) /*[*/
static Boolean accept_specified_host;
static const char *accept_dnsname;
struct in_addr host_inaddr;
static Boolean host_inaddr_valid;
# if defined(AF_INET6) /*[*/
struct in6_addr host_in6addr;
static Boolean host_in6addr_valid;
# endif /*]*/
static Boolean check_cert_name(const char *host);
#endif /*]*/

#if defined(_WIN32) /*[*/
extern char *appdata;
#endif /*]*/

/*   connection state */
enum cstate {
	NOT_CONNECTED,		/* no socket, unknown mode */
	PENDING,		/* connection pending */
	CONNECTED_INITIAL,	/* connected, no mode yet */
	CONNECTED_ANSI,		/* connected in NVT ANSI mode */
	CONNECTED_3270,		/* connected in old-style 3270 mode */
	CONNECTED_INITIAL_E,	/* connected in TN3270E mode, unnegotiated */
	CONNECTED_NVT,		/* connected in TN3270E mode, NVT mode */
	CONNECTED_SSCP,		/* connected in TN3270E mode, SSCP-LU mode */
	CONNECTED_TN3270E	/* connected in TN3270E mode, 3270 mode */
};
enum cstate cstate = NOT_CONNECTED;

#define PCONNECTED	((int)cstate >= (int)PENDING)
#define HALF_CONNECTED	(cstate == PENDING)
#define CONNECTED	((int)cstate >= (int)CONNECTED_INITIAL)
#define IN_NEITHER	(cstate == CONNECTED_INITIAL)
#define IN_ANSI		(cstate == CONNECTED_ANSI || cstate == CONNECTED_NVT)
#define IN_3270		(cstate == CONNECTED_3270 || cstate == CONNECTED_TN3270E || cstate == CONNECTED_SSCP)
#define IN_SSCP		(cstate == CONNECTED_SSCP)
#define IN_TN3270E	(cstate == CONNECTED_TN3270E)
#define IN_E		(cstate >= CONNECTED_INITIAL_E)

static char *connected_lu = NULL;
static char *connected_type = NULL;
static char *hostname = NULL;
static sockaddr_46_t host_sa;

#define BUFSZ		4096

#define N_OPTS		256

static int on = 1;

/* Globals */
time_t          ns_time;
int             ns_brcvd;
int             ns_rrcvd;
int             ns_bsent;
int             ns_rsent;
unsigned char  *obuf;		/* 3270 output buffer */
int             obuf_size = 0;
unsigned char  *obptr = (unsigned char *) NULL;
int             linemode = 1;
const char     *termtype = "IBM-3287-1";

/* Externals */
#if !defined(_WIN32) /*[*/
extern unsigned long inet_addr(const char *);
#endif /*]*/
static struct timeval ds_ts;

/* Statics */
static int      sock = -1;	/* active socket */
static unsigned char myopts[N_OPTS], hisopts[N_OPTS];
			/* telnet option flags */
static unsigned char *ibuf = (unsigned char *) NULL;
			/* 3270 input buffer */
static unsigned char *ibptr;
static int      ibuf_size = 0;	/* size of ibuf */
static unsigned char *obuf_base = (unsigned char *)NULL;
static unsigned char *netrbuf = (unsigned char *)NULL;
			/* network input buffer */
static unsigned char *sbbuf = (unsigned char *)NULL;
			/* telnet sub-option buffer */
static unsigned char *sbptr;
static unsigned char telnet_state;
static int      syncing;

static unsigned long e_funcs;	/* negotiated TN3270E functions */
#define E_OPT(n)	(1 << (n))
static unsigned short e_xmit_seq; /* transmit sequence number */
static int response_required;

static int	tn3270e_negotiated = 0;
static enum { E_NONE, E_3270, E_NVT, E_SSCP } tn3270e_submode = E_NONE;
static int	tn3270e_bound = 0;
static char	**lus = (char **)NULL;
static char	**curr_lu = (char **)NULL;
static char	*try_lu = NULL;
static char	*try_assoc = NULL;

static void setup_lus(char *luname, const char *assoc);
static int telnet_fsm(unsigned char c);
static void net_rawout(unsigned const char *buf, int len);
static void check_in3270(void);
static void store3270in(unsigned char c);
static int tn3270e_negotiate(void);
static int process_eor(void);
static const char *tn3270e_function_names(const unsigned char *, int);
static void tn3270e_subneg_send(unsigned char, unsigned long);
static unsigned long tn3270e_fdecode(const unsigned char *, int);
static void tn3270_ack(void);
static void tn3270_nak(enum pds);
static void tn3270e_ack(void);
static void tn3270e_nak(enum pds);
static void tn3270e_cleared(void);

#define trace_str(str)	vtrace("%s", (str))
static const char *cmd(int c);
static const char *opt(unsigned char c);
static const char *nnn(int c);

/* telnet states */
#define TNS_DATA	0	/* receiving data */
#define TNS_IAC		1	/* got an IAC */
#define TNS_WILL	2	/* got an IAC WILL */
#define TNS_WONT	3	/* got an IAC WONT */
#define TNS_DO		4	/* got an IAC DO */
#define TNS_DONT	5	/* got an IAC DONT */
#define TNS_SB		6	/* got an IAC SB */
#define TNS_SB_IAC	7	/* got an IAC after an IAC SB */

/* telnet predefined messages */
static unsigned char	do_opt[]	= { 
	IAC, DO, '_' };
static unsigned char	dont_opt[]	= { 
	IAC, DONT, '_' };
static unsigned char	will_opt[]	= { 
	IAC, WILL, '_' };
static unsigned char	wont_opt[]	= { 
	IAC, WONT, '_' };
static unsigned char	functions_req[] = {
	IAC, SB, TELOPT_TN3270E, TN3270E_OP_FUNCTIONS };

const char *telquals[2] = { "IS", "SEND" };
const char *reason_code[8] = { "CONN-PARTNER", "DEVICE-IN-USE", "INV-ASSOCIATE",
	"INV-NAME", "INV-DEVICE-TYPE", "TYPE-NAME-ERROR", "UNKNOWN-ERROR",
	"UNSUPPORTED-REQ" };
#define rsn(n)	(((n) <= TN3270E_REASON_UNSUPPORTED_REQ) ? \
			reason_code[(n)] : "??")
const char *function_name[5] = { "BIND-IMAGE", "DATA-STREAM-CTL", "RESPONSES",
	"SCS-CTL-CODES", "SYSREQ" };
#define fnn(n)	(((n) <= TN3270E_FUNC_SYSREQ) ? \
			function_name[(n)] : "??")
const char *data_type[9] = { "3270-DATA", "SCS-DATA", "RESPONSE", "BIND-IMAGE",
	"UNBIND", "NVT-DATA", "REQUEST", "SSCP-LU-DATA", "PRINT-EOJ" };
#define e_dt(n)	(((n) <= TN3270E_DT_PRINT_EOJ) ? \
			data_type[(n)] : "??")
const char *req_flag[1] = { " ERR-COND-CLEARED" };
#define e_rq(fn, n) (((fn) == TN3270E_DT_REQUEST) ? \
			(((n) <= TN3270E_RQF_ERR_COND_CLEARED) ? \
			req_flag[(n)] : " ??") : "")
const char *hrsp_flag[3] = { "NO-RESPONSE", "ERROR-RESPONSE",
	"ALWAYS-RESPONSE" };
#define e_hrsp(n) (((n) <= TN3270E_RSF_ALWAYS_RESPONSE) ? \
			hrsp_flag[(n)] : "??")
const char *trsp_flag[2] = { "POSITIVE-RESPONSE", "NEGATIVE-RESPONSE" };
#define e_trsp(n) (((n) <= TN3270E_RSF_NEGATIVE_RESPONSE) ? \
			trsp_flag[(n)] : "??")
#define e_rsp(fn, n) (((fn) == TN3270E_DT_RESPONSE) ? e_trsp(n) : e_hrsp(n))
const char *neg_type[4] = { "COMMAND-REJECT", "INTERVENTION-REQUIRED",
	"OPERATION-CHECK", "COMPONENT-DISCONNECTED" };
#define e_neg_type(n)	(((n) <= TN3270E_NEG_COMPONENT_DISCONNECTED) ? \
			    neg_type[n]: "??")

#if defined(HAVE_LIBSSL) /*[*/
Boolean ssl_supported = True;
Boolean secure_connection = False;
Boolean secure_unverified = False;
static SSL_CTX *ssl_ctx;
static SSL *ssl_con;
static Boolean need_tls_follows = False;
static int ssl_init(void);
#if OPENSSL_VERSION_NUMBER >= 0x00907000L /*[*/
#define INFO_CONST const
#else /*][*/
#define INFO_CONST
#endif /*]*/
static void client_info_callback(INFO_CONST SSL *s, int where, int ret);
static int continue_tls(unsigned char *sbbuf, int len);
#endif /*]*/

#if defined(_WIN32) /*[*/
#define socket_errno()	WSAGetLastError()
#define SE_EWOULDBLOCK	WSAEWOULDBLOCK
#define SE_ECONNRESET	WSAECONNRESET
#define SE_EINTR	WSAEINTR
#define SE_EAGAIN	WSAEINPROGRESS
#define SE_EPIPE	WSAECONNABORTED
#define SE_EINPROGRESS	WSAEINPROGRESS
#define SOCK_CLOSE(s)	closesocket(s)
#define SOCK_IOCTL(s, f, v)	ioctlsocket(s, f, (DWORD *)v)
#else /*][*/
#define socket_errno()	errno
#define SE_EWOULDBLOCK	EWOULDBLOCK
#define SE_ECONNRESET	ECONNRESET
#define SE_EINTR	EINTR
#define SE_EAGAIN	EAGAIN
#define SE_EPIPE	EPIPE
#if defined(EINPROGRESS) /*[*/
#define SE_EINPROGRESS	EINPROGRESS
#endif /*]*/
#define SOCK_CLOSE(s)	close(s)
#define SOCK_IOCTL	ioctl
#endif /*]*/


char *
sockerrmsg(void)
{
	static char buf[1024];

#if defined(_WIN32) /*[*/
	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    WSAGetLastError(),
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    buf,
	    sizeof(buf),
	    NULL) == 0) {

		sprintf(buf, "Windows error %d", WSAGetLastError());
	}
#else /*][*/
	strcpy(buf, strerror(errno));
#endif /*]*/
	return buf;
}

void
popup_a_sockerr(char *fmt, ...)
{
	va_list args;
	char buf[1024];

	va_start(args, fmt);
	(void) vsprintf(buf, fmt, args);
	va_end(args);
	sprintf(buf + strlen(buf), ": %s", sockerrmsg());
	errmsg("%s", buf);
}


/*
 * negotiate
 *	Initialize the connection, and negotiate TN3270 options with the host.
 */
int
negotiate(const char *host, struct sockaddr *sa, socklen_t len, int s,
	char *lu, const char *assoc)
{
	/* Save the hostname. */
    	char *h = Malloc(strlen(host) + 1);
	strcpy(h, host);
	Replace(hostname, h);
	memcpy(&host_sa.sa, sa, len);

	/* Set options for inline out-of-band data and keepalives. */
	if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (char *)&on,
		    sizeof(on)) < 0) {
		popup_a_sockerr("setsockopt(SO_OOBINLINE)");
		return -1;
	}
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
		    sizeof(on)) < 0) {
		popup_a_sockerr("setsockopt(SO_KEEPALIVE)");
		return -1;
	}

#if !defined(_WIN32) /*[*/
	/* Don't share the socket with our children. */
	(void) fcntl(s, F_SETFD, 1);
#endif /*]*/

	/* init ssl */
#if defined(HAVE_LIBSSL) /*[*/
	if (options.ssl.ssl_host && !secure_connection) {
		int rv;

		if (ssl_init() < 0)
			return -1;
		SSL_set_fd(ssl_con, s);
		rv = SSL_connect(ssl_con);
		if (rv != 1) {
			long v;

			v = SSL_get_verify_result(ssl_con);
			if (v != X509_V_OK)
				errmsg("Host certificate "
					"verification failed:\n"
					"%s (%ld)",
					X509_verify_cert_error_string(v), v);
			return -1;
		}       

		/* Check the host connection. */
		if (!check_cert_name(host)) {
			return -1;
		}

		secure_connection = True;
		vtrace("TLS/SSL tunneled connection complete.  "
			   "Connection is now secure.\n");
	}
#endif /*]*/

	/* Allocate the receive buffers. */
	if (netrbuf == NULL)
		netrbuf = (unsigned char *)Malloc(BUFSZ);
	if (ibuf == NULL)
		ibuf = (unsigned char *)Malloc(BUFSIZ);
	ibuf_size = BUFSIZ;
	ibptr = ibuf;

	/* Set up the LU list. */
	setup_lus(lu, assoc);

	/* Set up telnet options. */
	(void) memset((char *) myopts, 0, sizeof(myopts));
	(void) memset((char *) hisopts, 0, sizeof(hisopts));
	e_funcs = E_OPT(TN3270E_FUNC_BIND_IMAGE) |
		  E_OPT(TN3270E_FUNC_DATA_STREAM_CTL) |
		  E_OPT(TN3270E_FUNC_RESPONSES) |
		  E_OPT(TN3270E_FUNC_SCS_CTL_CODES) |
		  E_OPT(TN3270E_FUNC_SYSREQ);
	e_xmit_seq = 0;
	response_required = TN3270E_RSF_NO_RESPONSE;
#if defined(HAVE_LIBSSL) /*[*/
	need_tls_follows = False;
#endif /*]*/
	telnet_state = TNS_DATA;

	/* Clear statistics and flags. */
	(void) time(&ns_time);
	ns_brcvd = 0;
	ns_rrcvd = 0;
	ns_bsent = 0;
	ns_rsent = 0;
	syncing = 0;
	tn3270e_negotiated = 0;
	tn3270e_submode = E_NONE;
	tn3270e_bound = 0;

	/* Speak with the host until we suceed or give up. */
	cstate = CONNECTED_INITIAL;
	sock = s; /* hack! */
	while (!tn3270e_negotiated &&		/* TN3270E */
	       cstate != CONNECTED_3270 &&	/* TN3270 */
	       cstate != NOT_CONNECTED) {	/* gave up */

		if (net_input(s) < 0)
			return -1;
	}

	/* Success. */
	return 0;
}

int
process(int s)
{
	while (cstate != NOT_CONNECTED) {
		fd_set rfds;
		struct timeval t;
		struct timeval *tp;
		int nr;
		int maxfd = s;

		FD_ZERO(&rfds);
		FD_SET(s, &rfds);
		if (options.eoj_timeout) {
			t.tv_sec = options.eoj_timeout;
			t.tv_usec = 0;
			tp = &t;
		} else
			tp = NULL;
		if (syncsock >= 0) {
			if (syncsock > s) {
				maxfd = syncsock;
			}
			FD_SET(syncsock, &rfds);
		}
		nr = select(maxfd + 1, &rfds, NULL, NULL, tp);
		if (nr == 0 && options.eoj_timeout) {
			print_eoj();
		}
		if (nr > 0 && FD_ISSET(s, &rfds)) {
			if (net_input(s) < 0) {
				return -1;
			}
		}
		if (nr > 0 && syncsock >= 0 && FD_ISSET(syncsock, &rfds)) {
			vtrace("Input on syncsock -- exiting.\n");
			net_disconnect();
#if defined(_WIN32) /*[*/
			/* Let Windows send the TCP FIN. */
			Sleep(500);
#endif /*]*/
			pr3287_exit(0);
		}
	}
	return 0;
}

/* Disconnect from the host. */
void
net_disconnect(void)
{
	if (sock != -1) {
		vtrace("SENT disconnect\n");
		SOCK_CLOSE(sock);
		sock = -1;
#if defined(HAVE_LIBSSL) /*[*/
		if (ssl_con != NULL) {
			SSL_shutdown(ssl_con);
			SSL_free(ssl_con);
			ssl_con = NULL;
		}               
		secure_connection = False;
		secure_unverified = False;
#endif /*]*/
	}
}

/* Set up the LU list. */
static void
setup_lus(char *luname, const char *assoc)
{
	char *lu;
	char *comma;
	int n_lus = 1;
	int i;

	connected_lu = NULL;
	connected_type = NULL;
	curr_lu = (char **)NULL;
	try_lu = NULL;

	if (lus) {
		Free(lus);
		lus = (char **)NULL;
	}

	if (assoc != NULL) {
		try_assoc = NewString(assoc);
		return;
	}

	if (luname == NULL || !luname[0]) {
		return;
	}

	/*
	 * Count the commas in the LU name.  That plus one is the
	 * number of LUs to try. 
	 */
	lu = luname;
	while ((comma = strchr(lu, ',')) != NULL) {
		n_lus++;
		lu++;
	}

	/*
	 * Allocate enough memory to construct an argv[] array for
	 * the LUs.
	 */
	lus = (char **)Malloc((n_lus+1) * sizeof(char *) + strlen(luname) + 1);

	/* Copy each LU into the array. */
	lu = (char *)(lus + n_lus + 1);
	(void) strcpy(lu, luname);
	i = 0;
	do {
		lus[i++] = lu;
		comma = strchr(lu, ',');
		if (comma != NULL) {
			*comma = '\0';
			lu = comma + 1;
		}
	} while (comma != NULL);
	lus[i] = NULL;
	curr_lu = lus;
	try_lu = *curr_lu;
}


/*
 * net_input
 *	Called by the toolkit whenever there is input available on the
 *	socket.  Reads the data, processes the special telnet commands
 *	and calls process_ds to process the 3270 data stream.
 */
int
net_input(int s)
{
	register unsigned char *cp;
	ssize_t nr;

#if defined(HAVE_LIBSSL) /*[*/
	if (ssl_con != NULL)
		nr = SSL_read(ssl_con, (char *) netrbuf, BUFSZ);
	else   
#endif /*]*/
	nr = recv(s, (char *)netrbuf, BUFSZ, 0);
	if (nr < 0) {
		if (socket_errno() == SE_EWOULDBLOCK) {
			vtrace("EWOULDBLOCK\n");
			return 0;
		}
#if defined(HAVE_LIBSSL) /*[*/
                if (ssl_con != NULL) {
			unsigned long e;
			char err_buf[1024];

			e = ERR_get_error();
			(void) ERR_error_string(e, err_buf);
			vtrace("RCVD socket error %ld (%s)\n", e, err_buf);
			errmsg("SSL_read:\n%s", err_buf);
			cstate = NOT_CONNECTED;
			return -1;
		}
#endif /*]*/
		vtrace("RCVD socket error %s\n", sockerrmsg());
		popup_a_sockerr("Socket read");
		cstate = NOT_CONNECTED;
		return -1;
	} else if (nr == 0) {
		/* Host disconnected. */
		trace_str("RCVD disconnect\n");
		cstate = NOT_CONNECTED;
		return 0;
	}

	/* Process the data. */
	trace_netdata('<', netrbuf, nr);

	ns_brcvd += nr;
	for (cp = netrbuf; cp < (netrbuf + nr); cp++) {
		if (telnet_fsm(*cp)) {
			cstate = NOT_CONNECTED;
			return -1;
		}
	}
	return 0;
}



/* Advance 'try_lu' to the next desired LU name. */
static void
next_lu(void)
{
	if (curr_lu != (char **)NULL && (try_lu = *++curr_lu) == NULL)
		curr_lu = (char **)NULL;
}

/*
 * telnet_fsm
 *	Telnet finite-state machine.
 *	Returns 0 for okay, -1 for errors.
 */
static int
telnet_fsm(unsigned char c)
{
	switch (telnet_state) {
	    case TNS_DATA:	/* normal data processing */
		if (c == IAC) {	/* got a telnet command */
			telnet_state = TNS_IAC;
			break;
		}
		if (IN_ANSI && !IN_E) {
			/* NVT data? */
			;
		} else {
			store3270in(c);
		}
		break;
	    case TNS_IAC:	/* process a telnet command */
		if (c != EOR && c != IAC) {
			vtrace("RCVD %s ", cmd(c));
		}
		switch (c) {
		    case IAC:	/* escaped IAC, insert it */
			if (IN_ANSI && !IN_E) {
				;
			} else
				store3270in(c);
			telnet_state = TNS_DATA;
			break;
		    case EOR:	/* eor, process accumulated input */
			trace_str("RCVD EOR");
			if (IN_3270 || (IN_E && tn3270e_negotiated)) {
				trace_str("\n");
				ns_rrcvd++;
				if (process_eor())
					return -1;
			} else
				trace_str(" (ignored -- not in 3270 mode)\n");
			ibptr = ibuf;
			telnet_state = TNS_DATA;
			break;
		    case WILL:
			telnet_state = TNS_WILL;
			break;
		    case WONT:
			telnet_state = TNS_WONT;
			break;
		    case DO:
			telnet_state = TNS_DO;
			break;
		    case DONT:
			telnet_state = TNS_DONT;
			break;
		    case SB:
			telnet_state = TNS_SB;
			if (sbbuf == (unsigned char *)NULL)
				sbbuf = (unsigned char *)Malloc(1024);
			sbptr = sbbuf;
			break;
		    case DM:
			trace_str("\n");
			if (syncing) {
				syncing = 0;
				/* x_except_on(s); */
			}
			telnet_state = TNS_DATA;
			break;
		    case AO:
			if (IN_3270 && !IN_E) {
				trace_str("\n");
				if (print_eoj() < 0)
					tn3270_nak(PDS_FAILED);
			} else {
				trace_str(" (ignored -- not in TN3270 mode)\n");
			}
			ibptr = ibuf;
			telnet_state = TNS_DATA;
			break;
		    case GA:
		    case NOP:
			trace_str("\n");
			telnet_state = TNS_DATA;
			break;
		    default:
			trace_str(" (ignored -- unsupported)\n");
			telnet_state = TNS_DATA;
			break;
		}
		break;
	    case TNS_WILL:	/* telnet WILL DO OPTION command */
		vtrace("%s\n", opt(c));
		switch (c) {
		    case TELOPT_SGA:
		    case TELOPT_BINARY:
		    case TELOPT_EOR:
		    case TELOPT_TTYPE:
		    case TELOPT_ECHO:
		    case TELOPT_TN3270E:
			if (!hisopts[c]) {
				hisopts[c] = 1;
				do_opt[2] = c;
				net_rawout(do_opt, sizeof(do_opt));
				vtrace("SENT %s %s\n", cmd(DO), opt(c));

 				/* For UTS, volunteer to do EOR when they do. */
 				if (c == TELOPT_EOR && !myopts[c]) {
 					myopts[c] = 1;
 					will_opt[2] = c;
 					net_rawout(will_opt, sizeof(will_opt));
 					vtrace("SENT %s %s\n", cmd(WILL),
					    opt(c));
 				}

				check_in3270();
			}
			break;
		    default:
			dont_opt[2] = c;
			net_rawout(dont_opt, sizeof(dont_opt));
			vtrace("SENT %s %s\n", cmd(DONT), opt(c));
			break;
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_WONT:	/* telnet WONT DO OPTION command */
		vtrace("%s\n", opt(c));
		if (hisopts[c]) {
			hisopts[c] = 0;
			dont_opt[2] = c;
			net_rawout(dont_opt, sizeof(dont_opt));
			vtrace("SENT %s %s\n", cmd(DONT), opt(c));
			check_in3270();
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_DO:	/* telnet PLEASE DO OPTION command */
		vtrace("%s\n", opt(c));
		switch (c) {
		    case TELOPT_BINARY:
		    case TELOPT_EOR:
		    case TELOPT_TTYPE:
		    case TELOPT_SGA:
		    case TELOPT_TM:
		    case TELOPT_TN3270E:
#if defined(HAVE_LIBSSL) /*[*/
		    case TELOPT_STARTTLS:
			if (c == TELOPT_STARTTLS && !ssl_supported)
				goto wont;
#endif /*]*/
			if (!myopts[c]) {
				if (c != TELOPT_TM)
					myopts[c] = 1;
				will_opt[2] = c;
				net_rawout(will_opt, sizeof(will_opt));
				vtrace("SENT %s %s\n", cmd(WILL), opt(c));
				check_in3270();
			}
#if defined(HAVE_LIBSSL) /*[*/
			if (c == TELOPT_STARTTLS) {
				static unsigned char follows_msg[] = {
					IAC, SB, TELOPT_STARTTLS,
					TLS_FOLLOWS, IAC, SE
				};
				/*
				 * Send IAC SB STARTTLS FOLLOWS IAC SE
				 * to announce that what follows is TLS.
				 */
				net_rawout(follows_msg, sizeof(follows_msg));
				vtrace("SENT %s %s FOLLOWS %s\n",
						cmd(SB),
						opt(TELOPT_STARTTLS),
						cmd(SE));
				need_tls_follows = True;
			}
#endif /*]*/
			break;
		    wont:
		    default:
			wont_opt[2] = c;
			net_rawout(wont_opt, sizeof(wont_opt));
			vtrace("SENT %s %s\n", cmd(WONT), opt(c));
			break;
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_DONT:	/* telnet PLEASE DON'T DO OPTION command */
		vtrace("%s\n", opt(c));
		if (myopts[c]) {
			myopts[c] = 0;
			wont_opt[2] = c;
			net_rawout(wont_opt, sizeof(wont_opt));
			vtrace("SENT %s %s\n", cmd(WONT), opt(c));
			check_in3270();
		}
		telnet_state = TNS_DATA;
		break;
	    case TNS_SB:	/* telnet sub-option string command */
		if (c == IAC)
			telnet_state = TNS_SB_IAC;
		else
			*sbptr++ = c;
		break;
	    case TNS_SB_IAC:	/* telnet sub-option string command */
		*sbptr++ = c;
		if (c == SE) {
			telnet_state = TNS_DATA;
			if (sbbuf[0] == TELOPT_TTYPE &&
			    sbbuf[1] == TELQUAL_SEND) {
				int tt_len, tb_len;
				char *tt_out;

				vtrace("%s %s\n", opt(sbbuf[0]),
				    telquals[sbbuf[1]]);

				if (lus != (char **)NULL &&
                                    try_assoc == NULL &&
				    try_lu == NULL) {
					/* None of the LUs worked. */
					errmsg("Cannot connect to specified "
						"LU");
					return -1;
				}
				tt_len = strlen(termtype);
				if (try_lu != NULL && *try_lu) {
					tt_len += strlen(try_lu) + 1;
					connected_lu = try_lu;
				} else
					connected_lu = NULL;

				tb_len = 4 + tt_len + 2;
				tt_out = Malloc(tb_len + 1);
				(void) sprintf(tt_out, "%c%c%c%c%s%s%s%c%c",
				    IAC, SB, TELOPT_TTYPE, TELQUAL_IS,
				    termtype,
				    (try_lu != NULL && *try_lu) ? "@" : "",
				    (try_lu != NULL && *try_lu) ? try_lu : "",
				    IAC, SE);
				net_rawout((unsigned char *)tt_out, tb_len);

				vtrace("SENT %s %s %s %.*s %s\n",
				    cmd(SB), opt(TELOPT_TTYPE),
				    telquals[TELQUAL_IS],
				    tt_len, tt_out + 4,
				    cmd(SE));
				Free(tt_out);

				/* Advance to the next LU name. */
				next_lu();
			} else if (myopts[TELOPT_TN3270E] &&
				   sbbuf[0] == TELOPT_TN3270E) {
				if (tn3270e_negotiate())
					return -1;
			}
#if defined(HAVE_LIBSSL) /*[*/
			else if (need_tls_follows &&
					myopts[TELOPT_STARTTLS] &&
					sbbuf[0] == TELOPT_STARTTLS) {
				if (continue_tls(sbbuf, sbptr - sbbuf) < 0)
					return -1;
			}
#endif /*]*/
		} else {
			telnet_state = TNS_SB;
		}
		break;
	}
	return 0;
}

/* Send a TN3270E terminal type request. */
static void
tn3270e_request(void)
{
	int tt_len, tb_len;
	char *tt_out;
	char *t;

	tt_len = strlen(termtype);
	if (try_assoc != NULL)
		tt_len += strlen(try_assoc) + 1;
	else if (try_lu != NULL && *try_lu)
		tt_len += strlen(try_lu) + 1;

	tb_len = 5 + tt_len + 2;
	tt_out = Malloc(tb_len + 1);
	t = tt_out;
	t += sprintf(tt_out, "%c%c%c%c%c%s",
	    IAC, SB, TELOPT_TN3270E, TN3270E_OP_DEVICE_TYPE,
	    TN3270E_OP_REQUEST, termtype);

	if (try_assoc != NULL)
		t += sprintf(t, "%c%s", TN3270E_OP_ASSOCIATE, try_assoc);
	else if (try_lu != NULL && *try_lu)
		t += sprintf(t, "%c%s", TN3270E_OP_CONNECT, try_lu);

	(void) sprintf(t, "%c%c", IAC, SE);

	net_rawout((unsigned char *)tt_out, tb_len);

	vtrace("SENT %s %s DEVICE-TYPE REQUEST %.*s%s%s%s%s "
		   "%s\n",
	    cmd(SB), opt(TELOPT_TN3270E), strlen(termtype), tt_out + 5,
	    (try_assoc != NULL) ? " ASSOCIATE " : "",
	    (try_assoc != NULL) ? try_assoc : "",
	    (try_lu != NULL && *try_lu) ? " CONNECT " : "",
	    (try_lu != NULL && *try_lu) ? try_lu : "",
	    cmd(SE));

	Free(tt_out);
}

/*
 * Negotiation of TN3270E options.
 * Returns 0 if okay, -1 if we have to give up altogether.
 */
static int
tn3270e_negotiate(void)
{
#define LU_MAX	32
	static char reported_lu[LU_MAX+1];
	static char reported_type[LU_MAX+1];
	int sblen;
	unsigned long e_rcvd;

	/* Find out how long the subnegotiation buffer is. */
	for (sblen = 0; ; sblen++) {
		if (sbbuf[sblen] == SE)
			break;
	}

	vtrace("TN3270E ");

	switch (sbbuf[1]) {

	case TN3270E_OP_SEND:

		if (sbbuf[2] == TN3270E_OP_DEVICE_TYPE) {

			/* Host wants us to send our device type. */
			vtrace("SEND DEVICE-TYPE SE\n");

			tn3270e_request();
		} else {
			vtrace("SEND ??%u SE\n", sbbuf[2]);
		}
		break;

	case TN3270E_OP_DEVICE_TYPE:

		/* Device type negotiation. */
		vtrace("DEVICE-TYPE ");

		switch (sbbuf[2]) {
		case TN3270E_OP_IS: {
			int tnlen, snlen;

			/* Device type success. */

			/* Isolate the terminal type and session. */
			tnlen = 0;
			while (sbbuf[3+tnlen] != SE &&
			       sbbuf[3+tnlen] != TN3270E_OP_CONNECT)
				tnlen++;
			snlen = 0;
			if (sbbuf[3+tnlen] == TN3270E_OP_CONNECT) {
				while(sbbuf[3+tnlen+1+snlen] != SE)
					snlen++;
			}
			vtrace("IS %.*s CONNECT %.*s SE\n",
				tnlen, &sbbuf[3],
				snlen, &sbbuf[3+tnlen+1]);

			/* Remember the LU. */
			if (tnlen) {
				if (tnlen > LU_MAX)
					tnlen = LU_MAX;
				(void)strncpy(reported_type,
				    (char *)&sbbuf[3], tnlen);
				reported_type[tnlen] = '\0';
				connected_type = reported_type;
			}
			if (snlen) {
				if (snlen > LU_MAX)
					snlen = LU_MAX;
				(void)strncpy(reported_lu,
				    (char *)&sbbuf[3+tnlen+1], snlen);
				reported_lu[snlen] = '\0';
				connected_lu = reported_lu;
			}

			/* Tell them what we can do. */
			tn3270e_subneg_send(TN3270E_OP_REQUEST, e_funcs);
			break;
		}
		case TN3270E_OP_REJECT:

			/* Device type failure. */

			vtrace("REJECT REASON %s SE\n", rsn(sbbuf[4]));

			if (try_assoc != NULL) {
				errmsg("Cannot associate with specified LU: %s",
					 rsn(sbbuf[4]));
				return -1;
			}
			next_lu();
			if (try_lu != NULL) {
				/* Try the next LU. */
				tn3270e_request();
			} else if (lus != NULL) {
				/* No more LUs to try.  Give up. */
				errmsg("Cannot connect to specified LU: %s",
					rsn(sbbuf[4]));
				return -1;
			} else {
				errmsg("Device type rejected, cannot connect: "
					"%s", rsn(sbbuf[4]));
				return -1;
			}

			break;
		default:
			vtrace("??%u SE\n", sbbuf[2]);
			break;
		}
		break;

	case TN3270E_OP_FUNCTIONS:

		/* Functions negotiation. */
		vtrace("FUNCTIONS ");

		switch (sbbuf[2]) {

		case TN3270E_OP_REQUEST:

			/* Host is telling us what functions they want. */
			vtrace("REQUEST %s SE\n",
			    tn3270e_function_names(sbbuf+3, sblen-3));

			e_rcvd = tn3270e_fdecode(sbbuf+3, sblen-3);
			if ((e_rcvd == e_funcs) || (e_funcs & ~e_rcvd)) {
				/* They want what we want, or less.  Done. */
				e_funcs = e_rcvd;
				tn3270e_subneg_send(TN3270E_OP_IS, e_funcs);
				tn3270e_negotiated = 1;
				vtrace("TN3270E option negotiation "
				    "complete.\n");
				check_in3270();
			} else {
				/*
				 * They want us to do something we can't.
				 * Request the common subset.
				 */
				e_funcs &= e_rcvd;
				tn3270e_subneg_send(TN3270E_OP_REQUEST,
				    e_funcs);
			}
			break;

		case TN3270E_OP_IS:

			/* They accept our last request. */
			vtrace("IS %s SE\n",
			    tn3270e_function_names(sbbuf+3, sblen-3));
			e_rcvd = tn3270e_fdecode(sbbuf+3, sblen-3);
			if (e_rcvd != e_funcs) {
				if (e_funcs & ~e_rcvd) {
					/* They've removed something.  Fine. */
					e_funcs &= e_rcvd;
				} else {
					/*
					 * They've added something.  Abandon
					 * TN3270E, they're brain dead.
					 */
					vtrace("Host illegally added "
						"function(s), aborting "
						"TN3270E\n");
					wont_opt[2] = TELOPT_TN3270E;
					net_rawout(wont_opt, sizeof(wont_opt));
					vtrace("SENT %s %s\n", cmd(WONT),
						opt(TELOPT_TN3270E));
					myopts[TELOPT_TN3270E] = 0;
					check_in3270();
					break;
				}
			}
			tn3270e_negotiated = 1;
			vtrace("TN3270E option negotiation complete.\n");
			check_in3270();
			break;

		default:
			vtrace("??%u SE\n", sbbuf[2]);
			break;
		}
		break;

	default:
		vtrace("??%u SE\n", sbbuf[1]);
	}

	/* Good enough for now. */
	return 0;
}

/* Expand a string of TN3270E function codes into text. */
static const char *
tn3270e_function_names(const unsigned char *buf, int len)
{
	int i;
	static char text_buf[1024];
	char *s = text_buf;

	if (!len)
		return("(null)");
	for (i = 0; i < len; i++) {
		s += sprintf(s, "%s%s", (s == text_buf) ? "" : " ",
		    fnn(buf[i]));
	}
	return text_buf;
}

/* Transmit a TN3270E FUNCTIONS REQUEST or FUNCTIONS IS message. */
static void
tn3270e_subneg_send(unsigned char op, unsigned long funcs)
{
	unsigned char proto_buf[7 + 32];
	int proto_len;
	int i;

	/* Construct the buffers. */
	(void) memcpy(proto_buf, functions_req, 4);
	proto_buf[4] = op;
	proto_len = 5;
	for (i = 0; i < 32; i++) {
		if (funcs & E_OPT(i))
			proto_buf[proto_len++] = i;
	}

	/* Complete and send out the protocol message. */
	proto_buf[proto_len++] = IAC;
	proto_buf[proto_len++] = SE;
	net_rawout(proto_buf, proto_len);

	/* Complete and send out the trace text. */
	vtrace("SENT %s %s FUNCTIONS %s %s %s\n",
	    cmd(SB), opt(TELOPT_TN3270E),
	    (op == TN3270E_OP_REQUEST)? "REQUEST": "IS",
	    tn3270e_function_names(proto_buf + 5, proto_len - 7),
	    cmd(SE));
}

/* Translate a string of TN3270E functions into a bit-map. */
static unsigned long
tn3270e_fdecode(const unsigned char *buf, int len)
{
	unsigned long r = 0L;
	int i;

	/* Note that this code silently ignores options >= 32. */
	for (i = 0; i < len; i++) {
		if (buf[i] < 32)
			r |= E_OPT(buf[i]);
	}
	return r;
}

static int
process_eor(void)
{
	enum pds rv;

	if (syncing || !(ibptr - ibuf))
		return(0);

	if (IN_E) {
		tn3270e_header *h = (tn3270e_header *)ibuf;

		vtrace("RCVD TN3270E(%s%s %s %u)\n",
		    e_dt(h->data_type),
		    e_rq(h->data_type, h->request_flag),
		    e_rsp(h->data_type, h->response_flag),
		    h->seq_number[0] << 8 | h->seq_number[1]);

		switch (h->data_type) {
		case TN3270E_DT_3270_DATA:
		case TN3270E_DT_SCS_DATA:
			if ((e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)) &&
			    !tn3270e_bound)
				return 0;
			tn3270e_submode = E_3270;
			check_in3270();
			response_required = h->response_flag;
			if (h->data_type == TN3270E_DT_3270_DATA)
				rv = process_ds(ibuf + EH_SIZE,
				    (ibptr - ibuf) - EH_SIZE);
			else
				rv = process_scs(ibuf + EH_SIZE,
				    (ibptr - ibuf) - EH_SIZE);
			if (rv < 0 &&
			    response_required != TN3270E_RSF_NO_RESPONSE)
				tn3270e_nak(rv);
			else if (rv == PDS_OKAY_NO_OUTPUT &&
			    response_required == TN3270E_RSF_ALWAYS_RESPONSE)
				tn3270e_ack();
			response_required = TN3270E_RSF_NO_RESPONSE;
			return 0;
		case TN3270E_DT_BIND_IMAGE:
			if (!(e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			tn3270e_bound = 1;
			check_in3270();
			if (h->response_flag)
				tn3270e_ack();
			return 0;
		case TN3270E_DT_UNBIND:
			if (!(e_funcs & E_OPT(TN3270E_FUNC_BIND_IMAGE)))
				return 0;
			tn3270e_bound = 0;
			if (tn3270e_submode == E_3270)
				tn3270e_submode = E_NONE;
			check_in3270();
			if (print_eoj() == 0)
				rv = PDS_OKAY_NO_OUTPUT;
			else
				rv = PDS_FAILED;
			if (h->response_flag) {
				if (rv >= 0)
					tn3270e_ack();
				else
					tn3270e_nak(rv);
			}
			print_unbind();
			return 0;
		case TN3270E_DT_SSCP_LU_DATA:
		case TN3270E_DT_NVT_DATA:
			if (h->response_flag)
				tn3270e_nak(PDS_BAD_CMD);
			return 0;
		case TN3270E_DT_PRINT_EOJ:
			rv = PDS_OKAY_NO_OUTPUT;
			if (options.ignoreeoj)
				vtrace("(ignored)\n");
			else if (print_eoj() < 0)
				rv = PDS_FAILED;
			if (h->response_flag) {
				if (rv >= 0)
					tn3270e_ack();
				else
					tn3270e_nak(rv);
			}
			return 0;
		default:
			return 0;
		}
	} else {
		/* Plain old 3270 mode. */
		rv = process_ds(ibuf, ibptr - ibuf);
		if (rv < 0)
			tn3270_nak(rv);
		else
			tn3270_ack();
		return 0;
	}
}


/*
 * net_exception
 *	Called when there is an exceptional condition on the socket.
 */
void
net_exception(void)
{
	trace_str("RCVD urgent data indication\n");
	if (!syncing) {
		syncing = 1;
		/* x_except_off(); */
	}
}

/*
 * Flavors of Network Output:
 *
 *	net_output	send a 3270 record
 *	net_rawout	send telnet protocol data
 */


/*
 * net_rawout
 *	Send out raw telnet data.  We assume that there will always be enough
 *	space to buffer what we want to transmit, so we don't handle EAGAIN or
 *	EWOULDBLOCK.
 */
static void
net_rawout(unsigned const char *buf, int len)
{
	int	nw;

	trace_netdata('>', buf, len);

	while (len) {
#if defined(OMTU) /*[*/
		int n2w = len;
		int pause = 0;

		if (n2w > OMTU) {
			n2w = OMTU;
			pause = 1;
		}
#else
#		define n2w len
#endif
#if defined(HAVE_LIBSSL) /*[*/
                if (ssl_con != NULL)
			nw = SSL_write(ssl_con, (const char *) buf, n2w);
		else
#endif /*]*/    
		nw = send(sock, (const char *) buf, n2w, 0);
		if (nw < 0) {
#if defined(HAVE_LIBSSL) /*[*/ 
                        if (ssl_con != NULL) {
				unsigned long e;
				char err_buf[1024];

				e = ERR_get_error();
				(void) ERR_error_string(e, err_buf);
				vtrace("RCVD socket error %ld (%s)\n", e,
				    err_buf);
				errmsg("SSL_write:\n%s", err_buf);
				cstate = NOT_CONNECTED;
				return;
			}
#endif /*]*/
			vtrace("RCVD socket error %s\n", sockerrmsg());
			if (socket_errno() == SE_EPIPE ||
			    socket_errno() == SE_ECONNRESET) {
				cstate = NOT_CONNECTED;
				return;
			} else if (socket_errno() == SE_EINTR) {
				goto bot;
			} else {
				popup_a_sockerr("Socket write");
				cstate = NOT_CONNECTED;
				return;
			}
		}
		ns_bsent += nw;
		len -= nw;
		buf += nw;
	    bot:
#if defined(OMTU) /*[*/
		if (pause)
			sleep(1);
#endif /*]*/
		;
	}
}


/*
 * check_in3270
 *	Check for switches between NVT, SSCP-LU and 3270 modes.
 */
static void
check_in3270(void)
{
	enum cstate new_cstate = NOT_CONNECTED;
	static const char *state_name[] = {
		"unconnected",
		"pending",
		"connected initial",
		"TN3270 NVT",
		"TN3270 3270",
		"TN3270E",
		"TN3270E NVT",
		"TN3270E SSCP-LU",
		"TN3270E 3270"
	};

	if (myopts[TELOPT_TN3270E]) {
		if (!tn3270e_negotiated)
			new_cstate = CONNECTED_INITIAL_E;
		else switch (tn3270e_submode) {
		case E_NONE:
			new_cstate = CONNECTED_INITIAL_E;
			break;
		case E_NVT:
			new_cstate = CONNECTED_NVT;
			break;
		case E_3270:
			new_cstate = CONNECTED_TN3270E;
			break;
		case E_SSCP:
			new_cstate = CONNECTED_SSCP;
			break;
		}
	} else if (myopts[TELOPT_BINARY] &&
	           myopts[TELOPT_EOR] &&
	           myopts[TELOPT_TTYPE] &&
	           hisopts[TELOPT_BINARY] &&
	           hisopts[TELOPT_EOR]) {
		new_cstate = CONNECTED_3270;
	} else if (cstate == CONNECTED_INITIAL) {
		/* Nothing has happened, yet. */
		return;
	} else {
		new_cstate = CONNECTED_ANSI;
	}

	if (new_cstate != cstate) {
		int was_in_e = IN_E;

		vtrace("Now operating in %s mode.\n",
			state_name[new_cstate]);
		cstate =  new_cstate;

		/*
		 * If the user specified an association, and the host has
		 * entered TELNET NVT mode or TN3270 (non-TN3270E) mode,
		 * give up.
		 */
		if (try_assoc != NULL && !IN_E) {
			errmsg("Host does not support TN3270E, cannot "
				"associate with specified LU");
			/* No return value, gotta abort here. */
			pr3287_exit(1);
		}

		/*
		 * If we've now switched between non-TN3270E mode and
		 * TN3270E state, reset the LU list so we can try again
		 * in the new mode.
		 */
		if (lus != (char **)NULL && was_in_e != IN_E) {
			curr_lu = lus;
			try_lu = *curr_lu;
		}

		/* Allocate the initial 3270 input buffer. */
		if (new_cstate >= CONNECTED_INITIAL && !ibuf_size) {
			ibuf = (unsigned char *)Malloc(BUFSIZ);
			ibuf_size = BUFSIZ;
			ibptr = ibuf;
		}

		/* If we fell out of TN3270E, remove the state. */
		if (!myopts[TELOPT_TN3270E]) {
			tn3270e_negotiated = 0;
			tn3270e_submode = E_NONE;
			tn3270e_bound = 0;
		}
	}
}

/*
 * store3270in
 *	Store a character in the 3270 input buffer, checking for buffer
 *	overflow and reallocating ibuf if necessary.
 */
static void
store3270in(unsigned char c)
{
	if (ibptr - ibuf >= ibuf_size) {
		ibuf_size += BUFSIZ;
		ibuf = (unsigned char *)Realloc((char *)ibuf, ibuf_size);
		ibptr = ibuf + ibuf_size - BUFSIZ;
	}
	*ibptr++ = c;
}

/*
 * space3270out
 *	Ensure that <n> more characters will fit in the 3270 output buffer.
 *	Allocates the buffer in BUFSIZ chunks.
 *	Allocates hidden space at the front of the buffer for TN3270E.
 */
void
space3270out(int n)
{
	unsigned nc = 0;	/* amount of data currently in obuf */
	unsigned more = 0;

	if (obuf_size)
		nc = obptr - obuf;

	while ((nc + n + EH_SIZE) > (obuf_size + more)) {
		more += BUFSIZ;
	}

	if (more) {
		obuf_size += more;
		obuf_base = (unsigned char *)Realloc((char *)obuf_base,
			obuf_size);
		obuf = obuf_base + EH_SIZE;
		obptr = obuf + nc;
	}
}


/*
 * nnn
 *	Expands a number to a character string, for displaying unknown telnet
 *	commands and options.
 */
static const char *
nnn(int c)
{
	static char	buf[64];

	(void) sprintf(buf, "%d", c);
	return buf;
}

/*
 * cmd
 *	Expands a TELNET command into a character string.
 */
static const char *
cmd(int c)
{
	if (TELCMD_OK(c))
		return TELCMD(c);
	else
		return nnn((int)c);
}

/*
 * opt
 *	Expands a TELNET option into a character string.
 */
static const char *
opt(unsigned char c)
{
	if (TELOPT_OK(c))
		return TELOPT(c);
	else if (c == TELOPT_TN3270E)
		return "TN3270E";
#if defined(HAVE_LIBSSL) /*[*/
	else if (c == TELOPT_STARTTLS)
		return "START-TLS";
#endif /*]*/
	else
		return nnn((int)c);
}


#define LINEDUMP_MAX	32

void
trace_netdata(char direction, unsigned const char *buf, int len)
{
	int offset;
	struct timeval ts;
	double tdiff;

	if (tracef == NULL)
		return;
	(void) gettimeofday(&ts, (struct timezone *)NULL);
	if (IN_3270) {
		tdiff = ((1.0e6 * (double)(ts.tv_sec - ds_ts.tv_sec)) +
			(double)(ts.tv_usec - ds_ts.tv_usec)) / 1.0e6;
		vtrace("%c +%gs\n", direction, tdiff);
	}
	ds_ts = ts;
	for (offset = 0; offset < len; offset++) {
		if (!(offset % LINEDUMP_MAX))
			vtrace("%s%c 0x%-3x ",
			    (offset ? "\n" : ""), direction, offset);
		vtrace("%02x", buf[offset]);
	}
	vtrace("\n");
}


/*
 * net_output
 *	Send 3270 output over the network, prepending TN3270E headers and
 *	tacking on the necessary telnet end-of-record command.
 */
void
net_output(void)
{
#define BSTART	((IN_TN3270E || IN_SSCP) ? obuf_base : obuf)

	/* Set the TN3720E header. */
	if (IN_TN3270E || IN_SSCP) {
		tn3270e_header *h = (tn3270e_header *)obuf_base;

		/* Check for sending a TN3270E response. */
		if (response_required == TN3270E_RSF_ALWAYS_RESPONSE) {
			tn3270e_ack();
			response_required = TN3270E_RSF_NO_RESPONSE;
		}

		/* Set the outbound TN3270E header. */
		h->data_type = IN_TN3270E ?
			TN3270E_DT_3270_DATA : TN3270E_DT_SSCP_LU_DATA;
		h->request_flag = 0;
		h->response_flag = 0;
		h->seq_number[0] = (e_xmit_seq >> 8) & 0xff;
		h->seq_number[1] = e_xmit_seq & 0xff;
	}

	/* Count the number of IACs in the message. */
	{
		char *buf = (char *)BSTART;
		int len = obptr - BSTART;
		char *iac;
		int cnt = 0;

		while (len && (iac = memchr(buf, IAC, len)) != NULL) {
			cnt++;
			len -= iac - buf + 1;
			buf = iac + 1;
		}
		if (cnt) {
			space3270out(cnt);
			len = obptr - BSTART;
			buf = (char *)BSTART;

			/* Now quote them. */
			while (len && (iac = memchr(buf, IAC, len)) != NULL) {
				int ml = len - (iac - buf);

				memmove(iac + 1, iac, ml);
				len -= iac - buf + 1;
				buf = iac + 2;
				obptr++;
			}
		}
	}

	/* Add IAC EOR to the end and send it. */
	space3270out(2);
	*obptr++ = IAC;
	*obptr++ = EOR;
	if (IN_TN3270E || IN_SSCP) {
		vtrace("SENT TN3270E(%s NO-RESPONSE %u)\n",
			IN_TN3270E ? "3270-DATA" : "SSCP-LU-DATA", e_xmit_seq);
		if (e_funcs & E_OPT(TN3270E_FUNC_RESPONSES))
			e_xmit_seq = (e_xmit_seq + 1) & 0x7fff;
	}
	net_rawout(BSTART, obptr - BSTART);

	trace_str("SENT EOR\n");
	ns_rsent++;
#undef BSTART
}

/* Send a TN3270 positive response to the server. */
static void
tn3270_ack(void)
{
	unsigned char rsp_buf[7];
	int rsp_len = sizeof(rsp_buf);

	rsp_buf[0] = 0x01; /* SOH */
	rsp_buf[1] = 0x6c; /*  %  */
	rsp_buf[2] = 0xd9; /*  R  */
	rsp_buf[3] = 0x02; /* Device End - No Error */
	rsp_buf[4] = 0x00; /* No error */
	rsp_buf[5] = IAC;
	rsp_buf[6] = EOR;
	vtrace("SENT TN3270 PRINTER STATUS(OKAY)\n");
	net_rawout(rsp_buf, rsp_len);
}

/* Send a TN3270 negative response to the server. */
static void
tn3270_nak(enum pds rv)
{
	unsigned char rsp_buf[7];
	int rsp_len = sizeof(rsp_buf);

	rsp_buf[0] = 0x01; /* SOH */
	rsp_buf[1] = 0x6c; /*  %  */
	rsp_buf[2] = 0xd9; /*  R  */
	rsp_buf[3] = 0x04; /* Error */
	switch (rv) {
	case PDS_BAD_CMD:
		rsp_buf[4] = 0x20; /* Command Rejected (CR) */
		break;
	case PDS_BAD_ADDR:
		rsp_buf[4] = 0x04; /* Data check - invalid print data */
		break;
	case PDS_FAILED:
		rsp_buf[4] = 0x10; /* Printer not ready */
		break;
	default:
		rsp_buf[4] = 0x20; /* Command Rejected - shouldn't happen */
		break;
	}
	rsp_buf[5] = IAC;
	rsp_buf[6] = EOR;
	vtrace("SENT TN3270 PRINTER STATUS(ERROR)\n");
	net_rawout(rsp_buf, rsp_len);

	/*
	 * If we just told the host 'intervention required', tell it
	 * everything's okay now.
	 */
	if (rv == PDS_FAILED)
		tn3270_ack();
}

/* Send a TN3270E positive response to the server. */
static void
tn3270e_ack(void)
{
	unsigned char rsp_buf[9];
	tn3270e_header *h, *h_in;
	int rsp_len = EH_SIZE;

	h = (tn3270e_header *)rsp_buf;
	h_in = (tn3270e_header *)ibuf;

	h->data_type = TN3270E_DT_RESPONSE;
	h->request_flag = 0;
	h->response_flag = TN3270E_RSF_POSITIVE_RESPONSE;
	h->seq_number[0] = h_in->seq_number[0];
	h->seq_number[1] = h_in->seq_number[1];
	if (h->seq_number[1] == IAC)
		rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = TN3270E_POS_DEVICE_END;
	rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = EOR;
	vtrace("SENT TN3270E(RESPONSE POSITIVE-RESPONSE "
		"%u) DEVICE-END\n",
		h_in->seq_number[0] << 8 | h_in->seq_number[1]);
	net_rawout(rsp_buf, rsp_len);
}

/* Send a TN3270E negative response to the server. */
static void
tn3270e_nak(enum pds rv)
{
	unsigned char rsp_buf[9], r;
	tn3270e_header *h, *h_in;
	int rsp_len = EH_SIZE;

	h = (tn3270e_header *)rsp_buf;
	h_in = (tn3270e_header *)ibuf;

	h->data_type = TN3270E_DT_RESPONSE;
	h->request_flag = 0;
	h->response_flag = TN3270E_RSF_NEGATIVE_RESPONSE;
	h->seq_number[0] = h_in->seq_number[0];
	h->seq_number[1] = h_in->seq_number[1];
	if (h->seq_number[1] == IAC)
		rsp_buf[rsp_len++] = IAC;
	switch (rv) {
	default:
	case PDS_BAD_CMD:
	    rsp_buf[rsp_len++] = r = TN3270E_NEG_COMMAND_REJECT;
	    break;
	case PDS_BAD_ADDR:
	    rsp_buf[rsp_len++] = r = TN3270E_NEG_OPERATION_CHECK;
	    break;
	case PDS_FAILED:
	    rsp_buf[rsp_len++] = r = TN3270E_NEG_INTERVENTION_REQUIRED;
	    break;
	}
	rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = EOR;
	vtrace("SENT TN3270E(RESPONSE NEGATIVE-RESPONSE %u) %s\n",
		h_in->seq_number[0] << 8 | h_in->seq_number[1],
		e_neg_type(r));
	net_rawout(rsp_buf, rsp_len);

	/*
	 * If we just told the host 'intervention required', tell it
	 * everything's okay now.
	 */
	if (r == TN3270E_NEG_INTERVENTION_REQUIRED)
	    tn3270e_cleared();
}

/* Send a TN3270E error cleared indication to the host. */
static void
tn3270e_cleared(void)
{
	unsigned char rsp_buf[9];
	tn3270e_header *h;
	int rsp_len = EH_SIZE;

	h = (tn3270e_header *)rsp_buf;

	h->data_type = TN3270E_OP_REQUEST;
	h->request_flag = TN3270E_RQF_ERR_COND_CLEARED;
	h->response_flag = 0;
	h->seq_number[0] = (e_xmit_seq >> 8) & 0xff;
	h->seq_number[1] = e_xmit_seq & 0xff;

	if (h->seq_number[1] == IAC)
		rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = IAC;
	rsp_buf[rsp_len++] = EOR;
	vtrace("SENT TN3270E(REQUEST ERR-COND-CLEARED %u)\n", e_xmit_seq);
	net_rawout(rsp_buf, rsp_len);

	e_xmit_seq = (e_xmit_seq + 1) & 0x7fff;
}

/* Add a dummy TN3270E header to the output buffer. */
Boolean
net_add_dummy_tn3270e(void)
{
	tn3270e_header *h;

	if (!IN_E || tn3270e_submode == E_NONE)
		return False;

	space3270out(EH_SIZE);
	h = (tn3270e_header *)obptr;

	switch (tn3270e_submode) {
	case E_NONE:
		break;
	case E_NVT:
		h->data_type = TN3270E_DT_NVT_DATA;
		break;
	case E_SSCP:
		h->data_type = TN3270E_DT_SSCP_LU_DATA;
		break;
	case E_3270:
		h->data_type = TN3270E_DT_3270_DATA;
		break;
	}
	h->request_flag = 0;
	h->response_flag = TN3270E_RSF_NO_RESPONSE;
	h->seq_number[0] = 0;
	h->seq_number[1] = 0;
	obptr += EH_SIZE;
	return True;
}

/*
 * Add IAC EOR to a buffer.
 */
void
net_add_eor(unsigned char *buf, int len)
{
	buf[len++] = IAC;
	buf[len++] = EOR;
}

#if defined(HAVE_LIBSSL) /*[*/

/* Password callback. */
static int
passwd_cb(char *buf, int size, int rwflag, void *userdata)
{
    	if (options.ssl.key_passwd == CN) {
		errmsg("No OpenSSL private key password specified");
		return 0;
	}

	if (!strncasecmp(options.ssl.key_passwd, "string:", 7)) {
	    	/* Plaintext in the resource. */
		size_t len = strlen(options.ssl.key_passwd + 7);

		if (len > (size_t)size - 1)
		    	len = size - 1;
		strncpy(buf, options.ssl.key_passwd + 7, len);
		buf[len] = '\0';
		return len;
	} else if (!strncasecmp(options.ssl.key_passwd, "file:", 5)) {
	    	/* In a file. */
	    	FILE *f;
		char *s;

		f = fopen(options.ssl.key_passwd + 5, "r");
		if (f == NULL) {
		    	errmsg("OpenSSL private key file '%s': %s",
				options.ssl.key_passwd + 5, strerror(errno));
			return 0;
		}
		memset(buf, '\0', size);
		s = fgets(buf, size - 1, f);
		fclose(f);
		return s? strlen(s): 0;
	} else {
		errmsg("Unknown OpenSSL private key syntax '%s'",
			options.ssl.key_passwd);
		return 0;
	}
}

static int
parse_file_type(const char *s)
{
    	if (s == CN || !strcasecmp(s, "pem"))
		return SSL_FILETYPE_PEM;
	else if (!strcasecmp(s, "asn1"))
		return SSL_FILETYPE_ASN1;
	else
		return -1;
}

static char *
get_ssl_error(char *buf)
{
	unsigned long e;

	e = ERR_get_error();
	if (getenv("SSL_VERBOSE_ERRORS"))
		(void) ERR_error_string(e, buf);
	else {
		char xbuf[120];
		char *colon;

		(void) ERR_error_string(e, xbuf);
		colon = strrchr(xbuf, ':');
		if (colon != CN)
			strcpy(buf, colon + 1);
		else
			strcpy(buf, xbuf);
	}
	return buf;
}

void
ssl_base_init(void)
{
	char err_buf[120];
	int cft = SSL_FILETYPE_PEM;

#if defined(_WIN32) /*[*/
	if (ssl_dll_init() < 0) {
		/* The DLLs may not be there, or may be the wrong ones. */
		ssl_supported = False;
		return;
	}
#endif /*]*/

	/* Parse the -accepthostname option. */
	if (options.ssl.accept_hostname) {
		if (!strcasecmp(options.ssl.accept_hostname, "any") ||
		    !strcmp(options.ssl.accept_hostname, "*")) {
			accept_specified_host = True;
			accept_dnsname = "*";
		} else if (!strncasecmp(options.ssl.accept_hostname, "DNS:",
			    4) &&
			    options.ssl.accept_hostname[4] != '\0') {
			accept_specified_host = True;
			accept_dnsname = &options.ssl.accept_hostname[4];
		} else if (!strncasecmp(options.ssl.accept_hostname, "IP:",
			    3)) {
			unsigned short port;
			sockaddr_46_t ahaddr;
			socklen_t len;
			char err[256];

			if (resolve_host_and_port(&options.ssl.accept_hostname[3],
				"0", 0, &port, &ahaddr.sa, &len, err,
				sizeof(err), NULL) < 0) {

				errmsg("Invalid acceptHostname '%s': "
					"%s", options.ssl.accept_hostname,
					err);
				return;
			}
			switch (ahaddr.sa.sa_family) {
			case AF_INET:
				memcpy(&host_inaddr, &ahaddr.sin.sin_addr,
					sizeof(struct in_addr));
				host_inaddr_valid = True;
				accept_specified_host = True;
				accept_dnsname = "";
				break;
#if defined(AF_INET6) /*[*/
			case AF_INET6:
				memcpy(&host_in6addr, &ahaddr.sin6.sin6_addr,
					sizeof(struct in6_addr));
				host_in6addr_valid = True;
				accept_specified_host = True;
				accept_dnsname = "";
				break;
#endif /*]*/
			default:
				break;
			}

		} else {
			errmsg("Cannot parse acceptHostname '%s' "
				"(must be 'any' or 'DNS:name' or 'IP:addr')",
				options.ssl.accept_hostname);
			return;
		}
	}

	SSL_load_error_strings();
	SSL_library_init();
	ssl_ctx = SSL_CTX_new(SSLv23_method());
	if (ssl_ctx == NULL) {
		errmsg("SSL_CTX_new failed");
		goto fail;
	}
	SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);
	SSL_CTX_set_info_callback(ssl_ctx, client_info_callback);
	SSL_CTX_set_default_passwd_cb(ssl_ctx, passwd_cb);

	/* Pull in the CA certificate file. */
	if (options.ssl.ca_file != CN || options.ssl.ca_dir != CN) {
		if (SSL_CTX_load_verify_locations(ssl_ctx,
			    options.ssl.ca_file,
			    options.ssl.ca_dir) != 1) {
			errmsg("SSL_CTX_load_verify_locations("
					"\"%s\", \"%s\") failed:\n%s",
					options.ssl.ca_file?
					    options.ssl.ca_file: "",
					options.ssl.ca_dir?
					    options.ssl.ca_dir: "",
					get_ssl_error(err_buf));
			goto fail;
		}
	} else {
#if defined(_WIN32) /*[*/
		char *certs;

#if defined(USE_CERTS_DIR) /*[*/
		certs = Malloc(strlen(instdir) + 7);
		sprintf(certs, "%s\\certs", instdir);

		if (SSL_CTX_load_verify_locations(ssl_ctx, NULL,
			certs) != 1) {
			errmsg("SSL_CTX_load_verify_locations("
					"\"%s\", \"%s\") failed:\n%s",
					"", certs,
					get_ssl_error(err_buf));
			goto fail;
		}
#else /*][*/
		certs = Malloc(strlen(appdata) + 16);
		sprintf(certs, "%sroot_certs.txt", appdata);

		if (SSL_CTX_load_verify_locations(ssl_ctx,
			    certs, NULL) != 1) {
			errmsg("SSL_CTX_load_verify_locations("
					"\"%s\", \"%s\") failed:\n%s",
					certs, "",
					get_ssl_error(err_buf));
			goto fail;
		}
#endif /*]*/
		Free(certs);
#else /*][*/
		SSL_CTX_set_default_verify_paths(ssl_ctx);
#endif /*]*/
	}

	/* Pull in the client certificate file. */
	if (options.ssl.chain_file != CN) {
		if (SSL_CTX_use_certificate_chain_file(ssl_ctx,
			    options.ssl.chain_file) != 1) {
			errmsg("SSL_CTX_use_certificate_chain_file(\"%s\") failed:\n%s",
				options.ssl.chain_file,
				get_ssl_error(err_buf));
			goto fail;
		}
	} else if (options.ssl.cert_file != CN) {
		cft = parse_file_type(options.ssl.cert_file_type);
		if (cft == -1) {
			errmsg("Invalid OpenSSL certificate "
				"file type '%s'",
				options.ssl.cert_file_type);
			goto fail;
		}
		if (SSL_CTX_use_certificate_file(ssl_ctx,
			    options.ssl.cert_file,
			    cft) != 1) {
			errmsg("SSL_CTX_use_certificate_file(\"%s\") failed:\n%s",
				options.ssl.cert_file,
				get_ssl_error(err_buf));
			goto fail;
		}
	}

	/* Pull in the private key file. */
	if (options.ssl.key_file != CN) {
		int kft = parse_file_type(options.ssl.key_file_type);

		if (kft == -1) {
			errmsg("Invalid OpenSSL key file type "
				"'%s'",
				options.ssl.key_file_type);
			goto fail;
		}
		if (SSL_CTX_use_PrivateKey_file(ssl_ctx,
			    options.ssl.key_file,
			    kft) != 1) {
			errmsg("SSL_CTX_use_PrivateKey_file(\"%s\") failed:\n%s",
				options.ssl.key_file,
				get_ssl_error(err_buf));
			goto fail;
		}
	} else if (options.ssl.chain_file != CN) {
		if (SSL_CTX_use_PrivateKey_file(ssl_ctx,
			    options.ssl.chain_file,
			    SSL_FILETYPE_PEM) != 1) {
			errmsg("SSL_CTX_use_PrivateKey_file(\"%s\") failed:\n%s",
				options.ssl.chain_file,
				get_ssl_error(err_buf));
			goto fail;
		}
	} else if (options.ssl.cert_file != CN) {
		if (SSL_CTX_use_PrivateKey_file(ssl_ctx,
			    options.ssl.cert_file,
			    cft) != 1) {
			errmsg("SSL_CTX_use_PrivateKey_file(\"%s\") failed:\n%s",
				options.ssl.cert_file,
				get_ssl_error(err_buf));
			goto fail;
		}
	}

	/* Check the key. */
	if (options.ssl.key_file != CN &&
	    SSL_CTX_check_private_key(ssl_ctx) != 1) {
		errmsg("SSL_CTX_check_private_key failed:\n%s",
			get_ssl_error(err_buf));
		goto fail;
	}

	return;

fail:
	if (ssl_ctx != NULL) {
		SSL_CTX_free(ssl_ctx);
		ssl_ctx = NULL;
	}
	pr3287_exit(1);
}

/* Verify function. */
static int
ssl_verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	int err;
	char *why_not = CN;

	/* If OpenSSL thinks it's okay, so do we. */
	if (preverify_ok)
		return 1;

	/* Fetch the error. */
	err = X509_STORE_CTX_get_error(ctx);

	/* We might not care. */
	if (!options.ssl.verify_cert) {
		why_not = "not verifying";
	} else if (options.ssl.self_signed_ok &&
		(err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
		 err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)) {
		why_not = "self-signed okay";
	}
	if (why_not != CN) {
		vtrace("SSL_verify_callback: %s, ignoring '%s' (%d)\n",
			why_not, X509_verify_cert_error_string(err), err);
		secure_unverified = True;
		return 1;
	}

	/* Then again, we might. */
	return 0;
}

/* Hostname match function. */
static int
hostname_matches(const char *hostname, const char *cn, size_t len)
{
	/*
	 * If the name from the certificate contains an embedded NUL, then by
	 * definition it will not match the hostname.
	 */
	if (strlen(cn) < len)
		return 0;

	/*
	 * Try a direct comparison.
	 */
	if (!strcasecmp(hostname, cn))
	    	return 1;

	/*
	 * Try a wild-card comparison.
	 */
	if (!strncmp(cn, "*.", 2) &&
	    strlen(hostname) > strlen(cn + 1) &&
	    !strcasecmp(hostname + strlen(hostname) - strlen(cn + 1), cn + 1))
		return 1;

	return 0;
}

/* IP address match function. */
static int
ipaddr_matches(unsigned char *v4addr, unsigned char *v6addr,
	unsigned char *data, int len)
{
	switch (len) {
	case 4:
		if (v4addr)
			return !memcmp(v4addr, data, 4);
		break;
	case 16:
		if (v6addr)
			return !memcmp(v6addr, data, 16);
		break;
	default:
		break;
	}
	return 0;
}

/*
 * Certificate hostname expansion function.
 * Mostly, this expands NULs.
 */
static char *
expand_hostname(const char *cn, size_t len)
{
	static char buf[1024];
	int ix = 0;

	if (len > sizeof(buf) / 2 + 1)
		len = sizeof(buf) / 2 + 1;

	while (len--) {
		char c = *cn++;

		if (c)
		    	buf[ix++] = c;
		else {
		    	buf[ix++] = '\\';
		    	buf[ix++] = '0';
		}
	}
	buf[ix] = '\0';

	return buf;
}

/* Hostname validation function. */
static int
spc_verify_cert_hostname(X509 *cert, const char *hostname,
	unsigned char *v4addr, unsigned char *v6addr)
{
	int ok = 0;
	X509_NAME *subj;
	char name[256];
	GENERAL_NAMES *values;
	GENERAL_NAME *value;
	int num_an, i;
	unsigned char *dns;
	int len;

	/* Check the common name. */
	if (!ok &&
	    (subj = X509_get_subject_name(cert)) &&
	    (len = X509_NAME_get_text_by_NID(subj, NID_commonName, name,
		sizeof(name))) > 0) {

		name[sizeof(name) - 1] = '\0';
		if (!strcmp(hostname, "*") ||
		    (!v4addr && !v6addr &&
		     hostname_matches(hostname, name, len))) {
			ok = 1;
			vtrace("SSL_connect: common name %s matches "
				"hostname %s\n", name, hostname);
		} else
			vtrace("SSL_connect: non-matching common name: "
				"%s\n", expand_hostname(name, len));
	}

	/* Check the alternate names. */
	if (!ok &&
	    (values = X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0))) {
		num_an = sk_GENERAL_NAME_num(values);
		for (i = 0; i < num_an && !ok; i++) {
			value = sk_GENERAL_NAME_value(values, i);
			if (value->type == GEN_DNS) {
				len = ASN1_STRING_to_UTF8(&dns,
					value->d.dNSName);
				if (!strcmp(hostname, "*") ||
				    (!v4addr && !v6addr &&
				     hostname_matches(hostname, (char *)dns,
					 len))) {
					ok = 1;
					vtrace("SSL_connect: common name "
						"%s matches hostname %s\n",
						(char *)dns, hostname);
					OPENSSL_free(dns);
					break;
				} else
					vtrace("SSL_connect: non-matching "
						"alternate name: %s\n",
						expand_hostname((char *)dns,
						    len));
				OPENSSL_free(dns);
			} else if (value->type == GEN_IPADD) {
				int i;

				if (!strcmp(hostname, "*") ||
				    ipaddr_matches(v4addr, v6addr,
					value->d.iPAddress->data,
					value->d.iPAddress->length)) {
				    vtrace("SSL_connect: matching "
					    "alternateName IP:");
				    ok = 1;
				} else {
				    vtrace("SSL_connect: non-matching "
					    "alternateName: IP:");
				}
				switch (value->d.iPAddress->length) {
				case 4:
					for (i = 0; i < 4; i++) {
						vtrace("%s%u",
							(i > 0)? ".": "",
						  value->d.iPAddress->data[i]);
					}
					break;
				case 16:
					for (i = 0; i < 16; i+= 2) {
						vtrace("%s%u",
							(i > 0)? ":": "",
	 (value->d.iPAddress->data[i] << 8) | value->d.iPAddress->data[i + 1]);
					}
					break;
				default:
					for (i = 0;
					     i < value->d.iPAddress->length;
					     i++) {
						vtrace("%s%u",
							(i > 0)? "-": "",
						  value->d.iPAddress->data[i]);
					}
					break;
				}
				vtrace("\n");
			}
			if (ok)
				break;
		}
	}

	return ok;
}

static Boolean
is_numeric_host(const char *host)
{
	/* Is it an IPv4 address? */
#if defined(_WIN32) /*[*/
	if (inet_addr(host) != (u_long)-1)
#else /*][*/
	if (inet_addr(host) != (in_addr_t)-1)
#endif /*]*/
		return True;

# if defined(AF_INET6) /*[*/
	/*
	 * Is it an IPv6 address?
	 *
	 * The test here is imperfect, but a DNS name can't contain a colon,
	 * so if the name contains one, and getaddrinfo() succeeds, we can
	 * assume it is a numeric IPv6 address.  We add an extra level of
	 * insurance, that it only contains characters that are valid in an
	 * IPv6 numeric address (hex digits, colons and periods).
	 */
	if (strchr(host, ':') &&
	    strspn(host, ":.0123456789abcdefABCDEF") == strlen(host))
		return True;
# endif /*]*/

	return False;
}

/*
 * Check the name in the host certificate.
 *
 * Returns True if the certificate is okay (or doesn't need to be), False if
 * the connection should fail because of a bad certificate.
 */
static Boolean
check_cert_name(const char *host)
{
	X509 *cert;

	/*
	 * If they connected to a numeric host and didn't specify a name or
	 * address to compare to, set the IP or IPv6 address to compare.
	 */
	if (!accept_specified_host && is_numeric_host(host)) {
	    host_inaddr_valid = (host_sa.sa.sa_family == AF_INET);
	    if (host_inaddr_valid)
		    memcpy(&host_inaddr, &host_sa.sin.sin_addr,
			    sizeof(struct in_addr));
# if defined(AF_INET6) /*[*/
	    host_in6addr_valid = (host_sa.sa.sa_family == AF_INET6);
	    if (host_in6addr_valid)
		    memcpy(&host_in6addr, &host_sa.sin6.sin6_addr,
			    sizeof(struct in6_addr));
# endif /*]*/
	}

	cert = SSL_get_peer_certificate(ssl_con);
	if (cert == NULL) {
		if (options.ssl.verify_cert) {
			errmsg("No host certificate");
			return False;
		} else {
			secure_unverified = True;
			vtrace("No host certificate.\n");
			return True;
		}
	}

	if (!spc_verify_cert_hostname(cert,
		    accept_specified_host? accept_dnsname: host,
		    host_inaddr_valid? (unsigned char *)(void *)&host_inaddr:
				       NULL,
#if defined(AF_INET6) /*[*/
		    host_in6addr_valid? (unsigned char *)(void *)&host_in6addr:
				       NULL
#else /*][*/
		    NULL
#endif /*]*/
		    )) {
		X509_free(cert);
		if (options.ssl.verify_cert) {
			errmsg("Host certificate name(s) do not match "
				"%s", host);
			return False;
		} else {
			secure_unverified = True;
			vtrace("Host certificate name(s) do not match "
				"host.\n");
			return True;
		}
	}
	X509_free(cert);
	return True;
}

/* Initialize the OpenSSL library. */
static int
ssl_init(void)
{
	if (!ssl_supported) {
	    	errmsg("Cannot connect: SSL DLLs not found");
		return -1;
	}

	ssl_con = SSL_new(ssl_ctx);
	if (ssl_con == NULL) {
		errmsg("SSL_new failed");
		return -1;
	}
	SSL_set_verify_depth(ssl_con, 64);
	SSL_set_verify(ssl_con, SSL_VERIFY_PEER, ssl_verify_callback);

	return 0;
}

/* Callback for tracing protocol negotiation. */
static void
client_info_callback(INFO_CONST SSL *s, int where, int ret)
{
	if (where == SSL_CB_CONNECT_LOOP) {
		vtrace("SSL_connect: %s %s\n",
		    SSL_state_string(s), SSL_state_string_long(s));
	} else if (where == SSL_CB_CONNECT_EXIT) {
		if (ret == 0) {
			vtrace("SSL_connect: failed in %s %s\n",
			    SSL_state_string(s), SSL_state_string_long(s));
		} else if (ret < 0) {
			vtrace("SSL_connect: error in %s %s\n",
			    SSL_state_string(s), SSL_state_string_long(s));
		}
	}
}

/* Process a STARTTLS subnegotiation. */
static int
continue_tls(unsigned char *sbbuf, int len)
{
	/* Whatever happens, we're not expecting another SB STARTTLS. */
	need_tls_follows = False;

	/* Make sure the option is FOLLOWS. */
	if (len < 2 || sbbuf[1] != TLS_FOLLOWS) {
		/* Trace the junk. */
		vtrace("%s ? %s\n", opt(TELOPT_STARTTLS), cmd(SE));
		errmsg("TLS negotiation failure");
		return -1;
	}

	/* Trace what we got. */
	vtrace("%s FOLLOWS %s\n", opt(TELOPT_STARTTLS), cmd(SE));

	/* Initialize the SSL library. */
	if (ssl_init() < 0) {
		/* Failed. */
		return -1;
	}

	/* Set up the TLS/SSL connection. */
	SSL_set_fd(ssl_con, sock);
	if (SSL_connect(ssl_con) != 1) {
	    	long v;

		v = SSL_get_verify_result(ssl_con);
		if (v != X509_V_OK)
			errmsg("Host certificate verification failed:\n"
				"%s (%ld)",
				X509_verify_cert_error_string(v), v);

		return -1;
	}

	/* Check the host certificate. */
	if (!check_cert_name(hostname)) {
		return -1;
	}

	secure_connection = True;

	/* Success. */
	vtrace("TLS/SSL negotiated connection complete.  "
		  "Connection is now secure.\n");
	return 0;
}

#endif /*]*/
