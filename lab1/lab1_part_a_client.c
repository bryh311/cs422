#include <cnaiapi.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv)
{

  computer      comp;
  connection    conn;
  appnum        app;
  char*         computer_name;

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

  char* msg = "Hello, my name is BRYCE HILL\n";
  send(conn, msg, strlen(msg), 0);
  send_eof(conn);

  return 0;
}
