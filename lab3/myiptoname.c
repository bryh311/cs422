#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

/**
 * This program converts an ipaddress to a hostname using forward dns lookup
 * takes the ip address as an argument and prints the associated host name
 */
int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage: myiptoname <ipaddress>\n");
    exit(1);
  }

  char* ip_string = argv[1];

  struct in_addr ip_address;
  int ret = inet_aton(ip_string, &ip_address);

  if (ret == 0) {
    fprintf(stderr, "There was an issue parsing address\n");
    exit(1);
  }

  struct hostent* host = gethostbyaddr(&ip_address, sizeof(ip_address), AF_INET);

  if (host == NULL) {
    fprintf(stderr, "gethostbyaddr error: %s\n", hstrerror(h_errno));
    exit(1);
  }

  printf("Hostname: %s\n", host->h_name);
  exit(0);
}
