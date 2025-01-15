#include <stdio.h>
#include <stdlib.h>

#define OK      0
#define SYSERR  1

/*
 * converts an ip string to one that can be used by a socket api
 * [in] dotted - string of ip address
 * [out] result - result of the conversion
 * returns: OK if conversion successfull, SYSERR if error
 */
int my_inet_aton(char *dotted, unsigned int *result) {
  *result = 0;
  char buf[4];
  int count = 0;
  int i = 0;
  while (count != 4) {
    int j = 0;
    while (dotted[i] != '.' && dotted[i] != '\0' && j < 4) {
      buf[j] = dotted[i];
      i++;
      j++;
    }

    if (j > 4) {
      return SYSERR;
    }

    buf[j] = '\0';
    count++;

    if (dotted[i] == '\0' && count != 4) {
      return SYSERR;
    }

    i++;

    // parse the number
    int octet = 0;
    for (int k = 0; k < j; k++) {
      octet *= 10;
      octet += buf[k] - '0';
    }

    if (octet > 0xFF) {
      return SYSERR;
    }

    *result <<= 8;
    *result += octet;

    //printf("buf: %s\n", buf);
    //printf("oct: %d\n", octet);
  }

  return OK;
}

int main(void) { 

  char* input = "192.168.0.1";
  unsigned int encoded;
  int res = my_inet_aton(input, &encoded);
  printf("res=%d\n", res);
  printf("encoded=%X\n", encoded);
}
