#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct pcappkt {
  unsigned int timestamp;
  unsigned int subtimestamp;
  unsigned int captured_len;
  unsigned int original_len;
  char* data;
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

struct etherhdr {
  unsigned char destination_address[6];
  unsigned char source_address[6];
  unsigned short type;
};

// assumes that input is len 6! this is lazy code!!!
void print_mac(unsigned char * mac_array) {
  printf("%02x:%02x:%02x:%02x:%02x:%02x", mac_array[0], mac_array[1],
      mac_array[2], mac_array[3], mac_array[4], mac_array[5]);
}

int main(int argc, char** argv) {
  // for now, I do not care about the header
  if (argc < 2) {
    printf("usage: wireminnow-part3 [filename]\n");
    return -1;
  }

  FILE* in;
  char* filename = argv[1];

  in = fopen(filename, "rb");
  if (in == NULL) {
    printf("unable to file: %s\n", filename);
    return -1;
  }

  // parse the packet header
  struct pcaphdr header = {0};
  int res = fread(&header, sizeof(struct pcaphdr), 1, in);
  if (res == 0) {
    printf("error: no packet header!\n");
    fclose(in);
    return -1;
  }

  // check if uses nano seconds
  bool is_nano = header.magic == 0xA1B2C3D4;

  if (is_nano) {
    printf("nanoseconds are used\n");
  }
  else {
    printf("microseconds are used\n");
  }

  struct pcappkt packet = {0};
  int i = 1;
  while (fread(&packet, sizeof(struct pcappkt) - sizeof(char*) , 1, in) == 1) {
    // read in packet data separately by allocation
    packet.data = (char*)malloc(packet.captured_len);
    fread(packet.data, packet.captured_len, 1, in);

    struct etherhdr *ethernet = (struct etherhdr *)malloc(sizeof(struct etherhdr));
    memcpy(ethernet, packet.data, sizeof(struct etherhdr));


    printf("Packet %d: timestamp=0x%08X subtimestamp=0x%08X actual length=%d captured length=%d\n",
           i, packet.timestamp, packet.subtimestamp, packet.original_len, packet.captured_len);
    printf("Ether: dst=");
    print_mac(ethernet->destination_address);
    printf(" src=");
    print_mac(ethernet->source_address);
    printf(" typ=%04x\n", ntohs(ethernet->type));


    i++;

    free(ethernet);
    ethernet = NULL;

    free(packet.data);
    packet.data = NULL;
  }

  // cleanup
  fclose(in);
  in = NULL;

  return 0;
}
