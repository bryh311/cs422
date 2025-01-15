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

#pragma pack(push, 1)
struct udphdr {
  unsigned short srcport;
  unsigned short destport;
  unsigned short msglen;
  unsigned short payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct dhcphdr {
  unsigned char op;
  unsigned char htype;
  unsigned char hlen;
  unsigned char hops;
  unsigned int xid;
  unsigned short secs;
  unsigned short flags;
  unsigned char ciaddr[4];
  unsigned char yiaddr[4];
  unsigned char siaddr[4];
  unsigned char giaddr[4];
  unsigned char chaddr[16];
  unsigned char sname[64];
  unsigned char file[128];
  unsigned int magic_cookie;
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
  printf("usage: wireminnow-part9 [filename]\n");
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

void print_printable(unsigned char c) {
  if (c < 32 || c >= 127) {
    printf("%02x", c);
  }
  else {
    printf("%c", c);
  }
}

void print_dhcp_message_type(unsigned char msg) {
  switch (msg) {
    case 1:
      printf("DISCOVER");
      break;
    case 2:
      printf("OFFER");
      break;
    case 3:
      printf("REQUEST");
      break;
    case 4:
      printf("DECLINE");
      break;
    case 5:
      printf("ACK");
       break;
     case 6:
       printf("NAC");
       break;
     case 7:
       printf("RELEASE");
       break;
     case 8:
       printf("INFORM");
       break;
     default:
       printf("UNKNOWN");
  }
}

void dhcp_option_parser(unsigned char* stream, int len) {
  unsigned char op = 255;
  unsigned char op_len;
  int read = 0;
  do {
    op = *stream++;
    read++;
    // printf("operation: %hhu\n", op);
    switch (op) {
      case 0:
        break;
      case 1: // subnet mask
        op_len = *stream++;
        // printf("len: %hhu\n", op_len);
        if (op_len == 4) {
          printf("        Subnet Mask: "); print_ip(stream); printf("\n"); 
        }
        stream += op_len;
        read += op_len + 1;
        break;
      case 3: // router
        op_len = *stream++;
        if (op_len % 4 == 0) {

          if (op_len > 4) {
            printf("        Routers: ");
          }
          else {
            printf("        Router: ");
          }
          for(int i = 0; i < op_len; i+=4) {
            print_ip(stream + i);
            printf(" ");
          }
          printf("\n");
        }
        stream += op_len;
        read += op_len + 1;
        break;
      case 4: // time server
        op_len = *stream++;
        if (op_len % 4 == 0) {

          if (op_len > 4) {
            printf("        Time Servers: ");
          }
          else {
            printf("        Time Server: ");
          }
          for(int i = 0; i < op_len; i+=4) {
            print_ip(stream + i);
            printf(" ");
          }
          printf("\n");
        }
        stream += op_len;
        read += op_len + 1;

        break;
      case 6: // domain name server
        op_len = *stream++;
        if (op_len % 4 == 0) {

          if (op_len > 4) {
            printf("        DNS Servers: ");
          }
          else {
            printf("        DNS Server: ");
          }
          for(int i = 0; i < op_len; i+=4) {
            print_ip(stream + i);
            printf(" ");
          }
          printf("\n");
        }
        stream += op_len;
        read += op_len + 1;

        break;
      // host name
      case 12:
        op_len = *stream++;
        char* host_name = calloc(1, op_len + 1);
        memcpy(host_name, stream, op_len);
        printf("        Host Name: %s\n", host_name);
        free(host_name);
        stream += op_len;
        read += op_len + 1;
        break;
      // domain name
      case 15:
        op_len = *stream++;
        char* domain_name = calloc(1, op_len + 1);
        memcpy(domain_name, stream, op_len);
        printf("       Domain Name: %s\n", domain_name);
        free(domain_name);
        stream += op_len;
        read += op_len + 1;
        break;
      // ntp serve
      case 42:
        op_len = *stream++;
        if (op_len % 4 == 0) {

          if (op_len > 4) {
            printf("        NTP Servers: ");
          }
          else {
            printf("        NTP Server: ");
          }
          for(int i = 0; i < op_len; i+=4) {
            print_ip(stream + i);
            printf(" ");
          }
          printf("\n");
        }
        stream += op_len;
        read += op_len + 1;


        break;
      // dhcp message type
      case 53:
        op_len = *stream++;
        if (op_len == 1) {
          printf("        Message Type: ");
          print_dhcp_message_type(*stream);
          printf(" (%hhu)\n", *stream);
        }
        stream += op_len;
        read += op_len + 1;
        break;
      case 255: // end option
        break;
      default:
        op_len = *stream++;
        stream += op_len;
        read += op_len + 1;
    }
  } while (op != 255 && read < len);
}

int main(int argc, char** argv) {

  char* filename = NULL;
  bool domain_name_mode = false;
  // bool arp_detail = false;


  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-IP-use-names") == 0) {
      domain_name_mode = true;
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
      /*
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
      */
      // UDP packet
      if (ip->type == 17) {
        struct udphdr udp;
        memcpy(&udp, IP_PAYLOAD_OFFSET, sizeof(struct udphdr));
        printf("  UDP: srcport=%hu dstport=%hu len=%hu\n", ntohs(udp.srcport),
            ntohs(udp.destport), ntohs(udp.msglen));

        // payload length is 32 or udp length whichever smaller
        int payloadlen = ntohs(udp.msglen) > 32 ? 32 : ntohs(udp.msglen);
        char *udppayload = (char*)(IP_PAYLOAD_OFFSET + sizeof(struct udphdr));
        // dhcp 
        unsigned char destport = ntohs(udp.destport);
        unsigned char srcport = ntohs(udp.srcport);
        if (destport == 67 || destport == 68 || srcport == 67 || srcport == 68) {
          struct dhcphdr dhcp = *((struct dhcphdr *)(udppayload));

          printf("    DHCP:\n");
          printf("      OP: %hhu", dhcp.op);
          switch (dhcp.op) {
            case 1:
              printf(" (Boot Request)\n");
              break;
            case 2:
              printf(" (Boot Response)\n");
              break;
            default:
              printf(" (Neither)\n");
          }
          printf("      Hardware Type: %hhu", dhcp.htype);
          switch (dhcp.htype)  {
            case 1:
              printf(" (Ethernet)\n");
              break;
            default:
              printf(" (Unknown)\n");
          }
          printf("      Hardware Address Length: %hhu\n", dhcp.hlen);
          printf("      Hops: %hhu\n", dhcp.hops);
          printf("      Transaction ID: 0x%08x\n", ntohl(dhcp.xid));
          printf("      Seconds Elpased: %hhu\n", ntohs(dhcp.secs));
          printf("      Flags: 0x%04hx", ntohs(dhcp.flags));
          if (ntohs(dhcp.flags) && 0x8000) {
            printf(" (Broadcast)\n");
          }
          else {
            printf("\n");
          }
          printf("      Client IP: "); print_ip(dhcp.ciaddr); printf("\n");
          printf("      Your IP: "); print_ip(dhcp.yiaddr); printf("\n");
          printf("      Server IP: "); print_ip(dhcp.siaddr); printf("\n");
          printf("      Gateway IP: "); print_ip(dhcp.giaddr); printf("\n");
          printf("      Client Hardware Address: "); print_mac(dhcp.chaddr); printf("\n");
          printf("      Server Name: %s\n", dhcp.sname);
          printf("      File Name: %s\n", dhcp.file);
          unsigned char *options = (unsigned char*)(udppayload + sizeof(struct dhcphdr));
          int max_len = ntohs(udp.msglen) - sizeof(struct dhcphdr);
          printf("      Options:\n");
          dhcp_option_parser(options, max_len);
        }
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
