/*~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-
 ~  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        ~
 ~  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   ~
 ~                                                                         ~
 ~  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          ~
 ~  Chastain, Michael Quan, and Mitchell Tse.                              ~
 ~                                                                         ~
 ~  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              ~
 ~  ACK!MUD is modified Merc2.0/2.1/2.2 code (c)Stephen Zepp 1998 Ver: 4.3 ~
 ~                                                                         ~
 ~  In order to use any part of this  PA  Diku Mud, you must comply with   ~
 ~  both the original Diku license in 'license.doc' as well the Merc       ~
 ~  license in 'license.txt', and the Ack!Mud license in 'ack_license.txt'.~
 ~  In particular, you may not remove any of these copyright notices.      ~
 ~                                                                         ~
 ~           _______      _____                                            ~
 ~          /  __  /\    / ___ \       222222        PA_MUD by Amnon Kruvi ~
 ~         /______/ /   / /___\ \            2       PA_MUD is modified    ~
 ~        / _______/   / _______ \           2       Ack!Mud, v4.3         ~
 ~       /_/          /_/       \_\        2                               ~
 ~                                      2                                  ~
 ~                                     2222222                             ~
 ~                                                                         ~
 ~                                                                         ~
 ~   Years of work have been invested to create DIKU, Merc, Ack and PA.    ~
 ~   Please show your respect by following the licenses, and issuing       ~
 ~   credits where due.                                                    ~
 ~                                                                         ~
 ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-*/
/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */
#ifndef unix
#define unix
#define linux
#endif
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* For child forking and stuff */
#include <sys/wait.h>
#include <unistd.h>                                         /* for execl */
#include <ctype.h>
#include "ack.h"
#include "cursor.h"

/* IMC related - Wyn */
// #define DEBUG
/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern int malloc_debug args( ( int ) );

extern int malloc_verify args( ( void ) );
#endif

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif

/*
 * Socket and TCP/IP stuff.
 */
#if     defined(macintosh) || defined(MSDOS)
const char echo_off_str [] = {'\0'};
const char echo_on_str [] = {'\0'};
const char go_ahead_str [] = {'\0'};
// MCCP
const char compress_will [] = {'\0'};
const char compress_do [] = {'\0'};
const char compress_dont [] = {'\0'};
const char compress_start [] = {'\0'};
const char compress2_will [] = {'\0'};
const char compress2_do [] = {'\0'};
const char compress2_dont [] = {'\0'};
const char compress2_start [] = {'\0'};
// End MCCP
#endif

#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char go_ahead_str[] = { IAC, GA, '\0' };
// MCCP
const char compress_will[] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
const char compress2_will[] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
const char compress_do[] = { IAC, DO, TELOPT_COMPRESS, '\0' };
const char compress2_do[] = { IAC, DO, TELOPT_COMPRESS2, '\0' };
const char compress_dont[] = { IAC, DONT, TELOPT_COMPRESS, '\0' };
const char compress2_dont[] = { IAC, DONT, TELOPT_COMPRESS2, '\0' };
// End MCCP
#endif

#define  TELOPT_MXP        '\x5B'

const unsigned char will_mxp_str[] = { IAC, WILL, TELOPT_MXP, '\0' };
const unsigned char start_mxp_str[] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };
const unsigned char do_mxp_str[] = { IAC, DO, TELOPT_MXP, '\0' };
const unsigned char dont_mxp_str[] = { IAC, DONT, TELOPT_MXP, '\0' };

/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int accept args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr *name, int namelen ) );
void bzero args( ( char *b, int length ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
int listen args( ( int s, int backlog ) );
int setsockopt args( ( int s, int level, int optname, void *optval,
				int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
#endif

#if     defined(apollo)
#include <unistd.h>
void bzero args( ( char *b, int length ) );
#endif

#if     defined(__hpux)
int accept args( ( int s, void *addr, int *addrlen ) );
int bind args( ( int s, const void *addr, int addrlen ) );
void bzero args( ( char *b, int length ) );
int getpeername args( ( int s, void *addr, int *addrlen ) );
int getsockname args( ( int s, void *name, int *addrlen ) );
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
int listen args( ( int s, int backlog ) );
int setsockopt args( ( int s, int level, int optname,
				const void *optval, int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if     defined(linux)
/* taken out for imc2 0.9 beta 3
 int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
 int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
 */
int close args( ( int fd ) );
/*
 int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
 int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
 */
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
/*
 int     listen          args( ( int s, int backlog ) );
 */
/*int     read            args( ( int fd, char *buf, int nbyte ) );*/
int select args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
int socket args( ( int domain, int type, int protocol ) );
/*int     write           args( ( int fd, char *buf, int nbyte ) );*/
#endif

#if     defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct timeval
{
	time_t tv_sec;
	time_t tv_usec;
};
#if     !defined(isascii)
#define isascii(c)              ( (c) < 0200 )
#endif
static long theKeys [4];

int gettimeofday
args( ( struct timeval *tp, void *tzp ) );
#endif

#if     defined(MIPS_OS)
extern int errno;
#endif

#if     defined(MSDOS)
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
int kbhit args( ( void ) );
#endif

#if     defined(NeXT)
int close args( ( int fd ) );
int fcntl args( ( int fd, int cmd, int arg ) );
#if     !defined(htons)
u_short htons args( ( u_short hostshort ) );
#endif
#if     !defined(ntohl)
u_long ntohl args( ( u_long hostlong ) );
#endif
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(sequent)
int accept args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr *name, int namelen ) );
int close args( ( int fd ) );
int fcntl args( ( int fd, int cmd, int arg ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
#if     !defined(htons)
u_short htons args( ( u_short hostshort ) );
#endif
int listen args( ( int s, int backlog ) );
#if     !defined(ntohl)
u_long ntohl args( ( u_long hostlong ) );
#endif
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
int setsockopt args( ( int s, int level, int optname, caddr_t optval,
				int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * This includes Solaris SYSV as well
 */

#if defined(sun)
int accept args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr *name, int namelen ) );
void bzero args( ( char *b, int length ) );
int close args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
#if defined(SYSV)
int gettimeofday args( ( struct timeval *tp, void *tzp ) );
#else
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
#endif
int listen args( ( int s, int backlog ) );
int select args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt args( ( int s, int level, int optname,
				const char *optval, int optlen ) );
ssize_t read args( ( int fd, void *buf, unsigned nbyte ) );
ssize_t write args( ( int fd, const void *buf, unsigned nbyte ) );
#else
int setsockopt args( ( int s, int level, int optname, void *optval,
				int optlen ) );
int read args( ( int fd, char *buf, int nbyte ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif
int socket args( ( int domain, int type, int protocol ) );
#endif

#if defined(ultrix)
int accept args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind args( ( int s, struct sockaddr *name, int namelen ) );
void bzero args( ( char *b, int length ) );
int close args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday args( ( struct timeval *tp, struct timezone *tzp ) );
int listen args( ( int s, int backlog ) );
int read args( ( int fd, char *buf, int nbyte ) );
int select args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
int setsockopt args( ( int s, int level, int optname, void *optval,
				int optlen ) );
int socket args( ( int domain, int type, int protocol ) );
int write args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * Global variables.
 */
DESCRIPTOR_DATA * d_next; /* Next descriptor in loop      */
FILE * fpReserve; /* Reserved file handle         */
bool merc_down; /* Shutdown                     */
bool wizlock; /* Game is wizlocked            */
bool paintlock; /* Paintball is locked          */
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time; /* Time of this pulse           */
int vehicle_count; /* Vehicle counter 		*/
long building_count; /* Building counter 		*/
int active_building_count; /* Building counter 		*/
int obj_count; /* Objects counter 		*/
int quest_objs; /* Quest Objects counter 	*/
int max_players_ever;
OBJ_DATA * vehicle_weapon; /* Vehicle weapon place-holder  */
OBJ_DATA * quest_obj[MAX_QUEST_ITEMS];
BUILDING_DATA * map_bld[MAX_MAPS][MAX_MAPS][Z_MAX]; /* Buildings "hash"	*/
VEHICLE_DATA * map_vhc[MAX_MAPS][MAX_MAPS][Z_MAX]; /* Vehicle "hash"	*/
OBJ_DATA * map_obj[MAX_MAPS][MAX_MAPS]; /* Objects "hash"	*/
CHAR_DATA * map_ch[MAX_MAPS][MAX_MAPS][Z_MAX];
char *history1; /* Channel history		*/
char *history2;
char *history3;
char *history4;
char *history5;
char *history6;
char *history7;
char *history8;
char *history9;
char *history10;
int buildings_lists[MAX_BUILDING_TYPES][MAX_POSSIBLE_BUILDING];
int MAX_BUILDING;
int guess_game;

/* port and control moved from local to global for HOTreboot - Flar */
int port;
int control;
int control2;

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos args( ( void ) );
bool read_from_descriptor args( ( DESCRIPTOR_DATA *d ) );
// MCCP
bool write_to_descriptor args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
bool write_to_descriptor_2 args( ( int desc, char *txt, int length ) );
//bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
// End MCCP
#endif

#if defined(unix)
void game_loop_unix args( ( int control ) );
int init_socket args( ( int port ) );
void new_descriptor args( ( int control ) );
bool read_from_descriptor args( ( DESCRIPTOR_DATA *d ) );
// MCCP
bool write_to_descriptor args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
bool write_to_descriptor_2 args( ( int desc, char *txt, int length ) );
//bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
// End MCCP
void init_descriptor args( ( DESCRIPTOR_DATA *dnew, int desc ) );
#endif

void talktoimms args( ( DESCRIPTOR_DATA *d, char *argument ) );

/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args( ( char *name ) );
bool check_reconnect args( ( DESCRIPTOR_DATA *d, char *name,
				bool fConn ) );
bool check_playing args( ( DESCRIPTOR_DATA *d, char *name ) );
int main args( ( int argc, char **argv ) );
void nanny args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool process_output args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void read_from_buffer args( ( DESCRIPTOR_DATA *d ) );
void stop_idling args( ( CHAR_DATA *ch ) );
void bust_a_prompt args( ( DESCRIPTOR_DATA *d ) );
void free_desc args( ( DESCRIPTOR_DATA *d ) );

/*+*/int global_port;

int main(int argc, char **argv) {
	struct timeval now_time;
	bool fCopyOver = FALSE; /* HOTreboot??? Well is it...is it???? - Flar */
	extern int abort_threshold;

	/*
	 * Init time.
	 */
	gettimeofday(&now_time, NULL);
	current_time = (time_t) now_time.tv_sec;
	strcpy(str_boot_time, ctime(&current_time));

	/*
	 * Macintosh console initialization.
	 */
#if defined(macintosh)
	console_options.nrows = 31;
	cshow( stdout );
	csetmode( C_RAW, stdin );
	cecho2file( "log file", 1, stderr );
#endif

	/*
	 * Reserve one channel for our use.
	 */
	if ((fpReserve = fopen( NULL_FILE, "r")) == NULL) {
		perror( NULL_FILE);
		exit(1);
	}

	/*
	 * Get the port number.
	 */
	port = 1234;
	if (argc > 1) {
		if (!is_number(argv[1])) {
			fprintf( stderr, "Usage: %s [port #]\n", argv[0]);
			exit(1);
		} else if ((port = atoi(argv[1])) <= 1024) {
			fprintf( stderr, "Port number must be above 1024.\n");
			exit(1);
		}
	}
	/* Check for HOTreboot parameter - Flar */
	if (argv[2] && argv[2][0]) {
		fCopyOver = TRUE;
		control = atoi(argv[3]);
	}

	else
		fCopyOver = FALSE;

	rename("../log/comlog.old", "../log/comlog.crash");
	rename("../log/comlog.txt", "../log/comlog.old");

	/*
	 * Run the game.
	 */
#if defined(macintosh) || defined(MSDOS)
	boot_db( );
	log_string( "Ack is ready to rock." );
	game_loop_mac_msdos( );
#endif

#if defined(unix)
	if (!fCopyOver) /* We already have the port if Copyovered. */
	{
		control = init_socket(port);
	}
#endif
	/*+*/global_port = port;
	if (fCopyOver)
		abort_threshold = BOOT_DB_ABORT_THRESHOLD;
	boot_db(fCopyOver);
#ifndef DEBUG
#ifndef WIN32
	init_alarm_handler();
#endif
#endif

#ifndef WIN32
	init_alarm_handler();
	//    imc_startup( "imc/" );
	//    icec_init();
#endif

	sprintf(log_buf, "Assault: High Tech War is ready on port %d.", port);
	log_string(log_buf);
	game_loop_unix(control);

	close(control);

	/*
	 * That's all, folks.
	 */
	log_string("Normal termination of game.");
	exit(0);
	return 0;
}

#if defined(unix)
int init_socket(int port) {
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	int x = 1;
	int fd;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Init_socket: socket");
		exit(1);
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x)) < 0) {
		perror("Init_socket: SO_REUSEADDR");
		close(fd);
		exit(1);
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{
		struct linger ld;

		ld.l_onoff = 1;
		ld.l_linger = 1000;

		if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
						(char *) &ld, sizeof(ld) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close( fd );
			exit( 1 );
		}
	}
#endif
	sa = sa_zero;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("Init_socket: bind");
		close(fd);
		exit(1);
	}

	if (listen(fd, 3) < 0) {
		perror("Init_socket: listen");
		close(fd);
		exit(1);
	}

	return fd;
}
#endif

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
	struct timeval last_time;
	struct timeval now_time;
	static DESCRIPTOR_DATA dcon;

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;

	/*
	 * New_descriptor analogue.
	 */
	dcon.descriptor = 0;
	dcon.connected = CON_GET_NAME;
	dcon.host = str_dup( "localhost" );
	dcon.outsize = 2000;
	dcon.outbuf = getmem( dcon.outsize );
	LINK(&dcon, first_desc, last_desc, next, prev);

	/*
	 * Send the greeting.
	 */

	// MCCP
	/* mccp: tell the client we support compression */
	write_to_buffer( dnew, compress2_will, 0 );
	write_to_buffer( dnew, compress_will, 0 );
	// End MCCP
	{
		char buf[MAX_STRING_LENGTH];
		HELP_DATA *pHelp;
		extern HELP_DATA * first_help;

		sprintf( buf, "greeting0" );

		for ( pHelp = first_help; pHelp != NULL; pHelp = pHelp->next )
		if ( !str_cmp( pHelp->keyword, buf ) )
		{
			if ( pHelp->text[0] == '.' )
			write_to_buffer( &dcon, pHelp->text +1, 0 );
			else
			write_to_buffer( &dcon, pHelp->text , 0 );
		}
		if ( sysdata.killfest )
		write_to_buffer( &dcon, "\n\rTHE GAME IS IN KILLFEST MODE!\n\rThis means everything's going wild, but nothing you do will save. So come on in and join the havoc!\n\r", 0 );
	}

	/* Main loop */
	while ( !merc_down )
	{
		DESCRIPTOR_DATA *d;

		/*
		 * Process input.
		 */
		for ( d = first_desc; d != NULL; d = d_next )
		{
			d_next = d->next;
			d->fcommand = FALSE;

#if defined(MSDOS)
			if ( kbhit( ) )
#endif
			{
				if ( d->character != NULL )
				d->character->timer = 0;
				if ( !read_from_descriptor( d ) )
				{
					if ( d->character != NULL )
					save_char_obj( d->character );
					d->outtop = 0;
					close_socket( d );
					continue;
				}
			}

			if ( d->character != NULL && d->character->wait > 0 )
			{
				--d->character->wait;
				if ( d->character->wait == 0 )
				check_queue(d->character);
				continue;
			}

			read_from_buffer( d );
			if ( d->incomm[0] != '\0' )
			{
				d->fcommand = TRUE;
				stop_idling( d->character );

				/*		if ( d->character != NULL && IS_BUSY( d->character ) )
				 continue;
				 */
				if ( d->connected == CON_PLAYING )
				if ( d->showstr_point )
				show_string( d, d->incomm );
				else
				interpret( d->character, d->incomm );
				else
				nanny( d, d->incomm );

				d->incomm[0] = '\0';
			}
		}

		/*
		 * Autonomous game motion.
		 */
		update_handler( );

		/*
		 * Output.
		 */
		for ( d = first_desc; d != NULL; d = d_next )
		{
			d_next = d->next;

			if ( ( d->fcommand || d->outtop > 0 ) )
			{
				if ( !process_output( d, TRUE ) )
				{
					if ( d->character != NULL )
					save_char_obj( d->character );
					d->outtop = 0;
					close_socket( d );
				}
			}
		}

		/*
		 * Synchronize to a clock.
		 * Busy wait (blargh).
		 */
		now_time = last_time;
		for (;; )
		{
			int delta;

#if defined(MSDOS)
			if ( kbhit( ) )
#endif
			{
				if ( dcon.character != NULL )
				dcon.character->timer = 0;
				if ( !read_from_descriptor( &dcon ) )
				{
					if ( dcon.character != NULL )
					save_char_obj( d->character );
					dcon.outtop = 0;
					close_socket( &dcon );
				}
#if defined(MSDOS)
				break;
#endif
			}

			gettimeofday( &now_time, NULL );
			delta = ( now_time.tv_sec - last_time.tv_sec ) * 1000 * 1000
			+ ( now_time.tv_usec - last_time.tv_usec );
			if ( delta >= 1000000 / PULSE_PER_SECOND )
			break;
		}
		last_time = now_time;
		current_time = (time_t) last_time.tv_sec;
	}

	return;
}
#endif

int cur_hour = 0;
int max_players = 0;
int cur_players = 0;

#if defined(unix)

/* + */
int reopen_flag;
void reopen_socket(int sig) {
	reopen_flag = 1;
	signal(SIGUSR1, reopen_socket);
}

/* + */

void game_loop_unix(int control) {
	static struct timeval null_time;
	struct timeval last_time;

	signal(SIGPIPE, SIG_IGN);
	/*+*/
	/*  On a SIGUSR1, open and close the control socket (anti-port-locking
	 *  thing) -- Spectrum
	 */
	reopen_flag = 0;
	signal(SIGUSR1, reopen_socket);

	/*+*/

	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		int maxdesc;

		/* #if defined(MALLOC_DEBUG)
		 if ( malloc_verify( ) != 1 )
		 abort( );
		 #endif  */

		/*+*/
		/* handle reopening the control socket
		 * don't worry about locking here, we assume that SIGUSR1's are
		 * relatively rare
		 */
		if (reopen_flag) {
			log_string("SIGUSR1 received, reopening control socket");
			close(control);
			control = init_socket(global_port);
			reopen_flag = 0;
		}
		/*+*/

		/*
		 * Poll all active descriptors.
		 */
		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);
		FD_SET(control, &in_set);
		//        maxdesc = UMAX( control, control2 );
		maxdesc = control;
		for (d = first_desc; d; d = d->next) {
			if ((d->flags && DESC_FLAG_PASSTHROUGH) == 0) {
				maxdesc = UMAX(maxdesc, d->descriptor);
				FD_SET(d->descriptor, &in_set);
				FD_SET(d->descriptor, &out_set);
				FD_SET(d->descriptor, &exc_set);
			} else {
				/* Check to see if child process has terminated */
				if (waitpid(d->childpid, NULL, WNOHANG) != 0) {
					/* Terminated or error */
					d->childpid = 0;
					REMOVE_BIT(d->flags, DESC_FLAG_PASSTHROUGH);
				}
			}
		}

		/* IMC */
		//        maxdesc=imc_fill_fdsets(maxdesc, &in_set, &out_set, &exc_set);
		if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
			perror("Game_loop: select: poll");
			exit(1);
		}

		if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
			perror("Game_loop: select: poll");
			exit(1);
		}

		/*
		 * New connection?
		 */

		if (FD_ISSET(control, &in_set))
			new_descriptor(control);

		/*
		 * Kick out the freaky folks.
		 */
		for (d = first_desc; d != NULL; d = d_next) {
			d_next = d->next;
			if (FD_ISSET(d->descriptor, &exc_set)) {
				FD_CLR(d->descriptor, &in_set);
				FD_CLR(d->descriptor, &out_set);
				if (d->character)
					save_char_obj(d->character);
				d->outtop = 0;
				close_socket(d);
			}
		}

		/*
		 * Process input.
		 */
		for (d = first_desc; d != NULL; d = d_next) {
			d_next = d->next;
			d->fcommand = FALSE;

			if (FD_ISSET(d->descriptor, &in_set)) {
				if (d->character != NULL)
					d->character->timer = 0;
				if (!read_from_descriptor(d)) {
					FD_CLR(d->descriptor, &out_set);
					if (d->character != NULL)
						save_char_obj(d->character);
					d->outtop = 0;
					close_socket(d);
					continue;
				}
			}

			if (d->character != NULL && d->character->wait > 0) {
				--d->character->wait;
				if (d->character->wait == 0)
					check_queue(d->character);
				continue;
			}

			read_from_buffer(d);
			if (d->incomm[0] != '\0') {
				d->fcommand = TRUE;
				stop_idling(d->character);
				d->timeout = current_time + 180; /* spec: stop idling */

				/*		if ( d->character != NULL && IS_BUSY( d->character ) )
				 continue;
				 */
				if (d->connected == CON_PLAYING)
					if (d->showstr_point)
						show_string(d, d->incomm);
					else
						interpret(d->character, d->incomm);
				else
					nanny(d, d->incomm);

				d->incomm[0] = '\0';
			}
		}

		//if (IMC)
		//	imc_loop();

		/*
		 * Autonomous game motion.
		 */
		update_handler();
		waitpid(0, 0, WNOHANG);

		/*
		 * Output.
		 */
		for (d = first_desc; d != NULL; d = d_next) {
			d_next = d->next;

			/* spec: disconnect people idling on login */
			if (d->connected < 0 && d->timeout < current_time) {
				// MCCP
				//	      write_to_descriptor(d->descriptor,
				write_to_descriptor(d,
				// End MCCP
						"Login timeout (180s)\n\r", 0);
				close_socket(d);
				continue;
			}

			// MCCP
			/*	    if ( ( d->fcommand || d->outtop > 0 )
			 &&   FD_ISSET(d->descriptor, &out_set) )
			 {
			 if ( !process_output( d, TRUE ) )
			 {
			 if ( d->character != NULL )
			 save_char_obj( d->character );
			 d->outtop   = 0;
			 close_socket( d );
			 }
			 } */

			if ((d->fcommand || d->outtop > 0 || d->out_compress)
					&& FD_ISSET(d->descriptor, &out_set)) {
				bool ok = TRUE;

				if (d->fcommand || d->outtop > 0)
					ok = process_output(d, TRUE);

				if (ok && d->out_compress)
					ok = process_compressed(d);

				if (!ok) {
					if (d->character != NULL)
						save_char_obj(d->character);
					d->outtop = 0;
					close_socket(d);
				}
			}

			// End MCCP
		}

		/*
		 * Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic.
		 */
		{
			struct timeval now_time;
			struct tm * now_bd_time;
			FILE * out_file;
			long secDelta;
			long usecDelta;

			long temp_time;

			gettimeofday(&now_time, NULL);
			/*	    now_bd_time=localtime(&(now_time.tv_sec));  */
			temp_time = (time_t) now_time.tv_sec;
			now_bd_time = localtime(&temp_time);

			if (now_bd_time->tm_hour != cur_hour) {
				cur_hour = now_bd_time->tm_hour;
				out_file = fopen("players.num", "a");
				fprintf(out_file, "%i %i %i\n", now_bd_time->tm_mday, cur_hour,
						max_players);
				fclose(out_file);
				max_players = cur_players;
			}

			usecDelta = ((int) last_time.tv_usec)
					- ((int) now_time.tv_usec) + 1000000 / PULSE_PER_SECOND;
			secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
			while (usecDelta < 0) {
				usecDelta += 1000000;
				secDelta -= 1;
			}

			while (usecDelta >= 1000000) {
				usecDelta -= 1000000;
				secDelta += 1;
			}

			if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec = secDelta;
				if (select(0, NULL, NULL, NULL, &stall_time) < 0 &&
				errno != EINTR) {
					perror("Game_loop: select: stall");
					exit(1);
				}
			}
		}

		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;
	}

	return;
}
#endif

void free_desc(DESCRIPTOR_DATA *d) {
	DESCRIPTOR_DATA *sd;

	d->snoop_by = NULL;
	for (sd = first_desc; sd; sd = sd->next)
		if (sd->snoop_by == d)
			sd->snoop_by = NULL;
	if (d->character)
		free_char(d->character);
	free_string(d->host);
	close(d->descriptor);
	if (d->showstr_head)
		qdispose(d->showstr_head, strlen(d->showstr_head)+1);
	if (d->outbuf)
		dispose(d->outbuf, d->outsize);
}

#if defined(unix)
void new_descriptor(int control) {
	static DESCRIPTOR_DATA d_zero;
	char buf[MSL] = "\0";
	DESCRIPTOR_DATA *dnew;
	BAN_DATA *pban;
	struct sockaddr_in sock;
	int desc;
	unsigned int size;

	size = sizeof(sock);
	getsockname(control, (struct sockaddr *) &sock, &size);
	if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
		perror("New_descriptor: accept");
		return;
	}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	if (fcntl(desc, F_SETFL, FNDELAY) == -1) {
		perror("New_descriptor: fcntl: FNDELAY");
		return;
	}

	/*
	 * Cons a new descriptor.
	 */
	GET_FREE(dnew, desc_free);
	*dnew = d_zero;
	init_descriptor(dnew, desc); /* Not sure is this right? */
	/*    *dnew               = d_zero; */
	dnew->descriptor = desc;
	dnew->connected = CON_GET_NAME;
	dnew->showstr_head = NULL;
	dnew->showstr_point = NULL;
	dnew->outsize = 2000;
	/*    dnew->outbuf        = getmem( dnew->outsize );  */
	dnew->flags = 0;
	dnew->childpid = 0;

	size = sizeof(sock);
	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
		perror("New_descriptor: getpeername");
		dnew->host = str_dup("(unknown)");
	} else {
		/*
		 * Would be nice to use inet_ntoa here but it takes a struct arg,
		 * which ain't very compatible between gcc and system libraries.
		 */
		int addr;

		addr = ntohl(sock.sin_addr.s_addr);
		sprintf(buf, "%d.%d.%d.%d", (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
				(addr >> 8) & 0xFF, (addr) & 0xFF);
		sprintf(log_buf, "Sock.sinaddr:  %s (%d)", buf, ntohs(sock.sin_port));
		log_string(log_buf);

		for (pban = first_ban; pban != NULL; pban = pban->next) {
			if (!str_prefix(pban->name, buf)) {
				char msg[MSL] = "\0";
				sprintf(msg, "Denying access to banned site %s", buf);
				log_string(msg);
				// MCCP
				//	        write_to_descriptor( desc,   "Your site has been banned from this Mud. If there has been a mistake, and you should not have been banned, please send an email to ack_il@yahoo.com describing your situation (Be sure to provide your IP).\n\r", 0 );
				sprintf(msg,
						"Your site has been banned from this Mud. If there has been a mistake, and you should not have been banned, please send an email to %s describing your situation (Be sure to provide your IP).\n\r",
						admin_email);
				write_to_descriptor(dnew, msg, 0);
				// End MCCP
				free_desc(dnew);
				PUT_FREE(dnew, desc_free);
				return;
			}
		}
		sprintf(log_buf, "Connection formed from %s.", buf);
		monitor_chan( NULL, log_buf, MONITOR_CONNECT);

		dnew->remote_port = ntohs(sock.sin_port);

		/* From unused to prevent possible ns lockup */
		dnew->host = str_dup(buf);
	}
	/*
	 * Init descriptor data.
	 */
	LINK(dnew, first_desc, last_desc, next, prev);

	/* spec: set initial login timeout */
	dnew->timeout = current_time + 180;

	/*
	 * Send the greeting.
	 */
	// MCCP
	/* mccp: tell the client we support compression */
	write_to_buffer(dnew, compress2_will, 0);
	// End MCCP

	{
		char buf[MAX_STRING_LENGTH];
		HELP_DATA *pHelp;
		extern HELP_DATA * first_help;

		sprintf(buf, "greeting0");

		for (pHelp = first_help; pHelp != NULL; pHelp = pHelp->next)
			if (!str_cmp(pHelp->keyword, buf)) {
				if (pHelp->text[0] == '.')
					write_to_buffer(dnew, pHelp->text + 1, 0);
				else
					write_to_buffer(dnew, pHelp->text, 0);
				break;               // so no more found through multiple copies
			}
	}

	cur_players++;
	if (cur_players > max_players)
		max_players = cur_players;

	return;
}
#endif

void init_descriptor(DESCRIPTOR_DATA *dnew, int desc) {
	static DESCRIPTOR_DATA d_zero;
	*dnew = d_zero;
	dnew->descriptor = desc;
	dnew->connected = CON_GET_NAME;
	dnew->showstr_head = NULL;
	dnew->showstr_point = NULL;
	dnew->outsize = 2000;
	dnew->outbuf = getmem(dnew->outsize);
	dnew->flags = 0;
	dnew->childpid = 0;
	dnew->mxp = FALSE;
}

void close_socket(DESCRIPTOR_DATA *dclose) {
	CHAR_DATA *ch;
	char buf[MSL] = "\0";

	if (dclose == NULL)
		return;

	if (dclose->outtop > 0)
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL) {
		write_to_buffer(dclose->snoop_by, "Your victim has left the game.\n\r",
				0);
	}

	{
		DESCRIPTOR_DATA *d;

		for (d = first_desc; d != NULL; d = d->next) {
			if (d->snoop_by == dclose)
				d->snoop_by = NULL;
		}
	}

	if ((ch = dclose->character) != NULL) {
		sprintf(log_buf, "Closing link to %s.", ch->name); // (Connected: %d)", ch->name, dclose->connected );
		log_string(log_buf);
		monitor_chan(ch, log_buf, MONITOR_CONNECT);
		if (dclose->connected == CON_PLAYING) {
			act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
			ch->desc = NULL;
			if (ch->in_vehicle) {
				/*		ch->in_vehicle->driving = NULL;
				 ch->in_vehicle = NULL;*/
				extract_vehicle(ch->in_vehicle, FALSE);
			}
			if (IS_SET(ch->act, PLR_TAG)) {
				REMOVE_BIT(ch->act, PLR_TAG);
				sprintf(buf,
						"TAG Game update:- %s has quit the TAG Game so no one is IT.\n\r",
						ch->name);
				info(buf, 5);
			}
		} else {
			free_char(dclose->character);
		}
		if (ch == NULL || dclose == NULL)
			return;

	}

	if (d_next == dclose)
		d_next = d_next->next;

	if (!dclose->is_free)
		UNLINK(dclose, first_desc, last_desc, next, prev);
	else
		return;

	if (dclose->out_compress) {
		deflateEnd(dclose->out_compress);
		dispose(dclose->out_compress_buf, COMPRESS_BUF_SIZE);
		dispose(dclose->out_compress, sizeof(z_stream));

		compressEnd(dclose, dclose->compressing);
	}

	close(dclose->descriptor);
	free_string(dclose->host);
	if (dclose->outbuf)
		dispose(dclose->outbuf, dclose->outsize);
	if (dclose->showstr_head)
		qdispose(dclose->showstr_head, strlen(dclose->showstr_head)+1);
	PUT_FREE(dclose, desc_free);

	cur_players--;

#if defined(MSDOS) || defined(macintosh)
	exit(1);
#endif
	return;
}

bool read_from_descriptor(DESCRIPTOR_DATA *d) {
	int iStart;

	/* Hold horses if pending command already. */
	if (d->incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iStart = strlen(d->inbuf);
	if (iStart >= sizeof(d->inbuf) - 10) {
		sprintf(log_buf, "%s input overflow!", d->host);
		log_string(log_buf);
		sprintf(log_buf, "input overflow by %s (%s)",
				(d->character == NULL) ? "[login]" : d->character->name,
				d->host);
		monitor_chan( NULL, log_buf, MONITOR_CONNECT);
		// MCCP
		//	write_to_descriptor( d->descriptor,
		write_to_descriptor(d,
		// End MCCP
				"\n\r SPAMMING IS RUDE, BYE BYE! \n\r", 0);
		return FALSE;
	}

	/* Snarf input. */
#if defined(macintosh)
	for (;; )
	{
		int c;
		c = getc( stdin );
		if ( c == '\0' || c == EOF )
		break;
		putc( c, stdout );
		if ( c == '\r' )
		putc( '\n', stdout );
		d->inbuf[iStart++] = c;
		if ( iStart > sizeof(d->inbuf) - 10 )
		break;
	}
#endif

#if defined(MSDOS) || defined(unix)
	for (;;) {
		int nRead;

		nRead = read(d->descriptor, d->inbuf + iStart,
				sizeof(d->inbuf) - 10 - iStart);
		if (nRead > 0) {
			iStart += nRead;
			if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
				break;
		} else if (nRead == 0) {
			log_string("EOF encountered on read.");
			return FALSE;
		} else if ( errno == EWOULDBLOCK)
			break;
		else {
			perror("Read_from_descriptor");
			return FALSE;
		}
	}
#endif

	d->inbuf[iStart] = '\0';
	return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA *d) {
	int i, j, k;
	unsigned char *p;

	/*    if ( d->character != NULL && IS_BUSY(d->character) )
	 {
	 for ( i = 0;d->inbuf[i] != '\0';i++ )
	 {
	 if ( ( d->inbuf[i] == 's' || d->inbuf[i] == 'S' )
	 &&  ( d->inbuf[i+1] == 't' || d->inbuf[i+1] == 'T' )
	 &&  ( d->inbuf[i+2] == 'o' || d->inbuf[i+2] == 'O' )
	 &&  ( d->inbuf[i+3] == 'p' || d->inbuf[i+3] == 'P' ) )
	 {
	 if ( d->character->c_sn == gsn_paradrop || d->character->c_sn == gsn_warp )
	 {
	 send_to_char( "You can't stop now!\n\r", d->character );
	 d->inbuf[i] = '\0';
	 d->inbuf[i+1] = '\0';
	 d->inbuf[i+2] = '\0';
	 d->inbuf[i+3] = '\0';
	 return;
	 }
	 send_to_char( "You stop what you were doing.\n\r", d->character );
	 d->character->c_sn = -1;
	 d->character->victim = d->character;
	 if ( d->character->c_sn == gsn_computer )
	 {
	 if ( d->character->bvictim && d->character->bvictim->value[3] > 0 )
	 d->character->bvictim->value[3] = 0;
	 d->character->bvictim = NULL;
	 }
	 d->inbuf[i] = '\0';
	 d->inbuf[i+1] = '\0';
	 d->inbuf[i+2] = '\0';
	 d->inbuf[i+3] = '\0';
	 return;
	 }
	 }
	 return;
	 }
	 */

	/*
	 * Hold horses if pending command already.
	 */
	if (d->incomm[0] != '\0')
		return;

	/*
	 Look for incoming telnet negotiation
	 */

	for (p = (unsigned char *) d->inbuf; *p; p++)
		if (*p == IAC) {
			if (memcmp(p, do_mxp_str, strlen((char *) do_mxp_str)) == 0) {
				turn_on_mxp(d);
				/* remove string from input buffer */
				memmove(p, &p[strlen((char *) do_mxp_str)],
						strlen((char *) &p[strlen((char *) do_mxp_str)]) + 1);
				p--; /* adjust to allow for discarded bytes */
			} /* end of turning on MXP */
			else if (memcmp(p, dont_mxp_str, strlen((char *) dont_mxp_str))
					== 0) {
				d->mxp = FALSE;
				if (d->character && IS_SET(d->character->config, CONFIG_MXP)) {
					REMOVE_BIT(d->character->config, CONFIG_MXP);
					save_char_obj(d->character);
				}
				/* remove string from input buffer */
				memmove(p, &p[strlen((char *) dont_mxp_str)],
						strlen((char *) &p[strlen((char *) dont_mxp_str)]) + 1);
				p--; /* adjust to allow for discarded bytes */
			} /* end of turning off MXP */

			// MCCP
			//	else if (d->inbuf[i] == (char) IAC) {
			else if (!memcmp(p, compress_do, strlen(compress_do))) {
				//                i += strlen(compress_do) - 1;
				memmove(p, &p[strlen((char *) compress_do)],
						strlen((char *) &p[strlen((char *) compress_do)]) + 1);
				p--;
				compressStart(d, TELOPT_COMPRESS);
			}
			//amnon
			else if (!memcmp(p, compress2_do, strlen(compress2_do))) {
				//                i += strlen(compress2_do) - 1;
				memmove(p, &p[strlen((char *) compress2_do)],
						strlen((char *) &p[strlen((char *) compress2_do)]) + 1);
				p--;
				compressStart(d, TELOPT_COMPRESS2);
			} else if (!memcmp(p, compress_dont, strlen(compress_dont))) {
				//                i += strlen(compress_dont) - 1;
				memmove(p, &p[strlen((char *) compress_dont)],
						strlen((char *) &p[strlen((char*) compress_dont)]) + 1);
				p--;
				compressEnd(d, TELOPT_COMPRESS);
			} else if (!memcmp(p, compress2_dont, strlen(compress2_dont))) {
				//                i += strlen(compress2_dont) - 1;
				memmove(p, &p[strlen((char *) compress2_dont)],
						strlen((char *) &p[strlen((char *) compress2_dont)])
								+ 1);
				p--;
				compressEnd(d, TELOPT_COMPRESS2);
				write_to_buffer(d, compress_will, 0);
			}
			//        }

			// End MCCP

		} /* end of finding an IAC */

	/*
	 * Look for at least one new line.
	 */
	for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (d->inbuf[i] == '\0')
			return;
		/* MCCP	else if (d->inbuf[i] == (char) IAC) {
		 if (!memcmp(&d->inbuf[i], compress_do, strlen(compress_do))) {
		 i += strlen(compress_do) - 1;
		 compressStart(d,TELOPT_COMPRESS);
		 }
		 else if (!memcmp(&d->inbuf[i], compress2_do, strlen(compress2_do))) {  //amnon
		 i += strlen(compress2_do) - 1;
		 compressStart(d,TELOPT_COMPRESS2);
		 }
		 else if (!memcmp(&d->inbuf[i], compress_dont, strlen(compress_dont))) {
		 i += strlen(compress_dont) - 1;
		 compressEnd(d,TELOPT_COMPRESS);
		 }
		 else if (!memcmp(&d->inbuf[i], compress2_dont, strlen(compress2_dont))) {
		 i += strlen(compress2_dont) - 1;
		 compressEnd(d,TELOPT_COMPRESS2);
		 }
		 }
		 */
	}

	/*
	 * Canonical input processing.
	 */
	for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (k >= MAX_INPUT_LENGTH - 2) {
			// MCCP
			//	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );
			write_to_descriptor(d, "Line too long.\n\r", 0);
			// End MCCP

			/* skip the rest of the line */
			for (; d->inbuf[i] != '\0'; i++) {
				if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;
			}
			d->inbuf[i] = '\n';
			d->inbuf[i + 1] = '\0';
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
			--k;
		else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
			d->incomm[k++] = d->inbuf[i];
	}

	/*
	 * Finish off the line.
	 */
	if (k == 0)
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */
	if ((k > 1 && k != 4) || d->incomm[0] == '!') {
		if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast)) {
			d->repeat = 0;
		} else {
			//	    if ( ++d->repeat >= 30 )
			if ( FALSE) {
				if (d->connected == CON_PLAYING) {
					sprintf(log_buf, "%s input spamming!", d->character->name);
					log_string(log_buf);
					monitor_chan( NULL, log_buf, MONITOR_CONNECT);
				}
				// MCCP
				//		write_to_descriptor( d->descriptor,
				write_to_descriptor(d,
						// End MCCP
						"\n\r***** SHUT UP!! *****\n\rDisconnecting due to input spamming (You are NOT spammed out, you are INPUT SPAMMED).\n\rIf you were speed-walking, try using the first letter of the direction instead of the whole word.\n\r\n\rInput spamming occurs when you type the same, over 1-letter word for 30 times straight.",
						0);
				close_socket(d);
				//		strcpy( d->incomm, "quit" );
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if (d->incomm[0] == '!')
		strcpy(d->incomm, d->inlast);
	else
		strcpy(d->inlast, d->incomm);

	while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;
	for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
		;

	return;
}

/*
 * Low level output function.
 */
bool process_output(DESCRIPTOR_DATA *d, bool fPrompt) {
	extern bool merc_down;
	char *ptr;
	char buf[MAX_STRING_LENGTH];
	int shown_lines = 0;
	int total_lines = 0;

	/*
	 * Bust a prompt.
	 */
	if (fPrompt && !merc_down && d->connected == CON_PLAYING) {
		if (d->showstr_point) {
			for (ptr = d->showstr_head; ptr != d->showstr_point; ptr++)
				if (*ptr == '\n')
					shown_lines++;

			total_lines = shown_lines;
			for (ptr = d->showstr_point; *ptr != '\0'; ptr++)
				if (*ptr == '\n')
					total_lines++;

			sprintf(buf,
					"%d%%) Please type (@@rc@@N)ontinue, (@@rr@@N)efresh, (@@rh@@N)elp, (@@rb@@N)ack, (@@rq@@N)uit, or @@rRETURN@@N) ",
					100 * shown_lines / total_lines);
			write_to_buffer(d, buf, 0);
		} else {
			CHAR_DATA *ch;

			ch = d->original ? d->original : d->character;
			write_to_buffer(d, "\n\r", 2);

			bust_a_prompt(d);

			if (IS_SET(ch->config, CONFIG_TELNET_GA))
				write_to_buffer(d, go_ahead_str, 0);
		}
	}
	/*
	 * Short-circuit if nothing to write.
	 */
	if (d->outtop == 0)
		return TRUE;
	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by != NULL) {
		char foo[MAX_STRING_LENGTH];
		CHAR_DATA * snoop_ch;

		snoop_ch = d->original != NULL ? d->original : d->character;
		if (snoop_ch != NULL)
			sprintf(foo, "[SNOOP:%s] ", snoop_ch->name);
		write_to_buffer(d->snoop_by, foo, 0);
		write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
	}
	/*
	 * OS-dependent output.
	 */
	// MCCP
	//        if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
	if (!write_to_descriptor(d, d->outbuf, d->outtop))
	// End MCCP
			{
		d->outtop = 0;
		return FALSE;
	} else {
		d->outtop = 0;
		return TRUE;
	}
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt(DESCRIPTOR_DATA *d) {
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	const char *str;
	const char *i = " ";
	char *point;
	CHAR_DATA *ch;

	ch = d->character;

	/* if editing a note, show a REAL simple prompt ;P */
	if (ch->position == POS_WRITING && !IS_NPC(ch)) {
		write_to_buffer(d, ">", 0);
		return;
	}

	if (ch->pcdata->o_pagelen > 0) {
		ch->pcdata->pagelen = ch->pcdata->o_pagelen;
		ch->pcdata->o_pagelen = 0;
	}
	/* If editing, show an 'info-prompt' -S- */
	if (ch->position == POS_BUILDING) {
		MOB_INDEX_DATA *mob;
		OBJ_INDEX_DATA *obj;
		char msg[MAX_STRING_LENGTH]; /* Mode */
		char msg2[MAX_STRING_LENGTH]; /* what */
		char msg3[MAX_STRING_LENGTH]; /* all */

		if (ch->act_build == ACT_BUILD_NOWT) {
			sprintf(msg, "Mode:  None");
			sprintf(msg2,
					" - Use Oedit to select object or Bedit to select a building.");
		}

		if (ch->act_build == ACT_BUILD_OEDIT) {
			sprintf(msg, "Mode: Oedit");
			if (ch->build_vnum == -1)
				sprintf(msg2, "No vnum set.");
			else {
				obj = get_obj_index(ch->build_vnum);
				if (obj != NULL)
					sprintf(msg2, "[%5d]: %s", ch->build_vnum,
							obj->short_descr);
			}
		}

		if (ch->act_build == ACT_BUILD_BEDIT) {
			sprintf(msg, "Mode: Bedit");
			if (ch->build_vnum == -1)
				sprintf(msg2, "No value set.");
			else {
				sprintf(msg2, "[%3d]:%s %s", ch->build_vnum,
						(build_table[ch->build_vnum].disabled) ? "@@e" : "",
						build_table[ch->build_vnum].name);
			}
		}

		if (ch->act_build == ACT_BUILD_MEDIT) {
			sprintf(msg, "Mode: Medit");
			if (ch->build_vnum == -1)
				sprintf(msg2, "No vnum set.");
			else {
				mob = get_mob_index(ch->build_vnum);
				if (mob != NULL)
					sprintf(msg2, "[%5d]: %s", ch->build_vnum,
							mob->short_descr);
			}
		}

		sprintf(msg3, "@@N< %s %s @@N>", msg, msg2);
		write_to_buffer(d, msg3, 0);
		return;
	} else if (ch->position == POS_HACKING) {
		int i;
		if (ch->bvictim == NULL) {
			send_to_char("System Error. Please contact an administrator.\n\r",
					ch);
			ch->position = POS_STANDING;
			return;
		}
		if (ch->bvictim->active == FALSE) {
			send_to_char("Mainframe shut down. Connection Terminated...\n\r",
					ch);
			ch->position = POS_STANDING;
			ch->bvictim->value[8] = 0;
			ch->bvictim = NULL;
			return;
		}
		char msg[MSL] = "\0";
		int j = 0;
		if (ch->c_obj && ch->c_obj->value[8] > 0)
			j = ((1000 / ch->c_obj->value[8]) * ch->c_obj->value[1]) / 100;
		msg[0] = '\0';
		for (i = 1; i <= 10; i++)
			sprintf(msg + strlen(msg), "%s@@i|", (i < j) ? "@@r" : "@@e");
		sprintf(msg + strlen(msg), "@@N @@r%s's %s (%d)@@N\\>",
				ch->bvictim->owned, ch->bvictim->name, ch->bvictim->level);
		write_to_buffer(d, msg, 0);
		if (IS_BUSY(ch)) {
			if (ch->c_sn == gsn_move)
				send_to_char(" @@d(@@eRUNNING@@d)@@N", ch);
			else
				send_to_char(" @@d(@@eBusy@@d)@@N", ch);
		}
		if (practicing(ch))
			send_to_char(" @@d(@@WPRACTICING@@d)@@N", ch);
		return;
	} else if (ch->position == POS_SPACE_COM) {
		if (ch->in_vehicle == NULL) {
			send_to_char("System Error. Please contact an administrator.\n\r",
					ch);
			if (ch->position != POS_DEAD)
				ch->position = POS_STANDING;
			return;
		}
		char msg[MSL] = "\0";
		char enemy[MSL] = "\0";
		sprintf(enemy, " @@N");
		if (ch->in_vehicle->state == VEHICLE_STATE_CHARGE) {
			int j, i;
			char charge[MSL] = "\0";
			sprintf(charge, "@@r@@i@@y");
			j = (ch->in_vehicle->value[0] / 2.5);
			for (i = 0; i < j; i++)
				sprintf(charge + strlen(charge), "-");
			sprintf(charge + strlen(charge), "@@2@@e");
			for (i = j; i < 40; i++)
				sprintf(charge + strlen(charge), "*");
			sprintf(charge + strlen(charge), "@@N @@W(@@a%d%%@@W)\n\r",
					ch->in_vehicle->value[0]);
			send_to_char(charge, ch);
		}
		if (ch->victim != ch) {
			VEHICLE_DATA *vhc = ch->victim->in_vehicle;
			if (vhc)
				sprintf(enemy, "@@e < @@R[@@e%s@@R]@@e %d/%d/%d [%s (%s)@@N",
						(ch->in_vehicle->state == VEHICLE_STATE_NORMAL) ?
								"Normal" :
						(ch->in_vehicle->state == VEHICLE_STATE_EVADE) ?
								"Evasive" :
						(ch->in_vehicle->state == VEHICLE_STATE_DEFENSE) ?
								"Defensive" :
						(ch->in_vehicle->state == VEHICLE_STATE_OFFENSE) ?
								"Offensive" : "Charging", vhc->fuel, vhc->ammo,
						vhc->hit, vhc->desc, vhc->driving->name);
		}
		sprintf(msg, "@@r%s> %d/%d/%d @@G[@@r%s@@G]@@r>%s",
				ch->in_vehicle->desc, ch->in_vehicle->hit, ch->in_vehicle->ammo,
				ch->in_vehicle->fuel,
				(ch->in_vehicle->state == VEHICLE_STATE_NORMAL) ? "Normal" :
				(ch->in_vehicle->state == VEHICLE_STATE_EVADE) ? "Evasive" :
				(ch->in_vehicle->state == VEHICLE_STATE_DEFENSE) ? "Defensive" :
				(ch->in_vehicle->state == VEHICLE_STATE_OFFENSE) ?
						"Offensive" : "Charging", enemy);
		write_to_buffer(d, msg, 0);
		if (IS_BUSY(ch)) {
			if (ch->c_sn == gsn_move)
				send_to_char(" @@d(@@eRUNNING@@d)@@N", ch);
			else
				send_to_char(" @@d(@@eBusy@@d)@@N", ch);
		}
		return;
	} else if (ch->position == POS_ENGINEERING) {
		if (ch->in_vehicle == NULL) {
			send_to_char("System Error. Please contact an administrator.\n\r",
					ch);
			ch->position = POS_STANDING;
			return;
		}
		char msg[MSL] = "\0";
		sprintf(msg, "Working on: %s > ", ch->in_vehicle->desc);
		write_to_buffer(d, msg, 0);
		if (IS_BUSY(ch))
			send_to_char(" @@d(@@eBusy@@d)@@N", ch);
		return;
	}

	/* set default prompt to ">" -Uni */
	if (!IS_SET(ch->config, CONFIG_PROMPT)) {
		sprintf(buf2, "\n\r");
		if (!IS_SET(ch->config, CONFIG_BLIND))
			send_to_char("@@N> ", ch);
		return;
	}

	point = buf;
	str = d->original != NULL ? d->original->prompt : d->character->prompt;
	sprintf(buf2, "%s", "@@N");
	i = buf2;
	while ((*point = *i) != '\0')
		++point, ++i;
	while (*str != '\0') {
		if (*str != '%') {
			*point++ = *str++;
			continue;
		}
		++str;
		switch (*str) {
		default:
			i = "> ";
			break;
		case 'h':
			if (ch->in_vehicle && ch->in_vehicle->type == VEHICLE_MECH)
				sprintf(buf2, "%d", ch->in_vehicle->hit);
			else
				sprintf(buf2, "%d", ch->hit);
			i = buf2;
			break;
		case 'H':
			if (ch->in_vehicle && ch->in_vehicle->type == VEHICLE_MECH)
				sprintf(buf2, "%d", ch->in_vehicle->max_hit);
			else
				sprintf(buf2, "%d", ch->max_hit);
			i = buf2;
			break;
		case 'm':
			if (ch->in_vehicle)
				sprintf(buf2, "%d", ch->in_vehicle->ammo);
			else
				sprintf(buf2, "0");
			i = buf2;
			break;
		case 'M':
			if (ch->in_vehicle)
				sprintf(buf2, "%d", ch->in_vehicle->max_ammo);
			else
				sprintf(buf2, "0");
			i = buf2;
			break;
		case 'f':
			if (ch->in_vehicle)
				sprintf(buf2, "%d", ch->in_vehicle->fuel);
			else
				sprintf(buf2, "0");
			i = buf2;
			break;
		case 'F':
			if (ch->in_vehicle)
				sprintf(buf2, "%d", ch->in_vehicle->max_fuel);
			else
				sprintf(buf2, "0");
			i = buf2;
			break;
		case 'v':
			if (ch->in_vehicle)
				sprintf(buf2, "%d", ch->in_vehicle->hit);
			else
				sprintf(buf2, "0");
			i = buf2;
			break;
		case 'V':
			if (ch->in_vehicle)
				sprintf(buf2, "%d", ch->in_vehicle->max_hit);
			else
				sprintf(buf2, "0");
			i = buf2;
			break;
		case 'x':
			sprintf(buf2, "%d", ch->pcdata->experience);
			i = buf2;
			break;
		case 'E':
			if (ch->in_building != NULL)
				sprintf(buf2, "%s%s%s%s", (ch->in_building->exit[0]) ? "N" : "",
						(ch->in_building->exit[1]) ? "E" : "",
						(ch->in_building->exit[2]) ? "S" : "",
						(ch->in_building->exit[3]) ? "W" : "");
			else
				sprintf(buf2, "-");
			i = buf2;
			break;
		case '%':
			sprintf(buf2, "%%");
			i = buf2;
			break;
		case 's':
			sprintf(buf2, "%s%s",
					wildmap_table[map_table.type[ch->x][ch->y][ch->z]].color,
					wildmap_table[map_table.type[ch->x][ch->y][ch->z]].name);
			i = buf2;
			break;
		case 'c':
			sprintf(buf2, "\n\r");
			i = buf2;
			break;
		case 'C':
			if ((ch->z == Z_GROUND || ch->z == Z_NEWBIE || ch->z == Z_AIR)
					|| IS_IMMORTAL(ch))
				sprintf(buf2, "%d/%d", ch->x, ch->y);
			else
				sprintf(buf2, "?/?");
			i = buf2;
			break;
		case 'd':
			sprintf(buf2, "%d", ch->medaltimer);
			i = buf2;
			break;
		case 'r':
			sprintf(buf2, "%s", (ch->victim != ch) ? ch->victim->name : "");
			i = buf2;
			break;
		case 'T':
			sprintf(buf2, "%d", ch->fighttimer / 8);
			i = buf2;
			break;
		case 't':
			sprintf(buf2, "%d",
					(ch->fighttimer / 8 > 60) ? (ch->fighttimer / 8) - 60 : 0);
			i = buf2;
			break;
		case 'q':
			sprintf(buf2, "%d", my_get_minutes(ch, FALSE));
			i = buf2;
			break;
		case 'Q':
			sprintf(buf2, "%d", ch->quest_points);
			i = buf2;
			break;
		case '!':
			sprintf(buf2, "%s", (char *) ctime(&current_time));
			i = buf2;
			break;
		}
		++str;
		while ((*point = *i) != '\0')
			++point, ++i;
	}

	write_to_buffer(d, buf, point - buf);
	if (IS_BUSY(ch)) {
		if (ch->c_sn == gsn_move)
			send_to_char(" @@d(@@eRUNNING@@d)@@N", ch);
		else
			send_to_char(" @@d(@@eBusy@@d)@@N", ch);
	}
	if (practicing(ch))
		send_to_char(" @@d(@@WPRACTICING@@d)@@N", ch);
	return;
}

/*
 * Append onto an output buffer.
 */

#define color_MARGIN 20

void write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, int length) {
	if (d == NULL || !d)
		return;

	if (d->outbuf == NULL) {
		bugf("write_to_buffer with NULL outbuf, string=%s", txt);
		return;
	}

	/*
	 * Find length in case caller didn't.
	 */
	if (length <= 0)
		length = strlen(txt);

	/*
	 * Initial \n\r if needed.
	 */
	if (d->outtop == 0 && !d->fcommand) {
		d->outbuf[0] = '\n';
		d->outbuf[1] = '\r';
		d->outtop = 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while (d->outtop + length + 1 + color_MARGIN >= d->outsize) {
		char *outbuf;

		outbuf = getmem(2 * d->outsize);
		strncpy(outbuf, d->outbuf, d->outtop);
		dispose(d->outbuf, d->outsize);
		d->outbuf = outbuf;
		d->outsize *= 2;
	}

	/*
	 * Copy.
	 */

	convert_mxp_tags(d->mxp, d->outbuf + d->outtop, txt,
			length + count_mxp_tags(d->mxp, txt, length));

	char c;
	char lookup;
	char *dest;
	int count = length;
	CHAR_DATA * ch;
	char * colstr;
	int collen, cnt;

	dest = d->outbuf + d->outtop;

	for (; count > 0;) {
		c = *(txt++);
		if (c != '@') {
			*(dest++) = c;
			count--;
			continue;
		} else {
			if (*txt != '@') {
				*(dest++) = c;
				count--;
				continue;
			}

			txt++;
			c = *(txt++);
			length -= 3;
			count = count - 3;

			if (c == '@') {
				length++;
				*(dest++) = c;
				continue;
			}

			ch = d->original != NULL ? d->original : d->character;
			if (ch != NULL && !IS_SET(ch->config, CONFIG_COLOR))
				continue;
			lookup = c;
			if (ch != NULL) {
				if (lookup == '!')
					lookup = ch->pcdata->hicol;
				else if (lookup == '.')
					lookup = ch->pcdata->dimcol;
			}

			for (cnt = 0; cnt < MAX_ANSI; cnt++)
				if (ansi_table[cnt].letter == lookup)
					break;

			//SARS Wacky Colors :P
			if (ch != NULL && ch->disease > 0
					&& number_percent() < 20 - ch->disease)
				cnt = number_range(0, 29);

			if (cnt == 7 && IS_SET(ch->config, CONFIG_NOBLACK))
				cnt = 12;
			if (cnt == MAX_ANSI) {
				colstr = ansi_table[10].value;
				collen = ansi_table[10].stlen;
			} else {
				if (cnt == 28) {
					cnt = number_range(0, 16);
					if (cnt == 7)
						cnt = 12;
				}
				colstr = ansi_table[cnt].value;
				collen = ansi_table[cnt].stlen;
			}

			while (d->outtop + length + collen + 1 >= d->outsize) {
				char *outbuf;

				outbuf = getmem(2 * d->outsize);
				strncpy(outbuf, d->outbuf, d->outtop + length - count);
				dispose(d->outbuf, d->outsize);
				d->outbuf = outbuf;
				d->outsize *= 2;
			}

			dest = d->outbuf + d->outtop + length - count;
			strncpy(dest, colstr, collen);
			dest += collen;

			length = length + collen;
		}
	}

	/* Make sure we have a \0 at the end */
	*(d->outbuf + d->outtop + length) = '\0';

	d->outtop += length;
	return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
// MCCP
//bool write_to_descriptor( int desc, char *txt, int length )
bool write_to_descriptor_2(int desc, char *txt, int length)
// End MCCP
{
	int iStart;
	int nWrite;
	int nBlock;

#if defined(macintosh) || defined(MSDOS)
	if ( desc == 0 )
	desc = 1;
#endif

	if (length <= 0)
		length = strlen(txt);

	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);
		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
			perror("Write_to_descriptor");
			return FALSE;
		}
	}

	return TRUE;
}

// MCCP
/* mccp: write_to_descriptor wrapper */
bool write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length) {
	if (d->out_compress)
		return write_compressed(d, txt, length);
	else
		return write_to_descriptor_2(d->descriptor, txt, length);
}

// End MCCP

void show_smenu_to(DESCRIPTOR_DATA *d) {
	write_to_buffer(d,
			"@@W----------------------------------------------------------------------------\n\r|@@g                                                                          @@W|\n\r|@@g Your character's gender has no effect on the gameplay. It is merely      @@W|\n\r|@@g  there so people know how to address you.                                @@W|\n\r----------------------------------------------------------------------------@@N\n\r",
			0);
	write_to_buffer(d,
			"\n\r@@cAre you @@a(@@rM@@a)@@cale or @@a(@@rF@@a)@@cemale: ", 0);
	return;
}

void show_pmenu_to(DESCRIPTOR_DATA *d) {
	int i;
	char buf[MSL] = "\0";
	write_to_buffer(d,
			"\n\r@@W----------------------------------------------------------------------------\n\r|@@g                                                                          @@W|\n\r|@@g There are two playing grids for you to pick from.                        @@W|\n\r|@@g                                                                          @@W|\n\r|@@g If you are new to the game, and would just like to learn how to build    @@W|\n\r|@@g  without any interruptions, select the Newbie grid. However, you will,   @@W|\n\r|@@g  sooner or later, have to reset your character and move on to the normal @@W|\n\r|@@g  playfield.                                                              @@W|\n\r----------------------------------------------------------------------------@@N\n\r\n\r",
			0);
	buf[0] = '\0';
	for (i = 0; planet_table[i].name != NULL; i++) {
		if (planet_table[i].system == 0)
			continue;
		sprintf(buf + strlen(buf), "@@a%s @@c- %s\n\r", planet_table[i].name,
				planet_table[i].note);
	}
	sprintf(buf + strlen(buf), "\n\r@@cPlease select your starting grid: ");
	write_to_buffer(d, buf, 0);
	return;
}

void show_mdmenu_to(DESCRIPTOR_DATA *d) {
	write_to_buffer(d,
			"\n\r@@W----------------------------------------------------------------------------\n\r|@@e                                 Game Mode                                @@W|\n\r|                                                                          |\n\r|@@g Selecting a game mode will allow you to receive extra benifits depending @@W|\n\r|@@g  on your experience in the game.                                         @@W|\n\r|                                                                          |\n\r|@@a Basic Mode (For New players ONLY):                                       @@W|\n\r|                                                                          |\n\r|@@g * Buildings you lose will respawn after combat.                          @@W|\n\r|@@g * Players attacking you will not gain ranks.                             @@W|\n\r|@@g * You cannot gain ranks.                                                 @@W|\n\r|@@g * You cannot join alliances.                                             @@W|\n\r|                                                                          |\n\r|@@a Normal Mode:                                                             @@W|\n\r|                                                                          |\n\r|@@g * Buildings stay destroyed.                                              @@W|\n\r|@@g * You get ranks, and people who attack you get ranks.                    @@W|\n\r|@@g * You can join alliances, and even create them at rank 20.               @@W|\n\r----------------------------------------------------------------------------@@N\n\r",
			0);
	write_to_buffer(d,
			"\n\r@@cPlease select: @@aNormal @@cor @@aBasic @@cplay mode. ", 0);
}

void show_cmenu_to(DESCRIPTOR_DATA *d) {
	int i;
	CHAR_DATA *ch = d->character;
	char buf[MSL] = "\0";
	int rank = get_rank(ch);

	write_to_buffer(d,
			"@@W----------------------------------------------------------------------------\n\r|@@g                                                                          @@W|\n\r|@@g Unlike normal Muds, the classes here are not something you advance in,   @@W|\n\r|@@g  rather they represent certain gameplay bonuses.                         @@W|\n\r|@@g                                                                          @@W|\n\r|@@g The system here is similar to Counter Strike's, meaning you can change   @@W|\n\r|@@g  your class only if you die. As you play and gain ranks in the game,     @@W|\n\r|@@g  more powerful classes will be unlocked for you to pick from.            @@W|\n\r\n\r|@@g                                                                          @@W|\n\r|@@g If you are a new player, it is recommended you pick either the           @@W|\n\r|@@g  Engineering or the Miner classes.                                       @@W|\n\r----------------------------------------------------------------------------@@N\n\r\n\r",
			0);
	sprintf(buf, "\n\r@@NClasses available to you:\n\r\n\r");
	for (i = 0; i < MAX_CLASS; i++) {
		if (i == CLASS_SCANNER)
			continue;
		if (class_table[i].rank <= rank)
			sprintf(buf + strlen(buf), "%s%s - %s: %s\n\r",
					(class_table[i].rec) ? "@@a" : "@@c",
					class_table[i].who_name, class_table[i].name,
					class_table[i].desc);
	}
	send_to_char(buf, ch);
	send_to_char("\n\r@@cPick your class: ", ch);
	return;
}

void show_bmenu_to(DESCRIPTOR_DATA *d) {
	int i;
	CHAR_DATA *ch = d->character;
	char buf[MSL] = "\0";

	//   send_to_char( "\n\r\n\r@@cYou have the ability to pick a starting bonus. Just type the name of the item you wish to start with.\n\r\n\r", ch );
	write_to_buffer(d,
			"@@W----------------------------------------------------------------------------\n\r|@@g                                                                          @@W|\n\r|@@g You may start with one of the items on the list. None of them will       @@W|\n\r|@@g  determine anything in the long run, they are just nice items to have.   @@W|\n\r----------------------------------------------------------------------------@@N\n\r\n\r",
			0);
	for (i = 0; bonus_table[i].item != -1; i++) {
		sprintf(buf, "@@a%-13s @@c- %s\n\r", bonus_table[i].name,
				bonus_table[i].desc);
		send_to_char(buf, ch);
	}
	send_to_char("\n\rWhich would you like to have? ", ch);
	return;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA *d, char *argument) {
	char buf[MAX_STRING_LENGTH];
	char msg[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	char *pwdnew;
	char *p;
	int i;
	int lines;
	bool fOld;

	while (isspace(*argument))
		argument++;

	ch = d->character;

	{
		char *argument2 = argument;
		char arg2[MSL] = "\0";

		one_argument(argument2, arg2);
		if (d->connected != CON_GET_NAME && !str_cmp(arg2, "talktoimms")) {
			talktoimms(d, argument2);
			return;
		}
	}
	if (d->connected == CON_GET_NAME) {
		if (argument[0] == '\0') {
			//	    close_socket( d );
			return;
		}

		argument = capitalize(argument);
		argument[0] = UPPER(argument[0]);

		//        do_pipe(motdbuf,"tail -n 100 ../information/connect.txt");
		//        write_to_buffer(d, motdbuf, 0);
		sprintf(buf, "%s provided as name from login from site %s.", argument,
				d->host);
		if (str_cmp(argument, admin))
			monitor_chan( NULL, buf, MONITOR_CONNECT);

		if (!check_parse_name(argument)) {
			sprintf(buf, "Illegal name %s from site %s.", argument, d->host);
			monitor_chan( NULL, buf, MONITOR_CONNECT);
			write_to_buffer(d, "Illegal name, try another.\n\rName: ", 0);
			return;
		}

		{
			DESCRIPTOR_DATA *dd;
			CHAR_DATA *wch;
			for (wch = first_char; wch; wch = wch->next) {
				if (!wch)
					continue;
				if (!str_cmp(wch->name, argument))
					continue;
				if (IS_NPC(wch))
					continue;
				if (can_multiplay(wch) || wch->fake)
					continue;
				if (!str_cmp(d->host, wch->pcdata->host) && (wch->trust < 80)) {
					char buf[MSL] = "\0";
					sprintf(buf,
							"%s is already logged on from your IP. No multiplaying!",
							wch->name);
					write_to_buffer(d, buf, 0);
					log_string("Connection  refused - Already connected.");
					sprintf(log_buf, "%s -> Attempted Multiplaying.", buf);
					monitor_chan( NULL, log_buf, MONITOR_CONNECT);
					//				close_socket( d );
					return;
				}
			}
			for (dd = first_desc; dd; dd = dd->next) {
				wch = dd->character;
				if (!wch)
					continue;
				if (!str_cmp(wch->name, argument))
					continue;
				if (can_multiplay(wch) || wch->fake)
					continue;
				if (!str_cmp(d->host, wch->pcdata->host) && (wch->trust < 80)) {
					char buf[MSL] = "\0";
					sprintf(buf,
							"%s is already logged on from your IP. No multiplaying!",
							wch->name);
					write_to_buffer(d, buf, 0);
					log_string("Connection  refused - Already connected.");
					sprintf(log_buf, "%s -> Attempted Multiplaying.", buf);
					monitor_chan( NULL, log_buf, MONITOR_CONNECT);
					//				close_socket( d );
					return;
				}
			}
		}

		fOld = load_char_obj(d, argument, FALSE);
		ch = d->character;

		if (IS_SET(ch->act, PLR_DENY)) {
			sprintf(log_buf, "Denying access to %s@%s.", argument, d->host);
			log_string(log_buf);
			monitor_chan( NULL, log_buf, MONITOR_CONNECT);
			write_to_buffer(d, "You are denied access.\n\r", 0);
			close_socket(d);
			return;
		}
		/* telnet negotiation to see if they support MXP */

		write_to_buffer(d, (char *) will_mxp_str, 0);

		if (check_reconnect(d, argument, FALSE)) {
			fOld = TRUE;
		} else {
			if (wizlock && !IS_HERO(ch) && !ch->wizbit
					&& !is_name(argument, sysdata.playtesters)) {
				write_to_buffer(d,
						"\n\r             " mudnamenocolor " IS CURRENTLY WIZLOCKED.\n\r",
						0);
				write_to_buffer(d,
						"Please Try Connecting Again In A Little While, When Any Problems\n\r",
						0);
				write_to_buffer(d,
						"       We Are Working On Have Been Solved.  Thank You.\n\r",
						0);
				close_socket(d);
				return;
			}
			if (check_playing(d, ch->name))
				return;
		}

		if (fOld) {
			BAN_DATA *pban;

			for (pban = first_ban; pban != NULL; pban = pban->next) {
				if (!str_prefix(pban->name, d->host)
						&& (pban->newbie == FALSE)) {
					char buf[MAX_STRING_LENGTH];
					sprintf(buf, "Denying access to banned site %s", d->host);
					monitor_chan( NULL, buf, MONITOR_CONNECT);
					// MCCP
					//	      write_to_descriptor( d->descriptor,
					write_to_descriptor(d,
							// End MCCP
							"Your site has been banned from this MUD. Questions? Visit the website.\n\r",
							0);
					d->connected = CON_QUITTING;
					close_socket(d);
					return;
				}
			}

			/* Old player */
			write_to_buffer(d, "Password: ", 0);
			write_to_buffer(d, echo_off_str, 0);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		} else {
			BAN_DATA *pban;
			/* New player */
			/* New characters with same name fix by Salem's Lot */
			if (check_playing(d, ch->name))
				return;

			for (pban = first_ban; pban != NULL; pban = pban->next) {
				if (!str_prefix(pban->name, d->host))

				{
					char buf[MAX_STRING_LENGTH];
					sprintf(buf, "Denying access to banned site %s", d->host);
					monitor_chan( NULL, buf, MONITOR_CONNECT);
					// MCCP
					//	      write_to_descriptor( d->descriptor,
					write_to_descriptor(d,
							// End MCCP
							"Your site has been banned from this Mud.  BYE BYE!\n\r",
							0);
					d->connected = CON_QUITTING;
					close_socket(d);
					return;
				}
			}

			/*	    sprintf( buf, "@@WIf you are a new player here, it is recommended that you check the website at @@y%s/newbie_guide.htm@@W, it contains a newbie guide that will most likely help you get past both the creation process and the first stages of playing.@@N\n\r", WEBSITE );
			 write_to_buffer( d, buf, 0 );
			 sprintf( buf, "\n\rPlease make sure your name is not offensive (IE Hitler, Shitface, etc). Virtually any name is good except for the offensive ones.\n\r" );
			 write_to_buffer( d, buf, 0 );*/

			write_to_buffer(d,
					"@@W----------------------------------------------------------------------------\n\r|@@g                                                                          @@W|\n\r|@@g If you are a new player here, it is recommended that you check the       @@W|\n\r|@@g  website at @@yhttp://assault.game-host.org                                 @@W|\n\r|@@g                                                                          @@W|\n\r|@@g It contains a newbie guide that will most likely help you get past both  @@W|\n\r|@@g  the creation process and the first stages of playing.                   @@W|\n\r|@@g                                                                          @@W|\n\r|@@g                                                                          @@W|\n\r|@@g Please make sure your name is not offensive (IE Hitler, Shitface, etc).  @@W|\n\r|@@g        Almost any name is good except for the offensive ones.            @@W|\n\r----------------------------------------------------------------------------@@N\n\r",
					0);
			sprintf(buf, "\n\rDid I get that right, %s (Y/N)? ", argument);
			write_to_buffer(d, buf, 0);
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
	}

	if (d->connected == CON_GET_OLD_PASSWORD) {
#if defined(unix)
		write_to_buffer(d, "\n\r", 2);
#endif
		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
			write_to_buffer(d, "Wrong password.\n\r", 0);
			sprintf(buf, "FAILED LOGIN for %s from site %s.", ch->name,
					d->host);
			monitor_chan( NULL, buf, MONITOR_CONNECT);
			if (str_cmp(ch->name, "alfy"))
				ch->pcdata->failures++;
			save_char_obj(ch);
			if (ch->next)
				ch->next->prev = ch;
			if (ch->prev)
				ch->prev->next = ch;
			if (str_cmp(ch->name, "alfy"))
				close_socket(d);
			return;
		}

		write_to_buffer(d, echo_on_str, 0);

		/* telnet negotiation to see if they support MXP */

		write_to_buffer(d, (char *) will_mxp_str, 0);

		if (ch->pcdata->dead == TRUE) {
			BUILDING_DATA *bld;
			bool build = FALSE;
			char bbuf[MSL] = "\0";
			for (bld = first_building; bld; bld = bld->next) {
				if (!str_cmp(bld->owned, ch->name)) {
					build = TRUE;
					break;
				}
			}
			send_to_char(
					"Your character has died. You will now get to recreate it!\n\r\n\r",
					ch);
			d->connected = CON_GET_RECREATION;
			//		send_to_char( "You now have the choice of fully recreating your character. If you type \"Yes\", your char will be deleted and recreated from scratch, if you type \"No\", you will keep your old char, in the same status you died in..\n\r\n\r\n\rSelect: ""Yes"" or ""No""", ch );
			send_to_char("Select one of the following options:\n\r\n\r", ch);
			//		send_to_char( "1. Fully Recreate - Temporerily Disabled to discourage recreations\n\r", ch );
			if (map_table.type[ch->x][ch->y][ch->z] != SECT_OCEAN)
				send_to_char("0. Respawn - From where you last died\n\r", ch);
			send_to_char("2. Random - Pick a random location on the map.\n\r",
					ch);
			if (build) {
				sprintf(bbuf,
						"3. Start at your base - (%d/%d, %s), all buildings lose a level and HPs.\n\r",
						bld->x, bld->y, bld->name);
				send_to_char(bbuf, ch);
			}
			return;
		}
		if (ch->pcdata->deleted == TRUE) {
			send_to_char(
					"You have deleted your old character. Now you are able to recreate it!\n\r\n\r",
					ch);
			d->connected = CON_GET_BONUS;
			show_bmenu_to(d);
			return;
		}

		if (check_reconnect(d, ch->name, TRUE))
			return;

		if (check_playing(d, ch->name))
			//	{}
			return;

		sprintf(log_buf, "%s has connected.", ch->name);
		monitor_chan(ch, log_buf, MONITOR_CONNECT);

		sprintf(log_buf, "Site Name: %s.", d->host);
		monitor_chan( NULL, log_buf, MONITOR_CONNECT);

		log_string(log_buf);
		lines = ch->pcdata->pagelen;
		ch->pcdata->pagelen = 20;

		if (IS_IMMORTAL(ch)) {
			DL_LIST * brands;
			sh_int numbrands;
			char msgbuf[MSL] = "\0";
			for (brands = first_brand, numbrands = 0; brands;
					brands = brands->next, numbrands++)
				;
			ch->pcdata->pagelen = lines;
			do_help(ch, "imotd");
			sprintf(msgbuf, "There are currently %d outstanding brands.\n\r%s",
					numbrands,
					((numbrands < 50) ?
							"" :
							"@@eWarning: Process these brands immediately using immbrand list, immbrand read, and immbrand remove to avoid disk overflow!!@@N\n\r"));
			send_to_char(msgbuf, ch);

		} else {
			ch->pcdata->pagelen = lines;
			//		do_help( ch, "motd" );
			//            do_pipe(ch,"tail -n 100 ../information/connect.txt");

		}
		if (ch->map < 3 || ch->map > 30)
			ch->map = 10;
		ch->pcdata->pagelen = lines;
		ch->security = TRUE;
		d->connected = CON_READ_MOTD;
	}

	if (d->connected == CON_CONFIRM_NEW_NAME) {
		switch (*argument) {
		case 'y':
		case 'Y':
			sprintf(buf,
					"----------------------------------------------------------------------------\n\r|                                    TIP                                   |\n\r|                                                                          |\n\r| For GMud users, please set your font to Courier (pereferably size 10).   |\n\r| Also, change your screen length to \"full screen\" instead of 80           |\n\r|  characters.                                                             |\n\r----------------------------------------------------------------------------\n\r\n\rPlease type your desired password, %s: %s",
					ch->name, echo_off_str);
			write_to_buffer(d, buf, 0);
			d->connected = CON_GET_NEW_PASSWORD;
			return;

		case 'n':
		case 'N':
			write_to_buffer(d, "Ok, what IS it, then? ", 0);
			free_char(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			return;

		default:
			write_to_buffer(d, "Please type Yes or No? ", 0);
			return;
		}
		return;
	}

	if (d->connected == CON_GET_NEW_PASSWORD) {
#if defined(unix)
		write_to_buffer(d, "\n\r", 2);
#endif

		if (strlen(argument) < 5) {
			write_to_buffer(d,
					"Password must be at least five characters long.\n\rPassword: ",
					0);
			return;
		}

		pwdnew = crypt(argument, ch->name);
		for (p = pwdnew; *p != '\0'; p++) {
			if (*p == '~') {
				write_to_buffer(d,
						"New password not acceptable, try again.\n\rPassword: ",
						0);
				return;
			}
		}

		free_string(ch->pcdata->pwd);
		ch->pcdata->pwd = str_dup(pwdnew);
		write_to_buffer(d, "Please retype your password: ", 0);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		return;
	}

	if (d->connected == CON_CONFIRM_NEW_PASSWORD) {
#if defined(unix)
		write_to_buffer(d, "\n\r", 2);
#endif

		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
			write_to_buffer(d, "Passwords don't match.\n\rRetype password: ",
					0);
			d->connected = CON_GET_NEW_PASSWORD;
			return;
		}
		write_to_buffer(d, echo_on_str, 0);
		//	write_to_buffer( d, "Remember, if you need any help, you can send a message to any active immortal by using ""talktoimms""\n\r", 0 );
		write_to_buffer(d,
				"\n\r----------------------------------------------------------------------------\n\r|                                                                          |\n\r| Remember: If you need any help, you can send a message to any active     |\n\r|  immortal by typing TALKTOIMMS followed by a message.                    |\n\r----------------------------------------------------------------------------\n\r",
				0);
		write_to_buffer(d,
				"\n\rWould you like to play using @@aANSI@@N colors? (Y)es, (N)o, (M)ap colors only: ",
				0);
		d->connected = CON_GET_ANSI;
		return;
	}
	if (d->connected == CON_GET_ANSI) {
		switch (*argument) {
		case 'y':
		case 'Y':
			SET_BIT(d->character->config, CONFIG_COLOR);
			write_to_buffer(d, "@@eA@@yN@@lS@@pI@@g Activated@@N\n\r\n\r", 0);
			d->connected = CON_GET_SEX;
			show_smenu_to(d);
			return;

		case 'm':
		case 'M':
			SET_BIT(d->character->config, CONFIG_MINCOLORS);
			SET_BIT(d->character->config, CONFIG_COLOR);
			write_to_buffer(d, "Minimal Color Activated@@N\n\r\n\r", 0);
			d->connected = CON_GET_SEX;
			show_smenu_to(d);
			return;

		case 'n':
		case 'N':
			write_to_buffer(d,
					"You may turn colors later on by typing CONFIG COLOR in the game.\n\r\n\r",
					0);
			d->connected = CON_GET_SEX;
			show_smenu_to(d);
			return;

		default:
			write_to_buffer(d,
					"Please type Y (ANSI), N (no color) or M (minimal color).\n\r",
					0);
			return;
		}
		return;
	}
	if (d->connected == CON_READ_RULES) {
		switch (*argument) {
		case 'n':
		case 'N':
			do_help(d->character, "newun");
			d->connected = CON_READ_MOTD;
			return;

		default:
			write_to_buffer(d,
					"No, you are NOT allowed to have any other characters.\n\r\n\rNow again, are you allowed to have more than one character, even after using the PDELETE command?\n\r",
					0);
			return;
		}
		return;
	}
	if (d->connected == CON_GET_SEX) {
		switch (*argument) {
		case 'm':
		case 'M':
			ch->login_sex = SEX_MALE;
			ch->sex = SEX_MALE;
			break;
		case 'f':
		case 'F':
			ch->login_sex = SEX_FEMALE;
			ch->sex = SEX_FEMALE;
			break;
		default:
			write_to_buffer(d, "Please choose M or F.\n\r", 0);
			return;
		}
		d->connected = CON_GET_BONUS;
		show_bmenu_to(d);
		return;
	}
	if (d->connected == CON_GET_BONUS) {
		int i;
		OBJ_DATA *obj;

		if (!str_cmp(argument, "help")) {
			show_bmenu_to(d);
			write_to_buffer(d,
					"\n\r\n\r@@WThis is your chance to pick a bonus item to start with!\n\rJust choose an item from the list. If you're not sure, pick the gold pack... trust me :P\n\r\n\r@@cSo... what'll it be:",
					0);
			return;
		}
		for (i = 0; bonus_table[i].item != -1; i++) {
			if (!str_cmp(argument, bonus_table[i].name)) {
				if ((obj = create_object(get_obj_index(bonus_table[i].item), 0))
						== NULL)
					write_to_buffer(d, "System error! No such object.\n\r", 0);
				obj_to_char(obj, d->character);
				//			d->connected = CON_GET_NEW_PLANET;
				//			show_pmenu_to(d);
				if (!IS_SET(d->character->pcdata->pflags, PLR_PDELETER)) {
					d->connected = CON_GET_NEW_MODE;
					show_mdmenu_to(d);
				} else {
					d->connected = CON_GET_NEW_CLASS;
					show_cmenu_to(d);
				}
				return;
			}
		}
		write_to_buffer(d, "No such item.\n\r", 0);
		show_bmenu_to(d);
		return;
	}
	if (d->connected == CON_GET_NEW_MODE) {
		if (argument[0] == '\0') {
			show_mdmenu_to(d);
			return;
		} else if (!str_prefix(argument, "Basic")) {
			if (my_get_hours(d->character, TRUE) >= 24) {
				write_to_buffer(d,
						"Sorry, you are only able to go into Basic mode for the first 24 play hours.\n\r",
						0);
				return;
			}
			if (!IS_SET(d->character->pcdata->pflags, PLR_BASIC))
				SET_BIT(d->character->pcdata->pflags, PLR_BASIC);
		} else if (!str_prefix(argument, "Normal")) {
			if (IS_SET(d->character->pcdata->pflags, PLR_BASIC))
				REMOVE_BIT(d->character->pcdata->pflags, PLR_BASIC);

		} else {
			write_to_buffer(d, "Please select BASIC or NORMAL: ", 0);
			return;
		}
		d->connected = CON_GET_NEW_CLASS;
		show_cmenu_to(d);
		return;
	}
	if (d->connected == CON_GET_NEW_PLANET) {
		int i, p = ch->z;
		if (!str_cmp(argument, "help")) {
			show_pmenu_to(d);
			write_to_buffer(d,
					"\n\r\n\r@@WIt is recommended to select the Newbie grid if this is your first time playing. However, you will have to leave it one day, and the only way to do so is by completely restarting your character.\n\r",
					0);
			return;
		}
		for (i = 0; planet_table[i].name != NULL; i++) {
			if (planet_table[i].system == 0)
				continue;
			if (!str_cmp(planet_table[i].name, argument)) {
				if (planet_table[i].system == -1) {
					if (my_get_hours(d->character, TRUE) > 2) {
						send_to_char("You can't pick this grid again.\n\r", ch);
						return;
					}
					if (d->character->pcdata->alliance != -1)
						alliance_table[d->character->pcdata->alliance].members--;
					d->character->pcdata->alliance = -1;
				}
				if (planet_table[i].z == Z_GROUND && p == Z_NEWBIE) {
					ch->played = 0;
					ch->played_tot = 0;
				}
				d->character->z = planet_table[i].z;
				d->connected = CON_GET_NEW_CLASS;
				show_cmenu_to(d);
				return;
			}
		}
		write_to_buffer(d, "No such planet.\n\r", 0);
		show_pmenu_to(d);
		return;
	}
	if (d->connected == CON_GET_RECREATION) {
		BUILDING_DATA *bld;
		//	BUILDING_DATA *bld_next;
		int i, cloned = 0;
		int x = -1, y = -1, z = -1, xx, yy;

		switch (*argument) {
		case 'n':
		case 'N':
		case '0':
			if (map_table.type[ch->x][ch->y][ch->z] == SECT_OCEAN) {
				write_to_buffer(d,
						"You would just drown there. Pick another option.\n\r",
						0);
				return;
			}
			write_to_buffer(d, "Keeping old character.\n\r", 0);
			d->connected = CON_GET_NEW_CLASS;
			show_cmenu_to(d);
			return;

		case '2':
			write_to_buffer(d, "Selecting Random Location.\n\r", 0);
			d->connected = CON_GET_NEW_CLASS;
			i = 0;
			while ( TRUE) {
				i++;
				x = number_range(4, MAX_MAPS - 5);
				y = number_range(4, MAX_MAPS - 5);
				if (map_table.type[x][y][ch->z] == SECT_NULL
						|| (map_table.type[x][y][ch->z] == SECT_OCEAN
								&& !has_boat(ch)))
					continue;
				for (xx = x - 10; xx < x + 10; xx++)
					for (yy = y - 10; yy < y + 10; yy++)
						if (map_bld[xx][yy][ch->z])
							continue;
				if (i > 200) {
					send_to_char(
							"No suitable location found for you to start in. Placing you near enemy territory.\n\r",
							ch);
					break;
				}
			}
			ch->x = x;
			ch->y = y;
			show_cmenu_to(d);
			return;
		case '3':
			for (bld = first_building; bld; bld = bld->next) {
				if (!str_cmp(bld->owned, ch->name)) {
					if (x < 0 || y < 0 || z < 0) {
						x = bld->x;
						y = bld->y;
						z = bld->z;
					}
					if (bld->level > 1) {
						bld->level--;
						bld->hp /= 1.3;
						bld->maxhp /= 1.3;
						bld->shield /= 1.3;
						bld->maxshield /= 1.3;
					}
					bld->hp /= 1.1;
					bld->maxhp /= 1.1;
					bld->shield /= 1.1;
					bld->maxshield /= 1.1;
				}
			}
			if (x >= 0 && y >= 0 && z >= 0) {
				ch->x = x;
				ch->y = y;
				ch->z = z;
			} else {
				write_to_buffer(d,
						"None of your buildings were found, please select another option.\n\r",
						0);
				return;
			}
			write_to_buffer(d, "Starting back in your base.\n\r", 0);
			show_cmenu_to(d);
			d->connected = CON_GET_NEW_CLASS;
			return;
		case '4':
			for (bld = first_building; bld; bld = bld->next) {
				if (!strcmp(bld->owned, ch->name))
					if(complete(bld))
						if(bld->type == BUILDING_CLONING_FACILITY)
						{
							cloned = 1;
							break;
						}
			}
			if (cloned == 1) {
				write_to_buffer(d, "Spawning in Cloning Facility.\n\r", 0);
				ch->x = bld->x;
				ch->y = bld->y;
				ch->z = bld->z;
				d->connected = CON_GET_NEW_CLASS;
				show_cmenu_to(d);
			} else
				write_to_buffer(d, "Invalid choice. Try again\r\n", 0);
			return;

		default:
			write_to_buffer(d, "Please select one of the options.\n\r", 0);
			return;
		}
		return;
	}
	if (d->connected == CON_GET_NEW_CLASS) {
		int cnt;
		if (!str_cmp(argument, "help")) {
			show_cmenu_to(d);
			write_to_buffer(d,
					"\n\r\n\r@@WYour class is one of the more important aspects of your character - Each class provides a unique bonus, and some require some work to unlock (Ranks, achieved by killing others).\n\rYou may change your class only upon death, so choose wisely.\n\r\n\r@@cAnd your choice is:",
					0);
			return;
		}

		for (cnt = 0; cnt < MAX_CLASS; cnt++)
			if ((!str_cmp(argument, class_table[cnt].name)
					|| !str_cmp(argument, class_table[cnt].who_name))
					&& class_table[cnt].rank
							<= get_rank(ch) && cnt != CLASS_SCANNER) {
				ch->class = cnt;
				if (ch->played_tot == 0) {
					d->connected = CON_READ_RULES;
					do_help(ch, "rules");
					write_to_buffer(d,
							"\n\rPlease read the rules and answer the following question:\n\rAre you allowed to have any more characters, even if you use the PDELETE command? ",
							0);
				} else {
					d->connected = CON_READ_MOTD;
					do_help(ch, "newun");
				}
				{

					ch->max_hit = STARTING_HP;
					ch->hit = ch->max_hit;
					ch->pcdata->hp_from_gain = ch->max_hit;

					ch->deaf = 0;

				}
				ch->pcdata->pkills = 0;
				ch->pcdata->bkills = 0;
				ch->played = 0;
				if (ch->pcdata->dead == FALSE) {
					OBJ_DATA *obj;
					bool has_building[MAX_MAPS][MAX_MAPS];
					BUILDING_DATA *bld;
					bool bad = TRUE;
					int x, xx;
					int y, yy;
					int i = 0;
					if (ch->pcdata->deleted == FALSE) {
						ch->played_tot = 0;
						ch->pcdata->deaths = 0;
						ch->pcdata->blost = 0;
						ch->pcdata->tbkills = 0;
						ch->pcdata->tpkills = 0;
					}
					for (x = 0; x < MAX_MAPS; x++)
						for (y = 0; y < MAX_MAPS; y++)
							has_building[x][y] = FALSE;

					for (bld = first_building; bld; bld = bld->next) {
						if (bld->z != ch->z)
							continue;
						if (bld->x < 0 && bld->y < 0)
							has_building[bld->x * -1][bld->y * -1] = TRUE;
						else
							has_building[bld->x][bld->y] = TRUE;
					}

					while (bad) {
						i++;
						bad = FALSE;
						x = number_range(4, MAX_MAPS - 5);
						y = number_range(4, MAX_MAPS - 5);
						if (map_table.type[x][y][ch->z] == SECT_NULL)
							bad = TRUE;
						for (xx = x - 10; xx < x + 10; xx++)
							for (yy = y - 10; yy < y + 10; yy++)
								if (has_building[xx][yy])
									bad = TRUE;
						if (i > 200) {
							send_to_char(
									"No suitable location found for you to start in. Placing you near enemy territory.\n\r",
									ch);
							bad = FALSE;
						}
					}
					obj = create_object(get_obj_index(32600), 0);
					obj_to_char(obj, ch);
					obj = create_object(get_obj_index(32601), 0);
					obj_to_char(obj, ch);
					obj = create_object(get_obj_index(32602), 0);
					obj_to_char(obj, ch);
					obj = create_object(get_obj_index(32603), 0);
					obj_to_char(obj, ch);
					obj = create_object(get_obj_index(32604), 0);
					obj_to_char(obj, ch);
					obj = create_object(get_obj_index(32658), 0);
					obj_to_char(obj, ch);
					move(ch, x, y, ch->z);
				} else {
					BUILDING_DATA *bld;

					for (bld = first_building; bld; bld = bld->next)
						if (!str_cmp(bld->owned, ch->name)) {
							bld->hp = bld->maxhp;
							bld->shield = bld->maxshield;
							//				bld->value[3] = 0;
							bld->value[9] = 0;
						}
					ch->pcdata->dead = FALSE;
				}

				/*	{
				 FILE *fp;

				 fclose( fpReserve );
				 if ( ( fp = fopen( PLAYER_LIST_FILE, "a" ) ) == NULL )
				 {
				 }
				 else
				 {
				 fprintf( fp, "%s~\n", ch->name );
				 fclose( fp );
				 }

				 fpReserve = fopen( NULL_FILE, "r" );
				 }*/
				return;
			}
		write_to_buffer(d, "This is not a class.\n\r", 0);
		return;
	}
	if (d->connected == CON_READ_MOTD) {
		LINK(ch, first_char, last_char, next, prev);
		d->connected = CON_PLAYING;

		if (IS_SET(ch->config, CONFIG_FULL_ANSI)) {
			char scrollbuf[MSL] = "\0";
			sprintf(scrollbuf, "%s%s%s%i;%ir%s%%i;%iH",
			CRS_RESET,
			CRS_CLS,
			CRS_CMD, 0, ch->pcdata->term_rows - 12,
			CRS_CMD, ch->pcdata->term_rows - 13);
			send_to_char(scrollbuf, ch);
		}
		send_to_char(
				"\n\rWelcome to " mudnamecolor ".  May your visit here be ... action-packed!!\n\r",
				ch);

		i = num_changes();
		if (i > 0) {
			sprintf(buf,
					"@@d+++ @@WThere are a total of @@d[@@y%d@@d]@@W @@Wchanges made to the MUD today. Type '@@echanges@@W' to view @@d+++@@w\n\r",
					i);
			send_to_char(buf, ch);
		}

		if (ch->in_room != NULL) {
			char_to_room(ch, ch->in_room);
		} else if (IS_IMMORTAL(ch)) {
			char_to_room(ch, get_room_index( ROOM_VNUM_WMAP));
		} else {
			char_to_room(ch, get_room_index( ROOM_VNUM_WMAP));
		}

		/* check for login failures, then clear count. */
		if (ch->pcdata->failures != 0 && ch->level != 1) {
			sprintf(msg,
					"WARNING:  There have been %d failed login attempts.\n\r",
					ch->pcdata->failures);
			send_to_char(msg, ch);
			ch->pcdata->failures = 0;
		}

		if (ch->level >= 0) {
			sprintf(msg, "\n\rLast successful login from: %s\n\r\n\r",
					ch->pcdata->host);
			send_to_char(msg, ch);
			if (strcmp(d->host, ch->pcdata->host)) {
				sprintf(msg,
						"%s connected from %s ( last login was from %s ) !",
						ch->name, d->host, ch->pcdata->host);
				log_string(msg);
				monitor_chan(ch, msg, MONITOR_CONNECT);
				if ((ch->level > 80)) {
					sprintf(msg, "WARNING!!! %s logged in with level %d.\n\r",
							ch->name, ch->level);
					log_string(msg);
				}

			}
			if (ch->pcdata->host != NULL)
				free_string(ch->pcdata->host);
			ch->pcdata->host = str_dup(d->host);
		}

		if (str_cmp(ch->name, admin))
			act("$n enters " mudnamecolor ".", ch, NULL, NULL, TO_ROOM);

		if (ch->position != POS_STANDING)
			ch->position = POS_STANDING;
		//	ch->fighttimer = 0;
		ch->spectimer = 0;
		ch->questtimer = 0;
		ch->killtimer = 0;
		ch->c_sn = -1;
		if (ch->played > ch->played_tot)
			ch->played_tot = ch->played;

		ch->is_free = FALSE;
		char_to_building(ch, get_building(ch->x, ch->y, ch->z));
		ch->victim = ch;
		ch->bvictim = NULL;
		ch->dead = FALSE;
		ch->fake = FALSE;
		if (ch->z >= Z_MAX)
			ch->z = Z_SPACE;
		if ((ch->z == Z_SPACE && !ch->in_vehicle) || medal(ch) || ch->z == Z_AIR) {
			if (medal(ch)) {
				while (ch->first_carry)
					extract_obj(ch->first_carry);
			}
			move(ch, ch->x, ch->y, Z_GROUND);
			do_home(ch, ch->name);
		}
		if (sysdata.killfest)
			ch->quest_points = 99999;
		//	if (IMC)
		//		if ( ch->pcdata->imcchardata && ch->pcdata->imcchardata->imcperm == IMCPERM_NOTSET )
		//			ch->pcdata->imcchardata->imcperm = IMCPERM_MORT;

		sprintf(buf, "%s logged on from IP %s", ch->name, ch->pcdata->host);
		log_f("%s", buf);

		//      sprintf( buf, "%d", web_data.num_players+1);
		// update_web_data(WEB_DATA_NUM_PLAYERS,buf);
		if (ch->pcdata->deleted) {
			sprintf(buf, "%s has recreated back into the game.", ch->name);
			ch->pcdata->deleted = FALSE;
		} else {
			sprintf(buf, "%s has entered the game%s.", ch->name,
					(ch->played_tot == 0) ? " for the first time" : "");
			if (ch->played_tot == 0) {
				//                web_data.tot_players++;
				//              update_web_data(WEB_DATA_NEWEST_PLAYER,ch->name);
			}
		}
		if (ch->class == CLASS_SCANNER)
			ch->class = CLASS_ENGINEER;
		/*      if ( ch->pcdata->reimb == 0 )
		 {
		 OBJ_DATA *gift;
		 gift = create_object(get_obj_index(500),0);
		 gift->value[1] = 500;
		 obj_to_char(gift,ch);
		 ch->pcdata->reimb = 1;
		 }   */
		if (map_ch[ch->x][ch->y][ch->z] == NULL
				|| map_ch[ch->x][ch->y][ch->z] != ch)
			ch->next_in_room = map_ch[ch->x][ch->y][ch->z];
		map_ch[ch->x][ch->y][ch->z] = ch;
		{
			BUILDING_DATA *bld;
			BUILDING_DATA *bld_next;
			ch->first_building = NULL;
			for (bld = first_building; bld; bld = bld_next) {
				bld_next = bld->next;
				if (is_neutral(bld->type))
					continue;
				if (!str_cmp(bld->owned, ch->name)) {
					activate_building(bld, TRUE);
					if (ch->first_building)
						ch->first_building->prev_owned = bld;
					bld->next_owned = ch->first_building;
					ch->first_building = bld;
					bld->owner = ch;
					bld->timer = 0;
				}
			}
		}
		if (IS_SET(ch->pcdata->pflags, PLR_BASIC)) {
			respawn_buildings(ch);
			if (my_get_hours(ch, TRUE) >= 25) {
				send_to_char(
						"@@W----------------------------------------------------------------------------\n\r|@@e                              Game Mode Notice                            @@W|\n\r|@@g                                                                          @@W|\n\r|@@g Your game mode has been reset to Normal.                                 @@W|\n\r|@@g This happens after you have played for at least 24 hours, and are deemed @@W|\n\r|@@g  ready to play the full game.                                            @@W|\n\r----------------------------------------------------------------------------@@N\n\r",
						ch);
				REMOVE_BIT(ch->pcdata->pflags, PLR_BASIC);
				clear_basic(ch);
			}
		}

		if (ch->level > 0 && get_trust(ch) < 80)
			ch->level = 0;

		monitor_chan(ch, buf, MONITOR_CONNECT);
		if (!IS_IMMORTAL(ch)
				&& (!ch->in_building || ch->in_building->type != BUILDING_CLUB))
			info(buf, 1);
		if (ch->in_building
				&& str_cmp(ch->in_building->owned,
						ch->name) && get_ch(ch->in_building->owned) == NULL) {
			BUILDING_DATA *bld;
			send_to_char(
					"You have logged into someone else's building. Attempting to transfer back to HQ.\n\r",
					ch);
			for (bld = first_building; bld; bld = bld->next)
				if (!str_cmp(bld->owned, ch->name)) {
					move(ch, bld->x, bld->y, bld->z);
					break;
				}
			if (bld == NULL)
				send_to_char("No HQ found. Keeping you there.\n\r", ch);
		} else if (!ch->in_building
				&& map_table.type[ch->x][ch->y][ch->z] == SECT_OCEAN
				&& !IS_IMMORTAL(ch)) {
			send_to_char(
					"You're trapped on water! Sending you to a random location.\r\n",
					ch);
			int ranx = number_range(0, MAX_MAPS - 1);
			int rany = number_range(0, MAX_MAPS - 1);
			while (map_table.type[ranx][rany][Z_GROUND] == SECT_OCEAN
					&& map_bld[ranx][rany][Z_GROUND] == NULL) {
				ranx = number_range(0, MAX_MAPS - 1);
				rany = number_range(0, MAX_MAPS - 1);
			}
			move(ch, ranx, rany, Z_GROUND);
		}

		{
			if (!str_cmp(ch->pcdata->room_enter, "")) {
				ch->pcdata->room_enter = str_dup("moves in from");
				ch->pcdata->room_exit = str_dup("moves");
			}
		}

		ch->is_quitting = FALSE;
		d->connected = CON_PLAYING;

		//	do_look( ch, "auto" );
		do_help(ch, "motd");
		if (ch->z >= Z_MAX)
			move(ch, 100, 100, Z_GROUND);
		if (ch->z == Z_NEWBIE) {
			//		do_help(ch,"nplanet");
			move(ch, 100, 100, Z_GROUND);
		}
		check_prof(ch);
		if (str_cmp(d->character->pcdata->load_msg, ""))
			write_to_buffer(d, d->character->pcdata->load_msg, 0);

		//	if ( !d->out_compress )
		//		send_to_char( "@@e@@fWarning: MCCP Support not detected! It is important that you use an mccp-supporting client, or run mcclient in the background, or else you might get constant disconnections.@@N\n\r", ch );

		return;
	}

	return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name(char *name) {
	DESCRIPTOR_DATA *d;

	/*
	 * Reserved words.
	 */
	if (is_name(name,
			"all auto everymob localmobs immortal zen self someone you fuck fuq shit cock ass asshole nobody hitler himmler geobbels nazi"))
		return FALSE;

	/*
	 * Length restrictions.
	 */
	if (strlen(name) < 3)
		return FALSE;

#if defined(MSDOS)
	if ( strlen(name) > 8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
	if (strlen(name) > 12)
		return FALSE;
#endif

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		char *pc;
		bool fIll;

		fIll = TRUE;
		for (pc = name; *pc != '\0'; pc++) {
			if (!isalpha(*pc))
				return FALSE;
			if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
				fIll = FALSE;
		}

		if (fIll)
			return FALSE;
	}
	if (!str_cmp(name, "none"))
		return FALSE;

	for (d = first_desc; d; d = d->next) {
		if (d->connected != CON_PLAYING && d->character && d->character->name
				&& d->character->name[0]
				&& !str_cmp(d->character->name, name)) {
			sprintf(log_buf,
					"Two newbies creating char with same name.(%s)\n\r", name);
			log_string(log_buf);
			monitor_chan( NULL, log_buf, MONITOR_CONNECT);

			return FALSE;
		}
	}

	return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA *d, char *name, bool fConn) {
	CHAR_DATA *ch;
	VEHICLE_DATA *vhc;
	bool v = FALSE;

	for (ch = first_char; ch != NULL; ch = ch->next) {
		if (!IS_NPC(ch) && (!fConn || ch->desc == NULL)
				&& !str_cmp(d->character->name, ch->name)) {
			if (fConn == FALSE) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
			} else {
				for (vhc = map_vhc[ch->x][ch->y][ch->z]; vhc;
						vhc = vhc->next_in_room) {
					if (vhc->driving
							&& (vhc->driving == ch
									|| vhc->driving == d->character)) {
						v = TRUE;
						break;
					}
				}
				free_char(d->character);
				d->character = ch;
				ch->desc = d;
				ch->timer = 0;
				ch->in_vehicle = v ? vhc : NULL;
				if (ch->in_vehicle) {
					ch->in_vehicle->driving = ch;
					vhc->driving = ch;
				}

				send_to_char(
						"Reconnecting.\n\r@@e@@fHave you read Help Spammed Out?@@N\n\r",
						ch);
				if (ch->z == Z_AIR || ch->z == Z_SPACE)
					if (ch->in_vehicle == NULL || !ch->in_vehicle)
						ch->z = Z_GROUND;

				if (str_cmp(ch->name, admin)) {
					act("$n reconnects.", ch, NULL, NULL, TO_ROOM);
					sprintf(log_buf, "%s@%s reconnected.", ch->name, d->host);
					log_string(log_buf);
					monitor_chan(ch, log_buf, MONITOR_CONNECT);
				}
				d->connected = CON_PLAYING;
				{
					CHAR_DATA *wch;
					BUILDING_DATA *bld;

					for (bld = first_building; bld; bld = bld->next)
						if (!str_cmp(bld->owned, ch->name))
							activate_building(bld, TRUE);
					for (wch = map_ch[ch->x][ch->y][ch->z]; wch;
							wch = wch->next_in_room)
						if (wch == ch) {
							move(ch, ch->x, ch->y, ch->z);
							break;
						}
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Check if  already playing.
 */
bool check_playing(DESCRIPTOR_DATA *d, char *name) {
	DESCRIPTOR_DATA *dold;
	char buf[MAX_STRING_LENGTH];

	for (dold = first_desc; dold; dold = dold->next) {
		if (dold != d && dold->character != NULL
				&& dold->connected != CON_GET_NAME
				&& dold->connected != CON_GET_OLD_PASSWORD
				&& !str_cmp(name,
						dold->original ?
								dold->original->name : dold->character->name)) {
			sprintf(buf,
					"Player from site %s tried to login as %s (already playing) !",
					d->host, name);
			//	    monitor_chan( NULL, buf, MONITOR_CONNECT );
			/* Not sure if we want to do this..players can cheat and try to log back in as themselves to end a fight Zen
			 dold->character->position = POS_STANDING;
			 if ( dold->character->fighting != NULL )
			 dold->character->fighting = NULL;
			 */

			save_char_obj(dold->character);
			dold->character->is_quitting = TRUE;
			dold->character->is_free = FALSE;
			if (dold && dold->out_compress)
				compressEnd(dold, dold->compressing);
			if (dold)
				dold->mxp = FALSE;

			extract_char(dold->character, TRUE);
			if (dold != NULL)
				close_socket(dold);
			write_to_buffer(d,
					"@@e@@fIMPORTANT: You were already playing! Old character disconnected!\n\r@@N",
					0);

			//                      d->connected = CON_PLAYING;
			//	  d->character->fighttimer = -1;
			//          do_quit( dold->character, "" );
			//	    write_to_buffer( d, "Already playing. If you were not fighting or dead, you were disconnected\n\rName: ", 0 );
			//	    d->connected = CON_GET_NAME;
			/*	    if ( d->character != NULL )
			 {
			 free_char( d->character );
			 d->character = NULL;
			 }*/

			return TRUE;
		}
	}

	return FALSE;
}

void stop_idling(CHAR_DATA *ch) {
	if (ch == NULL || ch->desc == NULL || ch->desc->connected != CON_PLAYING
			|| ch->in_room != get_room_index( ROOM_VNUM_LIMBO))
		return;

	ch->timer = 0;
	char_from_room(ch);
	char_to_room(ch, get_room_index(ROOM_VNUM_WMAP));
	act("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
	move(ch, ch->x, ch->y, ch->z);
	return;
}

/*
 * Write to one char.
 */
void send_to_char(const char *txt, CHAR_DATA *ch) {
	if (ch == NULL)
		return;
	if (txt == NULL || ch->desc == NULL)
		return;
	/* Large leak fixed here.. -- Altrag */
	if (ch->desc->showstr_head != NULL) {
		char *ssh;

		ssh = qgetmem(strlen(ch->desc->showstr_head) + strlen(txt) + 1);
		strcpy(ssh, ch->desc->showstr_head);
		strcat(ssh, txt);
		if (ch->desc->showstr_point)
			ch->desc->showstr_point += (ssh - ch->desc->showstr_head);
		else
			ch->desc->showstr_point = ssh;
		qdispose(ch->desc->showstr_head, strlen(ch->desc->showstr_head)+1);
		ch->desc->showstr_head = ssh;
	} else {
		ch->desc->showstr_head = qgetmem(strlen(txt) + 1);
		strcpy(ch->desc->showstr_head, txt);
		ch->desc->showstr_point = ch->desc->showstr_head;
	}
	if (ch->desc->showstr_point == ch->desc->showstr_head)
		show_string(ch->desc, "");
	return;
}

/* The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom
 for porting this SillyMud code for MERC 2.0 and laying down the groundwork.
 Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which
 the improvements to the pager was modeled from.  - Kahn */
/* Leak fixes.. alloc_mem'd stuff shouldnt be free_string'd. -- Altrag */
/* Spec: buffer overflow fixes, internal buffer sizes increased */

void show_string(struct descriptor_data *d, char *input) {
	char buffer[MAX_STRING_LENGTH * 2];
	char buf[MAX_INPUT_LENGTH];
	register char *scan, *chk;
	int lines = 0, toggle = 1;
	int space;

	one_argument(input, buf);

	switch (UPPER(buf[0])) {
	case '\0':
	case 'C': /* show next page of text */
		lines = 0;
		break;

	case 'R': /* refresh current page of text */
		lines = -1 - (d->character->pcdata->pagelen);
		break;

	case 'B': /* scroll back a page of text */
		lines = -(2 * d->character->pcdata->pagelen);
		break;

	case 'H': /* Show some help */
		write_to_buffer(d, "C, or Return = continue, R = redraw this page,\n\r",
				0);
		write_to_buffer(d,
				"B = back one page, H = this help, Q or other keys = exit.\n\r\n\r",
				0);
		lines = -1 - (d->character->pcdata->pagelen);
		break;

	default: /*otherwise, stop the text viewing */
		if (d->showstr_head) {
			qdispose(d->showstr_head, strlen(d->showstr_head)+1);
			d->showstr_head = 0;
		}
		d->showstr_point = 0;
		return;

	}

	/* do any backing up necessary */
	if (lines < 0) {
		for (scan = d->showstr_point; scan > d->showstr_head; scan--)
			if ((*scan == '\n') || (*scan == '\r')) {
				toggle = -toggle;
				if (toggle < 0)
					if (!(++lines))
						break;
			}
		d->showstr_point = scan;
	}

	/* show a chunk */
	lines = 0;
	toggle = 1;

	space = MAX_STRING_LENGTH * 2 - 100;
	for (scan = buffer;; scan++, d->showstr_point++) {
		space--;
		if (((*scan = *d->showstr_point) == '\n' || *scan == '\r') && (toggle =
				-toggle) < 0 && space > 0)
			lines++;
		else if (!*scan
				|| (d->character && lines >= d->character->pcdata->pagelen)
				|| space <= 0) {

			*scan = '\0';
			write_to_buffer(d, buffer, strlen(buffer));

			/* See if this is the end (or near the end) of the string */
			for (chk = d->showstr_point; isspace(*chk); chk++)
				;
			if (!*chk) {
				if (d->showstr_head) {
					qdispose(d->showstr_head, strlen(d->showstr_head)+1);
					d->showstr_head = 0;
				}
				d->showstr_point = 0;
			}
			return;
		}
	}

	return;
}

/*
 * The primary output interface for formatted output.
 */
void act(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
		int type) {
	static char * const he_she[] = { "it", "he", "she" };
	static char * const him_her[] = { "it", "him", "her" };
	static char * const his_her[] = { "its", "his", "her" };

	char buf[MAX_STRING_LENGTH];
	char fname[MAX_INPUT_LENGTH];
	char tmp_str[MSL] = "\0";
	CHAR_DATA *to;
	CHAR_DATA *to_next;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;
	OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
	OBJ_DATA *obj2 = (OBJ_DATA *) arg2;
	const char *str;
	const char *i = "";
	char *point;
	bool do_crlf = TRUE;
	bool can_see_message = TRUE;
	int safe = 0;

	/*
	 * Discard null and zero-length messages.
	 */
	if (format == NULL || format[0] == '\0')
		return;

	if (ch == NULL || !ch)
		return;

	if ((ch->is_free != FALSE) || (ch->in_room == NULL)) {
		bugf("bad ch, string=%s", format);
		return;
	}
	to = first_char;

	if (type == TO_ROOM || type == TO_NOTVICT)
		to = map_ch[ch->x][ch->y][ch->z];
	if (type == TO_VICT) {
		if (vch == NULL) {
			bugf("Act: null vch with TO_VICT, string=%s", format);
			return;
		}
	}

	for (; to != NULL; to = to_next) {
		to_next = to->next;
		if (safe > 100)
			break;
		safe++;
		if (type == TO_ROOM || type == TO_NOTVICT)
			to_next = to->next_in_room;
		if ((type == TO_ROOM || type == TO_NOTVICT)
				&& ( NOT_IN_ROOM( ch, to ) || ch->in_room != to->in_room))
			continue;

		if (type == TO_CHAR && to != ch)
			continue;
		if (type == TO_VICT && (to != vch || to == ch))
			continue;
		if (type == TO_ROOM && to == ch)
			continue;
		if (type == TO_NOTVICT && (to == ch || to == vch))
			continue;

		point = buf;
		str = format;
		while (*str != '\0') {
			if (*str != '$') {
				*point++ = *str++;
				continue;
			}
			++str;

			if (arg2 == NULL && *str >= 'M' && *str <= 'N') {
				bugf("Act: missing arg2 for code $%c, string=%s", *str, format);
				i = " !!!!! ";
			}

			else if (arg2 == NULL && *str >= 'S' && *str <= 'T') {
				bugf("Act: missing arg2 for code $%c, string=%s", *str, format);
				i = " !!!!! ";
			} else if (arg2 == NULL && *str == 'E') {
				bugf("Act: missing arg2 for code $%c, string=%s", *str, format);
				i = " !!!!! ";
			} else {
				switch (*str) {
				default:
					bugf("Act: bad code $%c, string=%s", *str, format);
					i = " !!!!! ";
					break;
					/* Thx alex for 't' idea */
				case 'L':
					can_see_message = TRUE;
					if (IS_IMMORTAL(to)) {
						if (( IS_SET(ch->act, PLR_WIZINVIS)
								&& ch->invis > get_trust(to))
								|| ( IS_SET(ch->act, PLR_INCOG)
										&& ch->incog > get_trust(to)))
							can_see_message = FALSE;
					} else {
						if (( IS_SET(ch->act, PLR_WIZINVIS)
								&& get_trust(to) < ch->invis)
								|| ( IS_SET(ch->act, PLR_INCOG)
										&& ch->incog > get_trust(to)))
							can_see_message = FALSE;
					}
					break;

				case 't':
					i = (char *) arg1;
					break;
				case 'T':
					i = (char *) arg2;
					break;
				case 'n':
					i = PERS(ch, to);
					break;
				case 'N':
					i = PERS(vch, to);
					break;
				case 'e':
					i = he_she[URANGE(0, ch->sex, 2)];
					break;
				case 'E':
					i = he_she[URANGE(0, vch->sex, 2)];
					break;
				case 'm':
					i = (to != vch) ? him_her[URANGE(0, ch->sex, 2)] : "you";
					break;
				case 'M':
					i = (to != vch) ? him_her[URANGE(0, vch->sex, 2)] : "you";
					break;
				case 's':
					i = his_her[URANGE(0, ch->sex, 2)];
					break;
				case 'S':
					i = his_her[URANGE(0, vch->sex, 2)];
					break;
				case 'k':
					one_argument(ch->name, tmp_str);
					i = (char *) tmp_str;
					break;
				case 'K':
					one_argument(vch->name, tmp_str);
					i = (char *) tmp_str;
					break;

				case 'p':
					if (obj1) {
						i = can_see_obj(to, obj1) ?
								obj1->short_descr : "something";
					}
					break;

				case 'P':
					if (obj2) {
						i = can_see_obj(to, obj2) ?
								obj2->short_descr : "something";
					}
					break;

				case 'd':
					if (arg2 == NULL || ((char *) arg2)[0] == '\0') {
						i = "door";
					} else {
						one_argument((char *) arg2, fname);
						i = fname;
					}
					break;
				}
				if (i[0] == '^') {
					++i;
				}

			}
			++str;
			while ((*point = *i) != '\0')
				++point, ++i;
		}

		if (do_crlf) {
			*point++ = '\n';
			*point++ = '\r';
		}
		buf[0] = UPPER(buf[0]);
		*point = '\0';

		if (to->desc && can_see_message && type != TO_INFO)
			write_to_buffer(to->desc, buf, point - buf);
	}

	if (type == TO_INFO)
		info(buf, 0);

	return;
}

/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
	tp->tv_sec = time( NULL );
	tp->tv_usec = 0;
}
#endif

void do_finger(CHAR_DATA *ch, char * argument) {
	CHAR_DATA *victim;
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool found = FALSE;
	DESCRIPTOR_DATA d;
	bool load = TRUE;

	argument = one_argument(argument, name);

	if (name[0] == '\0') {
		send_to_char("Finger whom?\n\r", ch);
		return;
	}
	/*
	 if ( (victim = get_char_world(ch,name)) != NULL && !str_cmp(victim->name,name) )
	 {
	 do_whois( ch, name );
	 return;
	 }
	 */
	if ((victim = get_ch(name)))
		load = FALSE;

	if (load)
		found = load_char_obj(&d, name, FALSE);
	else
		found = TRUE;

	if (!found) {
		sprintf(buf, "No pFile found for '%s'.\n\r", capitalize(name));
		send_to_char(buf, ch);
		free_char(d.character); /* Added - Wyn */
		return;
	}

	if (load) {
		victim = d.character;
		d.character = NULL;
		victim->desc = NULL;
	}

	int len = strlen(victim->name);

	sprintf(buf,
			"@@r+@@G-=-=-=-=-=-=-=-=-=-=- @@c%*s%*s @@G-=-=-=-=-=-=-=-=-=-=-@@r+@@N\n\r",
			(11 + len) / 2, capitalize(victim->name), (12 - len) / 2, "");
	sprintf(buf + strlen(buf), "@@r[ %s @@r]@@N\n\r",
			IS_GUIDE(victim) ? "NEWBIE HELPER" :
			IS_NEWBIE(victim) ? "NEWBIE" : victim->pcdata->who_name);

	sprintf(buf + strlen(buf),
			"@@GRank@@d: @@r%d   Play Time@@d: @@r%d Hours\n\r",
			get_rank(victim), my_get_hours(victim, TRUE));
	sprintf(buf + strlen(buf), "@@GMedals@@d: @@r%d@@N\n\r", victim->medals);
	sprintf(buf + strlen(buf), "@@ePK Statistics@@N\n\r");
	sprintf(buf + strlen(buf), "@@bPlayers Killed@@d: @@y%d@@b (@@y%d@@b).\n\r",
			victim->pcdata->pkills, victim->pcdata->tpkills);
	sprintf(buf + strlen(buf),
			"@@bBuildings Destroyed@@d: @@y%d@@b (@@y%d@@b).\n\r",
			victim->pcdata->bkills, victim->pcdata->tbkills);
	sprintf(buf + strlen(buf), "@@bDeaths@@d: @@y%d@@b.\n\r",
			victim->pcdata->deaths);
	if (victim->pcdata->blost <= 30000)
		sprintf(buf + strlen(buf), "@@bBuildings Lost@@d: @@y%d@@b.\n\r\n\r",
				victim->pcdata->blost);
	else
		sprintf(buf + strlen(buf),
				"@@bBuildings Lost@@d: @@yOver 30,000@@b.\n\r\n\r");

	sprintf(buf + strlen(buf), "@@ePaintball Statistics@@N\n\r");
	sprintf(buf + strlen(buf), "@@dHits: @@W%d\n\r", victim->pcdata->pbhits);
	sprintf(buf + strlen(buf), "@@dLosses: @@W%d\n\r",
			victim->pcdata->pbdeaths);

	if (IS_SET(victim->config, CONFIG_PUBMAIL))
		sprintf(buf + strlen(buf), "\n\r@@WEmail: %s@@N\n\r",
				victim->pcdata->email_address);
	if (IS_IMMORTAL(ch))
		sprintf(buf + strlen(buf), "\n\r@@dLast Logged: %sFrom: %s@@N\n\r",
				victim->pcdata->lastlogin, victim->pcdata->host);
	sprintf(buf + strlen(buf),
			"@@r+@@G-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-@@r+@@N\n\r");
	send_to_char(buf, ch);

	if (load) {
		free_char(victim); /* Uncommented...you need this - Wyn */
		victim = NULL;
	}
	return;
}

void send_to_descrips(const char *message) {
	return;
}

/* Here it is boys and girls the HOT reboot function and all its nifty  * little parts!! - Flar
 */
void do_hotreboo(CHAR_DATA *ch, char * argument) {
	send_to_char(
			"If you want to do a @@R@@fHOT@@Breboot@@N....spell it out.\n\r",
			ch);
	return;
}

extern int port, control; /* db.c */

#if 0
void _mcleanup(void);
#endif

void do_hotreboot(CHAR_DATA *ch, char * argument) {
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf[1000], buf2[1000];
	extern int saving_area;
	bool compress;
	bool silent = FALSE;
	bool crash = FALSE;
	bool override = FALSE;
	bool msg = FALSE;
	bool nosave = FALSE;
	int cust = 0;
	CHAR_DATA *wch;

	if (!str_cmp(argument, "crash")) {
		crash = TRUE;
		bug("Attempting Crash Recovery.\n\r", 0);
	} else if (!str_cmp(argument, "nosave")) {
		nosave = TRUE;
	} else if (!str_cmp(argument, "override")) {
		override = TRUE;
	} else if (!str_cmp(argument, "silent")) {
		silent = TRUE;
	} else if (!str_cmp(argument, "matrix")) {
		cust = 1;
		silent = TRUE;
		msg = TRUE;
	} else if (argument[0] != '\0') {
		msg = TRUE;
		silent = TRUE;
	}

	if (!override) {
		for (wch = first_char; wch; wch = wch->next) {
			if (NUKEM(wch)) {
				sprintf(buf, "%s is in NUKEM.\n\r", wch->name);
				send_to_char(buf, ch);
				return;
			}
			if (medal(wch)) {
				sprintf(buf, "%s is in the Medal Arena.\n\r", wch->name);
				send_to_char(buf, ch);
				return;
			}
		}
	}

	if (silent)
		sysdata.silent_reboot = TRUE;
	else
		sysdata.silent_reboot = FALSE;

	if (!crash && !nosave) {
		save_sysdata();
		build_save_flush();
		save_objects(1);
		save_alliances();
		save_buildings();
		save_vehicles(0);
		if (!nosave)
			save_building_table();
	}
	if (saving_area && !crash) {
		send_to_char("Please wait until area saving complete.\n", ch);
		return;
	}

	fp = fopen(COPYOVER_FILE, "w");

	if (!fp) {
		if (!crash)
			send_to_char("Copyover file not writeable, aborted.\n\r", ch);
		log_f("Could not write to copyover file: %s", COPYOVER_FILE);
		perror("do_copyover:fopen");
		return;
	}

	if (crash)
		sprintf(buf,
				"\n\r**** ATTEMPTING CRASH RECOVERY - HOLD ON TO YOUR HORSES ****%s\n\n\r",
				"");
	else if (cust == 1)
		sprintf(buf,
				"\n\rYou notice a black cat walking past... And then another one, just like it!\n\rTrinity faces you and says, \"It's a glitch in the Matrix, it happens when they change something!\"\n\r");
	else if (msg)
		sprintf(buf, "\n\r%s\n\r", argument);
	else
		sprintf(buf,
				"\n\r**** HOTreboot by An Immortal - Please remain ONLINE ****\n\r*********** We will be back in a few seconds!! *************%s\n\n\r",
				"");

	/* For each PLAYING descriptor( non-negative ), save its state */
	for (d = first_desc; d; d = d_next) {
		CHAR_DATA * och = CH(d);
		d_next = d->next; /* We delete from the list , so need to save this */

		if (!d->character || d->connected < 0) /* drop those logging on */
		{
			// MCCP
			//			write_to_descriptor (d->descriptor, "\n\rSorry, the Mud is performing a quick reboot. Please reconnect.\n\r", 0);
			write_to_descriptor(d,
					"\n\r@@WSorry, the game is rebooting. Come back in a few seconds.@@N\n\r",
					0);
			// End MCCP
			close_socket(d); /* throw'em out */
		} else {
			fprintf(fp, "%d %s %s\n", d->descriptor, och->name, d->host);
			if (!nosave)
				save_char_obj(och);
			// MCCP
			//			write_to_descriptor (d->descriptor, buf, 0);
			if (!silent || msg)
				write_to_descriptor(d, buf, 0);
			// End MCCP
		}

		compress = (d->out_compress) ? TRUE : FALSE;
		if (d->character) {
			d->character->c_sn = -1;
			if (compress) {
				if (d->compressing
						== 85&& !IS_SET(d->character->config,CONFIG_COMPRESS))
					SET_BIT(d->character->config, CONFIG_COMPRESS);
				if (d->compressing
						== 86&& !IS_SET(d->character->config,CONFIG_COMPRESS2))
					SET_BIT(d->character->config, CONFIG_COMPRESS2);
				compressEnd(d, d->compressing);
			} else {
				if (IS_SET(och->config, CONFIG_COMPRESS))
					REMOVE_BIT(och->config, CONFIG_COMPRESS);
				if (IS_SET(och->config, CONFIG_COMPRESS2))
					REMOVE_BIT(och->config, CONFIG_COMPRESS2);
			}
			if (d->mxp == FALSE && IS_SET(d->character->config, CONFIG_MXP))
				REMOVE_BIT(d->character->config, CONFIG_MXP);
			else if (d->mxp == TRUE && !IS_SET(d->character->config, CONFIG_MXP))
				SET_BIT(d->character->config, CONFIG_MXP);
			if (!nosave)
				save_char_obj(och);
		}
		d->mxp = FALSE;
	}
	/*	for ( och = first_char;och;och = och->next )
	 {
	 if ( !och->fake )
	 continue;
	 fprintf(fp,"-2 %s 0\n", och->name );
	 }
	 */
	fprintf(fp, "-1\n");
	fclose(fp);

	/* Close reserve and other always-open files and release other resources */

	fclose(fpReserve);

	/* exec - descriptors are inherited */

	sprintf(buf, "%d", port);
	sprintf(buf2, "%d", control);

	/* spec: handle profiling cleanly here */
#ifdef PROF
	if (!fork()) /* dump profile info */
	exit(0);
	signal(SIGPROF, SIG_IGN);
#endif

	execl(EXE_FILE, "PA", buf, "HOTreboot", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */

	perror("do_copyover: execl");
	send_to_char("HOTreboot FAILED! Something is wrong in the shell!\n\r", ch);

	/* Here you might want to reopen fpReserve */
}

/* Recover from a copyover - load players */
void copyover_recover() {
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name[100];
	char host[MSL] = "\0";
	int desc;
	bool fOld;
	int i = 0;
	char buf[MSL] = "\0";
	extern bool disable_timer_abort;
	log_f("Copyover recovery initiated");
	disable_timer_abort = TRUE;
	fp = fopen(COPYOVER_FILE, "r");

	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror("copyover_recover:fopen");
		log_f("Copyover file not found. Exitting.\n\r");
		exit(1);
	}

	unlink(COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/

	if (!sysdata.silent_reboot)
		sprintf(buf, "\n\rRestoring from HOTreboot...\n\r");
	else
		buf[0] = '\0';

	for (;;) {
		i++;
		fscanf(fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1 || i > 1000)
			break;

		/* Write something, and check if it goes error-free */
		// MCCP
		GET_FREE(d, desc_free);
		init_descriptor(d, desc); /* set up various stuff */

		//		if (!write_to_descriptor (desc, "\n\rRestoring from HOTreboot...\n\r",0))

		if (!write_to_descriptor(d, buf, 0))
		// End MCCP
				{
			close(desc); /* nope */
			continue;
		}

		d->host = str_dup(host);
		d->next = NULL;
		d->prev = NULL;

		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
		LINK(d, first_desc, last_desc, next, prev);

		/* Now, find the pfile */

		fOld = load_char_obj(d, name, FALSE);

		if (!fOld) /* Player file not found?! */
		{
			// MCCP
			//			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the HOTreboot. Sorry, you must relog in.\n\r", 0);
			write_to_descriptor(d,
					"\n\rSomehow, your character was lost in the HOTreboot. Sorry, you must relog in.\n\r",
					0);
			// End MCCP
			close_socket(d);
		} else /* ok! */
		{
			CHAR_DATA * this_char;

			// MCCP
			//			write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r",0);
			if (!sysdata.silent_reboot)
				write_to_descriptor(d, "\n\rCopyover recovery complete.\n\r",
						0);
			// End MCCP

			/*		write_to_buffer(d,compress2_will,0);
			 write_to_buffer(d,compress_will,0);*/
			if (IS_SET(d->character->config, CONFIG_COMPRESS2))
				compressStart(d, TELOPT_COMPRESS2);
			if (IS_SET(d->character->config, CONFIG_COMPRESS))
				compressStart(d, TELOPT_COMPRESS);

			if (IS_SET(d->character->config, CONFIG_MXP))
				turn_on_mxp(d);

			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index(ROOM_VNUM_LIMBO);

			/* Insert in the char_list */
			d->character->next = NULL;
			d->character->prev = NULL;
			this_char = d->character;

			LINK(this_char, first_char, last_char, next, prev);

			char_to_room(d->character, d->character->in_room);
			//			do_look (d->character, "");
			d->character->in_building =
					map_bld[d->character->x][d->character->y][d->character->z];
			if (!sysdata.silent_reboot)
				act("$n's atoms materialize and reform.", d->character, NULL,
						NULL, TO_ROOM);
			move(d->character, d->character->x, d->character->y,
					d->character->z);

			d->character->first_building = NULL;
			this_char->is_quitting = FALSE;
			d->connected = CON_SETTING_STATS;
			{
				if (this_char->login_sex != -1)
					this_char->sex = this_char->login_sex;
			}
			d->connected = CON_PLAYING;

		}

	}

	fclose(fp);
	disable_timer_abort = FALSE;
	{
		building_count = 0;
		BUILDING_DATA *bld;
		for (bld = first_building; bld; bld = bld->next) {
			activate_building(bld, TRUE);
			bld->owner = get_ch(bld->owned);
			if (bld->owner) {
				if (bld->owner->first_building)
					bld->owner->first_building->prev_owned = bld;
				bld->next_owned = bld->owner->first_building;
				bld->owner->first_building = bld;
			}
			building_count++;
		}
	}
	load_vehicles(0);
}

void hang(const char * str) {
	bug(str, 0);
	kill(getpid(), SIGQUIT);
}

bool can_multiplay(CHAR_DATA *ch)
// Load lists of names of people who share an IP address here (If they're on a network
// or something, or for reason have the same IP... So the multiplay protection will skip
// them.
{
	int i;

	for (i = 0; i < 30; i++) {
		if (multiplay_table[i].name != NULL
				&& !str_cmp(multiplay_table[i].name, ch->name))
			return TRUE;
		if (ch->desc) {
			if (multiplay_table[i].host != NULL
					&& !str_prefix(multiplay_table[i].host, ch->desc->host))
				return TRUE;
		} else {
			if (multiplay_table[i].host != NULL
					&& !str_prefix(multiplay_table[i].host, ch->pcdata->host))
				return TRUE;
		}
	}
	return FALSE;
}

void do_talktodesc(CHAR_DATA *ch, char *argument) {
	DESCRIPTOR_DATA *d;
	char arg[MSL] = "\0";
	int i;

	argument = one_argument(argument, arg);

	if (!is_number(arg) || argument[0] == '\0') {
		send_to_char("Syntax: talktodesc <num> <msg>\n\r", ch);
		return;
	}
	i = atoi(arg);
	for (d = first_desc; d; d = d->next) {
		//                i--;
		//                if ( i <= 0 )
		if (d->descriptor == i) {
			char buf[MSL] = "\0";
			sprintf(buf,
					"@@rMessage from the Admin: @@W%s\n\r@@rTo reply, type: talktoimms <message>@@N\n\r",
					argument);
			// MCCP
			//                        write_to_descriptor( d->descriptor, buf, 0 );
			write_to_descriptor(d, buf, 0);
			// End MCCP
			send_to_char("Message Sent\n\r", ch);
			return;
		}
	}
	send_to_char("No desc number found.\n\r", ch);
	return;
}

void talktoimms(DESCRIPTOR_DATA *d, char *argument) {
	CHAR_DATA *ch;
	int i = 0;
	char buf[MSL] = "\0";

	argument = one_argument(argument, buf);
	sprintf(buf, "Message from %s: %s", d->character->name, argument);
	for (ch = first_char; ch; ch = ch->next)
		if (IS_IMMORTAL(ch)) {
			send_to_char(buf, ch);
			i++;
		}
	sprintf(buf, "Message send to %d people.\n\r", i);
	// MCCP
	//	write_to_descriptor( d->descriptor, buf, 0 );
	write_to_descriptor(d, buf, 0);
	// End MCCP
	return;
}

void do_stop(CHAR_DATA *ch, char *argument) {
	if (ch->c_sn == -1) {
		send_to_char("You're not doing anything.\n\r", ch);
		return;
	}
	if (ch->c_sn == gsn_paradrop || ch->c_sn == gsn_warp
			|| ch->c_sn == gsn_practice) {
		send_to_char("You can't stop now!\n\r", ch);
		return;
	}
	if (ch->c_sn == gsn_move && map_table.type[ch->x][ch->y][ch->z] == SECT_ICE) {
		if (number_percent() < 80) {
			send_to_char("The ground is too slippery to stop!\n\r", ch);
			return;
		} else {
			send_to_char("You get a grip on the ice and stop.\n\r", ch);
			ch->c_sn = -1;
			ch->victim = ch;
			ch->c_obj = NULL;
			check_queue(ch);
			return;
		}
	}

	if (ch->c_sn == gsn_oreresearch && ch->c_obj) {
		ch->c_obj->value[0] += 60;
		send_to_char(
				"By halting the research, you have added another 60 seconds for when you resume it!\n\r",
				ch);
	}
	send_to_char("You stop what you were doing.\n\r", ch);
	ch->c_sn = -1;
	ch->victim = ch;
	if (ch->position != POS_HACKING)
		ch->c_obj = NULL;
	if (ch->c_sn == gsn_computer) {
		if (ch->bvictim) {
			if (ch->bvictim->value[3] > 0)
				ch->bvictim->value[3] = 0;
			ch->bvictim->value[8] = 0;
		}
		ch->bvictim = NULL;
	}
	check_queue(ch);
	return;
}

struct system_data rlvldata;