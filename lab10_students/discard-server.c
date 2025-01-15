/* server.c - main */

/********************************************************************************/
/*										*/
/*  TCP discard server - repeatedly wait for a connection, read the data,	*/
/*			and close the connection when EOF occurs		*/
/*										*/
/* use:   server								*/
/*										*/
/********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>


/* TCP port to use */

#define		TCPPORT		50065	/* Just a random unused port		*/

/* Define buffer size */

#define		BUFF_SIZ	2048	/* size of a buffer			*/

#define		QLEN		32	/* Size of queue of new TCP requests	*/


int	dd;				/* descriptor for a data cconnection	*/

char	buffer[BUFF_SIZ];		/* I/O buffer				*/

/********************************************************************************/
/*										*/
/* main -- main program for the discard server					*/
/*										*/
/********************************************************************************/

int main (int argc, char *argv[]) {

    int		retval;			/* Return value for system calls	*/
    int		n;			/* Number of bytes read			*/

    struct	protoent *ppe;		/* pointer to protocol info. entry      */
    struct	sockaddr_in socin;	/* An IPv4 endpoint address             */
    struct	sockaddr_in socdata;	/* Endpoint address for accepted soc.	*/
    socklen_t	dataaddrlen;		/* Address length for above		*/

    // int		addrlen;		/* Length of a sockaddr_in structure    */
    int		msock;			/* Descriptor for master socket		*/

    /* Fork into background */

    retval = fork();
    if (retval < 0) {
	fprintf(stderr, "Error': cannot fork\n");
	exit(1);
    } else if (retval != 0) {	/* parent exits gracefully */
	fprintf(stderr, "server has process ID %d and is using port %d\n",
			retval, TCPPORT);
	exit(0);
    }

    /* Open master socket used to listen for connections */

    memset(&socin, 0, sizeof(socin));
    socin.sin_family = AF_INET;
    socin.sin_addr.s_addr = INADDR_ANY;
    socin.sin_port = htons( (unsigned short)TCPPORT );
    ppe = getprotobyname("tcp");

    /* Create the socket */

    msock = socket(PF_INET, SOCK_STREAM, ppe->p_proto);
    if (msock < 0) {
	fprintf(stderr, "cannot create master socket\n");
	exit(1);
    }

    /* Allow reuse for quick reboot */

    int yes = 1;
    setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    /* Bind to the local port */

    if (bind(msock, (struct sockaddr *)&socin, sizeof(socin)) < 0) {
	fprintf(stderr,
		"cannot bind to port %d -- did you leave a server running?\n",
		 htons(socin.sin_port));
	exit(1);
    }

    /* Set the listen queue */

    if (listen(msock, QLEN) < 0) {
	fprintf(stderr,
	  "cannot listen with queue %d on port %d\n",QLEN, ppe->p_proto);
    }

    /* Repeatedly accept a connection */

    while (1) {
	int	newsock;
	memset(&socdata, 0, sizeof(socdata));
	dataaddrlen = sizeof(socdata);
	newsock = accept(msock, (struct sockaddr *)&socdata, &dataaddrlen);
	if (newsock < 0) {
		fprintf(stderr, "accept failed (errno=%d)\n", errno);
		exit(1);
	}
	n = read(newsock, buffer, BUFF_SIZ);
	if (n < 0) {		/* Error occurred */
		fprintf(stderr,"Connection error\n");
		break;
	}
	while (n > 0) { /* Read and discard all data */
		n = read(newsock, buffer, BUFF_SIZ);
	}
	close(newsock);
    }
}
