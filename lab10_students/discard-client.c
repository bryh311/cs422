/* reg.c - main */

/********************************************************************************/
/*										*/
/*  Discard client - contact a server, send data from stdin, and close the	*/
/*			close the connection.					*/
/*										*/
/* typical use:   cat data_file | tcp-client [computer]				*/
/*										*/
/********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* Machine on which the server runs */

#define		SERVER_LOC	"128.10.137.73"

/* TCP port to use */

#define		TCPPORT		50065/* Same as the server port		*/

/* Define buffer size */

#define		BUFF_SIZ	2048	/* size of a buffer			*/


/********************************************************************************/
/*										*/
/* main -- main program for the discard client					*/
/*										*/
/********************************************************************************/

int	main (int argc, char *argv[]) {

	int		n;			/* number of chars read		*/
	char		*host = SERVER_LOC;	/* host on which server runs	*/

	struct	hostent	*phe;			/* pointer to host info. entry	*/
	struct	protoent *ppe;			/* ptr. to protocol info. entry	*/
	struct	sockaddr_in socin;		/* an IPv4 endpoint address	*/
	// int	addrlen;			/* len of a sockaddr_in struct	*/
	
	int	sock;				/* descriptor for socket	*/

	char	buffer[BUFF_SIZ];		/* input buffer for prompt	*/

	/* Check for server name as argument */

	if (argc > 2) {
		fprintf(stderr, "optional argument is server computer\n");
		exit(1);
	}
	if (argc == 2) {
		host = argv[1];
	}
	/* Open the socket and connect to the server */

	memset(&socin, 0, sizeof(socin));
	socin.sin_family = AF_INET;

	/* Map host name to IP address */

    if ((phe = gethostbyname(host))) {
        memcpy(&socin.sin_addr, phe->h_addr, phe->h_length);
    } else if ( (socin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE ) {
		fprintf(stderr, "can't find an IP address for host name %s\n", host);
		exit(1);
	}

	socin.sin_port = htons( (unsigned short)TCPPORT );
	ppe = getprotobyname("tcp");

	/* Create the socket */

	sock = socket(PF_INET, SOCK_STREAM, ppe->p_proto);
	if (sock < 0) {
		fprintf(stderr, "cannot create socket\n");
		exit(1);
	}

	/* Connect the socket */

	if (connect(sock, (struct sockaddr *)&socin, sizeof(socin)) < 0) {
		fprintf(stderr, "can't connect to port %d\n", TCPPORT);
		exit(1);
	}

	fprintf(stderr,"Sending to host %s\n", host);
	n = read(0, buffer, BUFF_SIZ);
	while( n > 0 ) {
		write(sock, buffer, n);
		n = read(0, buffer, BUFF_SIZ);
	}
	close(sock);
	exit(0);
}
