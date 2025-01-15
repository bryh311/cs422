#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MSGCNT    5

int main(int argc, char** argv)
{
  char*        messages[MSGCNT];
  messages[0] = strdup("Hello");
  messages[1] = strdup("My name is Bryce");
  messages[2] = strdup("How are you doing today?");
  messages[3] = strdup("I love computer inter-networking");
  messages[4] = strdup("JavaScript was a mistake");

  if (argc < 3) {
    printf("usage: lab1_part_a_client <ADDRESS> <PORT NUM>\n");
    return -1;
  }

  char *hostname = argv[1];
  int port = atoi(argv[2]);
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(hostname, argv[2], &hints, &res);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(-1);
  }

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("socket");
    exit(-1);
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("connect");
    close(sockfd);
    exit(-1);
  }

  for (int i = 0; i < MSGCNT; ++i) {
    char len_buf[12]; // max integer length is 10, 11 to include negative, and 12 to include '\0'
    sprintf(len_buf,"%ld",strlen(messages[i]));
    send(sockfd, len_buf, strlen(len_buf), 0);
    send(sockfd,"%", 1, 0);
    send(sockfd,messages[i], strlen(messages[i]), 0);
  }

  send(sockfd,"\0",1,0);

  for (int i = 0; i < MSGCNT; ++i) {
    free(messages[i]);
  }

  freeaddrinfo(res);

  return 0;
}
