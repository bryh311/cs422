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
  unsigned int ip_options; // may or may not be used
  // char * payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct arphdr {
  unsigned short hardware_type;
  unsigned short protocol_type;
  unsigned char hardware_length;
  unsigned char protocol_length;
  unsigned short operation;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct icmphdr {
  unsigned char type; // 1
  unsigned char code; // 1
  unsigned short checksum; // 2
  // unsigned char* content; // 8
};
#pragma pack(pop)

#pragma pack(push, 1)
struct echohdr {
  unsigned short identifier;
  unsigned short sequence_number;
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
  printf("usage: wireminnow-part6 [filename]\n");
  printf("options:\n");
  printf("-IP-use-names: print domain names instead of ip addresses\n");
  printf("-arpd: print all items in the arp packet\n");
}

void print_byte_array_hex(unsigned char *arr, size_t len) {
  for (size_t i = 0;  i < len; ++i) {
    printf("%02x", arr[i]);
  }
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
  /*
  if (argc < 2) {
    print_usage();
    return -1;
  }
  */

  char* filename = NULL;
  bool domain_name_mode = false;
  bool arp_detail = false;


  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-IP-use-names") == 0) {
      domain_name_mode = true;
    }
    else if (strcmp(argv[i], "-arpd") == 0) {
      arp_detail = true;
    }
    else if (filename == NULL) {
      filename = argv[i];
    }
  }

  if (filename == NULL) {
    print_usage();
    return -1;
  }
 
  /*
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
  */

  FILE* in;

  in = fopen(filename, "rb");
  if (in == NULL) {
    printf("unable to open file: %s\n", filename);
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

      // check if we need to worry about options
      unsigned char iplen = ip->vers_and_hlen & 0x0F;
      unsigned char * IP_PAYLOAD_OFFSET = (unsigned char*)(packet.data + sizeof(struct etherhdr) +
          sizeof(struct iphdr));
      if (iplen == 5) { // if no options
        IP_PAYLOAD_OFFSET -= sizeof(int);
      }

      // ICMP info
      if (ip->type == 1) {
        struct icmphdr *icmp = (struct icmphdr*)malloc(sizeof(struct icmphdr));
        memcpy(icmp, IP_PAYLOAD_OFFSET, sizeof(struct icmphdr));

        printf("ICMP: ");
        if (icmp->type == 0) {
          printf("Reply ");
        }
        else if (icmp->type == 8) {
          printf("Response ");
        }
        else {
          printf("Unknown\n");
        }

        if (icmp->type == 8 || icmp->type == 0) {
          struct echohdr* echo = (struct echohdr*)(IP_PAYLOAD_OFFSET + sizeof(struct icmphdr));
          printf("ident=%d seq=%d\n", ntohs(echo->identifier),
              ntohs(echo->sequence_number));
          unsigned char * payload = IP_PAYLOAD_OFFSET + sizeof(struct icmphdr) +
            sizeof(struct echohdr);
          printf("Start of payload: ");
          for (int j = 0; j < 32; j++) {
            printf("%02x ", payload[j]);
            if (j % 16 == 0 && j != 0) {
              printf("\n");
            }
          }
        }

        printf("\n");
        free(icmp);
        icmp = NULL;
      }


      free(ip);
      ip = NULL;
    }
    else if (ntohs(ethernet->type) == 0x0806) {
      struct arphdr* arp =  (struct arphdr*)(packet.data + sizeof(struct etherhdr));
      printf("ARP: ");
      if (ntohs(arp->operation) == 1) {
        printf("request ");
      }
      else if (ntohs(arp->operation) == 2) {
        printf("reply ");
      }
      else {
        printf("unknown op ");
      }

      int htype = ntohs(arp->hardware_type);
      int ptype = ntohs(arp->protocol_type);
      int hlen = arp->hardware_length;
      int plen = arp->protocol_length;

      if (arp_detail) {
        printf("htype=%d ", htype);
        printf("ptype=%04x ", ptype);
        printf("hlen=%d ", hlen);
        printf("plen=%d ", plen);
        printf("\n");
      }


      char * const BASE_ADDR = packet.data + sizeof(struct etherhdr) +
        sizeof(struct arphdr);

      unsigned char *sender_hardware_address = (unsigned char*)malloc(hlen);
      unsigned char *sender_protocol_address = (unsigned char*)malloc(plen);
      unsigned char *target_hardware_address = (unsigned char*)malloc(hlen);
      unsigned char *target_protocol_address = (unsigned char*)malloc(plen);

      memcpy(sender_hardware_address, BASE_ADDR, hlen);
      memcpy(sender_protocol_address, BASE_ADDR + hlen, plen);
      memcpy(target_hardware_address, BASE_ADDR + hlen + plen, hlen);
      memcpy(target_protocol_address, BASE_ADDR + 2 * hlen + plen, plen);

      if (ptype == 0x0800) {
        printf("snd= ");
        print_ip(sender_protocol_address);
        printf(" tar= ");
        print_ip(target_protocol_address);
        printf("\n");
      }
      else {
        // printf("protocol type unknown\n");
        printf("snd= unknown, hex: ");
        print_byte_array_hex(sender_protocol_address, plen);
        printf(" tar= unknown, hex: ");
        print_byte_array_hex(target_protocol_address, plen);
        printf("\n");
      }

      if (arp_detail) {
        if (htype == 1) {
          printf("sndmac= ");
          print_mac(sender_protocol_address);
          printf(" tarmac= ");
          print_mac(target_hardware_address);
          printf("\n");
        }
        else {
          printf("sndmac= unknown, hex: ");
          print_byte_array_hex(sender_hardware_address, hlen);
          printf("tarmac= unknown, hex: ");
          print_byte_array_hex(target_hardware_address, hlen);
          printf("\n");
        }
      }
      free(sender_hardware_address);
      free(sender_protocol_address);
      free(target_hardware_address);
      free(target_protocol_address);
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
