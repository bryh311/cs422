#include <cnaiapi.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>


#define PORT 50065
#define BUFFSIZE 256

int main(void)
{
  connection        conn;
  int               len;
  char              buff[BUFFSIZE];

  conn = await_contact((appnum)PORT);

  while((len = recv(conn, buff, BUFFSIZE, 0)) > 0) {
    printf("%s", buff);
  }

  printf("\n");
  send_eof(conn);

  return 0;
}
