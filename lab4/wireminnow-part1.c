#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

enum second_ver {
  SECONDS_MICROSECONDS,
  SECONDS_NANOSECONDS
};

struct pcaphdr {
  unsigned int magic;
  unsigned short major_version;
  unsigned short minor_version;
  unsigned int timestamp;
  unsigned int timestamp_acc;
  unsigned int snaplen;
  unsigned int link_type;
};

enum second_ver g_precision;

void print_pcaphdr(struct pcaphdr hdr) {
  printf("magic number:  0x%X\n", hdr.magic);
  printf("major version: 0x%hX\n", hdr.major_version);
  printf("minor version: 0x%hX\n", hdr.minor_version);
  printf("timestamp:     0x%X\n", hdr.timestamp);
  printf("timestamp_acc: 0x%X\n", hdr.timestamp_acc);
  printf("snap length:   0x%X\n", hdr.snaplen);
  printf("link type:     0x%X\n", hdr.link_type);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage: wireminnow-part1 [filename]\n");
    return -1;
  }

  FILE* in;
  char* filename = argv[1];

  in = fopen(filename, "rb");

  if (in == NULL) {
    printf("error reading file: %s\n", filename);
    return -1;
  }

  struct pcaphdr my_header = {0};
  fread(&my_header, sizeof(struct pcaphdr), 1, in);

  if (my_header.snaplen == 0) {
    printf("illegal snaplen of 0\n");
    fclose(in);
    return -1;
  }

  print_pcaphdr(my_header);


  if (my_header.magic == 0xA1B2C3D4) {
    g_precision = SECONDS_MICROSECONDS;
    printf("file uses seconds and microseconds\n");
  }
  else if (my_header.magic == 0xA1B23C4D) {
    g_precision = SECONDS_NANOSECONDS;
    printf("file uses seconds and nanoseconds\n");
  }
  else {
    printf("error: I don't know what second format the file uses!\n");
  }

  // this does not work on big endian, I would have to modify it for those systems
  // even though literally NO ONE uses big-endian anymore SPARC is dead, PowerPC is dead
  // and 68k is dead. The only living big-endian system is the mainframe, and who the
  // hell is writing C code for that??? its all PL/I and ASM.
  unsigned int flag = my_header.link_type & 0x80000000;
  if (flag) {
    printf("fcs bytes are set!\n");
  }
  else {
    printf("fcs bytes are not set!\n");
  }



  // cleanup
  fclose(in);
  in = NULL;

  return 0;
}
