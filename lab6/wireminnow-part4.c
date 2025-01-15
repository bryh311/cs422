#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>


#define N_IANA_TYPES 0xFF

char *iana_types[N_IANA_TYPES];

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

#pragma pack(push, 1)
struct iphdr {
  unsigned char vers_and_hlen;
  unsigned char service_type;
  unsigned short total_length;
  unsigned short identification;
  unsigned short flags_and_fragment_offset;
  unsigned char ttl;
  unsigned char type;
  unsigned short header_checksum;
  union {
    unsigned char source_address[4];
    unsigned int source_address_int;
  };
  union {
    unsigned char destination_address[4];
    unsigned int destination_address_int;
  };
  unsigned int ip_options;
  char * payload;
};
#pragma pack(pop)

// assumes that input is len 6! this is lazy code!!!
void print_mac(unsigned char * mac_array) {
  printf("%02x:%02x:%02x:%02x:%02x:%02x", mac_array[0], mac_array[1],
      mac_array[2], mac_array[3], mac_array[4], mac_array[5]);
}

void print_ip(unsigned char * ip_array) {
  printf("%hhu.%hhu.%hhu.%hhu", ip_array[0], ip_array[1],
      ip_array[2], ip_array[3]);
}

// don't use this
char * ip_array_to_string(unsigned char * ip_array) {
  char *ret = (char *)malloc(16);
  snprintf(ret, 16, "%hhu.%hhu.%hhu.%hhu", ip_array[0], ip_array[1],
      ip_array[2], ip_array[3]);
  return ret;
}

void print_usage(void) {
  printf("usage: wireminnow-part4 [filename]\n");
  printf("options:\n");
  printf("-IP-use-names: print domain names instead of ip addresses\n");
}

void init_iana_types(void) {
  for (int i = 0; i < N_IANA_TYPES; i++) {
    iana_types[i] = NULL;
  }

  iana_types[1] = strdup("ICMP");
  iana_types[2] = strdup("IGMP");
  iana_types[6] = strdup("TCP");
  iana_types[17] = strdup("UDP");
  iana_types[103] = strdup("PIM");
}

void free_iana_types(void) {
  for (int i = 0;i < N_IANA_TYPES; i++) {
    if (iana_types[i] != NULL) {
      free(iana_types[i]);
      iana_types[i] = NULL;
    }
  }
}


int main(int argc, char** argv) {
  // for now, I do not care about the header
  if (argc < 2) {
    print_usage();
    return -1;
  }

  char* filename;
  bool domain_name_mode = false;

  if (strcmp(argv[1], "-IP-use-names") == 0) {
    if (argc < 3) {
      print_usage();
      return -1;
    }
    filename = argv[2];
    domain_name_mode = true;
  }
  else {
    filename = argv[1];
  }

  FILE* in;

  in = fopen(filename, "rb");
  if (in == NULL) {
    printf("unable to file: %s\n", filename);
    return -1;
  }

  init_iana_types();

  // parse the packet header
  struct pcaphdr header = {0};
  int res = fread(&header, sizeof(struct pcaphdr), 1, in);
  if (res == 0) {
    printf("error: no packet header!\n");
    fclose(in);
    free_iana_types();
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

    if (ntohs(ethernet->type) == 0x0800) {
      struct iphdr *ip = (struct iphdr *)malloc(sizeof(struct iphdr));
      // offset for ethernet header
      memcpy(ip, packet.data + sizeof(struct etherhdr), sizeof(struct iphdr));
      if (domain_name_mode) {


        struct in_addr source_ip_address;
        source_ip_address.s_addr = ip->source_address_int;

        struct hostent* host = gethostbyaddr(&source_ip_address, sizeof(source_ip_address), AF_INET);
        printf("IP: src=");
        if (host == NULL) {
          print_ip(ip->source_address);
        }
        else {
          printf("%s", host->h_name);
        }

        struct in_addr destination_ip_address;
        destination_ip_address.s_addr = ip->destination_address_int;
        host = gethostbyaddr(&destination_ip_address, sizeof(destination_ip_address), AF_INET);

        printf(" dest=");
        if (host == NULL) {
          print_ip(ip->destination_address);
        }
        else {
          printf("%s", host->h_name);
        }

        printf(" typ=");
        if (iana_types[ip->type] == NULL) {
          printf("%hhu\n", ip->type);
        }
        else {
          printf("%s\n", iana_types[ip->type]);
        }
      }
      else {
        printf("IP: src=");
        print_ip(ip->source_address);
        printf(" dst=");
        print_ip(ip->destination_address);
        printf(" typ=%hhu\n", ip->type);
      }
      free(ip);
      ip = NULL;
    }


    printf("\n");
    i++;

    free(ethernet);
    ethernet = NULL;

    free(packet.data);
    packet.data = NULL;
  }

  // cleanup
  fclose(in);
  in = NULL;

  free_iana_types();

  return 0;
}
