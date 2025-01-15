#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

/**
 * Takes a hostname and lists all associated IP addresses
 */
int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage: mynametoip <hostname>\n");
    exit(1);
  }

  char* hostname = argv[1];
  struct hostent* host = gethostbyname(hostname);

  if (host == NULL) {
    fprintf(stderr, "error: %s\n", hstrerror(h_errno));
    exit(1);
  }

  struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;

  for (int i = 0; addr_list[i] != NULL; ++i) {
    printf("IP Address: %s\n", inet_ntoa(*addr_list[i]));
  }

  exit(0);
}
