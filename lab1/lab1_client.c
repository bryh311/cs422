#include <cnaiapi.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MSGCNT    5

int main(int argc, char** argv)
{

  computer      comp;
  connection    conn;
  appnum        app;
  char*         computer_name;
  char*        messages[MSGCNT];

  messages[0] = strdup("Hello");
  messages[1] = strdup("My name is Bryce");
  messages[2] = strdup("How are you doing today?");
  messages[3] = strdup("I love computer inter-networking");
  messages[4] = strdup("JavaScript was a mistake");


  /*
  for (int i = 0; i < MSGCNT; ++i) {
    char len_buf[12];
    sprintf(len_buf,"%ld",strlen(messages[i]));
    printf("%s%%", len_buf);
    printf("%s\n", messages[i]);
  }
  */

  if (argc < 3) {
    printf("usage: lab1_part_a_client <ADDRESS> <PORT NUM>\n");
    return -1;
  }


  comp = cname_to_comp(argv[1]);
  app = (appnum)atoi(argv[2]);
  
  conn = make_contact(comp, app);
  if (conn < 0) {
    exit(1);
  }

  for (int i = 0; i < MSGCNT; ++i) {
    char len_buf[12]; // max integer length is 10, 11 to include negative, and 12 to include '\0'
    sprintf(len_buf,"%ld",strlen(messages[i]));
    send(conn, len_buf, strlen(len_buf), 0);
    send(conn,"%", 1, 0);
    send(conn,messages[i], strlen(messages[i]), 0);
  }

  send_eof(conn);

  for (int i = 0; i < MSGCNT; ++i) {
    free(messages[i]);
  }

  return 0;
}
