#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>



#define PORT 50065
#define BUFFSIZE 256
#define MAX_MSG_LEN 2048

int main(void)
{
  int               len;
  char              buff[MAX_MSG_LEN];

  printf("Listening on port: %d\n", PORT);

  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  int conn = socket(AF_INET, SOCK_STREAM, 0);
  if (conn < 0) {
    perror("socket creation failed");
    exit(-1);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  bind(conn, (struct sockaddr*)&address, sizeof(address));
  listen(conn, 3);

  int my_sock = accept(conn, (struct sockaddr*)&address, &addrlen);

  char next_char;
  while((len = recv(my_sock, &next_char, 1, 0)) > 0) {
    int i = 0;
    while (next_char != '%') {
      if (next_char == '\0') {
         close(conn);
         return 0;
      }
      buff[i] = next_char;
      i++;
      recv(my_sock, &next_char, 1, 0);
    }
    buff[i] = '\0';
    int length = atoi(buff);
    int expected = length;

    for (int received = 0; received < expected;) {
      length = recv(my_sock, buff, (expected - received) < MAX_MSG_LEN ? (expected - received) : MAX_MSG_LEN, 0);
      if (length < 0) {
        send(conn, '\0',1,0);
        close(conn);
        return 1;
      }
      received += length;
    }
    buff[expected] = '\0';

    printf("%s\n", buff);
  }

  printf("\n");
  // send(conn,'\0',1,0);
  close(conn);

  return 0;
}
