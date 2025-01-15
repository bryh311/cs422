#include <cnaiapi.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>


#define PORT 50065
#define BUFFSIZE 256
#define MAX_MSG_LEN 2048

int main(void)
{
  connection        conn;
  int               len;
  char              buff[MAX_MSG_LEN];

  printf("Listening on port: %d\n", PORT);

  conn = await_contact((appnum)PORT);

  char next_char;
  while((len = recv(conn, &next_char, 1, 0)) > 0) {
    int i = 0;
    while (next_char != '%') {
      buff[i] = next_char;
      i++;
      recv(conn, &next_char, 1, 0);
    }
    buff[i] = '\0';
    int length = atoi(buff);
    int expected = length;

    for (int received = 0; received < expected;) {
      length = recv(conn, buff, (expected - received) < MAX_MSG_LEN ? (expected - received) : MAX_MSG_LEN, 0);
      if (length < 0) {
        send_eof(conn);
        return 1;
      }
      received += length;
    }
    buff[expected] = '\0';

    printf("%s\n", buff);
  }

  printf("\n");
  send_eof(conn);

  return 0;
}
